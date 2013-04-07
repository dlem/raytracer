#include "csg.hpp"
#include "cmdopts.hpp"
#include "intersection.hpp"
#include "rt.hpp"

using namespace std;

void CSGPrimitive::init(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans)
{
  // Flatten our operands.
  lhs->flatten(m_lhs_list, trans);
  rhs->flatten(m_rhs_list, trans);
  if(m_lhs_list.size() != 1 || m_rhs_list.size() != 1)
  {
    errs() << "CSG operand contains more than one primitive!" << endl;
    errs() << "Aborting" << endl;
    exit(1);
  }

  m_lhs = &m_lhs_list[0];
  m_rhs = &m_rhs_list[0];

  // Compute our bounding box.
  Box box, bl, br;
  m_lhs->prim->bounding_box(bl);
  m_rhs->prim->bounding_box(br);
  bl.apply(m_lhs->trans);
  br.apply(m_rhs->trans);

  combine_bounding_boxes(box, bl, br);

  m_mins = box.mins();
  m_maxes = box.maxes();
}

bool CSGPrimitive::intersect(const Point3D &src, const Point3D &dst, HitReporter &hr) const
{
  // This can take a while. Therefore, we'll test a bounding volume first.
  if(!GETOPT(disable_csg_bb) && !axis_aligned_box_check(src, dst, m_mins, m_maxes))
    return true;

  SegmentList segments;
  get_segments(segments, src, dst);

  // Each interface between our component volumes counts is a hit. Caller can
  // sort the mess out by checking their t-values.
  for(auto &seg : segments)
  {
    hr.report(seg);
    if(!hr.report())
      return false;
  }
  return true;
}

void CSGPrimitive::get_segments(SegmentList &out, const Point3D &src, const Point3D &dst) const
{
  SegmentList lhs, rhs;
  SegmentList *lists[] = {&lhs, &rhs};
  FlatGeo *geos[] = {m_lhs, m_rhs};

  // We'll first build the two segment lists for our two children, then we'll
  // combine them using derived's overrides.

  for(int i = 0; i < NUMELMS(lists); i++)
  {
    SegmentList &l = *lists[i];
    FlatGeo &geo = *geos[i];

    if(geo.prim->is_csg())
    {
      // We don't want to repeatedly convert back and forth between normal hits
      // and line segments; so if one of our children is a CSG object, use its
      // get_segments method.
      auto prim = static_cast<const CSGPrimitive *>(geo.prim);
      prim->get_segments(l, src, dst);
    }
    else
    {
      // If it's a primitive, do a normal raytrace and record each hit as an
      // interface to/from air (depending on the normal direction). Note that
      // bump mapping won't have been applied at this point.

      const Point3D srcprime = geo.invtrans * src;
      const Point3D dstprime = geo.invtrans * dst;
      const Vector3D rayprime = dstprime - srcprime;

      SegmentList lprim;
      RaytraceFn fn([&lprim, &rayprime]
	  (const HitInfo &hi)
	  {
	    assert(hi.primary);
	    lprim.push_back(hi);
	    SegInterface &si = lprim.back();
	    if(si.normal.dot(rayprime) > 0)
	    {
	      const FlatGeo *tmp = si.from;
	      si.from = si.to;
	      si.to = tmp;
	    }
	    return true;
	  });

      HitReporter hr(fn);
      hr.to = hr.primary = &geo;
      geo.prim->intersect(srcprime, dstprime, hr);

      // Sort primitive interfaces so that they're in ascending order of their
      // t-value. This is required by our adjust_segments function.

      struct SortCriteria
      {
	bool operator()(const SegInterface &si1, const SegInterface &si2)
	{ return si1.t < si2.t; }
      };

      sort(lprim.begin(), lprim.end(), SortCriteria());
      l.insert(l.end(), lprim.begin(), lprim.end());
    }
  }

  adjust_segments(out, *lists[0], *lists[1]);
}

void CSGUnion::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
  // The segment adjustment for the CSG Union function. Preserves interfaces
  // between the two primitives, which allows us to model crosses between
  // different transparent mediums. The right operand always takes precedence
  // over the left operand when they intersect.
  
  // We iterate through both segment lists, each time checking which of the
  // subsequent interections has the lowest t-value and adding an interface to
  // our output list as necessary.

  const double epsilon = 0.001;
  auto i1 = c1.begin(), i2 = c2.begin();
  const FlatGeo *cur1 = 0, *cur2 = 0;

  while(i1 != c1.end() && i2 != c2.end())
  {
    assert(i1->from || i1->to);
    assert(i2->from || i2->to);

    if(abs(i2->t - i1->t) < epsilon)
    {
      // Interfaces happen in (approximately) the same place, so count them as
      // just one; this makes refraction between non-air mediums work out for
      // objects with touching surfaces.

      cur1 = i1->to;
      cur2 = i2->to;

      out.push_back(*i2);
      out.back().from = i2->from ? i2->from : i1->from;
      out.back().to = i2->to ? i2->to : i1->to;

      i1++;
      i2++;
    }
    else if(i2->t < i1->t)
    {
      cur2 = i2->to;
      if(!i2->from && i2->to)
      {
	// Second child begins here. Push the interface unless we're already
	// inside first child and it's the same material.
	if(!cur1 || cur1 != i2->to)
	{
	  out.push_back(*i2);
	  out.back().from = cur1;
	}
      }
      else if(i2->from && !i2->to)
      {
	// Second child ends.
	if(!cur1 || cur1 != i2->from)
	{
	  out.push_back(*i2);
	  out.back().to = cur1;
	}
      }
      else
      {
	// Interface inside second child.
	out.push_back(*i2);
      }
      i2++;
    }
    else
    {
      cur1 = i1->to;
      // Second child overrides first, so only do anything if second is
      // inactive.
      if(!cur2)
      {
	out.push_back(*i1);
      }
      i1++;
    }
  }

  // Shove in any leftovers.
  out.insert(out.end(), i1, c1.end());
  out.insert(out.end(), i2, c2.end());
}

void CSGUnion::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  // For union, combine the bounding boxes.
  out.set(cw_min(bl.mins(), br.mins()), cw_max(bl.maxes(), br.maxes()));
}

void CSGIntersection::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  // For intersection, intersect them (sort of).
  out.set(cw_max(bl.mins(), br.mins()), cw_min(bl.maxes(), br.maxes())); 
}

void CSGDifference::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  // For difference, just use the bb of the first operand.
  out = bl;
}

void CSGIntersection::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
  auto i1 = c1.begin(), i2 = c2.begin();
  bool inside_i1 = false, inside_i2 = false;

  // Iterate through both segment lists, at each step checking which interface
  // occurs next and pushing and output interface as appropriate.
  while(i1 != c1.end() && i2 != c2.end())
  {
    if(i1->t < i2->t)
    {
      if(!i1->from && i1->to)
      {
	// 1st operand begins. We begin iff 2nd operand is active.
	inside_i1 = true;
	if(inside_i2)
	{
	  out.push_back(*i1);
	}
      }
      else if(i1->from && !i1->to)
      {
	// 1st operand ends. We end if we were active.
	inside_i1 = false;
	if(inside_i2)
	{
	  out.push_back(*i1);
	}
      }
      i1++;
    }
    else
    {
      if(!i2->from && i2->to)
      {
	// 2nd operand begins. We begin if 1st is active.
	inside_i2 = true;
	if(inside_i1)
	{
	  out.push_back(*i2);
	}
      }
      else if(i2->from && !i2->to)
      {
	// 2nd operand ends. We end if 1st is active.
	inside_i2 = false;
	if(inside_i1)
	{
	  out.push_back(*i2);
	}
      }
      i2++;
    }
  }
}

void CSGDifference::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
  auto i1 = c1.begin(), i2 = c2.begin();
  const FlatGeo *cur1 = 0, *cur2 = 0;

  // Same idea as union and intersection, except we use difference semantics.
  while(i1 != c1.end() || i2 != c2.end())
  {
    if(i1 != c1.end() && (i2 == c2.end() || i1->t < i2->t))
    {
      cur1 = i1->to;
      if(!i1->from && i1->to)
      {
	// 1st operand begins. If 2nd isn't currently active, we can begin.
	if(!cur2)
	{
	  out.push_back(*i1);
	}
      }
      else if(i1->from && !i1->to)
      {
	// 1st operand ends. If we're active, we end.
	if(!cur2)
	{
	  out.push_back(*i1);
	}
      }
      i1++;
    }
    else
    {
      cur2 = i2->to;
      if(!i2->from && i2->to)
      {
	// 2nd operand begins. If we were active, we end.
	if(cur1)
	{
	  // Make sure to invert the interfaces and the normal!
	  assert(out.size() > 0);
	  out.push_back(*i2);
	  out.back().from = out.back().primary = i2->to;
	  out.back().to = 0;
	  out.back().normal = -out.back().normal;
	}
      }
      else if(i2->from && !i2->to)
      {
	// 2nd operand ends. If 1st is active, we begin.
	if(cur1)
	{
	  out.push_back(*i2);
	  out.back().to = out.back().primary = out.back().from;
	  out.back().from = 0;
	  out.back().normal = -out.back().normal;
	}
      }
      i2++;
    }
  }
}

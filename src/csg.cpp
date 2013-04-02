#include "csg.hpp"
#include "cmdopts.hpp"
#include "intersection.hpp"
#include "rt.hpp"

using namespace std;

void CSGPrimitive::init(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans)
{
  lhs->flatten(m_lhs_list, trans);
  rhs->flatten(m_rhs_list, trans);
  if(m_lhs_list.size() != 1 || m_rhs_list.size() != 1)
  {
    errs() << "CSG object doesn't have exactly one child!" << endl;
    errs() << "Aborting" << endl;
    exit(1);
  }

  m_lhs = &m_lhs_list[0];
  m_rhs = &m_rhs_list[0];

  // Now compute the bounding box.
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

  for(int i = 0; i < NUMELMS(lists); i++)
  {
    SegmentList &l = *lists[i];
    FlatGeo &geo = *geos[i];

    if(geo.prim->is_csg())
    {
      auto prim = static_cast<const CSGPrimitive *>(geo.prim);
      prim->get_segments(l, src, dst);
    }
    else
    {
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
  const double epsilon = 0.001;
  auto i1 = c1.begin(), i2 = c2.begin();
  const FlatGeo *cur1 = 0, *cur2 = 0;

  while(i1 != c1.end() && i2 != c2.end())
  {
    assert(i1->from || i1->to);
    assert(i2->from || i2->to);

    if(abs(i2->t - i1->t) < epsilon)
    {
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
	if(!cur1 || cur1 != i2->to)
	{
	  out.push_back(*i2);
	  out.back().from = cur1;
	}
      }
      else if(i2->from && !i2->to)
      {
	out.push_back(*i2);
	out.back().to = cur1;
      }
      else
      {
	out.push_back(*i2);
      }
      i2++;
    }
    else
    {
      cur1 = i1->to;
      if(!cur2)
      {
	out.push_back(*i1);
      }
      i1++;
    }
  }

  out.insert(out.end(), i1, c1.end());
  out.insert(out.end(), i2, c2.end());
}

void CSGUnion::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  out.set(cw_min(bl.mins(), br.mins()), cw_max(bl.maxes(), br.maxes()));
}

void CSGIntersection::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  out.set(cw_max(bl.mins(), br.mins()), cw_min(bl.maxes(), br.maxes())); 
}

void CSGDifference::combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const
{
  out = bl;
}

void CSGIntersection::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
  auto i1 = c1.begin(), i2 = c2.begin();
  bool inside_i1 = false, inside_i2 = false;

  while(i1 != c1.end() && i2 != c2.end())
  {
    if(i1->t < i2->t)
    {
      if(!i1->from && i1->to)
      {
	inside_i1 = true;
	if(inside_i2)
	{
	  out.push_back(*i1);
	}
      }
      else if(i1->from && !i1->to)
      {
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
	inside_i2 = true;
	if(inside_i1)
	{
	  out.push_back(*i2);
	}
      }
      else if(i2->from && !i2->to)
      {
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

  while(i1 != c1.end() || i2 != c2.end())
  {
    if(i1 != c1.end() && (i2 == c2.end() || i1->t < i2->t))
    {
      cur1 = i1->to;
      if(!i1->from && i1->to)
      {
	if(!cur2)
	{
	  out.push_back(*i1);
	}
      }
      else if(i1->from && !i1->to)
      {
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
	if(cur1)
	{
	  assert(out.size() > 0);
	  out.push_back(*i2);
	  out.back().from = out.back().primary = i2->to;
	  out.back().to = 0;
	  out.back().normal = -out.back().normal;
	}
      }
      else if(i2->from && !i2->to)
      {
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

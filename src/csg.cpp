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

bool CSGPrimitive::intersect(const Point3D &src, const Point3D &dst, HitInfo &hi) const
{
  // This can take a while. Therefore, we'll test a bounding volume first.
  if(!axis_aligned_box_check(src, dst, m_mins, m_maxes))
    return true;

  SegmentList segments;
  get_segments(segments, src, dst);

  for(auto &seg : segments)
  {
    hi.geo = seg.penetrating ? seg.to : seg.from;
    assert(hi.geo);
    const FlatGeo *med = seg.penetrating ? seg.from : seg.to;
    hi.med = med ? med->mat : &Material::air;

    if(!hi.report(seg.t, seg.normal, seg.uv, seg.u, seg.v))
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
	  (const FlatGeo &g, const Material &, double t, const Vector3D &normal,
	   const Point2D &uv, const Vector3D &u, const Vector3D &v)
	  {
	    const bool penetrating = rayprime.dot(normal) < 0;
	    SegInterface si = { t, penetrating, normal, uv, u, v,
		      penetrating ? 0 : &g, penetrating ? &g : 0 };
	    lprim.push_back(si);
	    assert(&g);
	    assert(si.penetrating ? si.to : si.from);
	    return true;
	  });

      HitInfo hi(fn);
      hi.geo = &geo;
      hi.med = &PhongMaterial::air;
      geo.prim->intersect(srcprime, dstprime, hi);

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
  SegInterface *prev = 0;

  while(i1 != c1.end() && i2 != c2.end())
  {
    SegInterface *si = 0;

    if(abs(i1->t - i2->t) < epsilon)
    {
      si = &*i2;
      if(!si->from)
      {
	si->from = i1->from;
	assert(!prev || prev->to == i1->from);
      }
      if(!si->to)
      {
	si->to = i1->to;
      }

      i1++;
      i2++;
    }
    else if(i1->t < i2->t)
    {
      si = &*i1++;
      si->from = (prev && prev->to) ? prev->to : si->from;
    }
    else
    {
      si = &*i2++;
      si->from = (si->from || !prev) ? si->from : prev->to;
    }

    if(prev)
      prev->to = si->from;
    out.push_back(*si);
    prev = &out.back();
  }

  if(i2 != c2.end())
  {
    if(prev)
    {
      if(i2->from)
	prev->to = i2->from;
      else
	i2->from = prev->to;
    }

    out.insert(out.end(), i2, c2.end());
  }

  if(i1 != c1.end())
  {
    if(prev)
    {
      if(prev->to)
	i1->from = prev->to;
      else
	prev->to = i1->from;
    }

    out.insert(out.end(), i1, c1.end());
  }

  for(int i = 0; i < out.size(); i++)
  {
    assert(out[i].penetrating ? out[i].to : out[i].from);
  }
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
#if 0
  auto i1 = c1.begin(), i2 = c2.begin();
  while(i1 != c1.end() && i2 != c2.end())
  {
    if(i1->t1() < i2->t0())
      i1++;
    else if(i2->t1() < i1->t0())
      i2++;
    else
    {
      LineSegment ls;
      ls.start = i1->t0() > i2->t0() ? i1->start : i2->start;
      ls.end = i1->t1() > i2->t1() ? i2->end : i1->end;
      (i1->t1() > i2->t1() ? i2 : i1)++;
      out.push_back(ls);
    }
  }
#endif
}

void CSGDifference::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
#if 0
  auto i1 = c1.begin(), i2 = c2.begin();
  while(i1 != c1.end() && i2 != c2.end())
  {
    if(i1->t1() < i2->t0())
    {
      out.push_back(*i1++);
    }
    else if(i2->t1() < i1->t0())
    {
      i2++;
    }
    else
    {
      if(i1->t0() < i2->t0())
      {
	LineSegment ls;
	ls.start = i1->start;
	ls.end = i2->start;
	ls.end.normal = -ls.end.normal;
	out.push_back(ls);
      }
      if(i1->t1() < i2->t1())
      {
	i1++;
      }
      else
      {
	i1->start = i2->end;
	i1->start.normal = -i1->start.normal;
	i2++;
      }
    }
  }

  while(i1 != c1.end())
    out.push_back(*i1++);
#endif
}

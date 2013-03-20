#include "csg.hpp"
#include "cmdopts.hpp"
#include "intersection.hpp"
#include "rt.hpp"

using namespace std;

void CSGPrimitive::init(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans)
{
  lhs->flatten(m_lhs_list, trans);
  rhs->flatten(m_rhs_list, trans);
  if(m_lhs_list.size() > 1 || m_rhs_list.size() > 1)
  {
    errs() << "CSG object has more than one child!" << endl;
    errs() << "Aborting" << endl;
    exit(1);
  }
  m_lhs = &m_lhs_list[0];
  m_rhs = &m_rhs_list[0];
}

bool CSGPrimitive::intersect(const Point3D &src, const Point3D &dst, HitInfo &hi) const
{
#if 0
  // This can take a while. Therefore, we'll test a bounding volume first.
  if(!axis_aligned_box_check(src, dst, m_mins, m_maxes))
    return true;
#endif

  SegmentList segments;
  get_segments(segments, src, dst);

  for(auto &seg : segments)
  {
    for(int i = 0; i < 2; i++)
    {
      const SegmentEnd &end = seg[i];
      hi.geo = end.geo;
      if(!hi.report(end.t, end.normal, end.uv, end.u, end.v))
	return false;
    }
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

    if(const CSGPrimitive *prim = dynamic_cast<const CSGPrimitive *>(geo.prim))
    {
      prim->get_segments(l, src, dst);
    }
    else
    {
      // Assume it's concave. This is a good assumption, since it's a non-csg
      // primitive. Good thing torii aren't in my objectives.

      LineSegment ls;
      int n = 0;

      RaytraceFn fn([&n, &ls]
	  (const FlatGeo &, double t, const Vector3D &normal,
	   const Point2D &uv, const Vector3D &u, const Vector3D &v)
	  {
	    ls[n].t = t; 
	    ls[n].normal = normal;
	    ls[n].uv = uv;
	    ls[n].u = u;
	    ls[n].v = v;
	    n++;
	    return n < 2;
	  });

      HitInfo hi(fn);
      geo.prim->intersect(geo.invtrans * src, geo.invtrans * dst, hi);
      assert(n <= 2);
      if(n >= 2)
      {
	if(ls[0].t > ls[1].t)
	{
	  const SegmentEnd tmp = ls[0];
	  ls[0] = ls[1];
	  ls[1] = tmp;
	}

	ls[0].geo = ls[1].geo = &geo;
	l.push_back(ls);
      }
    }
  }

  adjust_segments(out, *lists[0], *lists[1]);
}

void CSGUnion::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
  auto i1 = c1.begin(), i2 = c2.begin();
  auto done = [&i1, &i2, &c1, &c2] ()
      { return i1 == c1.end() || i2 == c2.end(); };

  while(!done())
  {
    // Assume the input lists are sorted, and assume that no overlaps occur in a
    // single list. Our next output segment starts at whichever output segment
    // comes first, and ends as soon as we stop being able to merge subsequent
    // lines.
    auto &merge2 = i1->t0() < i2->t0() ? i1 : i2;
    auto &merge1 = i1->t0() < i2->t0() ? i2 : i1;
    auto end2 = i1->t0() < i2->t0() ? c1.end() : c2.end();
    auto end1 = i1->t0() < i2->t0() ? c2.end() : c1.end();
    out.push_back(*merge2++);
    LineSegment &segment = out.back();

    for(;;)
    {
      // Increment merge1 while it's entirely contained in our current segment.
      while(merge1 != end1 && merge1->t1() < segment.t1())
	merge1++;

      if(merge1 == end1)
	goto _done;

      if(merge1->t0() > segment.t1())
	// The lines don't intersect.
	break;

      segment.end = (merge1++)->end;

      while(merge2 != end2 && merge2->t1() < segment.t1())
	merge2++;

      if(merge2 == end2)
	goto _done;

      if(merge2->t0() > segment.t1())
	// The lines don't intersect.
	break;

      segment.t1() = (merge2++)->t1();
    }
  }

_done:
  while(i1 != c1.end())
    out.push_back(*i1++);
  while(i2 != c2.end())
    out.push_back(*i2++);
}

void CSGIntersection::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
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
}

void CSGDifference::adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const
{
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
}

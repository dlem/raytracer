#include "csg.hpp"
#include "cmdopts.hpp"
#include "intersection.hpp"

using namespace std;

CSGPrimitive::CSGPrimitive(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans)
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

bool CSGPrimitive::intersect(const Point3D &eye, const Point3D &dst, const IntersectFn &fn)
{
  // This can take a while. Therefore, we'll test a bounding volume first.
  if(!axis_aligned_box_check(eye, dst, m_mins, m_maxes))
    return true;

  SegmentList segments;
  get_segments(segments, eye, dst);

  for(auto &seg : segments)
  {
    // This isn't really correct -- two points should be reported per segment.
    // But we're guaranteed that there's no CSG above us, so it should work
    // anyway...
    if(!fn(seg.start, seg.normal, seg.uv, seg.u, seg.v))
      return false;
  }

  return true;
}

void CSGPrimitive::get_segments(SegmentList &out, const Point3D &eye, const Point3D &dst) const
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
      prim->get_segments(l, eye, dst);
    }
    else
    {
      // Assume everything is concave.
      LineSegment ls;
      int n = 0;
      geo.prim->intersect(eye, dst, [&n, &ls](double t, const Vector3D &normal, const Point2D &uv, const Vector3D &u, const Vector3D &v)
	  {
	    const bool first = 0 == n++;
	    const bool do_set = first || t < ls.start;
	    if(first)
	    {
	      ls.start = t;
	    }
	    else
	    {
	      ls.end = max(t, ls.start);
	      ls.start = min(t, ls.start);
	    }
	    if(do_set)
	    {
	      ls.normal = normal;
	      ls.uv = uv;
	      ls.u = u;
	      ls.v = v;
	    }
	    return n < 2;
	  });

      if(n == 2)
      {
	ls.geo = &geo;
	l.push_back(ls);
      }
    }

    struct SortCriteria
    {
      bool operator()(const LineSegment &s1, const LineSegment &s2) const
      { return s1.start < s2.start; }
    };

    sort(l.begin(), l.end(), SortCriteria());
  }

  adjust_segments(out, *lists[0], *lists[1]);
}

void CSGUnion::adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const
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
    auto &merge2 = i1->start < i2->start ? i1 : i2;
    auto &merge1 = i1->start < i2->start ? i2 : i1;
    auto end2 = i1->start < i2->start ? c1.end() : c2.end();
    auto end1 = i1->start < i2->start ? c2.end() : c1.end();
    out.push_back(*merge2++);
    Segment &segment = out.back();

    for(;;)
    {
      // Increment merge1 while it's entirely contained in our current segment.
      while(merge1 != end1 && merge1->end < segment.end)
	merge1++;

      if(merge1 == end1)
	goto _done;

      if(merge1->start > segment.end)
	// The lines don't intersect.
	break;

      segment.end = (merge1++)->end;

      while(merge2 != end2 && merge2->end < segment.end)
	merge2++;

      if(merge2 == end2)
	goto _done;

      if(merge2->start > segment.end)
	// The lines don't intersect.
	break;

      segment.end = (merge2++)->end;
    }
  }

_done:
  while(i1 != c1.end())
    out.push_back(*i1++);
  while(i2 != c2.end())
    out.push_back(*i2++);
}

void CSGIntersection::adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const
{
}

void CSGDifference::adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const
{
}

template<typename TPrim>
void CSGNode<TPrim>::flatten(FlatList &fl, const Matrix4x4 &trans)
{
  Matrix4x4 trans_prime = trans * m_trans;
  fl.push_back(FlatGen(trans_prime.invert(),
	       trans_prime.linear().invert().transpose(),
	       new TPrim(m_lhs, m_rhs, trans_prime),
	       0));
  static_assert(false, "Still need material");
}

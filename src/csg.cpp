#include "csg.hpp"
#include "cmdopts.hpp"
#include "intersection.hpp"

using namespace std;

CSGPrimitive::CSGPrimitive(SceneNode *lhs, SceneNode *rhs)
{
  lhs->flatten(m_lhs_list);
  rhs->flatten(m_rhs_list);
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
	      ls.end = max(t, ls.start);
	      ls.start = min(t, ls.start);
	    }
	    else
	    {
	      ls.start = t;
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
	ls.normal = geo.trans_normal * ls.normal;
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
  while(i1 != c1.end() && i2 != c2.end())
  {
    const double end = max(i1->end, i2->end);
    out.push_back(i1->start < i2->start ? *i1 : *i2);
    out.back().end = end;
    while(i1 != c1.end() && i1->end <= end) i1++;
    while(i2 != c2.end() && i2->end <= end) i2++;
  }

  while(i1 != c1.end())
    out.push_back(*i1++);
  while(i2 != c2.end())
    out.push_back(*i2++);
}

void CSGIntersection::adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const
{
  static_assert(false, "Not implemented");
}

void CSGDifference::adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const
{
  static_assert(false, "Not implemented");
}

template<typename TPrim>
void CSGNode<TPrim>::flatten(FlatList &fl, const Matrix4x4 &trans)
{
  Matrix4x4 trans_prime = trans * m_trans;
  fl.push_back(FlatGen(trans_prime.invert(),
	       trans_prime.linear().invert().transpose(),
	       new TPrim(m_lhs, m_rhs),
	       0));
  static_assert(false, "Still need material");
}

#ifndef __CSG_HPP__
#define __CSG_HPP__

#include "scene.hpp"

template<typename TPrim>
class CSGNode : public SceneNode
{
public:
  CSGNode(SceneNode *lhs, SceneNode *rhs)
    : SceneNode("CSG node")
    , m_lhs(lhs), m_rhs(rhs)
  {}

  virtual ~CSGNode() {}

  virtual void flatten(FlatList &fl, const Matrix4x4 &trans);

private:
  SceneNode *m_lhs, *m_rhs;
};

class CSGPrimitive : public Primitive
{
public:
  CSGPrimitive(SceneNode *lhs, SceneNode *rhs);

  virtual bool intersect(const Point3D &eye, const Point3D &dst, const IntersectFn &fn);

protected:
  struct LineSegment
  {
    double start, end;
    Vector3D normal;
    Point2D uv;
    Vector3D u, v;
  };
  typedef std::vector<LineSegment> SegmentList;

  void get_segments(SegmentList &out, const Point3D &eye, const Point3D &dst) const;
  virtual void adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const = 0;

private:
  FlatList m_lhs_list;
  FlatList m_rhs_list;
  FlatGeo *m_lhs;
  FlatGeo *m_rhs;
  Point3D m_mins;
  Point3D m_maxes;
};

class CSGUnion : public CSGPrimitive
{
protected:
  void adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const;
};

class CSGIntersection : public CSGPrimitive
{
protected:
  void adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const;
};

class CSGDifference : public CSGPrimitive
{
protected:
  void adjust_segments(SegmentList &out, const SegmentList &c1, const SegmentList &c2) const;
};

typedef CSGNode<CSGUnion> CSGUnionNode;
typedef CSGNode<CSGIntersection> CSGIntersectionNode;
typedef CSGNode<CSGDifference> CSGDifferenceNode;

#endif

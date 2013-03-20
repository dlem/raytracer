#ifndef __CSG_HPP__
#define __CSG_HPP__

#include "scene.hpp"


class CSGPrimitive : public Primitive
{
public:
  CSGPrimitive()
    : m_lhs(0)
    , m_rhs(0)
  {}

  void init(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans);

  virtual bool intersect(const Point3D &eye, const Point3D &dst, HitInfo &hi) const;

protected:

  struct SegmentEnd
  {
    double t;
    Vector3D normal;
    Point2D uv;
    Vector3D u, v;
    FlatGeo *geo;
  };

  struct LineSegment
  {
    inline SegmentEnd &operator[](int ix) { return ix == 0 ? start : end; }
    double t0() const { return start.t; }
    double t1() const { return end.t; }
    double &t0() { return start.t; }
    double &t1() { return end.t; }
    SegmentEnd start, end;
  };

  typedef std::vector<LineSegment> SegmentList;

  void get_segments(SegmentList &out, const Point3D &eye, const Point3D &dst) const;
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const = 0;
  virtual void bounding_sphere(Point3D &c, double &rad) const
  {
  }

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
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
};

class CSGIntersection : public CSGPrimitive
{
protected:
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
};

class CSGDifference : public CSGPrimitive
{
protected:
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
};

template<typename TPrim>
class CSGNode : public SceneNode
{
public:
  CSGNode(SceneNode *lhs, SceneNode *rhs)
    : SceneNode("CSG node")
    , m_lhs(lhs), m_rhs(rhs)
  {}

  virtual ~CSGNode() {}

  virtual void flatten(FlatList &fl, const Matrix4x4 &trans)
  {
    Matrix4x4 trans_prime = trans * m_trans;
    CSGPrimitive *prim = new TPrim();
    prim->init(m_lhs, m_rhs, trans_prime);
    fl.push_back(FlatGeo(Matrix4x4(),
		 Matrix4x4(),
		 *prim,
		 *new PhongMaterial(Colour(1), Colour(0), 0)));
  }

private:
  SceneNode *m_lhs, *m_rhs;
};

typedef CSGNode<CSGUnion> CSGUnionNode;
typedef CSGNode<CSGIntersection> CSGIntersectionNode;
typedef CSGNode<CSGDifference> CSGDifferenceNode;

#endif

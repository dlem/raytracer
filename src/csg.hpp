/*
 * This module contains support for CSG objects. C++ LUA callbacks can create
 * and use CSG(Union|Intersection|Difference)Node objects.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __CSG_HPP__
#define __CSG_HPP__

#include "scene.hpp"
#include "rt.hpp"

// Primitive class for CSG objects.
class CSGPrimitive : public Primitive
{
public:
  CSGPrimitive()
    : m_lhs(0)
    , m_rhs(0)
  {}
 
  void init(SceneNode *lhs, SceneNode *rhs, const Matrix4x4 &trans);

  // Prim overrides.
  virtual bool intersect(const Point3D &eye, const Point3D &dst, HitReporter &hr) const;
  virtual void bounding_box(Box &b) const { b.set(m_mins, m_maxes); }
  virtual bool is_csg() const { return true; }

protected:

  typedef HitInfo SegInterface;
  typedef std::vector<SegInterface> SegmentList;

  // Get the line segments for intersections with this object.
  void get_segments(SegmentList &out, const Point3D &eye, const Point3D &dst) const;

  // Derived must override and implement depending on how it handles its
  // operands (union vs intersection vs difference).
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const = 0;
  virtual void combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const = 0;

private:
  // Operands.
  FlatList m_lhs_list;
  FlatList m_rhs_list;
  FlatGeo *m_lhs;
  FlatGeo *m_rhs;

  // Bounding box info.
  Point3D m_mins;
  Point3D m_maxes;
  Box m_box;
};

class CSGUnion : public CSGPrimitive
{
protected:
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
  virtual void combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const;
};

class CSGIntersection : public CSGPrimitive
{
protected:
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
  virtual void combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const;
};

class CSGDifference : public CSGPrimitive
{
protected:
  virtual void adjust_segments(SegmentList &out, SegmentList &c1, SegmentList &c2) const;
  virtual void combine_bounding_boxes(Box &out, const Box &bl, const Box &br) const;
};

// Node representing a CSG object. Instantiated by LUA callbacks. Gets
// instantiated for different CSG primitive types.
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
    fl.push_back(FlatGeo(Matrix4x4(), *prim,
		 *new PhongMaterial(Colour(1), Colour(0), 0)));
  }

private:
  SceneNode *m_lhs, *m_rhs;
};

typedef CSGNode<CSGUnion> CSGUnionNode;
typedef CSGNode<CSGIntersection> CSGIntersectionNode;
typedef CSGNode<CSGDifference> CSGDifferenceNode;

#endif

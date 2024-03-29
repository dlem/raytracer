/**
 * Modeling hierarchy code.
 *
 * Donated code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef SCENE_HPP
#define SCENE_HPP

#include <list>
#include "algebra.hpp"
#include "primitive.hpp"
#include "material.hpp"

// Represents a single flattened primitive.
struct FlatGeo
{
  FlatGeo() {}
  FlatGeo(const Matrix4x4 &trans,
	  const Primitive &prim, const Material &mat)
    : trans(trans)
    , invtrans(trans.invert())
    , trans_normal(trans.linear().invert().transpose())
    , prim(&prim)
    , mat(&mat)
  {}

  const FlatGeo &operator=(const FlatGeo &other)
  {
    invtrans = other.invtrans;
    trans_normal = other.trans_normal;
    prim = other.prim;
    mat = other.mat;
    return *this;
  }

  Matrix4x4 trans, invtrans, trans_normal;
  const Primitive *prim;
  const Material *mat;
};

typedef std::vector<FlatGeo> FlatList;

class GeometryNode;

class SceneNode {
public:
  SceneNode(const std::string& name);
  virtual ~SceneNode();

  const Matrix4x4& get_transform() const { return m_trans; }
  const Matrix4x4& get_inverse() const { return m_invtrans; }
  
  void set_transform(const Matrix4x4& m)
  {
    m_trans = m;
    m_invtrans = m.invert();
  }

  void set_transform(const Matrix4x4& m, const Matrix4x4& i)
  {
    m_trans = m;
    m_invtrans = i;
  }

  void add_child(SceneNode* child)
  {
    m_children.push_back(child);
  }

  void remove_child(SceneNode* child)
  {
    m_children.remove(child);
  }

  // Callbacks to be implemented.
  // These will be called from Lua.
  void rotate(char axis, double angle);
  void scale(const Vector3D& amount);
  void translate(const Vector3D& amount);

  // Returns true if and only if this node is a JointNode
  virtual bool is_joint() const;

  const std::string &name() const { return m_name; }

  void flatten(FlatList &fl) { flatten(fl, Matrix4x4()); }
  virtual void flatten(FlatList &fl, const Matrix4x4 &trans);
  
protected:

  // Useful for picking
  int m_id;
  std::string m_name;

  // Transformations
  Matrix4x4 m_trans;
  Matrix4x4 m_invtrans;

  // Hierarchy
  typedef std::list<SceneNode*> ChildList;
  ChildList m_children;
};

class JointNode : public SceneNode {
public:
  JointNode(const std::string& name);
  virtual ~JointNode();

  virtual bool is_joint() const;

  void set_joint_x(double min, double init, double max);
  void set_joint_y(double min, double init, double max);

  struct JointRange {
    double min, init, max;
  };
  
protected:

  JointRange m_joint_x, m_joint_y;
};

class GeometryNode : public SceneNode {
public:
  GeometryNode(const std::string& name,
               Primitive* primitive);
  virtual ~GeometryNode();

  const Material* get_material() const { return m_material; }
  Material* get_material() { return m_material; }
  Primitive *get_primitive() { return m_primitive; }

  void set_material(Material* material)
  {
    m_material = material;
  }

  virtual void flatten(FlatList &fl, const Matrix4x4 &trans);

private:
  Material* m_material;
  Primitive* m_primitive;
};

#endif

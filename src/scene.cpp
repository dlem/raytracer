/**
 * Modeling hierarchy code.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/


#include "scene.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name)
  : m_name(name)
{
}

SceneNode::~SceneNode()
{
}

void SceneNode::rotate(char axis, double angle)
{
  set_transform(m_trans * Matrix4x4::rotate(axis, angle));
}

void SceneNode::scale(const Vector3D& amount)
{
  set_transform(m_trans * Matrix4x4::scale(amount));
}

void SceneNode::translate(const Vector3D& amount)
{
  set_transform(m_trans * Matrix4x4::translate(amount));
}

bool SceneNode::is_joint() const
{
  return false;
}

JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
}

JointNode::~JointNode()
{
}

bool JointNode::is_joint() const
{
  return true;
}

void JointNode::set_joint_x(double min, double init, double max)
{
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max)
{
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive)
  : SceneNode(name),
    m_primitive(primitive)
{
}

GeometryNode::~GeometryNode()
{
}

void SceneNode::flatten(FlatList &fl, const Matrix4x4 &trans)
{
  Matrix4x4 trans_prime = trans * m_trans;
  for(auto c : m_children)
    c->flatten(fl, trans_prime);
}

void GeometryNode::flatten(FlatList &fl, const Matrix4x4 &trans)
{
  if(!m_primitive)
    return;

  Matrix4x4 trans_prime = trans * m_trans * m_primitive->get_transform();
  fl.push_back(FlatGeo(trans_prime, *m_primitive, *m_material));
}

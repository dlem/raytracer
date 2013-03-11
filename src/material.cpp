#include "material.hpp"

Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess)
  : m_kd(kd), m_ks(ks), m_shininess(shininess)
  , m_bumpmap(0)
  , m_texture(0)
{
}

PhongMaterial::~PhongMaterial()
{
}

Colour PhongMaterial::kd(const Point2D &uv)
{
  if(m_texture)
  {
    Point2D pt(uv[0] * m_texture->width(), uv[1] * m_texture->height());
    pt[0] = clamp(pt[0], 0, m_texture->width());
    pt[1] = clamp(pt[1], 0, m_texture->height());
    const Image &tex = *m_texture;
    Colour rv(tex(pt[0], pt[1], 0), tex(pt[0], pt[1], 1), tex(pt[0], pt[1], 2));
    return rv;
  }
  else if(m_proc_texture)
  {
    return m_proc_texture(uv[0], uv[1]);
  }
  else
  {
    return m_kd;
  }
}

void PhongMaterial::set_bumpmap(const char *bumpmap)
{
}

void PhongMaterial::set_texture(const char *texture)
{
}

void PhongMaterial::get_normal(Vector3D &normal, const Point2d &uv,
			       const Vector3D &u, const Vector3D &v)
{
  if(m_bumpmap)
  {
  }
  else if(m_proc_bumpmap)
  {
    return m_proc_bumpmap(uv[0], uv[1]);
  }
}

#include "material.hpp"
#include <iostream>
#include <limits>
#include "image.hpp"
#include "textures.hpp"

using namespace std;

PhongMaterial PhongMaterial::air(Colour(0), Colour(1), 0, 1);

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess, double ri)
  : m_kd(kd), m_ks(ks), m_shininess(shininess)
  , m_bumpmap(0)
  , m_texture(0)
  , m_ri(ri)
{
}

PhongMaterial::~PhongMaterial()
{
  delete m_texture;
  delete m_bumpmap;
}

Colour PhongMaterial::kd(const Point2D &uv) const
{
  if(m_texture)
    return (*m_texture)(uv);
  else
    return m_kd;
}

void PhongMaterial::get_normal(Vector3D &normal, const Point2D &uv,
			       const Vector3D &u, const Vector3D &v) const
{
  if(!m_bumpmap)
    return;

  const Point2D perturb = (*m_bumpmap)(uv);
  normal = normal + perturb[0] * u + perturb[1] * v;
  normal.normalize();
}

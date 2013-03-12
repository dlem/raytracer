#include "material.hpp"
#include <iostream>
#include "image.hpp"
#include "textures.hpp"

using namespace std;

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

void PhongMaterial::set_bumpmap(const string &bumpmap)
{
  delete m_bumpmap;
  m_bumpmap = 0;

  if(bumpmap == "")
    return;

  if(m_bumpmap = Bumpmap::get(bumpmap))
    return;

  cerr << "Could not set bump map: " << bumpmap << endl;
  exit(1);
}

void PhongMaterial::set_texture(const string &texture)
{
  delete m_texture;
  m_texture = 0;

  if(texture == "")
    return;

  if(m_texture = Texture::get(texture))
    return;

  cerr << "Could not set texture: " << texture << endl;
  exit(1);
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

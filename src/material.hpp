#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include <string>
#include "algebra.hpp"

class Material {
public:
  virtual ~Material();

protected:
  Material()
  {
  }
};

class Texture;
class Bumpmap;

class PhongMaterial : public Material {
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess);
  virtual ~PhongMaterial();
  Colour ks(const Point2D &uv) const { return m_ks; }
  Colour kd(const Point2D &uv) const;
  double shininess(const Point2D &uv) const { return m_shininess; }
  void get_normal(Vector3D &normal, const Point2D &uv,
		  const Vector3D &u, const Vector3D &v) const;

  void set_bumpmap(const std::string &bumpmap);
  void set_texture(const std::string &texture);

private:
  Colour m_kd;
  Colour m_ks;
  double m_shininess;

  Texture *m_texture;
  Bumpmap *m_bumpmap;
};


#endif

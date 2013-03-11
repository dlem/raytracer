#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include "algebra.hpp"

class Material {
public:
  virtual ~Material();

protected:
  Material()
  {
  }
};

class Image;

class PhongMaterial : public Material {
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess);
  virtual ~PhongMaterial();
  Colour ks(const Point2D &uv) { return m_ks; }
  Colour kd(const Point2D &uv);
  double shininess(const Point2D &uv) { return m_shininess; }
  void get_normal(Vector3D &normal, const Point2D &uv,
		  const Vector3D &u, const Vector3D &v);

  void set_bumpmap(const char *);
  void set_texture(const char *);

private:
  Colour m_kd;
  Colour m_ks;
  double m_shininess;

  Image *m_bumpmap;
  Image *m_texture;
  std::function m_proc_texture;
  std::function m_proc_bumpmap;
};


#endif

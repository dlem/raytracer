#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include <string>
#include "algebra.hpp"

class Texture;
class Bumpmap;

class PhongMaterial {
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess);
  virtual ~PhongMaterial();
  Colour ks(const Point2D &uv) const { return m_ks; }
  Colour kd(const Point2D &uv) const;
  double shininess(const Point2D &uv) const { return m_shininess; }
  double opacity() const { return m_opacity; };
  double ri() const { return m_ri; }
  void get_normal(Vector3D &normal, const Point2D &uv,
		  const Vector3D &u, const Vector3D &v) const;

  void set_bumpmap(const std::string &bumpmap);
  void set_texture(const std::string &texture);
  void set_opacity(double opacity, double ri)
  { m_opacity = opacity; m_ri = ri; }

private:
  Colour m_kd;
  Colour m_ks;
  double m_shininess;
  double m_opacity;
  double m_ri;

  Texture *m_texture;
  Bumpmap *m_bumpmap;
};

typedef PhongMaterial Material;

#endif

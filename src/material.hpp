#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include <string>
#include "algebra.hpp"

class Texture;
class Bumpmap;

class PhongMaterial
{
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess);
  virtual ~PhongMaterial();
  Colour ks(const Point2D &uv) const { return m_ks; }
  Colour kd(const Point2D &uv) const;
  double shininess(const Point2D &uv) const { return m_shininess; }
  double ri() const { return m_ri; }
  void get_normal(Vector3D &normal, const Point2D &uv,
		  const Vector3D &u, const Vector3D &v) const;

  void set_bumpmap(const std::string &bumpmap);
  void set_texture(const std::string &texture);
  void set_ri(double ri) { m_ri = ri; }

private:
  Colour m_kd;
  Colour m_ks;
  double m_shininess;
  double m_ri; // dbl_max => it's only reflective, not transparent

  Texture *m_texture;
  Bumpmap *m_bumpmap;
};

typedef PhongMaterial Material;

#endif

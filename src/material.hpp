#ifndef CS488_MATERIAL_HPP
#define CS488_MATERIAL_HPP

#include <string>
#include "algebra.hpp"
#include "textures.hpp"

class PhongMaterial
{
public:
  PhongMaterial(const Colour& kd, const Colour& ks, double shininess,
		double ri = std::numeric_limits<double>::max(),
		bool reflective = false);
  virtual ~PhongMaterial();
  Colour ks(const Point2D &uv) const { return m_ks; }
  Colour kd(const Point2D &uv) const;
  double shininess(const Point2D &uv) const { return m_shininess; }
  double ri() const { return m_ri; }
  bool transmissive() const { return m_ri != std::numeric_limits<double>::max(); }
  bool reflective() const { return m_reflective; }
  void get_normal(Vector3D &normal, const Point2D &uv,
		  const Vector3D &u, const Vector3D &v) const;

  Texture *&texture() { return m_texture; }
  const Texture *texture() const { return m_texture; }
  Bumpmap *&bumpmap() { return m_bumpmap; }
  const Bumpmap *bumpmap() const { return m_bumpmap; }

  void set_ri(double ri) { m_ri = ri; m_reflective = true; }
  void set_reflective(bool reflective) { m_reflective = reflective; }

  static PhongMaterial air;

private:
  Colour m_kd;
  Colour m_ks;
  double m_shininess;
  double m_ri; // dbl_max => it's only reflective, not transparent
  bool m_reflective;

  Texture *m_texture;
  Bumpmap *m_bumpmap;
};

typedef PhongMaterial Material;

#endif

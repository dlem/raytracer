#include "lightingmodel.hpp"
#include "material.hpp"
#include "rt.hpp"
#include "cmdopts.hpp"

using namespace std;

// Computes the occlusion of a point with respect to a light -- ie, an amount by
// which to attenuate light from that light. In other words, this function
// computes soft shadows.
static double occlusion(RayTracer &rt, Light *light, const Point3D &pt)
{
  const int resolution = GETOPT(shadow_grid);
  const double radius = light->radius;

  Vector3D w = light->position - pt;
  const double dist = w.normalize();

  if(radius <= 0 || !GETOPT(soft_shadows) || resolution <= 1)
  {
    if(rt.raytrace_within(pt, w, RT_EPSILON, dist))
      return 0;
    return 1;
  }

  // We're going to trace rays towards nodes in a resolution x resolution grid
  // around the center of the light, where the width of the grid is 2 x the
  // radius of the light. We'll throw away points that aren't actually inside
  // the projected circle.

  // Opposite over adjacent -- ie, sin(o), where o is the angle between a ray
  // straight towards the light and a ray towards the edge of it.
  const double sintheta = radius / dist;

  // # of rays that _don't_ get to the light.
  double occ = 0;

  // Number of rays we cast.
  int count = 0;

  // Create a wuv orthonormal coordinate system, where w is towards the light.
  Vector3D u(w[1], -w[0], 0);
  u.normalize();
  const Vector3D v(w.cross(u));

  // Iterate through our grid, linearly interpolating the direction in which we
  // shoot the light with respect to our u direction (for the outer loop) and
  // our v direction (for the inner loop).
  for(int i = 0; i < resolution; i++)
  {
    const double du = sintheta * (-1 + (i + 0.5) * 2. / resolution);
    for(int j = 0; j < resolution; j++)
    {
      const double dv = sintheta * (-1 + (j + 0.5) * 2. / resolution);

      const double sumsqr = du * du + dv * dv;
      if(sumsqr < 1)
      {
	const double dw = sqrt(1 - sumsqr);
	const Vector3D ray = du * u + dv * v + dw * w;
	if(!rt.raytrace_within(pt, ray, RT_EPSILON, dist))
	  occ += 1;
	count += 1;
      }
    }
  }

  // The desired attenuation is given by the proportion of rays that actually
  // arrive at the light.
  return occ / count;
}

Colour PhongModel::compute_lighting(RayTracer &rt,
				  const Point3D &src,
				  const Vector3D &ray,
				  const HitInfo &hi,
				  const double refl_attn
				  ) const
{
  const Point3D phong_P = src + hi.t * ray;

  // Projection drawing stuff.
  const Colour projc(1, 0, 0);

  // Set up Phong model variables.

  const Point2D &uv = hi.uv;
  const PhongMaterial &mat = hi.tomat();
  const Colour phong_kd = mat.kd(uv);

  Vector3D phong_n = hi.tonorm();
  const Colour &phong_ks = refl_attn * mat.ks(uv);
  const double phong_p = mat.shininess(uv);
  Vector3D phong_v = src - phong_P;
  phong_v.normalize();

  const Colour ambient = m_gimap ?
    m_gimap->query_radiance(phong_P, -ray) * phong_n.dot(Vector3D() - ray) : m_ambient;

  Colour rv = ambient * phong_kd;

  for(auto &light : m_lights)
  {
    Vector3D phong_ell = light->position - phong_P;
    const double dist = phong_ell.normalize() / GETOPT(unit_distance);

    const double occ = occlusion(rt, light, phong_P);
    if(occ < 0.01)
      continue;

    if(phong_ell.dot(phong_n) <= 0)
      // This can happen even with +if occlusion due to area lights.
      // It'll cause a darkening artifact if we continue.
      continue;

    // This is the diffuse term.
    Colour phong_ro = phong_kd;

    // Add the specular term, which is based on how close we are to the light's
    // reflection angle.
    Vector3D phong_r = 2 * phong_ell.dot(phong_n) * phong_n - phong_ell;
    // We can maybe avoid this normalize?
    phong_r.normalize();
    const double refl = phong_r.dot(phong_v);

    if(refl > 0)
      phong_ro = phong_ro + phong_ks * (pow(refl, phong_p) * phong_n.dot(phong_ell));

    // Compute the attenuation.
    const double *falloff = light->falloff;
    const double c0 = falloff[0], c1 = falloff[1], c2 = falloff[2];
    const double attenuation = c0 + c1 * dist + c2 * dist * dist;
    
    // Light mostly propagates towards the normal -- adjust the outgoing light
    // based on normal vs view vector.
    const Colour terms = phong_ro * (phong_ell.dot(phong_n) / attenuation);

    rv += light->colour * occ * terms;
  }

  // This is expensive, so try to only do it when necessary.
  if(m_caustics.size() > 50 && phong_kd.Y() > 0.2)
  {
    Colour c = m_caustics.query_radiance(phong_P, -ray) * phong_n.dot(Vector3D() - ray);
    rv += c;
  }

  return rv;
}

Colour PhotonDrawModel::compute_lighting(RayTracer &rt,
			      const Point3D &src,
			      const Vector3D &ray,
			      const HitInfo &hi,
			      const double refl_attn
			      ) const
{
  if(hi.primary->mat->kd(hi.uv).Y() < 0.2)
    return Colour(0);

  const Point3D p = src + hi.t * ray;
  Vector3D pos_rel;
  Colour c = m_map.query_photon(p, pos_rel);
  c.normalize();

  if(pos_rel.length() < 0.01)
    return c;
  else
    return Colour(0);

}

Colour PhotonsOnlyModel::compute_lighting(RayTracer &rt,
			      const Point3D &src,
			      const Vector3D &ray,
			      const HitInfo &hi,
			      const double refl_attn
			      ) const
{
  const Point3D p = src + hi.t * ray;
  const Vector3D outgoing = -ray;
  if(hi.primary->mat->kd(hi.uv).Y() > 0.2)
  {
    return m_map.query_radiance(p, outgoing);
  }
  else
  {
    return Colour(0);
  }
}

#include "lightingmodel.hpp"
#include "material.hpp"
#include "rt.hpp"
#include "cmdopts.hpp"

Colour PhongModel::compute_lighting(RayTracer &rt,
				  const Point3D &src,
				  const Vector3D &ray,
				  const double t,
				  const FlatGeo &geo,
				  const Vector3D &normal,
				  const Point2D &uv,
				  const Vector3D &u,
				  const Vector3D &v,
				  const double refl_attn
				  ) const
{
  const Point3D phong_P = src + t * ray;

  // Projection drawing stuff.
  const Colour projc(1, 0, 0);
  const bool use_proj = GETOPT(draw_caustic_pm) &&
			m_caustics.test_pm(rt, phong_P, normal);
  const Colour phong_kd = use_proj ? projc * geo.mat->kd(uv).Y() : geo.mat->kd(uv);

  Vector3D phong_n = normal;
  geo.mat->get_normal(phong_n, uv, u, v);
  const Colour &phong_ks = refl_attn * geo.mat->ks(uv);
  const double phong_p = geo.mat->shininess(uv);
  Vector3D phong_v = src - phong_P;
  phong_v.normalize();

  Colour rv = m_ambient * phong_kd;

  for(auto &light : m_lights)
  {
    Vector3D phong_ell = light->position - phong_P;
    const double dist = phong_ell.normalize();

    if(rt.raytrace_within(phong_P, phong_ell, 0.001, dist))
      // Skip this light. There's an obstacle => shadow.
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

    rv += light->colour * terms;
  }

  return rv;
}

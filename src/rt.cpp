/**
 * Ray path tracing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#include "rt.hpp"

using namespace std;

bool raytrace(const RTContext &ctx, const Point3D &src, const Point3D &ray, const RaytraceFn &fn)
{
  for(auto &geo : ctx.geo)
  {
    if(!geo.prim->intersect(geo.invtrans * src, geo.invtrans * ray,
	  [&geo, &fn](double t, const Vector3D &normal, const Point2D &uv,
                      const Vector3D &u, const Vector3D &v)
	  {
	    Vector3D normal_prime = geo.trans_normal * normal;
	    normal_prime.normalize();
	    return fn(geo, t, normal_prime, uv, u, v);
	  }))
    {
      return false;
    }
  }
  return true;
}

bool raytrace_within(const RTContext &ctx, const Point3D &src,
                     const Point3D &ray,
                     double tlo, double thi)
{
  return !raytrace(ctx, src , ray,
    [tlo, thi](const FlatGeo &, double tcur, const Vector3D &,
               const Point2D &, const Vector3D &, const Vector3D &)
  {
    return tcur < tlo || tcur > thi;
  });
}

double raytrace_min(const RTContext &ctx, const Point3D &src,
    const Point3D &ray,
    double tlo, const FlatGeo **pg,
    Vector3D &n,
    Point2D &uv,
    Vector3D &u,
    Vector3D &v)
{
  *pg = 0;
  double tmin = numeric_limits<double>::max();
  raytrace(ctx, src, ray, [tlo, &tmin, pg, &n, &uv, &u, &v]
      (const FlatGeo &g, double tcur, const Vector3D &_n,
       const Point2D &_uv, const Vector3D &_u, const Vector3D &_v)
  {
    if(tcur < tmin && tcur >= tlo)
    {
      tmin = tcur;
      *pg = &g;
      n = _n;
      uv = _uv;
      u = _u;
      v = _v;
    }
    return true;
  });
  return tmin;
}

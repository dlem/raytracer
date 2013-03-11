/**
 * A4 render function.
 *
 * Donated code; modified by dlemmond.
**/

#include "a4.hpp"
#include <limits>
#include "image.hpp"
#include "threading.hpp"
#include "cmdopts.hpp"
#include "rt.hpp"

using namespace std;

// Generates a sunny background with rising-sun-style rays. I think it looks
// nice.
Colour ray_pupil_background(const RTContext &ctx,
                            const Point3D &,
                            const Point3D &projected)
{
  const Colour darker(1., 0.29, 0);
  const Colour lighter(1., 0.56, 0);
  const int arcs = 28;
  const double pupil_proportion = 0.1;
  bool in_pupil = false;

  const double offset_x = 2 * projected[0] / ctx.width - 1;
  const double offset_y = 2 * projected[1] / ctx.height - 1;
  const double radius = sqr(offset_x) + sqr(offset_y);

  in_pupil |= radius <= pupil_proportion;

  if(!in_pupil)
  {
    double theta = atan(safe_div(offset_y, offset_x));
    theta /= 2 * M_PI / arcs;
    theta -= 0.5;
    in_pupil |= (int)abs(floor(theta)) % 2 >= 1;
  }

  return in_pupil ? darker : lighter;
}

// Performs the phong model lighting calculation.
Colour phong(const RTContext &ctx,
             const Point3D &ray,
             const double t,
             const FlatGeo &geo,
             const Vector3D &normal,
	     const Point2D &uv,
	     const Vector3D &u,
	     const Vector3D &v
             )
{
  assert(dynamic_cast<const PhongMaterial *>(geo.mat));

  const PhongMaterial *pm = static_cast<const PhongMaterial *>(geo.mat);
  Vector3D phong_n = normal;
  pm->get_normal(phong_n, uv, u, v);
  const Point3D &eye = ctx.eye;
  const Colour &phong_kd = pm->kd(uv);
  const Colour &phong_ks = pm->ks(uv);
  const double phong_p = pm->shininess(uv);
  const Point3D phong_P = eye + t * (ray - eye);
  Vector3D phong_v = eye - phong_P;

  phong_v.normalize();

  // Ambient term.
  Colour rv = ctx.ambient * phong_kd;

  for(auto &light : ctx.lights)
  {
    Vector3D phong_ell = light->position - phong_P;
    const double dist = phong_ell.normalize();

    if(raytrace_within(ctx, phong_P, phong_P + phong_ell, 0.001, dist))
      // Skip this light. There's an obstacle => shadow.
      continue;

    Colour phong_ro = phong_kd;
    Vector3D phong_r = 2 * phong_ell.dot(phong_n) * phong_n - phong_ell;
    phong_r.normalize();
    const double refl = phong_r.dot(phong_v);

    if(refl > 0)
      phong_ro = phong_ro + phong_ks * (pow(refl, phong_p) * phong_n.dot(phong_ell));

    const double *falloff = light->falloff;
    const double c0 = falloff[0], c1 = falloff[1], c2 = falloff[2];
    const double attenuation = c0 + c1 * dist + c2 * dist * dist;
    const Colour terms = phong_ro * (phong_ell.dot(phong_n) / attenuation);

    rv = rv + light->colour * terms;
  }

  return rv;
}

// Does the lighting calculation for a ray. Either does phong lighting or
// generates a background pixel (if we haven't hit anything).
Colour lighting(const RTContext &ctx, const Point3D &ray,
                const Point3D &projected)
{
  const double tlo = 0.1;
  const FlatGeo *g;
  Vector3D normal;
  Point2D uv;
  Vector3D u, v;

  // Raytrace to find the hit with minimal t-value.
  const double t = raytrace_min(ctx, ctx.eye, ray, tlo, &g, normal, uv, u, v);

  if(t < numeric_limits<double>::max())
    // We've got a hit.
    return phong(ctx, ray, t, *g, normal, uv, u, v);
  else
    return ray_pupil_background(ctx, ray, projected);
}

void a4_render(// What to render
               SceneNode* root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               const Point3D& eye, const Vector3D& view,
               const Vector3D& up, double fov_deg,
               // Lighting parameters
               const Colour& ambient,
               const std::list<Light*>& lights
               )
{
  FlatList geometry;
  root->flatten(geometry);

  Image img(width, height, 3);

  Matrix4x4 viewport2world;
  RTContext ctx(geometry, eye, view, up, ambient, lights);
  ctx.width = width; ctx.height = height;

  // First build our inverse-projection matrix. All we're doing is changing the
  // range from [0, 16) to [-tan(fov/2), tan(fov/2).
  {
    const double aspect = width/(double)max(height, 1);
    const double fov_x = M_PI / 180 * fov_deg;
    const double fov_y = fov_x / aspect;
    ctx.fov_x = fov_x;
    ctx.fov_y = fov_y;

    Matrix4x4 invproj = Matrix4x4::scale(Vector3D(2./width, 2./height, 1));
    invproj = Matrix4x4::translate(Vector3D(-1, -1, 0)) * invproj;
    invproj = Matrix4x4::scale(Vector3D(tan(fov_x/2), tan(fov_y/2), 1)) * invproj;
    viewport2world = invproj;
  }

  // Next create the world to view matrix.
  {
    // Change of coordinates from world to view.
    Vector3D y(up);
    Vector3D z(view);
    Vector3D x(z.cross(y));
    x.normalize(); y.normalize(); z.normalize();

    Matrix4x4 world2view(x, y, z);

    // Do the eye translation. This has to come before the change of
    // coordinates.
    world2view = world2view * Matrix4x4::translate(Point3D() - eye);

    // Invert it, since we want to go from view to world.
    viewport2world = world2view.invert() * viewport2world;
  }

  const int aa_grid = GETOPT(aa) ? GETOPT(aa_grid) : 1;
  const double jitter = GETOPT(aa_jitter) / (4 * aa_grid);
  vector<Vector3D> offs(aa_grid * aa_grid);
  for(int i = 0; i < aa_grid; i++)
  {
    for(int j = 0; j < aa_grid; j++)
    {
      offs[i * aa_grid + j] = (1. / aa_grid) * Vector3D(0.5 + i, 0.5 + j, 0);
    }
  }

  if(aa_grid > 2)
  {
    auto swap = [&offs](int x, int y)
    {
      const Vector3D tmp = offs[x];
      offs[x] = offs[y];
      offs[y] = tmp;
    };
    swap(1, (aa_grid - 1) * aa_grid + 0);
    swap(2, (aa_grid - 1) * aa_grid + aa_grid - 1);
    swap(3, 0 + aa_grid - 1);
  }

  auto do_lighting = [&ctx, jitter, &viewport2world]
    (const Point3D &pt)
  {
    const Vector3D vjitter(jitter * (-0.5 + (1 + rand() % 98) * 0.01),
                           jitter * (-0.5 + (1 + rand() % 98) * 0.01), 0);
    const Point3D projected = pt + vjitter;
    const Point3D ray = viewport2world * projected;
    return lighting(ctx, ray, projected);
  };

  auto trace_row = [&ctx, &do_lighting, aa_grid, &offs, &img]
    (int ixrow)
  {
    const double y = ctx.height - (ixrow + 1);
    for(int ixcol = 0; ixcol < ctx.width; ixcol++)
    {
      const double x = ixcol;
      const Point3D centre(x, y, 1);
      Colour c(0);

      if(aa_grid <= 2)
      {
	for(const Vector3D &off : offs)
	{
          c = c + do_lighting(centre + off) * (1./offs.size());
	}
      }
      else
      {
	Colour first_four[4] = {0, 0, 0, 0};

	for(int i = 0; i < 4; i++)
	{
	  auto &off = offs[i];
	  const Colour sample = do_lighting(centre + off);
	  first_four[i] = sample;
	  c = c + sample * (1./offs.size());
	}

	double delta = 0;
        for(int i = 0; i < NUMELMS(first_four); i++)
        {
          for(int j = i + 1; j < NUMELMS(first_four); j++)
          {
            const Colour &c1 = first_four[i], &c2 = first_four[j];
            Colour diff(c1.R() - c2.R(), c1.G() - c2.G(), c1.B() - c2.B());
            diff = diff * diff;
            delta += diff.R() + diff.G() + diff.B();
          }
        }
		   
	if(delta > GETOPT(aa_threshold))
	{
          if(GETOPT(draw_aa))
          {
            c = Colour(1, 0, 0);
          }
          else
          {
            for(int i = 4; i < offs.size(); i++)
            {
              auto &off = offs[i];
              c = c + do_lighting(centre + off) * (1./offs.size());
            }
          }
	}
	else
	{
	  c = c * (offs.size()/4.);
	}
      }

      img(ixcol, ixrow, 0) = c.R();
      img(ixcol, ixrow, 1) = c.G();
      img(ixcol, ixrow, 2) = c.B();
    }
  };

  vector<int> count;
  count.resize(ctx.height);
  for(int ix = 0; ix < count.size(); ix++)
    count[ix] = ix;

  auto it = count.begin();
  auto pf = parfor<decltype(it), int>(it, count.end(), trace_row);

  pf.go(GETOPT(threads), true, [height](int i)
      {
	cout << "Done " << height - i << "/" << height << " rows..." << endl;
      });

  cout << "Done!" << endl;

  img.savePng(filename);
}

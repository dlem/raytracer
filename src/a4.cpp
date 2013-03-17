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
#include "textures.hpp"
#include "lightingmodel.hpp"
#include "timer.hpp"
#include "stats.hpp"
#include "photonmap.hpp"

using namespace std;

void a4_render(// What to render
               SceneNode* root,
               // Where to output the image
               const std::string& filename,
               // Image size
               int width, int height,
               // Viewing parameters
               const Point3D& eye, const Vector3D& _view,
               const Vector3D& _up, double fov_deg,
               // Lighting parameters
               const Colour& ambient,
               const std::list<Light*>& lights
               )
{
  Vector3D up(_up);
  Vector3D view(_view);
  up.normalize();
  view.normalize();
  const Vector3D right(view.cross(up));

  const double aspect = width/(double)max(height, 1);
  const double fov_x = M_PI / 180 * fov_deg;
  const double fov_y = fov_x / aspect;

  FlatList geometry;
  root->flatten(geometry);

#if 0
  auto ray_pupil_background = [fov_x, fov_y, &right, &up, &view](const Point3D &src, const Point3D &dst)
  {
    const Colour darker(1., 0.29, 0);
    const Colour lighter(1., 0.56, 0);
    const int arcs = 28;
    const double pupil_proportion = 0.1;
    const double ray_proportion = 0.5;

    Vector3D ray = dst - src;
    ray.normalize();
    
    const double dtheta = acos(ray.dot(view));
    if(dtheta <= pupil_proportion * fov_x)
      return lighter;
    else if(dtheta > ray_proportion * fov_x)
      return darker;

    const double x = ray.dot(right);
    const double y = ray.dot(up);
    double theta = atan2(y, x);
    theta /= 2 * M_PI / arcs;
    theta -= 0.5;

    return (int)abs(floor(theta)) % 2 >= 1 ? lighter : darker;
  };
#endif

  RayTracer rt(geometry, [](const Point3D &, const Vector3D &) { return Colour(0); });

  // Build the caustic photon map.
  CausticMap caustic_map;
  if(GETOPT(use_caustic_map))
    caustic_map.build(rt, lights);

  GIPhotonMap *gimap = 0;
  if(GETOPT(use_gi_map) || GETOPT(draw_gi_map) || GETOPT(draw_gi_only))
  {
    gimap = new GIPhotonMap;
    gimap->build(rt, lights);
  }

  LightingModel *model;
  
  if(GETOPT(draw_caustics_only))
  {
    model = new PhotonsOnlyModel(caustic_map);
  }
  else if(GETOPT(draw_caustic_map))
  {
    model = new PhotonDrawModel(caustic_map);
  }
  else if(GETOPT(draw_gi_only))
  {
    model = new PhotonsOnlyModel(*gimap);
  }
  else if(GETOPT(draw_gi_map))
  {
    model = new PhotonDrawModel(*gimap);
  }
  else
  {
    model = new PhongModel(ambient, lights, caustic_map, gimap);
  }

  Image img(width, height, 3);

  Matrix4x4 viewport2world;

  // First build our inverse-projection matrix. All we're doing is changing the
  // range from [0, 16) to [-tan(fov/2), tan(fov/2).
  {

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

  auto do_lighting = [model, &eye, &rt, jitter, &viewport2world]
    (const Point3D &pt)
  {
    const Vector3D vjitter(jitter * (-0.5 + (1 + rand() % 98) * 0.01),
                           jitter * (-0.5 + (1 + rand() % 98) * 0.01), 0);
    const Point3D projected = pt + vjitter;
    const Point3D dst = viewport2world * projected;
    Vector3D ray = dst - eye;
    ray.normalize();
    return rt.raytrace_recursive(*model, eye, ray);
  };

  auto trace_row = [width, height, &do_lighting, aa_grid, &offs, &img]
    (int ixrow)
  {
    const double y = height - (ixrow + 1);
    for(int ixcol = 0; ixcol < width; ixcol++)
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
  count.resize(height);
  for(int ix = 0; ix < count.size(); ix++)
    count[ix] = ix;

  auto it = count.begin();
  auto pf = parfor<decltype(it), int>(it, count.end(), trace_row);

  {
    SCOPED_TIMER("rendering");
    pf.go(GETOPT(threads), true, [height](int i)
	{
	  outs() << "Done " << height - i << "/" << height << " rows..." << endl;
	});
  }

  outs() << "Done!" << endl;

  img.savePng(filename);

  if(GETOPT(timing))
    Timer::dump_timings();

  if(GETOPT(stats))
    dump_stats();
}

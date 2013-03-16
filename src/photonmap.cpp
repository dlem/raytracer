#include "photonmap.hpp"
#include <vector>
#include <iostream>
#include "light.hpp"
#include "cmdopts.hpp"
#include "timer.hpp"
#include "stats.hpp"

using namespace std;

class ProjectionMap
{
public:
  ProjectionMap(int granularity)
    : m_map(2 * granularity * granularity, false)
    , m_granularity(granularity)
  {}

  void build(RayTracer &rt, const Point3D &centre,
	     std::function<bool(const FlatGeo &, const Point2D &)> pred)
  {
    const int philimit = m_granularity;
    const int thetalimit = 2 * m_granularity;
    for(int i = 0; i < thetalimit; i++)
    {
      for(int j = 0; j < philimit; j++)
      {
	const double phi = (j + 0.5) * M_PI / philimit;
	const double theta = (i + 0.5) * 2 * M_PI / thetalimit;
	const double sinphi = sin(phi);
	const double cosphi = sqrt(1 - sinphi * sinphi);
	const double sintheta = sin(theta);
	const double costheta = sqrt(1 - sintheta * sintheta);
	Vector3D ray(costheta * sinphi, cosphi, - sintheta * sinphi);

	const double tlo = 0.1;
	const FlatGeo *g;
	Vector3D n, u, v;
	Point2D uv;
	
	if(rt.raytrace_min(centre, ray, tlo, &g, n, uv, u, v) <
	    numeric_limits<double>::max())
	{
	  if(pred(*g, uv))
	  {
	    const int j1 = (j + 1) % philimit, j2 = (j + philimit - 1) % philimit;
	    const int i1 = (i + 1) % thetalimit, i2 = (i + thetalimit - 1) % thetalimit;
	    get(j, i) = true;
	    get(j, i1) = true;
	    get(j, i2) = true;
	    get(j1, i) = true;
	    get(j1, i1) = true;
	    get(j1, i2) = true;
	    get(j2, i) = true;
	    get(j2, i1) = true;
	    get(j2, i2) = true;
	  }
	}
      }
    }
  }

  _Bit_reference operator()(double phi, double theta)
  {
    const int deg_phi = clamp((int)(phi / M_PI * m_granularity), 0, m_granularity - 1);
    const int deg_theta = clamp((int)(theta / M_PI * 2 * m_granularity), 0, 2 * m_granularity - 1);
    return get(deg_phi, deg_theta);
  }

  bool operator()(double phi, double theta) const
  {
    return (*const_cast<ProjectionMap *>(this))(phi, theta);
  }

  bool operator()(const Vector3D &v)
  {
    const double theta = atan2(v[1], v[0]);
    const double phi = atan2(sqrt(v[0]*v[0]+v[1]*v[1]), -v[2]);
    return (*this)(phi, theta);
  }

  double proportion()
  {
    int sum = 0;
    for(int i = 0; i < m_map.size(); i++)
      sum += m_map[i];
    return sum / (double)m_map.size();
  }

private:
  _Bit_reference get(int deg_phi, int deg_theta) { return m_map[deg_phi + deg_theta * m_granularity]; }
  vector<bool> m_map;
  const int m_granularity;
};

void PhotonMap::build(RayTracer &rt, const list<Light *> &lights)
{
  for(auto &light : lights)
    build(rt, *light);
}

void CausticMap::build(RayTracer &rt, const Light &light)
{
  const int nphotons = GETOPT(caustic_num_photons);

  ProjectionMap pm(GETOPT(caustic_pm_gran));

  {
    SCOPED_TIMER("build caustic projection map");
    pm.build(rt, light.position, [](const FlatGeo &geo, const Point2D &uv)
    { return geo.mat->ks(uv).Y() > 0.2; });
  }


  const double falloff = 1; // light.falloff[2];
  const double dist = 1000;
  const Colour total_energy = light.colour * falloff * dist * dist;
  const double proportion = pm.proportion();
  const Colour photon_energy = pm.proportion() * (1./nphotons) * total_energy;

  add_stat("occupied fraction of projection map", proportion);

  {
  SCOPED_TIMER("shoot caustic photons");
  for(int i = 0; i < nphotons; i++)
  {
    Vector3D ray;
    for(;;)
    {
      ray[0] = rand() * 1. / RAND_MAX;
      ray[1] = rand() * 1. / RAND_MAX;
      ray[2] = rand() * 1. / RAND_MAX;

      if(ray.length() > 1)
	// Keeps the distribution uniform, I think.
	continue;

      if(!pm(ray))
	continue;

      break;
    }

    bool seen_specular = false;
    rt.raytrace_russian(light.position, ray, photon_energy, [&seen_specular, this]
	(const Point3D &p, const Vector3D &incident, const Colour &cdiffuse, double *prs)
	{
	  if(seen_specular && prs[RT_DIFFUSE] > 0.001)
	    m_photons.push_back(Photon(p, cdiffuse, incident));
	  const double pr = rand() * 1. / RAND_MAX;
	  if(pr > prs[RT_DIFFUSE] && pr <= prs[RT_SPECULAR])
	  {
	    seen_specular = true;
	    return RT_SPECULAR;
	  }
	  else
	    return RT_ABSORB;
	});
  }
  }

  vector<Photon *> ptrs(m_photons.size());
  for(int i = 0; i < m_photons.size(); i++)
    ptrs[i] = &m_photons[i];

  {
    SCOPED_TIMER("build caustic photon map kd-tree");
    m_map.build(ptrs.begin(), ptrs.end());
  }
}

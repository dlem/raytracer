#include "photonmap.hpp"
#include <vector>
#include <iostream>
#include "light.hpp"

using namespace std;

class ProjectionMap
{
public:
  ProjectionMap(RayTracer &rt, const Point3D &centre,
		std::function<bool(const FlatGeo &, const Point2D &)> pred)
    : m_map(360*180, false)
  {
    for(int i = 0; i < 360; i++)
    {
      for(int j = 0; j < 180; j++)
      {
	const double phi = (j + 0.5) * M_PI / 180;
	const double theta = (i + 0.5) * M_PI / 360;
	const double sinphi = sin(phi);
	const double cosphi = cos(phi);
	Vector3D ray(cosphi, cos(theta) * sinphi, sin(theta) * sinphi);

	const double tlo = 0.1;
	const FlatGeo *g;
	Vector3D n, u, v;
	Point2D uv;
	
	if(rt.raytrace_min(centre, ray, tlo, &g, n, uv, u, v) <
	    numeric_limits<double>::max())
	{
	  if(pred(*g, uv))
	  {
	    const int j1 = (j + 1) % 180, j2 = (j + 180 - 1) % 180;
	    const int i1 = (i + 1) % 360, i2 = (i + 360 - 1) % 360;
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

  bool &operator()(double phi, double theta)
  {
    // Give it per-degree precision (in the middle) for now, I guess.
    const int deg_phi = clamp((int)(phi / M_PI * 180), 0, 179);
    const int deg_theta = clamp((int)(theta / M_PI * 180), 0, 359);
    return get(deg_phi, deg_theta);
  }

  bool operator()(double phi, double theta) const
  {
    return *const_cast<ProjectionMap *>(this)(phi, theta);
  }

  bool operator()(const Vector3D &v)
  {
    const double theta = atan2(v[1], v[0]);
    const double phi = atan2(sqrt(v[0]*v[0]+v[1]*v[1]), v[2]);
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
  bool &get(int deg_phi, int deg_theta) { return m_map[deg_phi + deg_theta * 180]; }
  vector<bool> m_map;
};

void PhotonMap::build(RayTracer &rt, const list<Light *> &lights, int nphotons)
{
  for(auto &light : lights)
    build(rt, light, nphotons);
}

void CausticMap::build(RayTracer &rt, const Light &light, int nphotons)
{
  ProjectionMap pm(rt, light.position, [](const FlatGeo &geo, const Point2D &uv)
      { return geo.mat->ks(uv).Y() > 0.2; });

  const double falloff = 1; // light.falloff[2];
  const double dist = 1000;
  const Colour total_energy = light.colour * falloff * dist * dist;
  const Colour photon_energy = pm.proportion() * (1./nphotons) * total_energy;

  for(int i = 0; i < nphotons; i++)
  {
    Vector3D ray;
    for(;;)
    {
      ray[0] = rand() * 1. / RAND_MAX;
      ray[1] = rand() * 1. / RAND_MAX;
      ray[2] = rand() * 1. / RAND_MAX;

      if(ray.length() > 1)
	continue;

      if(!pm(ray))
	continue;

      break;
    }

    bool seen_specular = false;
    rt.raytrace_russian(light.position, ray, photon_energy, [&seen_specular]
	(const Point3D &p, const Vector3D &incident, const Colour &photon, double *prs)
	{
	  if(seen_specular && prs[RT_DIFFUSE] > 0.001)
	    m_photons.push_back(Photon(position, colour, incident));
	  if(prs[specular]
	  const double pr = rand() * 1. / RAND_MAX;
	  if(pr > prs[RT_DIFFUSE] && pr <= prs[RT_SPECULAR])
	    return RT_SPECULAR;
	  else
	    return RT_ABSORB;
	});
  }
}

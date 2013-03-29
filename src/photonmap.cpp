#include "photonmap.hpp"
#include <vector>
#include <iostream>
#include "light.hpp"
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

  ProjectionMap(const ProjectionMap &other)
    : m_map(other.m_map)
    , m_granularity(other.m_granularity)
  {
    assert(GETOPT(draw_caustic_pm));
  }

  double build(RayTracer &rt, const Point3D &centre,
	     const std::function<bool(const HitInfo &hi)> &pred)
  {
    const int philimit = m_granularity;
    const int thetalimit = 2 * m_granularity;
    for(int i = 0; i < thetalimit; i++)
    {
      for(int j = 0; j < philimit; j++)
      {
	double phi;
	double theta;
	iitopt(j, i, phi, theta);
	Vector3D ray;
	pttov(phi, theta, ray);

	HitInfo hi;
	if(rt.raytrace_min(centre, ray, RT_EPSILON, hi))
	{
	  if(pred(hi))
	  {
	    const int is[] = {i, (i + 1) % thetalimit, (i + thetalimit - 1) % thetalimit};
	    const int js[] = {j, j + 1, j - 1};
	    for(auto ixj : js)
	    {
	      if(!inrange(ixj, 0, philimit - 1))
		continue;

	      for(auto ixi : is)
		get(ixj, ixi) = true;
	    }
	  }
	}
      }
    }

    const double r = 1;
    const double total_area = 4 * M_PI * r * r;
    double sum = 0;
    for(int i = 0; i < thetalimit; i++)
    {
      double s1 = 0;
      for(int j = 0; j < philimit; j++)
      {
	if(get(j, i))
	{
	  // Surface element of a sphere: r * r * dtheta * dphi * sin(phi)
	  double phi, theta;
	  iitopt(j, i, phi, theta);
	  s1 += sin(phi) * 2 * M_PI * M_PI / (double)(thetalimit * philimit);
	}
      }
      sum += s1;
    }

    const double proportion = sum / total_area;
    return proportion;
  }

  _Bit_reference operator()(double phi, double theta)
  {
    int iphi, itheta;
    pttoii(phi, theta, iphi, itheta);
    return get(iphi, itheta);
  }

  bool operator()(double phi, double theta) const
  {
    return (*const_cast<ProjectionMap *>(this))(phi, theta);
  }

  bool operator()(const Vector3D &v)
  {
    double phi, theta;
    vtopt(v, phi, theta);
    return (*this)(phi, theta);
  }

private:
  inline void vtopt(const Vector3D &v, double &phi, double &theta)
  {
    phi = acos(v[1]);
    theta = atan2(-v[2], v[0]);
  }
  inline void pttov(double phi, double theta, Vector3D &v)
  {
    const double sinphi = sin(phi);
    const double cosphi = cos(phi);
    const double sintheta = sin(theta);
    const double costheta = cos(theta);
    v = Vector3D(costheta * sinphi, cosphi, - sintheta * sinphi);
  }
  inline void pttoii(double phi, double theta, int &iphi, int &itheta)
  {
    // phi should range from 0 to pi. Theta should range from -pi to pi.
    const double theta_lo = -M_PI;
    const double theta_hi = M_PI;
    const double phi_lo = 0;
    const double phi_hi = M_PI;
    const int itheta_hi = 2 * m_granularity;
    const int iphi_hi = m_granularity;
    assert(inrange(phi, phi_lo - 0.1, phi_hi + 0.1));
    assert(inrange(theta, theta_lo - 0.1, theta_hi + 0.1));
    phi = iphi_hi * (phi - phi_lo) / (phi_hi - phi_lo);
    theta = itheta_hi * (theta - theta_lo) / (theta_hi - theta_lo);
    itheta = clamp((int)theta, 0, itheta_hi - 1);
    iphi = clamp((int)phi, 0, iphi_hi - 1);
  }
  inline void iitopt(int iphi, int itheta, double &phi, double &theta)
  {
    const double theta_lo = -M_PI;
    const double theta_hi = M_PI;
    const double phi_lo = 0;
    const double phi_hi = M_PI;
    const int itheta_hi = 2 * m_granularity;
    const int iphi_hi = m_granularity;
    assert(inrange(iphi, 0, iphi_hi - 1));
    assert(inrange(itheta, 0, itheta_hi - 1));
    phi = (iphi + 0.5) / iphi_hi * (phi_hi - phi_lo) + phi_lo;
    theta = (itheta + 0.5) / itheta_hi * (theta_hi - theta_lo) + theta_lo;
  }

  _Bit_reference get(int deg_phi, int deg_theta)
  {
    const int idx = deg_phi + deg_theta * m_granularity;
    return m_map[idx];
  }
  vector<bool> m_map;
  const int m_granularity;
};

void PhotonMap::build(RayTracer &rt, const list<Light *> &lights)
{
  for(auto &light : lights)
  {
    if(light->falloff[2] <= 0)
      errs() << "Warning: light has no r^2 falloff term -- photon map won't work properly with it.";

    // This is a spherical light. We want it to shoot one full-energy photon per
    // unit area when the expanding light sphere has a radius such that the
    // light's falloff coefficient is 1. Hence energy is 4PIr^2, where
    // r is given by the falloff coefficient such that f r^2 = 1, ie. r =
    // sqrt(1/f).
    const double energy_fudge = 50;
    const Colour total_energy = 4 * M_PI * sqr(GETOPT(unit_distance)) * energy_fudge * light->colour;

    build_light(rt, *light, total_energy);
  }

  vector<Photon *> ptrs(m_photons.size());
  for(int i = 0; i < m_photons.size(); i++)
    ptrs[i] = &m_photons[i];

  {
    SCOPED_TIMER("build photon map kd-tree");
    m_map.build(ptrs.begin(), ptrs.end());
  }
}

ProjectionMap *s_caustic_pm = 0;
Point3D s_caustic_pm_centre;

Colour PhotonMap::query_radiance(const Point3D &pt, const Vector3D &outgoing)
{
  KDTree<Photon>::TPQueue nl;
  m_map.find_nnn(pt, GETOPT(caustic_num_neighbours), nl, 0.15);
  double maxdist = 0;
  Colour intensity(0);

  while(!nl.empty())
  {
    const KDTree<Photon>::PQNode &node = nl.top();
    const Photon &ph = *static_cast<Photon *>(node.node);
    maxdist = max(maxdist, node.dist);
    nl.pop();
    double fr = outgoing.dot(ph.outgoing);
    fr = fr < 0 ? 0 : 1;
    //const double wp = 1 - 0.25 * node.dist / maxdist;
    intensity += max(fr, 0.) * ph.colour;
  }

  // Do an area average.
  // 100 works here for caustics, 1 works for GI...
  return intensity * (1 / (M_PI * maxdist * maxdist));
}

Colour PhotonMap::query_photon(const Point3D &pt, Vector3D &pos_rel)
{
  Photon *p = m_map.find_nn(pt, 0.15);
  if(p)
  {
    pos_rel = p->pt - pt;
    return p->colour;
  }
  else
  {
    const double dmax = numeric_limits<double>::max();
    pos_rel = Vector3D(dmax, dmax, dmax);
    return Colour(0);
  }
}

void CausticMap::build_light(RayTracer &rt, const Light &light, const Colour &energy)
{
  ProjectionMap pm(GETOPT(caustic_pm_gran));
  double proportion;

  {
    SCOPED_TIMER("build caustic projection map");
    proportion = pm.build(rt, light.position, [](const HitInfo &hi)
    {
      const Colour ks = hi.primary->mat->ks(hi.uv);
      const bool rv = ks.Y() > 0.2;
      return rv;
    });
  }

  const int nphotons = (int)(proportion * GETOPT(caustic_num_photons));
  const Colour photon_energy = proportion * (1./nphotons) * energy;

  add_stat("occupied fraction of projection map", proportion);
  add_stat("number of caustic photons being shot", nphotons);

  if(proportion <= 0)
    return;

  {
  ProgressTimer timer("shoot caustic photons", nphotons);
  for(int i = 0; i < nphotons; i++)
  {
    Vector3D ray;
    for(;;)
    {
      ray[0] = -1 + rand() * 2. / RAND_MAX;
      ray[1] = -1 + rand() * 2. / RAND_MAX;
      ray[2] = -1 + rand() * 2. / RAND_MAX;

      if(ray.length() > 1)
	// Keeps the distribution uniform, I think.
	continue;

      if(!pm(ray))
	continue;

      break;
    }

    ray.normalize();
    bool seen_specular = false;
    rt.raytrace_russian(light.position, ray, photon_energy, [&seen_specular, this]
	(const Point3D &p, const Vector3D &outgoing, const Colour &cdiffuse, double *prs)
	{
	  if(seen_specular && prs[RT_DIFFUSE] > 0)
	    this->m_photons.push_back(Photon(p, cdiffuse, outgoing));
	  const double pr = rand() / (double)RAND_MAX;
	  if(pr > prs[RT_DIFFUSE] && pr <= prs[RT_SPECULAR])
	  {
	    seen_specular = true;
	    return RT_SPECULAR;
	  }
	  else
	    return RT_ABSORB;
	});
    timer.increment();
  }
  }

  add_stat("caustic photon count", m_photons.size());

  if(GETOPT(draw_caustic_pm) && !s_caustic_pm)
  {
    s_caustic_pm_centre = light.position;
    s_caustic_pm = new ProjectionMap(pm);
  }
}

bool CausticMap::test_pm(RayTracer &rt, const Point3D &pt, const Vector3D &normal)
{
  assert(s_caustic_pm);

  Vector3D dir = pt - s_caustic_pm_centre;
  const double distance = dir.normalize();

  if(!(*s_caustic_pm)(dir))
    return false;

  if(rt.raytrace_within(pt, -dir, 0.1, distance))
    return false;

  return true;
}

void GIPhotonMap::build_light(RayTracer &rt, const Light &light, const Colour &energy)
{
  const int nphotons = GETOPT(gi_num_photons);
  const Colour photon_energy = energy * (1./nphotons);

  {
    ProgressTimer timer("shoot gi photons", nphotons);
    for(int i = 0; i < nphotons; i++)
    {
      Vector3D ray;
      do
      {
	ray[0] = -1 + rand() * 2. / RAND_MAX;
	ray[1] = -1 + rand() * 2. / RAND_MAX;
	ray[2] = -1 + rand() * 2. / RAND_MAX;
      }
      while(ray.length() > 1);
      ray.normalize();

      int depth = 0;
      rt.raytrace_russian(light.position, ray, photon_energy, [&depth, this]
	  (const Point3D &p, const Vector3D &outgoing, const Colour &cdiffuse, double *prs)
	  {
	    if(prs[RT_DIFFUSE] > 0 && depth > 0)
	      this->m_photons.push_back(Photon(p, cdiffuse, outgoing));
	    depth += 1;

	    const double pr = rand() / (double)RAND_MAX;
	    for(int i = 0; i < RT_ACTION_COUNT; i++)
	      if(pr <= prs[i])
		return (RT_ACTION)i;
	    return RT_ABSORB;
	  });
      timer.increment();
    }
  }

  add_stat("gi photon count", m_photons.size());
}

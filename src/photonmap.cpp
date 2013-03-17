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

  ProjectionMap(const ProjectionMap &other)
    : m_map(other.m_map)
    , m_granularity(other.m_granularity)
  {
    assert(GETOPT(draw_caustic_pm));
  }

  double build(RayTracer &rt, const Point3D &centre,
	     const std::function<bool(const FlatGeo &, const Point2D &)> &pred)
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

	const double tlo = 0.1;
	const FlatGeo *g;
	Vector3D n, u, v;
	Point2D uv;
	
	if(rt.raytrace_min(centre, ray, tlo, &g, n, uv, u, v) <
	    numeric_limits<double>::max())
	{
	  if(pred(*g, uv))
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
    build_light(rt, *light);
}

ProjectionMap *s_caustic_pm = 0;
Point3D s_caustic_pm_centre;

Colour PhotonMap::query_radiance(const Point3D &pt, const Vector3D &outgoing)
{
  KDTree<Photon>::NeighbourList nl;
  m_map.find_nnn(pt, 4, nl);
  double maxdist = 0;
  Colour intensity(0);

  for(auto &ph : nl)
  {
    maxdist = max(maxdist, (ph->pt - pt).length());
    const double fr = outgoing.dot(ph->outgoing);
    intensity += fr * ph->colour;
    // Fr = BDRF = dot product between outgoing and 
  }

  // Do an area average.
  return intensity * (1 / (M_PI * maxdist * maxdist));
}

Colour PhotonMap::query_photon(const Point3D &pt, Vector3D &pos_rel)
{
  Photon *p = m_map.find_nn(pt);
  assert(p);
  pos_rel = p->pt - pt;
  return p->colour;
}

void CausticMap::build_light(RayTracer &rt, const Light &light)
{
  const int nphotons = GETOPT(caustic_num_photons);

  ProjectionMap pm(GETOPT(caustic_pm_gran));
  double proportion;

  {
    SCOPED_TIMER("build caustic projection map");
    proportion = pm.build(rt, light.position, [](const FlatGeo &geo, const Point2D &uv)
    {
      const Colour ks = geo.mat->ks(uv);
      const bool rv = ks.Y() > 0.2;
      return rv;
    });
  }

  const double falloff = 1; // light.falloff[2];
  const double dist = 1000;
  const Colour total_energy = light.colour * falloff * dist * dist;
  const Colour photon_energy = proportion * (1./nphotons) * total_energy;

  add_stat("occupied fraction of projection map", proportion);

  if(proportion <= 0)
    return;

  {
  SCOPED_TIMER("shoot caustic photons");
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

    bool seen_specular = false;
    rt.raytrace_russian(light.position, ray, photon_energy, [&seen_specular, this]
	(const Point3D &p, const Vector3D &outgoing, const Colour &cdiffuse, double *prs)
	{
	  if(seen_specular && prs[RT_DIFFUSE] > 0.001)
	    this->m_photons.push_back(Photon(p, cdiffuse, outgoing));
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

  cerr << m_photons.size() << endl;
  vector<Photon *> ptrs(m_photons.size());
  for(int i = 0; i < m_photons.size(); i++)
    ptrs[i] = &m_photons[i];

  {
    SCOPED_TIMER("build caustic photon map kd-tree");
    m_map.build(ptrs.begin(), ptrs.end());
  }

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

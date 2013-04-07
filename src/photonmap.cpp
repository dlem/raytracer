#include "photonmap.hpp"
#include <vector>
#include <iostream>
#include "light.hpp"
#include "timer.hpp"
#include "stats.hpp"

using namespace std;

// Used during the photon shooting step to determine whether there are any
// specular objects in a given direction (and thus whether we should bother
// shooting caustic photons). Maps angles on a sphere around a light
// (polar coordinates) to slots in a bit array and sets those slots if there are
// specular objects in the corresponding direction.
class ProjectionMap
{
public:
  // Granularity determines how the size of the area that each slot in the map
  // refers to -- higher granularity means a more precise map (ie, a smaller
  // area of the map will be occupied, but the map itself will be larger).
  //
  // Too small a granularity and bad stuff might start to happen because we're
  // failing to sample entire specular objects.
  ProjectionMap(int granularity)
    : m_map(2 * granularity * granularity, false)
    , m_granularity(granularity)
  {}

  // Hack for the draw_caustic_prm feature.
  ProjectionMap(const ProjectionMap &other)
    : m_map(other.m_map)
    , m_granularity(other.m_granularity)
  {
    assert(GETOPT(draw_caustic_prm));
  }

  // Builds the projection map by shooting a photon for every slot in the
  // projection map and seeing whether that photon hits an object satisfying the
  // provided predicate (in which case the slot is full).
  double build(RayTracer &rt, const Point3D &centre,
	     const std::function<bool(const HitInfo &hi)> &pred)
  {
    if(GETOPT(disable_caustic_prm))
    {
      m_map.assign(true, m_map.size());
      return 1;
    }

    const int philimit = m_granularity;
    const int thetalimit = 2 * m_granularity;
    for(int i = 0; i < thetalimit; i++)
    {
      for(int j = 0; j < philimit; j++)
      {
	// Each iteration corresponds to a single (phi, theta) slot of the
	// projection map.

	double phi;
	double theta;
	iitopt(j, i, phi, theta);
	Vector3D ray;
	pttov(phi, theta, ray);

	HitInfo hi;

	// Try shooting a photon in this directin.
	if(rt.raytrace_min(centre, ray, RT_EPSILON, hi))
	{
	  // Did we hit a desirable object?
	  if(pred(hi))
	  {
	    // Set this slot in the projection map as well as all of the
	    // surrounding slots (we need to set the surrounding slots so that
	    // our map is conservative rather than the opposite -- we don't want
	    // to miss the edges of objects).
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

    // Compute the proportion of the photon map which is occupied. This is
    // required in order to give photons the right amount of energy. But it's an
    // annoying computation since different slots in the map don't have the same
    // area because we're working with a polar coordinate parametrization.
    // Augh.
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

  // Index the map by (phi, theta).
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

  // Index the map with a direction from the centre of the sphere (which is
  // converted to angles).
  bool operator()(const Vector3D &v)
  {
    double phi, theta;
    vtopt(v, phi, theta);
    return (*this)(phi, theta);
  }

private:
  // Helper functions.
  // Converts a vector to (phi, theta).
  inline void vtopt(const Vector3D &v, double &phi, double &theta)
  {
    phi = acos(v[1]);
    theta = atan2(-v[2], v[0]);
  }
  // Converts (phi, theta) to a vector.
  inline void pttov(double phi, double theta, Vector3D &v)
  {
    const double sinphi = sin(phi);
    const double cosphi = cos(phi);
    const double sintheta = sin(theta);
    const double costheta = cos(theta);
    v = Vector3D(costheta * sinphi, cosphi, - sintheta * sinphi);
  }
  // Linearly converts phi in [0, pi] and theta in [-pi, pi] to iphi in [0,
  // granularity] and itheta in [0, 2 * granularity]. Necessary for indexing our
  // map (which is really just an array).
  inline void pttoii(double phi, double theta, int &iphi, int &itheta)
  {
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
  // Performs the opposite convering from the preceding one.
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

  // Index our map with (iphi, itheta).
  _Bit_reference get(int iphi, int itheta)
  {
    const int idx = iphi + itheta * m_granularity;
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

    // It's really important that we get the energy of each light right. The
    // energy of a light determines the intensity of each photon that it shoots
    // and thus the results of the lighting calculations for that light.

    // This is a spherical light. We want it to shoot one full-energy photon per
    // unit area when the expanding light sphere has a radius such that the
    // light's falloff coefficient is 1. Hence energy is 4PIr^2, where
    // r is given by the falloff coefficient such that f r^2 = 1, ie. r =
    // sqrt(1/f).
    const Colour total_energy = 1 / light->falloff[2] * 4 * M_PI * sqr(GETOPT(unit_distance)) * GETOPT(energy_fudge) * light->colour;

    build_light(rt, *light, total_energy);
  }

  vector<Photon *> ptrs(m_photons.size());
  for(int i = 0; i < m_photons.size(); i++)
    ptrs[i] = &m_photons[i];

  {
    ProgressTimer pt("build photon map kd-tree", 1);
    m_map.build(ptrs.begin(), ptrs.end());
  }
}

// Hack to support the draw-caustic-prm feature.
static ProjectionMap *s_caustic_pm = 0;
Point3D s_caustic_pm_centre;

Colour PhotonMap::query_radiance(const Point3D &pt, const Vector3D &outgoing)
{
  KDTree<Photon>::TPQueue nl;
  // To save time in dimly lit areas, only query photons that are close to the
  // point (far away photons won't contribute very much to the lighting).
  const double max_neighbour_dist = 0.15 * GETOPT(unit_distance);
  m_map.find_nnn(pt, num_neighbours(), nl, sqr(max_neighbour_dist));

  double maxdist2 = 0;
  Colour intensity(0);

  // Go through the neighbours we found that add their intensities to our
  // result lighting computation.
  while(!nl.empty())
  {
    const KDTree<Photon>::PQNode &node = nl.top();
    const Photon &ph = *static_cast<Photon *>(node.node);
    maxdist2 = max(maxdist2, node.dist2);
    nl.pop();
    // Ignore photons which were travelling in the opposite direction of the
    // outgoing vector (which is to the viewer).
    double fr = outgoing.dot(ph.outgoing);
    fr = fr < 0 ? 0 : 1;
    //const double wp = 1 - 0.25 * node.dist / maxdist;
    intensity += max(fr, 0.) * ph.colour;
  }

  // Compute the area average of the intensity -- ie, divide our intensity sum
  // by the area taken by the photons we found (assuming they're in a disc).
  return intensity * (1 / (M_PI * maxdist2));
}

Colour PhotonMap::query_photon(const Point3D &pt, Vector3D &pos_rel)
{
  Photon *p = m_map.find_nn(pt, sqr(0.15));
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

  // Modulate the # photons based on the occupied % of the projection map.
  const int nphotons = (int)(proportion * GETOPT(caustic_num_photons));
  const Colour photon_energy = energy * (1/(double)GETOPT(caustic_num_photons));

  add_stat("occupied fraction of projection map", proportion);
  add_stat("number of caustic photons being shot", nphotons);

  if(proportion <= 0)
    return;

  {
  ProgressTimer timer("shoot caustic photons", nphotons);
  for(int i = 0; i < nphotons; i++)
  {
    Vector3D ray;
    // Generate random rays until you get one inside the unit sphere -- keeps
    // the distribution uniform.
    for(;;)
    {
      ray = generate_ray();

      if(ray.length() > 1)
	continue;

      // Only shoot in directions occupied in the photon map. We've already
      // modified our intensity based on this, so it works out.
      if(!pm(ray))
	continue;

      break;
    }

    ray.normalize();
    bool seen_specular = false;

    // Do Russian roulette ray tracing for this photon. Add its current
    // intensity to the map whenever we hit a diffuse surface _if_ it's already
    // been through a caustic interaction. If we ever decide to do a diffuse
    // interaction, just let it get absorbed because it's no longer of interest
    // to us for caustics.
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

  // Part of the hack to support the draw--caustic--prm option.
  if(GETOPT(draw_caustic_prm) && !s_caustic_pm)
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
  // Same as CausticMap::build_light, but doesn't use a projection map and the
  // Russian roulette ray tracing algorithm is a bit different.
  const int nphotons = GETOPT(gi_num_photons);
  const Colour photon_energy = energy * (1./nphotons);

  {
    ProgressTimer timer("shoot gi photons", nphotons);
    for(int i = 0; i < nphotons; i++)
    {
      Vector3D ray;
      do
      {
	ray = generate_ray();
      }
      while(ray.length() > 1);
      ray.normalize();

      bool diffusely_reflected = false;

      // Only store interactions with diffuse surfaces where the photon has
      // already been diffusely reflected at least once (since we don't want
      // to be adding direct illumination).
      rt.raytrace_russian(light.position, ray, photon_energy, [&diffusely_reflected, this]
	  (const Point3D &p, const Vector3D &outgoing, const Colour &cdiffuse, double *prs)
	  {
	    if(prs[RT_DIFFUSE] > 0 && diffusely_reflected)
	      this->m_photons.push_back(Photon(p, cdiffuse, outgoing));

	    const double pr = rand() / (double)RAND_MAX;
	    for(int i = 0; i < RT_ACTION_COUNT; i++)
	      if(pr <= prs[i])
	      {
		if(i == RT_DIFFUSE)
		  diffusely_reflected = true;
		return (RT_ACTION)i;
	      }
	    return RT_ABSORB;
	  });
      timer.increment();
    }
  }

  add_stat("gi photon count", m_photons.size());
}

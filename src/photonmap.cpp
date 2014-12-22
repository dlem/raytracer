#include "photonmap.hpp"
#include <vector>
#include <iostream>
#include "light.hpp"
#include "timer.hpp"
#include "stats.hpp"

using namespace std;

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


void PhotonMap::build_light(RayTracer &rt, const Light &light, const Colour &energy)
{
  const int total_nphotons = num_photons();
  const Colour photon_energy = energy * (1/(double)total_nphotons);
  const unsigned int nsubdivs = light.num_subdivs();

  add_stat("projection map patches", nsubdivs);

  {
    ProgressTimer timer("shoot caustic photons", total_nphotons);
    ThreadLocal<default_random_engine> _rng([]
	{ return new default_random_engine(get_rng_seed(g_worker_thread_num)); });
    ThreadLocal<SurfaceSubdiv> _ss([&light] { return light.subdiv().release(); });
    mutex mtx;

    auto shoot_photons = [this, total_nphotons, &_rng,
			  &timer, &rt, &photon_energy, &_ss]
      (unsigned int next_subdiv)
    {
      auto &rng = _rng.get();
      auto &ss = _ss.get();

      ss.set(next_subdiv);

      const int nphotons = total_nphotons * ss.area();

      if(!patch_predicate(rt, ss))
      {
	timer.increment(nphotons);
	return;
      }

      for(int i = 0; i < nphotons; i++)
      {
	Point3D src;
	Vector3D ray;
	ss.generate_ray(rng, src, ray);
	shoot(rt, src, ray, photon_energy, rng);
	timer.increment();
      }
    };

    Parallelize<void> par;
    for(int i = 0; i < light.num_subdivs(); i++)
      par.add_task([&shoot_photons, i] { shoot_photons(i); });
    par.go();
  }

  add_stat("caustic photon count", m_photons.size());
}

Colour PhotonMap::query_radiance(const Point3D &pt, const Vector3D &outgoing)
{
  KDTree<Photon>::TPQueue nl;
  // To save time in dimly lit areas, only query photons that are close to the
  // point (far away photons won't contribute very much to the lighting).
  const double max_neighbour_dist = 0.15 * GETOPT(unit_distance);
  m_map.find_nnn(pt, num_neighbours(), nl, sqr(max_neighbour_dist));

  double maxdist2 = 0;
  Colour intensity(0);

  if(nl.empty())
    return Colour();

  // Go through the neighbours we found that add their intensities to our
  // result lighting computation.
  while(!nl.empty())
  {
    const KDTree<Photon>::PQNode &node = nl.top();
    const Photon &ph = *static_cast<Photon *>(node.node);
    maxdist2 = max(maxdist2, node.dist2);
    nl.pop();

    // Assume Lambertian model. If we were using another model, we'd want to
    // have the intensity be some function of the outgoing vector.
    intensity += ph.colour;
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

void CausticMap::shoot(RayTracer &rt, const Point3D &src, const Vector3D &ray,
			     Colour energy, default_random_engine &rng)
{
  bool seen_specular = false;

  // Do Russian roulette ray tracing for this photon. Add its current
  // intensity to the map whenever we hit a diffuse surface _if_ it's already
  // been through a caustic interaction. If we ever decide to do a diffuse
  // interaction, just let it get absorbed because it's no longer of interest
  // to us for caustics.
  rt.raytrace_russian(src, ray, energy, rng, [this, &seen_specular]
      (const Point3D &p, const Vector3D &outgoing, const Colour &cdiffuse, RT_ACTION action)
  {
    if(seen_specular && cdiffuse.Y() > 0)
    {
      lock_guard<mutex> lk(m_build_mutex);
      this->m_photons.push_back(Photon(p, cdiffuse, outgoing));
    }
    if(action == RT_TRANSMIT || action == RT_REFLECT)
    {
      seen_specular = true;
      return true;
    }
    return false;
  });
}

bool CausticMap::patch_predicate(RayTracer &rt, const SurfaceSubdiv &ss)
{
  for(int i = 0; i < ss.num_extreme_rays(); i++)
  {
    Point3D src;
    Vector3D ray;
    ss.extreme_ray(i, src, ray);
    HitInfo hi;
    if(rt.raytrace_min(src, ray, 0, hi))
    {
      const Colour ks = hi.primary->mat->ks(hi.uv);
      if(ks.Y() > 0.2)
	return true;
    }
  }

  return false;
}

void GIPhotonMap::shoot(RayTracer &rt, const Point3D &src, const Vector3D &ray,
			Colour energy, default_random_engine &rng)
{
  bool diffusely_reflected = false;

  // Only store interactions with diffuse surfaces where the photon has
  // already been diffusely reflected at least once (since we don't want
  // to be adding direct illumination).
  rt.raytrace_russian(src, ray, energy, rng, [this, &diffusely_reflected]
      (const Point3D &p, const Vector3D &incident, const Colour &cdiffuse, RT_ACTION action)
  {
    if(cdiffuse.Y() > 0 && diffusely_reflected)
    {
      lock_guard<mutex> lk(m_build_mutex);
      m_photons.push_back(Photon(p, cdiffuse, incident));
    }
    if(action == RT_DIFFUSE)
      diffusely_reflected = true;
    return true;
  });
}

#include "subdiv.hpp"
#include "isodecahedron.hpp"

using namespace std;

static inline bool halfspace_contains(const Point3D &pt, const Point3D &hspt, const Vector3D &hsn)
{
  return (pt - hspt).dot(hsn) > 0;
}

static inline double compute_edge_length(unsigned int nrecs)
{
  // Edge length for a 20-sided isodecahedron whose vertices are on the surface
  // of the unit sphere.
  // Similar triangles: 1 / sqrt(1^2 + (phi/2)^2)
  return 0.7774375248211360 / (nrecs + 1);
}

static inline double compute_randlo(unsigned int nrecs)
{
  const double edge_angle = 2 * atan(2. / phi) / (nrecs + 1);
  const double corner_angle = edge_angle / 2 / cos(M_PI / 6);
  const double randlo = (cos(corner_angle) + 1) / 2;
  return randlo;
}

unsigned int SphereSubdiv::num_subdivs(unsigned int nrecs) { return 20 * pow(4, nrecs); }

SphereSubdiv::SphereSubdiv(unsigned int nrecs)
  : m_nrecs(nrecs)
  , m_randlo(compute_randlo(nrecs))
{
  assert(m_randlo >= 0);
}

void SphereSubdiv::set(unsigned int nsubdiv)
{
  auto &face = isodecahedron::faces[nsubdiv % 20];
  auto vertices = isodecahedron::vertices;
  Vector3D v0 = vertices[face.v0], v1 = vertices[face.v1], v2 = vertices[face.v2];
  nsubdiv /= 20;

  for(int i = 0; i < m_nrecs; i++)
  {
    Vector3D v0prime = v0 + v1, v1prime = v1 + v2, v2prime = v0 + v2;
    v0prime.normalize(); v1prime.normalize(); v2prime.normalize();
    switch(nsubdiv % 4)
    {
      case 0: v0 = v0prime; v1 = v1prime; v2 = v2prime; break;
      case 1: v0 = v0prime; v1 = v1prime; break;
      case 2: v0 = v0prime; v2 = v2prime; break;
      case 3: v1 = v1prime; v2 = v2prime; break;
    }

    nsubdiv /= 4;
  }

  m_points[0] = Point3D() + v0;
  m_points[1] = Point3D() + v1;
  m_points[2] = Point3D() + v2;

  Vector3D ximg = v1 - v0;
  ximg.normalize();
  Vector3D yimg = v2 - 0.5 * (v0 + v1);
  yimg.normalize();
  Vector3D zimg(ximg.cross(yimg));

  m_trans = Matrix4x4();
  double *r0 = m_trans[0], *r1 = m_trans[1], *r2 = m_trans[2];
  r0[0] = ximg[0]; r0[1] = yimg[0]; r0[2] = zimg[0];
  r1[0] = ximg[1]; r1[1] = yimg[1]; r1[2] = zimg[1];
  r2[0] = ximg[2]; r2[1] = yimg[2]; r2[2] = zimg[2];
}

Vector3D SphereSubdiv::generate_ray(default_random_engine &rng) const
{
  for(;;)
  {
    const double theta = rng() / (double)rng.max() * 2 * M_PI;
    const double v = m_randlo + rng() * (1 - m_randlo) / rng.max();
    const double phi = acos(2 * v - 1);
    const double sinphi = sin(phi);
    Point3D dst(sinphi * cos(theta), sinphi * sin(theta), cos(phi));

    // Clip the ray to our halfspaces.
    const double edgelen = compute_edge_length(m_nrecs);
    const Vector3D n0(0, 1, 0), n1(1/M_SQRT2, -1/M_SQRT2, 0), n2(-1/M_SQRT2, -1/M_SQRT2, 0);
    const Point3D p0(0, 0, 0), p1(-edgelen / 2, 0, 0), p2(edgelen / 2, 0, 0);

    if(!halfspace_contains(dst, p0, n0) ||
       !halfspace_contains(dst, p1, n1) ||
       !halfspace_contains(dst, p2, n2))
      continue;

    return m_trans * dst - Point3D();
  }
}

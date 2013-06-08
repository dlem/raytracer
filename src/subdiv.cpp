#include "subdiv.hpp"
#include "cmdopts.hpp"
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

SphereSubdiv::SphereSubdiv(const Point3D &center, unsigned int nrecs)
  : m_nrecs(nrecs)
  , m_center(center)
{
}

void SphereSubdiv::set(unsigned int nsubdiv)
{
  // Compute the points for the specified triangle patch (m_points).

  auto &face = isodecahedron::faces[nsubdiv % 20];
  auto vertices = isodecahedron::vertices;
  Vector3D v0 = vertices[face.v0], v1 = vertices[face.v1], v2 = vertices[face.v2];
  v0.normalize(); v1.normalize(); v2.normalize();

  nsubdiv /= 20;

  for(int i = 0; i < m_nrecs; i++)
  {
    Vector3D v0prime = v0 + v1, v1prime = v1 + v2, v2prime = v0 + v2;
    v0prime.normalize(); v1prime.normalize(); v2prime.normalize();
    switch(nsubdiv % 4)
    {
      case 0: v0 = v1prime; v1 = v2prime; v2 = v0prime; break;
      case 1: v0 = v2prime; v1 = v1prime; break;
      case 3: v1 = v0prime; v2 = v2prime; break;
      case 2: v2 = v1prime; v0 = v0prime; break;
      default: assert(0); break;
    }
    nsubdiv /= 4;
  }

  m_points[0] = Point3D() + v0;
  m_points[1] = Point3D() + v1;
  m_points[2] = Point3D() + v2;
  
  // Next: compute a "center" point about which rays are generated (we generate
  // them in a partial hemisphere and then clip then to the actual patch).

  // Ideally, we'd minimize the maximal distance from this to any of the three
  // points. But this is easier.
  Vector3D vcenter = 1 / 3. * (v0 + v1 + v2);
  vcenter.normalize();
  Point3D center = Point3D() + vcenter;
  m_patch_center = center;

  // Next: compute the angle between the center point and the farthest one from
  // it to determine how large a partial hemisphere to generate rays upon.
  
  const double d0 = (center - m_points[0]).length2(),
	       d1 = (center - m_points[1]).length2(),
	       d2 = (center - m_points[2]).length2();

  const Point3D farthest = ((d0 > d1) && (d0 > d2)) ? m_points[0] :
		     (d1 > d2 ? m_points[1] : m_points[2]);

  m_randlo = ((farthest - Point3D()).dot(center - Point3D()) + 1) / 2;

  // Next: compute rotation matrix from the partial hemisphere on top of the
  // sphere to the corresponding partial hemisphere around our center point.

  // We're going to be generating rays in a partial hemisphere around the up
  // direction. I'll have this rotation map from the up direction to the
  // center direction for this patch; that way I can use it to point generated
  // rays in the right direction.
  const Vector3D top(0, 0, 1);
  Vector3D dir = top.cross(vcenter);
  dir.normalize();
  m_rotation = Matrix4x4::rotate(Point3D(), dir, acos(top.dot(vcenter)));

  // Next: set m_plane_points and m_plane_normals so that they define three
  // planes consituting a pyramid corresponding to our circular triangle patch.
  
  auto get_normal_vec = [&center](Point3D &l0, Point3D &l1, Point3D &target)
  {
    const Vector3D ortho1 = l1 - l0;
    const Vector3D ortho2 = l0 - Point3D();
    Vector3D line = target - l0;

    // This is Graham-Schmidt.
    line = line - ortho1.dot(line) * ortho1;
    line.normalize();
    line = line - ortho2.dot(line) * ortho2;
    line.normalize();
    return line;
  };

  m_plane_normals[0] = get_normal_vec(m_points[0], m_points[1], m_points[2]);
  m_plane_normals[1] = get_normal_vec(m_points[1], m_points[2], m_points[0]);
  m_plane_normals[2] = get_normal_vec(m_points[2], m_points[0], m_points[1]);

  assert(halfspace_contains(center, Point3D(), m_plane_normals[0]));
  assert(halfspace_contains(center, Point3D(), m_plane_normals[1]));
  assert(halfspace_contains(center, Point3D(), m_plane_normals[2]));
  
  // http://mathforum.org/library/drmath/view/65316.html
  {
  const double r = 1;
  const double total_area = 4 * M_PI * r * r;
  const double a = r * acos(v0.dot(v1)),
	       b = r * acos(v1.dot(v2)),
	       c = r * acos(v2.dot(v0)),
	       s = 0.5 * (a + b + c);
  const double rhs = sqrt(tan(s / 2) * tan((s - a) / 2) * tan((s - b) / 2) * tan((s - c) / 2));
  const double E = 4 * atan(rhs);
  m_area = r * r * E / total_area;
  }

#if 0
  errs() << "Center: " << m_patch_center << endl;
  errs() << "Points: " << m_points[0] << ", " << m_points[1] << ", " << m_points[2] << endl;
  errs() << "Randlo: " << m_randlo << endl;
  errs() << "Plane normals: " << m_plane_normals[0] << ", " << m_plane_normals[1] << ", " << m_plane_normals[2] << endl;
  errs() << "R: " << m_rotation << endl;
#endif
}

void SphereSubdiv::generate_ray(default_random_engine &rng, Point3D &src, Vector3D &dst) const
{
  for(;;)
  {
    // Generate a point uniformly (wrt area) distributed near the top (z = 1) of
    // the sphere.
    const double theta = 2 * M_PI * rng() / (double)rng.max();
    const double v = m_randlo + rng() * (1 - m_randlo) / rng.max();
    const double phi = acos(2 * v - 1);
    const double sinphi = sin(phi);
    Point3D pdst(sinphi * cos(theta), sinphi * sin(theta), cos(phi));

    // Change it so that the points are distributed about the center point for
    // this patch.
    pdst = m_rotation * pdst;

    // Points are generated in a partial hemisphere, not our target triangular
    // patch (the hemisphere is a superset). Clip the point to the corresponding
    // pyramid.
    if(!halfspace_contains(pdst, Point3D(), m_plane_normals[0]) ||
       !halfspace_contains(pdst, Point3D(), m_plane_normals[1]) ||
       !halfspace_contains(pdst, Point3D(), m_plane_normals[2]))
      continue;

    src = m_center;
    dst = pdst - Point3D();
    assert(normalized(dst));
    return;
  }
}

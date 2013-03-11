/**
 * Code for polygonal mesh objects.
 *
 * Donated code; modified by Daniel Lemmond, dlemmond, 20302247.
**/

#include "mesh.hpp"
#include <iostream>
#include <algorithm>
#include "cmdopts.hpp"

using namespace std;

Mesh::Mesh(const std::vector<Point3D>& verts,
           const std::vector< std::vector<int> >& faces)
  : m_verts(verts)
  , m_faces(faces)
{
  for(int i = 0; i < 3; i++)
  {
    m_mins[i] = numeric_limits<double>::max();
    m_maxes[i] = -m_mins[i];
  }

  for(auto v : verts)
  {
    for(int i = 0; i < 3; i++)
    {
      m_mins[i] = min(m_mins[i], v[i]);
      m_maxes[i] = max(m_maxes[i], v[i]);
    }
  }
}

bool Mesh::intersect(const Point3D &eye, const Point3D &_ray, const IntersectFn &fn) const
{
  if(GETOPT(draw_bv))
    return axis_aligned_box_check(eye, _ray, m_mins, m_maxes, fn);

  if(GETOPT(bv))
    if(axis_aligned_box_check(eye, _ray, m_mins, m_maxes,
          [](double, const Vector3D &) { return false; }))
      return true;

  // The maximum number of hits we'll allow ourselves to record. This bound
  // shouldn't be a problem (as long as it's at least 2) since when we get lots
  // of hits either 1) the polygon's strange/concave, which we assume isn't the
  // case, or 2) we're hitting near a multi-face seam, in which case it doesn't
  // really matter which face we decide we're hitting.
  const Vector3D ray = _ray - eye;
  const Polynomial<1> x = {eye[0], ray[0]};
  const Polynomial<1> y = {eye[1], ray[1]};
  const Polynomial<1> z = {eye[2], ray[2]};

  // We check whether the line intersects with each face. We have to keep going
  // after we find an intersection, as there might be one nearer to the eye.
  for(auto &face : m_faces)
  {
    if(face.size() < 3)
      // Can't figure out a plane with only two vertices.
      continue;

    // For this face, we'll first do some algebra to solve for a t, then we'll
    // see whether the point given by t is actually inside the polygon using a
    // winding algorithm.

    double t;
    Vector3D normal;
    {
      // Compute the normal to the plane. Assumes they're specified CCW.
      const Point3D &p0 = m_verts[face[0]], &p1 = m_verts[face[1]],
		    &p2 = m_verts[face[2]];
      const Vector3D v0 = p0 - p1, v1 = p2 - p1;
      normal = v1.cross(v0);
      // Use the normal and a vertex to compute D; gives us the plane equation.
      const double D = -(p0 - Point3D()).dot(normal);
      // Substitute x, y, z in the plane equation to get an equation in t.
      const Polynomial<1> p = normal[0] * x + normal[1] * y + normal[2] * z + D;
      // Solve it to find t. There should be at most one root.
      double roots[2];
      if(p.solve(roots) == 0)
	continue;
      t = roots[0];
    }

    const Point3D p = eye + t * ray;

    // Is p within the polygon or not? We'll check by computing its cross
    // product with each of the bounding lines; it should be on the same side of
    // all of them.
    int sign = 0;
    for(int i = 0; i < face.size(); i++)
    {
      const Point3D &p0 = m_verts[face[i]];
      const Point3D &p1 = m_verts[face[(i + 1) % face.size()]];
      const Vector3D v0 = p1 - p0;
      const Vector3D v1 = p - p0;
      double cursign = Vector3D(1, 1, 1).dot(v0.cross(v1));
      if(cursign == 0)
	// We'll count this as inside.
	continue;
      cursign /= abs(cursign);
      if(sign == 0)
	sign = cursign;
      else if(sign != (int)cursign)
	goto no_hit;
    }

    if(!fn(t, normal, Point2D(0, 0), Point3D(0, 0, 0), Point3D(0, 0, 0)))
      return false;

no_hit: ;
  }

  return true;
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  std::cerr << "mesh({";
  for (std::vector<Point3D>::const_iterator I = mesh.m_verts.begin(); I != mesh.m_verts.end(); ++I) {
    if (I != mesh.m_verts.begin()) std::cerr << ",\n      ";
    std::cerr << *I;
  }
  std::cerr << "},\n\n     {";
  
  for (std::vector<Mesh::Face>::const_iterator I = mesh.m_faces.begin(); I != mesh.m_faces.end(); ++I) {
    if (I != mesh.m_faces.begin()) std::cerr << ",\n      ";
    std::cerr << "[";
    for (Mesh::Face::const_iterator J = I->begin(); J != I->end(); ++J) {
      if (J != I->begin()) std::cerr << ", ";
      std::cerr << *J;
    }
    std::cerr << "]";
  }
  std::cerr << "});" << std::endl;
  return out;
}

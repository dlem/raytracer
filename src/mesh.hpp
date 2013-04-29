/**
 * Code for polygonal mesh objects.
 *
 * Modified donated code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef CS488_MESH_HPP
#define CS488_MESH_HPP

#include <vector>
#include <iosfwd>
#include "primitive.hpp"
#include "algebra.hpp"

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh(const std::vector<Point3D>& verts,
       const std::vector< std::vector<int> >& faces);

  virtual bool intersect(const Point3D &eye, const Point3D &ray, HitReporter &hr) const;
  virtual void bounding_box(Box &b) const;

  typedef std::vector<int> Face;
  
private:
  std::vector<Point3D> m_verts;
  std::vector<Face> m_faces;
  Point3D m_mins;
  Point3D m_maxes;
  Primitive *m_cube;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};

// This primitive is only here so that I can make sure my Isodecahedron mesh
// (used for spherical subdivision in photon mapping) is correct.
class Isodecahedron : public Mesh { public: Isodecahedron(); };

#endif

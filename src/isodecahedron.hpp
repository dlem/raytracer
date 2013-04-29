#ifndef __ISODECAHEDRON_HPP__
#define __ISODECAHEDRON_HPP__

#include "algebra.hpp"

namespace isodecahedron
{
  extern const Vector3D vertices[12];
  extern const struct faces_t {short v0, v1, v2;} faces[20];
}

#endif

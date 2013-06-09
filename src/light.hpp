/*
 * This module defines light objects.
 *
 * Modified donated code.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/


#ifndef CS488_LIGHT_HPP
#define CS488_LIGHT_HPP

#include <iosfwd>
#include <memory>

#include "algebra.hpp"
#include "subdiv.hpp"

// Represents a simple point light.
class Light
{
public:
  Light();
  virtual ~Light() {}

  virtual std::unique_ptr<SurfaceSubdiv> subdiv() const = 0;
  virtual unsigned int num_subdivs() const = 0;
  
  Colour colour;
  Point3D position;
  double falloff[3];
  double radius;
};

class PointLight : public Light
{
public:
  virtual std::unique_ptr<SurfaceSubdiv> subdiv() const
  { return std::unique_ptr<SurfaceSubdiv>(new SphereSubdiv(position, 8)); }
  virtual unsigned int num_subdivs() const { return SphereSubdiv::num_subdivs(8); }
};

std::ostream& operator<<(std::ostream& out, const Light& l);

#endif

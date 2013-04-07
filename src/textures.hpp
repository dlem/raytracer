/*
 * This module exposes the texture and bumpmap typedefs as well as the 'get'
 * functions to retrieve textures from LUA C++ callbacks.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include "algebra.hpp"

// Maps Point2Ds to something -- colours for textures and Point2Ds for bump maps
// -- and provides a static 'get' function for creating them from a string.
template<typename TMapped>
class UVMapper
{
public:
  virtual ~UVMapper() {}

  // Get the mapped value.
  virtual TMapped operator()(const Point2D &uv) = 0;

  // Creates the texture/bumpmap from a filename or the name of a programmatical
  // one.
  static UVMapper<TMapped> *get(const std::string &name);
};

typedef UVMapper<Colour> Texture;
typedef UVMapper<Point2D> Bumpmap;

#endif

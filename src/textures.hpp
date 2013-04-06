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

class Image;

void gradients(Image &grad, const Image &img);

template<typename TMapped>
class UVMapper
{
public:
  virtual ~UVMapper() {}
  virtual TMapped operator()(const Point2D &uv) = 0;
  static UVMapper<TMapped> *get(const std::string &name);
};

typedef UVMapper<Colour> Texture;
typedef UVMapper<Point2D> Bumpmap;

#endif

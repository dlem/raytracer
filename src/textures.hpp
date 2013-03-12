#ifndef __TEXTURE_HPP__
#define __TEXTURE_HPP__

#include "algebra.hpp"

class Image;

void gradients(Image &grad, const Image &img);

class Texture
{
public:
  virtual ~Texture() {}
  virtual Colour operator()(const Point2D &uv) = 0;

  static Texture *get(const std::string &name);
};

class Bumpmap
{
public:
  virtual ~Bumpmap() {}
  virtual Point2D operator()(const Point2D &uv) = 0;
  
  static Bumpmap *get(const std::string &name);
};

#endif

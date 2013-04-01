#ifndef __TEXDEFS_HPP__
#define __TEXDEFS_HPP__

#include "textures.hpp"

enum REMAPTYPE
{
  REMAP_CUBETOP,
  REMAP_CUBEFRONT,
  REMAP_CUBEBOT,
  REMAP_CYLTOP,
  REMAP_CONETOP,
};

template<typename TMapped>
class UVRemapper : public UVMapper<TMapped>
{
public:
  UVRemapper(UVMapper<TMapped> &tex, const std::string &ty);

  virtual TMapped operator()(const Point2D &uv);

private:
  UVMapper<TMapped> &m_tex;
  REMAPTYPE m_ty;
};

typedef UVRemapper<Point2D> BmRemapper;
typedef UVRemapper<Colour> TexRemapper;

#define DECLARE_BUMPMAP(name) \
  class name : public Bumpmap \
  { public: virtual Point2D operator()(const Point2D &uv); }

#define DECLARE_TEXTURE(name) \
  class name : public Texture \
  { public: virtual Colour operator()(const Point2D &uv); }

DECLARE_BUMPMAP(SineWavesBm);
DECLARE_BUMPMAP(BubblesBm);

class CheckerTexture : public Texture
{
public:
  CheckerTexture(const Colour &c) : m_c(c) {}
  virtual Colour operator()(const Point2D &uv);

private:
  Colour m_c;
};

#endif

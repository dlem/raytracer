#ifndef __TEXDEFS_HPP__
#define __TEXDEFS_HPP__

#include "textures.hpp"

enum REMAPTYPE
{
  REMAP_CUBETOP,
  REMAP_CYLTOP,
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


class SineWavesBm : public Bumpmap
{
public:
  virtual Point2D operator()(const Point2D &uv);
};

class BubblesBm : public Bumpmap
{
public:
  virtual Point2D operator()(const Point2D &uv);
};

#endif

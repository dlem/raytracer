#include "texdefs.hpp"

template<typename TMapped>
TMapped UVRemapper<TMapped>::operator()(const Point2D &uv)
{
  switch(m_ty)
  {
    case REMAP_CUBETOP:
    {
      const double l = 1/4., r = 2/4., b = 1/3., t = 2/3.;
      if(inrange(uv[0], l, r) && inrange(uv[1], b, t))
      {
	const double u = (uv[0] - l) * (r - l);
	const double v = (uv[1] - b) * (t - b);
	return m_tex(Point2D(u, v));
      }
      return TMapped();
    }
    case REMAP_CYLTOP:
    {
      break;
    }
    default:
    {
      assert(false);
    }
  }
  return TMapped();
}

template class UVRemapper<Point2D>;
template class UVRemapper<Colour>;


Point2D SineWavesBm::operator()(const Point2D &uv)
{
#if 0
  const double dist = 0.2;
  const double size = 0.03;
  const double rad = sqrt(sqr(uv[0] - 0.5) + sqr(uv[1] - 0.5));
  const double start = rad - rad % size;
  const double end = start + size;
#endif
  return Point2D();
}

Point2D BubblesBm::operator()(const Point2D &uv)
{
  return Point2D();
}

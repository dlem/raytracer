#include "texdefs.hpp"
#include "cmdopts.hpp"

using namespace std;

static map<const string, REMAPTYPE> s_remaptypes =
{
  { "cubetop", REMAP_CUBETOP },
  { "cyltop", REMAP_CYLTOP },
};

template<typename TMapped>
UVRemapper<TMapped>::UVRemapper(UVMapper<TMapped> &tex, const std::string &ty)
  : m_tex(tex)
{
  auto rmtype = s_remaptypes.find(ty);
  if(rmtype == s_remaptypes.end())
  {
    errs() << "Invalid remap type: '" << ty << "'" << endl;
    errs() << "Valid remap types:" << endl;
    for(auto p : s_remaptypes)
      cerr << p.first << endl;
    cerr << endl << "Aborting" << endl;
    exit(1);
  }
  m_ty = rmtype->second;
}

template<typename TMapped>
TMapped UVRemapper<TMapped>::operator()(const Point2D &uv)
{
  Point2D rv(uv);
  switch(m_ty)
  {
    case REMAP_CUBETOP:
    {
      const double l = 1/4., r = 2/4., b = 1/3., t = 2/3.;
      if(inrange(uv[0], l, r) && inrange(uv[1], b, t))
      {
	const double u = (uv[0] - l) * (r - l);
	const double v = (uv[1] - b) * (t - b);
	rv = Point2D(u, v);
      }
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
  return m_tex(rv);
}

template class UVRemapper<Point2D>;
template class UVRemapper<Colour>;


Point2D SineWavesBm::operator()(const Point2D &uv)
{
  const Point2D offset(uv[0] - 0.5, uv[1] - 0.5);
  const double len = sqrt(sqr(offset[0]) + sqr(offset[1]));
  const Point2D dirvec(offset[0] / max(len, 0.0001), offset[1] / max(len, 0.0001));
  const double factor = 2 * M_PI / 0.05;
  const double rad = factor * len;
  const double slope = cos(rad);
  Point2D rv(slope * dirvec[0], slope * dirvec[1]);
  errs() << rv << endl;
  return rv;
}

Point2D BubblesBm::operator()(const Point2D &uv)
{
  return Point2D();
}

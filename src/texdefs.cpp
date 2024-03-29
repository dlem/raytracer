#include "texdefs.hpp"
#include "cmdopts.hpp"

using namespace std;

// The list of remapping types that the 'get' function understands.
static map<const string, REMAPTYPE> s_remaptypes =
{
  { "cubetop", REMAP_CUBETOP },
  { "cubefront", REMAP_CUBEFRONT },
  { "cubebot", REMAP_CUBEBOT },
  { "cyltop", REMAP_CYLTOP },
  { "conetop", REMAP_CONETOP },
};

// Constructs a UV remapping by searching for the provided string in the list of
// remap types.
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

// Actually compute the remapping. These are all just linear transformations
// based on the nature of my UV mappings for the primitives involved.
template<typename TMapped>
TMapped UVRemapper<TMapped>::operator()(const Point2D &uv)
{
  bool perturb = true;
  Point2D rv(uv);
  switch(m_ty)
  {
    case REMAP_CUBETOP:
    {
      const double l = 1/4., r = 2/4., b = 1/3., t = 2/3.;
      perturb = inrange(uv[0], l, r) && inrange(uv[1], b, t);
      if(perturb)
	rv = Point2D((uv[0] - l) / (r - l), (uv[1] - b) / (t - b));
      break;
    }
    case REMAP_CUBEFRONT:
    {
      const double l = 1/4., r = 2/4., b = 0/3., t = 1/3.;
      perturb = inrange(uv[0], l, r) && inrange(uv[1], b, t);
      if(perturb)
	rv = Point2D((uv[0] - l) / (r - l), (uv[1] - b) / (t - b));
      break;

    }
    case REMAP_CUBEBOT:
    {
      const double l = 3/4., r = 4/4., b = 1/3., t = 2/3.;
      perturb = inrange(uv[0], l, r) && inrange(uv[1], b, t);
      if(perturb)
	rv = Point2D((uv[0] - l) / (r - l), (uv[1] - b) / (t - b));
      break;

    }
    case REMAP_CYLTOP:
    {
      const double l = 0.5, r = 1, b = 0.5, t = 1;
      perturb = inrange(uv[0], l, r) && inrange(uv[1], b, t);
      if(perturb)
      {
	rv = Point2D((uv[0] - l) / (r - l), (uv[1] - b) / (t - b));
      }
      break;
    }
    case REMAP_CONETOP:
    {
      const double b = 0.5, t = 1;
      perturb = inrange(uv[1], b, t);
      if(perturb)
      {
	rv = Point2D(uv[0], (uv[1] - b) / (t - b));
      }
      break;
    }
    default:
    {
      assert(false);
    }
  }
  if(!perturb)
    return TMapped();
  return m_tex(rv);
}

template class UVRemapper<Point2D>;
template class UVRemapper<Colour>;

Point2D SineWavesBm::operator()(const Point2D &uv)
{
  // Compute the distance from the center of the ripple (ie, the center of the
  // UV map).
  const Point2D offset(uv[0] - 0.5, uv[1] - 0.5);
  const double len = sqrt(sqr(offset[0]) + sqr(offset[1]));

  // Dirvec ~ vector pointing away from the center.
  const Point2D dirvec(offset[0] / max(len, 0.0001), offset[1] / max(len, 0.0001));
  const double factor = 3 * 2 * M_PI / 0.2;
  const double rad = factor * len;

  // The slope of a sine ripple is given by the cos function.
  const double slope = cos(rad);

  // Perturb the normal according to the slope.
  Point2D rv(slope * dirvec[0], slope * dirvec[1]);
  return rv;
}

Colour CheckerTexture::operator()(const Point2D &uv)
{
  // Divisible by both 3 and four => aligned properly with edges of a square.
  const double resolution = 24;
  const int n = (int)(uv[0] * resolution) + (int)(uv[1] * resolution);
  return n % 2 ? m_c : Colour(0);
}

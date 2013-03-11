#include "textures.hpp"
#include <string>
#include <functional>
#include <map>
#include "image.hpp"

using namespace std;

static map<const string, function<Bumpmap *()>> s_bumpmaps = {
};

static map<const string, function<Texture *()>> s_textures = {
};

class ImageBumpmap : public Bumpmap
{
public:
  ImageBumpmap(const Image &img)
    : m_bumpmap(img.width(), img.height(), 2)
  {
    auto intensity = [&img](int x, int y)
    {
      return (img(x, y, 0) + img(x, y, 1) + img(x, y, 2))/3;
    };

    // Compute x gradients.
    for(int ixrow = 0; ixrow < img.height(); ixrow++)
    {
      if(img.width() > 1)
      {
        m_bumpmap(0, ixrow, 0) = intensity(1, ixrow) - intensity(0, ixrow);
        m_bumpmap(img.width() - 1, ixrow, 0) = intensity(img.width() - 1, ixrow) -
                                               intensity(img.width() - 2, ixrow);
      }

      for(int ixcol = 1; ixcol < img.width() - 1; ixcol++)
      {
        m_bumpmap(ixcol, ixrow, 0) = 0.5 * (intensity(ixcol + 1, ixrow) -
                                            intensity(ixcol - 1, ixrow));
      }
    }

    // Compute y gradients.
    if(img.height() > 1)
    {
      for(int ixcol = 0; ixcol < img.width(); ixcol++)
      {
        m_bumpmap(ixcol, 0, 1) = intensity(ixcol, 1) - intensity(ixcol, 0);
        m_bumpmap(ixcol, img.height() - 1, 1) = intensity(ixcol, img.height() - 1) -
                                                intensity(ixcol, img.height() - 2);
      }
    }

    for(int ixrow = 1; ixrow < img.height() - 1; ixrow++)
    {
      for(int ixcol = 0; ixcol < img.width(); ixcol++)
      {
        m_bumpmap(ixcol, ixrow, 1) = 0.5 * (intensity(ixcol, ixrow + 1) -
                                            intensity(ixcol, ixrow - 1));
      }
    }
  }

  virtual Point2D operator()(const Point2D &uv)
  {
    const int u = clamp<double>(uv[0] * m_bumpmap.width(), 0, m_bumpmap.width());
    const int v = clamp<double>(uv[1] * m_bumpmap.height(), 0, m_bumpmap.height());
    return Point2D(m_bumpmap(u, v, 0), m_bumpmap(u, v, 1));
  }

private:
  Image m_bumpmap;
};

class ImageTexture : public Texture
{
public:
  // Takes ownership of the texture!
  ImageTexture(const Image &texture)
    : m_texture(texture) {}
  virtual ~ImageTexture() { delete &m_texture; }

  virtual Colour operator()(const Point2D &uv)
  {
    const int u = clamp<double>(uv[0] * m_texture.width(), 0, m_texture.width());
    const int v = clamp<double>(uv[1] * m_texture.height(), 0, m_texture.height());
    return Colour(m_texture(u, v, 0), m_texture(u, v, 1), m_texture(u, v, 2));
  }

private:
  const Image &m_texture;
};

Bumpmap *Bumpmap::get(const string &name)
{
  auto it = s_bumpmaps.find(name);
  if(it != s_bumpmaps.end())
    return it->second();

  Image img;
  if(img.loadPng(name))
    return new ImageBumpmap(img);
  else
    return 0;
}

Texture *Texture::get(const string &name)
{
  auto it = s_textures.find(name);
  if(it != s_textures.end())
    return it->second();

  Image *img = new Image;
  if(img->loadPng(name))
    return new ImageTexture(*img);
  else
  {
    delete img;
    return 0;
  }
}

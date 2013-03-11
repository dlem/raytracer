/**
 * Ray path tracing code.
 *
 * Daniel Lemmond, dlemmond, 20302247.
**/

#include <functional>
#include <list>
#include "scene.hpp"
#include "light.hpp"

typedef SceneNode::FlatList FlatList;
typedef SceneNode::FlatGeo FlatGeo;

// Callback type that gets passed to my main raytracing function.
typedef std::function<bool(const FlatGeo &, double, const RTHitInfo &)>
        RaytraceFn;

// Stores ray tracing "context" -- ie, anything I might need and don't want to
// pass around in a massive argument list.
struct RTContext
{
  RTContext(const FlatList &geo, const Point3D &eye, const Vector3D &view,
            const Vector3D &up, const Colour &ambient, const std::list<Light *> &lights)
    : geo(geo), width(width), height(height), eye(eye), view(view)
    , up(up), ambient(ambient), lights(lights)
  {}

  const FlatList &geo;
  int width, height;
  const Point3D &eye;
  const Vector3D &view;
  const Vector3D &up;
  double fov_x, fov_y;
  const Colour &ambient;
  const std::list<Light *> &lights;
};

bool raytrace(const RTContext &ctx, const Point3D &src, const Point3D &ray, const RaytraceFn &fn);

// Returns true if any primitive hit yields a t-value in [tlo, thi].
bool raytrace_within(const RTContext &ctx, const Point3D &src,
                     const Point3D &ray,
                     double tlo, double thi);

// Finds the hit with the smallest t-value greater or equal to tlo and returns
// the relevant information.
double raytrace_min(const RTContext &ctx, const Point3D &src,
    const Point3D &ray,
    double tlo,
    const FlatGeo **pg,
    Vector3D &normal,
    Point2D &uv,
    Vector3D &u,
    Vector3D &v);

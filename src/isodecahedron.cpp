#include "isodecahedron.hpp"

namespace isodecahedron
{
  const Vector3D vertices[12] = {
    {0, 1, phi},
    {0, 1, -phi},
    {0, -1, phi},
    {0, -1, -phi},
    
    {1, phi, 0},
    {1, -phi, 0},
    {-1, phi, 0},
    {-1, -phi, 0},
    
    {phi, 0, 1},
    {phi, 0, -1},
    {-phi, 0, 1},
    {-phi, 0, -1},
  };

  const faces_t faces[20] = {
    {0, 2, 8},
    {0, 2, 10},

    {1, 3, 9},
    {1, 3, 11},

    {4, 6, 0},
    {4, 6, 1},
    
    {5, 7, 2},
    {5, 7, 3},

    {8, 9, 4},
    {8, 9, 5},

    {10, 11, 6},
    {10, 11, 7},

    {0, 4, 8}, // +++
    {1, 4, 9}, // ++-
    {2, 5, 8}, // +-+
    {3, 5, 9}, // +--
    {0, 6, 10}, // -++
    {1, 6, 11}, // -+-
    {2, 7, 10}, // --+
    {3, 7, 11}, // ---
  };
}

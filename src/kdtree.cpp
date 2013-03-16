#include "kdtree.hpp"
#include <cstdlib>
#include <vector>

using namespace std;

void kdtree_test()
{
  auto r = [] { return (rand() % 100000) * 0.001; };
  const int num = 100;

  vector<KDNode *> nodes;
  for(int i = 0; i < num; i++)
  {
    KDNode *node = new KDNode(Point3D(r(), r(), r()));
    nodes.push_back(node);
  }

  KDTree<KDNode> tree;
  tree.build(nodes.begin(), nodes.end());

  for(int i = 0; i < 100; i++)
  {
    if(rand() % 2 == 0)
    {
      const Point3D pt = Point3D(r(), r(), r());
      assert(!tree.find(pt));
    }
    else
    {
      const int idx = rand() % num;
      const KDNode *node = nodes[idx];
      const Point3D &pt = node->pt;
      const KDNode *found = tree.find(pt);
      assert(found);
      assert(tree.find(pt) == node);
    }
  }

  for(int i = 0; i < 100; i++)
  {
    const int idx = rand() % num;
    const KDNode *node = nodes[idx];
    const Point3D &pt = node->pt;
    Point3D jittered = pt;
    jittered[rand() % 3] += -0.00001 + rand() % 1000 * 0.00000002;
    assert(tree.find_nn(jittered) == node);
  }

  cerr << "All tests passed." << endl;
}

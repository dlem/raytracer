#include "kdtree.hpp"
#include <cstdlib>
#include <vector>

using namespace std;

void kdtree_test()
{
  default_random_engine rng(time(0));
  auto r = [&rng] { return (rng() % 100000) * 0.001; };
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
    if(rng() % 2 == 0)
    {
      const Point3D pt = Point3D(r(), r(), r());
      assert(!tree.find(pt));
    }
    else
    {
      const int idx = rng() % num;
      const KDNode *node = nodes[idx];
      const Point3D &pt = node->pt;
      const KDNode *found = tree.find(pt);
      assert(found);
      assert(tree.find(pt) == node);
    }
  }

  for(int i = 0; i < 100; i++)
  {
    const int idx = rng() % num;
    const KDNode *node = nodes[idx];
    const Point3D &pt = node->pt;
    Point3D jittered = pt;
    jittered[rng() % 3] += -0.00001 + rng() % 1000 * 0.00000002;
    assert(tree.find_nn(jittered) == node);
  }

  for(int i = 0; i < 100; i++)
  {
    const int idx = rng() % num;
    const KDNode *node = nodes[idx];
    const Point3D &pt = node->pt;
    Point3D jittered = pt;
    jittered[rng() % 3] += -0.00001 + rng() % 1000 * 0.00000002;
    KDTree<KDNode>::TPQueue q;
    tree.find_nnn(jittered, 50, q, numeric_limits<double>::max());
    while(q.size() > 1)
    {
      q.pop();
    }
    assert(q.top().node == node);
    q.pop();
  }

  cerr << "All tests passed." << endl;
  exit(0);
}

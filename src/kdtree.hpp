/**
 * Daniel Lemmond
 * dlemmond
 * 20302247
**/

#ifndef __KD_TREE_HPP__
#define __KD_TREE_HPP__

#include <queue>
#include <limits>
#include "algebra.hpp"

struct KDNode
{
  KDNode(const Point3D &pt) : pt(pt), lchild(0), rchild(0) {}
  Point3D pt;
  KDNode *lchild;
  KDNode *rchild;
};

template<typename TNode>
class KDTree
{
private:

public:

  KDTree() : m_root(0) {}

  struct PQNode
  {
    PQNode() {}
    PQNode(const Point3D &pt, KDNode *node)
      : node(node)
      , dist((pt - node->pt).length())
    {}
    double dist;
    KDNode *node;
    bool operator<(const PQNode &other) const
    {
      return dist < other.dist;
    }
  };

  typedef std::priority_queue<PQNode, std::vector<PQNode>> TPQueue;

  template<typename TIt>
  void build(TIt start, TIt end)
  {
    m_root = (TNode *)build(start, end, 0);
  }

  TNode *find(const Point3D &pt)
  {
    return find(pt, m_root, 0);
  }

  TNode *find_nn(const Point3D &pt, double maxdist = std::numeric_limits<double>::max())
  {
    TPQueue q;
    find_nnn(pt, 1, m_root, q, 0, maxdist);
    return (TNode *)(q.top().node);
  }

  void find_nnn(const Point3D &pt, int n, TPQueue &q, double maxdist = std::numeric_limits<double>::max())
  {
    find_nnn(pt, n, m_root, q, maxdist, 0);
  }

private:

  static KDNode *find(const Point3D &pt, KDNode *node, int depth)
  {
    const int split = depth % 3;

    if(!node)
      return 0;
    const Point3D &npt = node->pt;
    if(npt[0] == pt[0] && npt[1] == pt[1] && npt[2] == pt[2])
      return node;
    if(pt[split] < npt[split])
      return find(pt, node->lchild, depth + 1);
    return find(pt, node->rchild, depth + 1);
  }

  static void find_nnn(const Point3D &pt, int n, KDNode *node, TPQueue &nns, double maxdist, int depth)
  {
    const int split = depth % 3;

    if(!node)
      return;
    
    KDNode *tree1, *tree2;
    if(pt[split] < node->pt[split])
    {
      tree1 = node->lchild;
      tree2 = node->rchild;
    }
    else
    {
      tree1 = node->rchild;
      tree2 = node->lchild;
    }

    find_nnn(pt, n, tree1, nns, maxdist, depth + 1);

    if(nns.size() < n)
    {
      nns.push(PQNode(pt, node));
      find_nnn(pt, n, tree2, nns, maxdist, depth + 1);
    }
    else
    {
      const double mindist = abs(pt[split] - node->pt[split]);
      if(mindist < maxdist && mindist < nns.top().dist)
      {
	PQNode pqn(pt, node);
	if(pqn.dist < nns.top().dist)
	{
	  nns.pop();
	  nns.push(pqn);
	}
	find_nnn(pt, n, tree2, nns, maxdist, depth + 1);
      }
    }
  }

  template<typename TIt>
  static KDNode *build(TIt start, TIt end, int depth)
  {
    const int split = depth % 3;

    if(start == end)
      return 0;

    std::sort(start, end, [split](const KDNode *c1, const KDNode *c2)
	{ return c1->pt[split] < c2->pt[split]; });

    auto median = start + (end - start) / 2;
    KDNode *node = *median;
    node->lchild = build(start, median, depth + 1);
    node->rchild = build(median + 1, end, depth + 1);
    return node;
  }

  TNode *m_root;
};

void kdtree_test();

#endif

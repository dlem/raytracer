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
      , dist2((pt - node->pt).length2())
    {}
    double dist2;
    KDNode *node;
    bool operator<(const PQNode &other) const
    {
      return dist2 < other.dist2;
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

  TNode *find_nn(const Point3D &pt, double maxdist2 = std::numeric_limits<double>::max())
  {
    TPQueue q;
    find_nnn(pt, 1, m_root, q, maxdist2, 0);
    return (TNode *)(q.top().node);
  }

  void find_nnn(const Point3D &pt, int n, TPQueue &q, double maxdist2 = std::numeric_limits<double>::max())
  {
    find_nnn(pt, n, m_root, q, maxdist2, 0);
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

  static void find_nnn(const Point3D &pt, int n, KDNode *node, TPQueue &nns, double maxdist2, int depth)
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

    find_nnn(pt, n, tree1, nns, maxdist2, depth + 1);

    if(nns.size() < n)
    {
      nns.push(PQNode(pt, node));
      find_nnn(pt, n, tree2, nns, maxdist2, depth + 1);
    }
    else
    {
      const double mindist2 = sqr(pt[split] - node->pt[split]);
      if(mindist2 < maxdist2 && mindist2 < nns.top().dist2)
      {
	PQNode pqn(pt, node);
	if(pqn.dist2 < nns.top().dist2)
	{
	  nns.pop();
	  nns.push(pqn);
	}
	find_nnn(pt, n, tree2, nns, maxdist2, depth + 1);
      }
    }
  }

  template<typename TIt>
  static KDNode *build(TIt start, TIt end, int depth)
  {
    const int split = depth % 3;

    if(start == end)
      return 0;

    struct SortCriteria
    {
      SortCriteria(int split) : split(split) {}
      bool operator()(const KDNode *c1, const KDNode *c2)
      { return c1->pt[split] < c2->pt[split]; }
      int split;
    };

    SortCriteria sc(split);
    std::sort(start, end, sc);

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

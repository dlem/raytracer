/**
 * Daniel Lemmond
 * dlemmond
 * 20302247
**/

#ifndef __KD_TREE_HPP__
#define __KD_TREE_HPP__

#include "algebra.hpp"
#include <queue>

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
  struct PQNode
  {
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

  typedef std::priority_queue<PQNode> TPQueue;

public:

  KDTree() : m_root(0) {}

  template<typename TIt>
  void build(TIt start, TIt end)
  {
    m_root = build(start, end, 0);
  }

  TNode *find(const Point3D &pt)
  {
    return find(pt, m_root, 0);
  }

  TNode *find_nn(const Point3D &pt)
  {
    TPQueue q;
    find_nnn(pt, 1, m_root, q, 0);
    return q.top().node;
  }

  TNode *find_nnn(const Point3D &pt, int n, std::vector<TNode *> &ns)
  {
    TPQueue nns;
    find_nnn(pt, n, nns); 
    while(!nns.empty())
    {
      ns.push(nns.top().node);
      nns.pop();
    }
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

  static void find_nnn(const Point3D &pt, int n, KDNode *node, TPQueue &nns, int depth)
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

    find_nnn(pt, n, tree1, nns, depth + 1);

    const double mindist = abs(pt[split] - node->pt[split]);
    if(nns.size() < n || mindist < nns.top().dist)
    {
      nns.push(PQNode(pt, node));
      if(nns.size() > n)
	nns.pop();
      find_nnn(pt, n, tree2, nns, depth + 1);
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

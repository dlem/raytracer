/*
 * This header file defines the KDTree template.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __KD_TREE_HPP__
#define __KD_TREE_HPP__

#include <queue>
#include <limits>
#include <future>
#include <thread>
#include "algebra.hpp"

/*
 * Herein resides KDTree class. KDTrees are data structures good at handling
 * searches of k-dimensional data (although this implementation is technically a
 * 3DTree, as it resides in three dimensions).
 *
 * In this implementation, you can't update the tree -- you must build it all at
 * once from a list of nodes using the build method.
 *
 * Use find_nn and find_nnn to find neighbours.
**/

// The template type passed to KDTree must inherit from this one.
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
public:

  // Use the build method to build the tree.
  KDTree() : m_root(0) {}

  // Node used in priority queue for NNN algorithm.
  struct PQNode
  {
    PQNode() {}
    PQNode(const Point3D &pt, KDNode *node)
      : node(node)
      , dist2((pt - node->pt).length2())
    {}

    // Store squared distance for performance. Squaredness makes a difference of
    // about 3% in searches.
    double dist2;
    KDNode *node;

    // Comparison operator used in std::priority_queue.
    bool operator<(const PQNode &other) const { return dist2 < other.dist2; }
  };

  // PQueue type used in NNN search.
  typedef std::priority_queue<PQNode, std::vector<PQNode>> TPQueue;

  // Build tree from an iterator of node pointers.
  template<typename TIt>
  void build(TIt start, TIt end) { m_root = (TNode *)build(start, end, 0); }

  // Find a point in the tree (exact match or nothing).
  TNode *find(const Point3D &pt) { return find(pt, m_root, 0); }

  // Find the nearest neighbour to a point.
  TNode *find_nn(const Point3D &pt,
      double maxdist2 = std::numeric_limits<double>::max())
  {
    TPQueue q;
    find_nnn(pt, 1, m_root, q, maxdist2, 0);
    return (TNode *)(q.top().node);
  }

  // Find the N nearest neighbours to a point and put them in the queue.
  // Specifying maxdist2, the maximum distance squared we allow neighbours to
  // have from the point, can significantly reduce the runtime of searches in
  // sparse areas.
  void find_nnn(const Point3D &pt, int n, TPQueue &q,
      double maxdist2 = std::numeric_limits<double>::max())
  {
    find_nnn(pt, n, m_root, q, maxdist2, 0);
  }

private:
  // Algorithm implementations.
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


  static void find_nnn(const Point3D &pt, int n, KDNode *node, TPQueue &nns,
		       double maxdist2, int depth)
  {
    const int split = depth % 3;

    if(!node)
      return;
    
    KDNode *tree1, *tree2;
    // tree1 := side closest to target
    // tree2 := side farthest from target
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

    // Always recurse on the side of the split that's closest to the target point.
    find_nnn(pt, n, tree1, nns, maxdist2, depth + 1);

    if(nns.size() < n)
    {
      // If we need more points, then just recurse regardless.
      nns.push(PQNode(pt, node));
      find_nnn(pt, n, tree2, nns, maxdist2, depth + 1);
    }
    else
    {
      // Is it possible for a point in the _other_ tree to be closer than our
      // current farthest?
      const double mindist2 = sqr(pt[split] - node->pt[split]);
      if(mindist2 < maxdist2 && mindist2 < nns.top().dist2)
      {
	PQNode pqn(pt, node);
	// priority_queue methods are surprisingly expensive, so do this check
	// ourselves.
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

    struct Recurser
    {
      Recurser(TIt start, TIt end, int depth)
	: start(start), end(end), depth(depth)
      {}

      KDNode *operator()()
      { return KDTree::build(start, end, depth); }

      TIt start, end;
      int depth;
    };

    SortCriteria sc(split);
    std::sort(start, end, sc);

    auto median = start + (end - start) / 2;
    KDNode *node = *median;

    Recurser half1(start, median, depth + 1);
    Recurser half2(median + 1, end, depth + 1);

    if(median - start > 100000)
    {
      auto fut = std::async(std::launch::async, half1);
      node->rchild = half2();
      node->lchild = fut.get();
    }
    else
    {
      node->lchild = half1();
      node->rchild = half2();
    }

    return node;
  }

  TNode *m_root;
};


void kdtree_test();

#endif

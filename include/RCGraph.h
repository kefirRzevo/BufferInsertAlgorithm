#pragma once

#include <iostream>
#include <vector>
#include <memory>

namespace algo {

enum class NodeKind {
  Buffer,
  Steiner,
  Point,
};

struct Point final {
  using CoordTy = int;

  CoordTy X;
  CoordTy Y;
};

struct Node {
  using NodeIdTy = unsigned;
  using FloatTy = float;

  NodeKind Kind;
  std::string Name;
  Point P;

  NodeIdTy Left;
  NodeIdTy Right;
};

struct BufferNode final : Node {

};

struct SteinerNode final : Node {

};

struct PointNode final : Node {
  using FloatTy = Node::FloatTy;

  FloatTy Capacity;
  FloatTy RAT;
};

struct Edge {
  using EdgeIdTy = unsigned;

  std::vector<Point> Ps;
};

class RCGraph final {
  using NodeTy = Node;
  using EdgeTy = Edge;

  using NodeIdTy = NodeTy::NodeIdTy;
  using EdgeIdTy = EdgeTy::EdgeIdTy;

  std::vector<std::unique_ptr<NodeTy>> Nodes;
  std::vector<std::unique_ptr<EdgeTy>> Edges;
  NodeTy* Root = nullptr;

public:

  Node& getNode(NodeIdTy Id) {

  }

  Edge& getEdge(EdgeIdTy Id) {

  }

  void read(std::istream& Is);

  std::vector<NodeIdTy> split(EdgeIdTy Id) {

  }
};

} // namespace algo

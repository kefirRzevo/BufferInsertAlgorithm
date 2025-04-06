#pragma once

#include <iostream>
#include <vector>

namespace algo {

enum class NodeKind {
  Buffer,
  Steiner,
  Point,
};

class Node {

};

class BufferNode : public Node {

};

class SteinerNode : public Node {

};

class PointNode : public Node {

};

class Edge final {

};

template<typename NodeAttrs, typename EdgeAttrs>
class Topology final {
  using NodeId = size_t;
  using EdgeId = size_t;

public:
  void read(std::istream& Is);


};


} // namespace algo

#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

namespace algo {

enum class NodeKindTy {
  Buffer,
  Steiner,
  Point,
};

struct PointTy final {
  using CoordTy = int;

  CoordTy X;
  CoordTy Y;
};

struct NodeTy {
  using NodeIdTy = unsigned;
  using FloatTy = float;

  NodeKindTy Kind;
  std::string Name;
  PointTy P;

  FloatTy Capacity;
  FloatTy RAT;
};

struct EdgeTy {
  using EdgeIdTy = unsigned;

  std::vector<PointTy> Ps;
};

class RCGraph final {
  using CoordTy = PointTy::CoordTy;
  using NodeIdTy = NodeTy::NodeIdTy;
  using FloatTy = NodeTy::FloatTy;
  using EdgeIdTy = EdgeTy::EdgeIdTy;

  class NodeEntryTy final {
    EdgeIdTy Parent;
    EdgeIdTy LHS;
    EdgeIdTy RHS;

  public:
    NodeEntryTy(NodeTy &&Node)
        : Parent(invalidEdgeId()), LHS(invalidEdgeId()), RHS(invalidEdgeId()),
          Node(std::move(Node)) {}

    void setParent(EdgeIdTy P) { Parent = P; }
    void setLHS(EdgeIdTy L) { LHS = L; }
    void setRHS(EdgeIdTy R) { RHS = R; }

    EdgeIdTy getParent() const { return Parent; }
    EdgeIdTy getLHS() const { return LHS; }
    EdgeIdTy getRHS() const { return RHS; }

    NodeTy Node;
  };

  class EdgeEntryTy final {
    NodeIdTy Begin;
    NodeIdTy End;

  public:
    EdgeEntryTy(NodeIdTy Begin, NodeIdTy End, EdgeTy &&Edge)
        : Begin(Begin), End(End), Edge(std::move(Edge)) {}

    void connectAsLHS(RCGraph &G, EdgeIdTy ThisEdgeId) {
      NodeEntryTy &NB = G.getNodeEntry(Begin);
      NB.setLHS(ThisEdgeId);
      NodeEntryTy &NE = G.getNodeEntry(End);
      NE.setParent(ThisEdgeId);
    }

    void connectAsRHS(RCGraph &G, EdgeIdTy ThisEdgeId) {
      NodeEntryTy &NB = G.getNodeEntry(Begin);
      NB.setRHS(ThisEdgeId);
      NodeEntryTy &NE = G.getNodeEntry(End);
      NE.setParent(ThisEdgeId);
    }

    NodeIdTy getBegin() const { return Begin; }
    NodeIdTy getEnd() const { return End; }

    EdgeTy Edge;
  };

  using NodeEntryVectorTy = std::vector<NodeEntryTy>;
  using FreeNodeIdVectorTy = std::vector<NodeIdTy>;
  using EdgeEntryVectorTy = std::vector<EdgeEntryTy>;
  using FreeEdgeIdVectorTy = std::vector<EdgeIdTy>;

  NodeEntryVectorTy Nodes;
  FreeNodeIdVectorTy FreeNodeIds;
  EdgeEntryVectorTy Edges;
  FreeEdgeIdVectorTy FreeEdgeIds;

  NodeIdTy Root;

  NodeEntryTy &getNodeEntry(NodeIdTy NId) {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }
  const NodeEntryTy &getNodeEntry(NodeIdTy NId) const {
    assert(NId < Nodes.size() && "Out of bound NodeId");
    return Nodes[NId];
  }

  EdgeEntryTy &getEdgeEntry(EdgeIdTy EId) {
    assert(EId < Edges.size() && "Out of bound EdgeId");
    return Edges[EId];
  }
  const EdgeEntryTy &getEdgeEntry(EdgeIdTy EId) const {
    assert(EId < Edges.size() && "Out of bound EdgeId");
    return Edges[EId];
  }

  NodeIdTy addConstructedNode(NodeEntryTy &&N) {
    NodeIdTy NId = invalidNodeId();
    if (!FreeNodeIds.empty()) {
      NId = FreeNodeIds.back();
      FreeNodeIds.pop_back();
      Nodes[NId] = std::move(N);
    } else {
      NId = Nodes.size();
      Nodes.push_back(std::move(N));
    }
    return NId;
  }

  EdgeIdTy addConstructedEdge(EdgeEntryTy &&E) {
    assert(findEdge(E.getN1Id(), E.getN2Id()) == BaseGraph::invalidEdgeId() &&
           "Attempt to add duplicate edge.");
    EdgeIdTy EId = invalidEdgeId();
    if (!FreeEdgeIds.empty()) {
      EId = FreeEdgeIds.back();
      FreeEdgeIds.pop_back();
      Edges[EId] = std::move(E);
    } else {
      EId = Edges.size();
      Edges.push_back(std::move(E));
    }

    EdgeEntryTy &EE = getEdgeEntry(EId);

    // Add the edge to the adjacency sets of its nodes.
    EE.connect(*this, EId);
    return EId;
  }

public:
  static NodeIdTy invalidNodeId() {
    return std::numeric_limits<NodeIdTy>::max();
  }

  static EdgeIdTy invalidEdgeId() {
    return std::numeric_limits<EdgeIdTy>::max();
  }

  NodeTy &getNode(NodeIdTy Id) {}

  EdgeTy &getEdge(EdgeIdTy Id) {}

  NodeIdTy addNode(NodeTy &&Node) {
    NodeIdTy NId = addConstructedNode(NodeEntryTy(std::move(Node)));
    return NId;
  }

  EdgeIdTy addEdge(NodeIdTy Begin, NodeIdTy End, EdgeTy &&Edge) {
    EdgeIdTy EId = addConstructedEdge(EdgeEntryTy(Begin, End, std::move(Edge)));
    return EId;
  }

  NodeIdTy getEdgeNodeBegin(EdgeIdTy EId) const {
    return getEdgeEntry(EId).getBegin();
  }

  NodeIdTy getEdgeNodeEnd(EdgeIdTy EId) const {
    return getEdgeEntry(EId).getEnd();
  }

  void removeNode(NodeIdTy NId) {
    NodeEntryTy &N = getNodeEntry(NId);
    for (AdjEdgeItr AEItr = N.adjEdgesBegin(), AEEnd = N.adjEdgesEnd();
         AEItr != AEEnd;) {
      EdgeId EId = *AEItr;
      ++AEItr;
      removeEdge(EId);
    }
    FreeNodeIds.push_back(NId);
  }

  void removeEdge(EdgeIdTy EId) {
    EdgeEntryTy &E = getEdgeEntry(EId);
    E.disconnectFrom(*this, E.getN1Id());
    E.disconnectFrom(*this, E.getN2Id());
    FreeEdgeIds.push_back(EId);
  }

  void read(std::istream &Is);

  void split(EdgeIdTy Id, unsigned PartsCount) {}
};

} // namespace algo

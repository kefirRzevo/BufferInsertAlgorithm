#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>

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

struct RCGraphInterface {
  using CoordTy = PointTy::CoordTy;
  using NodeIdTy = NodeTy::NodeIdTy;
  using FloatTy = NodeTy::FloatTy;
  using EdgeIdTy = EdgeTy::EdgeIdTy;

  static NodeIdTy invalidNodeId() {
    return std::numeric_limits<NodeIdTy>::max();
  }

  static EdgeIdTy invalidEdgeId() {
    return std::numeric_limits<EdgeIdTy>::max();
  }

  virtual NodeTy &getNode(NodeIdTy Id) = 0;

  virtual const NodeTy &getNode(NodeIdTy Id) const = 0;

  virtual EdgeTy &getEdge(EdgeIdTy Id) = 0;

  virtual const EdgeTy &getEdge(EdgeIdTy Id) const = 0;

  virtual NodeIdTy addNode(NodeTy &&Node) = 0;

  virtual EdgeIdTy addEdge(NodeIdTy First, NodeIdTy Last, EdgeTy &&Edge) = 0;

  virtual NodeIdTy getEdgeNodeFirst(EdgeIdTy EId) const = 0;

  virtual NodeIdTy getEdgeNodeLast(EdgeIdTy EId) const = 0;

  virtual EdgeIdTy getParent(NodeIdTy NId) const = 0;

  virtual EdgeIdTy getLHS(NodeIdTy NId) const = 0;

  virtual EdgeIdTy getRHS(NodeIdTy NId) const = 0;

  virtual NodeIdTy getRoot() const = 0;

  virtual void removeNode(NodeIdTy NId) = 0;

  virtual void removeEdge(EdgeIdTy EId) = 0;

  virtual void dump(std::ostream& OS) const = 0;
};

class RCGraph final: private RCGraphInterface {
  using CoordTy = RCGraphInterface::CoordTy;
  using NodeIdTy = RCGraphInterface::NodeIdTy;
  using FloatTy = RCGraphInterface::FloatTy;
  using EdgeIdTy = RCGraphInterface::EdgeIdTy;

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
    NodeIdTy First;
    NodeIdTy Last;

  public:
    EdgeEntryTy(NodeIdTy First, NodeIdTy Last, EdgeTy &&Edge)
        : First(First), Last(Last), Edge(std::move(Edge)) {}

    void connectAsLHS(RCGraph &G, EdgeIdTy ThisEdgeId) {
      NodeEntryTy &NH = G.getNodeEntry(First);
      NH.setLHS(ThisEdgeId);
      NodeEntryTy &NL = G.getNodeEntry(Last);
      NL.setParent(ThisEdgeId);
    }

    void connectAsRHS(RCGraph &G, EdgeIdTy ThisEdgeId) {
      NodeEntryTy &NH = G.getNodeEntry(First);
      NH.setRHS(ThisEdgeId);
      NodeEntryTy &NL = G.getNodeEntry(Last);
      NL.setParent(ThisEdgeId);
    }

    bool isLHS(RCGraph& G, EdgeIdTy ThisEdgeId) const {
      NodeEntryTy &NH = G.getNodeEntry(First);
      return NH.getLHS() == ThisEdgeId;
    }

    bool isRHS(RCGraph& G, EdgeIdTy ThisEdgeId) const {
      NodeEntryTy &NH = G.getNodeEntry(First);
      return NH.getRHS() == ThisEdgeId;
    }

    NodeIdTy getFirst() const { return First; }
    NodeIdTy getLast() const { return Last; }

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

  EdgeIdTy addConstructedEdge(EdgeEntryTy &&EE) {
    EdgeIdTy EId = invalidEdgeId();
    if (!FreeEdgeIds.empty()) {
      EId = FreeEdgeIds.back();
      FreeEdgeIds.pop_back();
      Edges[EId] = std::move(EE);
    } else {
      EId = Edges.size();
      Edges.push_back(std::move(EE));
    }
    EE = getEdgeEntry(EId);
    NodeIdTy FirstId = EE.getFirst();
    const NodeEntryTy& FNE = getNodeEntry(FirstId);
    if (FNE.getLHS() == invalidNodeId()) {
      EE.connectAsLHS(*this, EId);
    } else if (FNE.getRHS() == invalidNodeId()) {
      EE.connectAsRHS(*this, EId);
    } else {
      assert("Attempt to add duplicate edge.");
    }
    return EId;
  }

public:
  NodeTy &getNode(NodeIdTy Id) override {
    return Nodes.at(Id).Node;
  }

  const NodeTy &getNode(NodeIdTy Id) const override {
    return Nodes.at(Id).Node;
  }

  EdgeTy &getEdge(EdgeIdTy Id) override {
    return Edges.at(Id).Edge;
  }

  const EdgeTy &getEdge(EdgeIdTy Id) const override {
    return Edges.at(Id).Edge;
  }

  NodeIdTy addNode(NodeTy &&Node) override {
    NodeIdTy NId = addConstructedNode(NodeEntryTy(std::move(Node)));
    return NId;
  }

  EdgeIdTy addEdge(NodeIdTy First, NodeIdTy Last, EdgeTy &&Edge) override {
    EdgeIdTy EId = addConstructedEdge(EdgeEntryTy(First, Last, std::move(Edge)));
    return EId;
  }

  NodeIdTy getEdgeNodeFirst(EdgeIdTy EId) const override {
    return getEdgeEntry(EId).getFirst();
  }

  NodeIdTy getEdgeNodeLast(EdgeIdTy EId) const override {
    return getEdgeEntry(EId).getLast();
  }

  EdgeIdTy getParent(NodeIdTy NId) const override {
    return getNodeEntry(NId).getParent();
  }

  EdgeIdTy getLHS(NodeIdTy NId) const override {
    return getNodeEntry(NId).getLHS();
  }

  EdgeIdTy getRHS(NodeIdTy NId) const override {
    return getNodeEntry(NId).getRHS();
  }

  NodeIdTy getRoot() const override { return Root; }

  void removeNode(NodeIdTy NId) override {
    NodeEntryTy &N = getNodeEntry(NId);
    EdgeIdTy LHSId = N.getLHS();
    if (LHSId != invalidEdgeId()) {
      removeEdge(LHSId);
    }
    EdgeIdTy RHSId = N.getRHS();
    if (RHSId != invalidEdgeId()) {
      removeEdge(RHSId);
    }
    EdgeIdTy ParentId = N.getParent();
    if (ParentId != invalidEdgeId()) {
      removeEdge(ParentId);
    }
    FreeNodeIds.push_back(NId);
    std::sort(FreeNodeIds.begin(), FreeNodeIds.end());
  }

  void removeEdge(EdgeIdTy EId) override {
    EdgeEntryTy &E = getEdgeEntry(EId);
    NodeIdTy First = E.getFirst();
    NodeEntryTy& FNE = getNodeEntry(First);
    if (E.isLHS(*this, EId)) {
      FNE.setLHS(invalidEdgeId());
    }
    if (E.isRHS(*this, EId)) {
      FNE.setRHS(invalidEdgeId());
    }
    NodeIdTy Last = E.getLast();
    NodeEntryTy& LNE = getNodeEntry(Last);
    LNE.setParent(invalidEdgeId());
    FreeEdgeIds.push_back(EId);
    std::sort(FreeEdgeIds.begin(), FreeEdgeIds.end());
  }

  void dump(std::ostream& OS) const override {
    OS << "digraph {\n";
    OS << "\trankdir=LR;\n";
    OS << "\tnode[style=filled, fontcolor=black];\n";
    for (NodeIdTy NId = NodeIdTy{}; NId != Nodes.size(); ++NId) {
      if (std::binary_search(FreeNodeIds.begin(), FreeNodeIds.end(), NId)) {
        continue;
      }
      //const NodeEntryTy& NE = Nodes[NId];
      OS << "\tnode_" << NId << "[label = \"Id " << NId << "\"];\n";
    }
    for (EdgeIdTy EId = EdgeIdTy{}; EId != Edges.size(); ++EId) {
      if (std::binary_search(FreeEdgeIds.begin(), FreeEdgeIds.end(), EId)) {
        continue;
      }
      const EdgeEntryTy& EE = Edges[EId];
      NodeIdTy First = EE.getFirst();
      NodeIdTy Last = EE.getLast();
      OS << "\tnode_" << First << " -> node_" << Last << ";\n";
    }
    OS << "}" << std::endl;
  }
};

RCGraph read(std::istream &IS);

} // namespace algo

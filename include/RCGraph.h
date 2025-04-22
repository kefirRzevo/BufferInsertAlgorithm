#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <span>
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

  PointTy(CoordTy x, CoordTy y) : X{x}, Y{y} {}
  bool operator==(const PointTy &rhs) {
    return X == rhs.X && Y == rhs.Y;
  }
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

  virtual const std::vector<EdgeIdTy> &getChildren(NodeIdTy NId) const = 0;

  virtual void setRoot(NodeIdTy NId) = 0;

  virtual NodeIdTy getRoot() const = 0;

  virtual void removeNode(NodeIdTy NId) = 0;

  virtual void removeEdge(EdgeIdTy EId) = 0;
};

class RCGraph final : private RCGraphInterface {
public:
  using CoordTy = RCGraphInterface::CoordTy;
  using NodeIdTy = RCGraphInterface::NodeIdTy;
  using FloatTy = RCGraphInterface::FloatTy;
  using EdgeIdTy = RCGraphInterface::EdgeIdTy;
  using RCGraphInterface::invalidEdgeId;
  using RCGraphInterface::invalidNodeId;

private:
  class NodeEntryTy final {
    EdgeIdTy Parent;
    std::vector<EdgeIdTy> Children;

  public:
    NodeEntryTy(NodeTy &&Node)
        : Parent(invalidEdgeId()), Children({}), Node(std::move(Node)) {}

    void setParent(EdgeIdTy P) { Parent = P; }
    void addChild(EdgeIdTy EId) { Children.push_back(EId); }
    void removeChild(EdgeIdTy EId) {
      Children.erase(std::remove(Children.begin(), Children.end(), EId));
    }

    EdgeIdTy getParent() const { return Parent; }
    const std::vector<EdgeIdTy> &getChildren() const { return Children; }

    NodeTy Node;
  };

  class EdgeEntryTy final {
    NodeIdTy First;
    NodeIdTy Last;

  public:
    EdgeEntryTy(NodeIdTy First, NodeIdTy Last, EdgeTy &&Edge)
        : First(First), Last(Last), Edge(std::move(Edge)) {}

    void connect(RCGraph &G, EdgeIdTy ThisEdgeId) {
      NodeEntryTy &NF = G.getNodeEntry(First);
      assert(!isConnected(G, ThisEdgeId) && "Edge exists");
      NF.addChild(ThisEdgeId);
      NodeEntryTy &NL = G.getNodeEntry(Last);
      NL.setParent(ThisEdgeId);
    }

    bool isConnected(RCGraph &G, EdgeIdTy ThisEdgeId) const {
      NodeEntryTy &NF = G.getNodeEntry(First);
      const std::vector<EdgeIdTy> &Children = NF.getChildren();
      return std::find(Children.begin(), Children.end(), ThisEdgeId) !=
             Children.end();
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
    assert(!EE.isConnected(*this, EId) && "Attempt to add duplicate edge");
    EE.connect(*this, EId);
    return EId;
  }

public:
  NodeTy &getNode(NodeIdTy Id) override { return Nodes.at(Id).Node; }

  const NodeTy &getNode(NodeIdTy Id) const override {
    return Nodes.at(Id).Node;
  }

  EdgeTy &getEdge(EdgeIdTy Id) override { return Edges.at(Id).Edge; }

  const EdgeTy &getEdge(EdgeIdTy Id) const override {
    return Edges.at(Id).Edge;
  }

  NodeIdTy addNode(NodeTy &&Node) override {
    NodeIdTy NId = addConstructedNode(NodeEntryTy(std::move(Node)));
    return NId;
  }

  EdgeIdTy addEdge(NodeIdTy First, NodeIdTy Last, EdgeTy &&Edge) override {
    EdgeIdTy EId =
        addConstructedEdge(EdgeEntryTy(First, Last, std::move(Edge)));
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

  const std::vector<EdgeIdTy> &getChildren(NodeIdTy NId) const override {
    return getNodeEntry(NId).getChildren();
  }

  void setRoot(NodeIdTy NId) override { Root = NId; }

  NodeIdTy getRoot() const override { return Root; }

  void removeNode(NodeIdTy NId) override {
    NodeEntryTy &NE = getNodeEntry(NId);
    EdgeIdTy ParentId = NE.getParent();
    if (ParentId != invalidEdgeId()) {
      removeEdge(ParentId);
    }
    const std::vector<EdgeIdTy> Children = NE.getChildren();
    for (EdgeIdTy EId : Children) {
      removeEdge(EId);
    }
    FreeNodeIds.push_back(NId);
    std::sort(FreeNodeIds.begin(), FreeNodeIds.end());
  }

  void removeEdge(EdgeIdTy EId) override {
    EdgeEntryTy &E = getEdgeEntry(EId);
    NodeIdTy First = E.getFirst();
    NodeEntryTy &FNE = getNodeEntry(First);
    FNE.removeChild(EId);
    NodeIdTy Last = E.getLast();
    NodeEntryTy &LNE = getNodeEntry(Last);
    LNE.setParent(invalidEdgeId());
    FreeEdgeIds.push_back(EId);
    std::sort(FreeEdgeIds.begin(), FreeEdgeIds.end());
  }
};

RCGraph read(std::istream &IS);

void dumpDot(const RCGraph &G, std::ostream &OS);

void write(const RCGraph &G, std::ostream &OS);

} // namespace algo

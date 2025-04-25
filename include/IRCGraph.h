#pragma once

#include "Config.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <span>
#include <vector>

namespace algo {

template <typename NodeAttrs, typename EdgeAttrs, typename Attrs, typename Impl>
class IRCGraph {
  Impl *impl() { return static_cast<Impl *>(this); }

  const Impl *impl() const { return static_cast<const Impl *>(this); }

public:
  using NodeIdTy = typename NodeAttrs::NodeIdTy;
  using EdgeIdTy = typename EdgeAttrs::EdgeIdTy;

  static NodeIdTy invalidNodeId() {
    return std::numeric_limits<NodeIdTy>::max();
  }

  static EdgeIdTy invalidEdgeId() {
    return std::numeric_limits<EdgeIdTy>::max();
  }

  const Attrs &getAttrs() const { return impl()->getAttrs(); }

  Attrs &getAttrs() { return impl()->getAttrs(); }

  void setAttrs(Attrs &&A) { impl()->setAttrs(std::move(A)); }

  NodeAttrs &getNode(NodeIdTy NId) { return impl()->getNode(NId); }

  const NodeAttrs &getNode(NodeIdTy NId) const { return impl()->getNode(NId); }

  EdgeAttrs &getEdge(EdgeIdTy EId) { return impl()->getEdge(EId); }

  const EdgeAttrs &getEdge(EdgeIdTy EId) const { return impl()->getEdge(EId); }

  NodeIdTy addNode(NodeAttrs &&Node) {
    return impl()->addNode(std::move(Node));
  }

  EdgeIdTy addEdge(NodeIdTy First, NodeIdTy Last, EdgeAttrs &&Edge) {
    return impl()->addEdge(First, Last, std::move(Edge));
  }

  NodeIdTy getEdgeNodeFirst(EdgeIdTy EId) const {
    return impl()->getEdgeNodeFirst(EId);
  }

  NodeIdTy getEdgeNodeLast(EdgeIdTy EId) const {
    return impl()->getEdgeNodeLast(EId);
  }

  EdgeIdTy getParent(NodeIdTy NId) const { return impl()->getParent(NId); }

  const std::vector<EdgeIdTy> &getChildren(NodeIdTy NId) const {
    return impl()->getChildren(NId);
  }

  void setRoot(NodeIdTy NId) { impl()->setRoot(NId); }

  NodeIdTy getRoot() const { return impl()->getRoot(); }

  void removeNode(NodeIdTy NId) { impl()->removeNode(NId); }

  void removeEdge(EdgeIdTy EId) { impl()->removeEdge(EId); }
};

template <typename NodeAttrs, typename EdgeAttrs, typename Attrs>
class RCGraph final : public IRCGraph<NodeAttrs, EdgeAttrs, Attrs,
                                      RCGraph<NodeAttrs, EdgeAttrs, Attrs>> {
  using BaseGraph = IRCGraph<NodeAttrs, EdgeAttrs, Attrs,
                             RCGraph<NodeAttrs, EdgeAttrs, Attrs>>;

public:
  using NodeIdTy = typename BaseGraph::NodeIdTy;
  using EdgeIdTy = typename BaseGraph::EdgeIdTy;
  using BaseGraph::invalidEdgeId;
  using BaseGraph::invalidNodeId;

private:
  class NodeEntryTy final {
    EdgeIdTy Parent;
    std::vector<EdgeIdTy> Children;

  public:
    NodeEntryTy(NodeAttrs &&Node)
        : Parent(invalidEdgeId()), Children({}), Node(std::move(Node)) {}

    void setParent(EdgeIdTy P) { Parent = P; }
    void addChild(EdgeIdTy EId) { Children.push_back(EId); }
    void removeChild(EdgeIdTy EId) {
      Children.erase(std::remove(Children.begin(), Children.end(), EId));
    }

    EdgeIdTy getParent() const { return Parent; }
    const std::vector<EdgeIdTy> &getChildren() const { return Children; }

    NodeAttrs Node;
  };

  class EdgeEntryTy final {
    NodeIdTy First;
    NodeIdTy Last;

  public:
    EdgeEntryTy(NodeIdTy First, NodeIdTy Last, EdgeAttrs &&Edge)
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

    EdgeAttrs Edge;
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
  Config Cfg;

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
  Config &getAttrs() { return Cfg; }

  const Config &getAttrs() const { return Cfg; }

  void setAttrs(Config &&C) { Cfg = std::move(C); }

  NodeAttrs &getNode(NodeIdTy Id) { return Nodes.at(Id).Node; }

  const NodeAttrs &getNode(NodeIdTy Id) const { return Nodes.at(Id).Node; }

  EdgeAttrs &getEdge(EdgeIdTy Id) { return Edges.at(Id).Edge; }

  const EdgeAttrs &getEdge(EdgeIdTy Id) const { return Edges.at(Id).Edge; }

  NodeIdTy addNode(NodeAttrs &&Node) {
    NodeIdTy NId = addConstructedNode(NodeEntryTy(std::move(Node)));
    return NId;
  }

  EdgeIdTy addEdge(NodeIdTy First, NodeIdTy Last, EdgeAttrs &&Edge) {
    EdgeIdTy EId =
        addConstructedEdge(EdgeEntryTy(First, Last, std::move(Edge)));
    return EId;
  }

  NodeIdTy getEdgeNodeFirst(EdgeIdTy EId) const {
    return getEdgeEntry(EId).getFirst();
  }

  NodeIdTy getEdgeNodeLast(EdgeIdTy EId) const {
    return getEdgeEntry(EId).getLast();
  }

  EdgeIdTy getParent(NodeIdTy NId) const {
    return getNodeEntry(NId).getParent();
  }

  const std::vector<EdgeIdTy> &getChildren(NodeIdTy NId) const {
    return getNodeEntry(NId).getChildren();
  }

  void setRoot(NodeIdTy NId) { Root = NId; }

  NodeIdTy getRoot() const { return Root; }

  void removeNode(NodeIdTy NId) {
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

  void removeEdge(EdgeIdTy EId) {
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

} // namespace algo

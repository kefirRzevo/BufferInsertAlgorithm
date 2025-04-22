#include "RCGraph.h"
#include "JSON.h"
#include <string_view>
#include <unordered_map>

namespace algo {

static std::string toString(NodeKindTy Kind) {
  switch (Kind) {
  case NodeKindTy::Steiner:
    return "s";
  case NodeKindTy::Point:
    return "t";
  case NodeKindTy::Buffer:
    return "b";
  default:
    throw std::runtime_error("Unknown NodeKindTy");
  }
}

static NodeKindTy fromString(std::string_view S) {
  if (S == "s") {
    return NodeKindTy::Steiner;
  } else if (S == "t") {
    return NodeKindTy::Point;
  } else if (S == "b") {
    return NodeKindTy::Buffer;
  } else {
    throw std::runtime_error("unknown node kind");
  }
}

RCGraph readRCGraph(std::istream &IS) {
  using CoordTy = RCGraphInterface::CoordTy;
  using NodeIdTy = RCGraphInterface::NodeIdTy;
  using FloatTy = RCGraphInterface::FloatTy;
  using EdgeIdTy = RCGraphInterface::EdgeIdTy;

  RCGraph G;
  std::unordered_map<int, NodeIdTy> NodeMapping;
  std::unordered_map<int, EdgeIdTy> EdgeMapping;
  auto DataObj = nlohmann::json{};
  IS >> DataObj;
  assert(DataObj.is_object());
  assert(DataObj.contains("node"));
  auto NodeArr = DataObj["node"];
  assert(NodeArr.is_array());
  for (auto &&NodeObj : NodeArr) {
    assert(NodeObj.is_object());
    assert(NodeObj.contains("id"));
    auto NodeId = NodeObj["id"];
    assert(NodeObj.contains("x"));
    auto NodeX = NodeObj["x"];
    assert(NodeObj.contains("y"));
    auto NodeY = NodeObj["y"];
    assert(NodeObj.contains("type"));
    auto NodeTypeStr = NodeObj["type"];
    auto NodeTypeAsStr = NodeTypeStr.template get<std::string>();
    auto NodeKind = fromString(NodeTypeAsStr);
    assert(NodeObj.contains("name"));
    auto NodeName = NodeObj["name"];
    auto NodeCapacityFloat = FloatTy{};
    if (NodeObj.contains("capacitance")) {
      auto NodeCapacity = NodeObj["capacitance"];
      NodeCapacityFloat = NodeCapacity.template get<FloatTy>();
    }
    auto NodeRATFloat = FloatTy{};
    if (NodeObj.contains("rat")) {
      auto NodeRAT = NodeObj["rat"];
      NodeRATFloat = NodeRAT.template get<FloatTy>();
    }
    auto Point = PointTy{
        NodeX.template get<CoordTy>(),
        NodeY.template get<CoordTy>(),
    };
    auto Node = NodeTy{
        .Kind = NodeKind,
        .Name = NodeName.template get<std::string>(),
        .P = std::move(Point),
        .Capacity = NodeCapacityFloat,
        .RAT = NodeRATFloat,
    };
    auto NId = G.addNode(std::move(Node));
    if (NodeKind == NodeKindTy::Buffer) {
      G.setRoot(NId);
    }
    NodeMapping.emplace(NodeId, NId);
  }
  assert(DataObj.contains("edge"));
  auto EdgeArr = DataObj["edge"];
  assert(EdgeArr.is_array());
  for (auto &&EdgeObj : EdgeArr) {
    assert(EdgeObj.is_object());
    assert(EdgeObj.contains("vertices"));
    auto EdgeVerticesArr = EdgeObj["vertices"];
    assert(EdgeVerticesArr.is_array());
    assert(EdgeVerticesArr.size() == 2);
    auto FirstIdStr = EdgeVerticesArr[0];
    auto FirstIdInt = FirstIdStr.template get<int>();
    auto FirstId = NodeMapping[FirstIdInt];
    auto LastIdStr = EdgeVerticesArr[1];
    auto LastIdInt = LastIdStr.template get<int>();
    auto LastId = NodeMapping[LastIdInt];
    assert(EdgeObj.contains("segments"));
    auto EdgeSegmentsArr = EdgeObj["segments"];
    assert(EdgeSegmentsArr.is_array());
    auto Points = PointsTy{};
    for (auto &&EdgeSegmentArr : EdgeSegmentsArr) {
      assert(EdgeSegmentArr.is_array());
      assert(EdgeSegmentArr.size() == 2);
      auto XStr = EdgeSegmentArr[0];
      auto YStr = EdgeSegmentArr[1];
      auto Point = PointTy{
          XStr.template get<CoordTy>(),
          YStr.template get<CoordTy>(),
      };
      Points.push_back(std::move(Point));
    }
    auto Edge = EdgeTy{.Ps = std::move(Points)};
    G.addEdge(FirstId, LastId, std::move(Edge));
  }
  return G;
}

static void collect(const RCGraph &G, std::vector<RCGraph::NodeIdTy> &NIds,
                    std::vector<RCGraph::NodeIdTy> &EIds) {
  using NodeIdTy = RCGraphInterface::NodeIdTy;

  std::vector<NodeIdTy> CurNIds;
  NodeIdTy Root = G.getRoot();
  CurNIds.push_back(Root);
  NIds.push_back(Root);
  while (!CurNIds.empty()) {
    NodeIdTy NId = CurNIds.back();
    CurNIds.pop_back();
    const auto &Children = G.getChildren(NId);
    for (auto &&EId : Children) {
      EIds.push_back(EId);
      NodeIdTy Last = G.getEdgeNodeLast(EId);
      NIds.push_back(Last);
      CurNIds.push_back(Last);
    }
  }
}

static void printNode(const RCGraph &G, RCGraph::NodeIdTy NId,
                      std::ostream &OS) {
  using NodeIdTy = RCGraph::NodeIdTy;
  using EdgeIdTy = RCGraph::EdgeIdTy;

  auto PrintPoint = [](const PointTy &P) {
    std::string Res =
        "{" + std::to_string(P.X) + ", " + std::to_string(P.Y) + "}";
    return Res;
  };
  auto PrintPoints = [&](const PointsTy &Ps) {
    std::string Res = "[";
    for (auto &&P : Ps) {
      Res += PrintPoint(P);
    }
    Res += "]";
    return Res;
  };

  std::vector<NodeIdTy> NIds;
  std::vector<EdgeIdTy> EIds;
  collect(G, NIds, EIds);
  for (NodeIdTy NId : NIds) {
    const NodeTy &Node = G.getNode(NId);
    OS << "\tnode_" << NId << "[label = \"Id " << NId << ";Capacity "
       << Node.Capacity << ";RAT " << Node.RAT << ";P " << PrintPoint(Node.P)
       << "\"];\n";
  }
  for (EdgeIdTy EId : EIds) {
    const EdgeTy &Edge = G.getEdge(EId);
    NodeIdTy First = G.getEdgeNodeFirst(EId);
    NodeIdTy Last = G.getEdgeNodeLast(EId);
    OS << "\tnode_" << First << " -> node_" << Last << "[label = \""
       << PrintPoints(Edge.Ps) << "\"];\n";
  }
}

void dumpDot(const RCGraph &G, std::ostream &OS) {
  OS << "digraph {\n";
  OS << "\tnode[style=filled, fontcolor=black];\n";
  auto RootId = G.getRoot();
  printNode(G, RootId, OS);
  OS << "}" << std::endl;
}

void writeRCGraph(const RCGraph &G, std::ostream &OS) {
  using NodeIdTy = RCGraph::NodeIdTy;
  using EdgeIdTy = RCGraph::EdgeIdTy;

  std::vector<NodeIdTy> NIds;
  std::vector<EdgeIdTy> EIds;
  collect(G, NIds, EIds);
  auto DataObj = nlohmann::json{};
  auto &NodesArr = DataObj["node"];
  for (NodeIdTy NId : NIds) {
    const NodeTy &Node = G.getNode(NId);
    auto NodeObj = nlohmann::json{};
    NodeObj["id"] = NId;
    NodeObj["x"] = Node.P.X;
    NodeObj["y"] = Node.P.Y;
    NodeObj["type"] = toString(Node.Kind);
    NodeObj["name"] = Node.Name;
    if (Node.Kind == NodeKindTy::Point) {
      NodeObj["capacitance"] = Node.Capacity;
      NodeObj["rat"] = Node.RAT;
    }
    NodesArr.push_back(std::move(NodeObj));
  }
  auto &EdgesArr = DataObj["edge"];
  for (EdgeIdTy EId : EIds) {
    const EdgeTy &Edge = G.getEdge(EId);
    NodeIdTy First = G.getEdgeNodeFirst(EId);
    NodeIdTy Last = G.getEdgeNodeLast(EId);
    auto EdgeObj = nlohmann::json{};
    EdgeObj["id"] = EId;
    auto EdgeVerticesArr = nlohmann::json{};
    EdgeVerticesArr.push_back(First);
    EdgeVerticesArr.push_back(Last);
    EdgeObj["vertices"] = std::move(EdgeVerticesArr);
    auto EdgeSegmentsArr = nlohmann::json{};
    for (auto &&P : Edge.Ps) {
      auto EdgeSegmentArr = nlohmann::json{};
      EdgeSegmentArr.push_back(P.X);
      EdgeSegmentArr.push_back(P.Y);
      EdgeSegmentsArr.push_back(std::move(EdgeSegmentArr));
    }
    EdgeObj["segments"] = std::move(EdgeSegmentsArr);
    EdgesArr.push_back(std::move(EdgeObj));
  }
  OS << std::setw(4) << DataObj;
}
} // namespace algo

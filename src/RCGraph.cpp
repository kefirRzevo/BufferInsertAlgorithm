#include <unordered_map>
#include "RCGraph.h"
#include "JSON.h"

namespace algo {

RCGraph read(std::istream &IS) {
  using namespace algo;

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
    auto NodeKind = NodeKindTy{};
    if (NodeTypeAsStr == "s") {
      NodeKind = NodeKindTy::Steiner;
    } else if (NodeTypeAsStr == "t") {
      NodeKind = NodeKindTy::Point;
    } else if (NodeTypeAsStr == "b") {
      NodeKind = NodeKindTy::Buffer;
    } else {
      throw std::runtime_error("unknown node kind");
    }
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
        .X = NodeX.template get<CoordTy>(),
        .Y = NodeY.template get<CoordTy>(),
    };
    auto Node = NodeTy{
        .Kind = NodeKind,
        .Name = NodeName.template get<std::string>(),
        .P = std::move(Point),
        .Capacity = NodeCapacityFloat,
        .RAT = NodeRATFloat,
    };
    auto NId = G.addNode(std::move(Node));
    NodeMapping.emplace(NodeId, NId);
  }
  assert(DataObj.contains("edge"));
  auto EdgeArr = DataObj["edge"];
  assert(EdgeArr.is_array());
  for (auto && EdgeObj : EdgeArr) {
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
    auto Points = std::vector<PointTy>{};
    for (auto &&EdgeSegmentArr: EdgeSegmentsArr ) {
      assert(EdgeSegmentArr.is_array());
      assert(EdgeSegmentArr.size() == 2);
      auto XStr = EdgeSegmentArr[0];
      auto YStr = EdgeSegmentArr[1];
      auto Point = PointTy{
          .X = XStr.template get<CoordTy>(),
          .Y = YStr.template get<CoordTy>(),
      };
      Points.push_back(std::move(Point));
    }
    auto Edge = EdgeTy{
      .Ps = std::move(Points)
    };
    G.addEdge(FirstId, LastId, std::move(Edge));
  }
  return G;
}

}

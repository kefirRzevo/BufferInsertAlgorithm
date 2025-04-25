#pragma once

#include "Config.h"
#include "IRCGraph.h"

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

  bool operator==(const PointTy &rhs) const { return X == rhs.X && Y == rhs.Y; }

  unsigned distance(const PointTy &rhs) const {
    return std::abs(X - rhs.X) + std::abs(Y - rhs.Y);
  }
};

using PointsTy = std::vector<PointTy>;

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

  PointsTy Ps;
};

using RCGraphTy = RCGraph<NodeTy, EdgeTy, Config>;

RCGraphTy readRCGraph(std::istream &IS);

void dumpDot(const RCGraphTy &G, std::ostream &OS);

void writeRCGraph(const RCGraphTy &G, std::ostream &OS);

} // namespace algo

#pragma once

#include "Config.h"
#include "RCGraph.h"

#include <vector>

namespace algo {

struct CandidateTy {
  NodeTy::FloatTy Capacity;
  NodeTy::FloatTy RAT;
  PointTy P;
  EdgeTy::EdgeIdTy EId;
  bool HasBuffer;

  CandidateTy(NodeTy::FloatTy capacity, NodeTy::FloatTy rat, PointTy point,
              EdgeTy::EdgeIdTy eid, bool has_buffer)
      : Capacity{capacity}, RAT{rat}, P{point}, EId{eid}, HasBuffer{
                                                              has_buffer} {}

  friend std::ostream &operator<<(std::ostream &os,
                                  const CandidateTy &candidate) {
    os << "Buffer (" << candidate.P.X << " ," << candidate.P.Y << ")\n"
       << "\t RAT == " << candidate.RAT << "\n"
       << "\t Capacity == " << candidate.Capacity << "\n"
       << "\t EdgeId == " << candidate.EId << "\n"
       << "\t INSERT == " << candidate.HasBuffer;
    return os;
  }
};

using SolutionTy = std::vector<CandidateTy>;

SolutionTy bufferInsertion(const RCGraph &G, const Config &config,
                           unsigned step = 1);

} // namespace algo

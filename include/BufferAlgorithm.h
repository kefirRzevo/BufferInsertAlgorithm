#pragma once

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
       << "\tRAT = " << candidate.RAT << "\n"
       << "\tCapacity = " << candidate.Capacity << "\n"
       << "\tEdgeId = " << candidate.EId << "\n"
       << "\tINSERT = " << candidate.HasBuffer;
    return os;
  }
};

using SolutionTy = std::vector<CandidateTy>;

SolutionTy bufferInsertion(const RCGraphTy &G, unsigned step = 1);

} // namespace algo

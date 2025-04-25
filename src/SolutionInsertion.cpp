#include "SolutionInsertion.h"
#include "BufferAlgorithm.h"
#include "RCGraph.h"

#include <set>

using namespace algo;

enum class PointKind : bool {
  Simple,
  Buffer,
};

struct PointRecord final {
  PointKind Kind;
  PointTy Start;
  PointTy P;

  bool operator=(const PointRecord &RHS) const {
    return Start.distance(P) == RHS.Start.distance(RHS.P);
  }

  bool operator<(const PointRecord &RHS) const {
    return Start.distance(P) < RHS.Start.distance(RHS.P);
  }
};

static std::vector<PointsTy> splitPoints(const PointsTy &Points,
                                         const SolutionTy &Solutions) {
  assert(Points.size() > 1);
  assert(Solutions.size() > 0);

  auto Start = Points.front();
  std::set<PointRecord> Records;
  std::transform(Solutions.begin(), Solutions.end(),
                 std::inserter(Records, Records.begin()),
                 [&](auto &&Candidate) {
                   return PointRecord{.Kind = PointKind::Buffer,
                                      .Start = Start,
                                      .P = Candidate.P};
                 });
  for (auto &&P : Points) {
    auto Tmp = PointRecord{.Kind = PointKind::Simple, .Start = Start, .P = P};
    if (Records.find(Tmp) == Records.end()) {
      Records.insert(std::move(Tmp));
    }
  }

  std::vector<PointsTy> Res;
  auto PrevRecordIt = Records.begin();
  for (auto RecordIt = Records.begin(); RecordIt != Records.end(); ++RecordIt) {
    if (RecordIt->Kind == PointKind::Buffer) {
      PointsTy CurPoints;
      std::transform(PrevRecordIt, std::next(RecordIt),
                     std::back_inserter(CurPoints),
                     [&](auto &&RecIt) { return RecIt.P; });
      Res.emplace_back(std::move(CurPoints));
      PrevRecordIt = RecordIt;
    }
  };
  PointsTy CurPoints;
  std::transform(PrevRecordIt, Records.end(), std::back_inserter(CurPoints),
                 [&](auto &&RecIt) { return RecIt.P; });
  Res.emplace_back(std::move(CurPoints));
  return Res;
}

namespace algo {

void insertSolution(const SolutionTy &Solution, RCGraphTy &G) {
  using NodeIdTy = RCGraphTy::NodeIdTy;
  using EdgeIdTy = RCGraphTy::EdgeIdTy;

  std::unordered_map<EdgeIdTy, SolutionTy> Grouped;
  for (auto &&S : Solution) {
    auto &Group = Grouped[S.EId];
    Group.push_back(S);
  }

  for (auto &&[EId, Sols] : Grouped) {
    // Sorting solution
    const auto &Edge = G.getEdge(EId);
    auto Start = Edge.Ps.front();
    auto Solutions = Sols;
    std::sort(Solutions.begin(), Solutions.end(), [&](auto &&lhs, auto &&rhs) {
      return Start.distance(lhs.P) < Start.distance(rhs.P);
    });

    // Getting edge's points
    auto First = G.getEdgeNodeFirst(EId);
    auto Last = G.getEdgeNodeLast(EId);
    std::vector<PointsTy> SplittedEdgesPs = splitPoints(Edge.Ps, Solutions);

    // Fixing nodes
    std::vector<NodeIdTy> Nodes;
    Nodes.push_back(First);
    const Module &M = G.getAttrs().getModule(ModuleKind::Buffer);
    for (auto &&Solution : Solutions) {
      auto Node = NodeTy{.Kind = NodeKindTy::Buffer,
                         .Name = M.Name,
                         .P = Solution.P,
                         .Capacity = Solution.Capacity,
                         .RAT = Solution.RAT};
      auto NId = G.addNode(std::move(Node));
      Nodes.push_back(NId);
    }
    Nodes.push_back(Last);

    // Fixing edges
    G.removeEdge(EId);
    for (size_t Idx = 0; Idx != Nodes.size() - 1; ++Idx) {
      auto NodeFirst = Nodes[Idx];
      auto NodeLast = Nodes[Idx + 1];
      auto EdgePts = SplittedEdgesPs[Idx];
      G.addEdge(NodeFirst, NodeLast, EdgeTy{.Ps = EdgePts});
    }
  }
}

} // namespace algo

#include "BufferAlgorithm.h"

#include <unordered_set>

using namespace algo;

#if DEBUG

#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_NODE(node, solutions)                                              \
  do {                                                                         \
    auto best_solution =                                                       \
        std::max_element(solutions.begin(), solutions.end(),                   \
                         [](const auto &lhs, const auto &rhs) {                \
                           return lhs.back().RAT < rhs.back().RAT;             \
                         });                                                   \
    LOG("[DEBUG] Visiting Node %s (%d, %d):\n\tOptimal RAT = %lf\n\tCapacity " \
        "= %lf\n\n",                                                           \
        node.Name.c_str(), node.P.X, node.P.Y, best_solution->back().RAT,      \
        best_solution->back().Capacity);                                       \
  } while (false)

#else

#define LOG(...)
#define LOG_NODE(nodeid, solutions)

#endif

static PointsTy splitEdge(const EdgeTy &edge, unsigned step) {
  auto points = edge.Ps;
  if (points.front() == points.back())
    return {};

  PointsTy candidates;

  auto lhs_it = points.rbegin();
  auto rhs_it = std::next(points.rbegin());

  while (rhs_it != points.rend()) {
    if (lhs_it->X != rhs_it->X) {
      int adjustment = (rhs_it->X > lhs_it->X) ? step : -static_cast<int>(step);
      for (PointTy::CoordTy cnt = 1; cnt < (rhs_it->X - lhs_it->X) / adjustment;
           ++cnt)
        candidates.emplace_back(lhs_it->X + cnt * adjustment, lhs_it->Y);
    } else {
      int adjustment = (rhs_it->Y > lhs_it->Y) ? step : -static_cast<int>(step);
      for (PointTy::CoordTy cnt = 1; cnt < (rhs_it->Y - lhs_it->Y) / adjustment;
           ++cnt)
        candidates.emplace_back(lhs_it->X, lhs_it->Y + cnt * adjustment);
    }
    ++lhs_it;
    ++rhs_it;
  }

  candidates.push_back(*lhs_it);
  return candidates;
}

static void insert(SolutionTy &solution, unsigned length, PointTy position,
                   EdgeTy::EdgeIdTy eid, const RCGraphTy G) {
  auto &last_candidate = solution.back();
  auto rat = last_candidate.RAT;
  auto capacity = last_candidate.Capacity;

  Technology wire = G.getAttrs().getTechnology();

  NodeTy::FloatTy wire_delay =
      (wire.UnitR * wire.UnitC * (length * length)) / 2 +
      wire.UnitR * length * capacity;
  NodeTy::FloatTy new_rat = rat - wire_delay;
  NodeTy::FloatTy new_capacity = capacity + wire.UnitC * length;

  //  LOG("WIRE Insertion:\n\tRAT: %lf -> %lf\n\tCapacity: %lf -> %lf\n\n", rat,
  //  new_rat, capacity, new_capacity);

  solution.emplace_back(new_capacity, new_rat, position, eid,
                        /*HasBuffer=*/false);
}

void insert(SolutionTy &solution, const RCGraphTy G) {
  auto &last_candidate = solution.back();

  Module buffer = G.getAttrs().getModule(ModuleKind::Buffer);

  NodeTy::FloatTy buffer_delay = buffer.K + buffer.R * last_candidate.Capacity;
  last_candidate.RAT -= buffer_delay;
  last_candidate.Capacity = buffer.C;
  last_candidate.HasBuffer = true;

  //  LOG("BUFFER Insertion:\n\tRAT: %lf -> %lf\n\tCapacity: %lf -> %lf\n\n",
  //  rat, last_candidate.RAT, capacity, last_candidate.Capacity);
}

static std::vector<SolutionTy>
redundancy_elimination(std::vector<SolutionTy> &&solutions) {
  auto isRedundant = [](const SolutionTy &validator,
                        const SolutionTy &solution) {
    auto validator_candidate = validator.back();
    auto solution_candidate = solution.back();
    return validator_candidate.RAT >= solution_candidate.RAT &&
           validator_candidate.Capacity <= solution_candidate.Capacity;
  };

  unsigned id = 0;
  std::vector<std::pair<unsigned, SolutionTy>> identified_solutions;
  std::transform(
      solutions.begin(), solutions.end(),
      std::back_inserter(identified_solutions),
      [&id](const auto &solution) -> std::pair<unsigned, SolutionTy> {
        return {id++, solution};
      });

  std::unordered_set<unsigned> redundant_solutions;
  for (auto lhs_it = identified_solutions.begin();
       lhs_it != std::prev(identified_solutions.end()); ++lhs_it) {
    for (auto rhs_it = std::next(lhs_it); rhs_it != identified_solutions.end();
         ++rhs_it) {
      if (isRedundant(lhs_it->second, rhs_it->second))
        redundant_solutions.insert(rhs_it->first);
      else if (isRedundant(rhs_it->second, lhs_it->second))
        redundant_solutions.insert(lhs_it->first);
    }
  }

  identified_solutions.erase(
      std::remove_if(identified_solutions.begin(), identified_solutions.end(),
                     [&redundant_solutions](const auto &identified_solution) {
                       return redundant_solutions.find(
                                  identified_solution.first) !=
                              redundant_solutions.end();
                     }),
      identified_solutions.end());

  std::vector<SolutionTy> pruned_solutions;
  std::transform(identified_solutions.begin(), identified_solutions.end(),
                 std::back_inserter(pruned_solutions),
                 [](const auto &identified_solution) {
                   return identified_solution.second;
                 });
  return pruned_solutions;
}

static std::vector<SolutionTy>
mergeTwoSolutions(const std::vector<SolutionTy> &lhs,
                  const std::vector<SolutionTy> &rhs, PointTy position) {
  std::vector<SolutionTy> solutions;
  for (auto &lhs_solution : lhs) {
    for (auto &rhs_solution : rhs) {
      auto &lhs_candidate = lhs_solution.back();
      auto &rhs_candidate = rhs_solution.back();

      SolutionTy solution;
      std::copy(lhs_solution.begin(), lhs_solution.end(),
                std::back_inserter(solution));
      std::copy(rhs_solution.begin(), rhs_solution.end(),
                std::back_inserter(solution));

      solution.emplace_back(lhs_candidate.Capacity + rhs_candidate.Capacity,
                            std::min(lhs_candidate.RAT, rhs_candidate.RAT),
                            position, RCGraphTy::invalidEdgeId(), false);
      solutions.push_back(solution);
    }
  }
  return solutions;
}

static std::vector<SolutionTy>
mergeSolutions(const std::vector<std::vector<SolutionTy>> &children_solutions,
               const NodeTy &node) {
  if (node.Kind == NodeKindTy::Point) {
    assert(children_solutions.empty());
    return std::vector<SolutionTy>{SolutionTy{
        {node.Capacity, node.RAT, node.P, RCGraphTy::invalidEdgeId(), false}}};
  }

  assert(std::all_of(children_solutions.begin(), children_solutions.end(),
                     [](const auto &solution) { return !solution.empty(); }));

  if (children_solutions.size() == 1)
    return children_solutions.front();

  if (children_solutions.size() == 2)
    return mergeTwoSolutions(children_solutions.front(),
                             children_solutions.back(), node.P);

  std::vector<SolutionTy> solutions;
  for (auto current_child = std::prev(children_solutions.end());
       current_child != children_solutions.begin(); --current_child) {
    std::vector<SolutionTy> current_solutions = *current_child;

    std::vector<SolutionTy> other_solutions;
    std::for_each(children_solutions.begin(), current_child,
                  [&other_solutions](const auto &solutions) {
                    std::copy(solutions.begin(), solutions.end(),
                              std::back_inserter(other_solutions));
                  });

    auto solution =
        mergeTwoSolutions(current_solutions, other_solutions, node.P);
    std::move(solution.begin(), solution.end(), std::back_inserter(solutions));
  }
  return solutions;
}

namespace algo {

SolutionTy bufferInsertion(const RCGraphTy &G, unsigned step) {
  std::vector<NodeTy::NodeIdTy> backtrack{G.getRoot()};
  std::unordered_map<NodeTy::NodeIdTy, std::vector<SolutionTy>> visited{
      {RCGraphTy::invalidNodeId(), {}}};

  while (!backtrack.empty()) {
    auto top = backtrack.back();

    std::vector<NodeTy::NodeIdTy> children;
    std::transform(G.getChildren(top).begin(), G.getChildren(top).end(),
                   std::back_inserter(children),
                   [&G](const EdgeTy::EdgeIdTy edge_id) {
                     return G.getEdgeNodeLast(edge_id);
                   });

    std::vector<std::vector<SolutionTy>> children_solutions;
    for (auto child : children) {
      auto solution_it = visited.find(child);
      if (solution_it == visited.end())
        backtrack.push_back(child);
      else
        children_solutions.push_back(solution_it->second);
    }

    if (children_solutions.size() < children.size())
      continue;

    auto solutions = mergeSolutions(children_solutions, G.getNode(top));
    solutions = redundancy_elimination(std::move(solutions));

    LOG_NODE(G.getNode(top), solutions);

    if (top == G.getRoot()) {
      for (auto &solution : solutions) {
        insert(solution, G);
        solution.back().HasBuffer = false;
      }
      visited[top] = solutions;

      backtrack.pop_back();
      assert(backtrack.empty());

      continue;
    }

    EdgeTy::EdgeIdTy edge_id = G.getParent(top);
    PointsTy points = splitEdge(G.getEdge(edge_id), step);

    for (auto &point : points) {
      auto last_candidate = solutions.back().back();
      unsigned length = last_candidate.P.distance(point);
      for (auto &solution : solutions)
        insert(solution, length, point, edge_id, G);

      solutions = redundancy_elimination(std::move(solutions));

      auto copy_solutions = solutions;
      for (auto &copy_solution : copy_solutions)
        insert(copy_solution, G);

      std::move(copy_solutions.begin(), copy_solutions.end(),
                std::back_inserter(solutions));
      solutions = redundancy_elimination(std::move(solutions));
    }

    visited[top] = solutions;
    backtrack.pop_back();
  }
  /*
  for (auto &[id, solutions] : visited) {
    std::cout << "NODE = " << id << std::endl;
    for (auto &solution : solutions) {
      std::cout << "Solution" << std::endl;
      for (auto candidate : solution)
        std::cout << candidate << "\n";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
*/
  std::vector<SolutionTy> solutions = visited[G.getRoot()];

  auto best_solution = std::max_element(
      solutions.begin(), solutions.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.back().RAT < rhs.back().RAT;
      });
  return *best_solution;
}

} // namespace algo


#include "BufferAlgorithm.h"
#include "Config.h"
#include "RCGraph.h"
#include "SolutionInsertion.h"

#include <filesystem>
#include <fstream>
#include <chrono>

using namespace algo;

static std::string getOutputFilePath(std::string_view Input) {
  namespace fs = std::filesystem;

  auto InputPath = fs::path{Input};
  auto Stem = std::string(InputPath.stem());
  return fs::current_path() / (Stem + "_out.json");
}

static SolutionTy extractSolution(const SolutionTy &Candidates) {
  auto Solution = SolutionTy{};
  std::copy_if(Candidates.begin(), Candidates.end(),
               std::back_inserter(Solution),
               [](auto &&Candidate) { return Candidate.HasBuffer; });
  return Solution;
}

static NodeTy::FloatTy resultingRAT(const SolutionTy &Solution) {
  return Solution.back().RAT;
}

int main(int argc, const char *argv[]) {
  using namespace std::chrono;

  try {
    if (argc != 3) {
      throw std::runtime_error("Usage: " + std::string(argv[0]) +
                               " <technology_file_name>.json <test_name>.json");
    }
    std::string TechFile = argv[1];
    std::ifstream CfgIS{TechFile};
    auto Cfg = readConfig(CfgIS);
    std::string TestFile = argv[2];
    std::ifstream TestIS{TestFile};
    auto G = readRCGraph(TestIS);
    auto start = high_resolution_clock::now();
    auto Candidates = bufferInsertion(G, Cfg);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    auto Solution = extractSolution(Candidates);
    for (auto &&Candidate : Solution) {
      std::cout << Candidate << "\n";
    }
    std::cout << std::endl;
    NodeTy::FloatTy RAT = resultingRAT(Candidates);
    std::cout << "Resulting RAT = " << RAT << std::endl;
    std::cout << "Resulting AlgoTime = " << duration.count() << std::endl;

    insertSolution(Solution, G);
    auto OutputPath = getOutputFilePath(TestFile);
    std::ofstream OS{OutputPath};
    writeRCGraph(G, OS);
    return 0;
  } catch (const std::exception &E) {
    std::cerr << E.what() << std::endl;
    return 1;
  }
}

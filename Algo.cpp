
#include "Config.h"
#include "RCGraph.h"
#include <fstream>
#include <filesystem>

#include "BufferAlgorithm.h"

using namespace algo;

static std::string getOutputFilePath(std::string_view Input) {
  namespace fs = std::filesystem;

  auto InputPath = fs::path{Input};
  auto Stem = std::string(InputPath.stem());
  return fs::current_path() / (Stem + "_out.json");
}

int main(int argc, const char *argv[]) {
  try {
    if (argc != 3) {
      throw std::runtime_error("Usage: " + std::string(argv[0]) +
                               " <technology_file_name>.json <test_name>.json");
    }
    std::string TechFile = argv[1];
    std::ifstream CfgIS{TechFile};
    Config Cfg;
    Cfg.read(CfgIS);
    std::string TestFile = argv[2];
    std::ifstream TestIS{TestFile};
    auto G = read(TestIS);
    //std::ofstream GOs{"a.dot"};
    //dumpDot(G, GOs);
    auto OutputPath = getOutputFilePath(TestFile);
    std::ofstream OS{OutputPath};
    write(G, OS);

    auto solution = BufferInsertion(G, Cfg);
    for (auto &candidate : solution)
      std::cout << candidate << "\n";
    std::cout << std::endl;
    
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

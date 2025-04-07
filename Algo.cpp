
#include <fstream>
#include "Config.h"
#include "RCGraph.h"

using namespace algo;

int main(int argc, const char *argv[]) {
  try {
    std::ifstream CfgIs{"tests/tech1.json"};
    Config Cfg;
    Cfg.read(CfgIs);
    std::ifstream TopIs{"tests/test01.json"};
    auto G = read(TopIs);
    std::ofstream GOs{"a.dot"};
    G.dump(GOs);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}


#include <fstream>
#include "Config.h"
#include "RCGraph.h"

using namespace algo;

int main(int argc, const char *argv[]) {
  try {
    std::ifstream CfgIs{"tests/tech1.json"};
    Config Cfg;
    Cfg.read(CfgIs);
    std::ifstream TopIs{"tests/tech1.json"};
    RCGraph G;
    G.read(TopIs);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

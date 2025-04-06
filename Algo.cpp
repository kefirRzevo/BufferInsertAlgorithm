
#include <fstream>
#include "Config.h"

using namespace algo;

int main(int argc, const char *argv[]) {
  try {
    std::ifstream Is{"tests/tech1.json"};
    Config Cfg;
    Cfg.read(Is);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

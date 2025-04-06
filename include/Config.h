#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace algo {

enum class ModuleKind {
  Buffer,
};

struct Module {
  using FloatTy = float;

  ModuleKind Kind;
  std::string Name;
  FloatTy R;
  FloatTy C;
  FloatTy K;
};

struct Technology final {
  using FloatTy = float;

  FloatTy UnitWireResistance;
  FloatTy UnitWireCapacitance;
  std::string UnitWireResistanceComment;
  std::string UnitWireCapacitanceComment;
};

class Config final {
  std::unordered_map<ModuleKind, Module> Modules;
  Technology Tech;

public:
  void read(std::istream &Is);

  const Technology &getTechnology() const { return Tech; }

  const Module &getModule(ModuleKind Kind) const {
    auto Found = Modules.find(Kind);
    if (Found == Modules.end()) {
      throw std::runtime_error("there is no such Module");
    }
    return Found->second;
  }
};

} // namespace algo

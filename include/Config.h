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

  FloatTy UnitR;
  FloatTy UnitC;
  std::string UnitRComment;
  std::string UnitCComment;
};

class Config final {
  std::unordered_map<ModuleKind, Module> Modules;
  Technology Tech;

public:
  void setTechnology(Technology &&T) { Tech = std::move(T); }

  const Technology &getTechnology() const { return Tech; }

  void addModule(ModuleKind Kind, Module &&M) {
    Modules.emplace(Kind, std::move(M));
  }

  const Module &getModule(ModuleKind Kind) const {
    auto Found = Modules.find(Kind);
    if (Found == Modules.end()) {
      throw std::runtime_error("there is no such Module");
    }
    return Found->second;
  }
};

Config readConfig(std::istream &Is);

} // namespace algo

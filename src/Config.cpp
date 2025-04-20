#include "Config.h"
#include "JSON.h"

using namespace algo;

void Config::read(std::istream &Is) {
  auto DataObj = nlohmann::json{};
  Is >> DataObj;
  assert(DataObj.is_object());
  assert(DataObj.contains("module"));
  auto ModuleArr = DataObj["module"];
  assert(ModuleArr.is_array());
  assert(ModuleArr.size() == 1);
  auto ModuleObj = ModuleArr.at(0);
  assert(ModuleObj.is_object());
  assert(ModuleObj.contains("name"));
  auto Kind = ModuleKind::Buffer;
  auto NameStr = ModuleObj["name"];
  assert(ModuleObj.contains("input"));
  auto InputArr = ModuleObj["input"];
  assert(InputArr.is_array());
  assert(InputArr.size() == 1);
  auto InputObj = InputArr.at(0);
  assert(InputObj.is_object());
  assert(InputObj.contains("C"));
  auto CFloat = InputObj["C"];
  assert(InputObj.contains("R"));
  auto RFloat = InputObj["R"];
  assert(InputObj.contains("intrinsic_delay"));
  auto KFloat = InputObj["intrinsic_delay"];
  auto Mod = Module{
      .Kind = Kind,
      .Name = NameStr.template get<std::string>(),
      .R = RFloat.template get<Module::FloatTy>(),
      .C = CFloat.template get<Module::FloatTy>(),
      .K = KFloat.template get<Module::FloatTy>(),
  };
  Modules.emplace(Kind, std::move(Mod));
  assert(DataObj.contains("technology"));
  auto TechObj = DataObj["technology"];
  assert(TechObj.is_object());
  assert(TechObj.contains("unit_wire_resistance"));
  auto UnitWireRFloat = TechObj["unit_wire_resistance"];
  assert(TechObj.contains("unit_wire_resistance_comment0"));
  auto UnitWireRCommentStr = TechObj["unit_wire_resistance_comment0"];
  assert(TechObj.contains("unit_wire_capacitance"));
  auto UnitWireCFloat = TechObj["unit_wire_capacitance"];
  assert(TechObj.contains("unit_wire_capacitance_comment0"));
  auto UnitWireCCommentStr = TechObj["unit_wire_capacitance_comment0"];
  Tech = Technology{
      .UnitR = UnitWireRFloat.template get<Technology::FloatTy>(),
      .UnitC = UnitWireCFloat.template get<Technology::FloatTy>(),
      .UnitRComment = UnitWireCCommentStr.template get<std::string>(),
      .UnitCComment = UnitWireRCommentStr.template get<std::string>(),
  };
}

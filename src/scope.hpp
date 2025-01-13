/**
 * @file translation_unit.hpp
 * @brief
 */

#pragma once

#include "type.hpp"
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>

/**
 * @struct
 * @brief
 */
struct Scope {
  struct StructInfo {
    std::map<std::string, Type> members;
    std::list<std::string> order;
  };
  struct EnumInfo {
    std::map<std::string, Type> members;
    std::list<std::string> order;
    std::string default_value;
  };

  std::map<std::string,
           std::variant<StructInfo, EnumInfo, Type>>
      symbols;
  std::list<std::string> order;

  std::optional<std::shared_ptr<Scope>> parent;
};

/**
 * @brief All the information needed to translate targeting `C`
 */
class TranslationUnit {
public:
protected:
};

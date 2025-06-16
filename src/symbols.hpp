#pragma once

#include "ast_node.hpp"
#include "macro.hpp"
#include "templates.hpp"
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <variant>

/// Information about a single function. The type should be
/// unique.
struct FnInfo {
  /// Pragma-like tags
  std::map<std::string, std::string> tags;

  /// The function's birth name (might be different from the
  /// key used to find it!)
  std::string name;

  /// The fn's FULL type (not just return type)
  Type t;

  /// An AST node defining the behaviour
  ASTNodes::Statement n;
};

/// Information about a single struct definition.
class StructInfo {
public:
  /// Pragma-like tags
  std::map<std::string, std::string> tags;

  /// The order of the members, since that matters
  std::list<std::string> member_order;

  /// The types of the members
  std::map<std::string, Type> members;

  /// Returns a constructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_constructor(const std::string &_self_name) const;

  /// Returns a destructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_destructor(const std::string &_self_name) const;
};

/// Information about a single enum definition
class EnumInfo {
public:
  /// Pragma-like tags (EG "casual")
  std::map<std::string, std::string> tags;

  /// The order of the options, since this matters
  std::list<std::string> option_order;

  /// The types of the options
  std::map<std::string, Type> options;

  /// Returns a constructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_constructor(const std::string &_self_name) const;

  /// Returns a destructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_destructor(const std::string &_self_name) const;
};

/// Manages the pushing and popping of variables, types,
/// templates, macros, etc.
class ScopeManager {
public:
  /// Adds a single empty frame by default
  ScopeManager();

  /// A single value in the lookup table. `Type` is for variable
  /// instances.
  using Value =
      std::variant<std::list<FnInfo>, StructInfo, EnumInfo,
                   TemplateInfo, MacroInfo, Type>;

  /// An entry that is not a function
  using NonFnValue =
      std::variant<StructInfo, EnumInfo, TemplateInfo,
                   MacroInfo, Type>;

  /// Pushes a scope onto the stack
  void push_frame() noexcept;

  /// Pops a scope off the stack and returns destructors
  ASTNodes::Statement pop_frame();

  /// Returns whether there are no frames remaining
  bool empty() const noexcept;

  /// Inserts a (non-function) value into the current scope,
  /// throwing if duplicate
  void add(const std::string &_key, const NonFnValue &_value);

  /// Inserts a (function) value into the current scope
  void add(const std::string &_key,
           const FnInfo &_value) noexcept;

  /// Resolve _thing_that_exists, then add an entry pointing to
  /// it. Note that aliases are not destructed.
  void alias(const std::string &_name_of_alias,
             const std::string &_thing_that_exists);

  /// Alias-es all items with some prefix to not have that
  /// prefix
  void remove_prefix(const std::string &_prefix);

  /// Find the closest instance and resolve aliases
  std::optional<std::reference_wrapper<Value>>
  get(const std::string &_name) const noexcept;

protected:
  ///
  using ValueOrAlias =
      std::variant<Value, std::reference_wrapper<Value>>;

  /// Resolves aliases
  static std::reference_wrapper<Value>
  dealias(ValueOrAlias &_what);

  /// Back is most recent
  std::list<std::map<std::string, ValueOrAlias>> frames;
};

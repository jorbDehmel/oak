/**
 * @file
 * @brief Scope manager object and all the variants it may
 * contain
 */

#pragma once

#include "ast_node.hpp"
#include "lexer.hpp"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <variant>

/// Information about a single function. The type should be
/// unique.
struct FnInfo {
  /// Pragma-like tags
  std::map<std::string, std::string> tags;

  /// The function's birth name (might be different from the
  /// key used to find it!). Unmangled
  std::string name;

  /// The fn's FULL type (not just return type)
  Type t;

  /// An AST node defining the behaviour
  ASTNode n;
};

/// Information about a single struct definition.
class StructInfo {
public:
  /// The birth name (might be different from the key used to
  /// find it!)
  std::string name;

  /// Pragma-like tags
  std::map<std::string, std::string> tags;

  /// The order of the members, since that matters
  std::list<std::string> member_order;

  /// The types of the members
  std::map<std::string, Type> members;
};

/// Information about a single enum definition
class EnumInfo {
public:
  /// The birth name (might be different from the key used
  /// to find it!)
  std::string name;

  /// Pragma-like tags (EG "casual")
  std::map<std::string, std::string> tags;

  /// The order of the options, since this matters
  std::list<std::string> option_order;

  /// The types of the options
  std::map<std::string, Type> options;
};

/// Information about a single template block
class TemplateInfo {
public:
  /// The values to substitute in place of generics
  using Substitution = std::list<std::list<std::string>>;

  /// Initialize with the info needed to reconstruct (since
  /// instantiated templates take the region of their source,
  /// not their instantiator)
  TemplateInfo(const std::filesystem::path &_p,
               const uint64_t &_l, const uint64_t &_c,
               const std::string &_name)
      : path(_p), line(_l), col(_c), name(_name) {
  }

  TemplateInfo(const TemplateInfo &_other) = default;

  /// The filepath it came from
  const std::filesystem::path path;

  /// The line it came from
  const uint64_t line;

  /// The column it came from
  const uint64_t col;

  /// The birth name
  const std::string name;

  /// Returns a list of tokens based on _to_augment wherein
  /// all occurrences of generics are replaced with their
  /// corresponding replacements
  static std::list<Token>
  replace(const std::list<Token> &_to_augment,
          const std::list<std::string> &_generics,
          const Substitution &_replacements);

  /// The things to replace
  std::list<std::string> generics;

  /// The body of the template
  std::list<Token> instantiate_block;
};

/**
 * @brief Holds information pertaining to inline/alias
 * macros (EG LINE!, FILE!, etc)
 */
struct InlineMacro {
  /// The thing the macro should be replaced with
  std::list<Token> contents;
};

/**
 * @brief Holds information for compiled (EG assert!(...))
 * macros
 */
struct CompiledMacro {
  /// The path to the compiled macro
  std::filesystem::path executable;
};

/**
 * @class Rule
 * @brief An abstract rule, independent of engine
 */
class Rule {
public:
  /// Rules that must be done first: Externally handled
  const std::list<std::string> prereqs;
};

/**
 * @brief Given some information about a fn call, construct a
 * printable string.
 * @param _name The name of the function
 * @param _args The types of the arguments
 * @returns A string representing the fn call
 */
std::string fn_call_str(const std::string &_name,
                        const std::list<ASTNode> &_args);

/// Manages the pushing and popping of variables, types,
/// templates, macros, etc.
class ScopeManager {
public:
  /// Adds a single empty frame by default
  ScopeManager();

  /// The results served by a fn query
  using FnValue = std::list<FnInfo>;

  /// A list of overloadable template entries
  using TemplValue = std::list<TemplateInfo>;

  /// A single value in the lookup table. `Type` is for
  /// variable instances.
  using Value =
      std::variant<FnValue, TemplValue, StructInfo, EnumInfo,
                   InlineMacro, CompiledMacro, Type>;

  /// An entry that is not type-overloadable (not
  /// fns/templates)
  using SingularValue =
      std::variant<StructInfo, EnumInfo, InlineMacro,
                   CompiledMacro, Type>;

  /// Pushes a scope onto the stack
  void push_frame() noexcept;

  /// Pushes a new frame. Trying to access anything before this
  /// frame will be flagged as a capture.
  void push_capture_frame() noexcept;

  /// Given that this frame is a capture frame, returns any
  /// captures which have been caught
  std::list<std::string> get_captures() const noexcept;

  /// Pops a scope off the stack and returns a list of OBJECTS
  /// to destroy
  std::list<ASTNode> pop_frame();

  /// Returns whether there are no frames remaining
  bool empty() const noexcept;

  /// Pushes a namespace prefix which will be prepended to all
  /// additions
  void push_prefix(const std::string &_prefix);

  /// Pops the most recent prefix
  void pop_prefix();

  /// Adds all the prefixes which have been pushed to some raw
  /// name. This is used internally upon addition
  std::string add_prefix(const std::string &_raw_name) const;

  /// Inserts a (non-function) value into the current scope,
  /// throwing if duplicate
  void add(const std::string &_key,
           const SingularValue &_value);

  /// Inserts a (function) value into the current scope
  void add(const std::string &_key, const FnInfo &_value);

  /// Inserts a (template) value into the current scope
  void add(const std::string &_key, const TemplateInfo &_value);

  /// Resolve _thing_that_exists, then add an entry pointing
  /// to it. Note that aliases are not destructed.
  void alias(const std::string &_name_of_alias,
             const std::string &_thing_that_exists);

  /// Alias-es all items with some prefix to not have that
  /// prefix
  void remove_prefix(const std::string &_prefix);

  /// Returns whether the given name can be found
  bool contains(const std::string &_name) const noexcept;

  /// Returns true iff the given name is DIRECTLY associated
  /// with an atomic type (struct/enum). Note that an atomic
  /// type may be shadowed by a non-atomic-type in a closer
  /// scope, in which case this returns false.
  bool
  contains_atomic_type(const std::string &_name) const noexcept;

  /// Find the closest instance and resolve aliases
  std::optional<Value> get(const std::string &_name) noexcept;

  /// Find the closest instance, resolve aliases, and murder it
  void erase(const std::string &_name) noexcept;

  /// Retrieves the fn closest to the given call spec WITHOUT
  /// doing any templates (we don't have a parser!)
  ASTNode get_fn(const std::string &_name,
                 const std::list<ASTNode> &_args);

  /// Drops any fn/templates with given name, type, key, and
  /// value
  void drop_fn_with_tag(const std::string &_name,
                        const Type &_to_match,
                        const std::string &_key,
                        const std::string &_value) noexcept;

  /// Gets all valid names
  std::set<std::string> names() const noexcept;

  /// Dumps everything
  void dump(std::ostream &_into) const noexcept;

  /// Finds the given value, asserts that it exists, asserts
  /// that it is the right type (the template parameter) and
  /// returns a COPY of it. This is not mutable because it is
  /// meant to be an external function, and references to
  /// variants get weird.
  template <typename T> inline T at(const std::string &_name) {
    const auto gotten = get(_name);
    if (!gotten.has_value() ||
        !std::holds_alternative<T>(gotten.value())) {
      throw std::runtime_error(
          "Unexpected meta-type for symbol '" + _name + "'");
    }
    return std::get<T>(gotten.value());
  }

  /// Used at reconstruction
  std::list<std::variant<FnInfo, StructInfo, EnumInfo>>
      in_order;

  /// A value or an alias to one: Used internally
  using ValueOrAlias = std::variant<Value, std::string>;

  /// Resolves aliases. Try not to use externally.
  Value dealias(ValueOrAlias &_what);

  /// A stack of frames: Back is most recent
  std::list<std::map<std::string, ValueOrAlias>> frames;

  /// The namespaces to prepend upon addition
  std::list<std::string> prefixes;

  /// Either the empty option (not a capture frame) or a list of
  /// all the variables which had to be located from above this
  /// frame.
  std::list<std::optional<std::list<std::string>>>
      barrier_captures;
};

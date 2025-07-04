/**
 * @file
 * @brief Scope manager object and all the variants it may
 * contain
 */

#pragma once

#include "ast_node.hpp"
#include "lexer.hpp"
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <memory>
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
  /// The birth name (might be different from the key used to
  /// find it!)
  std::string name;

  /// Pragma-like tags
  std::map<std::string, std::string> tags;

  /// The order of the members, since that matters
  std::list<std::string> member_order;

  /// The types of the members
  std::map<std::string, Type> members;

  /// Returns a constructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_default_constructor(const std::string &_self_name) const;

  /// Returns a destructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_default_destructor(const std::string &_self_name) const;
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

  /// Returns a constructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_default_constructor(const std::string &_self_name) const;

  /// Returns a destructor definition (struct-name-agnostic)
  ASTNodes::Statement
  get_default_destructor(const std::string &_self_name) const;
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
               const uint64_t &_l, const uint64_t &_c)
      : path(_p), line(_l), col(_c) {
  }

  /// The filepath it came from
  const std::filesystem::path path;

  /// The line it came from
  const uint64_t line;

  /// The column it came from
  const uint64_t col;

  /// Given some name and signature, return the generics which
  /// would satisfy them (if such substitutions exist)
  std::optional<Substitution>
  find_substitutions(const std::string &_name,
                     const std::list<std::string>
                         &_signature_to_provide) const;

  /// Returns whether the given substitutions would cause the
  /// `provides` list to match the given list
  bool
  does_provide(const Substitution &_substitutions,
               const std::list<std::string> &_desired) const;

  /// Returns a list of tokens based on _to_augment wherein
  /// all occurrences of generics are replaced with their
  /// corresponding replacements
  static std::list<std::string>
  replace(const std::list<std::string> &_to_augment,
          const std::list<std::string> &_generics,
          const Substitution &_replacements);

  /// The things to replace
  std::list<std::string> generics;

  /// "Sample" of body used for auto-instantiation.
  /// "enum"
  /// "struct"
  /// "( whatever : T )"
  std::list<std::string> provides_block;

  /// Run beforehand: If fail, no error
  std::list<std::string> validate_block;

  /// Run if valid
  std::list<std::string> instantiate_block;

  /// Instances which already exist
  std::set<Substitution> existing_instances;
};

/**
 * @brief Holds information pertaining to inline/alias
 * macros (EG LINE!, FILE!, etc)
 */
struct InlineMacro {
  /// The thing the macro should be replaced with
  std::list<Lexer::Token> contents;
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
  /// If returns false, write the input. Else if output_size
  /// is 0 (default), don't write anything. If output_size is
  /// nonzero, put the first `output_size` items of `output`.
  using DeltaFn = bool (*)(::TokenList input[],
                           ::TokenList output[],
                           uint *output_size);

  /// The delta function for the quasi-FST, probably externally
  /// loaded
  const DeltaFn delta_fn;

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
std::string
fn_call_str(const std::string &_name,
            const std::list<ASTNodes::Object> &_args);

/// Manages the pushing and popping of variables, types,
/// templates, macros, etc.
class ScopeManager {
public:
  /// Adds a single empty frame by default
  ScopeManager();

  /// The results served by a fn query
  using FnValue = std::list<
      std::variant<FnInfo, std::shared_ptr<TemplateInfo>>>;

  /// A single value in the lookup table. `Type` is for
  /// variable instances.
  using Value = std::variant<FnValue, StructInfo, EnumInfo,
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

  /// Pops a scope off the stack and returns destructors
  ASTNodes::Statement pop_frame();

  /// Returns whether there are no frames remaining
  bool empty() const noexcept;

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

  /// Retrieves the fn closest to the given call spec WITHOUT
  /// doing any templates (we don't have a parser!)
  ASTNodes::Call get_fn(const std::string &_name,
                        const std::list<ASTNodes::Node> &_args);

  /// Drops any fn/templates with given name, type, key, and
  /// value
  void drop_fn_with_tag(const std::string &_name,
                        const Type &_to_match,
                        const std::string &_key,
                        const std::string &_value) noexcept;

  /// Marks the most recent fn entry of some name with some tag
  /// info
  void tag_fn(const std::string &_name, const std::string &_key,
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
  template <typename T> T at(const std::string &_name) {
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

protected:
  /// A value or an alias to one: Used internally
  using ValueOrAlias =
      std::variant<Value, std::reference_wrapper<Value>>;

  /// Resolves aliases
  static Value dealias(ValueOrAlias &_what);

  /// A stack of frames: Back is most recent
  std::list<std::map<std::string, ValueOrAlias>> frames;

  /// Either the empty option (not a capture frame) or a list of
  /// all the variables which had to be located from above this
  /// frame.
  std::list<std::optional<std::list<std::string>>>
      barrier_captures;
};

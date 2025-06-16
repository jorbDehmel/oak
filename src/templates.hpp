#pragma once

#include "lexer.hpp"
#include <filesystem>
#include <list>
#include <set>
#include <sys/types.h>

/// Information about a single template block
class TemplateInfo {
public:
  ///
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

  /// Run the given parser as necessary on this template.
  /// This first checks for existing instances. If none
  /// exist, it replaces and parses the validate block. If
  /// that works, it replaces and parses the instantiate
  /// block. If the instantiate block fails, it raises an
  /// error. If not, the instance is logged and we return
  /// without error. Returns true on full success, false
  /// on failure w/o error
  TokenStream instantiate(const Substitution &_substitutions);

private:
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

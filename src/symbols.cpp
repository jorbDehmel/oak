#include "symbols.hpp"
#include "ast_node.hpp"
#include "macro.hpp"
#include <functional>
#include <optional>
#include <stdexcept>
#include <variant>

/// Adds a single empty frame by default
ScopeManager::ScopeManager() {
  push_frame();
}

void ScopeManager::push_frame() noexcept {
  frames.push_back(
      std::map<std::string, ScopeManager::ValueOrAlias>());
}

bool ScopeManager::empty() const noexcept {
  return frames.empty();
}

ASTNodes::Statement ScopeManager::pop_frame() {
  if (frames.size() <= 1) {
    throw std::runtime_error("Cannot pop final stack frame");
  } else {
    ASTNodes::Statement destructors;

    const auto popped = frames.back();
    frames.pop_back();

    for (const auto &entry : popped) {
      if (std::holds_alternative<Value>(entry.second)) {
        const auto non_ref_value =
            std::get<Value>(entry.second);
        if (std::holds_alternative<Type>(non_ref_value)) {
          const auto instance = std::get<Type>(non_ref_value);
          destructors.children.push_back(
              instance.get_destructor_call(
                  ASTNodes::Object(entry.first)));
        }
      }
    }

    return destructors;
  }
}

void ScopeManager::add(const std::string &_key,
                       const NonFnValue &_value) {
  if (frames.back().contains(_key)) {
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function entry with the same name "
        "already exists");
  } else {
    if (std::holds_alternative<StructInfo>(_value)) {
      frames.back()[_key] = std::get<StructInfo>(_value);
    } else if (std::holds_alternative<EnumInfo>(_value)) {
      frames.back()[_key] = std::get<EnumInfo>(_value);
    } else if (std::holds_alternative<TemplateInfo>(_value)) {
      frames.back()[_key] = std::get<TemplateInfo>(_value);
    } else if (std::holds_alternative<MacroInfo>(_value)) {
      frames.back()[_key] = std::get<MacroInfo>(_value);
    } else if (std::holds_alternative<Type>(_value)) {
      frames.back()[_key] = std::get<Type>(_value);
    }
  }
}

void ScopeManager::add(const std::string &_key,
                       const FnInfo &_value) noexcept {
  if (frames.back().contains(_key) &&
      std::holds_alternative<std::list<FnInfo>>(
          dealias(frames.back().at(_key)).get())) {
    // Already exists and is of right type
    std::get<std::list<FnInfo>>(
        dealias(frames.back().at(_key)).get())
        .push_back(_value);
  } else if (!frames.back().contains(_key)) {
    // Does not exist yet
    frames.back()[_key] = std::list<FnInfo>({_value});
  } else {
    // Exists, but as wrong type
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function entry with the same name "
        "already exists");
  }
}

void ScopeManager::alias(
    const std::string &_name_of_alias,
    const std::string &_thing_that_exists) {
  // Resolve
  const auto target = get(_thing_that_exists);

  // Ensure valid constraints
  if (!target.has_value()) {
    throw std::runtime_error("Target of alias must exist");
  }

  // Add pointer
  frames.back()[_name_of_alias] = std::ref(target.value());
}

void ScopeManager::remove_prefix(const std::string &_prefix) {
  // Find anything (in any scope) that has this prefix
  for (auto scope_iter = frames.rbegin();
       scope_iter != frames.rend(); ++scope_iter) {
    for (const auto &entry : *scope_iter) {
      if (entry.first.starts_with(_prefix) &&
          entry.first != _prefix) {
        // Match: Add alias
        alias(entry.first.substr(_prefix.size()), entry.first);
      }
    }
  }
}

std::optional<std::reference_wrapper<ScopeManager::Value>>
ScopeManager::get(const std::string &_name) const noexcept {
  for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
    if (it->contains(_name)) {
      // Resolve any aliasing
      auto cur = it->at(_name);
      return dealias(cur);
    }
  }
  return {};
}

std::reference_wrapper<ScopeManager::Value>
ScopeManager::dealias(ValueOrAlias &_what) {
  if (std::holds_alternative<
          std::reference_wrapper<ScopeManager::Value>>(_what)) {
    // Points to a reference
    return std::get<
        std::reference_wrapper<ScopeManager::Value>>(_what);
  } else {
    // Points to a literal
    return std::ref(std::get<ScopeManager::Value>(_what));
  }
}

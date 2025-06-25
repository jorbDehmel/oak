#include "symbols.hpp"
#include "ast_node.hpp"
#include "debug.hpp"
#include <functional>
#include <optional>
#include <stdexcept>
#include <variant>

/// Adds a single empty frame by default
ScopeManager::ScopeManager() {
  push_frame();
}

void ScopeManager::push_capture_frame() noexcept {
  frames.push_back(
      std::map<std::string, ScopeManager::ValueOrAlias>());
  barrier_captures.push_back(std::list<std::string>()); // Some
}

std::list<std::string>
ScopeManager::get_captures() const noexcept {
  return barrier_captures.back().value_or({});
}

void ScopeManager::push_frame() noexcept {
  frames.push_back(
      std::map<std::string, ScopeManager::ValueOrAlias>());
  barrier_captures.push_back(
      std::optional<std::list<std::string>>()); // None
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
    barrier_captures.pop_back();

    for (const auto &entry : popped) {
      if (std::holds_alternative<Value>(entry.second)) {
        const auto non_ref_value =
            std::get<Value>(entry.second);
        if (std::holds_alternative<Type>(non_ref_value)) {
          const auto instance = std::get<Type>(non_ref_value);
          destructors.children.push_back(
              ASTNodes::Box<ASTNodes::Node>(ASTNodes::Object(
                  instance.get_destructor_call(entry.first))));
        }
      }
    }

    return destructors;
  }
}

void ScopeManager::add(const std::string &_key,
                       const SingularValue &_value) {
  if (frames.back().contains(_key)) {
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function entry with the same name "
        "already exists");
  } else {
    if (std::holds_alternative<StructInfo>(_value)) {
      frames.back()[_key] = std::get<StructInfo>(_value);
      in_order.push_back(std::get<StructInfo>(_value));
    } else if (std::holds_alternative<EnumInfo>(_value)) {
      frames.back()[_key] = std::get<EnumInfo>(_value);
      in_order.push_back(std::get<EnumInfo>(_value));
    }

    else if (std::holds_alternative<Type>(_value)) {
      frames.back()[_key] = std::get<Type>(_value);
    } else if (std::holds_alternative<InlineMacro>(_value)) {
      frames.back()[_key] = std::get<InlineMacro>(_value);
    } else if (std::holds_alternative<CompiledMacro>(_value)) {
      frames.back()[_key] = std::get<CompiledMacro>(_value);
    }

    else {
      // Me-proofing for when I add another variant and forget
      // to change this
      throw std::runtime_error(__FILE__ ":" +
                               std::to_string(__LINE__) +
                               " Unreachable state reached!");
    }
  }
}

void ScopeManager::add(
    const std::string &_key,
    const std::variant<FnInfo, TemplateInfo> &_value) {
  if (frames.back().contains(_key) &&
      std::holds_alternative<FnValue>(
          dealias(frames.back().at(_key)).get())) {
    // Already exists and is of right type
    std::get<FnValue>(dealias(frames.back().at(_key)).get())
        .push_back(_value);

    if (std::holds_alternative<FnInfo>(_value)) {
      in_order.push_back(std::get<FnInfo>(_value));
    }
  } else if (!frames.back().contains(_key)) {
    // Does not exist yet
    frames.back()[_key] = FnValue({_value});
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

std::string
fn_call_str(const std::string &_name,
            const std::list<ASTNodes::Object> &_args) {
  debug_print();
  std::string call_text = _name + "(";
  bool first = true;
  for (const auto &arg : _args) {
    if (first) {
      first = false;
    } else {
      call_text += ", ";
    }
    call_text += "_: " + arg.type.oak_repr();
  }
  call_text += ")";
  return call_text;
}

std::string
fn_call_str(const std::string &_name,
            const std::list<ASTNodes::Node> &_args) {
  debug_print();
  std::string call_text = _name + "(";
  bool first = true;
  for (const auto &arg : _args) {
    if (first) {
      first = false;
    } else {
      call_text += ", ";
    }
    call_text += "_: " + ASTNodes::type(arg).oak_repr();
  }
  call_text += ")";
  return call_text;
}

std::optional<std::reference_wrapper<ScopeManager::Value>>
ScopeManager::get(const std::string &_name) noexcept {
  auto frame_it = frames.rbegin();
  auto capture_it = barrier_captures.rbegin();
  for (; frame_it != frames.rend() &&
         capture_it != barrier_captures.rend();
       ++frame_it, ++capture_it) {
    if (frame_it->contains(_name)) {
      // Resolve any aliasing
      auto cur = frame_it->at(_name);
      return dealias(cur);
    }

    // Log any captures
    if (capture_it->has_value()) {
      capture_it->value().push_back(_name);
    }
  }
  return {};
}

bool ScopeManager::contains(
    const std::string &_name) const noexcept {
  for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
    if (it->contains(_name)) {
      return true;
    }
  }
  return false;
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

std::optional<TemplateInfo::Substitution>
TemplateInfo::find_substitutions(
    const std::string &_name,
    const std::list<std::string> &_signature_to_provide) const {
  debug_print();

  // Check instance
  // For as long as we haven't finished the template
  // If literal on both sides that matches, advance
  // Else if template has generic, log what that template
  // needs to be
  std::map<std::string, uint> generic_indices;
  std::vector<std::list<std::string>> substitutions;
  for (const auto &item : generics) {
    generic_indices[item] = substitutions.size();
    substitutions.push_back({});
  }

  auto desired_it = _signature_to_provide.begin();
  auto templ_it = provides_block.begin();

  while (desired_it != _signature_to_provide.end() &&
         templ_it != provides_block.end()) {
    if (generic_indices.contains(*templ_it)) {
      do {
        substitutions.at(generic_indices.at(*templ_it))
            .push_back(*desired_it);
        ++desired_it;
      } while (desired_it != _signature_to_provide.end() &&
               *desired_it != *std::next(templ_it));
      ++templ_it;
    } else if (*desired_it == *templ_it) {
      ++desired_it;
      ++templ_it;
    } else {
      return {};
    }
  }

  TemplateInfo::Substitution out;
  for (uint j = 0; j < substitutions.size(); ++j) {
    out.push_back(substitutions.at(j));
  }
  return out;
}

std::optional<ASTNodes::Call>
ScopeManager::get_fn(const std::string &_name,
                     const std::list<ASTNodes::Node> &_args) {
  debug_print();

  std::vector<FnInfo> fn_candidates;
  std::list<ASTNodes::Call> exact_matches, cast_matches,
      ref_matches;

  const auto all_candidates = get(_name);

  try {
    if (!all_candidates.has_value()) {
      debug_print();
      return {};
    } else if (!std::holds_alternative<std::list<
                   std::variant<FnInfo, TemplateInfo>>>(
                   all_candidates.value().get())) {
      debug_print();
      return {};
    }

    for (auto &cand :
         std::get<FnValue>(all_candidates.value().get())) {
      if (std::holds_alternative<FnInfo>(cand)) {
        fn_candidates.push_back(std::get<FnInfo>(cand));
      }
    }

    // Attempt existing instances
    for (uint i = 0; i < fn_candidates.size(); ++i) {
      const auto instance_args =
          fn_candidates.at(i).t.fn_args();
      if (instance_args.size() != _args.size()) {
        continue;
      }

      ASTNodes::Call out;
      bool exact = true, ref = true, cast = true;
      out.mangled_c_fn_name = fn_candidates.at(i).t.mangle(
          fn_candidates.at(i).name);
      out.return_type = fn_candidates.at(i).t.fn_return_type();

      auto args_at_j = _args.begin();
      for (uint j = 0;
           j < instance_args.size() && args_at_j != _args.end();
           ++j, ++args_at_j) {
        ASTNodes::Call::Arg arg_to_add;
        arg_to_add.name.get() = *args_at_j;
        arg_to_add.derefs = 0;
        arg_to_add.type = ASTNodes::type(*args_at_j);

        if (exact &&
            !ASTNodes::type(*args_at_j)
                 .exact_match(instance_args[j].second)) {
          exact = false;
        }

        if (ref && !ASTNodes::type(*args_at_j)
                        .ref_match(instance_args[j].second,
                                   arg_to_add.derefs)) {
          ref = false;
        }

        if (cast && !ASTNodes::type(*args_at_j)
                         .cast_match(instance_args[i].second)) {
          cast = false;
        }

        out.args.push_back(arg_to_add);
      }

      if (exact) {
        if (exact_matches.empty() ||
            fn_candidates.at(i).tags["casual"] != "true") {
          exact_matches.push_back(out);
        }
      } else if (ref) {
        ref_matches.push_back(out);
      } else if (cast) {
        cast_matches.push_back(out);
      }
    }

    if (exact_matches.empty()) {
      if (ref_matches.empty()) {
        if (!cast_matches.empty()) {
          // Use casting matches
          if (cast_matches.size() != 1) {
            throw std::runtime_error(
                "Multiple castable matches were "
                "found for function call '" +
                fn_call_str(_name, _args) + "'");
          } else {
            return cast_matches.front();
          }
        }
      } else {
        // Use ref matches
        if (ref_matches.size() != 1) {
          throw std::runtime_error(
              "Multiple reference matches were "
              "found for function call '" +
              fn_call_str(_name, _args) + "'");
        } else {
          return ref_matches.front();
        }
      }
    } else {
      // Use exact matches
      return exact_matches.front();
    }

    // Throw error if it couldn't be resolved
    throw std::runtime_error("No existing candidate nor "
                             "providing template could be "
                             "found for function call '" +
                             fn_call_str(_name, _args) + "'");
  } catch (std::runtime_error &e) {
    std::string msg = "Candidates:\n";
    for (uint i = 0; i < fn_candidates.size(); ++i) {
      msg += fn_candidates.at(i).tags["file"] + ":" +
             fn_candidates.at(i).tags["line"] + "> " +
             fn_candidates.at(i).t.oak_repr(_name) + '\n';
    }
    throw std::runtime_error(msg + e.what());
  }

  debug_print();
}

std::set<std::string> ScopeManager::names() const noexcept {
  std::set<std::string> out;
  for (const auto &frame : frames) {
    for (const auto &p : frame) {
      out.insert(p.first);
    }
  }
  return out;
}

void ScopeManager::drop_fn_with_tag(
    const std::string &_name, const std::string &_key,
    const std::string &_value) noexcept {
  auto val = get(_name);
  if (!val.has_value()) {
    return;
  } else if (!std::holds_alternative<FnValue>(
                 val.value().get())) {
    return;
  }

  std::erase_if(
      std::get<FnValue>(val.value().get()),
      [&](const std::variant<FnInfo, TemplateInfo> &_entry) {
        if (std::holds_alternative<TemplateInfo>(_entry)) {
          return false;
        } else {
          const FnInfo unwrapped = std::get<FnInfo>(_entry);
          if (unwrapped.tags.contains(_key)) {
            return unwrapped.tags.at(_key) == _value;
          }
          return _value == "";
        }
      });
}

void ScopeManager::tag_fn(const std::string &_name,
                          const std::string &_key,
                          const std::string &_value) noexcept {
  std::get<FnInfo>(
      std::get<ScopeManager::FnValue>(get(_name).value().get())
          .back())
      .tags[_key] = _value;
}

bool ScopeManager::contains_atomic_type(
    const std::string &_name) const noexcept {
  for (const auto &frame : frames) {
    if (frame.contains(_name)) {
      if (!std::holds_alternative<Value>(frame.at(_name))) {
        // Aliases don't count
        return false;
      }
      const auto val = std::get<Value>(frame.at(_name));
      return std::holds_alternative<StructInfo>(val) ||
             std::holds_alternative<EnumInfo>(val);
    }
  }

  // None exist, even in shadow form
  return false;
}

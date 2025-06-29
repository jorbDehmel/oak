#include "symbols.hpp"
#include "ast_node.hpp"
#include "debug.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>

ScopeManager::ScopeManager() {
  debug_print();
  push_frame();
}

void ScopeManager::push_capture_frame() noexcept {
  debug_print();
  frames.push_back(
      std::map<std::string, ScopeManager::ValueOrAlias>());
  barrier_captures.push_back(std::list<std::string>()); // Some
}

std::list<std::string>
ScopeManager::get_captures() const noexcept {
  debug_print();
  return barrier_captures.back().value_or({});
}

void ScopeManager::push_frame() noexcept {
  debug_print();
  frames.push_back(
      std::map<std::string, ScopeManager::ValueOrAlias>());
  barrier_captures.push_back(
      std::optional<std::list<std::string>>()); // None
}

bool ScopeManager::empty() const noexcept {
  debug_print();
  return frames.empty();
}

ASTNodes::Statement ScopeManager::pop_frame() {
  debug_print();
  if (frames.size() <= 1) {
    throw std::runtime_error("Cannot pop final stack frame");
  } else {
    ASTNodes::Statement destructors;

    debug_print();
    const auto popped = frames.back();

    debug_print();
    db_assert(barrier_captures.size() == frames.size());
    frames.pop_back();

    debug_print();
    barrier_captures.pop_back();

    debug_print();
    for (const auto &entry : popped) {
      if (std::holds_alternative<Value>(entry.second)) {
        const auto non_ref_value =
            std::get<Value>(entry.second);
        if (std::holds_alternative<Type>(non_ref_value)) {
          const auto instance = std::get<Type>(non_ref_value);
          destructors.children.push_back(
              ASTNodes::OptBox<ASTNodes::Node>(ASTNodes::Object(
                  instance.get_destructor_call(entry.first))));
        }
      }
    }

    debug_print();

    return destructors;
  }
}

void ScopeManager::add(const std::string &_key,
                       const SingularValue &_value) {
  debug_print();
  if (frames.back().contains(_key)) {
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function entry with the same name "
        "already exists");
  } else {
    if (std::holds_alternative<StructInfo>(_value)) {
      debug_print();
      frames.back()[_key] = std::get<StructInfo>(_value);
      in_order.push_back(std::get<StructInfo>(_value));
    } else if (std::holds_alternative<EnumInfo>(_value)) {
      debug_print();
      frames.back()[_key] = std::get<EnumInfo>(_value);
      in_order.push_back(std::get<EnumInfo>(_value));
    }

    else if (std::holds_alternative<Type>(_value)) {
      debug_print();
      frames.back()[_key] = std::get<Type>(_value);
    } else if (std::holds_alternative<InlineMacro>(_value)) {
      debug_print();
      frames.back()[_key] = std::get<InlineMacro>(_value);
    } else if (std::holds_alternative<CompiledMacro>(_value)) {
      debug_print();
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

void ScopeManager::add(const std::string &_key,
                       const FnInfo &_value) {
  debug_print();
  if (frames.back().contains(_key) &&
      std::holds_alternative<Value>(frames.back().at(_key)) &&
      std::holds_alternative<FnValue>(
          dealias(frames.back().at(_key)))) {
    // Already exists and is of right type
    debug_print();
    std::get<FnValue>(std::get<Value>(frames.back().at(_key)))
        .push_back(_value);
    in_order.push_back(_value);
  } else if (!frames.back().contains(_key)) {
    // Does not exist yet
    frames.back()[_key] = FnValue({});
    std::get<FnValue>(std::get<Value>(frames.back().at(_key)))
        .push_back(_value);
    in_order.push_back(_value);
  } else {
    // Exists, but as wrong type
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function or alias entry with the same name "
        "already exists");
  }
}

void ScopeManager::add(const std::string &_key,
                       const TemplateInfo &_value) {
  debug_print();
  if (frames.back().contains(_key) &&
      std::holds_alternative<Value>(frames.back().at(_key)) &&
      std::holds_alternative<FnValue>(
          dealias(frames.back().at(_key)))) {
    // Already exists and is of right type
    debug_print();
    std::get<FnValue>(std::get<Value>(frames.back().at(_key)))
        .push_back(std::shared_ptr<TemplateInfo>(
            new TemplateInfo(_value)));
  } else if (!frames.back().contains(_key)) {
    // Does not exist yet
    frames.back()[_key] = FnValue({});
    std::get<FnValue>(std::get<Value>(frames.back().at(_key)))
        .push_back(std::shared_ptr<TemplateInfo>(
            new TemplateInfo(_value)));
  } else {
    // Exists, but as wrong type
    throw std::runtime_error(
        "Cannot name function '" + _key +
        "': A non-function or alias entry with the same name "
        "already exists");
  }
}

void ScopeManager::alias(
    const std::string &_name_of_alias,
    const std::string &_thing_that_exists) {
  debug_print();
  // Resolve
  const auto target = get(_thing_that_exists);

  // Ensure valid constraints
  if (!target.has_value()) {
    throw std::runtime_error("Target of alias must exist");
  }

  // Add pointer
  frames.back().insert_or_assign(_name_of_alias,
                                 std::ref(target.value()));
}

void ScopeManager::remove_prefix(const std::string &_prefix) {
  debug_print();
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

std::optional<ScopeManager::Value>
ScopeManager::get(const std::string &_name) noexcept {
  debug_print();
  auto frame_it = frames.rbegin();
  auto capture_it = barrier_captures.rbegin();
  for (; frame_it != frames.rend() &&
         capture_it != barrier_captures.rend();
       ++frame_it, ++capture_it) {
    if (frame_it->contains(_name)) {
      // Resolve any aliasing
      debug_print();
      auto cur = frame_it->at(_name);
      return dealias(cur);
    }

    // Log any captures
    if (capture_it->has_value()) {
      debug_print();
      capture_it->value().push_back(_name);
    }
  }
  return {};
}

bool ScopeManager::contains(
    const std::string &_name) const noexcept {
  debug_print();
  for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
    if (it->contains(_name)) {
      return true;
    }
  }
  return false;
}

ScopeManager::Value ScopeManager::dealias(ValueOrAlias &_what) {
  debug_print();
  if (std::holds_alternative<
          std::reference_wrapper<ScopeManager::Value>>(_what)) {
    // Points to a reference
    debug_print();
    return std::get<
        std::reference_wrapper<ScopeManager::Value>>(_what);
  } else {
    // Points to a literal
    debug_print();
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

ASTNodes::Call
ScopeManager::get_fn(const std::string &_name,
                     const std::list<ASTNodes::Node> &_args) {
  debug_print();

  std::vector<FnInfo> fn_candidates;
  std::list<ASTNodes::Call> exact_matches, cast_matches,
      ref_matches;

  const auto all_candidates = get(_name);

  try {
    if (!all_candidates.has_value()) {
      throw std::runtime_error("No entries for '" + _name +
                               "' exist");
    } else if (!std::holds_alternative<FnValue>(
                   all_candidates.value())) {
      throw std::runtime_error(
          "Entries for '" + _name +
          "' exist, but are not of callable type");
    } else if (std::get<FnValue>(all_candidates.value())
                   .empty()) {
      throw std::runtime_error("No entries for '" + _name +
                               "' exist");
    }

    for (auto &cand :
         std::get<FnValue>(all_candidates.value())) {
      if (std::holds_alternative<FnInfo>(cand)) {
        fn_candidates.push_back(std::get<FnInfo>(cand));
      }
    }

    if (fn_candidates.empty()) {
      throw std::runtime_error(
          "No non-template candidates exist for function '" +
          _name + "'");
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
        arg_to_add.name = *args_at_j;
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
                         .cast_match(instance_args[j].second)) {
          cast = false;
        }

        out.args.push_back(arg_to_add);
      }

      if (exact) {
        if (exact_matches.empty()) {
          exact_matches.push_back(out);
        } else if (fn_candidates.at(i).tags["casual"] !=
                   "true") {
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
    if (all_candidates.has_value() &&
        std::holds_alternative<FnValue>(
            all_candidates.value()) &&
        !std::get<FnValue>(all_candidates.value()).empty()) {
      std::string msg = "Candidates:\n";
      const auto c = std::get<FnValue>(all_candidates.value());
      for (const auto &entry : c) {
        if (std::holds_alternative<
                std::shared_ptr<TemplateInfo>>(entry)) {
          auto val =
              std::get<std::shared_ptr<TemplateInfo>>(entry);
          msg += val->path.string() + ":" +
                 std::to_string(val->line) + "> Template\n";
        } else {
          auto val = std::get<FnInfo>(entry);
          msg += val.tags["file"] + ":" + val.tags["line"] +
                 "> " + val.t.oak_repr(_name) + '\n';
        }
      }
      throw std::runtime_error(msg + e.what());
    } else {
      throw e;
    }
  }
}

std::set<std::string> ScopeManager::names() const noexcept {
  debug_print();
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
  debug_print();
  auto val = get(_name);
  if (!val.has_value()) {
    return;
  } else if (!std::holds_alternative<FnValue>(val.value())) {
    return;
  }

  std::erase_if(
      std::get<FnValue>(val.value()),
      [&](const std::variant<
          FnInfo, std::shared_ptr<TemplateInfo>> &_entry) {
        if (std::holds_alternative<
                std::shared_ptr<TemplateInfo>>(_entry)) {
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
  debug_print();
  std::get<FnInfo>(
      std::get<ScopeManager::FnValue>(get(_name).value())
          .back())
      .tags[_key] = _value;
}

bool ScopeManager::contains_atomic_type(
    const std::string &_name) const noexcept {
  debug_print();
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

void ScopeManager::dump(std::ostream &_into) const noexcept {
  const std::function<void(const ValueOrAlias &)> print_entry =
      [&](const ValueOrAlias &p) -> void {
    _into << "(";

    // Print the type of this symbol
    if (std::holds_alternative<std::reference_wrapper<Value>>(
            p)) {
      _into << "Alias of ";
      print_entry(
          std::get<std::reference_wrapper<Value>>(p).get());
    } else {
      const auto &d = std::get<Value>(p);

      // FnValue, StructInfo, EnumInfo, InlineMacro,
      // CompiledMacro, Type
      if (std::holds_alternative<FnValue>(d)) {
        // Name-overloadables: Functions and templates
        const auto info = std::get<FnValue>(d);
        _into << "Name-overloadable with entries [";
        bool first = true;
        for (const auto &entry : info) {
          if (first) {
            first = false;
          } else {
            _into << ", ";
          }
          if (std::holds_alternative<FnInfo>(entry)) {
            const auto instance_info = std::get<FnInfo>(entry);
            _into << instance_info.t.oak_repr(
                instance_info.name);
          } else {
            const auto instance_info =
                std::get<std::shared_ptr<TemplateInfo>>(entry);
            _into << "template from " << instance_info->path
                  << ":" << instance_info->line;
          }
        }
        _into << "]";
      } else if (std::holds_alternative<StructInfo>(d)) {
        // Struct
        const auto info = std::get<StructInfo>(d);
        _into << "Struct with members [";
        bool first = true;
        for (const auto &member : info.member_order) {
          if (first) {
            first = false;
          } else {
            _into << ", ";
          }
          _into << info.members.at(member).oak_repr(member);
        }
        _into << "]";
      } else if (std::holds_alternative<EnumInfo>(d)) {
        // Enum
        const auto info = std::get<EnumInfo>(d);
        _into << "Enum with options [";
        bool first = true;
        for (const auto &branch : info.option_order) {
          if (first) {
            first = false;
          } else {
            _into << ", ";
          }
          _into << info.options.at(branch).oak_repr(branch);
        }
        _into << "]";
      } else if (std::holds_alternative<InlineMacro>(d)) {
        // Inline macro
        _into << "Inline macro yielding `";
        bool first = true;
        for (const auto &tok :
             std::get<InlineMacro>(d).contents) {
          if (first) {
            first = false;
          } else {
            _into << ' ';
          }
          _into << tok.text;
        }
        _into << "`";
      } else if (std::holds_alternative<CompiledMacro>(d)) {
        // Compiled macro
        _into << "Compiled macro at "
              << std::get<CompiledMacro>(d).executable;
      } else {
        // Instance
        _into << "Instance of " << std::get<Type>(d).oak_repr();
      }
    }

    _into << ")";
  };

  _into << "v-------------Begin-------------v\n";

  uint i = 0;
  for (const auto &frame : frames) {
    _into << "Frame " << i << ":\n";
    ++i;
    for (const auto &p : frame) {
      _into << '\t' << p.first << " ";
      print_entry(p.second);
      _into << "\n";
    }
  }

  _into << "^--------------End--------------^\n";
}

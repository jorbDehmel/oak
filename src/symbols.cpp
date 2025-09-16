#include "symbols.hpp"
#include "ast_node.hpp"
#include "debug.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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
  if (barrier_captures.back().has_value()) {
    return barrier_captures.back().value();
  } else {
    return std::list<std::string>{};
  }
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

void ScopeManager::add(const std::string &_name,
                       const SingularValue &_value) {
  debug_print();
  const auto real_name = add_prefix(_name);
  if (frames.back().contains(real_name)) {
    throw std::runtime_error(
        "Cannot name entry '" + real_name +
        "': A non-overloadable local entry with the same name "
        "already exists");
  } else {
    if (std::holds_alternative<StructInfo>(_value)) {
      debug_print();
      frames.back()[real_name] = std::get<StructInfo>(_value);
      in_order.push_back(std::get<StructInfo>(_value));
    } else if (std::holds_alternative<EnumInfo>(_value)) {
      debug_print();
      frames.back()[real_name] = std::get<EnumInfo>(_value);
      in_order.push_back(std::get<EnumInfo>(_value));
    }

    else if (std::holds_alternative<Type>(_value)) {
      debug_print();

      // No globals allowed
      if (frames.size() == 1) {
        throw std::runtime_error(
            "Global variables are not allowed.");
      }

      frames.back()[real_name] = std::get<Type>(_value);
    } else if (std::holds_alternative<InlineMacro>(_value)) {
      debug_print();
      frames.back()[real_name] = std::get<InlineMacro>(_value);
    } else if (std::holds_alternative<CompiledMacro>(_value)) {
      debug_print();
      frames.back()[real_name] =
          std::get<CompiledMacro>(_value);
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

void ScopeManager::add(const std::string &_name,
                       const FnInfo &_value) {
  debug_print();
  const auto real_name = add_prefix(_name);

  if (!frames.back().contains(real_name)) {
    // Does not exist yet
    frames.back()[real_name] = FnValue({});
  } else if (!std::holds_alternative<Value>(
                 frames.back().at(real_name)) ||
             !std::holds_alternative<FnValue>(
                 dealias(frames.back().at(real_name)))) {
    // Exists, but as wrong type
    throw std::runtime_error(
        "Cannot name function '" + real_name +
        "': A non-function or alias entry with the same name "
        "already exists");
  }

  // Clean as needed
  // explicit beats casual and autogen
  // casual beats autogen
  if (_value.tags.contains("casual") &&
      _value.tags.at("casual") == "true") {
    drop_fn_with_tag(_value.name, _value.t, "autogen", "true");
  } else if (!_value.tags.contains("autogen") ||
             _value.tags.at("autogen") != "true") {
    drop_fn_with_tag(_value.name, _value.t, "casual", "true");
    drop_fn_with_tag(_value.name, _value.t, "autogen", "true");
  }

  // Add
  std::get<FnValue>(
      std::get<Value>(frames.back().at(real_name)))
      .push_back(_value);
  in_order.push_back(_value);
}

void ScopeManager::push_prefix(const std::string &_prefix) {
  prefixes.push_back(_prefix);
}

void ScopeManager::pop_prefix() {
  prefixes.pop_back();
}

std::string
ScopeManager::add_prefix(const std::string &_raw_name) const {
  debug_print();
  std::string out = "";
  bool first = true;
  for (const auto &prefix : prefixes) {
    if (first) {
      first = false;
    } else {
      out.push_back('_');
    }
    out += prefix;
  }
  return out + _raw_name;
}

void ScopeManager::add(const std::string &_key,
                       const TemplateInfo &_value) {
  debug_print();
  const auto real_key = add_prefix(_key);
  if (frames.back().contains(real_key) &&
      std::holds_alternative<Value>(
          frames.back().at(real_key)) &&
      std::holds_alternative<TemplValue>(
          dealias(frames.back().at(real_key)))) {
    // Already exists and is of right type
    debug_print();
    std::get<TemplValue>(
        std::get<Value>(frames.back().at(real_key)))
        .push_back(std::shared_ptr<TemplateInfo>(
            new TemplateInfo(_value)));
  } else if (!frames.back().contains(real_key)) {
    // Does not exist yet
    frames.back()[real_key] = TemplValue({});
    std::get<TemplValue>(
        std::get<Value>(frames.back().at(real_key)))
        .push_back(std::shared_ptr<TemplateInfo>(
            new TemplateInfo(_value)));
  } else {
    // Exists, but as wrong type
    throw std::runtime_error(
        "Cannot name template '" + real_key +
        "': A non-template or alias entry with the same name "
        "already exists");
  }
}

void ScopeManager::alias(
    const std::string &_name_of_alias,
    const std::string &_thing_that_exists) {
  debug_print();
  const auto real_name_of_alias = add_prefix(_name_of_alias);
  // Resolve
  const auto target = get(_thing_that_exists);

  // Ensure valid constraints
  if (!target.has_value()) {
    throw std::runtime_error("Target of alias must exist");
  }

  // Add pointer
  frames.back().insert_or_assign(real_name_of_alias,
                                 std::ref(target.value()));
}

void ScopeManager::remove_prefix(const std::string &_prefix) {
  debug_print();
  // Find anything (in any scope) that has this prefix
  const auto real_prefix = _prefix + "_";
  for (auto scope_iter = frames.rbegin();
       scope_iter != frames.rend(); ++scope_iter) {
    for (const auto &entry : *scope_iter) {
      if (entry.first.starts_with(real_prefix) &&
          entry.first != real_prefix) {
        // Match: Add alias
        alias(entry.first.substr(real_prefix.size()),
              entry.first);
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

void ScopeManager::erase(const std::string &_name) noexcept {
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
      frame_it->erase(_name);
      return;
    }

    // Log any captures
    if (capture_it->has_value()) {
      return;
    }
  }
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
               std::reference_wrapper<ScopeManager::Value>>(
               _what)
        .get();
  } else {
    // Points to a literal
    debug_print();
    return std::get<ScopeManager::Value>(_what);
  }
}

ASTNodes::Call
ScopeManager::get_fn(const std::string &_name,
                     const std::list<ASTNodes::Node> &_args) {
  debug_print();

  std::list<ASTNodes::Call> exact_matches, cast_matches,
      ref_matches;

  const auto fn_candidates = get(_name);

  try {
    if (!fn_candidates.has_value()) {
      throw std::runtime_error("No entries for '" + _name +
                               "' exist");
    } else if (!std::holds_alternative<FnValue>(
                   fn_candidates.value())) {
      throw std::runtime_error(
          "Entries for '" + _name +
          "' exist, but are not of callable type");
    } else if (std::get<FnValue>(fn_candidates.value())
                   .empty()) {
      throw std::runtime_error("No entries for '" + _name +
                               "' exist");
    }

    // Attempt existing instances
    for (const auto &instance :
         std::get<FnValue>(fn_candidates.value())) {
      const auto instance_args = instance.t.fn_args();
      if (instance_args.size() != _args.size()) {
        continue;
      }

      ASTNodes::Call out;
      bool exact = true, ref = true, cast = true;
      out.mangled_c_fn_name = instance.t.mangle(instance.name);
      out.return_type = instance.t.fn_return_type();

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
        } else if (!instance.tags.contains("casual") ||
                   instance.tags.at("casual") != "true") {
          exact_matches.push_back(out);
        }
      } else if (ref) {
        ref_matches.push_back(out);
      } else if (cast) {
        cast_matches.push_back(out);
      }
    }

    if (exact_matches.empty()) {
      // No exact: Use ref or cast matches
      if (ref_matches.empty()) {
        // No exact or refs: Use casts
        if (!cast_matches.empty()) {
          if (cast_matches.size() != 1) {
            // Too many!
            throw std::runtime_error(
                "Multiple castable matches were "
                "found for function call '" +
                fn_call_str(_name, _args) + "'");
          } else {
            // Use casting match
            return cast_matches.front();
          }
        }
        // Falls through to empty case
      } else {
        // Has ref matches: Use them
        if (ref_matches.size() != 1) {
          // Too many!
          throw std::runtime_error(
              "Multiple (" +
              std::to_string(ref_matches.size()) +
              ") reference matches were "
              "found for function call '" +
              fn_call_str(_name, _args) + "'");
        } else {
          // Use ref match
          return ref_matches.front();
        }
      }
    } else {
      // Use exact matches
      return exact_matches.front();
    }

    // Throw error if it couldn't be resolved
    throw std::runtime_error("No existing candidate could be "
                             "found for function call '" +
                             fn_call_str(_name, _args) + "'");
  } catch (std::runtime_error &e) {
    if (fn_candidates.has_value() &&
        std::holds_alternative<FnValue>(
            fn_candidates.value()) &&
        !std::get<FnValue>(fn_candidates.value()).empty()) {
      std::string msg = "\nCandidates:\n\n";
      const auto c = std::get<FnValue>(fn_candidates.value());
      for (const auto &entry : c) {
        auto val = entry;
        msg += val.t.oak_repr(_name) + "\n\tlocation:\t" +
               val.tags["file"] + ":" + val.tags["line"] + "." +
               val.tags["col"];
        for (const auto &p : val.tags) {
          if (p.first == "file" || p.first == "line" ||
              p.first == "col") {
            continue;
          }
          msg += "\n\t" + p.first + ":\t" + p.second;
        }

        msg += "\n\n";
      }
      throw std::runtime_error(msg + "\n" + e.what());
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
    const std::string &_name, const Type &_to_match,
    const std::string &_key,
    const std::string &_value) noexcept {
  debug_print();

  // Locate
  auto frame = frames.rbegin();
  for (; frame != frames.rend(); ++frame) {
    if (frame->contains(_name)) {
      break;
    }
  }

  // No results!
  if (frame == frames.rend() || !frame->contains(_name)) {
    return;
  }

  auto &entry = frame->at(_name);
  if (std::holds_alternative<Value>(entry)) { // Value
    if (!std::holds_alternative<FnValue>(
            std::get<Value>(entry))) {
      return;
    }
    std::erase_if(
        std::get<FnValue>(std::get<Value>(entry)),
        [&](const std::variant<
            FnInfo, std::shared_ptr<TemplateInfo>> &_entry) {
          if (std::holds_alternative<
                  std::shared_ptr<TemplateInfo>>(_entry)) {
            return false;
          } else {
            const FnInfo unwrapped = std::get<FnInfo>(_entry);
            if (!_to_match.exact_match(unwrapped.t)) {
              return false;
            }
            if (unwrapped.tags.contains(_key)) {
              return unwrapped.tags.at(_key) == _value;
            }
            return _value == "";
          }
        });
  } else { // Alias
    if (!std::holds_alternative<FnValue>(
            std::get<std::reference_wrapper<Value>>(entry)
                .get())) {
      return;
    }
    std::erase_if(
        std::get<FnValue>(
            std::get<std::reference_wrapper<Value>>(entry)
                .get()),
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
}

void ScopeManager::tag_fn(const std::string &_name,
                          const std::string &_key,
                          const std::string &_value) noexcept {
  debug_print();

  // Locate
  auto frame = frames.rbegin();
  for (; frame != frames.rend(); ++frame) {
    if (frame->contains(_name)) {
      break;
    }
  }

  // No results!
  if (frame == frames.rend() || !frame->contains(_name)) {
    return;
  }

  auto &entry = frame->at(_name);
  if (std::holds_alternative<Value>(entry)) { // Value
    if (!std::holds_alternative<FnValue>(
            std::get<Value>(entry))) {
      return;
    }
    std::get<ScopeManager::FnValue>(std::get<Value>(entry))
        .back()
        .tags[_key] = _value;
  } else { // Alias
    if (!std::holds_alternative<FnValue>(
            std::get<std::reference_wrapper<Value>>(entry)
                .get())) {
      return;
    }
    std::get<ScopeManager::FnValue>(
        std::get<std::reference_wrapper<Value>>(entry).get())
        .back()
        .tags[_key] = _value;
  }
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
        // Name-overloadables functions
        const auto info = std::get<FnValue>(d);
        _into << "Name-overloadable function with entries [";
        bool first = true;
        for (const auto &entry : info) {
          if (first) {
            first = false;
          } else {
            _into << ", ";
          }
          const auto instance_info = entry;
          _into << instance_info.t.oak_repr(instance_info.name);
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
      } else if (std::holds_alternative<Type>(d)) {
        // Instance
        _into << "Instance of " << std::get<Type>(d).oak_repr();
      } else if (std::holds_alternative<TemplValue>(d)) {
        // Name-overloadables templates
        const auto info = std::get<TemplValue>(d);
        _into << "Name-overloadable template with "
              << info.size() << " entries\n";
      } else {
        _into << "Unknown value\n";
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

TokenStream StructInfo::get_default_constructor(
    const Lexer::Token &_where) const {
  std::string to_lex = "(self: ^" + name + ") -> void { ";
  for (const auto &member : member_order) {
    to_lex += "New(self." + member + "); ";
  }
  to_lex += "}";
  uint64_t line = _where.line, col = _where.col;
  return Lexer::lex(to_lex, _where.file, line, col);
}

TokenStream StructInfo::get_default_destructor(
    const Lexer::Token &_where) const {
  std::string to_lex = "(self: ^" + name + ") -> void {";
  for (auto it = member_order.rbegin();
       it != member_order.rend(); ++it) {
    to_lex += "Del(self." + *it + ");";
  }
  to_lex += "}";
  uint64_t line = _where.line, col = _where.col;
  return Lexer::lex(to_lex, _where.file, line, col);
}

/// Returns a constructor definition, ready to be parsed
TokenStream EnumInfo::get_default_constructor(
    const Lexer::Token &_where) const {
  const std::string op = option_order.front();
  const std::string text = "(self: ^" + name +
                           ") -> void {"
                           "let __data: " +
                           options.at(op).oak_repr() +
                           "; wrap_" + op +
                           "(self, __data);"
                           "}";
  uint64_t line = _where.line, col = _where.col;
  return Lexer::lex(text, _where.file, line, col);
}

TokenStream EnumInfo::get_default_destructor(
    const Lexer::Token &_where) const {
  std::string to_lex = "(self: ^" + name +
                       ") -> void {\n"
                       "match (self) {\n";
  for (const auto &option : option_order) {
    to_lex += "case " + option + "(" +
              options.at(option).ref().oak_repr("data") +
              ") {\n"
              "Del(data);\n"
              "}\n";
  }
  to_lex.append("}\n}");
  uint64_t line = _where.line, col = _where.col;
  return Lexer::lex(to_lex, _where.file, line, col);
}

std::list<FnInfo>
EnumInfo::get_wrappers(const Lexer::Token &_where) const {
  std::list<FnInfo> out;
  for (const auto &p : options) {
    const auto wrapper_name = "wrap_" + p.first;
    FnInfo to_add;
    to_add.name = wrapper_name;

    to_add.tags["file"] = _where.file;
    to_add.tags["line"] = std::to_string(_where.line);
    to_add.tags["col"] = std::to_string(_where.col);

    // Construct wrapper type
    to_add.t.append_fn();
    to_add.t.nodes.back().following_arg_name = "self";
    to_add.t.append_ptr();
    to_add.t.append_literal(name); // Enum name
    to_add.t.append_join();
    to_add.t.nodes.back().following_arg_name = "__data";
    to_add.t.append_type(p.second);
    to_add.t.append_maps();
    to_add.t.append_literal("void");

    // Node
    ASTNodes::RawCFormat child;

    child.format_string = "{ self->__info = " + name + "_OPT_" +
                          p.first + "; self->__data." +
                          p.first + " = __data; }";

    to_add.n.children = {ASTNodes::OptBox<ASTNodes::Node>(
        ASTNodes::RawCFormat())};
    to_add.tags["autogen"] = "true";

    // Insert fn
    out.push_back(to_add);
  }
  return out;
}

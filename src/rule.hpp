/**
 * @file
 */

#pragma once

#include "lexer.hpp"
#include <list>
#include <string>

/**
 * @class Rule
 * @brief An abstract rule, independent of engine
 */
class Rule {
public:
  /// If returns false, write the input. Else if output_size is
  /// 0 (default), don't write anything. If output_size is
  /// nonzero, put the first `output_size` items of `output`.
  using DeltaFn = bool (*)(OakToken input[], OakToken output[],
                           uint *output_size);

  ///
  const DeltaFn delta_fn;

  /// Rules that must be done first: Externally handled
  const std::list<std::string> prereqs;
};

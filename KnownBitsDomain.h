// KnownBitsDomain.h - The abstract domain
//
// A `BitKind`:
// A four-point lattice recording whether a bit is known to be 0 or 1.
//
//        Top          nothing is known
//       /   \
//    Zero   One
//       \   /
//       Bottom       unreachable, or not yet analyzed
//
// A `BitFlagsState` is an N-array of `BitKind`s.
//
// This is the file to replace first when building a different analysis.  MLIR's
// dataflow framework asks only three things of a lattice value:
//
//   * a default constructor, which must produce the bottom element, because the
//     solver starts every value optimistically and lowers it as facts arrive;
//   * a static join(), which must be commutative, associative, idempotent, and
//     monotone -- assertions in Lattice<> check monotonicity in debug builds;
//   * operator== and print().
//

#ifndef KNOWNBITS_DOMAIN_H
#define KNOWNBITS_DOMAIN_H

#include "llvm/Support/raw_ostream.h"

namespace knownbits {

enum class Kind { Bottom, Zero, One, Top };

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "bottom";
  case Kind::Zero:
    return "zero";
  case Kind::One:
    return "one";
  case Kind::Top:
    return "top";
  }
  return "top";
}

struct BitState {
  Kind kind = Kind::Bottom;

  BitState() = default;
  /* implicit */ BitState(Kind kind) : kind(kind) {}

  static BitState bottom() { return Kind::Bottom; }
  static BitState top() { return Kind::Top; }

  bool isBottom() const { return kind == Kind::Bottom; }

  static BitState join(const BitState &lhs, const BitState &rhs) {
    if (lhs.kind == Kind::Bottom)
      return rhs;
    if (rhs.kind == Kind::Bottom)
      return lhs;
    if (lhs.kind == rhs.kind)
      return lhs;
    return top();
  }

  bool operator==(const BitState &other) const { return kind == other.kind; }
  bool operator!=(const BitState &other) const { return kind != other.kind; }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const BitState &state) {
  state.print(os);
  return os;
}

} // namespace knownbits

#endif

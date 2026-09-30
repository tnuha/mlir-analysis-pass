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

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

// defines the N-array that a `BitFlagsState` is :)
#define NBITS 8

namespace knownbits {

enum class Kind { Bottom, Zero, One, Top };

inline const char *name(Kind kind) {
  switch (kind) {
  case Kind::Bottom:
    return "B";
  case Kind::Zero:
    return "0";
  case Kind::One:
    return "1";
  case Kind::Top:
    return "T";
  }
  return "T";
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

  BitState operator|(const BitState &other) {
    if (kind == Kind::Bottom || other.kind == Kind::Bottom)
      return Kind::Bottom;
    else if (kind == Kind::One || other.kind == Kind::One)
      return Kind::One;
    else if (kind == Kind::Zero && other.kind == Kind::Zero)
      return Kind::Zero;
    return Kind::Top;
  }
  BitState operator&(const BitState &other) {
    if (kind == Kind::Bottom || other.kind == Kind::Bottom)
      return Kind::Bottom;
    else if (kind == Kind::Zero || other.kind == Kind::Zero)
      return Kind::Zero;
    else if (kind == Kind::One && other.kind == Kind::One)
      return Kind::One;
    return Kind::Top;
  }
  BitState operator^(const BitState &other) {
    if (kind == Kind::Bottom || other.kind == Kind::Bottom)
      return Kind::Bottom;
    else if (kind == Kind::Top || other.kind == Kind::Top)
      return Kind::Top;
    else if (kind == other.kind)
      return Kind::Zero;
    return Kind::One;
  }

  void print(llvm::raw_ostream &os) const { os << name(kind); }
};

struct BitFlagsState {
  // TODO: revisit, is this valid memory?
  llvm::SmallVector<BitState, NBITS> bits = bottom().bits;

  BitFlagsState() = default;
  BitFlagsState(llvm::SmallVector<BitState, NBITS> bits) : bits(bits) {}

  static BitFlagsState bottom() {
    llvm::SmallVector<BitState, NBITS> bits(NBITS, Kind::Bottom);
    return bits;
  }
  static BitFlagsState top() {
    llvm::SmallVector<BitState, NBITS> bits(NBITS, Kind::Top);
    return bits;
  }

  // TODO: generalize to other sizes
  BitFlagsState(uint8_t raw) {
    for (auto i = 0; i < 8; i++) {
      bits[i] = (raw & 0b1) ? Kind::One : Kind::Zero;
      raw >>= 1;
    }
  }
  // TODO: generalize to other sizes
  uint8_t asRaw() const {
    assert(this->fullyKnown());
    uint8_t res = 0x0;
    for (auto i = 0; i < NBITS; i++)
      res |= (this->bits[i] == Kind::One ? 1 : 0) << i;

    return res;
  }

  bool isBottom() const {
    BitFlagsState bot = bottom();
    return *this == bot;
  }

  // join defined bitwise
  static BitFlagsState join(const BitFlagsState &lhs,
                            const BitFlagsState &rhs) {
    BitFlagsState res;
    for (auto i = 0; i < NBITS; i++)
      res.bits[i] = BitState::join(lhs.bits[i], rhs.bits[i]);

    return res;
  }

  bool operator==(const BitFlagsState other) const {
    for (auto i = 0; i < NBITS; i++)
      if (this->bits[i] != other.bits[i])
        return false;
    return true;
  }
  bool operator!=(const BitFlagsState other) const { return !(*this == other); }
  BitFlagsState operator|(const BitFlagsState &other) {
    BitFlagsState res;
    for (auto i = 0; i < NBITS; i++)
      res.bits[i] = this->bits[i] | other.bits[i];

    return res;
  }
  BitFlagsState operator&(const BitFlagsState &other) {
    BitFlagsState res;
    for (auto i = 0; i < NBITS; i++)
      res.bits[i] = this->bits[i] & other.bits[i];

    return res;
  }
  BitFlagsState operator^(const BitFlagsState &other) {
    BitFlagsState res;
    for (auto i = 0; i < NBITS; i++)
      res.bits[i] = this->bits[i] ^ other.bits[i];

    return res;
  }

  bool fullyKnown() const {
    for (auto i = 0; i < NBITS; i++)
      if (this->bits[i].kind == Kind::Top)
        return false;

    return true;
  }

  void print(llvm::raw_ostream &os) const {
    for (auto i = NBITS - 1; i >= 0; i--)
      os << name(bits[i].kind);
  }
};

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const BitState &state) {
  state.print(os);
  return os;
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                     const BitFlagsState &state) {
  state.print(os);
  return os;
}

} // namespace knownbits

#endif

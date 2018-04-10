#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace Lox {
  enum class OpCode : unsigned char {
    Constant,
    Add,
    Subtract,
    Multiply,
    Divide,
    Negative,
    Return
  };

  class Chunk {
  public:
    std::byte read(size_t offset) const { return bytecode_[offset]; }
    void write(std::byte byte) { bytecode_.push_back(byte); }
    void write(OpCode opCode, unsigned line);

    size_t size() const noexcept { return bytecode_.size(); }

    double getConstant(size_t index) const { return constants_[index]; }
    size_t addConstant(double value);

    unsigned getLineNumber(size_t offset) const;

  private:
    std::vector<std::byte> bytecode_ {};
    std::vector<double> constants_ {};
    std::vector<std::pair<unsigned, size_t>> lineStarts_ {};
  };
}

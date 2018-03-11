#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace Lox {
  enum class OpCode : uint8_t {
    Constant,
    Return
  };

  class Chunk {
  public:
    uint8_t read(size_t offset) const { return bytecode_[offset]; }
    void write(uint8_t byte) { bytecode_.push_back(byte); }
    void write(OpCode opCode, unsigned line);

    size_t size() const noexcept { return bytecode_.size(); }

    double getConstant(size_t index) const { return constants_[index]; }
    size_t addConstant(double value);

    unsigned getLineNumber(size_t offset) const;

  private:
    std::vector<uint8_t> bytecode_ {};
    std::vector<double> constants_ {};
    std::vector<std::pair<unsigned, size_t>> lineStarts_ {};
  };
}

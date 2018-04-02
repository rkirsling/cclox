#include "chunk.h"

#include <algorithm>

namespace Lox {
  void Chunk::write(OpCode opCode, unsigned line) {
    write(static_cast<std::byte>(opCode));
    if (!lineStarts_.empty() && lineStarts_.back().first == line) return;

    lineStarts_.emplace_back(line, size() - 1);
  }

  size_t Chunk::addConstant(double value) {
    constants_.push_back(value);
    return constants_.size() - 1;
  }

  unsigned Chunk::getLineNumber(size_t offset) const {
    const auto lessEqual = [=](const auto& lineStart) { return lineStart.second <= offset; };
    return std::find_if(lineStarts_.crbegin(), lineStarts_.crend(), lessEqual)->first;
  }
}

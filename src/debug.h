#pragma once

#include <cstddef>
#include <string>
#include <utility>

namespace Lox {
  class Chunk;

  class ChunkPrinter {
  public:
    ChunkPrinter(const Chunk& chunk, std::string&& name)
      : chunk_(chunk), name_(std::move(name)) {}

    void print();

  private:
    void printInstruction();

    const Chunk& chunk_;
    const std::string name_;
    size_t offset_ { 0 };
    unsigned line_ { 1 };
  };
}

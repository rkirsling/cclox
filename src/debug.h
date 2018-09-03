#pragma once

#include <cstddef>
#include <string>

namespace Lox {
  class Chunk;

  class ChunkPrinter {
  public:
    void print(const Chunk& chunk, const std::string& name);

  private:
    void printInstruction();

    const Chunk* chunk_;
    size_t offset_ { 0 };
  };
}

#include "chunk.h"
#include "debug.h"

using namespace Lox;

int main() {
  Chunk chunk {};
  ChunkPrinter chunkPrinter { chunk, "test chunk" };

  const auto index = chunk.addConstant(1.2);
  chunk.write(OpCode::Constant, 1);
  chunk.write(index);
  chunk.write(OpCode::Return, 2);

  chunkPrinter.print();
  return 0;
}

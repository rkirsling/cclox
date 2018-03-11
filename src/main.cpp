#include "chunk.h"
#include "debug.h"

using namespace Lox;

int main() {
  Chunk chunk {};
  ChunkPrinter chunkPrinter { chunk, "test chunk" };

  const auto index = chunk.addConstant(1.2);
  chunk.write(OpCode::Constant, 0);
  chunk.write(index);
  chunk.write(OpCode::Return, 1);

  chunkPrinter.print();
  return 0;
}

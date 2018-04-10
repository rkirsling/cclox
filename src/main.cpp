#include "chunk.h"
#include "debug.h"
#include "vm.h"

using namespace Lox;

int main() {
  VM vm {};

  // -((1.2 + 3.4) / 5.6)
  Chunk chunk {};
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<std::byte>(chunk.addConstant(1.2)));
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<std::byte>(chunk.addConstant(3.4)));
  chunk.write(OpCode::Add, 1);
  chunk.write(OpCode::Constant, 1);
  chunk.write(static_cast<std::byte>(chunk.addConstant(5.6)));
  chunk.write(OpCode::Divide, 1);
  chunk.write(OpCode::Negative, 1);
  chunk.write(OpCode::Return, 1);

  ChunkPrinter chunkPrinter { chunk, "test chunk" };
  chunkPrinter.print();
  printf("\n");

  vm.interpret(chunk);
  return 0;
}

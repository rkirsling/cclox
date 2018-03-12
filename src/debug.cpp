#include "debug.h"

#include "chunk.h"
#include <cstdio>

namespace Lox {
  void ChunkPrinter::print() {
    printf("== %s ==\n", name_.c_str());

    offset_ = 0;
    while (offset_ < chunk_.size()) printInstruction();
  }

  void ChunkPrinter::printInstruction() {
    printf("%02zx", offset_);

    const auto line = chunk_.getLineNumber(offset_);
    if (offset_ > 0 && line == line_) {
      printf("   ..   ..   ");
    } else {
      printf(" <line %04d> ", line);
      line_ = line;
    }

    const auto opCode = static_cast<OpCode>(chunk_.read(offset_++));
    switch (opCode) {
      case OpCode::Constant:
        printConstant();
        break;
      case OpCode::Return:
        printf("return\n");
        break;
      default:
        printf("unknown\n");
        break;
    }
  }

  void ChunkPrinter::printConstant() {
    const auto index = chunk_.read(offset_++);
    printf("constant %02x  # value: %g\n", index, chunk_.getConstant(index));
  }
}

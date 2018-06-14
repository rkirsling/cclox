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
      printf(" <line %04u> ", line);
      line_ = line;
    }

    const auto opCode = static_cast<OpCode>(chunk_.read(offset_++));
    switch (opCode) {
      case OpCode::Constant: {
        const auto index = static_cast<size_t>(chunk_.read(offset_++));
        printf("constant %02zx  # value: %g\n", index, chunk_.getConstant(index));
      } break;
      case OpCode::Add:
        printf("add\n");
        break;
      case OpCode::Subtract:
        printf("subtract\n");
        break;
      case OpCode::Multiply:
        printf("multiply\n");
        break;
      case OpCode::Divide:
        printf("divide\n");
        break;
      case OpCode::Negative:
        printf("negative\n");
        break;
      case OpCode::Return:
        printf("return\n");
        break;
      default:
        printf("unknown\n");
        break;
    }
  }
}

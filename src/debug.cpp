#include "debug.h"

#include "chunk.h"
#include <iomanip>
#include <iostream>

namespace Lox {
  void ChunkPrinter::print() {
    std::cout << "== " << name_ << " ==" << std::setfill('0');

    offset_ = 0;
    while (offset_ < chunk_.size()) printInstruction();

    std::cout << std::setfill(' ') << std::endl;
  }

  void ChunkPrinter::printInstruction() {
    std::cout << "\n" << std::setw(2) << std::hex << offset_;

    const auto line = chunk_.getLineNumber(offset_);
    if (offset_ > 0 && line == line_) {
      std::cout << "    ....    ";
    } else {
      std::cout << " <line " << std::setw(3) << std::dec << line << "> ";
      line_ = line;
    }

    const auto opCode = static_cast<OpCode>(chunk_.read(offset_++));
    switch (opCode) {
      case OpCode::Constant:
        printConstant();
        break;
      case OpCode::Return:
        std::cout << "return";
        break;
      default:
        std::cout << "<unknown>";
        break;
    }
  }

  void ChunkPrinter::printConstant() {
    const auto index = chunk_.read(offset_++);
    std::cout << "constant#" << std::setw(3) << index << ": " << chunk_.getConstant(index);
  }
}

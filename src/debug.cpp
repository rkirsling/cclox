#ifndef NDEBUG
#include "debug.h"

#include "chunk.h"
#include <cstdio>

namespace Lox {
  void ChunkPrinter::print(const Chunk& chunk, std::string_view name) {
    printf("== %s ==\n", name.data());

    chunk_ = &chunk;
    offset_ = 0;
    while (offset_ < chunk_->size()) printInstruction();

    printf("== end ==\n");
  }

  void ChunkPrinter::printInstruction() {
    printf("%02zx", offset_);

    const auto position = chunk_->getPosition(offset_);
    printf(" (%3u:%3u) ", position.first, position.second);

    const auto opCode = static_cast<OpCode>(chunk_->read(offset_++));
    switch (opCode) {
      case OpCode::Constant: {
        const auto index = static_cast<size_t>(chunk_->read(offset_++));
        const auto value = chunk_->getConstant(index);
        if (const auto string = std::get_if<std::string>(&value)) {
          printf("constant %02zx   # value: \"%s\"\n", index, string->c_str());
        } else if (const auto number = std::get_if<double>(&value)) {
          printf("constant %02zx   # value: %g\n", index, *number);
        }
      } break;
      case OpCode::Nil:
        printf("nil\n");
        break;
      case OpCode::True:
        printf("true\n");
        break;
      case OpCode::False:
        printf("false\n");
        break;
      case OpCode::Pop:
        printf("pop\n");
        break;
      case OpCode::DefineGlobal:
        printf("define_global\n");
        break;
      case OpCode::SetGlobal:
        printf("set_global\n");
        break;
      case OpCode::GetGlobal:
        printf("get_global\n");
        break;
      case OpCode::SetLocal: {
        const auto index = static_cast<size_t>(chunk_->read(offset_++));
        printf("set_local %02zx\n", index);
      } break;
      case OpCode::GetLocal: {
        const auto index = static_cast<size_t>(chunk_->read(offset_++));
        printf("get_local %02zx\n", index);
      } break;
      case OpCode::Equal:
        printf("equal\n");
        break;
      case OpCode::NotEqual:
        printf("not_equal\n");
        break;
      case OpCode::Greater:
        printf("greater\n");
        break;
      case OpCode::GreaterEqual:
        printf("greater_equal\n");
        break;
      case OpCode::Less:
        printf("less\n");
        break;
      case OpCode::LessEqual:
        printf("less_equal\n");
        break;
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
      case OpCode::Not:
        printf("not\n");
        break;
      case OpCode::Print:
        printf("print\n");
        break;
      case OpCode::Jump: {
        const auto distance = static_cast<size_t>(chunk_->read(offset_++));
        printf("jump %02zx       # ->%02zx\n", distance, offset_ + distance);
      } break;
      case OpCode::JumpIfTrue: {
        const auto distance = static_cast<size_t>(chunk_->read(offset_++));
        printf("jump_true %02zx  # ->%02zx\n", distance, offset_ + distance);
      } break;
      case OpCode::JumpIfFalse: {
        const auto distance = static_cast<size_t>(chunk_->read(offset_++));
        printf("jump_false %02zx # ->%02zx\n", distance, offset_ + distance);
      } break;
      case OpCode::Loop: {
        const auto distance = static_cast<size_t>(chunk_->read(offset_++));
        printf("loop %02zx       # ->%02zx\n", distance, offset_ - distance);
      } break;
      case OpCode::Return:
        printf("return\n");
        break;
      default:
        printf("unknown\n");
        break;
    }
  }
}
#endif

#include "vm.h"

#include "chunk.h"

namespace Lox {
  ResultStatus VM::interpret(const Chunk& chunk) {
    auto offset = 0u;

    for (;;) {
      const auto opCode = static_cast<OpCode>(chunk.read(offset++));

      switch (opCode) {
        case OpCode::Constant: {
          const auto index = static_cast<size_t>(chunk.read(offset++));
          valueStack_.push_back(chunk.getConstant(index));
        } break;
        case OpCode::Add:
          performBinaryOp(std::plus<> {});
          break;
        case OpCode::Subtract:
          performBinaryOp(std::minus<> {});
          break;
        case OpCode::Multiply:
          performBinaryOp(std::multiplies<> {});
          break;
        case OpCode::Divide:
          performBinaryOp(std::divides<> {});
          break;
        case OpCode::Negative:
          valueStack_.back() = -valueStack_.back();
          break;
        case OpCode::Return:
          printf("%g\n", valueStack_.back());
          valueStack_.pop_back();
          return ResultStatus::OK;
      }
    }
  }

  inline void VM::performBinaryOp(const std::function<double(double, double)>& op) {
    const auto rightOperand = valueStack_.back();
    valueStack_.pop_back();
    valueStack_.back() = op(valueStack_.back(), rightOperand);
  }
}

#include "vm.h"

#include "chunk.h"

namespace Lox {
  ResultStatus VM::interpret(const std::string& source, unsigned line) {
    errorReporter_.reset();

    const auto chunk = compiler_.compile(source, line);
    if (errorReporter_.errorCount() > 0) return ResultStatus::StaticError;

#ifndef NDEBUG
    chunkPrinter_.print(*chunk, "root");
#endif
    return execute(*chunk);
  }

  ResultStatus VM::execute(const Chunk& chunk) {
    for (auto offset = 0u; offset < chunk.size(); ++offset) {
      const auto opCode = static_cast<OpCode>(chunk.read(offset));

      switch (opCode) {
        case OpCode::Constant: {
          const auto index = static_cast<size_t>(chunk.read(++offset));
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
          printf("%g\n", valueStack_.back()); // temporary
          valueStack_.pop_back();
          return ResultStatus::OK;
      }
    }

    return ResultStatus::DynamicError;
  }

  void VM::performBinaryOp(const std::function<double(double, double)>& op) {
    const auto rightOperand = valueStack_.back();
    valueStack_.pop_back();
    valueStack_.back() = op(valueStack_.back(), rightOperand);
  }
}

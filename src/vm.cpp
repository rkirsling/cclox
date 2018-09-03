#include "vm.h"

#include <stdexcept>

namespace Lox {
  constexpr static bool isTruthy(Value value) {
    return std::holds_alternative<bool>(value) ? std::get<bool>(value) : !std::holds_alternative<std::monostate>(value);
  }

  ResultStatus VM::interpret(const std::string& source, unsigned line) {
    errorReporter_.reset();

    chunk_ = compiler_.compile(source, line);
    if (errorReporter_.errorCount() > 0) {
      errorReporter_.displayErrorCount();
      return ResultStatus::StaticError;
    }

#ifndef NDEBUG
    chunkPrinter_.print(*chunk_, "root");
#endif
    try {
      execute();
    } catch (const LoxError& error) {
      errorReporter_.report(error, true);
      return ResultStatus::DynamicError;
    }

    return ResultStatus::OK;
  }

  void VM::execute() {
    for (offset_ = 0; offset_ < chunk_->size(); ++offset_) {
      const auto opCode = static_cast<OpCode>(chunk_->read(offset_));

      switch (opCode) {
        case OpCode::Constant: {
          const auto index = static_cast<size_t>(chunk_->read(++offset_));
          valueStack_.push_back(chunk_->getConstant(index));
        } break;
        case OpCode::Nil:
          valueStack_.emplace_back();
          break;
        case OpCode::True:
          valueStack_.emplace_back(true);
          break;
        case OpCode::False:
          valueStack_.emplace_back(false);
          break;
        case OpCode::Equal: {
          const auto rightOperand = pop();
          valueStack_.back() = valueStack_.back() == rightOperand;
        } break;
        case OpCode::NotEqual: {
          const auto rightOperand = pop();
          valueStack_.back() = valueStack_.back() != rightOperand;
        } break;
        case OpCode::Greater: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() > rightOperand;
        } break;
        case OpCode::GreaterEqual: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() >= rightOperand;
        } break;
        case OpCode::Less: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() < rightOperand;
        } break;
        case OpCode::LessEqual: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() <= rightOperand;
        } break;
        case OpCode::Add: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() + rightOperand;
        } break;
        case OpCode::Subtract: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() - rightOperand;
        } break;
        case OpCode::Multiply: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() * rightOperand;
        } break;
        case OpCode::Divide: {
          const auto rightOperand = popNumberOperand();
          valueStack_.back() = peekNumberOperand() / rightOperand;
        } break;
        case OpCode::Negative:
          valueStack_.back() = -peekNumberOperand();
          break;
        case OpCode::Not:
          valueStack_.back() = !isTruthy(valueStack_.back());
          break;
        case OpCode::Return:
          // temporary
          if (const auto number = std::get_if<double>(&valueStack_.back())) {
            printf("%g\n", *number);
          } else if (const auto boolean = std::get_if<bool>(&valueStack_.back())) {
            printf(*boolean ? "true\n" : "false\n");
          } else {
            printf("nil\n");
          }
          valueStack_.pop_back();
          return;
      }
    }

    throw std::logic_error { "Chunk ended without a return!" };
  }

  template<typename T>
  bool VM::peekIs() const {
    return std::holds_alternative<T>(valueStack_.back());
  }

  Value VM::pop() {
    const auto value = valueStack_.back();
    valueStack_.pop_back();
    return value;
  }

  template<typename T>
  T VM::expect(std::string&& errorMessage, bool shouldPop) {
    if (!peekIs<T>()) throw LoxError { chunk_->getPosition(offset_), std::move(errorMessage) };

    return std::get<T>(shouldPop ? pop() : valueStack_.back());
  }
}

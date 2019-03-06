#include "vm.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace Lox {
  static constexpr bool isTruthy(const Value& value) {
    return std::holds_alternative<bool>(value) ? std::get<bool>(value) : !std::holds_alternative<std::monostate>(value);
  }

  static std::string stringify(const Value& value) {
    if (const auto string = std::get_if<std::string>(&value)) return *string;

    if (const auto number = std::get_if<double>(&value)) {
      std::ostringstream oss {};
      oss << *number;
      return oss.str();
    }

    if (const auto boolean = std::get_if<bool>(&value)) return *boolean ? "true" : "false";

    return "nil";
  }

  ResultStatus VM::interpret(std::string_view source, unsigned line) {
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
      valueStack_.clear();
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
        case OpCode::Pop:
          pop();
          break;
        case OpCode::DefineGlobal: {
          const auto value = pop();
          const auto name = std::get<std::string>(pop());
          if (globals_.find(name) != globals_.cend()) {
            throw LoxError { chunk_->getPosition(offset_), "Identifier '" + name + "' is already defined." };
          }
          globals_.emplace(name, value);
        } break;
        case OpCode::SetGlobal: {
          const auto newValue = pop();
          const auto name = std::get<std::string>(valueStack_.back());
          const auto oldValue = globals_.find(name);
          if (oldValue == globals_.cend()) {
            throw LoxError { chunk_->getPosition(offset_), "Identifier '" + name + "' is undefined." };
          }
          valueStack_.back() = oldValue->second = newValue;
        } break;
        case OpCode::GetGlobal: {
          const auto name = std::get<std::string>(valueStack_.back());
          const auto value = globals_.find(name);
          if (value == globals_.cend()) {
            throw LoxError { chunk_->getPosition(offset_), "Identifier '" + name + "' is undefined." };
          }
          valueStack_.back() = value->second;
        } break;
        case OpCode::Equal: {
          const auto rightOperand = pop();
          valueStack_.back() = valueStack_.back() == rightOperand;
        } break;
        case OpCode::NotEqual: {
          const auto rightOperand = pop();
          valueStack_.back() = valueStack_.back() != rightOperand;
        } break;
        case OpCode::Greater: {
          if (peekSecondIs<double>()) {
            const auto rightOperand = popNumberOperand();
            valueStack_.back() = std::get<double>(valueStack_.back()) > rightOperand;
          } else {
            const auto rightOperand = popStringOperand();
            valueStack_.back() = peekStringOperand().compare(rightOperand) > 0;
          }
        } break;
        case OpCode::GreaterEqual: {
          if (peekSecondIs<double>()) {
            const auto rightOperand = popNumberOperand();
            valueStack_.back() = std::get<double>(valueStack_.back()) >= rightOperand;
          } else {
            const auto rightOperand = popStringOperand();
            valueStack_.back() = peekStringOperand().compare(rightOperand) >= 0;
          }
        } break;
        case OpCode::Less: {
          if (peekSecondIs<double>()) {
            const auto rightOperand = popNumberOperand();
            valueStack_.back() = std::get<double>(valueStack_.back()) < rightOperand;
          } else {
            const auto rightOperand = popStringOperand();
            valueStack_.back() = peekStringOperand().compare(rightOperand) < 0;
          }
        } break;
        case OpCode::LessEqual: {
          if (peekSecondIs<double>()) {
            const auto rightOperand = popNumberOperand();
            valueStack_.back() = std::get<double>(valueStack_.back()) <= rightOperand;
          } else {
            const auto rightOperand = popStringOperand();
            valueStack_.back() = peekStringOperand().compare(rightOperand) <= 0;
          }
        } break;
        case OpCode::Add: {
          if (peekIs<std::string>() || peekSecondIs<std::string>()) {
            const auto rightOperand = pop();
            valueStack_.back() = stringify(valueStack_.back()) + stringify(rightOperand);
          } else {
            const auto rightOperand = popNumberOperand();
            valueStack_.back() = peekNumberOperand() + rightOperand;
          }
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
          if (rightOperand == 0) throw LoxError { chunk_->getPosition(offset_), "Cannot divide by zero." };

          valueStack_.back() = peekNumberOperand() / rightOperand;
        } break;
        case OpCode::Negative:
          valueStack_.back() = -peekNumberOperand();
          break;
        case OpCode::Not:
          valueStack_.back() = !isTruthy(valueStack_.back());
          break;
        case OpCode::Print:
          std::cout << stringify(pop()) << '\n';
          break;
        case OpCode::Return:
          return;
      }
    }

    throw std::logic_error { "Chunk ended without a return!" };
  }

  template<typename T>
  bool VM::peekIs() const {
    return std::holds_alternative<T>(valueStack_.back());
  }

  template<typename T>
  bool VM::peekSecondIs() const {
    return std::holds_alternative<T>(valueStack_.crbegin()[1]);
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

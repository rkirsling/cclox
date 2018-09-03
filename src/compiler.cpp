#include "compiler.h"

#include "error-reporter.h"
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Lox {
  std::unique_ptr<Chunk> Compiler::compile(const std::string& source, unsigned line) {
    scanner_.initialize(source, line);
    chunk_ = std::make_unique<Chunk>();
    advance();

    try {
      parseExpression();
      expect(TokenType::Eof, "Unexpected continuation of input.");
      emit(OpCode::Return, peek_);
    } catch (const LoxError& error) {
      errorReporter_.report(error);
    }

    return std::move(chunk_);
  }

  void Compiler::emit(OpCode opCode, const Token& token) {
    chunk_->write(opCode, token);
  }

  void Compiler::emit(Value value, const Token& token) {
    const auto index = chunk_->addConstant(value);
    if (index > std::numeric_limits<unsigned char>::max()) {
      throw std::overflow_error { "Too many constants in one chunk!" };
    }

    chunk_->write(OpCode::Constant, token);
    chunk_->write(static_cast<std::byte>(index));
  }

  void Compiler::parseExpression() {
    parseEquality();
  }

  void Compiler::parseEquality() {
    static const OperatorMap equalities {
      { TokenType::EqualEqual, OpCode::Equal },
      { TokenType::BangEqual, OpCode::NotEqual },
    };
    parseBinary(&Compiler::parseComparison, equalities);
  }

  void Compiler::parseComparison() {
    static const OperatorMap comparisons {
      { TokenType::Greater, OpCode::Greater },
      { TokenType::GreaterEqual, OpCode::GreaterEqual },
      { TokenType::Less, OpCode::Less },
      { TokenType::LessEqual, OpCode::LessEqual }
    };
    parseBinary(&Compiler::parseAdditive, comparisons);
  }

  void Compiler::parseAdditive() {
    static const OperatorMap additives {
      { TokenType::Plus, OpCode::Add },
      { TokenType::Minus, OpCode::Subtract }
    };
    parseBinary(&Compiler::parseMultiplicative, additives);
  }

  void Compiler::parseMultiplicative() {
    static const OperatorMap multiplicatives {
      { TokenType::Star, OpCode::Multiply },
      { TokenType::Slash, OpCode::Divide }
    };
    parseBinary(&Compiler::parseUnary, multiplicatives);
  }

  void Compiler::parseBinary(const CompilerMethod& parseOperand, const OperatorMap& operators) {
    parseOperand(this);

    for (;;) {
      const auto op = operators.find(peek_.type);
      if (op == operators.cend()) return;

      const auto token = advance();
      parseOperand(this);
      emit(op->second, token);
    }
  }

  void Compiler::parseUnary() {
    static const OperatorMap unaries {
      { TokenType::Minus, OpCode::Negative },
      { TokenType::Bang, OpCode::Not }
    };

    const auto op = unaries.find(peek_.type);
    if (op == unaries.cend()) return parsePrimary();

    const auto token = advance();
    parseUnary();
    emit(op->second, token);
  }

  void Compiler::parsePrimary() {
    switch (peek_.type) {
      case TokenType::LeftParen:
        parseParenthesized();
        return;
      case TokenType::Nil:
        emit(OpCode::Nil, advance());
        return;
      case TokenType::True:
        emit(OpCode::True, advance());
        return;
      case TokenType::False:
        emit(OpCode::False, advance());
        return;
      case TokenType::Number:
        parseNumber();
        return;
      default:
        throw LoxError {
          peek_,
          isAtEnd() ? "Unexpected end of input." : "Unexpected token '" + std::string { peek_.lexeme } + "'."
        };
    }
  }

  void Compiler::parseParenthesized() {
    parseExpression();
    expect(TokenType::RightParen, "Expected ')'.");
  }

  void Compiler::parseNumber() {
    const auto number = std::strtod(peek_.lexeme.data(), nullptr);
    const auto token = advance();
    emit(number, token);
  }

  bool Compiler::isAtEnd() const {
    return peekIs(TokenType::Eof);
  }

  bool Compiler::peekIs(TokenType type) const {
    return peek_.type == type;
  }

  Token Compiler::advance() {
    const auto token = peek_;
    for (;;) {
      peek_ = scanner_.scanToken();
      if (!peekIs(TokenType::Error)) return token;

      error();
    }
  }

  bool Compiler::advanceIf(TokenType type) {
    const auto isMatch = peekIs(type);
    if (isMatch) advance();

    return isMatch;
  }

  void Compiler::expect(TokenType type, std::string&& errorMessage) {
    if (!peekIs(type)) throw LoxError { peek_, std::move(errorMessage) };

    advance();
  }

  void Compiler::error() const {
    errorReporter_.report(peek_.line, peek_.column, peek_.lexeme.data());
  }
}

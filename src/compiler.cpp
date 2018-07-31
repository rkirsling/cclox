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
      emit(OpCode::Return, peek_.line);
    } catch (const LoxError& error) {
      errorReporter_.report(error);
    }

    return std::move(chunk_);
  }

  void Compiler::emit(OpCode opCode, unsigned line) {
    chunk_->write(opCode, line);
  }

  void Compiler::emit(double value, unsigned line) {
    const auto index = chunk_->addConstant(value);
    if (index > std::numeric_limits<unsigned char>::max()) {
      throw std::overflow_error("Too many constants in one chunk!");
    }

    chunk_->write(OpCode::Constant, line);
    chunk_->write(static_cast<std::byte>(index));
  }

  void Compiler::parseExpression() {
    parseAdditive();
  }

  void Compiler::parseAdditive() {
    parseBinary(
      &Compiler::parseMultiplicative,
      { { TokenType::Plus, OpCode::Add }, { TokenType::Minus, OpCode::Subtract } }
    );
  }

  void Compiler::parseMultiplicative() {
    parseBinary(
      &Compiler::parseUnary,
      { { TokenType::Star, OpCode::Multiply }, { TokenType::Slash, OpCode::Divide } }
    );
  }

  void Compiler::parseBinary(const CompilerMethod& parseOperand, const OperatorMap& operators) {
    parseOperand(this);

    for (;;) {
      const auto op = operators.find(peek_.type);
      if (op == operators.end()) return;

      const auto line = advance();
      parseOperand(this);
      emit(op->second, line);
    }
  }

  void Compiler::parseUnary() {
    if (!peekIs(TokenType::Minus)) return parsePrimary();

    const auto line = advance();
    parseUnary();
    emit(OpCode::Negative, line);
  }

  void Compiler::parsePrimary() {
    if (advanceIf(TokenType::LeftParen)) return parseParenthesized();

    if (peekIs(TokenType::Number)) return parseNumber();

    throw LoxError {
      peek_,
      isAtEnd() ? "Unexpected end of input." : "Unexpected token '" + std::string { peek_.lexeme } + "'."
    };
  }

  void Compiler::parseParenthesized() {
    parseExpression();
    expect(TokenType::RightParen, "Expected ')'.");
  }

  void Compiler::parseNumber() {
    const auto value = std::strtod(peek_.lexeme.data(), nullptr);
    const auto line = advance();
    emit(value, line);
  }

  bool Compiler::isAtEnd() const {
    return peekIs(TokenType::Eof);
  }

  bool Compiler::peekIs(TokenType type) const {
    return peek_.type == type;
  }

  unsigned Compiler::advance() {
    const auto line = peek_.line;
    for (;;) {
      peek_ = scanner_.scanToken();
      if (!peekIs(TokenType::Error)) return line;

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

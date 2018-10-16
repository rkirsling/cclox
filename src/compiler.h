#pragma once

#include "chunk.h"
#include "scanner.h"
#include "token.h"
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Lox {
  class ErrorReporter;

  class Compiler {
  public:
    explicit Compiler(ErrorReporter& errorReporter)
      : errorReporter_(errorReporter) {}

    std::unique_ptr<Chunk> compile(std::string_view source, unsigned line);

  private:
    using CompilerMethod = std::function<void(Compiler*)>;
    using OperatorMap = std::unordered_map<TokenType, OpCode>;

    void emit(OpCode opCode, const Token& token);
    void emit(Value&& value, const Token& token);

    void parseExpression();
    void parseEquality();
    void parseComparison();
    void parseAdditive();
    void parseMultiplicative();
    void parseBinary(const CompilerMethod& parseOperand, const OperatorMap& operators);
    void parseUnary();
    void parsePrimary();
    void parseParenthesized();
    void parseString();
    void parseNumber();

    constexpr bool isAtEnd() const;
    constexpr bool peekIs(TokenType type) const;
    Token advance();
    bool advanceIf(TokenType type);
    void expect(TokenType type, std::string&& errorMessage);

    constexpr void error() const;

    ErrorReporter& errorReporter_;

    Scanner scanner_ {};
    std::unique_ptr<Chunk> chunk_;
    Token peek_ { TokenType::Eof, {}, 0, 0 };
  };
}

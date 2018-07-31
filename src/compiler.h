#pragma once

#include "chunk.h"
#include "scanner.h"
#include "token.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace Lox {
  class ErrorReporter;

  class Compiler {
  public:
    explicit Compiler(ErrorReporter& errorReporter)
      : errorReporter_(errorReporter) {}

    std::unique_ptr<Chunk> compile(const std::string& source, unsigned line);

  private:
    using CompilerMethod = std::function<void(Compiler*)>;
    using OperatorMap = std::unordered_map<TokenType, OpCode>;

    void emit(OpCode opCode, unsigned line);
    void emit(double value, unsigned line);

    void parseExpression();
    void parseAdditive();
    void parseMultiplicative();
    void parseBinary(const CompilerMethod& parseOperand, const OperatorMap& operators);
    void parseUnary();
    void parsePrimary();
    void parseParenthesized();
    void parseNumber();

    bool isAtEnd() const;
    bool peekIs(TokenType type) const;
    unsigned advance();
    bool advanceIf(TokenType type);
    void expect(TokenType type, std::string&& errorMessage);

    void error() const;

    ErrorReporter& errorReporter_;

    Scanner scanner_ {};
    std::unique_ptr<Chunk> chunk_;
    Token peek_ { TokenType::Eof, {}, 0, 0 };
  };
}

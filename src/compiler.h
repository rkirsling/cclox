#pragma once

#include "chunk.h"
#include "scanner.h"
#include "token.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

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

    struct Instruction {
      OpCode opCode;
      Token token;
      std::optional<std::byte> argument;
    };

    void emit(OpCode opCode, const Token& token, std::optional<std::byte> argument = std::nullopt);
    void emitPendingInstruction();
    void emitConstant(Value&& value, const Token& token);

    void declareLocal(const Token& identifier);
    void definePendingLocal();
    void resolveLocal(const Token& identifier);
    void popScopeLocals();

    void parseStatement(bool inBlock = false);
    void parseVariable();
    void parseNonDeclaration();
    void parseBlock();
    void parsePrint();
    void parseExpressionStatement();

    void parseExpression();
    void parseAssignment();
    void parseEquality();
    void parseComparison();
    void parseAdditive();
    void parseMultiplicative();
    void parseBinary(const CompilerMethod& parseOperand, const OperatorMap& operators);
    void parseUnary();
    void parsePrimary();
    void parseParenthesized();
    void parseIdentifier();
    void parseString();
    void parseNumber();

    constexpr bool isAtEnd() const;
    constexpr bool peekIs(TokenType type) const;
    Token advance();
    bool advanceIf(TokenType type);
    void expect(TokenType type, std::string&& errorMessage);
    void expectSemicolon();
    Token expectIdentifier();

    void synchronizeStatement(bool inBlock);

    constexpr void error() const;

    ErrorReporter& errorReporter_;

    Scanner scanner_ {};
    std::unique_ptr<Chunk> chunk_;
    std::vector<std::pair<std::string_view, unsigned>> locals_ {};

    std::optional<Instruction> pendingInstruction_;
    std::optional<std::string_view> pendingLocal_;

    Token peek_ { TokenType::Eof, {}, 0, 0 };
    unsigned scopeDepth_ { 0 };
  };
}

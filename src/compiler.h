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

    void reset();

  private:
    using CompilerMethod = std::function<void(Compiler*)>;
    using OperatorMap = std::unordered_map<TokenType, OpCode>;

    struct Instruction {
      OpCode opCode;
      Token token;
      std::optional<std::byte> argument;
    };

    void emit(OpCode opCode, const Token& token, std::optional<std::byte> argument = std::nullopt);
    void emitPendingGet();
    void emitConstant(Value&& value, const Token& token);
    void emitPop();

    constexpr size_t target() const noexcept { return chunk_->size(); }
    size_t emitJump(OpCode opCode, const Token& token);
    void patchJump(size_t offset);
    void emitLoop(size_t offset, const Token& token);

    void declareLocal(const Token& identifier);
    void definePendingLocal();
    void resolveLocal(const Token& identifier);
    void beginScope();
    void endScope();

    void parseStatement(bool inBlock = false);
    void parseVariable();
    void parseNonDeclaration();
    void parseBreak();
    void parseFor();
    void parseWhile();
    void parseIf();
    void parseBlock();
    void parsePrint();
    void parseExpressionStatement();

    void parseExpression();
    void parseAssignment();
    void parseTernary();
    void parseOr();
    void parseAnd();
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

    std::vector<size_t> unpatchedBreaks_;

    std::optional<Instruction> pendingGet_;
    std::optional<std::string_view> pendingLocal_;

    Token peek_ { TokenType::Eof, {}, 0, 0 };
    unsigned scopeDepth_ { 0 };
    unsigned loopDepth_ { 0 };
  };
}

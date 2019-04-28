#include "compiler.h"

#include "error-reporter.h"
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace Lox {
  static const std::unordered_set<TokenType> statementOpeners {
    TokenType::LeftBrace,
    TokenType::Break,
    TokenType::Class,
    TokenType::Fun,
    TokenType::For,
    TokenType::If,
    TokenType::Print,
    TokenType::Return,
    TokenType::Var,
    TokenType::While
  };

  std::unique_ptr<Chunk> Compiler::compile(std::string_view source, unsigned line) {
    scanner_.initialize(source, line);
    chunk_ = std::make_unique<Chunk>();
    advance();

    while (!isAtEnd()) parseStatement();

    emit(OpCode::Return, peek_);
    return std::move(chunk_);
  }

  void Compiler::emit(OpCode opCode, const Token& token, std::optional<std::byte> argument) {
    if (pendingInstruction_) emitPendingInstruction();

    chunk_->write(opCode, token);
    if (argument) chunk_->write(*argument);
  }

  void Compiler::emitPendingInstruction() {
    chunk_->write(pendingInstruction_->opCode, pendingInstruction_->token);
    if (pendingInstruction_->argument) chunk_->write(*pendingInstruction_->argument);

    pendingInstruction_.reset();
  }

  void Compiler::emitConstant(Value&& value, const Token& token) {
    const auto index = chunk_->addConstant(std::move(value));
    if (index > std::numeric_limits<unsigned char>::max()) {
      throw std::overflow_error { "Too many constants in one chunk!" };
    }

    emit(OpCode::Constant, token, static_cast<std::byte>(index));
  }

  void Compiler::declareLocal(const Token& identifier) {
    if (locals_.size() == std::numeric_limits<unsigned char>::max()) {
      throw std::overflow_error { "Too many locals in one function!" };
    }

    for (auto it = locals_.crbegin(); it != locals_.crend() && it->second == scopeDepth_; ++it) {
      if (it->first == identifier.lexeme) {
        throw LoxError {
          identifier,
          "Identifier '" + std::string { identifier.lexeme } + "' is already declared in this scope."
        };
      }
    }

    pendingLocal_ = identifier.lexeme;
  }

  void Compiler::definePendingLocal() {
    locals_.emplace_back(*pendingLocal_, scopeDepth_);
    pendingLocal_.reset();
  }

  void Compiler::resolveLocal(const Token& identifier) {
    if (pendingLocal_ && *pendingLocal_ == identifier.lexeme) {
      pendingLocal_.reset();
      throw LoxError {
        identifier,
        "Identifier '" + std::string { identifier.lexeme } + "' is referenced in its own declaration."
      };
    }

    for (auto i = locals_.size(); i-- > 0;) {
      if (locals_[i].first == identifier.lexeme) {
        pendingInstruction_ = { OpCode::GetLocal, identifier, static_cast<std::byte>(i) };
        return;
      }
    }
  }

  void Compiler::popScopeLocals() {
    while (locals_.size() > 0 && locals_.back().second == scopeDepth_) {
      locals_.pop_back();
      emit(OpCode::Pop, peek_);
    }
  }

  void Compiler::parseStatement(bool inBlock) {
    try {
      switch (peek_.type) {
        case TokenType::Var:
          parseVariable();
          return;
        default:
          parseNonDeclaration();
          return;
      }
    } catch (const LoxError& error) {
      errorReporter_.report(error);
      synchronizeStatement(inBlock);
    }
  }

  void Compiler::parseVariable() {
    const auto keyword = advance();
    const auto identifier = expectIdentifier();
    if (!scopeDepth_) {
      emitConstant(std::string { identifier.lexeme }, identifier);
    } else {
      declareLocal(identifier);
    }

    if (advanceIf(TokenType::Equal)) {
      parseExpression();
    } else {
      emit(OpCode::Nil, peek_);
    }

    expectSemicolon();
    if (!scopeDepth_) {
      emit(OpCode::DefineGlobal, keyword);
    } else {
      definePendingLocal();
    }
  }

  void Compiler::parseNonDeclaration() {
    switch (peek_.type) {
      case TokenType::LeftBrace:
        parseBlock();
        return;
      case TokenType::Print:
        parsePrint();
        return;
      default:
        parseExpressionStatement();
        return;
    }
  }

  void Compiler::parseBlock() {
    advance();
    scopeDepth_++;

    static constexpr auto inBlock = true;
    while (!peekIs(TokenType::RightBrace) && !isAtEnd()) parseStatement(inBlock);

    popScopeLocals();
    scopeDepth_--;
    expect(TokenType::RightBrace, "Expected '}'.");
  }

  void Compiler::parsePrint() {
    const auto token = advance();
    parseExpression();
    expectSemicolon();
    emit(OpCode::Print, token);
  }

  void Compiler::parseExpressionStatement() {
    parseExpression();
    emit(OpCode::Pop, peek_);
    expectSemicolon();
  }

  void Compiler::parseExpression() {
    parseAssignment();
  }

  void Compiler::parseAssignment() {
    parseEquality();
    if (!peekIs(TokenType::Equal)) return;

    const auto op = advance();
    if (!pendingInstruction_) throw LoxError { op, "Invalid left-hand side of assignment." };

    const auto opCode = pendingInstruction_->opCode;
    const auto argument = *pendingInstruction_->argument;
    pendingInstruction_.reset();

    parseAssignment();
    if (opCode == OpCode::GetGlobal) {
      emit(OpCode::SetGlobal, op);
    } else {
      emit(OpCode::SetLocal, op, argument);
    }
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
      case TokenType::Identifier:
        parseIdentifier();
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
      case TokenType::String:
        parseString();
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
    advance();
    parseExpression();
    expect(TokenType::RightParen, "Expected ')'.");
  }

  void Compiler::parseIdentifier() {
    const auto identifier = advance();
    if (pendingInstruction_) emitPendingInstruction();

    resolveLocal(identifier);
    if (pendingInstruction_) return;

    emitConstant(std::string { identifier.lexeme }, identifier);
    pendingInstruction_ = { OpCode::GetGlobal, identifier, std::nullopt };
  }

  void Compiler::parseString() {
    const auto string = std::string { peek_.lexeme.cbegin() + 1, peek_.lexeme.cend() - 1 };
    const auto token = advance();
    emitConstant(string, token);
  }

  void Compiler::parseNumber() {
    const auto number = std::strtod(peek_.lexeme.data(), nullptr);
    const auto token = advance();
    emitConstant(number, token);
  }

  constexpr bool Compiler::isAtEnd() const {
    return peekIs(TokenType::Eof);
  }

  constexpr bool Compiler::peekIs(TokenType type) const {
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

  void Compiler::expectSemicolon() {
    if (!peekIs(TokenType::Semicolon)) throw LoxError { peek_, "Expected ';'." };

    advance();
  }

  Token Compiler::expectIdentifier() {
    if (!peekIs(TokenType::Identifier)) throw LoxError { peek_, "Expected variable name." };

    return advance();
  }

  void Compiler::synchronizeStatement(bool inBlock) {
    for (;; advance()) {
      const auto isSynchronized =
        isAtEnd() ||
        advanceIf(TokenType::Semicolon) ||
        (peekIs(TokenType::RightBrace) && inBlock) ||
        statementOpeners.find(peek_.type) != statementOpeners.cend();

      if (isSynchronized) break;
    }
  }

  constexpr void Compiler::error() const {
    errorReporter_.report(peek_.line, peek_.column, peek_.lexeme.data());
  }
}

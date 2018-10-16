#pragma once

#include "token.h"
#include <functional>
#include <string_view>

namespace Lox {
  class Scanner {
  public:
    void initialize(std::string_view source, unsigned line);

    Token scanToken();

  private:
    Token scanString();
    Token scanNumber();
    Token scanIdentifierOrKeyword();

    constexpr Token token(TokenType type) const;
    constexpr Token eofToken() const;
    constexpr Token errorToken(std::string_view message) const;

    constexpr std::string_view lexeme() const;

    constexpr bool isAtEnd() const;
    constexpr char peek() const;
    constexpr char peekSecond() const;
    char advance();
    bool advanceIf(char expected);
    bool advanceTo(char expected);
    bool advanceTo(char expected, char expectedSecond);
    void advanceWhile(const std::function<bool(char)>& predicate);

    std::string_view source_ {};

    unsigned offset_ { 0 };
    unsigned line_ { 1 };
    unsigned lineStart_ { 0 };
    unsigned tokenOffset_ { 0 };
    unsigned tokenLine_ { 1 };
    unsigned tokenColumn_ { 1 };
  };
}

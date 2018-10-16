#pragma once

#include "token.h"
#include <exception>
#include <string>
#include <utility>

namespace Lox {
  struct LoxError : public std::exception {
    LoxError(const Token& token, std::string&& message)
      : line(token.line), column(token.column), message(std::move(message)) {}

    LoxError(std::pair<unsigned, unsigned> position, std::string&& message)
      : line(position.first), column(position.second), message(std::move(message)) {}

    const unsigned line;
    const unsigned column;
    const std::string message;
  };

  class ErrorReporter {
  public:
    constexpr unsigned errorCount() const { return errorCount_; }

    void report(const LoxError& error, bool isDynamic = false);
    void report(unsigned line, unsigned column, const std::string& message, bool isDynamic = false);
    void displayErrorCount() const;
    void reset();

  private:
    unsigned errorCount_ { 0 };
  };
}

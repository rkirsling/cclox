#pragma once

#include "token.h"
#include <exception>
#include <string>
#include <utility>

namespace Lox {
  struct LoxError : public std::exception {
    LoxError(const Token& _token, std::string&& _message)
      : token(_token), message(std::move(_message)) {}

    const Token token;
    const std::string message;
  };

  class ErrorReporter {
  public:
    unsigned errorCount() const { return errorCount_; }

    void report(const LoxError& error, bool isDynamic = false);
    void report(unsigned line, unsigned column, const std::string& message, bool isDynamic = false);
    void displayErrorCount() const;
    void reset();

  private:
    unsigned errorCount_ { 0 };
  };
}

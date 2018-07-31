#include "error-reporter.h"

#include <iomanip>
#include <iostream>

namespace {
  constexpr auto resetText = "\033[0m";
  constexpr auto redText = "\033[31m";
  constexpr auto greyText = "\033[90m";
}

namespace Lox {
  void ErrorReporter::report(const LoxError& error, bool isDynamic) {
    report(error.token.line, error.token.column, error.message, isDynamic);
  }

  void ErrorReporter::report(unsigned line, unsigned column, const std::string& message, bool isDynamic) {
    const auto stage = isDynamic ? "runtime" : "syntax";
    std::cerr
      << redText << std::setw(8) << stage << " error  " << resetText
      << message
      << greyText << " (" << line << ':' << column << ")\n" << resetText;

    errorCount_++;
  }

  void ErrorReporter::displayErrorCount() const {
    const auto suffix = errorCount_ == 1 ? "" : "s";
    std::cerr << errorCount_ << " error" << suffix << " identified.\n";
  }

  void ErrorReporter::reset() {
    errorCount_ = 0;
  }
}

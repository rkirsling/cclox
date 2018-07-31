#pragma once

#include "compiler.h"
#ifndef NDEBUG
#include "debug.h"
#endif
#include "error-reporter.h"
#include <functional>
#include <string>
#include <vector>

namespace Lox {
  class Chunk;

  enum class ResultStatus {
    OK,
    StaticError,
    DynamicError
  };

  class VM {
  public:
    ResultStatus interpret(const std::string& source, unsigned line);

  private:
    ResultStatus execute(const Chunk& chunk);
    void performBinaryOp(const std::function<double(double, double)>& op);

#ifndef NDEBUG
    ChunkPrinter chunkPrinter_ {};
#endif
    ErrorReporter errorReporter_ {};
    Compiler compiler_ { errorReporter_ };
    std::vector<double> valueStack_ {};
  };
}

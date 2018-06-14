#pragma once

#include "compiler.h"
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

    Compiler compiler_ {};
    std::vector<double> valueStack_ {};
  };
}

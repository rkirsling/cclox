#pragma once

#include "chunk.h"
#include "compiler.h"
#ifndef NDEBUG
#include "debug.h"
#endif
#include "error-reporter.h"
#include <string>
#include <vector>

namespace Lox {
  enum class ResultStatus {
    OK,
    StaticError,
    DynamicError
  };

  class VM {
  public:
    ResultStatus interpret(const std::string& source, unsigned line);

  private:
    void execute();

    template<typename T> bool peekIs() const;
    Value pop();

    template<typename T> T expect(std::string&& errorMessage, bool shouldPop);
    double peekNumberOperand() { return expect<double>("Operand must be a number.", false); }
    double popNumberOperand() { return expect<double>("Operand must be a number.", true); }

    ErrorReporter errorReporter_ {};
    Compiler compiler_ { errorReporter_ };
    std::vector<Value> valueStack_ {};
#ifndef NDEBUG
    ChunkPrinter chunkPrinter_ {};
#endif

    std::unique_ptr<Chunk> chunk_;
    size_t offset_ { 0 };
  };
}

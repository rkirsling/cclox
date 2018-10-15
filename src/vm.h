#pragma once

#include "chunk.h"
#include "compiler.h"
#ifndef NDEBUG
#include "debug.h"
#endif
#include "error-reporter.h"
#include <string>
#include <string_view>
#include <vector>

namespace Lox {
  enum class ResultStatus {
    OK,
    StaticError,
    DynamicError
  };

  class VM {
  public:
    ResultStatus interpret(std::string_view source, unsigned line);

  private:
    void execute();

    template<typename T> bool peekIs() const;
    template<typename T> bool peekSecondIs() const;
    Value pop();

    template<typename T> T expect(std::string&& errorMessage, bool shouldPop);
    double peekNumberOperand() { return expect<double>("Operand must be a number.", false); }
    double popNumberOperand() { return expect<double>("Operand must be a number.", true); }
    std::string peekStringOperand() { return expect<std::string>("Operand must be a string.", false); }
    std::string popStringOperand() { return expect<std::string>("Operand must be a string.", true); }

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

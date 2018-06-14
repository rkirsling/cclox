#include "vm.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace Lox;

namespace {
  // based on Unix sysexits.h
  constexpr auto successCode = 0;
  constexpr auto usageErrorCode = 64;
  constexpr auto staticErrorCode = 65;
  constexpr auto dynamicErrorCode = 70;
  constexpr auto ioErrorCode = 74;

  VM vm {};
}

int run(const std::string& source, unsigned line = 1) {
  const auto status = vm.interpret(source, line);
  return
    status == ResultStatus::StaticError ? staticErrorCode :
    status == ResultStatus::DynamicError ? dynamicErrorCode : successCode;
}

int runFile(const char* path) {
  std::ifstream input { path };
  if (!input) {
    std::cerr << "Could not open file: " << path << '\n';
    return ioErrorCode;
  }

  std::string source { std::istreambuf_iterator<char> { input }, {} };
  return run(source);
}

int runPrompt() {
  std::cout << "This is a minimal Lox REPL for debug use.\n";
  std::cout << "* Statements may not include line breaks.\n";
  std::cout << "* Standalone expressions are not allowed.\n\n";

  std::string source;
  for (auto line = 1u; ; ++line) {
    std::cout << "cclox:" << line << "> ";
    std::getline(std::cin, source);
    run(source, line);
  }
}

int main(int argc, char** argv) {
  if (argc > 2) {
    std::cerr << "Usage: cclox [<path>]\n";
    return usageErrorCode;
  }

  return argc == 2 ? runFile(argv[1]) : runPrompt();
}

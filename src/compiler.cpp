#include "compiler.h"

namespace Lox {
  void Compiler::compile(const std::string& source, unsigned line) {
    scanner_.initialize(source, line);

    // temporary
    for (;;) {
      const auto token = scanner_.scanToken();
      printf("(%u,%u) %s (%i)\n", token.line, token.column, token.lexeme.data(), token.type);

      if (token.type == TokenType::Eof) break;
    }
  }
}

#pragma once

#include "scanner.h"
#include <string>

namespace Lox {
  class Compiler {
  public:
    void compile(const std::string& source, unsigned line);

  private:
    Scanner scanner_ {};
  };
}

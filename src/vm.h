#include <functional>
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
    ResultStatus interpret(const Chunk& chunk);

  private:
    inline void performBinaryOp(const std::function<double(double, double)>& op);

    std::vector<double> valueStack_ {};
  };
}

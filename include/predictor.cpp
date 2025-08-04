#include <cstdint>
class Predictor {
public:
  Predictor() = default;
  ~Predictor() = default;
  bool predict(uint32_t pc) {
    return true;
  }
};
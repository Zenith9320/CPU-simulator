#include <cstdint>
#include <vector>

const int REG_SIZE = 32;

class RegisterFile {
 private:
  std::vector<uint32_t> reg;     // 实际的寄存器值
  std::vector<int> reorder;      // ROB中最后一次写这个寄存器的ID（-1表示无重命名）

 public:
  RegisterFile() {
    reg.resize(REG_SIZE, 0);
    reorder.resize(REG_SIZE, -1);
  }

  ~RegisterFile() = default;

  bool is_pending(uint32_t reg) {
    return reorder[reg] != -1;
  }

  void set(uint32_t index, uint32_t data) {
    if (index == 0) return;
    reg[index] = data;
  }

  void set_reorder(uint32_t index, int rob_id) {
    if (index == 0) return;
    reorder[index] = rob_id;
  }

  bool is_ready(uint32_t index) const {
    return reorder[index] == -1;
  }

  int get_reorder(uint32_t index) const {
    return reorder[index];
  }

  uint32_t read_unsigned(uint32_t index) const {
    return reg[index];
  }

  int32_t read_signed(uint32_t index) const {
    return static_cast<int32_t>(reg[index]);
  }

  void clear_reorder(uint32_t index) {
    if (index == 0) return;
    reorder[index] = -1;
  }

  bool if_equal(uint32_t id1, uint32_t id2) const {
    return reg[id1] == reg[id2];
  }

  bool if_geq_unsigned(uint32_t id1, uint32_t id2) const {
    return reg[id1] >= reg[id2];
  }

  bool if_geq_signed(uint32_t id1, uint32_t id2) const {
    return static_cast<int32_t>(reg[id1]) >= static_cast<int32_t>(reg[id2]);
  }

  bool if_less_unsigned(uint32_t id1, uint32_t id2) const {
    return reg[id1] < reg[id2];
  }

  bool if_less_signed(uint32_t id1, uint32_t id2) const {
    return static_cast<int32_t>(reg[id1]) < static_cast<int32_t>(reg[id2]);
  }

  void reset() {
    for (int i = 0; i < REG_SIZE; ++i) {
      reg[i] = 0;
      reorder[i] = -1;
    }
  }
};

#include <cstdint>
#include "memory.cpp"
#include "register.cpp"
#include "instruction.cpp"

enum ROB_State {
  ISSUE, COMMIT, WRITE_RESULT, EXECUTE
};

struct ROB_Entry {
  uint32_t ID;
  bool busy;
  ROB_State state;
  uint32_t instruction;
  uint32_t destination;
  uint32_t value;

  bool is_branch;
  bool is_taken;
  bool predicted_taken; 
  uint32_t predicted_pc;

  ROB_Entry() = default;

  ROB_Entry(bool b, ROB_State s, uint32_t id, uint32_t d, uint32_t v,
            bool is_b = false, bool taken = false, bool pred_taken = false, uint32_t pred_pc = 0)
      : busy(b), state(s), ID(id), destination(d), value(v),
        is_branch(is_b), is_taken(taken), predicted_taken(pred_taken), predicted_pc(pred_pc) {}
};

class ROB {
 private:
  std::vector<ROB_Entry> entries;
  uint32_t capacity;
  uint32_t head = 0;
  uint32_t tail = 0;
  uint32_t size = 0;
  Instruction input = Instruction();

 public:
  ROB() : capacity(1024), entries(1024) {}
  ROB(uint32_t c) : capacity(c), entries(c) {}

  bool is_full() const { return size == capacity; }
  bool is_empty() const { return size == 0; }
  bool has_free_entry() const { return !is_full(); }

  int allocate_reg(uint32_t inst, RegisterFile regs) {
    Instruction instruction = Instruction(inst);
    auto dest = instruction.get_rd();
    int rob_id = allocate(inst, dest);
    regs.set_reorder(dest, rob_id);
    return rob_id;
  }

  int allocate(uint32_t ins, uint32_t dest,
               bool is_branch = false, bool is_taken = false,
               bool predicted_taken = false, uint32_t predicted_pc = 0) {
    if (is_full()) return -1;

    ROB_Entry entry(true, ROB_State::ISSUE, tail, dest, 0,
                    is_branch, is_taken, predicted_taken, predicted_pc);
    entry.instruction = ins;
    entries[tail] = entry;

    int allocated_id = tail;
    tail = (tail + 1) % capacity;
    size++;
    return allocated_id;
  }

  int get_cur_id() {
    return (tail + capacity - 1) % capacity;
  }

  ROB_Entry& get_entry(uint32_t index) {
    return entries[index];
  }

  void write_result(uint32_t rob_id, uint32_t res) {
    if (rob_id >= capacity || !entries[rob_id].busy) {
      throw std::runtime_error("Invalid ROB ID or entry not busy");
    }
    entries[rob_id].value = res;
    entries[rob_id].state = ROB_State::WRITE_RESULT;
  }

  bool check_mispredict(uint32_t& correct_pc) {
    if (!is_empty()) {
      ROB_Entry &entry = entries[head];
      if (entry.is_branch && entry.state == ROB_State::WRITE_RESULT) {
        if (entry.is_taken != entry.predicted_taken) {
          correct_pc = entry.is_taken ? entry.value : entry.predicted_pc;
          return true;
        }
      }
    }
    return false;
  }

  bool ready_to_commit() const {
    return (!is_empty()) && (entries[head].state == ROB_State::WRITE_RESULT);
  }

  std::tuple<uint32_t, uint32_t, uint32_t> commit() {
    if (!ready_to_commit()) {
      throw std::runtime_error("No instruction ready to commit");
    }

    ROB_Entry &entry = entries[head];
    entry.state = ROB_State::COMMIT;
    entry.busy = false;

    uint32_t rob_id = entry.ID;
    uint32_t value = entry.value;
    uint32_t dest = entry.destination;

    head = (head + 1) % capacity;
    size--;

    return {rob_id, value, dest};
  }

  void flush() {
    for (auto& entry : entries) {
      entry.busy = false;
    }
    head = tail;
    size = 0;
  }

  void set_input(Instruction& a) {
    input = a;
  }

  Instruction get_input() {
    return input;
  }

  void run(Memory& mem, RegisterFile& reg) {
    uint32_t correct_PC;
    if (check_mispredict(correct_PC)) {
      flush();
      mem.set_PC(correct_PC);
    }

    if (ready_to_commit()) {
      auto [rob_id, value, dest] = commit();

      if (dest != 0) {
        reg.set(dest, value);
        reg.clear_reorder(dest);
      }
    }
  }
};

#include <vector>
#include <optional>
#include <cstdint>
#include <string>
#include "ALU.cpp"

struct RS_Entry {
  uint32_t instruction = 0;
  bool busy = false;
  uint32_t Vj = 0, Vk = 0;    // 操作数值
  uint32_t Qj = 0, Qk = 0;    // 操作数依赖的ROB编号，0表示无依赖
  uint32_t ROB_ID = 0;
  bool if_executed = false;
  RS_Entry() = default;

  RS_Entry(uint32_t i, bool b, uint32_t vj, uint32_t vk, uint32_t qj, uint32_t qk, uint32_t robid)
          : instruction(i), busy(b), Vj(vj), Vk(vk), Qj(qj), Qk(qk), ROB_ID(robid), if_executed(false) {};
};

class ReservationStation {
private:
  std::vector<RS_Entry> entries;
  uint32_t capacity;
  ALU* alu = nullptr;
  uint32_t instFromROB = 0;
  int idFromROB = 0;
  RegisterFile* rf;
  
  std::optional<RS_Entry> current;  // 当前执行的指令

public:
  ReservationStation() : entries(1024), capacity(1024) {}
  ReservationStation(uint32_t c) : entries(c), capacity(c) {}

  void set_alu(ALU* a) { alu = a; }
  void set_rf(RegisterFile* RF) { rf = RF; }

  // 插入一个新条目，成功返回true，满了返回false
  bool insert(const RS_Entry& entry) {
    for (auto& e : entries) {
      if (!e.busy) {
        e = entry;
        return true;
      }
    }
    return false;
  }

  bool insert_inst(Instruction inst, ROB& rob, RegisterFile& regs) {
    if (is_full()) return false;
    uint32_t rd = inst.get_rd();
    uint32_t rs1 = inst.get_rs1();
    uint32_t rs2 = inst.get_rs2();

    uint32_t Vj = 0, Vk = 0;
    uint32_t Qj = 0, Qk = 0;

    if (rf->is_pending(rs1)) {
      Qj = rf->get_reorder(rs1);
    } else {
      Vj = rf->read_unsigned(rs1);
    }

    if (rf->is_pending(rs2)) {
      Qk = rf->get_reorder(rs2);
    } else {
      Vk = rf->read_unsigned(rs2);
    }

    int rob_id = rob.allocate_reg(inst.code, regs);
    if (rob_id == -1) return false;

    regs.set_reorder(rd, rob_id);

    RS_Entry entry(inst.code, true, Vj, Vk, Qj, Qk, rob_id);
    return insert(entry);
  }

  std::optional<RS_Entry> get_ready_entry() {
    for (auto& entry : entries) {
      if (entry.busy && entry.Qj == 0 && entry.Qk == 0 && !entry.if_executed) {
        return entry;
      }
    }
    return std::nullopt;
  }

  bool has_ready_entry() const {
    for (const auto& entry : entries) {
      if (entry.busy && entry.Qj == 0 && entry.Qk == 0 && !entry.if_executed) {
        return true;
      }
    }
    return false;
  }

  void remove(uint32_t id) {
    for (auto& entry : entries) {
      if (entry.busy && entry.ROB_ID == id) {
        entry.busy = false;
        entry.if_executed = false;
        break;
      }
    }
  }

  void update_operand(uint32_t rob_id, uint32_t value) {
    for (auto& e : entries) {
      if (e.busy) {
        if (e.Qj == rob_id) {
          e.Vj = value;
          e.Qj = 0;
        }
        if (e.Qk == rob_id) {
          e.Vk = value;
          e.Qk = 0;
        }
      }
    }
  }

  bool is_full() const {
    for (const auto& e : entries) {
      if (!e.busy) {
        return false;
      }
    }
    return true;
  }

  bool has_free_entry() const {
    return !is_full();
  }

  void run() {
    //ROB有指令传入，RS插入依赖关系
    if (instFromROB != 0) {
      auto inst = Instruction(instFromROB);
      auto rs1 = inst.get_rs1();
      auto rs2 = inst.get_rs2();
      auto e = RS_Entry(instFromROB, true, 0, 0, rf->get_reorder(rs1), rf->get_reorder(rs2), idFromROB);
      instFromROB = 0;
    }

    auto ready = get_ready_entry();
    if (!ready.has_value()) return;

    RS_Entry& e = ready.value();
    std::string op = Instruction(e.instruction).get_op();

    bool is_alu_op =
        (op == "slti" || op == "addi" || op == "sltiu" || op == "xori" || op == "ori" || op == "andi" ||
         op == "slli" || op == "srli" || op == "srai" || op == "add" || op == "sub" || op == "sll" ||
         op == "slt" || op == "sltu" || op == "xor" || op == "srl" || op == "sra" || op == "or" || op == "and" || op == "lui");

    if (is_alu_op && alu) {
      alu->set_input(e.instruction);
      e.if_executed = true;
      return;
    }

    bool is_lsb_op =
        (op == "lb" || op == "lbu" || op == "lh" || op == "lhu" || op == "lw" || op == "sb" ||
         op == "sh" || op == "sw");
  }

  void update() {
    if (!current.has_value()) return;
    uint32_t id = current->ROB_ID;
    for (auto& e : entries) {
      if (e.Qj == id) e.Qj = 0;
      if (e.Qk == id) e.Qk = 0;
    }
    current.reset();
  }
};

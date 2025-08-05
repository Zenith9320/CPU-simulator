#include <vector>
#include <optional>
#include <cstdint>
#include <string>
#include "ROB.cpp"

enum LSB_Op {
  LB, LBU, LH, LHU, LW, SB, SH, SW
};

struct LSB_Entry {
  bool busy = false;
  LSB_Op op;
  uint32_t ROB_ID;
  uint32_t addr = 0;
  uint32_t Vj = 0;
  uint32_t Qj = 0;  // 写入指令依赖的ROB_ID，0表示无依赖
  uint32_t A = 0;   // 偏移量
  uint32_t value = 0;  // store指令要写入内存的值
  uint32_t Q_val = 0;  // store指令依赖的ROB号，0表示无依赖
  int execution_cycle = 0;  // 执行周期计数
  uint32_t instruction;

  LSB_Entry() = default;

  LSB_Entry(LSB_Op op_, uint32_t rob_id, uint32_t vj, uint32_t qj, uint32_t a, uint32_t inst,
            uint32_t val = 0, uint32_t q_val = 0)
      : busy(true), op(op_), ROB_ID(rob_id), Vj(vj), Qj(qj), A(a),
        value(val), Q_val(q_val), execution_cycle(0), instruction(inst) {}
};

class LoadStoreBuffer {
private:
  std::vector<std::optional<LSB_Entry>> entries;
  uint32_t capacity;
  uint32_t size = 0;

public:
  LoadStoreBuffer() : entries(1024), capacity(1024) {}
  LoadStoreBuffer(uint32_t c) : entries(c), capacity(c) {}

  bool insert_inst(Instruction inst, RegisterFile& regs, ROB& rob) {
    uint32_t instruction = inst.code;
    std::string op = inst.get_op();
    uint32_t rd = inst.get_rd();
    uint32_t rs1 = inst.get_rs1();
    uint32_t rs2 = inst.get_rs2();
    int32_t imm = 0;
    LSB_Op lsb_op;
    bool is_store = false;
    bool is_load = false;
    if (op == "lb") {
      lsb_op = LSB_Op::LB;
      is_load = true;
      imm = inst.get_i_imm();
    } else if (op == "lh") {
      lsb_op = LSB_Op::LH;
      is_load = true;
      imm = inst.get_i_imm();
    } else if (op == "lw") {
      lsb_op = LSB_Op::LW;
      is_load = true;
      imm = inst.get_i_imm();
    } else if (op == "lbu") {
      lsb_op = LSB_Op::LBU; 
      is_load = true;
      imm = inst.get_i_imm();
    } else if (op == "lhu") {
      lsb_op = LSB_Op::LHU;
      is_load = true;
      imm = inst.get_i_imm();
    } else if (op == "sb") {
      lsb_op = LSB_Op::SB;
      is_store = true;
      imm = inst.get_s_imm();
    } else if (op == "sh") {
      lsb_op = LSB_Op::SH;
      is_store = true;
      imm = inst.get_s_imm();
    } else if (op == "sw") {
      lsb_op = LSB_Op::SW;
      is_store = true;
      imm = inst.get_s_imm();
    }
    if (rob.is_full()) return false;
    int rob_id = rob.allocate(instruction, rd);
    if (rob_id == -1) return false;

    if (is_load) regs.set_reorder(rd, rob_id);
    uint32_t vj = 0, qj = 0;
    if (!regs.is_pending(rs1)) {
      vj = regs.read_unsigned(rs1);
      qj = 0;
    } else {
      qj = regs.get_reorder(rs1);
    }
    uint32_t val = 0, q_val = 0;
    if (is_store) {
      if (!regs.is_pending(rs2)) {
        val = regs.read_unsigned(rs2);
        q_val = 0;
      } else {
        q_val = regs.get_reorder(rs2);
      }
    }

    LSB_Entry entry(lsb_op, rob_id, vj, qj, imm, val, q_val, instruction);

    return insert(entry);
  }

  bool insert(const LSB_Entry& entry) {
    for (auto& e : entries) {
      if (!e.has_value()) {
        e = entry;
        calculate_address(*e);
        size++;
        return true;
      }
    }
    return false;
  }

  void calculate_address(LSB_Entry& e) {
    e.addr = e.Vj + e.A;
  }

  void update_operand(uint32_t rob_id, uint32_t val) {
    for (auto& e : entries) {
      if (e.has_value()) {
        if (e->Qj == rob_id) {
          e->Vj = val;
          e->Qj = 0;
          calculate_address(*e);
        }
        if (e->Q_val == rob_id) {
          e->value = val;
          e->Q_val = 0;
        }
      }
    }
  }

  std::optional<LSB_Entry> get_ready_entry() {
    for (auto& e : entries) {
      if (e.has_value() && e->busy) {
        if ((e->op >= LB && e->op <= LW) && e->Qj == 0) {
          return e;
        }
        if ((e->op >= SB && e->op <= SW) && e->Qj == 0 && e->Q_val == 0) {
          return e;
        }
      }
    }
    return std::nullopt;
  }

  void remove(uint32_t rob_id) {
    for (auto& e : entries) {
      if (e.has_value() && e->ROB_ID == rob_id) {
        e.reset();
        size--;
        return;
      }
    }
  }

  bool is_full() const {
    return size == capacity;
  }

  bool has_free_entry_for(Instruction& ins) {
    std::string op = ins.get_op();
    if (op == "lb" || op == "lh" || op == "lw" || op == "lbu" || op == "lhu" ||
        op == "sb" || op == "sh" || op == "sw") {
      return !is_full();
    }
    return true;
  }

  int execute(LSB_Entry& e, const Instruction& ins, Memory& mem, ROB& rob) {
    e.execution_cycle++;
    if (e.execution_cycle < 3) {
      return e.execution_cycle;
    }

    switch (e.op) {
      case LB: {
        int8_t byte = static_cast<int8_t>(mem.read_byte(e.addr));
        uint32_t data = static_cast<uint32_t>(byte);
        rob.write_result(e.ROB_ID, data);
        break;
      }
      case LBU: {
        uint32_t data = static_cast<uint32_t>(mem.read_byte(e.addr));
        rob.write_result(e.ROB_ID, data);
        break;
      }
      case LH: {
        int16_t half = static_cast<int16_t>(mem.read_halfword(e.addr));
        uint32_t data = static_cast<uint32_t>(half);
        rob.write_result(e.ROB_ID, data);
        break;
      }
      case LHU: {
        uint32_t data = static_cast<uint32_t>(mem.read_halfword(e.addr));
        rob.write_result(e.ROB_ID, data);
        break;
      }
      case LW: {
        uint32_t data = mem.read_word(e.addr);
        rob.write_result(e.ROB_ID, data);
        break;
      }
      case SB: {
        mem.write_byte(e.addr, static_cast<uint8_t>(e.value & 0xFF));
        rob.write_result(e.ROB_ID, 0);
        break;
      }
      case SH: {
        mem.write_byte(e.addr, static_cast<uint8_t>(e.value & 0xFF));
        mem.write_byte(e.addr + 1, static_cast<uint8_t>((e.value >> 8) & 0xFF));
        rob.write_result(e.ROB_ID, 0);
        break;
      }
      case SW: {
        mem.write_byte(e.addr, static_cast<uint8_t>(e.value & 0xFF));
        mem.write_byte(e.addr + 1, static_cast<uint8_t>((e.value >> 8) & 0xFF));
        mem.write_byte(e.addr + 2, static_cast<uint8_t>((e.value >> 16) & 0xFF));
        mem.write_byte(e.addr + 3, static_cast<uint8_t>((e.value >> 24) & 0xFF));
        rob.write_result(e.ROB_ID, 0);
        break;
      }
      default:
        throw std::runtime_error("Unknown LSB operation");
    }
    e.execution_cycle = 0;
    return 3;
  }

  void run(Memory& mem, ROB& rob) {
    for (auto& e_opt : entries) {
      if (!e_opt.has_value()) continue;
      auto& e = e_opt.value();

      bool operands_ready = (e.Qj == 0) && (e.op >= LB && e.op <= LW);
      if (e.busy && operands_ready) {
        Instruction inst(e.instruction);
        int stage = execute(e, inst, mem, rob);
        if (stage == 3) {
          remove(e.ROB_ID);
        }
      }
    }
  }
};

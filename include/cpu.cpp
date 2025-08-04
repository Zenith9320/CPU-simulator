#include "ReservationStation.cpp"
#include <iostream>

class CPU {
 private:
  Memory mem;
  RegisterFile regs;
  ROB rob;
  ReservationStation RS;
  LoadStoreBuffer LSB;
  ALU alu;
 public:
  CPU() = default;
  ~CPU() = default;

  void set_PC(uint32_t pos) {
    mem.set_PC(pos);
  }

  void beq(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (regs.if_equal(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void bne(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (!regs.if_equal(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void bge(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (regs.if_geq_signed(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void bgeu(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (regs.if_geq_unsigned(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void blt(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (regs.if_less_signed(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void bltu(uint32_t rs1, uint32_t rs2, int32_t offset) {
    if (regs.if_less_unsigned(rs1, rs2)) {
      mem.modify_PC(offset);
    }
  }

  void lui(uint32_t rd, uint32_t imm) {
    regs.set(rd, imm << 12);
  }

  void auipc(uint32_t rd, uint32_t imm) {
    regs.set(rd, (imm << 12) + mem.get_PC());
  }

  void jal(uint32_t rd, int32_t offset) {
    regs.set(rd, mem.get_PC() + 4);
    mem.modify_PC(offset);
  }

  void jalr(uint32_t rd, uint32_t rs1, int32_t offset) {
    regs.set(rd, mem.get_PC() + 4);
    mem.set_PC((regs.read_unsigned(rs1) + offset) & ~1u);
  }

  void lb(uint32_t rd, uint32_t rs1, int32_t offset) {
    int8_t byte = static_cast<int8_t>(mem.read_byte(regs.read_unsigned(rs1) + offset));
    uint32_t data = static_cast<uint32_t>(byte);//先符号扩展再转化成无符号32位
    regs.set(rd, data);
  }

  void lbu(uint32_t rd, uint32_t rs1, int32_t offset) {
    uint32_t data = static_cast<uint32_t>(mem.read_byte(regs.read_unsigned(rs1) + offset));
    regs.set(rd, data);
  }

  void lh(uint32_t rd, uint32_t rs1, int32_t offset) {
    int16_t halfword = static_cast<int16_t>(mem.read_halfword(regs.read_unsigned(rs1) + offset));
    uint32_t data = static_cast<uint32_t>(halfword);
    regs.set(rd, data);
  }

  void lhu(uint32_t rd, uint32_t rs1, int32_t offset) {
    uint32_t data = static_cast<uint32_t>(mem.read_halfword(regs.read_unsigned(rs1) + offset));
    regs.set(rd, data);
  }

  void lw(uint32_t rd, uint32_t rs1, int32_t offset) {
    regs.set(rd, mem.read_word(regs.read_unsigned(rs1) + offset));
  }

  void sb(uint32_t rs1, uint32_t rs2, int32_t offset) {
    uint8_t data = static_cast<uint8_t>(regs.read_unsigned(rs2) & 0xFF);
    mem.write_byte(regs.read_unsigned(rs1) + offset, data);
  }

  void sh(uint32_t rs1, uint32_t rs2, int32_t offset) {
    uint16_t data = static_cast<uint16_t>(regs.read_unsigned(rs2) & 0xFFFF);
    mem.write_byte(regs.read_unsigned(rs1) + offset, static_cast<uint8_t>(data & 0xFF));
    mem.write_byte(regs.read_unsigned(rs1) + offset + 1, static_cast<uint8_t>((data >> 8) & 0xFF));
  }

  void sw(uint32_t rs1, uint32_t rs2, int32_t offset) {
    uint32_t data = regs.read_unsigned(rs2);
    mem.write_byte(regs.read_unsigned(rs1) + offset, static_cast<uint8_t>(data & 0xFF));
    mem.write_byte(regs.read_unsigned(rs1) + offset + 1, static_cast<uint8_t>((data >> 8) & 0xFF));
    mem.write_byte(regs.read_unsigned(rs1) + offset + 2, static_cast<uint8_t>((data >> 16) & 0xFF));
    mem.write_byte(regs.read_unsigned(rs1) + offset + 3, static_cast<uint8_t>((data >> 24) & 0xFF));
  }

  void cpu_write_byte(uint32_t addr, uint8_t byte) {
    mem.write_byte(addr, byte);
  }

  void addi(uint32_t rd, uint32_t rs1, int32_t imm) {
    regs.set(rd, alu.compute("add", static_cast<int32_t>(regs.read_unsigned(rs1)), imm));
  }

  void slti(uint32_t rd, uint32_t rs1, int32_t imm) {
    int32_t rs1_data = static_cast<int32_t>(regs.read_unsigned(rs1));
    uint32_t new_data = (rs1_data < imm) ? 1 : 0;
    regs.set(rd, new_data);
  }

  void sltiu(uint32_t rd, uint32_t rs1, uint32_t imm) {
    uint32_t rs1_data = regs.read_unsigned(rs1);
    uint32_t new_data = (rs1_data < imm) ? 1 : 0;
    regs.set(rd, new_data);
  }

  void xori(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("xor", regs.read_unsigned(rs1), imm));
  }

  void ori(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("or", regs.read_unsigned(rs1), imm));
  }

  void andi(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("and", regs.read_unsigned(rs1), imm));
  }

  void slli(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("sll", regs.read_unsigned(rs1), imm));
  }

  void srli(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("srl", regs.read_unsigned(rs1), imm));
  }

  void srai(uint32_t rd, uint32_t rs1, uint32_t imm) {
    regs.set(rd, alu.compute("sra", static_cast<int32_t>(regs.read_unsigned(rs1)), imm));
  }

  void add_op(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int32_t a = static_cast<int32_t>(regs.read_unsigned(rs1));
    int32_t b = static_cast<int32_t>(regs.read_unsigned(rs2));
    regs.set(rd, static_cast<uint32_t>(alu.compute("add", a, b)));
  }

  void sub_op(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int32_t a = static_cast<int32_t>(regs.read_unsigned(rs1));
    int32_t b = static_cast<int32_t>(regs.read_unsigned(rs2));
    regs.set(rd, static_cast<uint32_t>(alu.compute("sub", a, b)));
  }

  void sll_op(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("sll", a, b)));
  }

  void slt_op(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    int32_t b = static_cast<int32_t>(regs.read_unsigned(rs2));
    regs.set(rd, static_cast<uint32_t>(alu.compute("slt", a, b)));
  }

  void sltu_op(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("sltu", a, b)));
  }

  void cpu_xor(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("xor", a, b)));
  }

  void cpu_srl(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("srl", a, b)));
  }

  void cpu_sra(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("sra", a, b)));
  }

  void cpu_and(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("and", a, b)));
  }

  void cpu_or(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    uint32_t a = regs.read_unsigned(rs1);
    uint32_t b = regs.read_unsigned(rs2);
    regs.set(rd, static_cast<uint32_t>(alu.compute("or", a, b)));
  }

  void cpu_set_PC(uint32_t addr) {
    mem.set_PC(addr);
  }

  uint32_t get_PC() {
    return mem.get_PC();
  }

  uint32_t get_instruction() {
    //std::cout << "PC: " << mem.get_PC() << std::endl;
    //std::cout << "instruction: " << mem.read_word(mem.get_PC()) << std::endl;
    return mem.read_word(mem.get_PC());
  }

  uint8_t cpu_get_byte(uint32_t pos) {
    return mem.read_byte(pos);
  }

  void execute(uint32_t instruction) {
    if (instruction == 0x0FF00513) {
      uint32_t res = regs.read_unsigned(10);
      std::cout << (res & 0xFF) << std::endl;
      exit(0);
    }
    Instruction ins(instruction);
    std::string operation = ins.get_op();
    if (instruction != 0) std::cout << "instruction: " << std::bitset<32>(instruction) << std::endl;
    if (instruction != 0) std::cout << "pos: " << std::bitset<32>(mem.get_PC()) << std::endl;    
    if (instruction != 0) std::cout << "op: " << operation<< std::endl;
    if (operation == "lb") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      lb(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "lh") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      lh(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "lw") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      lw(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "lbu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      lbu(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "lhu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      lhu(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "lui") {
      uint32_t rd = ins.get_rd();
      uint32_t imm = ins.get_u_imm();
      lui(rd, imm);
    } else if (operation == "auipc") {
      uint32_t rd = ins.get_rd();
      uint32_t imm = ins.get_u_imm();
      auipc(rd, imm);
    } else if (operation == "jal") {
      uint32_t rd = ins.get_rd();
      uint32_t offset = ins.get_jal_imm();
      jal(rd, static_cast<int32_t>(offset));
    } else if (operation == "jalr") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      jalr(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "beq") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      beq(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "bne") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      bne(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "blt") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      blt(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "bge") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      bge(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "bltu") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      bltu(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "bgeu") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_b_imm();
      bgeu(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "sb") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_s_imm();
      sb(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "sh") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_s_imm();
      sh(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "sw") {
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      uint32_t imm = ins.get_s_imm();
      sw(rs1, rs2, static_cast<int32_t>(imm));
    } else if (operation == "addi") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      addi(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "slti") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      slti(rd, rs1, static_cast<int32_t>(imm));
    } else if (operation == "sltiu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      sltiu(rd, rs1, imm);
    } else if (operation == "xori") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      xori(rd, rs1, imm);
    } else if (operation == "ori") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      ori(rd, rs1, imm);
    } else if (operation == "andi") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
      andi(rd, rs1, imm);
    } else if (operation == "slli") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
      slli(rd, rs1, imm);
    } else if (operation == "srli") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
      srli(rd, rs1, imm);
    } else if (operation == "srai") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
      srai(rd, rs1, imm);
    } else if (operation == "add") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      add_op(rd, rs1, rs2);
    } else if (operation == "sub") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      sub_op(rd, rs1, rs2);
    } else if (operation == "sll") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      sll_op(rd, rs1, rs2);
    } else if (operation == "slt") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      slt_op(rd, rs1, rs2);
    } else if (operation == "sltu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      sltu_op(rd, rs1, rs2);
    } else if (operation == "xor") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      cpu_xor(rd, rs1, rs2);
    } else if (operation == "srl") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      cpu_srl(rd, rs1, rs2);
    } else if (operation == "sra") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      cpu_sra(rd, rs1, rs2);
    } else if (operation == "or") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      cpu_or(rd, rs1, rs2);
    } else if (operation == "and") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
      cpu_and(rd, rs1, rs2);
    }
  }

  void cpu_reset() {
    regs.reset();
  }

  void fetch() {
    uint8_t raw_inst1 = mem.read_byte(mem.get_PC());
    uint8_t raw_inst2 = mem.read_byte(mem.get_PC() + 1);    
    uint8_t raw_inst3 = mem.read_byte(mem.get_PC() + 2);
    uint8_t raw_inst4 = mem.read_byte(mem.get_PC() + 3);
    uint32_t raw_inst = (raw_inst4 << 24) | (raw_inst3 << 16) | (raw_inst2 << 8) | raw_inst1;
    Instruction inst = Instruction(raw_inst);
    rob.set_input(inst);
  }

  bool is_memory(Instruction& inst) {
    std::string op = inst.get_op();
    if (op == "lb" || op == "lh" || op == "lw" || op == "sb" || op == "sw" || op == "sh"
      || op == "lhu" || op == "lbu") return true;
    return false;
  }

  bool issue(Instruction& inst,
           ROB& rob, RegisterFile& regs,
           ReservationStation& rs, LoadStoreBuffer& lsb) {
    std::string op = inst.get_op();
    uint32_t rs1 = inst.get_rs1(), rs2 = inst.get_rs2(), rd = inst.get_rd(), imm = inst.get_imm();
    uint32_t raw = inst.code;

    int rob_id = rob.allocate(raw, rd, is_branch(op), false, false, 0);
    if (rob_id == -1) return false;
    if (has_dest(op)) {
        regs.set_reorder(rd, rob_id);
    }

    if (is_memory(inst)) {
      if (!lsb.insert_inst(inst, regs, rob)) return false;
    } else {
      if (!rs.insert_inst(inst, rob, regs)) return false;
    }

    return true;
  }
};
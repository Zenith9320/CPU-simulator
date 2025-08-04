#include <cstdint>
#include <string>

enum class InstType { 
  R, I, S, B, U, J, INVALID
};

enum class OpType {
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
  LUI, AUIPC, LB, LH, LW, LBU, LHU, SB, SH, SW,
  BEQ, BNE, BLT, BGE, BLTU, BGEU, JAL, JALR, INVALID
};

struct Instruction {
  uint32_t code;

  Instruction(uint32_t c): code(c) {}

  uint8_t get_opcode() { 
    return code & 0x7F; 
  }

  char get_type() {
    uint8_t res = code & 0x7F;
    switch(res) {
      case (0b0110011): return 'R'; break;
      case (0b0000011):
      case (0b1100111):
      case (0b0010011): return 'I'; break;
      case (0b0100011): return 'S'; break;
      case (0b0110111):
      case (0b0010111): return 'U'; break;
      case (0b1101111): return 'J'; break;
      case (0b1100011): return 'B'; break;
    }
    return 'X';
  }

  std::string get_op() {
    char type = this->get_type();
    if (type == 'I') {//I has 15 instructions
      uint8_t f7 = this->get_funct7(), f3 = this->get_funct3();
      uint8_t opcode = code & 0x7F;
      if (opcode == 0b0000011) {
        switch(f3) {
          case(0b000): return "lb";
          case(0b100): return "lbu";
          case(0b001): return "lh";
          case(0b101): return "lhu";
          case(0b010): return "lw";
        }
      } else if (opcode == 0b1100111) {
        if (f3 == 0b000) {
          return "jalr";
        }
      } else if (opcode == 0b0010011) {
        switch(f3) {
          case(0b000): return "addi";
          case(0b010): return "slti";
          case(0b011): return "sltiu";
          case(0b100): return "xori";
          case(0b110): return "ori";
          case(0b111): return "andi";
          case(0b001): return "slli";
          case(0b101): {
            if (f7 == 0b0000000) return "srli";
            else return "srai";
          }
        }
      }
    } else if (type == 'R') {//R has 10 instructions
      uint8_t f7 = this->get_funct7(), f3 = this->get_funct3();
      switch (f3) {
        case(0b000): {
          if (f7 == 0b0) {
            return "add";
          } else {
            return "sub";
          }
        }
        case(0b001): return "sll";
        case(0b010): return "slt";
        case(0b011): return "sltu";
        case(0b100): return "xor";
        case(0b101): {
          if (f7 == 0b0) {
            return "srl";
          } else {
            return "sra";
          }
        }
        case(0b110): return "or";
        case(0b111): return "and";
      }
    } else if (type == 'U') {//U has 2 instructions
      uint8_t opcode = code & 0x7F;
      if (opcode == 0b0110111) return "lui";
      else return "auipc";
    } else if (type == 'J') {//J has 1 instruction
      return "jal";
    } else if (type == 'B') {//B has 6 instructions
      uint8_t f7 = this->get_funct7(), f3 = this->get_funct3();
      switch(f3) {
        case(0b000): return "beq";
        case(0b001): return "bne";
        case(0b100): return "blt";
        case(0b101): return "bge";
        case(0b110): return "bltu";
        case(0b111): return "bgeu";
      }
    } else if (type == 'S') {//S has 3 instructions
      uint8_t f7 = this->get_funct7(), f3 = this->get_funct3();
      switch(f3) {
        case(0b000): return "sb";
        case(0b001): return "sh";
        case(0b010): return "sw";
      }
    }
    return "no instruction";
  }

  uint8_t get_funct3() { return (code >> 12) & 0x7; }
  uint8_t get_funct7() { return (code >> 25) & 0x7F; }

  uint32_t get_rd() { return (code >> 7) & 0x1F; }
  uint32_t get_rs1() { return (code >> 15) & 0x1F; }
  uint32_t get_rs2() { return (code >> 20) & 0x1F; }

  int32_t get_i_imm() {
    int32_t imm = static_cast<int32_t>(code) >> 20;
    return imm;
  }

  int32_t get_s_imm() {
    int32_t imm = ((code >> 25) << 5) | ((code >> 7) & 0x1F);
    if (imm & 0x800) imm |= 0xFFFFF000;
    return imm;
  }

  int32_t get_b_imm() {
    int32_t imm = 0;
    imm |= ((code >> 31) & 0x1) << 12;
    imm |= ((code >> 7) & 0x1) << 11;
    imm |= ((code >> 25) & 0x3F) << 5;
    imm |= ((code >> 8) & 0xF) << 1;
    if (imm & (1 << 12)) {
      imm |= 0xFFFFE000;
    }
    return imm;
  }

  uint32_t get_u_imm() {
    uint32_t imm = (code >> 12) << 12;
    return imm;
  }

  int32_t get_j_imm() {
    int32_t imm = 0;
    imm |= ((code >> 31) & 0x1) << 20;
    imm |= ((code >> 21) & 0x3FF) << 1;
    imm |= ((code >> 20) & 0x1) << 11;
    imm |= ((code >> 12) & 0xFF) << 12;
    if (imm & (1 << 20)) {
      imm |= 0xFFF00000;
    }
    return imm;
  }

  int32_t get_shamt() {
    return (code >> 20) & 0x1F;
  }
};

struct DecodedInst {
  OpType op = OpType::INVALID;
  InstType type = InstType::INVALID;

  uint32_t rd = 0;
  uint32_t rs1 = 0;
  uint32_t rs2 = 0;
  int32_t imm = 0;

  bool is_branch = false;
  bool is_jump = false;
  bool is_load = false;
  bool is_store = false;

  bool pred_taken = false;
  uint32_t pred_pc = 0;

  uint32_t raw_code = 0;
};

class Decoder {
public:
  DecodedInst decode(Instruction& inst) {
    DecodedInst d;
    d.raw_code = inst.code;
    d.type = decode_type(inst.get_opcode());
    d.rd = inst.get_rd();
    d.rs1 = inst.get_rs1();
    d.rs2 = inst.get_rs2();

    switch (d.type) {
      case InstType::R: decode_r(inst, d); break;
      case InstType::I: decode_i(inst, d); break;
      case InstType::S: decode_s(inst, d); break;
      case InstType::B: decode_b(inst, d); break;
      case InstType::U: decode_u(inst, d); break;
      case InstType::J: decode_j(inst, d); break;
      default: d.op = OpType::INVALID; break;
    }
    return d;
  }

private:
  InstType decode_type(uint8_t opcode) {
    switch (opcode) {
      case 0b0110011: return InstType::R;
      case 0b0000011:
      case 0b1100111:
      case 0b0010011: return InstType::I;
      case 0b0100011: return InstType::S;
      case 0b1100011: return InstType::B;
      case 0b0110111:
      case 0b0010111: return InstType::U;
      case 0b1101111: return InstType::J;
      default: return InstType::INVALID;
    }
  }

  void decode_r(Instruction& inst, DecodedInst& d) {
    uint8_t f3 = inst.get_funct3();
    uint8_t f7 = inst.get_funct7();
    switch (f3) {
      case 0b000: d.op = (f7 == 0b0000000) ? OpType::ADD : OpType::SUB; break;
      case 0b001: d.op = OpType::SLL; break;
      case 0b010: d.op = OpType::SLT; break;
      case 0b011: d.op = OpType::SLTU; break;
      case 0b100: d.op = OpType::XOR; break;
      case 0b101: d.op = (f7 == 0b0000000) ? OpType::SRL : OpType::SRA; break;
      case 0b110: d.op = OpType::OR; break;
      case 0b111: d.op = OpType::AND; break;
      default: d.op = OpType::INVALID; break;
    }
  }

  void decode_i(Instruction& inst, DecodedInst& d) {
    uint8_t opcode = inst.get_opcode();
    uint8_t f3 = inst.get_funct3();
    uint8_t f7 = inst.get_funct7();

    switch (opcode) {
      case 0b0000011: {
        d.is_load = true;
        switch (f3) {
          case 0b000: d.op = OpType::LB; break;
          case 0b001: d.op = OpType::LH; break;
          case 0b010: d.op = OpType::LW; break;
          case 0b100: d.op = OpType::LBU; break;
          case 0b101: d.op = OpType::LHU; break;
          default: d.op = OpType::INVALID; break;
        }
        d.imm = inst.get_i_imm();
        break;
      }

      case 0b1100111: {
        if (f3 == 0b000) {
          d.op = OpType::JALR;
          d.is_jump = true;
          d.imm = inst.get_i_imm();
        } else {
          d.op = OpType::INVALID;
        }
        break;
      }

      case 0b0010011: {
        switch (f3) {
          case 0b000: d.op = OpType::ADDI; break;
          case 0b010: d.op = OpType::SLTI; break;
          case 0b011: d.op = OpType::SLTIU; break;
          case 0b100: d.op = OpType::XORI; break;
          case 0b110: d.op = OpType::ORI; break;
          case 0b111: d.op = OpType::ANDI; break;
          case 0b001: d.op = OpType::SLLI; d.imm = inst.get_shamt(); break;
          case 0b101:
            if (f7 == 0b0000000)
              d.op = OpType::SRLI;
            else
              d.op = OpType::SRAI;
            d.imm = inst.get_shamt();
            break;
          default:
            d.op = OpType::INVALID;
            break;
        }
        if (d.imm == 0)
          d.imm = inst.get_i_imm();
        break;
      }

      default: {
        d.op = OpType::INVALID;
        break;
      }
    }
  }

  void decode_s(Instruction& inst, DecodedInst& d) {
    d.is_store = true;
    uint8_t f3 = inst.get_funct3();
    switch (f3) {
      case 0b000: d.op = OpType::SB; break;
      case 0b001: d.op = OpType::SH; break;
      case 0b010: d.op = OpType::SW; break;
      default: d.op = OpType::INVALID; break;
    }
    d.imm = inst.get_s_imm();
  }

  void decode_b(Instruction& inst, DecodedInst& d) {
    d.is_branch = true;
    uint8_t f3 = inst.get_funct3();
    switch (f3) {
      case 0b000: d.op = OpType::BEQ; break;
      case 0b001: d.op = OpType::BNE; break;
      case 0b100: d.op = OpType::BLT; break;
      case 0b101: d.op = OpType::BGE; break;
      case 0b110: d.op = OpType::BLTU; break;
      case 0b111: d.op = OpType::BGEU; break;
      default: d.op = OpType::INVALID; break;
    }
    d.imm = inst.get_b_imm();
  }

  void decode_u(Instruction& inst, DecodedInst& d) {
    uint8_t opcode = inst.get_opcode();
    switch (opcode) {
      case 0b0110111: d.op = OpType::LUI; break;
      case 0b0010111: d.op = OpType::AUIPC; break;
      default: d.op = OpType::INVALID; break;
    }
    d.imm = inst.get_u_imm();
  }

  void decode_j(Instruction& inst, DecodedInst& d) {
    d.op = OpType::JAL;
    d.is_jump = true;
    d.imm = inst.get_j_imm();
  }
};

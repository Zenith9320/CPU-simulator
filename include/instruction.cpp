#include <cstdint>
#include <string>
#include <bitset>
struct Instruction {
  uint32_t code;

  Instruction() = default;
  Instruction(uint32_t c): code(c) {};

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

  uint8_t get_funct7() {
    uint8_t res = (code >> 25) & 0x7F;
    return res;
  }

  uint8_t get_funct3() {
    uint8_t res = (code >> 12) & 0x7;
    return res;
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

  uint32_t get_rd() {
    return (code >> 7) & 0x1F;
  }

  uint32_t get_rs1() {
    return (code >> 15) & 0x1F;
  }

  uint32_t get_rs2() {
    return (code >> 20) & 0x1F;
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

  int32_t get_jal_imm() {
    int32_t imm = 0;
    imm |= ((code >> 31) & 0x1) << 20;
    imm |= ((code >> 21) & 0x3FF) << 1;
    imm |= ((code >> 20) & 0x1) << 11;
    imm |= ((code >> 12) & 0xFF) << 12;
    if (imm & (1 << 20)) imm |= 0xFFF00000;
    return imm;
  }

  int32_t get_s_imm() {
    int32_t imm = 0;
    imm |= ((code >> 25) & 0x7F) << 5;
    imm |= ((code >> 7) & 0x1F);
    if (imm & 0x800) imm |= 0xFFFFF000;
    return imm; 
  }

  int32_t get_i_imm() {
    int32_t imm = code >> 20;
    if (imm & 0x800) imm |= 0xFFFFF000;
    return imm;
  }

  int32_t get_shamt() {
    int32_t shamt = ((code >> 20) & 0x1F);
    return shamt;
  }

  int32_t get_imm() {
    char type = get_type();
    switch (type) {
      case 'I': return get_i_imm();
      case 'S': return get_s_imm();
      case 'B': return get_b_imm();
      case 'U': return static_cast<int32_t>(get_u_imm());
      case 'J': return get_jal_imm();
      default: return 0;
  }
}

};

inline bool has_dest(const std::string& op) {
  return !(op == "sb" || op == "sh" || op == "sw" ||
           op == "beq" || op == "bne" || op == "blt" || op == "bge" ||
           op == "bltu" || op == "bgeu");
}

inline bool has_rs1(const std::string& op) {
  return !(op == "lui" || op == "auipc" || op == "jal");
}

inline bool has_rs2(const std::string& op) {
  return (op == "add" || op == "sub" || op == "sll" || op == "slt" ||
          op == "sltu" || op == "xor" || op == "srl" || op == "sra" ||
          op == "or" || op == "and" || op == "beq" || op == "bne" || op == "blt" || op == "bge" ||
          op == "bltu" || op == "bgeu" || op == "sb" || op == "sh" || op == "sw");
}

inline bool is_memory(const std::string& op) {
  return (op == "lb" || op == "lbu" || op == "lh" || op == "lhu" ||
          op == "lw" || op == "sb" || op == "sh" || op == "sw");
}

inline bool is_branch(const std::string& op) {
  return (op == "beq" || op == "bne" || op == "blt" || op == "bge" ||
          op == "bltu" || op == "bgeu" || op == "jal" || op == "jalr");
}


#include <cstdint>
#include <stdexcept>
#include <string>
#include "LSB.cpp"

enum class ALU_Op {
  ALU_ADD, ALU_SUB, ALU_AND, ALU_OR, ALU_XOR, ALU_SLL,
  ALU_SRL, ALU_SRA, ALU_SLT, ALU_SLTU, ALU_ADDI, ALU_ANDI, ALU_ORI,
  ALU_XORI, ALU_SLLI, ALU_SRLI, ALU_SRAI, ALU_SLTI, ALU_SLTIU, ALU_LUI,
  ALU_BEQ, ALU_BNE, ALU_BLT, ALU_BGE, ALU_BLTU, ALU_BGEU, ALU_JAL,
  ALU_JALR, ALU_INVALID
};


class ALU {
private:
  uint32_t inst_from_RS;

public:

  void set_input(uint32_t request) {
    inst_from_RS = request;
  }

  uint32_t compute(const std::string& op, uint32_t a, uint32_t b) {
    if (op == "add" || op == "addi") return a + b;
    if (op == "sub") return a - b;
    if (op == "and" || op == "andi") return a & b;
    if (op == "or"  || op == "ori")  return a | b;
    if (op == "xor" || op == "xori") return a ^ b;
    if (op == "sll" || op == "slli") return a << (b & 0x1F);
    if (op == "srl" || op == "srli") return a >> (b & 0x1F);
    if (op == "sra" || op == "srai") return static_cast<int32_t>(a) >> (b & 0x1F);
    if (op == "slt" || op == "slti") return static_cast<int32_t>(a) < static_cast<int32_t>(b) ? 1 : 0;
    if (op == "sltu"|| op == "sltiu") return a < b ? 1 : 0;
    if (op == "lui") return b << 12;

    throw std::runtime_error("Unsupported ALU op: " + op);
  }

  void execute(Instruction& ins, uint32_t val1, uint32_t val2, int rob_id, ROB& rob) {
    std::string op = ins.get_op();
  
    if (!is_ALU_op(op)) {
      throw std::runtime_error("ALU cannot execute non-ALU op: " + op);
    }

    if (op == "addi") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "slti") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "sltiu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "xori") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "ori") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "andi") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_i_imm();
    } else if (op == "slli") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
    } else if (op == "srli") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
    } else if (op == "srai") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t imm = ins.get_shamt();
    } else if (op == "add") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "sub") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "sll") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "slt") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "sltu") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "xor") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "srl") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "sra") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "or") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    } else if (op == "and") {
      uint32_t rd = ins.get_rd();
      uint32_t rs1 = ins.get_rs1();
      uint32_t rs2 = ins.get_rs2();
    }
  
    uint32_t result = compute(op, val1, val2);
    rob.write_result(rob_id, result);
  }

  bool is_ALU_op(const std::string& op) {
    return op == "add" || op == "addi" || op == "sub" ||
           op == "and" || op == "andi" || op == "or"  || op == "ori" ||
           op == "xor" || op == "xori" || op == "sll" || op == "slli" ||
           op == "srl" || op == "srli" || op == "sra" || op == "srai" ||
           op == "slt" || op == "slti" || op == "sltu" || op == "sltiu" ||
           op == "lui" || op == "jal" || op == "jalr" ||
           op == "beq" || op == "bne" || op == "blt" || op == "bge" || op == "bltu" || op == "bgeu";
  }
};

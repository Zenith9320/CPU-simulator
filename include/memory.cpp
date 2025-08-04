#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <cstdio>

const int MEMORY_SIZE = 1 << 20;

class Memory {
 private:
  uint32_t PC;
  std::unordered_map<uint32_t, uint8_t> memory;

 public:
  Memory() : PC(0x00000000) {}
  ~Memory() = default;

  uint32_t get_PC() const {
    return PC;
  }

  void set_PC(uint32_t tar) {
    PC = tar;
  }

  void modify_PC(int32_t offset) {
    PC += offset;
  }

  void step_PC() {
    PC += 4;
  }

  void write_byte(uint32_t pos, uint8_t val) {
    memory[pos] = val;
  }

  void write_halfword(uint32_t pos, uint16_t val) {
    memory[pos]     = val & 0xFF;
    memory[pos + 1] = (val >> 8) & 0xFF;
  }

  void write_word(uint32_t pos, uint32_t val) {
    memory[pos]     = val & 0xFF;
    memory[pos + 1] = (val >> 8) & 0xFF;
    memory[pos + 2] = (val >> 16) & 0xFF;
    memory[pos + 3] = (val >> 24) & 0xFF;
  }

  uint8_t read_byte(uint32_t pos) const {
    auto it = memory.find(pos);
    return (it != memory.end()) ? it->second : 0;
  }

  uint16_t read_halfword(uint32_t pos) const {
    uint8_t low  = read_byte(pos);
    uint8_t high = read_byte(pos + 1);
    return static_cast<uint16_t>(low | (high << 8));
  }

  uint32_t read_word(uint32_t pos) const {
    return static_cast<uint32_t>(read_byte(pos)) |
           (static_cast<uint32_t>(read_byte(pos + 1)) << 8) |
           (static_cast<uint32_t>(read_byte(pos + 2)) << 16) |
           (static_cast<uint32_t>(read_byte(pos + 3)) << 24);
  }

  int8_t read_byte_signed(uint32_t pos) const {
    return static_cast<int8_t>(read_byte(pos));
  }

  int16_t read_halfword_signed(uint32_t pos) const {
    return static_cast<int16_t>(read_halfword(pos));
  }
};

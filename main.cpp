#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include "include/cpu.cpp"
int main() {
  freopen("testcases/1.out", "w", stdout);
  std::ifstream infile("testcases/array_test1.data");
  std::string s;
  CPU cpu;
  uint32_t store_pos;//扫一遍输入 写入指令的位置
  std::vector<uint8_t> temp_instructions;
  while (std::getline(infile, s)) {
    if (s.empty()) continue;
    if (s[0] == '@') {
      store_pos = static_cast<uint32_t>(std::stoi(s.substr(1), nullptr, 16));
    } else {
      s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
      std::string temp = "";
      for (int i = 0; i < s.length(); i++) {
        if (i % 2 == 0) {
          temp = "";
          temp += s[i];
        } else {
          temp += s[i];
          uint8_t byte = static_cast<uint8_t>(std::stoi(temp, nullptr, 16));
          cpu.cpu_write_byte(store_pos, byte);
          store_pos += 1;
        }
      }
    }
  }
  cpu.cpu_set_PC(0x0);
  while (true) {
    cpu.cpu_reset();
    uint32_t old_pc = cpu.get_PC();
    uint32_t inst = cpu.get_instruction();
    cpu.execute(inst);
    if (cpu.get_PC() == old_pc) {
      cpu.cpu_set_PC(old_pc + 4);
    }
  }
}
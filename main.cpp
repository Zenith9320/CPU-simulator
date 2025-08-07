#include <iostream>
#include <bitset>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include "include/cpu.cpp"
int main() {
  //freopen("testcases/2.out", "w", stdout);
  //std::ifstream infile("testcases/array_test2.data");
  std::string s;
  CPU cpu;
  uint32_t store_pos;//扫一遍输入 写入指令的位置
  std::vector<uint8_t> temp_instructions;
  while (std::getline(std::cin, s)) {
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
  //uint32_t temp = 0;
  //while (temp < store_pos) {
  //  cpu.set_PC(temp);
  //  auto inst = cpu.get_instruction();
  //  if (Instruction(inst).get_op() == "no instruction") {
  //    temp += 4;
  //    continue;
  //  }
  //  std::cout << temp << std::endl;
  //  std::cout << std::bitset<32>(inst) << std::endl;
  //  std::cout << Instruction(inst).get_op() << std::endl;
  //  temp += 4;
  //}
  cpu.cpu_set_PC(0x0);
  while (true) {
    cpu.cpu_reset();
    uint32_t inst = cpu.get_instruction();
    cpu.execute(inst);
  }
}
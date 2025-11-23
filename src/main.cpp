#include "../include/kv_store.h"
#include <iostream>
#include <sstream>
#include <string>


int main() {
  // Khởi tạo FlashKV (Nó sẽ tự load dữ liệu cũ và khởi tạo Cache bên trong)
  KVStore kv;

  std::cout << "=== FlashKV Interactive Console ===" << std::endl;
  std::cout << "Type 'EXIT' to quit." << std::endl;

  std::string line;
  while (true) {
    // 1. In dấu nhắc lệnh
    std::cout << "> ";

    // 2. Đọc dòng lệnh từ bàn phím
    if (!std::getline(std::cin, line) || line == "EXIT") {
      break;
    }
    if (line.empty())
      continue;

    // 3. Phân tích cú pháp (Tách chuỗi)
    // Ví dụ: "SET A 1" -> cmd="SET", arg1="A", arg2="1"
    std::stringstream ss(line);
    std::string cmd, arg1, arg2;
    ss >> cmd >> arg1 >> arg2;

    // 4. Xử lý lệnh
    if (cmd == "SET") {
      if (arg1.empty() || arg2.empty()) {
        std::cout << "[ERROR] Usage: SET <key> <value>" << std::endl;
      } else {
        kv.set(arg1, arg2);
        std::cout << "OK" << std::endl;
      }
    } else if (cmd == "GET") {
      if (arg1.empty()) {
        std::cout << "[ERROR] Usage: GET <key>" << std::endl;
      } else {
        // Khi gọi get, bên dưới nó sẽ tự kiểm tra Cache trước
        auto result = kv.get(arg1);

        if (result.has_value()) {
          std::cout << "\"" << *result << "\"" << std::endl;
        } else {
          std::cout << "(nil)" << std::endl;
        }
      }
    } else if (cmd == "DEL") {
      if (kv.del(arg1)) {
        std::cout << "(integer) 1" << std::endl; // Đã xóa thành công
      } else {
        std::cout << "(integer) 0" << std::endl; // Không tìm thấy để xóa
      }
    } else {
      std::cout << "[ERROR] Unknown command" << std::endl;
    }
  }

  std::cout << "Bye bye!" << std::endl;
  return 0;
}
#include "../include/kv_store.h" // <--- Đã có não
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 4096 // Tăng buffer lên chút

// Hàm phụ trợ: Tách chuỗi (VD: "SET A 1" -> ["SET", "A", "1"])
std::vector<std::string> split_command(const std::string &input) {
  std::vector<std::string> tokens;
  std::stringstream ss(input);
  std::string token;
  while (ss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

int main() {
  // 1. Khởi tạo Database Engine
  KVStore kv;
  std::cout << "[Server] FlashKV Engine Initialized." << std::endl;

  // 2. Setup Winsock & Server (Giống cũ)
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  char opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) ==
      SOCKET_ERROR) {
    std::cerr << "Bind failed. Port busy?" << std::endl;
    return 1;
  }
  listen(server_fd, 3);
  std::cout << "[Server] Listening on port " << PORT << "..." << std::endl;

  // 3. Vòng lặp chính
  int addrlen = sizeof(address);
  SOCKET client_socket =
      accept(server_fd, (struct sockaddr *)&address, &addrlen);

  if (client_socket != INVALID_SOCKET) {
    std::cout << "[Server] Client connected!" << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    std::string line_buffer = "";

    while (true) {
      memset(buffer, 0, BUFFER_SIZE);
      int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
      if (valread <= 0)
        break;

      line_buffer.append(buffer);

      // Xử lý từng dòng lệnh (\n)
      size_t pos = 0;
      while ((pos = line_buffer.find('\n')) != std::string::npos) {
        std::string raw_cmd = line_buffer.substr(0, pos);
        // Xóa \r nếu có (Do Windows gửi \r\n)
        if (!raw_cmd.empty() && raw_cmd.back() == '\r')
          raw_cmd.pop_back();

        std::cout << "[Exec]: " << raw_cmd << std::endl;

        // --- LOGIC XỬ LÝ LỆNH FLASHKV ---
        auto tokens = split_command(raw_cmd);
        std::string response = "ERROR\n"; // Mặc định là lỗi

        if (tokens.empty()) {
          response = "ERROR: Empty command\n";
        } else if (tokens[0] == "SET" && tokens.size() >= 3) {
          kv.set(tokens[1], tokens[2]);
          response = "OK\n";
        } else if (tokens[0] == "GET" && tokens.size() >= 2) {
          auto result = kv.get(tokens[1]);
          if (result.has_value()) {
            response = *result + "\n";
          } else {
            response = "(nil)\n";
          }
        } else if (tokens[0] == "DEL" && tokens.size() >= 2) {
          if (kv.del(tokens[1]))
            response = "(integer) 1\n";
          else
            response = "(integer) 0\n";
        } else {
          response = "ERROR: Unknown command or wrong arguments\n";
        }

        // Gửi kết quả trả về cho Client
        send(client_socket, response.c_str(), response.length(), 0);

        // Cắt phần đã xử lý đi
        line_buffer.erase(0, pos + 1);
      }
    }
    closesocket(client_socket);
  }

  closesocket(server_fd);
  WSACleanup();
  return 0;
}
#include "../include/kv_store.h"
#include <iostream>
#include <sstream>
#include <string>
#include <thread> // <--- THƯ VIỆN ĐA LUỒNG
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 4096

// KVStore bây giờ phải là biến toàn cục hoặc static để các luồng dùng chung
// Vì nó đã Thread-safe (có mutex) nên dùng chung thoải mái!
KVStore kv;

std::vector<std::string> split_command(const std::string &input) {
  std::vector<std::string> tokens;
  std::stringstream ss(input);
  std::string token;

  // FIX NHANH (LEVEL 2): Đọc command đầu tiên
  ss >> token;
  tokens.push_back(token);

  // Nếu là SET, đọc phần Key
  if (token == "SET") {
    std::string key;
    ss >> key;
    tokens.push_back(key);

    // Đọc toàn bộ phần còn lại làm Value (Hỗ trợ khoảng trắng)
    std::string value, temp;
    std::getline(ss, value); // Đọc hết phần sau

    // Xóa khoảng trắng thừa ở đầu do getline để lại
    if (!value.empty() && value[0] == ' ') {
      value.erase(0, 1);
    }
    tokens.push_back(value);
  } else {
    // Các lệnh khác (GET, DEL) xử lý như cũ
    while (ss >> token)
      tokens.push_back(token);
  }
  return tokens;
}

// Hàm xử lý riêng cho từng khách hàng (Chạy trên luồng riêng)
void handle_client(SOCKET client_socket) {
  char buffer[BUFFER_SIZE] = {0};
  std::string line_buffer = "";

  while (true) {
    memset(buffer, 0, BUFFER_SIZE);
    int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (valread <= 0)
      break;

    line_buffer.append(buffer);
    size_t pos = 0;
    while ((pos = line_buffer.find('\n')) != std::string::npos) {
      std::string raw_cmd = line_buffer.substr(0, pos);
      if (!raw_cmd.empty() && raw_cmd.back() == '\r')
        raw_cmd.pop_back();

      // LOG ID của luồng để thấy sự song song
      std::cout << "[Thread " << std::this_thread::get_id()
                << "] Exec: " << raw_cmd << std::endl;

      auto tokens = split_command(raw_cmd);
      std::string response = "ERROR\n";

      // --- LOGIC CŨ ---
      if (tokens.empty())
        response = "ERROR\n";
      else if (tokens[0] == "SET" && tokens.size() >= 3) {
        kv.set(tokens[1], tokens[2]);
        response = "OK\n";
      } else if (tokens[0] == "GET" && tokens.size() >= 2) {
        auto result = kv.get(tokens[1]);
        response = result.has_value() ? (*result + "\n") : "(nil)\n";
      } else if (tokens[0] == "DEL" && tokens.size() >= 2) {
        response = kv.del(tokens[1]) ? "(integer) 1\n" : "(integer) 0\n";
      }

      send(client_socket, response.c_str(), response.length(), 0);
      line_buffer.erase(0, pos + 1);
    }
  }

  closesocket(client_socket);
  std::cout << "[Thread " << std::this_thread::get_id()
            << "] Client disconnected." << std::endl;
}

int main() {
  // ... (Khởi tạo Winsock như cũ) ...
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
      SOCKET_ERROR)
    return 1;
  listen(server_fd, 10); // Tăng hàng đợi lên 10

  std::cout << "[Multi-Threaded Server] Listening on port " << PORT << "..."
            << std::endl;

  while (true) {
    int addrlen = sizeof(address);
    // Chờ kết nối (Main thread bị block ở đây)
    SOCKET client_socket =
        accept(server_fd, (struct sockaddr *)&address, &addrlen);

    if (client_socket != INVALID_SOCKET) {
      std::cout << "[Main] New connection accepted. Spawning worker thread..."
                << std::endl;

      // TẠO LUỒNG MỚI: Giao client_socket cho hàm handle_client xử lý
      std::thread worker(handle_client, client_socket);

      // Detach để luồng chạy độc lập, Main thread quay lại vòng lặp đón khách
      // tiếp
      worker.detach();
    }
  }

  closesocket(server_fd);
  WSACleanup();
  return 0;
}
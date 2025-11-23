#ifndef KV_STORE_H
#define KV_STORE_H

#include "lru_cache.h"
#include <atomic>             // Dùng cho cờ hiệu running
#include <condition_variable> // Dùng để báo hiệu cho luồng log
#include <mutex>
#include <optional>
#include <queue> // Dùng cho hàng đợi log
#include <string>
#include <thread> // Dùng cho luồng ghi log
#include <unordered_map>

class KVStore {
private:
  // 1. Kho dữ liệu chính trên RAM
  std::unordered_map<std::string, std::string> data_map;

  // 2. Tên file log để lưu xuống đĩa
  const std::string DB_FILE = "flashkv.data";

  // 3. Cache LRU (Lớp đệm tốc độ cao)
  LRUCache cache;

  // 4. Mutex bảo vệ Logic nghiệp vụ (Map + Cache)
  std::mutex store_mtx;

  // --- CÁC THÀNH PHẦN ASYNC LOGGING (Mới) ---
  // Hàng đợi chứa các câu lệnh chờ ghi xuống đĩa
  std::queue<std::string> log_queue;

  // Mutex riêng để bảo vệ hàng đợi log (Giúp I/O không chặn Logic chính)
  std::mutex log_mtx;

  // Biến điều kiện để đánh thức luồng logger khi có việc
  std::condition_variable log_cv;

  // Luồng nhân viên chuyên đi ghi log (Chạy ngầm)
  std::thread logger_thread;

  // Cờ hiệu để báo khi nào Server dừng hoạt động
  std::atomic<bool> running;

  // Hàm worker của luồng Logger (Chạy vòng lặp vô tận để ghi đĩa)
  void logger_worker();

  // Hàm đẩy log vào hàng đợi (Thay thế hàm ghi đĩa trực tiếp cũ)
  void _enqueue_log(const std::string &cmd, const std::string &key,
                    const std::string &value);

public:
  KVStore();
  ~KVStore(); // Destructor quan trọng: Để dọn dẹp luồng logger khi tắt server

  void set(const std::string &key, const std::string &value);
  std::optional<std::string> get(const std::string &key);
  bool del(const std::string &key);
  void restore(); // Hàm khôi phục dữ liệu khi khởi động
};

#endif
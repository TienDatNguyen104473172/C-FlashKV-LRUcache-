#include "../include/kv_store.h"
#include <fstream> // Quan trọng: Để đọc ghi file (sửa lỗi ofstream/ifstream)
#include <iostream>
#include <sstream> // Quan trọng: Để xử lý chuỗi (sửa lỗi stringstream)


// Constructor: Khởi động luồng Logger ngay khi KVStore được tạo
KVStore::KVStore() : cache(3), running(true) {
  std::cout << "[KVStore] System Online. Starting Background Logger..."
            << std::endl;
  restore();

  // Tạo một luồng riêng, giao cho nó chạy hàm logger_worker
  logger_thread = std::thread(&KVStore::logger_worker, this);
}

// Destructor: Dọn dẹp sạch sẽ trước khi hủy object
KVStore::~KVStore() {
  std::cout << "[KVStore] Shutting down. Waiting for logger to finish..."
            << std::endl;

  // 1. Báo hiệu dừng
  running = false;

  // 2. Rung chuông đánh thức luồng logger dậy (nếu nó đang ngủ)
  log_cv.notify_one();

  // 3. Chờ luồng logger kết thúc công việc (Join)
  if (logger_thread.joinable()) {
    logger_thread.join();
  }
  std::cout << "[KVStore] Shutdown complete." << std::endl;
}

// --- LOGIC LUỒNG GHI LOG (CHẠY NGẦM) ---
void KVStore::logger_worker() {
  std::ofstream file;
  // Mở file ở chế độ append (ghi nối đuôi)
  file.open(DB_FILE, std::ios::app);

  while (running) {
    std::string log_entry;

    // VÙNG KHÓA: Chỉ giữ khóa lúc lấy log ra khỏi hàng đợi
    {
      std::unique_lock<std::mutex> lock(log_mtx);

      // Nếu hàng đợi rỗng và server vẫn đang chạy -> Đi ngủ (Wait)
      log_cv.wait(lock, [this] { return !log_queue.empty() || !running; });

      // Nếu bị đánh thức dậy mà hàng đợi vẫn rỗng (do server tắt) -> Thoát
      if (log_queue.empty() && !running) {
        break;
      }

      // Lấy log ra
      log_entry = log_queue.front();
      log_queue.pop();
    }
    // Ra khỏi ngoặc -> Tự động nhả khóa log_mtx

    // VÙNG KHÔNG KHÓA: Ghi xuống đĩa
    if (file.is_open()) {
      file << log_entry << "\n";
      file.flush(); // Đẩy dữ liệu xuống đĩa ngay
    }
  }

  if (file.is_open())
    file.close();
}

// Hàm đẩy log vào hàng đợi (Thay thế hàm ghi đĩa trực tiếp cũ)
void KVStore::_enqueue_log(const std::string &cmd, const std::string &key,
                           const std::string &value) {
  // Tạo chuỗi log
  std::string entry = cmd + " " + key + " " + value;

  {
    std::lock_guard<std::mutex> lock(log_mtx);
    log_queue.push(entry);
  }

  // Đánh thức luồng logger
  log_cv.notify_one();
}

// --- CÁC HÀM NGHIỆP VỤ CHÍNH ---

void KVStore::set(const std::string &key, const std::string &value) {
  std::lock_guard<std::mutex> lock(store_mtx); // Lock Store

  data_map[key] = value;
  cache.put(key, value);

  // Đẩy vào hàng đợi (Async)
  _enqueue_log("SET", key, value);
}

bool KVStore::del(const std::string &key) {
  std::lock_guard<std::mutex> lock(store_mtx); // Lock Store

  cache.remove(key);
  bool exists = data_map.erase(key) > 0;

  if (exists) {
    _enqueue_log("DEL", key, "0");
  }
  return exists;
}

std::optional<std::string> KVStore::get(const std::string &key) {
  // 1. Check Cache trước
  auto cached_val = cache.get(key);
  if (cached_val.has_value()) {
    return cached_val;
  }

  // 2. Check Map (Cần lock)
  std::lock_guard<std::mutex> lock(store_mtx);
  if (data_map.find(key) != data_map.end()) {
    std::string val = data_map[key];
    cache.put(key, val); // Update cache
    return val;
  }
  return std::nullopt;
}

// Hàm khôi phục dữ liệu (Đã fix lỗi đọc khoảng trắng)
void KVStore::restore() {
  std::ifstream file(DB_FILE);
  if (!file.is_open())
    return;

  std::string line;
  int count = 0;

  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string cmd, key, value;

    ss >> cmd >> key;

    if (cmd == "SET") {
      // Đọc hết phần còn lại làm value (hỗ trợ space)
      std::getline(ss >> std::ws, value);
      data_map[key] = value;
    } else if (cmd == "DEL") {
      data_map.erase(key);
    }
    count++;
  }
  file.close();
  std::cout << "[Storage] Restored " << count << " operations from disk."
            << std::endl;
}
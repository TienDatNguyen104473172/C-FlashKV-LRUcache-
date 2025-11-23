#include "../include/kv_store.h"
#include <iostream>

// Khởi tạo Cache dung lượng 3
KVStore::KVStore() : cache(3) {
  std::cout << "[KVStore] System Online. LRU Cache Ready." << std::endl;
  restore();
}

void KVStore::restore() {
  std::ifstream file(DB_FILE);
  if (!file.is_open())
    return;

  std::string cmd, key, value;
  while (file >> cmd >> key) {
    if (cmd == "SET") {
      file >> value;
      data_map[key] = value;
      // Không nạp vào cache lúc restore để tránh làm đầy cache ngay lập tức
    } else if (cmd == "DEL") {
      data_map.erase(key);
    }
  }
  file.close();
}

void KVStore::_log_to_disk(const std::string &cmd, const std::string &key,
                           const std::string &value) {
  std::ofstream file(DB_FILE, std::ios::app);
  if (file.is_open()) {
    file << cmd << " " << key << " " << value << "\n";
    file.close();
  }
}

void KVStore::set(const std::string &key, const std::string &value) {
  std::lock_guard<std::mutex> lock(store_mtx);
  data_map[key] = value;
  cache.put(key, value); // Update Cache
  _log_to_disk("SET", key, value);
}

std::optional<std::string> KVStore::get(const std::string &key) {
  // 1. Check Cache (Cache tự lock, không cần store_mtx ở đây cũng được,
  // Tối ưu: Không lock store_mtx khi gọi cache.get()
  auto cached_val = cache.get(key);
  if (cached_val.has_value()) {
    return cached_val;
  }

  // 2. Vào Critical Section của Store
  std::lock_guard<std::mutex> lock(store_mtx);
  if (data_map.find(key) != data_map.end()) {
    std::string val = data_map[key];
    // Unlock store_mtx tự động khi hết hàm, nhưng cache.put sẽ tự lock mutex
    // của cache
    cache.put(key, val);
    return val;
  }
  return std::nullopt;
}

bool KVStore::del(const std::string &key) {
  std::lock_guard<std::mutex> lock(store_mtx);

  cache.remove(key);
  bool exists = data_map.erase(key) > 0;
  if (exists) {
    _log_to_disk("DEL", key, "0");
  }
  return exists;
}

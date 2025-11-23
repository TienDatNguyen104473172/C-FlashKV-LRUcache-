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
  data_map[key] = value;
  cache.put(key, value); // Update Cache
  _log_to_disk("SET", key, value);
}

std::optional<std::string> KVStore::get(const std::string &key) {
  // Check Cache
  auto cached = cache.get(key);
  if (cached.has_value()) {
    std::cout << "[Cache HIT] ";
    return cached;
  }

  // Check DB
  if (data_map.find(key) != data_map.end()) {
    std::cout << "[Cache MISS] ";
    std::string val = data_map[key];
    cache.put(key, val); // Fill Cache
    return val;
  }
  return std::nullopt;
}

bool KVStore::del(const std::string &key) {
  cache.remove(key); // Clear Cache
  bool exists = data_map.erase(key) > 0;
  if (exists) {
    _log_to_disk("DEL", key, "0");
  }
  return exists;
}
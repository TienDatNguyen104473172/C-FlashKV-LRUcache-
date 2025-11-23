#ifndef KV_STORE_H
#define KV_STORE_H

#include "lru_cache.h"
#include <fstream> // MỚI: Thư viện đọc ghi file
#include <optional>
#include <string>
#include <unordered_map>

class KVStore {
private:
  std::unordered_map<std::string, std::string> data_map;

  // Tên file lưu trữ
  const std::string DB_FILE = "flashkv.data";

  LRUCache cache;

  // Hàm nội bộ: Ghi log vào file
  void _log_to_disk(const std::string &cmd, const std::string &key,
                    const std::string &value);

public:
  KVStore(); // Constructor sẽ tự động load dữ liệu cũ

  void set(const std::string &key, const std::string &value);
  std::optional<std::string> get(const std::string &key);
  bool del(const std::string &key);

  // Hàm khôi phục dữ liệu từ file khi khởi động
  void restore();
};

#endif
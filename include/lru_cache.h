#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <mutex> // <--- THÊM
#include <optional>
#include <string>
#include <unordered_map>


class LRUCache {
private:
  size_t capacity;
  std::list<std::pair<std::string, std::string>> items;
  using ListIterator = std::list<std::pair<std::string, std::string>>::iterator;
  std::unordered_map<std::string, ListIterator> lookup;

  // Mutex bảo vệ cấu trúc dữ liệu bên trong LRU
  mutable std::mutex mtx; // <--- THÊM

public:
  LRUCache(size_t cap);
  std::optional<std::string> get(const std::string &key);
  void put(const std::string &key, const std::string &value);
  void remove(const std::string &key);
};

#endif
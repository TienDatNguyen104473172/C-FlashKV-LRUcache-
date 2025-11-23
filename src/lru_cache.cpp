#include "../include/lru_cache.h"

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

std::optional<std::string> LRUCache::get(const std::string &key) {
  // Tự động khóa luồng tại đây. Chỉ 1 người được vào!
  std::lock_guard<std::mutex> lock(mtx);

  auto it = lookup.find(key);
  if (it == lookup.end())
    return std::nullopt;

  // Di chuyển node (Thao tác này rất nguy hiểm nếu không có khóa)
  items.splice(items.begin(), items, it->second);
  return it->second->second;
}

void LRUCache::put(const std::string &key, const std::string &value) {
  std::lock_guard<std::mutex> lock(mtx); // Khóa toàn bộ hàm put

  auto it = lookup.find(key);
  if (it != lookup.end()) {
    items.splice(items.begin(), items, it->second);
    it->second->second = value;
    return;
  }

  if (items.size() == capacity) {
    auto last_pair = items.back();
    lookup.erase(last_pair.first);
    items.pop_back();
  }

  items.push_front({key, value});
  lookup[key] = items.begin();
}

void LRUCache::remove(const std::string &key) {
  std::lock_guard<std::mutex> lock(mtx); // Khóa hàm remove
  auto it = lookup.find(key);
  if (it != lookup.end()) {
    items.erase(it->second);
    lookup.erase(it);
  }
}
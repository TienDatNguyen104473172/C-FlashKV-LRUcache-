#include "../include/lru_cache.h"
#include <iostream>

LRUCache::LRUCache(size_t cap) : capacity(cap) {}

std::optional<std::string> LRUCache::get(const std::string &key) {
  auto it = lookup.find(key);

  // Trường hợp 1: Không tìm thấy trong Cache
  if (it == lookup.end()) {
    return std::nullopt;
  }

  // Trường hợp 2: Tìm thấy -> Phải đưa nó lên đầu danh sách (đánh dấu là mới
  // dùng)
  items.splice(items.begin(), items, it->second);

  // Trả về giá trị
  return it->second->second;
}

void LRUCache::put(const std::string &key, const std::string &value) {
  auto it = lookup.find(key);

  // Trường hợp A: Key đã tồn tại -> Cập nhật value và đưa lên đầu
  if (it != lookup.end()) {
    items.splice(items.begin(), items, it->second);
    it->second->second = value;
    return;
  }

  // Trường hợp B: Key mới
  // B1. Kiểm tra xem đầy chưa?
  if (items.size() == capacity) {
    // Xóa thằng cuối cùng (Lâu nhất không dùng)
    auto last_pair = items.back();
    lookup.erase(last_pair.first); // Xóa khỏi Map
    items.pop_back();              // Xóa khỏi List
  }

  // B2. Thêm mới vào đầu List
  items.push_front({key, value});

  // B3. Lưu địa chỉ của nó vào Map
  lookup[key] = items.begin();
}
// <-- QUAN TRỌNG: Dấu ngoặc đóng hàm put ở đây, có thể bạn đã thiếu nó

void LRUCache::remove(const std::string &key) {
  auto it = lookup.find(key);
  if (it != lookup.end()) {
    items.erase(it->second); // Xóa khỏi List
    lookup.erase(it);        // Xóa khỏi Map
  }
}
#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <list>
#include <optional>
#include <string>
#include <unordered_map>

class LRUCache {
private:
  size_t capacity; // Dung lượng tối đa của Cache

  // 1. LIST: Lưu trữ dữ liệu thực sự theo thứ tự sử dụng
  // Phần tử đầu list (front) = Mới dùng gần đây nhất (Most Recently Used)
  // Phần tử cuối list (back) = Lâu chưa dùng nhất (Least Recently Used) -> Ứng
  // viên bị xóa
  std::list<std::pair<std::string, std::string>> items;

  // 2. MAP: Lưu Key và "địa chỉ" (Iterator) của nó trong List để tìm nhanh O(1)
  using ListIterator = std::list<std::pair<std::string, std::string>>::iterator;
  std::unordered_map<std::string, ListIterator> lookup;

public:
  LRUCache(size_t cap);

  // Lấy dữ liệu: Nếu có thì trả về và đưa lên đầu List
  std::optional<std::string> get(const std::string &key);

  // Thêm/Cập nhật dữ liệu: Nếu đầy thì xóa thằng cuối List đi
  void put(const std::string &key, const std::string &value);

  void remove(const std::string &key); // <-- THÊM DÒNG NÀY
};

#endif
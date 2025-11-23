cmake_minimum_required(VERSION 3.10)

project(FlashKV VERSION 1.0)

# Cấu hình cho clangd (nếu bạn dùng)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Chuẩn C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Thư mục chứa file header (.h)
include_directories(include)

# 1. Chương trình chạy dòng lệnh (Client cũ)
add_executable(FlashKV 
    src/main.cpp 
    src/kv_store.cpp
    src/lru_cache.cpp  
)

# 2. Chương trình Server (MỚI - Đã bao gồm logic Database)
# Lưu ý: Chỉ được khai báo FlashServer 1 lần duy nhất ở đây
add_executable(FlashServer 
    src/server.cpp 
    src/kv_store.cpp 
    src/lru_cache.cpp
)

# Liên kết thư viện mạng cho Server (Bắt buộc trên Windows)
target_link_libraries(FlashServer ws2_32)


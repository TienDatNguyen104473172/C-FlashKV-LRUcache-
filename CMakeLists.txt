cmake_minimum_required(VERSION 3.10)

project(FlashKV VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Báo cho CMake biết thư mục include nằm ở đâu
include_directories(include)

# Thêm kv_store.cpp vào danh sách cần build
add_executable(FlashKV 
    src/main.cpp 
    src/kv_store.cpp
    src/lru_cache.cpp  # <--- THÊM DÒNG NÀY
)
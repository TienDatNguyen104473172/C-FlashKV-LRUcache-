import socket
import threading
import time
import random
import sys

# --- CẤU HÌNH TẤN CÔNG ---
SERVER_IP = 'localhost'
SERVER_PORT = 8888
NUM_THREADS = 100        # Giả lập 100 người dùng cùng lúc
REQUESTS_PER_THREAD = 200 # Mỗi người spam 200 lệnh
# Tổng cộng = 20,000 requests

# Biến đếm toàn cục (Atomic trong Python nhờ GIL, nhưng dùng Lock cho chắc)
lock = threading.Lock()
success_count = 0
fail_count = 0

def attack_server(thread_id):
    global success_count, fail_count
    
    try:
        # 1. Kết nối
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5) # Timeout sau 5s nếu server đơ
        s.connect((SERVER_IP, SERVER_PORT))
        
        for i in range(REQUESTS_PER_THREAD):
            key = f"user_{thread_id}_{i}"
            val = f"data_{random.randint(1000, 9999)}"
            
            try:
                # 2. Gửi lệnh SET
                cmd_set = f"SET {key} {val}\n"
                s.sendall(cmd_set.encode())
                resp_set = s.recv(1024).decode()
                
                # 3. Gửi lệnh GET để kiểm tra lại (Read-after-Write)
                cmd_get = f"GET {key}\n"
                s.sendall(cmd_get.encode())
                resp_get = s.recv(1024).decode()
                
                # Kiểm tra kết quả
                if "OK" in resp_set and val in resp_get:
                    with lock:
                        success_count += 1
                else:
                    with lock:
                        fail_count += 1
                        print(f"[Thread {thread_id}] Logic Error: {resp_set} | {resp_get}")

            except Exception as e:
                with lock:
                    fail_count += 1
                # Nếu mất kết nối thì break luôn
                break
                
        s.close()
        
    except Exception as e:
        print(f"[Thread {thread_id}] Connection Failed: {e}")
        with lock:
            fail_count += REQUESTS_PER_THREAD

# --- CHẠY TEST ---
print(f"---  STARTING STRESS TEST ON FLASHKV ---")
print(f"Target: {SERVER_IP}:{SERVER_PORT}")
print(f"Virtual Users: {NUM_THREADS}")
print(f"Total Requests: {NUM_THREADS * REQUESTS_PER_THREAD * 2} (SET + GET)")
print("-" * 40)

start_time = time.time()
threads = []

# Khởi động đội quân threads
for i in range(NUM_THREADS):
    t = threading.Thread(target=attack_server, args=(i,))
    threads.append(t)
    t.start()

# Chờ tất cả threads hoàn thành
for t in threads:
    t.join()

end_time = time.time()
duration = end_time - start_time
total_ops = success_count * 2 # Nhân 2 vì 1 vòng lặp gồm cả SET và GET

print("-" * 40)
print(f"---  REPORT ---")
print(f"Time Taken: {duration:.2f} seconds")
print(f"Successful Operations: {success_count} cycles (SET+GET)")
print(f"Failed Operations: {fail_count}")
print(f" THROUGHPUT (RPS): {total_ops / duration:.2f} requests/second")

if fail_count == 0:
    print(" STATUS: PASSED - Server is Rock Solid!")
else:
    print(" STATUS: FAILED - Data loss or crashes detected.")
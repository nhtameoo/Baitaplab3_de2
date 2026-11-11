# Baitaplab3_de2
# Hệ thống giám sát vườn cây công nghiệp
 Họ và tên: Hoàng Nhất Công
 MSV: 231432181
 Lớp: Kỹ thuật Điện tử - Tin học công nghiệp 2 K64
 Link Wokwi: [Truy cập link tại đây](https://wokwi.com/projects/480205288590103553)
 Báo cáo: 
## 1. Giới thiệu
Dự án xây dựng hệ thống giám sát môi trường vườn cây công nghiệp (cà phê, hồ tiêu) sử dụng ESP32 và cảm biến DHT22.

Hệ thống đo nhiệt độ, độ ẩm không khí, gửi dữ liệu lên MQTT Broker và hiển thị trên Web Dashboard. Ngoài ra, hệ thống có thể điều khiển thiết bị tự động hoặc thủ công.

## 2. Chức năng chính
- Đo nhiệt độ và độ ẩm bằng cảm biến DHT22.
- Hiển thị dữ liệu trên Serial Monitor và Web Dashboard.
- Gửi dữ liệu qua MQTT.
- Tự động điều khiển máy phun sương theo độ ẩm.
- Điều khiển máy bơm và quạt thông qua giao diện Web.
- Sử dụng FreeRTOS để chạy các tác vụ song song.

## 3. Công nghệ sử dụng
- **Phần cứng:** ESP32, DHT22, LED mô phỏng thiết bị.
- **Lập trình:** Arduino IDE, FreeRTOS.
- **Truyền dữ liệu:** WiFi, MQTT.
- **Giao diện:** HTML, CSS, JavaScript.

## 4. Nguyên lý hoạt động
1. DHT22 đo nhiệt độ và độ ẩm không khí.
2. ESP32 đọc dữ liệu và gửi lên MQTT Broker.
3. Web Dashboard nhận và hiển thị dữ liệu theo thời gian thực.
4. ESP32 tự động điều khiển máy phun sương dựa trên độ ẩm.
5. Người dùng có thể bật/tắt máy bơm và quạt trên giao diện Web.

## 5. Ngưỡng điều khiển phun sương

| Độ ẩm không khí | Công suất | Thời gian |
|---|---:|---:|
| 35–40% | 25% | 10 giây |
| 30–35% | 50% | 20 giây |
| Dưới 30% | 100% | 30 giây |
| Từ 40% trở lên | Tắt | - |

## 6. Cấu trúc dự án
```text
VuonCayCongNghiep/
├── README.md
├── ESP32/
│   └── ESP32.ino
├── Web/
│   └── index.html
└── Wokwi/
    └── diagram.json
```

## 7. Mục tiêu
Xây dựng mô hình giám sát và điều khiển môi trường vườn cây từ xa, góp phần ứng dụng IoT vào chăm sóc cây trồng và tự động hóa nông nghiệp.



#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ================= KHAI BAO CHAN =================

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define BOM_PIN 18
#define QUAT_PIN 19
#define PHUN_SUONG_PIN 5

// ================= WIFI =================

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================= MQTT =================

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* topic_data = "vuoncay/hoang/data";
const char* topic_bom = "vuoncay/hoang/bom";
const char* topic_quat = "vuoncay/hoang/quat";

// ================= DOI TUONG =================

DHT dht(DHT_PIN, DHT_TYPE);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ================= CAU TRUC DU LIEU =================

struct SensorData {
  float nhietdo;
  float doam;
};

// ================= QUEUE =================

QueueHandle_t dataQueue;
QueueHandle_t dataAlert;

// ================= HAM MQTT CALLBACK =================

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  message.trim();

  // Dieu khien may bom
  if (String(topic) == topic_bom) {

    if (message == "ON") {
      digitalWrite(BOM_PIN, HIGH);
      Serial.println("MAY BOM: BAT");
    }
    else if (message == "OFF") {
      digitalWrite(BOM_PIN, LOW);
      Serial.println("MAY BOM: TAT");
    }
  }

  // Dieu khien quat
  if (String(topic) == topic_quat) {

    if (message == "ON") {
      digitalWrite(QUAT_PIN, HIGH);
      Serial.println("QUAT: BAT");
    }
    else if (message == "OFF") {
      digitalWrite(QUAT_PIN, LOW);
      Serial.println("QUAT: TAT");
    }
  }
}

// =================================================
// TASK 1: TRUYEN DATA - CORE 0
// =================================================

void TaskTruyenData(void *pvParameters) {

  WiFi.begin(ssid, password);

  Serial.println("Dang ket noi WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  Serial.println("Da ket noi WiFi!");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(256);

  SensorData data;

  for (;;) {

    // Kiem tra WiFi
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    // Ket noi MQTT
    if (!mqttClient.connected()) {

      String clientId = "ESP32-VuonCay-" +
                        String((uint32_t)ESP.getEfuseMac(), HEX);

      if (mqttClient.connect(clientId.c_str())) {

        Serial.println("Da ket noi MQTT Broker!");

        mqttClient.subscribe(topic_bom);
        mqttClient.subscribe(topic_quat);
      }
      else {
        vTaskDelay(pdMS_TO_TICKS(2000));
        continue;
      }
    }

    // Xu ly cac ban tin MQTT den
    mqttClient.loop();

    // Doc du lieu tu dataQueue
    if (xQueueReceive(dataQueue, &data,
                      pdMS_TO_TICKS(100)) == pdTRUE) {

      if (mqttClient.connected()) {

        char jsonData[100];

        snprintf(jsonData, sizeof(jsonData),
                 "{\"nhietdo\":%.2f,\"doam\":%.2f}",
                 data.nhietdo, data.doam);

        mqttClient.publish(topic_data, jsonData);

        Serial.print("Da gui MQTT: ");
        Serial.println(jsonData);
      }
    }
  }
}

// =================================================
// TASK 2: DOC DATA - CORE 1
// =================================================

void TaskDocData(void *pvParameters) {

  SensorData data;

  for (;;) {

    // Doc cam bien DHT22
    float nhietdo = dht.readTemperature();
    float doam = dht.readHumidity();

    // Kiem tra du lieu cam bien
    if (isnan(nhietdo) || isnan(doam)) {

      Serial.println("Loi doc cam bien DHT22!");

    }
    else {

      data.nhietdo = nhietdo;
      data.doam = doam;

      // Hien thi Serial
      Serial.println("======================");
      Serial.print("Nhiet do: ");
      Serial.print(nhietdo);
      Serial.println(" *C");

      Serial.print("Do am: ");
      Serial.print(doam);
      Serial.println(" %");

      // Gui du lieu day du vao dataQueue
      if (xQueueSend(dataQueue, &data, 0) != pdTRUE) {

        // Neu Queue day, bo du lieu cu nhat
        xQueueReceive(dataQueue, &data, 0);

        // Gui lai du lieu moi
        data.nhietdo = nhietdo;
        data.doam = doam;
        xQueueSend(dataQueue, &data, 0);
      }

      // Gui do am vao dataAlert
      // Queue chi giu gia tri moi nhat
      xQueueOverwrite(dataAlert, &doam);
    }

    vTaskDelay(pdMS_TO_TICKS(2500));
  }
}

// =================================================
// TASK 3: CANH BAO - CORE 1
// =================================================

void TaskCanhBao(void *pvParameters) {

  float doam;

  for (;;) {

    // Cho nhan du lieu do am
    if (xQueueReceive(dataAlert, &doam,
                      portMAX_DELAY) == pdTRUE) {

      int congSuat = 0;
      int thoiGian = 0;

      // Xac dinh muc do phun suong
      if (doam < 30) {

        congSuat = 255;
        thoiGian = 30;

      }
      else if (doam < 35) {

        congSuat = 128;
        thoiGian = 20;

      }
      else if (doam < 40) {

        congSuat = 64;
        thoiGian = 10;

      }
      else {

        congSuat = 0;
        thoiGian = 0;
      }

      // Dieu khien may phun suong bang PWM
      if (congSuat > 0) {

        analogWrite(PHUN_SUONG_PIN, congSuat);

        Serial.println("----------------------");
        Serial.print("CANH BAO DO AM: ");
        Serial.print(doam);
        Serial.println(" %");

        Serial.print("Cong suat phun: ");
        Serial.print(congSuat * 100 / 255);
        Serial.println(" %");

        Serial.print("Thoi gian phun: ");
        Serial.print(thoiGian);
        Serial.println(" giay");

        // Phun trong thoi gian quy dinh
        vTaskDelay(pdMS_TO_TICKS(thoiGian * 1000));

        // Het thoi gian thi tat
        analogWrite(PHUN_SUONG_PIN, 0);

        Serial.println("Da tat may phun suong!");
      }
      else {

        analogWrite(PHUN_SUONG_PIN, 0);

        Serial.println("Do am dat nguong, khong can phun!");
      }
    }
  }
}

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(BOM_PIN, OUTPUT);
  pinMode(QUAT_PIN, OUTPUT);
  pinMode(PHUN_SUONG_PIN, OUTPUT);

  digitalWrite(BOM_PIN, LOW);
  digitalWrite(QUAT_PIN, LOW);
  analogWrite(PHUN_SUONG_PIN, 0);

  // Tao Queue
  dataQueue = xQueueCreate(10, sizeof(SensorData));

  dataAlert = xQueueCreate(1, sizeof(float));

  if (dataQueue == NULL || dataAlert == NULL) {
    Serial.println("Loi tao Queue!");
    while (1) {
      delay(1000);
    }
  }

  // Tao Task truyen data - Core 0
  xTaskCreatePinnedToCore(
    TaskTruyenData,
    "Task Truyen Data",
    8192,
    NULL,
    2,
    NULL,
    0
  );

  // Tao Task doc data - Core 1
  xTaskCreatePinnedToCore(
    TaskDocData,
    "Task Doc Data",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  // Tao Task canh bao - Core 1
  xTaskCreatePinnedToCore(
    TaskCanhBao,
    "Task Canh Bao",
    4096,
    NULL,
    2,
    NULL,
    1
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

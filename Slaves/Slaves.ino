#include <DHT.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_now.h>

#define BLYNK_PRINT Serial

unsigned long previousMillis = 0;
// Thời gian delay gửi dữ liệu lên sheet
const long timedelay = 10000;
// Thay bằng token lấy từ Blynk
char auth[] = "_UdXyXEsqedKwODLofMX1etn24ZCEJFk";
// Tên và mật khẩu wifi
char ssid[] = "Wifi";
char password[] = "27060204";
// Chuỗi script lấy từ google sheet
String GOOGLE_SCRIPT_ID = "AKfycbxJvvGNYh-R-GxEocnENVuEShpRrj_rpqjfN_DdW6irSzTa6eE7wFpjHD_m7qmFqQE6AA";
// Địa chỉ MAC của mạch điều khiển
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Khai báo cảm biến ánh sáng
// #define LUXMETER_1_PIN 35
// #define LUXMETER_2_PIN 34
// #define LUXMETER_3_PIN 39
float lux_1 = 0, lux_2 = 0, lux_3 = 0;

// Khai báo cảm biến DHT22
#define DHTTYPE DHT22
#define SENSOR_1_PIN 32
/*#define SENSOR_2_PIN 33
#define SENSOR_3_PIN 25
#define SENSOR_4_PIN 26
#define SENSOR_5_PIN 27
#define SENSOR_6_PIN 14
#define SENSOR_7_PIN 12
#define SENSOR_8_PIN 13
#define SENSOR_9_PIN 15
#define SENSOR_10_PIN 2
#define SENSOR_11_PIN 0
#define SENSOR_12_PIN 4*/
DHT dht_1(SENSOR_1_PIN, DHTTYPE);
/*DHT dht_2(SENSOR_2_PIN, DHTTYPE);
DHT dht_3(SENSOR_3_PIN, DHTTYPE);
DHT dht_4(SENSOR_4_PIN, DHTTYPE);
DHT dht_5(SENSOR_5_PIN, DHTTYPE);
DHT dht_6(SENSOR_6_PIN, DHTTYPE);
DHT dht_7(SENSOR_7_PIN, DHTTYPE);
DHT dht_8(SENSOR_8_PIN, DHTTYPE);
DHT dht_9(SENSOR_9_PIN, DHTTYPE);
DHT dht_10(SENSOR_10_PIN, DHTTYPE);
DHT dht_11(SENSOR_11_PIN, DHTTYPE);
DHT dht_12(SENSOR_12_PIN, DHTTYPE);*/
float t_1 = 0, t_2 = 0, t_3 = 0, t_4 = 0,t_5 = 0, t_6 = 0, t_7 = 0, t_8 = 0,t_9 = 0, t_10 = 0, t_11 = 0, t_12 = 0;
float h_1 = 0, h_2 = 0, h_3 = 0, h_4 = 0,h_5 = 0, h_6 = 0, h_7 = 0, h_8 = 0,h_9 = 0, h_10 = 0, h_11 = 0, h_12 = 0;
float t_tb, h_tb;

//Khai báo cân
// Cân 1
// #define calibration_factor_1 525680
// #define LOADCELL_1_DOUT_PIN  5
// #define LOADCELL_1_SCK_PIN  18
HX711 scale_1;
float weight_lbs_1;
float weight_1 = 0;
//Cân 2
// #define calibration_factor_2 525680
// #define LOADCELL_2_DOUT_PIN  19
// #define LOADCELL_2_SCK_PIN  21
HX711 scale_2;
float weight_lbs_2;
float weight_2 = 0;
//Cân 3
// #define calibration_factor_3 525680
// #define LOADCELL_3_DOUT_PIN  3
// #define LOADCELL_3_SCK_PIN  1
HX711 scale_3;
float weight_lbs_3;
float weight_3 = 0;

BlynkTimer timer;
WiFiClientSecure client;

// Hàm gửi dữ liệu lên Blynk
void sendSensors() {
  // DHT22
  Blynk.virtualWrite(V0, t_tb);
  Blynk.virtualWrite(V1, h_tb);
  Blynk.virtualWrite(V2, t_1);
  Blynk.virtualWrite(V3, h_1);
  Blynk.virtualWrite(V4, t_2);
  Blynk.virtualWrite(V5, h_2);
  Blynk.virtualWrite(V6, t_3);
  Blynk.virtualWrite(V7, h_3);
  Blynk.virtualWrite(V8, t_4);
  Blynk.virtualWrite(V9, h_4);
  Blynk.virtualWrite(V10, t_5);
  Blynk.virtualWrite(V11, h_5);
  Blynk.virtualWrite(V12, t_6);
  Blynk.virtualWrite(V13, h_6);
  Blynk.virtualWrite(V14, t_7);
  Blynk.virtualWrite(V15, h_7);
  Blynk.virtualWrite(V16, t_8);
  Blynk.virtualWrite(V17, h_8);
  Blynk.virtualWrite(V18, t_9);
  Blynk.virtualWrite(V19, h_9);
  Blynk.virtualWrite(V20, t_10);
  Blynk.virtualWrite(V21, h_10);
  Blynk.virtualWrite(V22, t_11);
  Blynk.virtualWrite(V23, h_11);
  Blynk.virtualWrite(V24, t_12);
  Blynk.virtualWrite(V25, h_12);
  // Load cell
  Blynk.virtualWrite(V26, weight_1);
  Blynk.virtualWrite(V27, weight_2);
  Blynk.virtualWrite(V28, weight_3);
  //Luxmeter
  Blynk.virtualWrite(V29, lux_1);
  Blynk.virtualWrite(V30, lux_2);
  Blynk.virtualWrite(V31, lux_3);
}

//Nối chuỗi
String T_A() {
  String T_A = String(t_1) + "," + String(t_2) + "," + String(t_3) + "," + String(t_4) 
      + "," + String(t_5) + "," + String(t_6) + "," + String(t_7) + "," + String(t_8) 
      + "," + String(t_9) + "," + String(t_10) + "," + String(t_11) + "," + String(t_12);
  return T_A;
}

String H_A() {
  String H_A = String(h_1) + "," + String(h_2) + "," + String(h_3) + "," + String(h_4) 
      + "," + String(h_5) + "," + String(h_6) + "," + String(h_7) + "," + String(h_8) 
      + "," + String(h_9) + "," + String(h_10) + "," + String(h_11) + "," + String(h_12);
  return H_A;
}

String W_A() {
  String W_A = String(weight_1) + "," + String(weight_2) + "," + String(weight_3);
  return W_A;
}

String LUX_A() {
  String LUX_A = String(lux_1) + "," + String(lux_2) + "," + String(lux_3);
  return LUX_A;
}

// Hàm gửi dữ liệu lên google sheet
void sendData(String params) {
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  Serial.println(url);
  http.begin(url); 
  int httpCode = http.GET();
  http.end();
}

//Hàm đọc cảm biến DHT22
void readDHT22() {
    int count = 0;
    t_1 = dht_1.readTemperature(); if (isnan(t_1)) {t_1 = 0;}
    h_1 = dht_1.readHumidity(); if (isnan(h_1)) {h_1 = 0;} else {count = count + 1;}
    /*t_2 = dht_2.readTemperature(); if (isnan(t_2)) {t_2 = 0;}
    h_2 = dht_2.readHumidity(); if (isnan(h_2)) {h_2 = 0;} else {count = count + 1;}
    t_3 = dht_3.readTemperature(); if (isnan(t_3)) {t_3 = 0;}
    h_3 = dht_3.readHumidity(); if (isnan(h_3)) {h_3 = 0;} else {count = count + 1;}
    t_4 = dht_4.readTemperature(); if (isnan(t_4)) {t_4 = 0;}
    h_4 = dht_4.readHumidity(); if (isnan(h_4)) {h_4 = 0;} else {count = count + 1;}
    t_5 = dht_5.readTemperature(); if (isnan(t_5)) {t_5 = 0;}
    h_5 = dht_5.readHumidity(); if (isnan(h_5)) {h_5 = 0;} else {count = count + 1;}
    t_6 = dht_6.readTemperature(); if (isnan(t_6)) {t_6 = 0;}
    h_6 = dht_6.readHumidity(); if (isnan(h_6)) {h_6 = 0;} else {count = count + 1;}
    t_7 = dht_7.readTemperature(); if (isnan(t_7)) {t_7 = 0;}
    h_7 = dht_7.readHumidity(); if (isnan(h_7)) {h_7 = 0;} else {count = count + 1;}
    t_8 = dht_8.readTemperature(); if (isnan(t_8)) {t_8 = 0;}
    h_8 = dht_8.readHumidity(); if (isnan(h_8)) {h_8 = 0;} else {count = count + 1;}
    t_9 = dht_9.readTemperature(); if (isnan(t_9)) {t_9 = 0;}
    h_9 = dht_9.readHumidity(); if (isnan(h_9)) {h_9 = 0;} else {count = count + 1;}
    t_10 = dht_10.readTemperature(); if (isnan(t_10)) {t_10 = 0;}
    h_10 = dht_10.readHumidity(); if (isnan(h_10)) {h_10 = 0;} else {count = count + 1;}
    t_11 = dht_11.readTemperature(); if (isnan(t_11)) {t_11 = 0;}
    h_11 = dht_11.readHumidity(); if (isnan(h_11)) {h_11 = 0;} else {count = count + 1;}
    t_12 = dht_12.readTemperature(); if (isnan(t_12)) {t_12 = 0;}
    h_12 = dht_12.readHumidity(); if (isnan(h_12)) {h_12 = 0;} else {count = count + 1;}*/
    t_tb = (t_1 + t_2 + t_3 + t_4 + t_5 + t_6 + t_7 + t_8 + t_9 + t_10 + t_11 + t_12)/count;
    h_tb = (h_1 + h_2 + h_3 + h_4 + h_5 + h_6 + h_7 + h_8 + h_9 + h_10 + h_11 + h_12)/count;
}

// Hàm đọc cảm biến cân nặng
void readLoadcell() {
  weight_lbs_1 = scale_1.get_units();
  weight_1 = weight_lbs_1*453.592;
  weight_lbs_2= scale_2.get_units();
  weight_2 = weight_lbs_2*453.592;
  weight_lbs_3 = scale_3.get_units();
  weight_3 = weight_lbs_3*453.592;
}

// gửi nhiệt độ, độ ẩm trung bình đến mạch điều khiển
typedef struct struct_message {
    int id; // must be unique for each sender board
    float t_tb;
    float h_tb;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  // Set serial monitor
  Serial.begin(115200);
  delay(10);

  // Set wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int wifi_ctr = 0;
  Serial.println("Đang khởi động");
  Serial.print("Đang kết nối");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Đã kết nối. Bắt đầu hoạt động.");

  // Set DHT
  dht_1.begin();
  /*dht_2.begin();
  dht_3.begin();
  dht_4.begin();
  dht_5.begin();
  dht_6.begin();
  dht_7.begin();
  dht_8.begin();
  dht_9.begin();
  dht_10.begin();
  dht_11.begin();
  dht_12.begin();

  // Set cân
  scale_1.begin(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN);
  scale_1.set_scale(calibration_factor_1);
  scale_1.tare();
  scale_2.begin(LOADCELL_2_DOUT_PIN, LOADCELL_2_SCK_PIN);
  scale_2.set_scale(calibration_factor_2);
  scale_2.tare();
  scale_3.begin(LOADCELL_3_DOUT_PIN, LOADCELL_3_SCK_PIN);
  scale_3.set_scale(calibration_factor_3);
  scale_3.tare();
  */

  // Set Blynk
  Blynk.begin(auth, ssid, password,"sv.bangthong.com", 8080);
  timer.setInterval(timedelay, sendSensors);

  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Thêm peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Lỗi thêm peer");
    return;
  }
  
}

void loop() {
  Blynk.run();
  timer.run();
  readDHT22();
  readLoadcell();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timedelay) {
    previousMillis = currentMillis;       
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.reconnect();
    }
    
    Serial.println(t_1);
    Serial.println(h_1);
    Serial.println(t_2);
    Serial.println(h_2);
    Serial.println(t_3);
    Serial.println(h_3);
    Serial.println(t_4);
    Serial.println(h_4);
    Serial.println(t_5);
    Serial.println(h_5);
    Serial.println(t_6);
    Serial.println(h_6);
    Serial.println(t_7);
    Serial.println(h_7);
    Serial.println(t_8);
    Serial.println(h_8);
    Serial.println(t_9);
    Serial.println(h_9);
    Serial.println(t_10);
    Serial.println(h_10);
    Serial.println(t_11);
    Serial.println(h_11);
    Serial.println(t_12);
    Serial.println(h_12);
    Serial.println(weight_1);
    // Gửi dữ liệu lên mạch điều khiển
    myData.id = 1;
    myData.t_tb = t_tb;
    myData.h_tb = h_tb;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
    Serial.println("Gửi dữ liệu thành công");
    }
    else {
      Serial.println("Lỗi gửi dữ liệu");
    }
    
    // Gửi dữ liệu lên Sheet
    sendData("T_A1,T_A2,T_A3,T_A4,T_A5,T_A6,T_A7,T_A8,T_A9,T_A10,T_A11,T_A12=" + T_A()
           + "&H_A1,H_A2,H_A3,H_A4,H_A5,H_A6,H_A7,H_A8,H_A9,H_A10,H_A11,H_A12=" + H_A()
           + "&W_A1,W_A2,W_A3=" + W_A()
           + "&L_A1,L_A2,L_A3=" + LUX_A());
  }
}

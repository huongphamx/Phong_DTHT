#include <DHT.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "time.h"
#include <TimeLib.h>
#include <WidgetRTC.h>

#define BLYNK_PRINT Serial

// Lấy thời gian từ NTP server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*60*60;
const int   daylightOffset_sec = 0;
char timeHour[3];
char timeMin[3];

unsigned long previousMillis = 0;
// Thời gian delay gửi dữ liệu lên sheet
const long timedelay = 10000;
// Bỏ comment dòng Blynk token tương ứng với từng mạch giá nuôi
char auth[] = "dakSG7QNxiYOVnT3kPJTShAMBeFjAuRw"; // Giá A
// char auth[] = "XmD4p3tLrF0tGTSjC15Tjg_wjze2RsS7" // Giá B
// char auth[] = "gegz_7L_KsPGNy5van4fMi2kQ7dCqgb2" // Giá C
// char auth[] = "1bz-1ss7LPDZH8o8ADINjv-yRBFHRPV-" // Giá D

BlynkTimer timer;
WidgetRTC rtc;
WiFiClientSecure client;
BLYNK_CONNECTED() {
  rtc.begin();
}

// Tên và mật khẩu wifi
constexpr char WIFI_SSID[] = "TP-LINK_D282AE";
const char* ssid = "TP-LINK_D282AE";
const char* password = "57971834";
/*
constexpr char WIFI_SSID[] = "";
const char* ssid = "";
const char* password = "hoilamgi";
 */

// Bỏ comment dòng script id tương ứng với từng mạch giá nuôi
String GOOGLE_SCRIPT_ID = "AKfycbyNTqIPgAAZk0_j0tFaM61Lj5YA4ZXVStal7OzQub_NC1zeC6e4Ed4RiWDMqxChLoIWyQ"; // Google sheet giá A
// String GOOGLE_SCRIPT_ID = "AKfycbwVRP5zSPQV5_bbOmCaKHSPMfo2yYDMtMz1Yy7XH6Sa-u1DzK_-_0n2o_g7tD2uwACbNQ"; // Google sheet giá B
// String GOOGLE_SCRIPT_ID = "AKfycbwwLJclAnITIa7j9_qyCAExYpDgjjNnWP94iPa0qNXanaH0FOqiMK4T08GyxmUO1FhC"; // Google sheet giá C
// String GOOGLE_SCRIPT_ID = "AKfycbyA6r_hmp84_okj3PAuK-rIpQy94Sa_fCPAV88NsqemtnVp3nY4HbjTBB4KdaWYK51Iig"; // Google sheet giá D

// Địa chỉ MAC của mạch điều khiển
uint8_t broadcastAddress[] = {0xEC, 0x94, 0xCB, 0x4B, 0x48, 0x64};

// Khai báo chân relay bật/tắt LEDs
int LED = 21;
// int LED = 22;
const int AUTO = 1;
const int MANUAL = 2;

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
// #define LOADCELL_1_DOUT_PIN  16
// #define LOADCELL_1_SCK_PIN  17
HX711 scale_1;
float weight_lbs_1;
float weight_1 = 0;
//Cân 2
// #define calibration_factor_2 525680
// #define LOADCELL_2_DOUT_PIN  5
// #define LOADCELL_2_SCK_PIN  18
HX711 scale_2;
float weight_lbs_2;
float weight_2 = 0;
//Cân 3
// #define calibration_factor_3 525680
// #define LOADCELL_3_DOUT_PIN  19
// #define LOADCELL_3_SCK_PIN  23
HX711 scale_3;
float weight_lbs_3;
float weight_3 = 0;


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
String Temps() {
  String Temps = String(t_1) + "," + String(t_2) + "," + String(t_3) + "," + String(t_4) 
      + "," + String(t_5) + "," + String(t_6) + "," + String(t_7) + "," + String(t_8) 
      + "," + String(t_9) + "," + String(t_10) + "," + String(t_11) + "," + String(t_12);
  return Temps;
}

String Humis() {
  String H_A = String(h_1) + "," + String(h_2) + "," + String(h_3) + "," + String(h_4) 
      + "," + String(h_5) + "," + String(h_6) + "," + String(h_7) + "," + String(h_8) 
      + "," + String(h_9) + "," + String(h_10) + "," + String(h_11) + "," + String(h_12);
  return Humis;
}

String Weights() {
  String Weights = String(weight_1) + "," + String(weight_2) + "," + String(weight_3);
  return Weights;
}

String LUXs() {
  String LUXs = String(lux_1) + "," + String(lux_2) + "," + String(lux_3);
  return LUXs;
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
    float t;
    float h;
} struct_message;

struct_message myData;

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Điều khiển LED qua Blynk
// 
int LED_control_mode = AUTO;
BLYNK_WRITE(V39) {
  LED_control_mode = param.asInt();
}

void controlLED() {
  if (LED_control_mode == AUTO) {auto_control_LED();}
  if (LED_control_mode == MANUAL) {manual_control_LED();}
}

// Automatic control LEDs
// Delare time variable
int start_hour, start_min, stop_hour, stop_min;
BLYNK_WRITE(V40) {
  TimeInputParam t(param);
  start_hour = t.getStartHour();
  start_min = t.getStartMinute();
  stop_hour = t.getStopHour();
  stop_min = t.getStopMinute();
}

void auto_control_LED() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  if (((int)p_tm->tm_hour == start_hour) && ((int)p_tm->tm_min == start_min)) {
      digitalWrite(LED, 1);
    }
  if (((int)p_tm->tm_hour == stop_hour) && ((int)p_tm->tm_min == stop_min)) {
      digitalWrite(LED, 0);
    }
}

// Manual control LEDs
// Initial LEDs' status value
int LED_button = 0;
BLYNK_WRITE(V41) {
  LED_button = param.asInt();
}

void manual_control_LED() {
  if (LED_button == 1) {digitalWrite(LED, 1);}
  if (LED_button == 0) {digitalWrite(LED, 0);}
}


void setup() {
  // Set serial monitor
  Serial.begin(115200);
  delay(10);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  
  // Set wifi
  WiFi.mode(WIFI_AP_STA);
  
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

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial);

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
  Blynk.begin(auth, ssid, password,"blynk-server.com", 8080);
  timer.setInterval(timedelay, sendSensors);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);

  //Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);  
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
  //readLoadcell();
  controlLED(); 
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timedelay) {
    previousMillis = currentMillis;
    // Reconnect wifi       
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.reconnect();
    }
    
    Serial.println(t_1);
    Serial.println(h_1);
    /*Serial.println(t_2);
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
    Serial.println(weight_1);*/
    // Gửi dữ liệu lên mạch điều khiển
    myData.id = 1; // Giá A
    // myData.id = 2; // Giá B
    // myData.id = 3; // Giá C
    // myData.id = 4; // Giá D
    myData.t = t_tb;
    myData.h = h_tb;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
    Serial.println("Gửi dữ liệu thành công");
    }
    else {
      Serial.println("Lỗi gửi dữ liệu");
    }
    
    // Gửi dữ liệu lên Sheet
    sendData("T_1,T_2,T_3,T_4,T_5,T_6,T_7,T_8,T_9,T_10,T_11,T_12=" + Temps()
           + "&H_1,H_2,H_3,H_4,H_5,H_6,H_7,H_8,H_9,H_10,H_11,H_12=" + Humis()
           + "&W_1,W_2,W_3=" + Weights()
           + "&L_1,L_2,L_3=" + LUXs());
  }
}

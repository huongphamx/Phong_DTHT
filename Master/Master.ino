// Khai báo wifi
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <esp_now.h>

const char* ssid     = "Wifi";
const char* password = "27060204";
const int AUTO = 1;
const int MANUAL = 2;
const int ON = 1;
const int OFF = 0;

// Lấy thời gian từ NTP server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*60*60;
const int   daylightOffset_sec = 0;
char timeHour[3];
char timeMin[3];

// Khai báo Blynk
#define BLYNK_PRINT Serial
char auth[] = "9NXkkZnJO2-1aUxPfmxDHPtBnJgUJW1O";
unsigned long previousMillis = 0;
const long timedelay = 10000;
BlynkTimer timer;
WidgetRTC rtc;
WiFiClientSecure client;
BLYNK_CONNECTED() {
  rtc.begin();
}

//Khai báo chân relay
int humi_pump = 18;
int humi_us = 5;
int re_pump = 17;
int UVw = 17;
int UVws = 17;
int ven_fan = 4;
int UVa = 16;

//*********************ĐIỀU KHIỂN ẨM***********************//
/////////////////////////////////////////////////////////////
int t_tb = 0, h_tb = 0;
void sendDHT() {
  Blynk.virtualWrite(V0, t_tb);
  Blynk.virtualWrite(V1, h_tb);
}

// Nhận nhiệt độ, độ ẩm trung bình từ các mạch Slave
// Tạo structure để nhận data
typedef struct struct_message {
  int id;
  float t_tb;
  float h_tb;
} struct_message;

// Tạo struct myData
struct_message myData;

// Tạo struct để chứa dữ liệu từ mỗi board
struct_message boardA;
struct_message boardB;
struct_message boardC;
struct_message boardD;

// Tạo array chứa tất cả struct
struct_message boardsStruct[4] = {boardA, boardB, boardC, boardD};

// Gọi hàm được thực thi khi nhận data
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Nhận dữ liệu từ: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // update structure với dữ liệu mới nhận
  boardsStruct[myData.id-1].t_tb = myData.t_tb;
  boardsStruct[myData.id-1].h_tb = myData.h_tb;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].t_tb);
  Serial.printf("y value: %d \n", boardsStruct[myData.id-1].h_tb);
  Serial.println();
}

//Điều khiển bơm phun sương từ Blynk
//Đọc trạng thái điều khiển bơm phun sương
int CM_humi_pump = 1;
BLYNK_WRITE(V2) {
  CM_humi_pump = param.asInt();
}
//Điều khiển bơm phun sương
void controlhumi_pump() {
  if (CM_humi_pump == 1) {auto_humi_pump();}
  if (CM_humi_pump == 2) {manual_humi_pump();}
}
//Bật tắt bơm phun sương tự động
int h_max, h_min;
BLYNK_WRITE(V3) {
  h_max = param.asInt();
}
BLYNK_WRITE(V4) {
  h_min = param.asInt();
}
void auto_humi_pump() {
  if (h_tb > h_max) {
    digitalWrite(humi_pump, 0);
  } else if (h_tb < h_min) {
    digitalWrite(humi_pump, 1);    
  }
}
//Bật tắt bơm phun sương thủ công
int humi_pump_button = 0;
BLYNK_WRITE(V5) {
  humi_pump_button = param.asInt();
}
void manual_humi_pump() {
  if (humi_pump_button == 1) {digitalWrite(humi_pump, 1);}
  if (humi_pump_button == 0) {digitalWrite(humi_pump, 0);}
}

//Điều khiển phun sương siêu âm từ Blynk
//Đọc trạng thái điều khiển phun sương siêu âm
int CM_humi_us = 1;
BLYNK_WRITE(V6) {
  CM_humi_us = param.asInt();
}
//Điều khiển phun sương siêu âm
void controlhumi_us() {
  if (CM_humi_us == 1) {auto_humi_us();}
  if (CM_humi_us == 2) {manual_humi_us();}
}
//Bật tắt phun sương siêu âm tự động
int h_max_us, h_min_us;
BLYNK_WRITE(V7) {
  h_max_us = param.asInt();
}
BLYNK_WRITE(V8) {
  h_min_us = param.asInt();
}
void auto_humi_us() {
  if (h_tb > h_max_us) {
    digitalWrite(humi_us, 0);
  } else if (h_tb < h_min_us) {
    digitalWrite(humi_us, 1);    
  }
}
//Bật tắt phun sương siêu âm thủ công
int humi_us_button = 0;
BLYNK_WRITE(V9) {
  humi_us_button = param.asInt();
}
void manual_humi_us() {
  if (humi_us_button == 1) {digitalWrite(humi_us, 1);}
  if (humi_us_button == 0) {digitalWrite(humi_us, 0);}
}
/////////////////////////////////////////////////////////////////////////




//*********************ĐIỀU KHIỂN NƯỚC TUẦN HOÀN***********************//
/////////////////////////////////////////////////////////////////////////

//Đọc mức nước trong thùng, gửi lên Blynk




//Điều khiển bơm nước tuần hoàn từ Blynk
//Đọc trạng thái điều khiển bơm 
int CM_re_pump = 1;
BLYNK_WRITE(V12) {
  CM_re_pump = param.asInt();
}
//Điều khiển bơm nước
void controlre_pump() {
  if (CM_re_pump == 1) {auto_re_pump();}
  if (CM_re_pump == 2) {manual_re_pump();}
}
//Bật tắt bơm nước tự động
int re_pump_cycle, re_pump_acttime, re_pump_blynk_h, re_pump_blynk_min, re_pump_RESET;
BLYNK_WRITE(V13) {
  re_pump_cycle = param.asInt();
}
BLYNK_WRITE(V14) {
  re_pump_acttime = param.asInt();
}
BLYNK_WRITE(V15) {
  TimeInputParam t_re_pump(param);
  re_pump_blynk_h = t_re_pump.getStartHour();
  re_pump_blynk_min = t_re_pump.getStartMinute();
}
BLYNK_WRITE(V16) {
  re_pump_RESET = param.asInt();
}

  int re_pump_on_h;
  int re_pump_on_min;
  int i_rp = 0;
  unsigned long re_pump_p_millis = 0;
void auto_re_pump() {
  time_t re_pump_now = time(nullptr);
  struct tm* re_pump_p_tm = localtime(&re_pump_now);
  int re_pump_off_h, re_pump_off_min;  
  delay(4000);
  if (((re_pump_blynk_h == 0)&&(re_pump_blynk_min == 0))||(re_pump_cycle == 0)) {i_rp = 0;}
  if (re_pump_RESET == 1) {i_rp = 0;}
  Serial.println(i_rp);
  re_pump_on_h = (re_pump_blynk_h + re_pump_cycle*i_rp)%24;
  re_pump_on_min = re_pump_blynk_min;
  Serial.println(String("re_pump start: ") + re_pump_on_h +":" +re_pump_on_min);
  if (((int)re_pump_p_tm->tm_hour == re_pump_on_h) && ((int)re_pump_p_tm->tm_min == re_pump_on_min) && (re_pump_cycle != 0)) {
    digitalWrite(re_pump, 1);
    Serial.println("re_pump is ON");
  }
  re_pump_off_h = (re_pump_on_h + (re_pump_acttime)/60)%24;
  re_pump_off_min = (re_pump_on_min + re_pump_acttime)%60;
  Serial.println(String("re_pump stop: ") + re_pump_off_h +":" +re_pump_off_min);
  if (((int)re_pump_p_tm->tm_hour == re_pump_off_h) && ((int)re_pump_p_tm->tm_min == re_pump_off_min) && (re_pump_cycle != 0)) {
    digitalWrite(re_pump, 0);
    Serial.println("re_pump is OFF");
  }
  unsigned long re_pump_c_millis = millis();
  if ((re_pump_c_millis - re_pump_p_millis) >= re_pump_cycle*60*60*1000) {
    re_pump_p_millis = re_pump_c_millis;
    i_rp++; 
  }
  Serial.println(digitalRead(re_pump));
}
//Bật tắt UV nước thủ công
int re_pump_button = 0;
BLYNK_WRITE(V17) {
  re_pump_button = param.asInt();
}
void manual_re_pump() {
  if (re_pump_button == 1) {digitalWrite(re_pump, 1);}
  if (re_pump_button == 0) {digitalWrite(re_pump, 0);}
}
//----------------------------------------------------------------------------------//
//Điều khiển UV nước to từ Blynk
//Đọc trạng thái điều khiển UV nước 
int CM_UVw = 1;
BLYNK_WRITE(V18) {
  CM_UVw = param.asInt();
}
//Điều khiển UV nước
void controlUVw() {
  if (CM_UVw == 1) {auto_UVw();}
  if (CM_UVw == 2) {manual_UVw();}
}
//Bật tắt UV nước tự động
int UVw_cycle, UVw_acttime, UVw_blynk_h, UVw_blynk_min, UVw_RESET;
BLYNK_WRITE(V19) {
  UVw_cycle = param.asInt();
}
BLYNK_WRITE(V20) {
  UVw_acttime = param.asInt();
}
BLYNK_WRITE(V21) {
  TimeInputParam t_UVw(param);
  UVw_blynk_h = t_UVw.getStartHour();
  UVw_blynk_min = t_UVw.getStartMinute();
}
BLYNK_WRITE(V22) {
  UVw_RESET = param.asInt();
}

  int UVw_on_h;
  int UVw_on_min;
  int i = 0;
  unsigned long UVw_p_millis = 0;
void auto_UVw() {
  time_t UVw_now = time(nullptr);
  struct tm* UVw_p_tm = localtime(&UVw_now);
  int UVw_off_h, UVw_off_min;  
  delay(4000);
  if (((UVw_blynk_h == 0)&&(UVw_blynk_min == 0))||(UVw_cycle == 0)) {i = 0;}
  if (UVw_RESET == 1) {i = 0;}
  Serial.println(i);
  UVw_on_h = (UVw_blynk_h + UVw_cycle*i)%24;
  UVw_on_min = UVw_blynk_min;
  Serial.println(String("UVw start: ") + UVw_on_h +":" +UVw_on_min);
  if (((int)UVw_p_tm->tm_hour == UVw_on_h) && ((int)UVw_p_tm->tm_min == UVw_on_min) && (UVw_cycle != 0)) {
    digitalWrite(UVw, 1);
    Serial.println("UVw is ON");
  }
  UVw_off_h = (UVw_on_h + (UVw_acttime)/60)%24;
  UVw_off_min = (UVw_on_min + UVw_acttime)%60;
  Serial.println(String("UVw stop: ") + UVw_off_h +":" +UVw_off_min);
  if (((int)UVw_p_tm->tm_hour == UVw_off_h) && ((int)UVw_p_tm->tm_min == UVw_off_min) && (UVw_cycle != 0)) {
    digitalWrite(UVw, 0);
    Serial.println("UVw is OFF");
  }
  unsigned long UVw_c_millis = millis();
  if ((UVw_c_millis - UVw_p_millis) >= UVw_cycle*60*60*1000) {
    UVw_p_millis = UVw_c_millis;
    i++; 
  }
  Serial.println(digitalRead(UVw));
}
//Bật tắt UV nước thủ công
int UVw_button = 0;
BLYNK_WRITE(V23) {
  UVw_button = param.asInt();
}
void manual_UVw() {
  if (UVw_button == 1) {digitalWrite(UVw, 1);}
  if (UVw_button == 0) {digitalWrite(UVw, 0);}
}
//-------------------------------------------------------------------------------------//
//Điều khiển UV nước nhỏ từ Blynk
// Get contrạng thái điều khiển UV nước nhỏ
int CM_UVws = 1;
BLYNK_WRITE(V24) {
  CM_UVws= param.asInt();
}
//Điều khiển UV nước nhỏ
void controlUVws() {
  if (CM_UVws == 1) {auto_UVws();}
  if (CM_UVws == 2) {manual_UVws();}
}
//Bật tắt UV nước tự động
int UVws_cycle, UVws_acttime, UVws_blynk_h, UVws_blynk_min, UVws_RESET;
BLYNK_WRITE(V25) {
  UVws_cycle = param.asInt();
}
BLYNK_WRITE(V26) {
  UVws_acttime = param.asInt();
}
BLYNK_WRITE(V27) {
  TimeInputParam t_UVws(param);
  UVws_blynk_h = t_UVws.getStartHour();
  UVws_blynk_min = t_UVws.getStartMinute();
}
BLYNK_WRITE(V28) {
  UVws_RESET = param.asInt();
}

  int UVws_on_h;
  int UVws_on_min;
  int i_s = 0;
  unsigned long UVws_p_millis = 0;
void auto_UVws() {
  time_t UVws_now = time(nullptr);
  struct tm* UVws_p_tm = localtime(&UVws_now);
  int UVws_off_h, UVws_off_min;  
  delay(4000);
  if (((UVws_blynk_h == 0)&&(UVws_blynk_min == 0))||(UVws_cycle == 0)) {i_s = 0;}
  if (UVws_RESET == 1) {i_s = 0;}
  Serial.println(i_s);
  UVws_on_h = (UVws_blynk_h + UVws_cycle*i_s)%24;
  UVws_on_min = UVws_blynk_min;
  Serial.println(String("UVws start: ") + UVws_on_h +":" +UVws_on_min);
  if (((int)UVws_p_tm->tm_hour == UVws_on_h) && ((int)UVws_p_tm->tm_min == UVws_on_min) && (UVws_cycle != 0)) {
    digitalWrite(UVws, 1);
    Serial.println("UVws is ON");
  }
  UVws_off_h = (UVws_on_h + (UVws_acttime)/60)%24;
  UVws_off_min = (UVws_on_min + UVws_acttime)%60;
  Serial.println(String("UVws stop: ") + UVws_off_h +":" +UVws_off_min);
  if (((int)UVws_p_tm->tm_hour == UVws_off_h) && ((int)UVws_p_tm->tm_min == UVws_off_min) && (UVws_cycle != 0)) {
    digitalWrite(UVws, 0);
    Serial.println("UVws is OFF");
  }
  unsigned long UVws_c_millis = millis();
  if ((UVws_c_millis - UVws_p_millis) >= UVws_cycle*60*60*1000) {
    UVws_p_millis = UVws_c_millis;
    i_s++; 
  }
  Serial.println(digitalRead(UVws));
}
// Manual mode
int UVws_button = 0;
BLYNK_WRITE(V29) {
  UVws_button = param.asInt();
}
void manual_UVws() {
  if (UVws_button == 1) {digitalWrite(UVws, 1);}
  if (UVws_button == 0) {digitalWrite(UVws, 0);}
}
/////////////////////////////////////////////////////////////////////////


//*********************Control ventilator fan***********************//
/////////////////////////////////////////////////////////////////////////

// Get control mode from Blynk, initial value = AUTO
int control_mode_ven_fan = AUTO;
BLYNK_WRITE(V30) {
  control_mode_ven_fan = param.asInt();
}

void control_ven_fan() {
  if (control_mode_ven_fan == AUTO) {auto_ven_fan();}
  if (control_mode_ven_fan == MANUAL) {manual_ven_fan();}
}

// Automatic mode
int start_hour_ven_fan, start_min_ven_fan, stop_hour_ven_fan, stop_min_ven_fan;
BLYNK_WRITE(V31) { // Get set time from Blynk
  TimeInputParam t_ven_fan(param);
  start_hour_ven_fan = t_ven_fan.getStartHour();
  start_min_ven_fan = t_ven_fan.getStartMinute();
  stop_hour_ven_fan = t_ven_fan.getStopHour();
  stop_min_ven_fan = t_ven_fan.getStopMinute();
}

void auto_ven_fan() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  if (((int)p_tm->tm_hour == start_hour_ven_fan) && ((int)p_tm->tm_min == start_min_ven_fan)) {
      digitalWrite(ven_fan, 1);
    }
  if (((int)p_tm->tm_hour == stop_hour_ven_fan) && ((int)p_tm->tm_min == stop_min_ven_fan)) {
      digitalWrite(ven_fan, 0);
    }
}

// Manual mode
int ven_fan_button = 0;
BLYNK_WRITE(V32) {
  ven_fan_button = param.asInt();
}
void manual_ven_fan() {
  if (ven_fan_button == 1) {digitalWrite(ven_fan, 1);}
  if (ven_fan_button == 0) {digitalWrite(ven_fan, 0);}
}
//---------------------------------------------------------------------------------------------//
/*/Điều khiển UV khí từ Blynk
//Đọc trạng thái điều khiển UV khí
int CM_UVa = 1;
BLYNK_WRITE(V33) {
  CM_UVa = param.asInt();
}
//Điều khiển UV khí
void controlUVa() {
  if (CM_UVa == 1) {auto_UVa();}
  if (CM_UVa == 2) {manual_UVa();}
}
//Bật tắt UV khí tự động
int UVa_cycle, UVa_acttime, UVa_blynk_h, UVa_blynk_min, UVa_RESET;
BLYNK_WRITE(V34) {
  UVa_cycle = param.asInt();
}
BLYNK_WRITE(V35) {
  UVa_acttime = param.asInt();
}
BLYNK_WRITE(V36) {
  TimeInputParam t_UVa(param);
  UVa_blynk_h = t_UVa.getStartHour();
  UVa_blynk_min = t_UVa.getStartMinute();
}
BLYNK_WRITE(V37) {
  UVa_RESET = param.asInt();
}

  int UVa_on_h;
  int UVa_on_min;
  int ia = 0;
  unsigned long UVa_p_millis = 0;
void auto_UVa() {
  time_t UVa_now = time(nullptr);
  struct tm* UVa_p_tm = localtime(&UVa_now);
  int UVa_off_h, UVa_off_min;
  delay(4000);  
  if (((UVa_blynk_h == 0)&&(UVa_blynk_min == 0))||(UVa_cycle == 0)) {ia = 0;}
  if (UVa_RESET == 1) {ia = 0;}
  Serial.println(ia);
  UVa_on_h = (UVa_blynk_h + UVa_cycle*ia)%24;
  UVa_on_min = UVa_blynk_min;
  Serial.println(String("UVa start: ") + UVa_on_h +":" +UVa_on_min);
  if (((int)UVa_p_tm->tm_hour == UVa_on_h) && ((int)UVa_p_tm->tm_min == UVa_on_min) && (UVa_cycle != 0)) {
    digitalWrite(UVa, 1);
    Serial.println("UVa is ON");
  }
  UVa_off_h = (UVa_on_h + (UVa_acttime)/60)%24;
  UVa_off_min = (UVa_on_min + UVa_acttime)%60;
  Serial.println(String("UVa stop: ") + UVa_off_h +":" +UVa_off_min);
  if (((int)UVa_p_tm->tm_hour == UVa_off_h) && ((int)UVa_p_tm->tm_min == UVa_off_min) && (UVa_cycle != 0)) {
    digitalWrite(UVa, 0);
    Serial.println("UVa is OFF");
  }
  unsigned long UVa_c_millis = millis();
  if ((UVa_c_millis - UVa_p_millis) >= UVa_cycle*60*60*1000) {
    UVa_p_millis = UVa_c_millis;
    i++; 
  }
  Serial.println(digitalRead(UVa));
}
//Bật tắt UV khí thủ công
int UVa_button = 0;
BLYNK_WRITE(V38) {
  UVa_button = param.asInt();
}
void manual_UVa() {
  if (UVa_button == 1) {digitalWrite(UVa, 1);}
  if (UVa_button == 0) {digitalWrite(UVa, 0);}
}*/
//////////////////////////////////////////////////////////////////////////////



//*********************Control LEDs section***********************/////////////////
/////////////////////////////////////////////////////////////////////////////

// LED pins
int LED_A = 4, LED_B = 2, LED_C = 0, LED_D = 15;
// Get control mode from Blynk
int LED_control_mode = AUTO;
BLYNK_WRITE(V39) {
  LED_control_mode = param.asInt();
}

void controlLED() {
  if (LED_control_mode == AUTO) {auto_LED();}
  if (LED_control_mode == MANUAL) {manual_control_LED();}
}

// Automatic control LEDs
// Delare time variable
int start_hour_A, start_min_A, stop_hour_A, stop_min_A;
int start_hour_B, start_min_B, stop_hour_B, stop_min_B;
int start_hour_C, start_min_C, stop_hour_C, stop_min_C;
int start_hour_D, start_min_D, stop_hour_D, stop_min_D;

BLYNK_WRITE(V40) {
  TimeInputParam t_A(param);
  start_hour_A = t_A.getStartHour();
  start_min_A = t_A.getStartMinute();
  stop_hour_A = t_A.getStopHour();
  stop_min_A = t_A.getStopMinute();
}
BLYNK_WRITE(V42) {
  TimeInputParam t_B(param);
  start_hour_B = t_B.getStartHour();
  start_min_B = t_B.getStartMinute();
  stop_hour_B = t_B.getStopHour();
  stop_min_B = t_B.getStopMinute();
}
BLYNK_WRITE(V44) {
  TimeInputParam t_C(param);
  start_hour_C = t_C.getStartHour();
  start_min_C = t_C.getStartMinute();
  stop_hour_C = t_C.getStopHour();
  stop_min_C = t_C.getStopMinute();
}
BLYNK_WRITE(V46) {
  TimeInputParam t_D(param);
  start_hour_D = t_D.getStartHour();
  start_min_D = t_D.getStartMinute();
  stop_hour_D = t_D.getStopHour();
  stop_min_D = t_D.getStopMinute();
}
void auto_LED() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  if (((int)p_tm->tm_hour == start_hour_A) && ((int)p_tm->tm_min == start_min_A)) {
      digitalWrite(LED_A, 1);
    }
  if (((int)p_tm->tm_hour == stop_hour_A) && ((int)p_tm->tm_min == stop_min_A)) {
      digitalWrite(LED_A, 0);
    }
  if (((int)timeHour == start_hour_B) && ((int)timeMin == start_min_B)) {
      digitalWrite(LED_B, 1);
    }
  if (((int)timeHour == stop_hour_B) && ((int)timeMin == stop_min_B)) {
      digitalWrite(LED_B, 0);
    }
  if (((int)timeHour == start_hour_C) && ((int)timeMin == start_min_C)) {
      digitalWrite(LED_C, 1);
    }
  if (((int)timeHour == stop_hour_C) && ((int)timeMin == stop_min_C)) {
      digitalWrite(LED_C, 0);
    }
  if (((int)timeHour == start_hour_D) && ((int)timeMin == start_min_D)) {
      digitalWrite(LED_D, 1);
    }
  if (((int)timeHour == stop_hour_D) && ((int)timeMin == stop_min_D)) {
      digitalWrite(LED_D, 0);
    }
}
// Manual control LEDs
// Initial LEDs' status value
int LED_A_button = 0, LED_B_button = 0, LED_C_button = 0, LED_D_button = 0;
BLYNK_WRITE(V41) {
  LED_A_button = param.asInt();
}
BLYNK_WRITE(V43) {
  LED_B_button = param.asInt();
}
BLYNK_WRITE(V45) {
  LED_C_button = param.asInt();
}
BLYNK_WRITE(V47) {
  LED_D_button = param.asInt();
}
void manual_control_LED() {
  if (LED_A_button == 1) {digitalWrite(LED_A, 1);}
  if (LED_A_button == 0) {digitalWrite(LED_A, 0);}
  if (LED_B_button == 1) {digitalWrite(LED_B, 1);}
  if (LED_B_button == 0) {digitalWrite(LED_B, 0);}
  if (LED_C_button == 1) {digitalWrite(LED_C, 1);}
  if (LED_C_button == 0) {digitalWrite(LED_C, 0);}
  if (LED_D_button == 1) {digitalWrite(LED_D, 1);}
  if (LED_D_button == 0) {digitalWrite(LED_D, 0);}
}
///////////////////////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int wifi_ctr = 0;
  pinMode(UVw, OUTPUT);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);
  pinMode(LED_D, OUTPUT);
  Serial.println("Starting...");
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected, starting...");
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Lỗi khởi tạo ESP-NOW");
    return;
  }

  // Khi khởi tạo thành công espnow, đăng kí recv CB
  // để nhận thông tin từ gói recv
  esp_now_register_recv_cb(OnDataRecv);
  
  Blynk.begin(auth, ssid, password,"blynk-server.com", 8080);
  timer.setInterval(timedelay, sendDHT);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  Blynk.run();
  timer.run();
  t_tb = (boardsStruct[0].t_tb + boardsStruct[1].t_tb
          + boardsStruct[2].t_tb + boardsStruct[3].t_tb);
  h_tb = (boardsStruct[0].h_tb + boardsStruct[1].h_tb
          + boardsStruct[2].h_tb + boardsStruct[3].h_tb);
  sendDHT();
  controlhumi_pump();
  controlhumi_us();
  controlre_pump();
  controlUVw();
  controlUVws();
  control_ven_fan();
  //controlUVa();
  controlLED();
  unsigned long currentMillis = millis();
  // If wifi is disconnect in more than 30 secs, then reconnect wifi       
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= 30000)) {
     WiFi.disconnect();
     WiFi.reconnect();
     previousMillis = currentMillis;
  }
}

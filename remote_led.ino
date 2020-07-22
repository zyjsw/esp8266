#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <U8g2lib.h>
#include <dht11.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
const char* ssid = "taiyang";  //改为自己家里的wifi名称
const char* psw = "hh414567";  //改为自己家里的wifi密码
uint32_t cur_t = 0;
int tem = 0, hum = 0;
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 8 * 3600, 60000);
dht11 DHT11;

#define DHT11PIN 0   //dht11 2号引脚接在esp8266的d3（gpio0）引脚上

String html_update() {
  char temp[700];
  String state = digitalRead(BUILTIN_LED) == 1 ? "关" : "开";
  snprintf(temp, 700, "<!doctype html>\
  <html lang='en'>\
  <head>\
    <meta charset='UTF-8' http-equiv='refresh' content='10'>\
    <title>远程遥控</title>\
    <style>\
        input{\
        height:100px;\
        width: 300px;\
        text-align：center;\
        font-size: 60px;\
      }\
      h1{\
      font-size: 40px;\
      }\
    </style>\
  </head>\
  <body>\
    <form name='input' action='/' method='POST'>\
      <center><h1>灯泡状态：%s</h1><center>\
      <center><h1>室内温度：%d， 室内湿度：%d</h1><center>\
      <center><input type='submit' value='开灯' name='led_1'> <input type='submit' value='关灯' name='led_1'></center>\
    </form>\
  </body>\
  </html>", state.c_str(), tem, hum
          );
  return temp;
}

//处理post请求
void handleRoot() {
  if (server.hasArg("led_1")) {
    if (server.arg("led_1") == "开灯") {
      digitalWrite(BUILTIN_LED, LOW);
      Serial.println("led on");
    }
    if (server.arg("led_1") == "关灯") {
      digitalWrite(BUILTIN_LED, HIGH);
      Serial.println("led off");
    }
  }
  String webpage_html = html_update();
  server.send(200, "text / html", webpage_html);
}

//处理非法url请求
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += server.args();
  server.send(404, "text / plain", message);
}

void wifi_init() {
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, psw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    u8g2.firstPage();
    do {
      u8g2.setCursor(0, 15);
      u8g2.print("Connecting to ");
      u8g2.setCursor(0, 30);
      u8g2.print("WiFi...");
    } while (u8g2.nextPage());
  }
  Serial.println("connected wifi");
}

//初始化服务器
void initWebServer() {
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy15_t_gb2312);
  wifi_init();
  initWebServer();
  timeClient.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();
  String t = timeClient.getFormattedTime();

  if (millis() - cur_t >= 10000) {
    int chk = DHT11.read(DHT11PIN);
    tem = (float)DHT11.temperature;             //将温度值赋值给tem
    hum = (float)DHT11.humidity;                 //将湿度值赋给hum
    cur_t = millis();
  }
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("时间:");
  u8g2.setCursor(35, 15);
  u8g2.print(t);

  u8g2.setCursor(0, 31);
  u8g2.print("温度:");
  u8g2.setCursor(35, 31);
  u8g2.print(tem);
  u8g2.setCursor(50, 31);
  u8g2.print("℃");

  u8g2.setCursor(65, 31);
  u8g2.print("湿度:");
  u8g2.setCursor(100, 31);
  u8g2.print(hum);
  u8g2.setCursor(118, 31);
  u8g2.print("%");

  u8g2.setCursor(0, 47);
  u8g2.print("服务器IP:");
  u8g2.setCursor(0, 64);
  u8g2.print(WiFi.localIP());
  u8g2.sendBuffer();
}

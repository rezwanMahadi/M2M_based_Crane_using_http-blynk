#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

WiFiServer server(80);

#define i2c_Address 0x3C
#define boom_up D4
#define boom_down D3
#define forward D8
#define back D7
#define left D6
#define right D5
#define clip D0
#define pwm A0
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "fkr";
const char* pass = "1234567890";
const char* host = "192.168.0.105";

// char ssid[] = "Alpha";
// char pass[] = "245025.asdfjkl";

IPAddress local_IP(192, 168, 4, 2);
IPAddress gateway(192, 168, 2, 4);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

int clip_state = 0;
int i = 0;
int previousCount = 0;
int count = 0;
int previousPwm = 0;
int currentPwm = 0;
int weight = 0;
String baseIP = "192.168.4.6";
String boomIP = "192.168.4.7";

void testdrawrect(void) {
  for (int16_t i = 0; i < oled.height() / 2; i += 2) {
    oled.drawRect(i, i, oled.width() - 2 * i, oled.height() - 2 * i, SH110X_WHITE);
    oled.display();
    delay(1);
  }
}

void wificonnect() {
  Serial.print("CONNECTING WIFI");
  WiFi.mode(WIFI_STA);
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //   Serial.println("Failed Config");
  // }
  WiFi.begin(ssid, pass);
  oled.setTextColor(SH110X_WHITE);
  oled.clearDisplay();
  oled.setCursor(5, 0);
  oled.setTextSize(2);
  oled.print("CONNECTING");
  oled.setCursor(40, 20);
  oled.print("WIFI");
  oled.drawRect(12, 38, 100, 10, SH110X_WHITE);
  oled.display();
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    oled.fillRect(15, 40, i, 6, SH110X_WHITE);
    Serial.println(".");
    oled.display();
    if (i <= 53) {
      i++;
    }
  }
  for (i = 53; i <= 94; i++) {
    oled.fillRect(15, 40, i, 6, SH110X_WHITE);
    oled.display();
  }
  delay(2000);
  i = 0;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("CONTROLLER CONNECTED");
  oled.setCursor(0, 10);
  oled.print(WiFi.localIP());
  oled.display();
  delay(3000);
}

void setup() {
  Serial.begin(115200);
  oled.begin(i2c_Address, true);
  oled.clearDisplay();
  wificonnect();
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  pinMode(boom_up, INPUT_PULLUP);
  pinMode(boom_down, INPUT_PULLUP);
  pinMode(forward, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(back, INPUT_PULLUP);
  pinMode(clip, INPUT_PULLUP);
  server.begin();
  oled.clearDisplay();
  oled.display();
}

void loop() {
  while ((WiFi.status() == WL_CONNECTED)) {
    currentPwm = analogRead(pwm);
    int dispPWM = map(currentPwm, 0, 1024, 0, 125);
    Serial.print(count);
    Serial.print("     ");
    Serial.println(previousCount);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("CONTAINER :");
    oled.setCursor(70, 0);
    oled.print(count);
    oled.setCursor(0, 10);
    oled.print("WEIGHT :");
    oled.setCursor(52, 10);
    oled.print(weight);
    oled.drawRect(0, 20, 127, 10, SH110X_WHITE);
    oled.fillRect(3, 22, dispPWM - 4, 6, SH110X_WHITE);
    oled.display();
    WiFiClient Client = server.available();
    if (Client == 1) {
      String request = Client.readStringUntil('\n');
      request.trim();
      String input = request;
      int startIndex = 0;
      int endIndex = request.indexOf('$');
      String newInput = request.substring(startIndex, endIndex);
      // Serial.println(request);
      // Serial.println(newInput);
      if (newInput == "GET /weight") {
        int startIndex = request.indexOf('<') + 1;
        int endIndex = request.indexOf('>');
        String value = request.substring(startIndex, endIndex);
        weight = value.toInt();
        // Serial.println(request);
        int startIndex2 = request.indexOf('>') + 1;
        int endIndex2 = request.indexOf('*');
        String value2 = request.substring(startIndex2, endIndex2);
        count = value2.toInt();
        // Serial.println(value2);
      }
    }
    if (digitalRead(boom_up) == LOW) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/boomUp$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("BOOM UP");
      oled.display();
      Serial.println("boomUp");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(boom_down) == LOW) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/boomDown$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("BOOM DOWN");
      oled.display();
      Serial.println("boomDown");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(forward) == HIGH) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/forward$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("FORWARD");
      oled.display();
      Serial.println("forward");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(back) == LOW) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/back$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("BACK");
      oled.display();
      Serial.println("back");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(right) == LOW) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/right$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("RIGHT");
      oled.display();
      Serial.println("right");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(left) == LOW) {
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/left$";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print("LEFT");
      oled.display();
      Serial.println("left");
      int response = http.GET();
      http.end();
      delay(100);
    } else if (digitalRead(clip) == HIGH) {
      if (clip_state == 0) {
        HTTPClient http;
        WiFiClient client;
        String address = "http://" + boomIP + "/clipopen$";
        http.begin(client, address);
        oled.setTextSize(2);
        oled.setCursor(0, 40);
        oled.print("CLIP OPEN");
        oled.display();
        Serial.println("clipopen");
        int response = http.GET();
        http.end();
        clip_state = 1;
        delay(100);
      } else if (clip_state == 1) {
        HTTPClient http;
        WiFiClient client;
        String address = "http://" + boomIP + "/clipclose$";
        http.begin(client, address);
        oled.setTextSize(2);
        oled.setCursor(0, 40);
        oled.print("CLIP CLOSE");
        oled.display();
        Serial.println("clipclose");
        int response = http.GET();
        http.end();
        clip_state = 0;
        delay(100);
      }
    } else if (abs(currentPwm - previousPwm) >= 30) {
      int pwmValue = analogRead(pwm);
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + baseIP + "/pwm$/<" + String(pwmValue) + ">";
      http.begin(client, address);
      oled.setTextSize(2);
      oled.setCursor(0, 40);
      oled.print(pwmValue);
      oled.display();
      Serial.println(pwmValue);
      int response = http.GET();
      http.end();
      currentPwm = pwmValue;
      previousPwm = currentPwm;
      delay(100);
    }
    // else if (previousCount < count) {
    //   WiFiClient client3;
    //   const int httpPort = 8081;
    //   if (!client3.connect(host, httpPort)) {
    //     Serial.println("connection failed");
    //     return;
    //   }
    //   String url = "/";
    //   Serial.print("Requesting URL: ");
    //   Serial.println(url);
    //   int c = 1;
    //   String data = "count=" + String(c) + "&totalCount=" + String(count) + "&weight=" + String(weight);
    //   Serial.print("Requesting POST: ");
    //   client3.println("POST / HTTP/1.1");
    //   client3.println("Host: server_name");
    //   client3.println("Accept: /");
    //   client3.println("Content-Type: application/x-www-form-urlencoded");
    //   client3.print("Content-Length: ");
    //   client3.println(data.length());
    //   client3.println();
    //   client3.print(data);
    //   previousCount=count;
    //   unsigned long timeout = millis();
    //   while (client3.available() == 0) {
    //     if (millis() - timeout > 5000) {
    //       Serial.println(">>> Client Timeout !");
    //       client3.stop();
    //       return;
    //     }
    //   }
    //   while (client3.available()) {
    //     String line = client3.readStringUntil('\r');
    //     Serial.print(line);
    //   }
    // }
  }
  wificonnect();
}

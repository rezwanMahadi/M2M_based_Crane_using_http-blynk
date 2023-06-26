#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// WiFiClient client;
WiFiServer server(80);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define mr1 D7
#define mr2 D6
#define ml1 D5
#define ml2 D0
#define rPWM D4
#define lPWM D3
#define r1 1
#define r2 3
#define ls1 A0
#define ls2 D8

const char* ssid = "fkr";
const char* password = "1234567890";

// IPAddress local_IP(192, 168, 22, 2);
// IPAddress gateway(192, 168, 2, 22);
// IPAddress subnet(255, 255, 0, 0);
// IPAddress primaryDNS(8, 8, 8, 8);
// IPAddress secondaryDNS(8, 8, 4, 4);

String m;
int pwm;
int t = 60;
int count = 0;

void forward() {
  digitalWrite(mr2, HIGH);
  digitalWrite(mr1, LOW);
  digitalWrite(ml1, HIGH);
  digitalWrite(ml2, LOW);
  analogWrite(rPWM, pwm);
  analogWrite(lPWM, pwm);
}

void back() {
  digitalWrite(mr2, LOW);
  digitalWrite(mr1, HIGH);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, HIGH);
  analogWrite(rPWM, pwm);
  analogWrite(lPWM, pwm);
}

void right() {
  digitalWrite(mr2, HIGH);
  digitalWrite(mr1, LOW);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, HIGH);
  analogWrite(rPWM, pwm);
  analogWrite(lPWM, pwm);
}

void left() {
  digitalWrite(mr2, LOW);
  digitalWrite(mr1, HIGH);
  digitalWrite(ml1, HIGH);
  digitalWrite(ml2, LOW);
  analogWrite(rPWM, pwm);
  analogWrite(lPWM, pwm);
}

void stop() {
  digitalWrite(mr1, LOW);
  digitalWrite(mr2, LOW);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, LOW);
  analogWrite(rPWM, 0);
  analogWrite(lPWM, 0);
  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
}

void boomUp() {
  if (analogRead(ls1) <= 1000) {
    digitalWrite(r1, LOW);
    digitalWrite(r2, HIGH);
  }
}

void boomDown() {
  if (digitalRead(ls2) == 0) {
    digitalWrite(r1, HIGH);
    digitalWrite(r2, LOW);
  }
}

void setup() {
  pinMode(mr1, OUTPUT);
  pinMode(mr2, OUTPUT);
  pinMode(ml1, OUTPUT);
  pinMode(ml2, OUTPUT);
  pinMode(ls1, INPUT);
  pinMode(ls2, INPUT);
  pinMode(rPWM, OUTPUT);
  pinMode(lPWM, OUTPUT);
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  // Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  wificonnect();
  server.begin();
}

void loop() {
  lcd.clear();
  while ((WiFi.status() == WL_CONNECTED)) {
    // lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CONTAINER : ");
    lcd.setCursor(0, 1);
    lcd.print("WEIGHT : ");
    // lcd.setCursor(12, 0);
    // lcd.print(count);
    WiFiClient client;
    client = server.available();
    if (client == 1) {
      String request = client.readStringUntil('\n');
      request.trim();
      String input = request;
      int startIndex = 0;
      int endIndex = request.indexOf('$');
      String newInput = request.substring(startIndex, endIndex);
      if (newInput == "GET /boomUp") {
        boomUp();
        delay(t);
      } else if (newInput == "GET /boomDown") {
        boomDown();
        delay(t);
      } else if (newInput == "GET /forward") {
        forward();
        delay(t);
      } else if (newInput == "GET /back") {
        back();
        delay(t);
      } else if (newInput == "GET /right") {
        right();
        delay(t);
      } else if (newInput == "GET /left") {
        left();
        delay(t);
      } else if (newInput == "GET /pwm") {
        int startIndex = request.indexOf('<') + 1;
        int endIndex = request.indexOf('>');
        String value = request.substring(startIndex, endIndex);
        pwm = value.toInt();
      } else if (newInput == "GET /weight") {
        int startIndex1 = request.indexOf('<') + 1;
        int endIndex1 = request.indexOf('>');
        String value1 = request.substring(startIndex, endIndex);
        int weight = value1.toInt();
        int startIndex = request.indexOf('>') + 1;
        int endIndex = request.indexOf('*');
        String value = request.substring(startIndex, endIndex);
        count = value.toInt();
        // lcd.clear();
        lcd.setCursor(12, 0);
        lcd.print(value);
        lcd.setCursor(9, 1);
        lcd.print(weight);
      }
    } else {
      stop();
    }
  }
  wificonnect();
}

void wificonnect() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONNECTING WIFI");
  lcd.setCursor(0, 1);
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //   lcd.clear();
  //   lcd.setCursor(0, 0);
  //   lcd.print("Failed Config");
  // }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WIFI CONNECTED");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
}

#define BLYNK_TEMPLATE_ID "TMPL62K7zgsrp"
#define BLYNK_TEMPLATE_NAME "Crane"
#define BLYNK_AUTH_TOKEN "DorHwuUIlzp6tmxPyXISFbGjhZDyQ0WC"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>
#include <Wire.h>
#include <BlynkSimpleEsp8266.h>

// IPAddress local_IP(192, 168, 25, 5);
// IPAddress gateway(192, 168, 5, 25);
// IPAddress subnet(255, 255, 255, 255);
// IPAddress primaryDNS(8, 8, 8, 8);
// IPAddress secondaryDNS(8, 8, 4, 4);

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "fkr";
char pass[] = "1234567890";
const char* host = "192.168.4.3";

// char ssid[] = "Alpha";
// char pass[] = "245025.asdfjkl";

#define VPIN_BUTTON_1 V0
#define VPIN_BUTTON_2 V1
#define VPIN_BUTTON_3 V2
#define VPIN_BUTTON_4 V3
#define VPIN_BUTTON_5 V4
#define VPIN_BUTTON_6 V5
#define Blynk_Vpin_IR_1 V6
#define Blynk_Vpin_IR_2 V7
#define Blynk_Vpin_servo V8
#define Blynk_Vpin_count V9

int RightMotor[] = { 12, 14 };
int LeftMotor[] = { 15, 13 };
int boomMotor[] = { 1, 3 };
int irPin[] = { 16, 0 };
int weightPin = A0;
int count = 0;
int previousStage = 0;
int midle_stage = 0;
int count_status = 0;
BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2);

Adafruit_MPU6050 mpu;

Servo clip;
int i = 0;

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON_1);
  Blynk.syncVirtual(VPIN_BUTTON_2);
  Blynk.syncVirtual(VPIN_BUTTON_3);
  Blynk.syncVirtual(VPIN_BUTTON_4);
  Blynk.syncVirtual(VPIN_BUTTON_5);
  Blynk.syncVirtual(VPIN_BUTTON_6);
  Blynk.syncVirtual(Blynk_Vpin_IR_1);
  Blynk.syncVirtual(Blynk_Vpin_IR_2);
  Blynk.syncVirtual(Blynk_Vpin_servo);
  Blynk.syncVirtual(Blynk_Vpin_count);
}

void wheel(int a) {
  switch (a) {
    case 1: /*FORWARD*/
      digitalWrite(RightMotor[1], LOW);
      digitalWrite(RightMotor[0], HIGH);
      digitalWrite(LeftMotor[1], LOW);
      digitalWrite(LeftMotor[0], HIGH);
      lcd.clear();
      lcd.print("FORWARD");
      break;
    case 2: /*BACKWARD*/
      digitalWrite(LeftMotor[0], LOW);
      digitalWrite(LeftMotor[1], HIGH);
      digitalWrite(RightMotor[0], LOW);
      digitalWrite(RightMotor[1], HIGH);
      lcd.clear();
      lcd.print("BACK");
      break;
    case 3: /*RIGHT*/
      digitalWrite(LeftMotor[0], HIGH);
      digitalWrite(LeftMotor[1], LOW);
      digitalWrite(RightMotor[0], LOW);
      digitalWrite(RightMotor[1], HIGH);
      lcd.clear();
      lcd.print("RIGHT");
      break;
    case 4: /*LEFT*/
      digitalWrite(LeftMotor[0], LOW);
      digitalWrite(LeftMotor[1], HIGH);
      digitalWrite(RightMotor[0], HIGH);
      digitalWrite(RightMotor[1], LOW);
      lcd.clear();
      lcd.print("LEFT");
      break;
    default: /*STOP*/
      digitalWrite(LeftMotor[0], LOW);
      digitalWrite(LeftMotor[1], LOW);
      digitalWrite(RightMotor[0], LOW);
      digitalWrite(RightMotor[1], LOW);
      lcd.clear();
      lcd.print("STOP");
      break;
  }
}

void boom(int a) {
  switch (a) {
    case 1: /*UP*/
      digitalWrite(boomMotor[0], HIGH);
      digitalWrite(boomMotor[1], LOW);
      break;

    case 2: /*DOWN*/
      digitalWrite(boomMotor[0], LOW);
      digitalWrite(boomMotor[1], HIGH);
      break;

    default: /*STOP*/
      digitalWrite(boomMotor[0], HIGH);
      digitalWrite(boomMotor[1], HIGH);
      break;
  }
}

BLYNK_WRITE(VPIN_BUTTON_1) /*Boom Up*/
{
  int State = param.asInt();
  if (State == 1) {
    boom(1);
    delay(100);
    boom(0);
    lcd.clear();
  }
}

BLYNK_WRITE(VPIN_BUTTON_6) /*Boom Down*/
{
  int State = param.asInt();
  if (State == 1) {
    boom(2);
    delay(100);
    boom(0);
    lcd.clear();
  }
}

BLYNK_WRITE(VPIN_BUTTON_2) /*FORWARD*/
{
  int state = param.asInt();
  if (state == 1) {
    wheel(1);
    delay(50);
    wheel(0);
  }
}

BLYNK_WRITE(VPIN_BUTTON_3) /*BACKWARD*/
{
  int state = param.asInt();
  if (state == 1) {
    wheel(2);
    delay(50);
    wheel(0);
  }
}

BLYNK_WRITE(VPIN_BUTTON_4) /*RIGHT*/
{
  int State = param.asInt();
  if (State == 1) {
    wheel(3);
    delay(60);
    wheel(0);
  }
}

BLYNK_WRITE(VPIN_BUTTON_5) /*LEFT*/
{
  int State = param.asInt();
  if (State == 1) {
    wheel(4);
    delay(60);
    wheel(0);
  }
}

BLYNK_WRITE(Blynk_Vpin_servo) /*SERVO*/
{
  int State = param.asInt();
  if (analogRead(A0) >= 800) {
    clip.write(State);
    delay(1000);
    if (State == 0) {
      count_status = 1;
    } else {
      count_status = 0;
    }
  } else {
    Blynk.virtualWrite(Blynk_Vpin_servo, 180);
  }
}

void wificonnect() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONNECTING WIFI");
  lcd.setCursor(0, 1);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(300);
    i++;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WIFI CONNECTED");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  i = 0;
  delay(1000);
}

void sendSensor() {
  int ir_2 = 0;
  if (analogRead(A0) >= 800) {
    ir_2 = 1;
  }
  // Blynk.virtualWrite(Blynk_Vpin_IR_1, !digitalRead(irPin[0]));
  Blynk.virtualWrite(Blynk_Vpin_IR_1, ir_2);
  Blynk.virtualWrite(Blynk_Vpin_count, count);
}

void setup() {
  for (int x = 0; x <= 1; x++) {  // pinMode
    pinMode(RightMotor[x], OUTPUT);
    pinMode(LeftMotor[x], OUTPUT);
    pinMode(boomMotor[x], OUTPUT);
    pinMode(irPin[x], INPUT);
  }
  digitalWrite(boomMotor[0], HIGH);
  digitalWrite(boomMotor[1], HIGH);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Adafruit MPU6050 test!");
  delay(1000);
  if (!mpu.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NO MPU6050 CHIP");
    while (1) {
      delay(10);
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MPU6050 FOUND!");
  delay(1000);
  wificonnect();
  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
  clip.attach(2);
  clip.write(180);
  lcd.clear();
  Blynk.config(auth);
  Blynk.connect();
  timer.setInterval(1000L, sendSensor);
  delay(2000);
  lcd.clear();
}

void loop() {
  while ((WiFi.status() == WL_CONNECTED)) {
    if (mpu.getMotionInterruptStatus() && count_status == 1) {
      count_status = 0;
      count++;
      WiFiClient client;
      const int httpPort = 8081;
      if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
      }
      String url = "/";
      Serial.print("Requesting URL: ");
      Serial.println(url);
      int c = 1;
      String data = "count=" + String(c) + "&totalCount=" + String(count);
      Serial.print("Requesting POST: ");
      client.println("POST / HTTP/1.1");
      client.println("Host: server_name");
      client.println("Accept: /");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.print(data);
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
        }
      }
      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
    }
    Blynk.run();
    timer.run();
  }
  wificonnect();
}

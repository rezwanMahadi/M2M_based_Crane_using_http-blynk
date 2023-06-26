#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>

#define clipPin D4
#define forcePin A0
#define ir_1_pin D5
#define ir_2_pin D6

Adafruit_MPU6050 mpu;
WiFiServer server(80);
Servo clip;

IPAddress local_IP(192, 168, 21, 1);
IPAddress gateway(192, 168, 1, 21);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

const char* ssid = "fkr";
const char* pass = "1234567890";
// char ssid[] = "Alpha";
// char pass[] = "245025.asdfjkl";
const char* host = "192.168.4.3";

String controllerIP = "192.168.4.5";
String baseIP = "192.168.4.6";
int t = 100;
unsigned long previousmilis = 0;
int current_x = 0;
int previous_x = 0;
int count = 0;
int count_status = 0;
int clip_State = 0;

void wificonnect() {
  Serial.print("CONNECTING WIFI");
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //   Serial.println("Failed Config");
  // }
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WIFI CONNECTED");
  Serial.println(WiFi.localIP());
  delay(3000);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ir_1_pin, INPUT);
  pinMode(ir_2_pin, INPUT);
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit MPU6050 test!");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  Serial.println("");
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
  delay(100);
  wificonnect();
  clip.attach(clipPin);
  clip.write(180);
  delay(1000);
  server.begin();
}

void loop() {
  while ((WiFi.status() == WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, LOW);
    if (mpu.getMotionInterruptStatus() && count_status == 1) {
      count_status = 0;
      count++;
      int f = analogRead(forcePin);
      int weight = map(f, 900, 0, 0, 10);
      HTTPClient http;
      WiFiClient client;
      String address = "http://" + controllerIP + "/weight$/<" + String(weight) + ">" + String(count) + "*";
      Serial.println(address);
      http.begin(client, address);
      int response = http.GET();
      http.end();
      client.stop();

      HTTPClient http3;
      WiFiClient client3;
      String address3 = "http://" + baseIP + "/weight$/<" + String(weight) + ">" + String(count) + "*";
      Serial.println(address3);
      http3.begin(client3, address3);
      int response3 = http3.GET();
      http3.end();
      client3.stop();

      WiFiClient client2;
      const int httpPort = 8081;
      if (!client2.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
      }
      String url = "/";
      Serial.print("Requesting URL: ");
      Serial.println(url);
      int c = 1;
      int n = 2;
      String data = "Crane=" + String(n) + "&count=" + String(c) + "&totalCount=" + String(count) + "&weight=" + String(weight);
      Serial.print("Requesting POST: ");
      client2.println("POST / HTTP/1.1");
      client2.println("Host: server_name");
      client2.println("Accept: /");
      client2.println("Content-Type: application/x-www-form-urlencoded");
      client2.print("Content-Length: ");
      client2.println(data.length());
      client2.println();
      client2.print(data);
      unsigned long timeout = millis();
      while (client2.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client2.stop();
          return;
        }
      }
      while (client2.available()) {
        String line = client2.readStringUntil('\r');
        Serial.print(line);
      }
      // previousmilis = currentmilis;
    }
    WiFiClient client;
    client = server.available();
    if (client == 1) {
      String request = client.readStringUntil('\n');
      request.trim();
      String input = request;
      int startIndex = 0;
      int endIndex = request.indexOf('$');
      String newInput = request.substring(startIndex, endIndex);
      if (newInput == "GET /clipopen") {
        Serial.println("clipopen");
        if (digitalRead(ir_1_pin) == 0 && digitalRead(ir_2_pin) == 0) {
          clip.write(180);
          delay(1000);
          count_status = 0;
        }
      } else if (newInput == "GET /clipclose") {
        Serial.println("clipclose");
        if (digitalRead(ir_1_pin) == 0 && digitalRead(ir_2_pin) == 0) {
          clip.write(0);
          delay(1000);
          count_status = 1;
        }
      }
    }
  }
  wificonnect();
}
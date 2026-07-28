#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>

const char* ssid = "Test Code";
const char* password = "12345678";

WebServer server(80);

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
Adafruit_SSD1306 display(128, 32, &Wire, -1);
TinyGPSPlus gps;

const int trigPins[] = {23, 32, 5, 4, 13};
const int echoPins[] = {26, 25, 14, 18, 19};
String usNames[] = {"45_Deg", "Front", "Back", "Left", "Right"};

float readUS(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  return duration * 0.034 / 2.0;
}

void handleRoot() {
  String html = "<html><head><meta http-equiv='refresh' content='2'><style>body{font-family:sans-serif;}</style></head><body><h2>Sensor Diagnostics</h2>";

  sensors_event_t event;
  bno.getEvent(&event);
  html += "<p><b>BNO055 IMU:</b> X: " + String(event.orientation.x) + " Y: " + String(event.orientation.y) + " Z: " + String(event.orientation.z) + "</p>";

  html += "<p><b>GPS (GY-NEO6MV2):</b> ";
  if (gps.location.isValid()) {
    html += "Lat: " + String(gps.location.lat(), 6) + " Lng: " + String(gps.location.lng(), 6);
  } else {
    html += "Waiting for fix";
  }
  html += "</p>";

  html += "<p><b>LiDAR (LD20):</b> ";
  if (Serial1.available()) {
    html += "UART Data Active (" + String(Serial1.available()) + " bytes waiting)";
    while(Serial1.available()) Serial1.read();
  } else {
    html += "No UART data received";
  }
  html += "</p>";

  html += "<p><b>Ultrasonic Sensors (cm):</b><br>";
  for(int i=0; i<5; i++) {
    html += usNames[i] + ": " + String(readUS(trigPins[i], echoPins[i])) + "<br>";
  }
  html += "</p></body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 27, 33);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  for(int i=0; i<5; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  Wire.begin(21, 22);
  bno.begin();

  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Sensors Init OK");
    display.println("Connect to WiFi");
    display.display();
  }

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("URL: http://");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }
}
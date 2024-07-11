#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>

//
//#define WIFI_SSID "Xom Tro"
//#define WIFI_PASSWORD "lecong113"
//#define WIFI_SSID "Minh dep trai"
//#define WIFI_PASSWORD "0968375929@"
//#define WIFI_SSID "iphone 16"
//#define WIFI_PASSWORD "55555555"

//#define WIFI_SSID "nqp"
//#define WIFI_PASSWORD "88888889"
//#define WIFI_SSID "T 2"
//#define WIFI_PASSWORD "123456789"
#define WIFI_SSID "Sơn"
#define WIFI_PASSWORD "motdentam"

#define FIREBASE_HOST "datn-ea67f-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "R9zWunxt5CniNk6QW5O6cXVPMYjm4hqYBmfNsbHP"

constexpr uint8_t RST_PIN = D3;
constexpr uint8_t SS_PIN = D4;
MFRC522 rfid(SS_PIN, RST_PIN);

String tag;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("RFID Tag Scanner"); // Display title
  display.display();
  delay(2000); // Pause for 2 seconds

  SPI.begin();
  rfid.PCD_Init();
  timeClient.begin();
}

void loop() {
  timeClient.update();
  String formattedDate = timeClient.getFormattedDate();

  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (rfid.PICC_ReadCardSerial()) {
    tag = "";
    for (byte i = 0; i < 4; i++) {
      tag += String(rfid.uid.uidByte[i], HEX);
    }
    tag.toUpperCase();
    Serial.println("RFID Tag ID: " + tag);

    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("RFID Tag ID:");
    display.println(tag);
    display.display();
    delay(2000); // Pause for 2 seconds

    if (!Firebase.getBool("/Check-in/" + tag)) {
      Firebase.setString("/Check-in/" + tag + "/time", formattedDate);
      Serial.println("New data sent to Firebase");
    } else {
      Firebase.setString("/Check-in/" + tag + "/time", formattedDate);
      Serial.println("Data updated on Firebase");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

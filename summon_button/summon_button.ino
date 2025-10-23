#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Information
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Telegram Information
const String botToken = TELEGRAM_BOT_TOKEN;
const String chatId = TELEGRAM_CHAT_ID;

// GPIO Fields
const int buttonPin = 4;
bool lastState = LOW;

// Message/Scroll LCD Fields
String message = "Chase has been summoned...";
int scrollIndex = 0;
unsigned long lastScroll = 0;
const unsigned long scrollDelay = 300; // ms per scroll step
bool scrolling = false;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(buttonPin, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("Ready.");
  delay(2000);
  lcd.noBacklight();
}

void loop() {
  bool currentState = digitalRead(buttonPin);

  // Trigger only once per full cycle
  if (currentState == HIGH && lastState == LOW && !scrolling) {
    scrolling = true;
    scrollIndex = 0;
    lastScroll = millis();

    lcd.clear();
    lcd.backlight();
    Serial.println("Button Pressed!");
    sendTelegramMessage("🚨 The button has been pressed...");
  }
  lastState = currentState;

  // Handle scrolling
  if (scrolling && millis() - lastScroll >= scrollDelay) {
    lcd.setCursor(0, 0);
    if (message.length() <= 16) {
      lcd.print(message);
    } else {
      String toDisplay;
      if (scrollIndex + 16 <= message.length()) {
        toDisplay = message.substring(scrollIndex, scrollIndex + 16);
      } else {
        toDisplay = message.substring(scrollIndex) + "                ";
        toDisplay = toDisplay.substring(0, 16);
      }
      lcd.print(toDisplay);
      scrollIndex++;
      if (scrollIndex > message.length() + 16) {
        scrolling = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ready.");
        lcd.noBacklight();
      }
    }
    lastScroll = millis();
  }
}

void sendTelegramMessage(String text) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "https://api.telegram.org/bot" + String(botToken) +
                 "/sendMessage?chat_id=" + String(chatId) +
                 "&text=" + text;

    http.begin(url);
    int httpResponseCode = http.GET();

    Serial.print("Telegram Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }
}


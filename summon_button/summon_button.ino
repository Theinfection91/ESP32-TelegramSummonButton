#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"

// Message/Scroll LCD Fields
LiquidCrystal_I2C lcd(0x27, 16, 2);
String message = "Chase has been summoned...";
int scrollIndex = 0;
unsigned long lastScroll = 0;
const unsigned long scrollDelay = 300; // ms per scroll step
bool scrolling = false;

// GPIO Fields
const int buttonPin = 4;
bool lastState = LOW;

// WiFi and Telegram Bot Fields
WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, client);

// Timing Fields
int botRequestDelay = 1000; // Every one second
unsigned long lastTimeBotRan;

void setup() {
  // Initialize Serial, Button, and LCD
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Connect to WiFi
  lcd.print("Connecting WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // WiFi connected, clear LCD and turn off backlight, notify via Telegram
  lcd.clear();
  lcd.print("Ready.");
  delay(2000);
  lcd.noBacklight();
  bot.sendMessage(TELEGRAM_CHAT_ID, "🤖 The summon button is now online.");
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

    String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) +
                 "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) +
                 "&text=" + text;

    http.begin(url);
    int httpResponseCode = http.GET();

    Serial.print("Telegram Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }
}


#include <DHT.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include "Waveshare_LCD1602.h"

// WiFi Credentials
#define WIFI_SSID "TelstraDBFC51"
#define WIFI_PASSWORD "fqwt6djgzc"

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds =
    (10.5 * 3600);  // Adjust for timezone (e.g., UTC + 10.5)

// Create an NTPClient Instance
WiFiUDP udp;
NTPClient timeClient(udp, ntpServer, utcOffsetInSeconds);

// LCD Configuration
Waveshare_LCD1602 lcd(16, 2);  // 16 Characters & 2 Lines
int r, g, b, t = 0;            // SDA: D2 (GPIO 4) & SCL: D1 (GPIO 5)
#define LCD_POWER D0           // GPIO pin to control LCD Power

// DHT Sensor Configuration
#define DHTTYPE DHT22
#define DHTPIN D7     // GPIO pin to control DHT Data
#define DHT_POWER D8  // GPIO pin to control DHT Power
DHT dht(DHTPIN, DHTTYPE);

// Variables to Store Sensor Data
float temperature = 0.0;
float humidity = 0.0;
char temp[10];
char humd[10];

// Custom Charcter for Degrees Symbol
uint8_t degreeSymbol[8] = {0b00110, 0b01001, 0b01001, 0b00110,
                           0b00000, 0b00000, 0b00000, 0b00000};

// CODE THAT RUNS ONCE AT START-UP
void setup() {
  // Wait 5 seconds Before Doing Anything Else
  delay(5000);
  Serial.begin(115200);

  // Start the lCD Display and Serial
  pinMode(LCD_POWER, OUTPUT);     // Enable Pin Writing
  digitalWrite(LCD_POWER, HIGH);  // Turn Power Output On
  lcd.init();
  lcd.customSymbol(0, degreeSymbol);  // Create and store the degree symbol

  // Initialise DHT Sensor
  pinMode(DHT_POWER, OUTPUT);     // Enable Pin Writing
  digitalWrite(DHT_POWER, HIGH);  // Turn Power Output On
  dht.begin();

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);                   // Set WiFi Mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Join WiFi Network

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the NTP Client
  timeClient.begin();

  // Synchronise with NTP server
  timeClient.update();
}

// CODE TO TURN OFF DEVICES
void sleepMode() {
  // Notify Me of Sleep Mode
  Serial.println("Entering Sleep Mode...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.send_string("Sleeping...");
  delay(5000);

  // Disconnect from Wi-Fi to Save Power
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  // Turn Off Temp Sensor
  digitalWrite(DHT_POWER, LOW);

  // Turn Off Display
  digitalWrite(LCD_POWER, LOW);

  // Push Control Chip into Sleep
  delay(3600 * 8000);  // Sleep for 8 Hours

  // Wake Chip Up
  Serial.println("Exiting Sleep Mode...");
  setup();
}

// CODE TO DISPLAY DATA
void displayData() {
  // Print Information to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");

  // Display Temperature for 30 seconds
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.send_string("TEMPERATURE:");
  lcd.setCursor(5, 1);
  lcd.send_string(temp);
  lcd.write_char(0);  // Add the degree symbol
  lcd.send_string("C");
  delay(30000);

  // Display Humidity for 30 seconds
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.send_string("HUMIDITY:");
  lcd.setCursor(5, 1);
  lcd.send_string(humd);
  lcd.send_string("%");
  delay(30000);
}

// CODE THAT RUNS REPEATEDLY EACH CYCLE
void loop() {
  // Update the Time
  timeClient.update();

  // Get the Current Hour
  int currentHour = timeClient.getHours();

  // Check if it is Time to Sleep
  if (currentHour < 8) {  // Sleep at 12 AM for 8 Hours
    // Put the Device to Sleep
    sleepMode();
  } else {
    // Read Sensor Data
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Convert to a String
    dtostrf(temperature, 0, 1, temp);
    dtostrf(humidity, 0, 1, humd);

    // If Data is Successfully Obtained, Print to Serial and LCD
    if (isnan(temperature) || isnan(humidity)) {
      // Send Error Messages
      lcd.clear();
      lcd.send_string("Sensor Error!");
      Serial.println("Failed to read from DHT sensor!");
    } else {
      // Display the Data from the Sensor
      displayData();
    }
  }
}
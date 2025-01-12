#include <DHT.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

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
LiquidCrystal_I2C lcd(0x27, 16, 2);  // SDA: D2 (GPIO 4) & SCL: D1 (GPIO 5)

// DHT Sensor Configuration
#define DHTTYPE DHT22
#define DHTPIN D7     // GPIO pin to control DHT Data
#define DHT_POWER D8  // GPIO pin to control DHT Power
DHT dht(DHTPIN, DHTTYPE);

// Variables to Store Sensor Data
float temperature = 0.0;
float humidity = 0.0;

// CODE THAT RUNS ONCE AT START-UP
void setup() {
  // Wait 5 seconds Before Doing Anything Else
  delay(5000);

  // Start the Device
  Serial.begin(115200);
  lcd.init();

  // Initialise DHT Sensor
  pinMode(DHT_POWER, OUTPUT);     // Enable Pin Writing
  digitalWrite(DHT_POWER, HIGH);  // Turn Power Output On
  dht.begin();                    // Start Sensor

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

  // Turn on the LCD Backlight
  lcd.backlight();
}

// CODE TO TURN OFF DEVICES
void sleepMode() {
  // Notify Me of Sleep Mode
  Serial.println("Entering Sleep Mode...");
  lcd.print("Sleeping...");
  delay(5000);

  // Turn off Temp Sensor
  digitalWrite(DHT_POWER, LOW);

  // Turn off the LCD Display
  lcd.off();

  // Disconnect from Wi-Fi to Save Power
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  // Push Control Chip into Sleep
  delay(3600 * 8000);  // Sleep for 8 Hours

  // Wake Chip Up
  Serial.println("Exiting Sleep Mode...");
  setup();
  lcd.print("Waking Up...");
}

// CODE TO DISPLAY DATA
void displayData() {
  // Print Information to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");

  // Display on LCD
  lcd.clear();
  lcd.print("Temperature: ");
  lcd.print(temperature, 1);
  lcd.print("°C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity, 1);
  lcd.print("%");
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

    // If Data is Successfully Obtained, Print to Serial and LCD
    if (isnan(temperature) || isnan(humidity)) {
      // Send Error Messages
      lcd.clear();
      lcd.print("Sensor Error!");
      Serial.println("Failed to read from DHT sensor!");
    } else {
      // Display the Data from the Sensor
      displayData();
    }
  }

  // Wait 60 Seconds Before Next Reading
  delay(60000);
}
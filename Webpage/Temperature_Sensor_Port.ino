#include <DHT.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID "TelstraDBFC51"
#define WIFI_PASSWORD "fqwt6djgzc"
#define DHTPIN D7
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start server and sensor
  server.begin();
  dht.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");

    String request = client.readStringUntil('\r');
    Serial.println("Client request: " + request);  // Debugging request info
    client.flush();

    // Handle CORS preflight request
    if (request.indexOf("OPTIONS") >= 0) {
      client.println("HTTP/1.1 204 No Content");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
      client.println("Access-Control-Allow-Headers: Content-Type");
      client.println("Connection: close");
      client.println();
      return;
    }

    // Read sensor data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Access-Control-Allow-Origin: *");
      client.println("Connection: close");
      client.println();
      client.println("Failed to read from DHT sensor!");
      return;
    }

    // Send sensor data in JSON format
    String jsonResponse = String("{\"temperature\":") + String(temperature, 2) +
                          String(",\"humidity\":") + String(humidity, 2) +
                          String("}");

    Serial.println("JSON Response: " + jsonResponse);  // Debugging JSON output

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    client.println(jsonResponse);
  }
}
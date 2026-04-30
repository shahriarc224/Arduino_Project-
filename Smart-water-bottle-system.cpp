// Smart Water Bottle 

#include <WiFi.h>

#define DISABLE_FIRESTORE
#define DISABLE_FCM
#define DISABLE_FB_STORAGE
#define DISABLE_GC_STORAGE
#define DISABLE_FB_FUNCTIONS
#define DISABLE_ERROR_QUEUE


#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiClientSecure.h>
#include <time.h>

// WiFi
#define WIFI_SSID "Loading......"
#define WIFI_PASSWORD "19111921911192"


#define FIREBASE_API_KEY "AIzaSyAmHKNHe2aLB0qXVVFR4_r-5sYB7vc6ABo"
#define FIREBASE_DB_URL "smartwaterbottle-a4dd9-default-rtdb.asia-southeast1.firebasedatabase.app"

#define USER_EMAIL "shahriarc224@gmail.com"
#define USER_PASSWORD "#@Workhard9966AMI"


#define ONE_WIRE_BUS 4
#define WATER_LEVEL_SENSOR 34
#define TURBIDITY_SENSOR 35
#define BLUE_LED 22
#define RED_LED 23

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


FirebaseData fbdo_write;
FirebaseData fbdo_led;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long previousMillis = 0;
const long interval = 5000;


void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Syncing time");
  time_t now = time(nullptr);
  int retry = 0;

  while (now < 1670000000 && retry < 30) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
    retry++;
  }

  if (now < 1670000000) {
    Serial.println("\n❌ Time sync FAILED");
  } else {
    Serial.println("\n✅ Time synced");
  }
}


void testSSL() {
  WiFiClientSecure client;
  client.setInsecure();

  Serial.print("Testing SSL...");
  if (!client.connect("google.com", 443)) {
    Serial.println("❌ SSL FAILED (network problem)");
  } else {
    Serial.println("✅ SSL OK");
    client.stop();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(RED_LED, LOW);

  sensors.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(false);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nConnected!");
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  
  syncTime();


  testSSL();

  
  delay(2000);

  // 🔥 Firebase config
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DB_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.cert.data = NULL;
  config.cert.file = "";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // 🔧 Increase TLS buffers (VERY IMPORTANT)
  fbdo_write.setBSSLBufferSize(8192, 2048);

  Serial.print("Waiting for Firebase");
  unsigned long t = millis();

  while (!Firebase.ready()) {
    Serial.print(".");
    delay(500);

    if (millis() - t > 30000) {
      Serial.println("\nTimeout! Restarting...");
      ESP.restart();
    }
  }

  Serial.println("\n🔥 Firebase Ready!");
}

void loop() {

  // WiFi recovery
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Reconnecting...");
    WiFi.reconnect();
    delay(3000);
    return;
  }

  if (!Firebase.ready()) {
    Serial.println("Firebase not ready...");
    delay(1000);
    return;
  }

  // 📊 Send sensor data every 5 sec
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    if (temperature == -127.0 || temperature == 85.0) {
      temperature = 0.0;
    }

    int waterLevel = map(analogRead(WATER_LEVEL_SENSOR), 0, 4095, 0, 100);
    int turbidity  = map(analogRead(TURBIDITY_SENSOR), 0, 4095, 100, 0);

    bool isSafe = (temperature > 0 && temperature < 40 &&
                   waterLevel > 20 && turbidity > 50);

    FirebaseJson dataJson;
    dataJson.set("temperature", temperature);
    dataJson.set("waterLevel", waterLevel);
    dataJson.set("turbidity", turbidity);
    dataJson.set("isSafe", isSafe);

    Serial.println("--- Writing ---");

    if (Firebase.RTDB.setJSON(&fbdo_write, "/currentStatus", &dataJson)) {
      Serial.println("✅ Data sent");
    } else {
      Serial.print("❌ Error: ");
      Serial.println(fbdo_write.errorReason());
    }
  }
}

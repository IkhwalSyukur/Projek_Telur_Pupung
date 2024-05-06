
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#include <addons/TokenHelper.h>

#include <addons/RTDBHelper.h>

#define WIFI_SSID "Variasi Aluminium 1"
#define WIFI_PASSWORD "hapisahsyukur2"

#define API_KEY "AIzaSyDQPMI1VJUBlEXXUcOgCaMu1eT2rdmh2N8"

#define DATABASE_URL "https://projek-telur-default-rtdb.firebaseio.com"

#define USER_EMAIL "projektelurpupung@gmail.com"
#define USER_PASSWORD "Projektelur123"

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
int jumlah_telur_besar;
int jumlah_telur_kecil;
int total_telur;

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; 
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096,102);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);


}

void loop()
{

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    unsigned long timestamp = millis();

    // String dataJTB = String(jumlah_telur_besar);
    // String dataJTK = String(jumlah_telur_kecil);
    // String dataTotal = String(total_telur);

    String pathJTB = "/Monitoring/jumlah_telur_besar";
    String pathJTK = "/Monitoring/jumlah_telur_kecil";
    String pathTotal = "/Monitoring/jumlah_total_telur";

    String pathHistoryJTB = "/History/" + String(timestamp) + "/jumlah_telur_besar";
    String pathHistoryTotal = "/History/" + String(timestamp) + "/Total_telur";

    Serial.printf("Set data at path %s: %s\n", pathJTB.c_str(), Firebase.setInt(fbdo, pathJTB.c_str(), jumlah_telur_besar) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set data at path %s: %s\n", pathJTK.c_str(), Firebase.setInt(fbdo, pathJTK.c_str(), jumlah_telur_kecil) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set data at path %s: %s\n", pathTotal.c_str(), Firebase.setInt(fbdo, pathTotal.c_str(), total_telur) ? "ok" : fbdo.errorReason().c_str());

    // Serial.printf("Set data at path %s: %s\n", pathHistoryJTB.c_str(), Firebase.setInt(fbdo, pathHistoryJTB.c_str(), jumlah_telur_besar) ? "ok" : fbdo.errorReason().c_str());
    // Serial.printf("Set data at path %s: %s\n", pathHistoryJTK.c_str(), Firebase.setInt(fbdo, pathHistoryJTK.c_str(), jumlah_telur_kecil) ? "ok" : fbdo.errorReason().c_str());
    // Serial.printf("Set data at path %s: %s\n", pathHistoryTotal.c_str(), Firebase.setInt(fbdo, pathHistoryTotal.c_str(), total_telur) ? "ok" : fbdo.errorReason().c_str());


    Serial.printf("Get int telur besar atas %s\n", Firebase.getInt(fbdo, F("/Ubah_ukuran_telur/telur_besar/nilai_atas")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get int telur besar bawah %s\n", Firebase.getInt(fbdo, F("/Ubah_ukuran_telur/telur_besar/nilai_bawah")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get int telur kecil atas %s\n", Firebase.getInt(fbdo, F("/Ubah_ukuran_telur/telur_kecil/nilai_atas")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get int telur kecil bawah %s\n", Firebase.getInt(fbdo, F("/Ubah_ukuran_telur/telur_kecil/nilai_bawah")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());

    // Serial.printf("Set string... %s\n", Firebase.setString(fbdo, F("/test/string"), "Hello World!") ? "ok" : fbdo.errorReason().c_str());

    // Serial.printf("Get string... %s\n", Firebase.getString(fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());
    total_telur++;
    Serial.println();

    count++;
  }
}
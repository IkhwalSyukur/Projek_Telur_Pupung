#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Variasi Aluminium 1"
#define WIFI_PASSWORD "hapisahsyukur2"

#define API_KEY "AIzaSyDQPMI1VJUBlEXXUcOgCaMu1eT2rdmh2N8"

#define DATABASE_URL "https://projek-telur-default-rtdb.firebaseio.com"

#define USER_EMAIL "projektelurpupung@gmail.com"
#define USER_PASSWORD "Projektelur123"

#define CALIBRATION_FACTOR (-19787 / 50)

Servo myservo;  // Membuat objek servo
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int sensorPin1 = 33;  // Pin untuk sensor inframerah pertama
const int sensorPin2 = 32;  // Pin untuk sensor inframerah kedua

int count1 = 0;  // Inisialisasi variabel count1
int count2 = 0;  // Inisialisasi variabel count2

const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 19;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;
int jumlah_telur_besar;
int jumlah_telur_kecil;
int total_telur;

HX711 scale;
int reading;
int lastReading;

void displayWeight(int weight) {
  // Baca status sensor

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Berat:");
  lcd.setCursor(6, 0);
  lcd.print(weight);
  lcd.print(" gr");

  if (weight > 5 && weight <= 55) {  //55 diambil dari database bisa dirubah secara otomatis lewat aplikasi
    lcd.setCursor(0, 1);
    lcd.print("Status Kecil");
  } else if (weight > 55) {
    lcd.setCursor(0, 1);
    lcd.print("Status Besar");
  }
}

void controlServo(int weight) {
  // Membuat servo bergerak sesuai dengan kondisi berat
  if (weight >= 5 && weight <= 55) {
    // Bergerak ke kiri (posisi awal)
    myservo.write(0);
    delay(1000);
    myservo.write(90);
  } else if (weight > 55) {
    // Bergerak ke kanan (posisi maksimum)
    myservo.write(180);
    delay(1000);
    myservo.write(90);
  } else {
    // Berat kurang dari 5 gram, servo tidak bergerak (diam)
    // Tidak perlu menuliskan instruksi servo di sini
  }
}


void setup(){
    Serial.begin(115200);
    pinMode(sensorPin1, INPUT);  // Mengatur pin sensor 1 sebagai input
    pinMode(sensorPin2, INPUT);  // Mengatur pin sensor 2 sebagai input

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
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);

    // Inisialisasi pin sensor sebagai input
    pinMode(sensorPin1, INPUT);
    pinMode(sensorPin2, INPUT);

    // Initializing LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing ...");
    delay(1000);

      // Initializing the scale
    Serial.println("Initializing the scale");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(CALIBRATION_FACTOR);
    scale.tare();  // reset the scale to 0

    // Initializing servo
    myservo.attach(12);  // Menginisialisasi servoMotor pada pin 12 (GPIO 12)

}

void loop(){

    // Baca status sensor infrared
    int sensorState1 = digitalRead(sensorPin1);
    int sensorState2 = digitalRead(sensorPin2);

    // Periksa apakah ada perubahan status sensor
    static int lastSensorState1 = LOW;
    static int lastSensorState2 = LOW;

    // Modifikasi kode untuk menghitung dari 0
    if (sensorState1 != lastSensorState1) {
        // Jika sensor 1 mendeteksi benda
        if (sensorState1 == HIGH) {
        count1++;
        Serial.print("Jumlah Telur Kecil: ");
        Serial.println(count1);
        }
        lastSensorState1 = sensorState1;
    }

    if (sensorState2 != lastSensorState2) {
        // Jika sensor 2 mendeteksi benda
        if (sensorState2 == HIGH) {
        count2++;
        Serial.print("Jumlah Telur Besar: ");
        Serial.println(count2);
        }
        lastSensorState2 = sensorState2;
    }

    // Periksa kondisi awal
    static bool resetCount = true;
    if (resetCount) {
        count1 = 0;  // Mulai hitungan dari 0
        count2 = 0;  // Mulai hitungan dari 0
        resetCount = false;
    }
    int totaltelur = count1 + count2;
    Serial.print("Total Telur: ");
    Serial.println(totaltelur);

    if (scale.wait_ready_timeout(200)) {
        reading = round(scale.get_units());
        Serial.print("Weight: ");
        Serial.println(reading);
        if (reading < 0) {
        reading = 0;
        }
        displayWeight(reading);
        controlServo(reading);
    } else {
        Serial.println("HX711 not found.");
    }

    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        String path = "/Monitoring/parameter";

        String _data = "{"
            "\"Telur_Kecil\": " + String(count1) + "," +
            "\"Telur_Besar\": " + String(count2) + "," +
            "\"Jumlah_Telur\": " + String(totaltelur) +
            "}";
        Serial.printf("Set string... %s\n", Firebase.setString(fbdo, path, _data) ? "ok" : fbdo.errorReason().c_str());

    }
}


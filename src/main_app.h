#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "config.h"
#include "fbhandler.h"

#define WIFI_SSID "Variasi Aluminium 1"
#define WIFI_PASSWORD "hapisahsyukur2"

#define CALIBRATION_FACTOR (-19787 / 50)

#define debug

FbHandler myfb(CONFIG_MAIN_API_KEY, CONFIG_MAIN_DATABASE_URL, CONFIG_MAIN_USER_EMAIL, CONFIG_MAIN_USER_PASSWORD);

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
int total_telur;
int last_count1;
int last_count2;

HX711 scale;
int reading;
int lastReading;
int batas_telur;

void streamCallback(StreamData data)
{
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!fbdo.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", fbdo.httpCode(), fbdo.errorReason().c_str());
}

void displayWeight(int weight);
void controlServo(int weight);
void checkchange();

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
    // Serial.println("Initializing the scale");
    // scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    // scale.set_scale(CALIBRATION_FACTOR);
    // scale.tare();  // reset the scale to 0

    // Initializing servo
    myservo.attach(12);  // Menginisialisasi servoMotor pada pin 12 (GPIO 12)

    myfb.begin();
    fbdo.keepAlive(5, 5, 1);
    if (!Firebase.beginStream(fbdo, "Ubah_Ukuran_Telur"))
    Serial.printf("stream begin error, %s\n\n", fbdo.errorReason().c_str());

  Firebase.setStreamCallback(fbdo, streamCallback, streamTimeoutCallback);
}

void loop(){

#ifndef debug
    // Baca status sensor infrared
    int sensorState1 = digitalRead(sensorPin1);
    int sensorState2 = digitalRead(sensorPin2);

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

#else
    bool sensorState1 = false;
    bool sensorState2 = false;
    if (Serial.available() > 0) {
    char command = Serial.read(); // Read the incoming byte
    
    // Check the command received from the Serial Monitor
    if (command == 'y') {
      sensorState1 = true; // Set sensorState1 to true
      Serial.println("Sensor state set to true.");
    } else if (command == 'g') {
      sensorState1 = false; // Set sensorState1 to false
      Serial.println("Sensor state set to false.");
    }
    else if (command == 't') {
      sensorState2 = true; // Set sensorState1 to true
      Serial.println("Sensor state set to true.");
    } else if (command == 'f') {
      sensorState2 = false; // Set sensorState1 to false
      Serial.println("Sensor state set to false.");
    }
     else {
      Serial.println("Invalid command. Please enter 't' for true or 'f' for false.");
    }
  }

  // myfb.getData(batas_telur);

#endif

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
        checkchange();
        }
        lastSensorState1 = sensorState1;
        last_count1 = count1;
    }

    if (sensorState2 != lastSensorState2) {
        // Jika sensor 2 mendeteksi benda
        if (sensorState2 == HIGH) {
        count2++;
        Serial.print("Jumlah Telur Besar: ");
        Serial.println(count2);
        checkchange();
        }
        lastSensorState2 = sensorState2;
        last_count2 = count2;
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

  

    DataTelur_t telurParam;

    telurParam.telur_kecil = count1;
    telurParam.telur_besar = count2;
    telurParam.jumlah_telur = totaltelur;

    myfb.JSONFormater(telurParam);

    delay(500);
}

void checkchange(){
    if(last_count1 != count1 || last_count2 != count2){
        bool thisstate = true;
        myfb.isDataChange(thisstate);
    }
    else{
      bool thisstate = false;
      myfb.isDataChange(thisstate);
    }
}

void displayWeight(int weight) {
  // Baca status sensor

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Berat:");
  lcd.setCursor(6, 0);
  lcd.print(weight);
  lcd.print(" gr");

  if (weight < batas_telur) {  //55 diambil dari database bisa dirubah secara otomatis lewat aplikasi
    lcd.setCursor(0, 1);
    lcd.print("Status Kecil");
  } else if (weight > batas_telur) {
    lcd.setCursor(0, 1);
    lcd.print("Status Besar");
  }
}

void controlServo(int weight) {
  // Membuat servo bergerak sesuai dengan kondisi berat
  if ( weight < batas_telur) {
    // Bergerak ke kiri (posisi awal)
    myservo.write(0);
    delay(1000);
    myservo.write(90);
  } else if (weight > batas_telur) {
    // Bergerak ke kanan (posisi maksimum)
    myservo.write(180);
    delay(1000);
    myservo.write(90);
  } else {
    // Berat kurang dari 5 gram, servo tidak bergerak (diam)
    // Tidak perlu menuliskan instruksi servo di sini
  }
}


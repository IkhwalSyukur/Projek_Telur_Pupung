// #define UNIT_TEST

#ifdef UNIT_TEST

#include "TES/firebase_manual.h"

#else
#include <Arduino.h>
#include "fbhandler.h"
#include "config.h"
#include <WiFi.h>

#define WIFI_SSID "Variasi Aluminium 1"
#define WIFI_PASSWORD "hapisahsyukur2"

int Telur_Kecil_Value;
int Telur_Besar_Value;
int Jumlah_Telur_Value;

FbHandler myfb(CONFIG_MAIN_API_KEY, CONFIG_MAIN_DATABASE_URL, CONFIG_MAIN_USER_EMAIL, CONFIG_MAIN_USER_PASSWORD);

void fb_task (void *pvParameter);

void setup(){

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

    myfb.begin();
    xTaskCreate(fb_task, "Firebase Task Handler", 1024*1, NULL, 1, NULL);
}

void loop(){

}


void fb_task(void *pvParameter){
    DataTelur_t data;
    while(1){
        data.telur_kecil=Telur_Kecil_Value;
        data.telur_besar=Telur_Besar_Value;
        data.jumlah_telur=Jumlah_Telur_Value;

        char buffer[512];
        sprintf(buffer,
            "{"
            "\"Telur_Kecil\": %d,"
            "\"Telur_Besar\": %d,"
            "\"Jumlah_Telur\": %d"
            "}",
            data.telur_kecil, data.telur_besar,
            data.jumlah_telur);

        myfb.setData(buffer);

        Telur_Kecil_Value++;
        Telur_Besar_Value++;
        Jumlah_Telur_Value++;

        vTaskDelay(2000);
    }
}

#endif


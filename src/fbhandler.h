#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

typedef struct
{
    int telur_kecil;
    int telur_besar;
    int jumlah_telur;

} DataTelur_t;


class FbHandler
{
    public:
        FbHandler(String api_key,String database_url,String user_email,String user_password);
        bool begin();
        String JSONFormater(DataTelur_t data);


    private:
        bool setData(String msg);
        bool getData(int &data);

        void task_func();
        static void static_task_func(void *pvParameter);

        String _api_key;
        String _database_url;
        String _user_email;
        String _user_password;
        FirebaseData _fbdo;
        FirebaseAuth _auth;
        FirebaseConfig _config;
        String _prefix_monitoring = "/Monitoring/parameter";
        String _prefix_get_data = "/Ubah_ukuran_telur";

};
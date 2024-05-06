#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// void fb_task (void *pvParameter);
typedef struct
{
    int telur_kecil;
    int telur_besar;
    int jumlah_telur;

} DataTelur_t;

class FbHandler
{
public:
    FbHandler(String api_key, String database_url, String user_email, String user_password);
    bool begin(int interval = 2000);
    bool JSONFormater(DataTelur_t data);
    bool setData(String msg);
    bool getData(int &data);

private:
    void task_func();
    static void static_task_func(void *pvParam);
    String _bufferMsg = "";
    int _interval;

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

FbHandler::FbHandler(String api_key, String database_url, String user_email, String user_password)
    : _api_key(api_key), _database_url(database_url), _user_email(user_email), _user_password(user_password)
{
}

bool FbHandler::begin(int interval)
{
    _interval = interval;
    _config.api_key = _api_key;
    _auth.user.email = _user_email;
    _auth.user.password = _user_password;
    _config.database_url = _database_url;
    _config.token_status_callback = tokenStatusCallback;
    Firebase.reconnectNetwork(true);
    _fbdo.setBSSLBufferSize(4096, 1024);
    Firebase.begin(&_config, &_auth);
    Firebase.setDoubleDigits(5);

    xTaskCreate(&FbHandler::static_task_func, "Firebase Handler", 1024 * 8, this, 1, NULL);
    return true;
}

bool FbHandler::JSONFormater(DataTelur_t data)
{
    char buffer[512];
    sprintf(buffer,
            "{"
            "Telur_Kecil: %d,"
            "Telur_Besar: %d,"
            "Jumlah_Telur: %d"
            "}",
            data.telur_kecil, data.telur_besar,
            data.jumlah_telur);

    _bufferMsg = String(buffer);
    return true;
}

bool FbHandler::setData(String msg)
{

    bool _setdata = Firebase.setString(_fbdo, _prefix_monitoring.c_str(), msg.c_str());
    // bool _setdata = Firebase.setString(_fbdo, _prefix_monitoring.c_str(), "halooou");

    if (!_setdata)
    {
        Serial.printf("Send data status : %s\n", _fbdo.errorReason().c_str());
        return false;
    }
    else
    {
        Serial.println("Send data success");
    }

    return _setdata;
}

bool FbHandler::getData(int &data)
{
    bool _getData = Firebase.getInt(_fbdo, F(_prefix_get_data.c_str()));

    if (!_getData)
    {
        Serial.printf("Get data status : %s\n", _fbdo.errorReason().c_str());
        return false;
    }
    else
    {
        data = _fbdo.to<int>();
        Serial.printf("Get data : %d\n", data);
    }

    return _getData;
}

/*STATIC*/ void FbHandler::static_task_func(void *pvParam)
{
    FbHandler *handler = reinterpret_cast<FbHandler *>(pvParam);
    handler->task_func();
}

void FbHandler::task_func()
{
    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Waiting for wifi connection");

            while (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("Wifi not connected !");
                delay(1000);
            }

            Serial.printf("Connected with IP: %s\n", String(WiFi.localIP()).c_str());
        }
        else if (WiFi.status() != WL_CONNECTED && !Firebase.ready())
        {
            Firebase.reconnectNetwork(true);
        }
        else
        {
            this->setData(_bufferMsg);
            Serial.printf("Sending msg buffer: %s\n", _bufferMsg.c_str());
        }
        vTaskDelay(_interval);
    }
    vTaskDelete(NULL);
}

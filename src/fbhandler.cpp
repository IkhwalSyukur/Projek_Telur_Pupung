#include "fbhandler.h"

FbHandler::FbHandler(String api_key,String database_url,String user_email,String user_password)
: _api_key(api_key), _database_url(database_url), _user_email(user_email), _user_password(user_password)
{
    
}

bool FbHandler::begin()
{
     _config.api_key = _api_key;
     _auth.user.email = _user_email;
     _auth.user.password = _user_password;
     _config.database_url = _database_url;
     _config.token_status_callback = tokenStatusCallback;
     Firebase.reconnectNetwork(true);
     _fbdo.setBSSLBufferSize(4096, 1024);
     Firebase.begin(&_config, &_auth);
     Firebase.setDoubleDigits(5);


}

String FbHandler::JSONFormater(DataTelur_t data)
{
    char buffer[512];
    sprintf(buffer,
            "{"
            "\"Telur_Kecil\": %d,"
            "\"Telur_Besar\": %d,"
            "\"Jumlah_Telur\": %d"
            "}",
            data.telur_kecil, data.telur_besar,
            data.jumlah_telur);

    return String(buffer);
}

bool FbHandler::setData(String msg)
{


    bool _setdata = Firebase.setString(_fbdo, _prefix_monitoring.c_str(), msg.c_str());
    
    if (!_setdata)
    {
        Serial.printf("Send data status : %s\n", _fbdo.errorReason().c_str());
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
        Serial.printf("Get data status : %s\n" , _fbdo.errorReason().c_str());
        return false;
    }
    else
    {
        data = _fbdo.to<int>();
        Serial.printf("Get data : %d\n",data);
    }

    return _getData;
}
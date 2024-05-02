// #define UNIT_TEST

#ifdef UNIT_TEST

#include "TES/firebase_manual.h"

#else
#include <Arduino.h>
#include "fbhandler.h"
#include "config.h"

FbHandler myfb(CONFIG_MAIN_API_KEY, CONFIG_MAIN_DATABASE_URL, CONFIG_MAIN_USER_EMAIL, CONFIG_MAIN_USER_PASSWORD);

void setup(){
    myfb.begin();
}

void loop(){

}


#endif


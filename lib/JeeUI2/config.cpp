#include "JeeUI2.h"

void jeeui2::save(const char *_cfg)
{
    if(SPIFFS.begin()){
        File configFile;
        if(_cfg == nullptr)
            configFile = SPIFFS.open(F("/config.json"), "w"); // PSTR("w") использовать нельзя, будет исключение!
        else
            configFile = SPIFFS.open(_cfg, "w"); // PSTR("w") использовать нельзя, будет исключение!

        String cfg_str;
        serializeJson(cfg, cfg_str);
        configFile.print(cfg_str);
        configFile.flush();
        configFile.close();
        cfg.garbageCollect();

        LOG(println, F("Save Config"));
    }
}

void jeeui2::autosave(){
    if (isConfSaved) return;
    if (!isConfSaved && astimer + asave < millis()){
        save();
        LOG(println, F("AutoSave"));
        astimer = millis();
        isConfSaved = true; // сохранились
        //sv = false;
        //mqtt_update();
    }
}

void jeeui2::pre_autosave(){
    if (!sv) return;
    if (sv && astimer + 1000 < millis()){
        LOG(println, F("pre_autosave"));
        fcallback_update();
        mqtt_update();
        sv = false;
        isConfSaved = false;
        astimer = millis(); // обновляем счетчик после последнего изменения UI
    }
}

jeeui2::updateCallback jeeui2::updateCallbackHndl(){
    return fcallback_update;
}

void jeeui2::updateCallbackHndl(updateCallback func){
    fcallback_update = func;
}

jeeui2::httpCallback jeeui2::httpCallbackHndl(){
    return fcallback_http;
}

void jeeui2::httpCallbackHndl(httpCallback func){
    fcallback_http = func;
}


void jeeui2::as(){
    sv = true;
    astimer = millis();
}

void jeeui2::load(const char *_cfg)
{
    if(SPIFFS.begin()){
        File configFile;
        if(_cfg == nullptr)
            configFile = SPIFFS.open(F("/config.json"), "r"); // PSTR("r") использовать нельзя, будет исключение!
        else
            configFile = SPIFFS.open(_cfg, "w"); // PSTR("w") использовать нельзя, будет исключение!

        String cfg_str = configFile.readString();
        if (cfg_str == F("")){
            LOG(println, F("Failed to open config file"));
            save();
            return;
        }
        DeserializationError error = deserializeJson(cfg, cfg_str);
        if (error) {
            LOG(print, F("JSON config deserializeJson error: "));
            LOG(println, error.code());
            return;
        }
        LOG(println, F("JSON config loaded"));
        configFile.close();
        sv = false;
    }
}

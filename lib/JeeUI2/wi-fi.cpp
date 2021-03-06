
bool _wifi_connected = false;
#include "JeeUI2.h"


#ifdef ESP8266
void onSTAConnected(WiFiEventStationModeConnected ipInfo)
{
    Serial.printf_P(PSTR("Connected to %s\r\n"), ipInfo.ssid.c_str());
}

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
    Serial.printf_P(PSTR("Got IP: %s\r\n"), ipInfo.ip.toString().c_str());
    Serial.printf_P(PSTR("Connected: %s\r\n"), WiFi.status() == WL_CONNECTED ? F("yes") : F("no"));
    _wifi_connected = true;
}

void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
    Serial.printf_P(PSTR("Disconnected from SSID: %s\n"), event_info.ssid.c_str());
    Serial.printf_P(PSTR("Reason: %d\n"), event_info.reason);
    _wifi_connected = false;
}
#else
void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_START:
        Serial.println(F("Station Mode Started"));
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("Connected to :" + String(WiFi.SSID()));
        Serial.print(F("Got IP: "));
        Serial.println(WiFi.localIP());
        _wifi_connected = true;
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println(F("Disconnected from station, attempting reconnection"));
        WiFi.reconnect();
        _wifi_connected = false;
        break;
    default:
        break;
    }
}
#endif

 void jeeui2::_connected(){
     connected = _wifi_connected;
 }

void jeeui2::wifi_connect()
{
    bool wf = false;
    String wifi = param(F("wifi"));
    if (wifi == F("AP")) wifi_mode = 1;
    if (wifi == F("STA")) wifi_mode = 2;
    switch (wifi_mode)
    {
        case 1:
        {
            WiFi.disconnect();
            WiFi.mode(WIFI_AP);
            WiFi.softAP(param(F("ap_ssid")).c_str(), param(F("ap_pass")).c_str(), 6, 0, 4);
            if(dbg)Serial.println(F("Start Wi-Fi AP mode!"));
            save();
            _wifi_connected = false;
            break;
        }
        case 2:
        {
#ifdef ESP8266
            static WiFiEventHandler e1, e2, e3;
            e1 = WiFi.onStationModeGotIP(onSTAGotIP);
            e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
            e3 = WiFi.onStationModeConnected(onSTAConnected);
#else
            WiFi.onEvent(WiFiEvent);
#endif
            WiFi.disconnect();
            WiFi.mode(WIFI_OFF);
            WiFi.mode(WIFI_STA);
            WiFi.begin(param(F("ssid")).c_str(), param(F("pass")).c_str());
            if(dbg)Serial.println(F("Connecting STA"));
            save();

            unsigned long interval = millis();
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(1);
                if (LED_PIN != -1) {
                    delay(50);
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
                }
                else
                {
                    delay(500);
                }
                
                
                btn();
                if(a_ap + interval < millis() && a_ap != 0){
                    wf = true;
                    break;
                }
            }

            uint8_t macaddr[6];
            WiFi.macAddress(macaddr);
            String _mac;
            for (byte i = 0; i < 6; ++i) {
                _mac += String(macaddr[i], HEX);
                if (i < 5)
                    _mac += ':';
            }
            strncpy(mac,_mac.c_str(),sizeof(mac)-1);
            strncpy(ip,WiFi.localIP().toString().c_str(),sizeof(ip)-1);
            if(dbg)Serial.println();
            break;
        }
    }
    if (wf){
        if(dbg)Serial.println(F("RECONNECT AP"));
        var(F("wifi"), F("AP"));
        wifi_connect();
    } 
    WiFi.scanNetworks(true);
}

void jeeui2::ap(unsigned long interval){
    a_ap = interval;
}
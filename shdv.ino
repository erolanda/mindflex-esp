#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <WebSocketsClient.h>
#include <SoftwareSerial.h>
#include <Brain.h>
#include <vector>

//OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define DEBUG_ON
WebSocketsClient webSocket;
const char* ssid     = "sensor";
const char* password = "1234567278";
const char* ws_server = "192.168.0.100";
int ws_port = 3000;

// Set up the software serial port on pins 10 (RX) and 11 (TX). We'll only actually hook up pin 10.
SoftwareSerial softSerial(5, 11);

// Set up the brain reader, pass it the software serial object you want to listen on.
Brain brain(softSerial);

const int ledPin = 2;
long interval = 500; // Changes based on attention value.
long previousMillis = 0;
int ledState = LOW;

//flag for sending data
bool ban = false;

//struct SensorData {
//        int16_t strength;
//        int16_t attention;
//        int16_t meditation;
//        int16_t delta;
//        int16_t theta;
//        int16_t lowAlpha;
//        int16_t highAlpha;
//        int16_t lowBeta;
//        int16_t highBeta;
//        int16_t lowGamma;
//        int16_t highGamma;
//};

struct SensorData {
        int16_t strength;
        int16_t attention;
        int16_t meditation;
};

const int BUFFER_SIZE = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(80) + 200;
uint8_t nLect = 1;

static std::vector<struct SensorData> vData(nLect);
int counter = 0;

void setup() {
        // put your setup code here, to run once:
        Serial.begin(115200);
        Serial.println();

        // Start the software serial.
        softSerial.begin(9600);

        //connect to WiFi
        setupWiFi();

        setupOTA();

        //Configure ws connection
        webSocket.begin(ws_server, ws_port);
        webSocket.onEvent(webSocketEvent);

        vData.clear();

        pinMode(ledPin, OUTPUT);
}

void setupOTA(){
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setupWiFi(){
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
        #ifdef DEBUG_ON
                Serial.print(".");
        #endif
        }
  #ifdef DEBUG_ON
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
  #endif
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
        switch(type) {
        case WStype_DISCONNECTED: {
      #ifdef DEBUG_ON
                Serial.println("[WSc] Disconnected!");
      #endif
        }
        break;
        case WStype_CONNECTED: {
      #ifdef DEBUG_ON
                Serial.println("[WSc] Connected");
      #endif
        }
        break;
        case WStype_TEXT: {
                String message = (char * )payload;
      #ifdef DEBUG_ON
                Serial.println(message);
      #endif
                if(message == "startPreview")
                        ban = true;
                else if(message == "stopPreview")
                        ban = false;
        }
        break;
        }
}

String serialize(){
        StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["strength"] = vData[0].strength;
        root["attention"] = vData[0].attention;
        root["meditation"] = vData[0].meditation;
        String JSON;
        root.printTo(JSON);
        return JSON;
}

void sendData(){
        String json = serialize();
        webSocket.sendTXT(json);
        //Serial.println(json);
        vData.clear();
}

void loop() {
        // put your main code here, to run repeatedly:
        ArduinoOTA.handle();
        webSocket.loop();
        if (brain.update()) {
          int16_t atencion = 0;
          Serial.println(brain.readErrors());
          Serial.println(brain.readCSV());
          atencion = brain.readAttention();
          SensorData data = {brain.readSignalQuality(), atencion, brain.readMeditation()};
          vData.push_back(data);
          sendData();
          // Attention runs from 0 to 100.
          interval = (100 - atencion) * 10;
          if (millis() - previousMillis > interval) {
            previousMillis = millis();
            if(ledState == LOW)
              ledState = HIGH;
            else
              ledState = LOW;
            digitalWrite(ledPin,ledState);
          }
        }
}

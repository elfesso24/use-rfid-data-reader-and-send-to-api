#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>    
#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = D3;     
constexpr uint8_t SS_PIN = D4;      

MFRC522 rfid(SS_PIN, RST_PIN);      
MFRC522::MIFARE_Key key;

String tag;

const char* ssid = "LTT4G_MF";
const char* password = "MohammedFurrara";

const char* serverUrl = "https://192.168.1.20:7029/Machine";
const int boardno = 1;

WiFiClientSecure client; 

void setup() {
    Serial.begin(9600);
    SPI.begin(); 
    rfid.PCD_Init(); 
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
     client.setInsecure();
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent())
        return;
    if (rfid.PICC_ReadCardSerial()) {
        for (byte i = 0; i < 4; i++) {
            tag += rfid.uid.uidByte[i];
        }
        Serial.println(tag);
        
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            String getUrl = String(serverUrl) + "?debtor_nfc=" + tag + "&Creditor=" + boardno;
            http.begin(client, getUrl);
            
            int httpResponseCode = http.GET();  
            
            if (httpResponseCode > 0) {
                Serial.println("Tag sent successfully");
            } else {
                Serial.print("Error on sending tag: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        } else {
            Serial.println("WiFi Disconnected");
        }
        
        tag = "";
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }
}

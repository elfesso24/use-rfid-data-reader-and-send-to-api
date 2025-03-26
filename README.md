# RFID to HTTP GET Request Documentation

## 1. Overview

This project uses an **ESP8266 NodeMCU** board and an **MFRC522 RFID** module to read RFID tags and then send the scanned tag information to a web server via an HTTP GET request. This can be useful for creating an access control system, attendance tracking, or any application where RFID tag information needs to be recorded remotely.

### Key Components

1. **NodeMCU (ESP8266)**
   - A microcontroller board with built-in Wi-Fi.
   - Official documentation: [NodeMCU Documentation](https://www.nodemcu.com/index_en.html)

2. **MFRC522 RFID Module**
   - An RFID reader/writer that typically operates at 13.56 MHz.
   - Commonly used to read and write data on RFID tags.
   - Library reference: [MFRC522 Library by miguelbalboa on GitHub](https://github.com/miguelbalboa/rfid)

### References & Sources

- [Arduino Reference](https://www.arduino.cc/reference/en/) – Official Arduino language reference  
- [ESP8266/NodeMCU Documentation](https://www.espressif.com/en/products/socs/esp8266) – Official ESP8266 documentation from Espressif  
- [MFRC522 Library GitHub](https://github.com/miguelbalboa/rfid) – Library used to interface with the RFID module  

---

## 2. Hardware Requirements

1. **ESP8266 NodeMCU Board**  
2. **MFRC522 RFID Module**  
3. **Jumper Wires** (male-to-male)  
4. **USB Cable** (for programming the NodeMCU)  
5. **Power Supply** (the NodeMCU can be powered via USB, which typically also powers the RFID module)

---

## 3. Wiring Diagram

Below is the mapping between the **NodeMCU** pins and the **MFRC522** pins as shown in the provided diagram (`wireing.png`). Ensure the pins are correctly aligned. The exact pin labels can vary depending on the NodeMCU board version, so always confirm your board’s pin labels.

| MFRC522 Pin | NodeMCU Pin | Notes                                                                 |
|-------------|-------------|-----------------------------------------------------------------------|
| **SDA** (SS)  | D8          | This acts as the chip select (CS) pin for the MFRC522.                |
| **SCK**       | D5          | SPI clock pin on the NodeMCU.                                         |
| **MOSI**      | D7          | SPI MOSI pin on the NodeMCU.                                          |
| **MISO**      | D6          | SPI MISO pin on the NodeMCU.                                          |
| **IRQ**       | Not used    | Not required for basic functionality.                                 |
| **GND**       | GND         | Common ground.                                                        |
| **RST**       | D3          | Reset pin for the MFRC522 (often mapped to a GPIO pin like D3).       |
| **3.3V**      | 3.3V        | Power input for the MFRC522 module. **Do not** power at 5V.           |

> **Note**: The MFRC522 module operates at 3.3V, and the NodeMCU’s I/O pins are also 3.3V compatible. Double-check your board’s voltage levels to avoid damage.

### Additional Wiring References

- [MFRC522 & Arduino Tutorial by LastMinuteEngineers](https://lastminuteengineers.com/how-rfid-works-rc522-arduino-tutorial/)  
  *(Although this tutorial uses an Arduino Uno as an example, the pin mapping concept is similar for NodeMCU (ESP8266)).*

---

## 4. Software Setup

1. **Arduino IDE**  
   - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).  
   - Install the **ESP8266 Board Package**: In the Arduino IDE, go to **File > Preferences**, add the appropriate URL in **Additional Board Manager URLs**, then go to **Tools > Board > Board Manager** and install “esp8266 by ESP8266 Community”.

2. **MFRC522 Library**  
   - Install the MFRC522 library from the Arduino Library Manager:  
     **Sketch > Include Library > Manage Libraries...** and search for “MFRC522”.  
   - Or manually from GitHub: [https://github.com/miguelbalboa/rfid](https://github.com/miguelbalboa/rfid)

---

## 5. Code Explanation

Below is a breakdown of the `send get request.ino` file. The main functionality is:

1. **Connect to Wi-Fi** using the provided SSID and password.  
2. **Initialize the MFRC522** module.  
3. **Continuously check for RFID tags** and read their Unique ID (UID).  
4. **Send an HTTP GET request** to a specified server with the UID as a parameter.

### Pseudocode Overview

```cpp
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

```
#### Key Functions
1. **WiFi.begin(ssid, password)**
    - Connects the ESP8266 to the specified Wi-Fi network.
    - Arduino WiFi Reference
2. **mfrc522.PICC_IsNewCardPresent()** and **mfrc522.PICC_ReadCardSerial()**
    - Checks if there is a new card/tag present and reads its data.
    - [﻿MFRC522 Library Reference](https://github.com/miguelbalboa/rfid) 
3. **client.connect(server, 80)**
    - Initiates a connection to the server on port 80 (HTTP).
4. **client.print()**
    - Sends an HTTP GET request to the server.
5. **mfrc522.PICC_HaltA()**
    - Halts the currently active RFID tag so the module can detect the next one.
---

## 6. Testing and Troubleshooting
1. **Check Serial Monitor**
    - Use the Arduino IDE’s Serial Monitor (set to `115200 baud` ) to see connection messages and any potential error logs.
2. **Verify Wiring**
    - Double-check all pins as per the table in [﻿Section 3](https://#3-wiring-diagram) .
    - Make sure the MFRC522 is powered at **3.3V**.
3. **Check Wi-Fi Credentials**
    - If the ESP8266 never connects, ensure SSID and password are correct and the router is within range.
4. **Server-Side Script**
    - Make sure the endpoint (`/script.php?uid=...` ) or the path you are using is correct and can handle the GET request.
    - You can verify by accessing `http://yourserver.com/script.php?uid=test`  in a web browser.
### Additional Troubleshooting Resources
- ESP8266 Troubleshooting Guide – Official Arduino Docs
- [﻿Common MFRC522 Issues on GitHub Issues Page](https://github.com/miguelbalboa/rfid/issues) 



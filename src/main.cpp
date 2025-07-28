#include <Arduino.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <MFRC522.h>

#define irSensorPin 15
#define servoPin 13
#define RST_PIN 22
#define SS_PIN 5
#define buzzer 4

#define Gate_Open 0
#define Gate_Closed 90
#define gateDelay 2000

Servo tollgate; 
MFRC522 rfid(SS_PIN, RST_PIN);

const byte authorizedCards[][4] = {
    {0x43, 0x94, 0x0F, 0x2A},
    {0x7D, 0x37, 0xAF, 0x02}
};

const int numCards = sizeof(authorizedCards) / sizeof(authorizedCards[0]);

bool gateOpen = false;
bool carDetected = false;
unsigned long beamClearTime = 0;
const unsigned long tailgateDelay = 2000;

bool compareUID(byte *uid, const byte *authorizedUID){
    for (byte i = 0; i < 4; i++){
        if (uid[i] !=authorizedUID[i]) return false; 
    }
    return true;
}

bool isAuthorized(byte *uid){
    for (int i = 0; i < numCards; i++){
        if (compareUID(uid, authorizedCards[i])) return true;
    }
    return false;
}

void setup(){
    Serial.begin(9600);
    pinMode(irSensorPin, INPUT);
    pinMode(buzzer, OUTPUT);
    tollgate.attach(servoPin);
    SPI.begin();
    rfid.PCD_Init();
    tollgate.write(Gate_Closed);
    Serial.print("System ready");
}
void loop(){
    bool beamBroken = digitalRead(irSensorPin) == LOW;

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()){
        return;
        if (isAuthorized(rfid.uid.uidByte)){
            tollgate.write(Gate_Open);
            gateOpen = true;
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

    }
    if (gateOpen && beamBroken){
        carDetected = true;
    }
    if (carDetected && !beamBroken){
        beamClearTime = millis();
        carDetected = false;
        gateOpen = false;
        if(!gateOpen && millis() - beamClearTime >= gateDelay){
        tollgate.write(Gate_Closed);
        beamClearTime = 0;
        }
    }
    if (!gateOpen && beamBroken){
        for(int i=0; i < 3; i++){
            digitalWrite(buzzer, HIGH);
            delay(1000);
            digitalWrite(buzzer, LOW);
            delay(1000);
        }
    }
    if (beamBroken && !gateOpen){
        tollgate.write(Gate_Open);
        delay (5000);
        tollgate.write(Gate_Closed);
    }
}
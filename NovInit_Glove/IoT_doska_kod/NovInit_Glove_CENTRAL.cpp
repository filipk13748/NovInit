#include <Wire.h>
#include <MPU6050.h>
#include <BLEPeripheral.h>

#define TCA9548A_ADDR 0x70  // I2C adresa multiplexeru TCA9548A
#define MPU6050_ADDR 0x68   // I2C adresa MPU6050 (predvolená)

// Pole pre uchovanie 5 objektov MPU6050
MPU6050 mpu[5];  

// BLE objekt pre perifériu
BLEPeripheral blePeripheral;

// Vytvorenie BLE služby
BLEService sensorService("180C");  

// Charakteristika na odosielanie dát o gyroskope cez BLE
BLECharacteristic gyroDataChar("2A1C", BLERead | BLENotify);

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Inicializácia I2C komunikácie
  
  // Inicializácia BLE
  blePeripheral.setLocalName("MPU6050 BLE");
  blePeripheral.setAdvertisedServiceUuid(sensorService.uuid());
  blePeripheral.addAttribute(sensorService);
  blePeripheral.addAttribute(gyroDataChar);
  
  blePeripheral.begin();  // Spustenie BLE periférie
  
  // Inicializácia senzorov MPU6050 a multiplexeru TCA9548A
  for (int i = 0; i < 5; i++) {
    selectTCAChannel(i);  // Výber I2C kanála na TCA9548A
    mpu[i].initialize();  // Inicializácia senzora MPU6050
    if (mpu[i].testConnection()) {  // Testovanie pripojenia k senzoru
      Serial.print("MPU6050 ");
      Serial.print(i);
      Serial.println(" úspešne inicializovaný.");
    } else {
      Serial.print("MPU6050 ");
      Serial.print(i);
      Serial.println(" zlyhala inicializácia.");
    }
  }
}

void loop() {
  blePeripheral.poll();  // Polling pre BLE perifériu (skontrolovanie pripojení)
  
  // Zbieranie dát z každého MPU6050 senzora
  String gyroData = "";
  for (int i = 0; i < 5; i++) {
    selectTCAChannel(i);  // Výber správneho kanála na TCA9548A
    
    // Čítanie gyroskopických dát zo senzora MPU6050
    int16_t gx, gy, gz;
    mpu[i].getRotation(&gx, &gy, &gz);  // Získanie hodnôt gyroskopu (X, Y, Z)
    
    gyroData += "Sensor " + String(i) + ": ";
    gyroData += "X=" + String(gx) + " Y=" + String(gy) + " Z=" + String(gz);
    if (i < 4) gyroData += " | ";  // Pridanie oddelovača medzi senzory
  }
  
  // Odoslanie gyroskopických dát cez BLE
  gyroDataChar.setValue(gyroData.c_str());
  gyroDataChar.notify();  // Oznámenie o nových dátach
  
  delay(1);  // 1ms pauza medzi odosielaním dát
}

// Funkcia na výber kanála na I2C multiplexeru TCA9548A
void selectTCAChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_ADDR);  // Začiatok I2C komunikácie
  Wire.write(1 << channel);  // Nastavenie bitu podľa vybraného kanála
  Wire.endTransmission();  // Ukončenie komunikácie
}

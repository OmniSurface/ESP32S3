#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

// put function declarations here:
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID "453184cc-3737-47be-ab4b-9a6991a92d6d"
#define CHARACTERISTIC_IO_UUID "bff7f0c9-5fbf-4b63-8d83-b8e077176fbe"

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

int count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);

  BLEDevice::init("OmniSurface"); // Give it a name
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // cannot write like BLECharacteristic *pCharacteristic1. Or later pCharacteristic1->setValue(combined.c_str()); will reboot the chip
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_IO_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("");
  pService->addCharacteristic(pCharacteristic);

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  if (deviceConnected) {
    int sensorValue = analogRead(A0);
    String valueString = String(sensorValue);
    pCharacteristic->setValue(valueString.c_str()); // Fix: Pass a const char* argument to setValue
    pCharacteristic->notify();
    Serial.print("Notify value ");
    Serial.println(sensorValue);
  }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    // delay(1000);
}
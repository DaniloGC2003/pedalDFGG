#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#include <FastLED.h>

//for on-board RGB LED control
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define PIN_LED 48
CRGB leds[1];

//for Arduino serial communication
#define RXp2 16
#define TXp2 17
#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8
#define UPDATE_ENCODER_MESSAGE_ID 11
#define UPDATE_ALL_ENCODERS_MESSAGE_ID 12

uint8_t buffer_rx[PAYLOAD_LEN];
uint8_t buffer_tx[HEADER_LEN + PAYLOAD_LEN];

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLE2901 *descriptor_2901 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t ble_message[PAYLOAD_LEN];
uint32_t value = 0;
int button_timer = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void clearArray(uint8_t* arr) {
  for (int i = 0; i < sizeof(arr); i++)
    arr[i] = 0;
}

void createSerialPayload(uint8_t* buffer, uint8_t message_id, uint8_t* message_bytes) {
  clearArray(buffer);
  int offset = 0;
  memcpy(buffer + offset, SERIAL_HEADER, HEADER_LEN);// Header
  offset += HEADER_LEN;
  memcpy(buffer + offset, &message_id, 1);// message id
  offset++;
  if (message_bytes != NULL)
    memcpy(buffer + offset, message_bytes, sizeof(message_bytes));// message
}

void printBufferBytes(uint8_t *buffer, size_t length) {
  Serial.println("---- Buffer Dump ----");
  for (size_t i = 0; i < length; i++) {
    Serial.print("Byte ");
    Serial.print(i);
    Serial.print(": 0x");
    if (buffer[i] < 0x10) Serial.print('0');  // leading zero for single-digit hex
    Serial.print(buffer[i], HEX);
    Serial.print(" (");
    Serial.print(buffer[i], DEC);
    Serial.println(")");
  }
  Serial.println("----------------------");
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("device connected. Requesting decoder values");

    // Request values
    clearArray(buffer_tx);
    createSerialPayload(buffer_tx, UPDATE_ALL_ENCODERS_MESSAGE_ID, NULL);
    Serial2.write(buffer_tx, sizeof(buffer_tx));
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("disconnecting");
  }
};

//callback function for BLE characteristic 2
class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override { 
    String pChar2_value_string = pChar->getValue();
    Serial.println("BLE packet received: ");
    for (int i = 0; i < pChar2_value_string.length(); i++) {
      Serial.print(uint8_t(pChar2_value_string[i]));
      Serial.print(" ");
    }
    Serial.println();

    //send serial data
    Serial.println("Sending serial data");
    int offset = 0;
    memcpy(buffer_tx + offset, SERIAL_HEADER, HEADER_LEN);
    offset += HEADER_LEN;
    int message_id = UPDATE_ENCODER_MESSAGE_ID;
    memcpy(buffer_tx + offset, &message_id, 1);
    offset ++;
    memcpy(buffer_tx + offset, pChar2_value_string.c_str(), pChar2_value_string.length());
    Serial2.write(buffer_tx, sizeof(buffer_tx));
    printBufferBytes(buffer_tx, sizeof(buffer_tx));
  }
};

void setup() {
  Serial.begin(115200);

  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  //for on-board RGB LED
  FastLED.addLeds<LED_TYPE, PIN_LED, COLOR_ORDER>(leds, 1);
  FastLED.setBrightness(255);
  leds[0] = CRGB::Black;   // equivalent to (0,0,0)
  FastLED.show();

  //button
  pinMode(0, INPUT_PULLUP);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
  );

  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pCharacteristic->addDescriptor(new BLE2902());
  // Adds also the Characteristic User Description - 0x2901 descriptor
  descriptor_2901 = new BLE2901();
  descriptor_2901->setDescription("My own description for this characteristic.");
  descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
  pCharacteristic->addDescriptor(descriptor_2901);
  pCharacteristic->setCallbacks(new CharacteristicCallBack());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  int i = 0;
  char c;
  bool match;
  //Serial.println("Message Received: ");
  //Serial.println(Serial2.readString());
  //Serial2.println("SUP");
  
  if (Serial2.available() >= HEADER_LEN + PAYLOAD_LEN){
    //Serial.println("Data in buffer");
    // Check header match
    c = Serial2.read();
    if (c == SERIAL_HEADER[0]){
      Serial.println("first char match");
      match = 1;
      for (i = 1; i < HEADER_LEN && match; i++){
        c = Serial2.read();
        if (c != SERIAL_HEADER[i]){
          Serial.println("Header mismatch");
          match = 0;
        }
      }
      
    }
    if (match){
      Serial.println("Header match.");
      Serial2.readBytes(buffer_rx, sizeof(buffer_rx));
      Serial.print("Message received through serial ports: ");
      printBufferBytes(buffer_rx, sizeof(buffer_rx));
      Serial.println();

      if (buffer_rx[0] == UPDATE_ENCODER_MESSAGE_ID) {
        clearArray(ble_message);
        ble_message[0] = UPDATE_ENCODER_MESSAGE_ID;
        ble_message[1] = buffer_rx[1];
        ble_message[2] = buffer_rx[2];
        pCharacteristic->setValue(ble_message, sizeof(ble_message));
        pCharacteristic->notify();
        Serial.println("sending BLE data");
      }
      else if (buffer_rx[0] == UPDATE_ALL_ENCODERS_MESSAGE_ID) {
        Serial.println("UPDATE_ALL_ENCODERS_MESSAGE_ID");
        clearArray(ble_message);
        ble_message[0] = UPDATE_ALL_ENCODERS_MESSAGE_ID;
        ble_message[1] = buffer_rx[1];
        ble_message[2] = buffer_rx[2];
        ble_message[3] = buffer_rx[3];
        ble_message[4] = buffer_rx[4];
        pCharacteristic->setValue(ble_message, sizeof(ble_message));
        pCharacteristic->notify();
      }
    }
  }
  
  if (digitalRead(0) == LOW && millis() - button_timer > 100) {
    button_timer = millis();
    Serial.println("pressed");

    // notify changed value
    if (deviceConnected) {
      for (int i = 0; i < 4; i++){
        ble_message[i] = value;
      }
      pCharacteristic->setValue(ble_message, sizeof(ble_message));
      //Serial.print("sending bytes: ");

      pCharacteristic->notify();
      Serial.println("sending data");
      leds[0] = CRGB::Green;
      FastLED.show();
      delay(100);
      leds[0] = CRGB::Black;   // equivalent to (0,0,0)
      FastLED.show();
      value = (value + 1) % 100;
    }
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimerFive.h>
#include <avr/io.h>
#define inputCLK 3
#define DT1 4
#define DT2 5
#define DT3 6
#define DT4 7

#define SW1 8
#define SW2 9
#define SW3 10
#define SW4 11

#define Up A0
#define Dw A1
#define BYP A2

#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8
#define UPDATE_ENCODER_MESSAGE_ID 11
#define UPDATE_ALL_ENCODERS_MESSAGE_ID 12

uint8_t buffer_tx[HEADER_LEN + PAYLOAD_LEN];
uint8_t buffer_rx[PAYLOAD_LEN];
uint8_t slider_value[2];
int offset = 0;
 
// Use volatile if you ever move to interrupts
volatile int counter[4] = {0, 0, 0, 0}; 
volatile int counter_[4] = {0, 0, 0, 0};

volatile int pwm_out[4] = {0, 0, 0, 0};

int eff_count=0;
int eff_layer=0;

int currentStateCLK;
int previousStateCLK; 


int currentStateUP;
int previousStateUP; 


int currentStateDW;
int previousStateDW; 

int eff[6] = {0, 0, 0, 0, 0, 0};
int effPin[6] = {32, 34, 36, 38, 40, 42};

unsigned long lastChangeTime = 0;
const int scrollSpeed = 500; // Delay in milliseconds

int pushButt[4] = {0, 0, 0, 0};
 

int pin_pwm_out[4] = {2, 44, 45, 46};
String encdir ="";
String label[4] = {"EC0", "EC1", "EC2", "EC3"};

void clearArray(uint8_t* arr, int len) {
  for (int i = 0; i < len; i++)
    arr[i] = 0;
}

void createSerialPayload(uint8_t* buffer, int buffer_size, uint8_t message_id, uint8_t* message_bytes, int message_length) {
  clearArray(buffer, buffer_size);
  offset = 0;
  memcpy(buffer + offset, SERIAL_HEADER, HEADER_LEN);// Header
  offset += HEADER_LEN;
  memcpy(buffer + offset, &message_id, 1);// message id
  offset++;
  if (message_bytes != NULL)
    memcpy(buffer + offset, message_bytes, message_length);// message
  else
    Serial.println("null arr");
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
 
void setup() { 
  slider_value[0] = 3;
  Timer5.initialize(31);
  Timer5.pwm(46, 512);
  // Set all encoder and switch pins as inputs with pull-ups
  // This is the most important fix!
  pinMode(inputCLK, INPUT_PULLUP);
  pinMode(DT1, INPUT_PULLUP);
  pinMode(DT2, INPUT_PULLUP);
  pinMode(DT3, INPUT_PULLUP);
  pinMode(DT4, INPUT_PULLUP);
  
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);

  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);


  //fast pwm setting
  TCCR2B &= B11111000;
  TCCR2B |= B00000001;
  //fast pwm setting
  TCCR1B &= B11111000;
  TCCR1B |= B00000001;


  eff_count = 0;
  // Setup Serial Monitor
  Serial.begin(9600);

    
  // Read the initial state of inputCLK
  // Assign to previousStateCLK variable
  previousStateCLK = digitalRead(inputCLK);

  analogWrite(effPin[4], eff_layer*255);
} 

void update_eff(int counter, int flag){
  eff[0] = counter % 2 * 255;
  eff[1] = int(floor(counter/2)) % 2 * 255;
  eff[2] = int(floor(counter/4)) % 2 * 255;

  eff[3] = (flag == 0) * 255;
  eff[4] = (flag == 1) * 255;
  eff[5] = (flag == 2) * 255;

  for (int i = 0; i < 6; i++){    
    analogWrite(effPin[i], eff[i]);
  }
}

void update_pwm(int index){
  if (index != 3){
    pwm_out[index] = map(counter[index], 0, 100, 0, 255);
    analogWrite(pin_pwm_out[index], pwm_out[index]);
  } else{
    long targetFreq = map(counter[index], 0, 100, 20, 32768);
    long newPeriod = 1000000 / targetFreq;
    Timer5.setPeriod(newPeriod);
    Timer5.setPwmDuty(46, 512);
  }
}

void update_stats(int aux) {
  // Read this encoder's DT pin
  int currentDTState = digitalRead(aux);
  
  // --- THIS IS THE FIX ---
  // Only run logic if this pin is active (LOW).
  // If it's HIGH (idle), this function will do nothing.
  if (currentDTState == LOW) { 
    int index;

    if (aux != DT2){
      index = aux - DT1; // Get the index (0, 1, or 2)
    } else{
      index = 1;
    }

    
    int count = counter[index];
    
    // Your original logic is now inside the check
    if (digitalRead(aux) != currentStateCLK) { 
      if (count > 0) {
        count--;
        encdir = "CCW " + label[index];
      }
    } else {
      if (count < 100) {
        // Encoder is rotating clockwise
        count++;
        encdir = " CW " + label[index];
      }
      
    }
    // Update the counter
    counter[index] = count;
    update_pwm(index);
  }
}
char c;
int i = 0;
bool match;
void loop() { 
  if (Serial.available() >= HEADER_LEN + PAYLOAD_LEN){
    //Serial.println("Data in buffer");
    // Check header match
    c = Serial.read();
    if (c == SERIAL_HEADER[0]){
      Serial.println("first char match");
      match = 1;
      for (i = 1; i < HEADER_LEN && match; i++){
        c = Serial.read();
        if (c != SERIAL_HEADER[i]){
          Serial.println("Header mismatch");
          match = 0;
        }
      }
      
    }
    if (match){
      Serial.println("Header match.");
      Serial.readBytes(buffer_rx, sizeof(buffer_rx));
      //printBufferBytes(buffer, sizeof(buffer));
      Serial.print("Message received through serial ports: ");
      for (int i = 0; i < sizeof(buffer_rx); i++){
        Serial.print(buffer_rx[i]);
        Serial.print(" ");
      }
      Serial.println();

      if (buffer_rx[0] == UPDATE_ALL_ENCODERS_MESSAGE_ID) {
        uint8_t counterbyte[4];
        for (int i = 0; i < 4; i++) {
          counterbyte[i] = (uint8_t)(counter[i]);
        }
        createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ALL_ENCODERS_MESSAGE_ID, counterbyte, 4);
        Serial.println("sending all slider values");
        Serial.println("Serial message to be sent: ");
        printBufferBytes(buffer_tx, HEADER_LEN + PAYLOAD_LEN);
        Serial.write(buffer_tx, sizeof(buffer_tx));
      }
      else if (buffer_rx[0] == UPDATE_ENCODER_MESSAGE_ID) {
        Serial.print("Update encoder ");
        Serial.print(buffer_rx[1]);
        Serial.print(" value ");
        Serial.println(buffer_rx[2]);
        counter[buffer_rx[1]] = buffer_rx[2];
        //old_values[buffer_rx[1]] = buffer_rx[2];

      }
    }
  }
  currentStateUP = digitalRead(A0);
  currentStateDW = digitalRead(A1);

  unsigned long currentTime = millis();


  if (currentTime - lastChangeTime > scrollSpeed) {
    
    if (currentStateUP != previousStateUP) {
      eff_count = (eff_count + 1) % 8;
      if (eff_count == 0) {
        eff_layer = (eff_layer + 1) % 3;
      }
      lastChangeTime = currentTime;
      update_eff(eff_count, eff_layer);
    }
      
    if (currentStateDW != previousStateDW) {
      eff_count = (eff_count - 1) % 8;
      if (eff_count < 0){
        eff_count = 7;
        eff_layer = (eff_layer - 1) % 3;
        if (eff_layer < 0){
          eff_layer = 2;
        }
      }
      lastChangeTime = currentTime;
      update_eff(eff_count, eff_layer);
    }
  }

  // Read the current state of inputCLK
  currentStateCLK = digitalRead(inputCLK);

  // Read all 3 buttons
  pushButt[0] = digitalRead(SW1);
  pushButt[1] = digitalRead(SW2);
  pushButt[2] = digitalRead(SW3);
  pushButt[3] = digitalRead(SW4);

  // Check all 3 buttons
  for (int i = 0; i < 4; i++) {
    if (pushButt[i] == LOW && (counter[i] != 100)) { // LOW means pressed (due to PULLUP)
      counter[i] = 100;
      update_pwm(i);
     
      //Serial.print("Direction: ");
      //Serial.print("PBW " + label[i]);
      //Serial.print(" -- Value:");
      for (int i = 0; i < 4; i++) {
        //Serial.print(" ");
        //Serial.print(counter[i]);
      }
      //Serial.println("");
    }
  }
    
  // If the previous and the current state of the inputCLK are different then a pulse has occured
  if (currentStateCLK != previousStateCLK) { 

    // Now, these calls will only affect the encoder that is
    // *actually* active (whose DT pin is LOW).
    update_stats(DT1);
    update_stats(DT2);
    update_stats(DT3);
    update_stats(DT4);

    // Only print if a direction was set (avoids printing "Direction: -- Value: ...")
    if (encdir != "") {
      /*Serial.print("Direction: ");
      Serial.print(encdir);
      Serial.print(" -- Value:");*/
      for (int i = 0; i < 4; i++) {
        //Serial.print(" ");
        //Serial.print(counter[i]);
        
        if (counter[i] != counter_[i]){
          update_pwm(i);
        }
      }
      /*Serial.println("");
      Serial.print(eff_count);
      Serial.print(" ");
      Serial.print(eff_layer);
      Serial.println("");*/
      encdir = ""; // Clear the direction string after printing

      //Send current encoder values
      uint8_t counterbyte[4];
      for (int i = 0; i < 4; i++) {
        counterbyte[i] = (uint8_t)(counter[i]);
      }
      createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ALL_ENCODERS_MESSAGE_ID, counterbyte, 4);
      //Serial.println("sending one slider value");
      Serial.write(buffer_tx, sizeof(buffer_tx));
    }
  }
  // Update previousStateCLK with the current state
  previousStateUP = currentStateUP; 
  previousStateDW = currentStateDW; 
  previousStateCLK = currentStateCLK; 
}
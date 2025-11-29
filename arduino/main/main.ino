#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8
#define UPDATE_ENCODER_MESSAGE_ID 11
#define UPDATE_ALL_ENCODERS_MESSAGE_ID 12

// Rotary Encoder Inputs
#define inputCLK 2
#define DT1 4
#define DT2 5
#define DT3 6
#define DT4 7

#define SW1 8
#define SW2 9
#define SW3 10
#define SW4 11

uint8_t buffer_tx[HEADER_LEN + PAYLOAD_LEN];
uint8_t buffer_rx[PAYLOAD_LEN];
uint8_t slider_value[2];
int offset = 0;

// Use volatile if you ever move to interrupts
volatile uint8_t counter[4] = {0, 0, 0, 0}; 
volatile uint8_t old_values[4] = {0, 0, 0, 0}; 
int currentStateCLK;
int previousStateCLK; 
int pushButt[4] = {0, 0, 0, 0};
 
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

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for (int i = 0; i < 4; i++) {
    
  }
  display.display();
  delay(2000);
}

void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  for (int i = 0; i < 4; i++) {
    display.println(counter[i]);
  }

  display.display();
  delay(2000);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino boot");
  pinMode(8, INPUT_PULLUP);
  slider_value[0] = 3;

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

  // Read the initial state of inputCLK
  // Assign to previousStateCLK variable
  previousStateCLK = digitalRead(inputCLK);


   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();

 // testdrawstyles();
  testdrawchar();
}

void update_stats(int aux) {
  // Read this encoder's DT pin
  int currentDTState = digitalRead(aux);
  
  // --- THIS IS THE FIX ---
  // Only run logic if this pin is active (LOW).
  // If it's HIGH (idle), this function will do nothing.
  if (currentDTState == LOW) { 
    
    int index = aux - DT1; // Get the index (0, 1, or 2)
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
        createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ALL_ENCODERS_MESSAGE_ID, counter, 4);
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
        old_values[buffer_rx[1]] = buffer_rx[2];
      }
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
     
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" clicked.");
      slider_value[0] = i;
      slider_value[1] = counter[i];
      createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ENCODER_MESSAGE_ID, slider_value, 2);
      Serial.write(buffer_tx, sizeof(buffer_tx));
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
      for (int i = 0; i < 4; i++) {
        if (old_values[i] != counter[i]) {
          Serial.print("Encoder ");
          Serial.print(i);
          Serial.print(" changed.");
          slider_value[0] = i;
          slider_value[1] = counter[i];
          Serial.print("Value ");
          Serial.println(counter[i]);
          createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ENCODER_MESSAGE_ID, slider_value, 2);
          //printBufferBytes(buffer_tx, HEADER_LEN + PAYLOAD_LEN);
          Serial.write(buffer_tx, sizeof(buffer_tx));
        }
      }
      encdir = ""; // Clear the direction string after printing
    }
  }
  // Update previousStateCLK with the current state
  previousStateCLK = currentStateCLK; 
}
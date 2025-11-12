#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8
#define UPDATE_ENCODER_MESSAGE_ID 11
#define UPDATE_ALL_ENCODERS_MESSAGE_ID 12

uint8_t buffer_tx[HEADER_LEN + PAYLOAD_LEN];
uint8_t buffer_rx[PAYLOAD_LEN];
uint8_t slider_value[2];
int offset = 0;

void clearArray(uint8_t* arr) {
  for (int i = 0; i < sizeof(arr); i++)
    arr[i] = 0;
}

void createSerialPayload(uint8_t* buffer, int buffer_size, uint8_t message_id, uint8_t* message_bytes, int message_length) {
  clearArray(buffer);
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
  Serial.begin(9600);
  pinMode(8, INPUT_PULLUP);
  slider_value[0] = 3;

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
        uint8_t test_values[4] = {56, 1, 9, 38};
        createSerialPayload(buffer_tx, HEADER_LEN + PAYLOAD_LEN, UPDATE_ALL_ENCODERS_MESSAGE_ID, test_values, 4);
        Serial.println("sending all slider values");
        Serial.println("Serial message to be sent: ");
        printBufferBytes(buffer_tx, HEADER_LEN + PAYLOAD_LEN);
        Serial.write(buffer_tx, sizeof(buffer_tx));
      }
      else if (buffer_rx[0] == UPDATE_ENCODER_MESSAGE_ID) {
        Serial.println("UPDATE VALUE (CURRENTLY NOT IMPLEMENTED)");
      }
    }
  }

  
  /*if (digitalRead(8) == LOW) {
    Serial.println("Sending serial data");
    slider_value[1] ++;
    createSerialPayload(buffer_tx, UPDATE_ENCODER_MESSAGE_ID, slider_value);
    Serial.write(buffer_tx, sizeof(buffer_tx));
  }*/
  delay(500);
}
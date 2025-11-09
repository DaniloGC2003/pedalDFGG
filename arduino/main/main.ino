#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8

uint8_t buffer_tx[HEADER_LEN + PAYLOAD_LEN];
uint8_t buffer_rx[PAYLOAD_LEN];
int offset = 0;

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
      printBufferBytes(buffer_rx, sizeof(buffer_rx));
    }
  }

  
  // Write header into buffer
  offset = 0;
  memcpy(buffer_tx + offset, SERIAL_HEADER, HEADER_LEN);
  offset += HEADER_LEN;
  memcpy(buffer_tx + offset, "abacaxis", 8);
  //printBufferBytes(buffer_tx, sizeof(buffer_tx));
  //Serial.print("Sending serial data: ");
  //Serial.write(buffer_tx, sizeof(buffer_tx));
  //Serial.println();
  //Serial.println(Serial.readString());
  delay(500);
}
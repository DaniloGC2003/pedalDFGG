#define SERIAL_HEADER "ESP"
#define HEADER_LEN 3
#define PAYLOAD_LEN 8

uint8_t buffer[HEADER_LEN + PAYLOAD_LEN];
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
void loop() {
  // Write header into buffer
  offset = 0;
  memcpy(buffer + offset, SERIAL_HEADER, HEADER_LEN);
  offset += HEADER_LEN;
  memcpy(buffer + offset, "abacaxis", 8);
  //printBufferBytes(buffer, sizeof(buffer));
  Serial.print("Sending serial data: ");
  Serial.write(buffer, sizeof(buffer));
  Serial.println();
  //Serial.println(Serial.readString());
  delay(5000);
}
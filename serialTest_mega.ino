void setup() {
  Serial.begin(9600);
}

void loop() {
  while(Serial.available())
    Serial.print(Serial.readString());
    //Serial.write(Serial.read()); // alternative
}

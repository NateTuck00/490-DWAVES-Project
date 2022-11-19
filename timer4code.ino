void analogWrite25k(int value)
{
    OCR4C = value;
}

void setup()
{    
    TCCR4A = 0;
    TCCR4B = 0;
    TCNT4  = 0;

    // Mode 10: phase correct PWM with ICR4 as Top (= F_CPU/2/25000)
    // OC4C as Non-Inverted PWM output
    ICR4   = (F_CPU/25000)/2;
    OCR4C  = ICR4/2;                    // default: about 50:50
    TCCR4A = _BV(COM4C1) | _BV(WGM41);
    TCCR4B = _BV(WGM43) | _BV(CS40);

    Serial.begin(115200);

    // Set the PWM pin as output.
    pinMode( 8, OUTPUT);
}

void loop()
{
    int w = Serial.parseInt();
    if (w>0) {
      analogWrite25k(w);
      Serial.println(w);
    }
}
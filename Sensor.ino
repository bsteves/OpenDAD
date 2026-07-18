/******************************************************************************
  OpenDAD TLE5012 sensor interface
******************************************************************************/

// Old ESP32 pins
// const int DATA_PIN = 23;
// const int SCK_PIN  = 18;
// const int CS_PIN   = 5;


const int DATA_PIN = D4;
const int CS_PIN   = D2;
const int SCK_PIN  = D3;

const uint16_t CMD_READ_ANGLE = 0x8021;

/******************************************************************************
  Configure sensor pins
******************************************************************************/

void setupPins()
{
  pinMode(CS_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);

  digitalWrite(CS_PIN, HIGH);
  digitalWrite(SCK_PIN, LOW);
}



/******************************************************************************
  Transfer one 16-bit SSC word
******************************************************************************/
uint16_t sscTransfer16(uint16_t command)
{
  uint16_t response = 0;

  /*
    Keep timing consistent while the 16-bit command and response
    are transferred.
  */
  noInterrupts();

  /*
    SSC clock idles LOW.
  */
  digitalWrite(SCK_PIN, LOW);

  /**************************************************************************
    Send the 16-bit command, MSB first
  **************************************************************************/

  pinMode(DATA_PIN, OUTPUT);

  for (int bit = 15; bit >= 0; bit--)
  {
    /*
      Put the next command bit on DATA while SCK is LOW.
    */
    digitalWrite(
      DATA_PIN,
      (command & (1U << bit)) ? HIGH : LOW
    );

    delayMicroseconds(1);

    /*
      The sensor accepts the transmitted bit on the rising edge.
    */
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(2);

    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(2);
  }

  /**************************************************************************
    Release DATA so the TLE5012B can drive it
  **************************************************************************/

  /*
    Set DATA high briefly before changing it to input. The board's
    470-ohm series resistor limits contention during the handoff.
  */
  digitalWrite(DATA_PIN, HIGH);
  pinMode(DATA_PIN, INPUT);

  delayMicroseconds(2);

  /**************************************************************************
    Read the 16-bit response, MSB first
  **************************************************************************/

  for (int bit = 15; bit >= 0; bit--)
  {
    /*
      The TLE5012B updates DATA around the rising edge.
    */
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(2);

    /*
      Sample DATA while bringing the clock low.
    */
    digitalWrite(SCK_PIN, LOW);

    if (digitalRead(DATA_PIN))
    {
      response |= static_cast<uint16_t>(1U << bit);
    }

    delayMicroseconds(2);
  }

  /*
    Leave the bus in its idle state.
  */
  digitalWrite(SCK_PIN, LOW);
  pinMode(DATA_PIN, INPUT);

  interrupts();

  return response;
}


/******************************************************************************
  Read the raw sensor word
******************************************************************************/

uint16_t readSensorRawWord()
{
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(2);

  uint16_t rawWord = sscTransfer16(CMD_READ_ANGLE);

  delayMicroseconds(2);
  digitalWrite(CS_PIN, HIGH);

  return rawWord;
}




/******************************************************************************
  Convert the raw signed sensor word to 0-360 degrees
******************************************************************************/

float rawWordToDegrees(uint16_t raw)
{
  int16_t signedRaw = static_cast<int16_t>(raw);

  float degrees =
    static_cast<float>(signedRaw) * 360.0 / 32768.0;

  if (degrees < 0.0)
  {
    degrees += 360.0;
  }

  return degrees;
}
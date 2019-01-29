// RECEIVER

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

#define TOO_HOT "HOT"
#define TOO_COLD "COL"
#define OK "OK"
#define TOO_HOT_ADDR 0
#define TOO_COLD_ADDR 1

RH_ASK driver;

uint8_t buf[13];
uint8_t buflen;
uint8_t key[4];
uint8_t value[8];

bool buttonPressed = false;
bool buttonPressedTwice = false;

unsigned long firstDebounceTime = 0;
unsigned long secondDebounceTime = 0;

const unsigned long debounceDelay = 3000;
const unsigned long cancelDelay = 5000;

void setup()
{
  pinMode(A0, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(3), buttonEvent, RISING);
  Serial.begin(9600); // Debugging only
  if (!driver.init())
  {
    Serial.println("init failed");
  }
}

void buttonEvent()
{
  Serial.println("Button noise");
  if (buttonPressed == true)
  {
    secondDebounceTime = millis();
    Serial.print("Time between debounces: ");
    Serial.println(secondDebounceTime - firstDebounceTime);
  }
  if (buttonPressed == false)
  {
    buttonPressed = true;
    firstDebounceTime = millis();
    secondDebounceTime = millis();
  }
}

void loop()
{
  receiveData();
  interpretSignal();
  checkButtonTwicePress();
  // displayData();
}

void receiveData()
{
  buflen = sizeof(buf);
  for (int i = 0; i < 12; i++)
  {
    buf[i] = 0;
  }
  if (driver.recv(buf, &buflen)) // Non-blocking
  {
    readData(buf, key, value);
  }
}

void readData(const uint8_t *data, uint8_t *key, uint8_t *value)
{
  int i = 0;
  int j = 0;

  while (data[i] != ':')
  {
    key[i] = data[i];
    ++i;
  }
  key[i] = '\0';

  ++i;

  while (data[i] != '\0')
  {
    value[j] = data[i];
    ++i;
    ++j;
  }
  value[j] = '\0';
}

void interpretSignal()
{
  if (strcmp(TOO_COLD, (char *)key) == 0 && buttonPressed == false)
  {
    Serial.print("Alert!: ");
    Serial.println((char *)buf);
    tone(A0, 500, 500);
    delay(1000);
  }
  else if (strcmp(TOO_HOT, (char *)key) == 0 && buttonPressedTwice == false)
  {
    Serial.print("Alert!: ");
    Serial.println((char *)buf);
    tone(A0, 1000, 150);
    delay(300);
  }
  else if (strcmp(OK, (char *)key) == 0)
  {
    buttonPressed = false;
    buttonPressedTwice = false;
    // Message with a good checksum received, dump it.
    Serial.print("Message: ");
    Serial.println((char *)buf);
    delay(900);
  }
}

void checkButtonTwicePress()
{
  if (buttonPressed == true)
  {
    if ((secondDebounceTime - firstDebounceTime) > debounceDelay)
    {
      buttonPressedTwice = true;
      Serial.println("Pressed button twice");
    }
    firstDebounceTime = 0;
    secondDebounceTime = 0;
  }
}

/*
 * What it should do:
 * 1. Show current temperature sent by the transmitter.
 * 2a. Turn on the sound alarm if received signal from
 *     the transmitter, until snooze button is pressed.
 * 2b. Turn on the sound alarm if temperature has gone
 *     too high or low, based on saved variables received
 *     from the transmitter.
 * 3a. Make different sound alarms when received different
 *     events.
 * 3b. Make different sound alarms when different events
 *     occur (when device is capable of storing variables)
 */
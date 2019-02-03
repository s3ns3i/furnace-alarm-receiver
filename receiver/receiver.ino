// RECEIVER

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <stdlib.h>

#define TOO_HOT "HOT"
#define TOO_COLD "COL"
#define CRITICAL "CRT"
#define OK "OK"
#define TOO_HOT_ADDR 0
#define TOO_COLD_ADDR 1
#define DEBOUNCE_DELAY 3000
#define CANCEL_DELAY 5000
#define SIGNAL_DELAY 10000
#define TEMP_THRESHOLD 5.0f

RH_ASK driver;

uint8_t buf[13];
uint8_t buflen;
uint8_t key[4];
uint8_t value[8];

bool buttonPressed = false;
bool buttonPressedTwice = false;
bool lostSignal = false;
bool reachedCriticalTemperature = false;

unsigned long firstDebounceTime = 0;
unsigned long secondDebounceTime = 0;
unsigned long lastSignalReceived = 0;

float currentTemperature = 0.0f;
float milestoneTemperature = 0.0f;

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
  if (reachedCriticalTemperature == true)
  {
    return;
  }
  if (buttonPressed == true)
  {
    secondDebounceTime = millis();
    Serial.println(secondDebounceTime - firstDebounceTime);
  }
  if (buttonPressed == false)
  {
    buttonPressed = true;
    firstDebounceTime = millis();
    secondDebounceTime = millis();
  }
}

void checkButtonTwicePress()
{
  if (buttonPressed == true)
  {
    if ((secondDebounceTime - firstDebounceTime) > DEBOUNCE_DELAY)
    {
      buttonPressedTwice = true;
      milestoneTemperature = currentTemperature;
    }
    firstDebounceTime = 0;
    secondDebounceTime = 0;
  }
}

void loop()
{
  receiveData();
  checkSignalDelay();
  if (lostSignal == false)
  {
    interpretSignal();
    checkButtonTwicePress();
    checkTemperatureGrowAfterSnooze();
    // displayData();
  }
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
    lastSignalReceived = millis();
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
  currentTemperature = atof((char *)value);
}

void interpretSignal()
{  
  if (strcmp(TOO_COLD, (char *)key) == 0 && buttonPressed == false)
  {
    reachedCriticalTemperature = false;
    tone(A0, 500, 500);
    delay(1000);
  }
  else if (strcmp(TOO_HOT, (char *)key) == 0 && buttonPressedTwice == false)
  {
    reachedCriticalTemperature = false;
    tone(A0, 1000, 150);
    delay(300);
  }
  else if (strcmp(CRITICAL, (char *)key) == 0)
  {
    reachedCriticalTemperature = true;
    tone(A0, 1000, 120);
    delay(240);
  }
  else if (strcmp(OK, (char *)key) == 0)
  {
    reachedCriticalTemperature = false;
    buttonPressed = false;
    buttonPressedTwice = false;
    delay(900);
  }
}

void checkTemperatureGrowAfterSnooze()
{
  if(currentTemperature >= (milestoneTemperature + TEMP_THRESHOLD) && buttonPressedTwice == true)
  {
    buttonPressedTwice = false;
  }
}

void checkSignalDelay()
{
  if ((millis() - lastSignalReceived) > SIGNAL_DELAY)
  {
    if (lostSignal == false)
    {
      lostSignal = true;
    }
    buttonPressed = false;
    buttonPressedTwice = false;
    reachedCriticalTemperature = false;
    firstDebounceTime = 0;
    secondDebounceTime = 0;
    tone(A0, 300, 700);
    delay(1400);
  }
  else
  {
    if (lostSignal == true)
    {
      tone(A0, 900, 150);
      delay(300);
      tone(A0, 900, 150);
      delay(300);
    }
    lostSignal = false;
  }
}

/*
 * What it should do:
 * 1.  Show current temperature sent by the transmitter.        CHECK
 * 2a. Turn on the sound alarm if received signal from          CHECK
 *     the transmitter, until snooze button is pressed.
 * 2b. Turn on the sound alarm if temperature has gone          CHECK
 *     too high or low, based on saved variables received
 *     from the transmitter.
 * 3.  Make different sound alarms when received different      CHECK
 *     events.
 * 4.  Turn on the sound alarm if battery level is too low      TODO
 * 5.  Turn on the sound alarm if device loses signal           CHECK
 * 6.  If temperature is above threshold and device is          CHECK
 *     snoozed, then if it goes another 5 degrees up, it
 *     should start alarming again
 * 7.  Do the same when temperature is below threshold          NOT IMPLEMENTING
 * 8.  If max temperature or above has been reached             CHECK
 *     (CRITICAL event), alarm cannot be snoozed until it
 *     gets TOO_HOT event again
 */
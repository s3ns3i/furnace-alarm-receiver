// RECEIVER

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <EEPROM.h>

#define TOO_HOT "HOT"
#define TOO_COLD "COL"
#define OK "OK"
#define TOO_HOT_ADDR 0
#define TOO_COLD_ADDR 1

RH_ASK driver;

uint8_t buf[13];
uint8_t buflen;
int addr = 0;

void setup()
{
  pinMode(A1, OUTPUT);
  Serial.begin(9600); // Debugging only
  if (!driver.init())
    Serial.println("init failed");
}

void loop()
{
  // tone(A1, 1000, 2000);
  receiveData();
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
    uint8_t key[4];
    uint8_t value[8];
    readData(buf, key, value);
    Serial.print("Key: ");
    Serial.println((char *)key);
    Serial.print("Value: ");
    Serial.println((char *)value);
    if (strcmp(TOO_COLD, (char *)key) == 0)
    {
      Serial.print("Alert!: ");
      saveData(buf);
    }
    else if (strcmp(TOO_HOT, (char *)key) == 0)
    {
      Serial.print("Alert!: ");
      Serial.println((char *)buf);
      saveData(buf);
    }
    else if (strcmp(OK, (char *)key) == 0)
    {
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      Serial.println((char *)buf);
    }
  }
}

void saveData(uint8_t *data)
{
  Serial.println("Saving data to EEPROM");
  uint8_t key[4];
  uint8_t value[8];
  readData(data, key, value);
  Serial.print("Int value: ");
  Serial.println((char *)value);
  EEPROM.write(TOO_HOT_ADDR, atoi((char *)value));
  int readValue = EEPROM.read(TOO_HOT_ADDR);
  Serial.print("Read value: ");
  Serial.println(readValue);
  // uint8_t readValue = 
}

void readData(const uint8_t *data, uint8_t *key, uint8_t *value)
{
  Serial.println("Reading data");
  // char _key[4];
  // char _value[8];
  int i = 0;
  int j = 0;
  
  while(data[i] != ':')
  {
    Serial.print("Reading key: ");
    Serial.println(data[i]);
    key[i] = data[i];
    ++i;
  }
  key[i] = '\0';

  ++i;
  
  while(data[i] != '\0')
  {
    Serial.print("Reading value: ");
    Serial.println(data[i]);
    value[j] = data[i];
    ++i;
    ++j;
  }
  value[j] = '\0';
  
  // key = _key;
  // value = _value;
  // for(int i = 0; data[i] != '\0'; i++)
  // {
  //   _key[i] = 
  // }
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
#include <SoftwareSerial.h>
#include "RDM6300.h"

SoftwareSerial rdm_serial(2, 3);
RDM6300<SoftwareSerial> rdm(&rdm_serial);

int led_pin = 13;

void blink(int n = 1) 
{
  for(int i = 0; i < n; i++) {
    digitalWrite(led_pin, HIGH);
    delay(200);
    digitalWrite(led_pin, LOW);
    delay(200);
  }
}

void setup()
{
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  Serial.begin(115200);
  Serial.println("SETUP");
  blink(5);
}

void loop()
{
  static const unsigned long long my_id = 0x0000ABCDEF;
  static unsigned long long last_id = 0;

  last_id = rdm.read();
  Serial.print("RFID: 0x");
  rdm.print_int64(last_id);
  Serial.println();
  
  if(last_id == my_id) blink(3);

}

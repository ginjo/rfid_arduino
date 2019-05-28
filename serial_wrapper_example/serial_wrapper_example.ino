/*
  Creates a generic software/hardware serial wrapper for arduino.

  Copied from arduino discussion thread:
  https://forum.arduino.cc/index.php?topic=503782.0
*/


#include <SoftwareSerial.h>

#define MESSAGE_LENGTH 64

class SerialMessenger {
  public:
    SerialMessenger( HardwareSerial& device) {hwStream = &device;}
    SerialMessenger( SoftwareSerial& device) {swStream = &device;}
    void begin(uint32_t baudRate);
    char* checkForNewMessage(const char endMarker, bool errors);
    
  private:
    HardwareSerial* hwStream;
    SoftwareSerial* swStream;
    Stream* stream;
    char incomingMessage[MESSAGE_LENGTH];
    size_t idx = 0;
};

char* SerialMessenger::checkForNewMessage(const char endMarker, bool errors = false)
{
  stream = !hwStream? (Stream*)swStream : hwStream;
  if (stream->available())
  {
    if (stream->peek() == '\r')
    {
      (void)stream->read();
      return nullptr;
    }
    incomingMessage[idx] = stream->read();
    if (incomingMessage[idx] == endMarker)
    {
      incomingMessage[idx] = '\0';
      idx = 0;
      return incomingMessage;
    }
    else
    {
      idx++;
      if (idx > MESSAGE_LENGTH - 1)
      {
        if (errors)
        {
          stream->print(F("{\"error\":\"message too long\"}\n"));
        }
        while (stream->read() != '\n')
        {
          delay(50);
        }
        idx = 0;
        incomingMessage[idx] = '\0';
      }
    }
  }
  return nullptr;
}

void SerialMessenger::begin(uint32_t baudRate)
{
  if (hwStream)
  {
    hwStream->begin(baudRate);
  }
  else
  {
    swStream->begin(baudRate);
  }
}

SoftwareSerial softSerial(2, 3);

SerialMessenger vera(Serial);
SerialMessenger electron(softSerial);

void setup()
{
  vera.begin(9600);
  electron.begin(9600);
}


void loop()
{
  if(const char* veraMessage = vera.checkForNewMessage('\n'))
  {
    char newMessage[MESSAGE_LENGTH];
    strcpy(newMessage, veraMessage);
    Serial.println(newMessage);
  }
  if(const char* electronMessage = electron.checkForNewMessage('\n'))
  {
    char newMessage[MESSAGE_LENGTH];
    strcpy(newMessage, electronMessage);
    Serial.println(newMessage);
  }
}

#include <SoftwareSerial.h>
#include "serial_port.h"

  SerialPort::SerialPort() :
    is_bt(false),
    is_sw_serial(false)
  {;}

  bool SerialPort::can_output() {
    return CanLogToBT() || ! is_bt;
  }

  bool SerialPort::isListening() {
    if (is_sw_serial) {
      return (SoftwareSerial*)this->isListening();
    } else {
      return true;
    }
  }

  bool SerialPort::listen() {
    if (is_sw_serial) {
      return (SoftwareSerial*)this->listen();
    } else {
      return false;
    }
  }

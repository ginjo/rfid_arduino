#include <SoftwareSerial.h>
#include "serial_port.h"

  SerialPort *SerialPort::List[SERIAL_PORT_LIST_SIZE] = {};
  uint8_t SerialPort::Count = 0U;

  SerialPort* SerialPort::Add (HardwareSerial *hw, bool _is_bt) {
    List[Count] = (SerialPort*)hw;
    List[Count]->is_bt = _is_bt;
    Count++;
    return List[Count - 1U];
  }

  SerialPort* SerialPort::Add (SoftwareSerial *sw, bool _is_bt) {
    List[Count] = (SerialPort*)sw;
    List[Count]->is_bt = _is_bt;
    List[Count]->is_sw_serial = true;
    Count++;
    return List[Count - 1U];
  }


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


  

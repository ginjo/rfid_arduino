#include "serial_port.h"

  bool SerialPort::can_output() {
    return CanLogToBT() || ! is_bt;
  }

  #include "settings.h"

  extern const char SettingNames[16][24] = {
      "failure_status",
      "timeout_s",
      "rfid_skip_cycle_ms",
      "rfid_power_cycle_ms"
  };
  
  extern const int MySettings[16] = {
      1, 20, 1000, 5000
  };


  extern const int my_setting(int val = 0) {
    return val;
  }

  extern const int index_of(char _name[], char _names[16][24]) {
      for (int n = 0; n < 16; n++) {
          if (strcmp(_name, _names[n]) == 0) {
              return n;
          }
      }
      return -1;
  }
  
  extern const int value_of(char _name[], char _names[16][24], int _settings[]) {
      int index = index_of(_name, _names);
      return _settings[index];
  }


  // This was first prototyped in onlinegbd.com
  //
  //  int main()
  //  {
  //      printf("Name at 3: %s\n", names[3]);
  //      printf("Index of 'timeout_s' in names[]: %i\n", index_of("timeout_s", names));
  //      printf("value_of of 'rfid_power_cycle_ms' in MySettings: %i\n",
  //          value_of("rfid_power_cycle_ms", names, MySettings)
  //      );
  //  
  //      return 0;
  //  }

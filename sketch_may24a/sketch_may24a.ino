      
const byte BUTTON_PIN = 7;

class Button {
  public:
    void setup() {
      pinMode(BUTTON_PIN, INPUT_PULLUP);
    }

    void loop() {
    }
};

Button button;

void setup() {
  button.setup();
}

void loop() {
  button.loop();
}

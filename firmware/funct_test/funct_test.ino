#define btn_1 13
#define btn_2 12
#define btn_3 14
#define btn_4 27
#define btn_5 26
#define btn_6 25
#define btn_7 33
#define btn_8 32

#define SR_data 5
#define SR_OE 18
#define SR_latch 19
#define SR_clk 21

#define ROT_SW 15
#define ROT_A 2
#define ROT_B 4 // clk

#define LEDRING_PIN 23
#define LEDRING_NUM 12

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel ring = Adafruit_NeoPixel(LEDRING_NUM, LEDRING_PIN, NEO_GRB + NEO_KHZ800);

volatile bool prev_rot_b;
volatile bool cur_rot_b;
volatile uint8_t counter = 0;

void setup() {
  // init buttons
  pinMode(btn_1, INPUT_PULLDOWN);
  pinMode(btn_2, INPUT_PULLDOWN);
  pinMode(btn_3, INPUT_PULLDOWN);
  pinMode(btn_4, INPUT_PULLDOWN);
  pinMode(btn_5, INPUT_PULLDOWN);
  pinMode(btn_6, INPUT_PULLDOWN);
  pinMode(btn_7, INPUT_PULLDOWN);
  pinMode(btn_8, INPUT_PULLDOWN);

  // init shift register
  pinMode(SR_data, OUTPUT);
  pinMode(SR_OE, OUTPUT);
  pinMode(SR_latch, OUTPUT);
  pinMode(SR_clk, OUTPUT);
  for (uint8_t i = 0; i < 4; i++) {
    updateShiftRegister((1 << i) | (1 << i + 4));
    delay(120);
  }
  updateShiftRegister(0); // clear any persistent data
  digitalWrite(SR_OE, HIGH); // disable output

  // init rotary encoder
  pinMode(ROT_SW, INPUT);
  pinMode(ROT_A, INPUT_PULLUP);
  pinMode(ROT_B, INPUT);
  prev_rot_b = digitalRead(ROT_B);


  // init ledring
  ring.begin();
  ring.clear();
  ring.show();
  for (uint8_t i = 0; i < 12; i++) {
    delay(70);
    ring.setPixelColor(i, ring.Color(0, 120, 0));
    ring.show();
  }

  // Serial
  Serial.begin(115200);
  Serial.println("Setup completed! Interrupts will now be attached");

  // attach interrupts
  attachInterrupt(ROT_A, updateEncoder, CHANGE);
  attachInterrupt(ROT_B, updateEncoder, CHANGE);
}

void loop() {
  // test Keys and keycap leds
  digitalWrite(SR_OE, LOW); // enable SR output
  if (digitalRead(btn_1))updateShiftRegister(1 << 7);
  if (digitalRead(btn_2))updateShiftRegister(1 << 6);
  if (digitalRead(btn_3))updateShiftRegister(1 << 5);
  if (digitalRead(btn_4))updateShiftRegister(1 << 4);
  if (digitalRead(btn_5))updateShiftRegister(1 << 3);
  if (digitalRead(btn_6))updateShiftRegister(1 << 2);
  if (digitalRead(btn_7))updateShiftRegister(1 << 1);
  if (digitalRead(btn_8))updateShiftRegister(1);


  // disco mode
  if (!digitalRead(ROT_SW)) {
    detachInterrupt(digitalPinToInterrupt(ROT_A));
    detachInterrupt(digitalPinToInterrupt(ROT_B));
    uint8_t pos = 0;
    while (!digitalRead(ROT_SW)) {

      for (uint8_t i = 0; i < 4; i++) {
        ring.setPixelColor((pos + i) % 12  , ring.Color(120, 0, 0));
        ring.setPixelColor((pos + i + 4) % 12, ring.Color(0, 120, 0));
        ring.setPixelColor((pos + i + 8) % 12, ring.Color(0, 0, 120));
      }
      ring.show();
      delay(50);
      pos++;
      if (pos >= 12) pos = 0;
      yield();
    }
    attachInterrupt(ROT_A, updateEncoder, CHANGE);
    attachInterrupt(ROT_B, updateEncoder, CHANGE);
    setRingToCounter();
  }
}

// rotary interrupt
void updateEncoder() {
  cur_rot_b = digitalRead(ROT_B);
  if (cur_rot_b != prev_rot_b && cur_rot_b == 1) {
    if (digitalRead(ROT_A) != cur_rot_b) {
      counter--;
    } else {
      counter++;
    }
  }
  prev_rot_b = cur_rot_b;
  setRingToCounter();
}

void setRingToCounter() {
  for (uint8_t i = 0; i < (counter >> 1 ) % 12 ; i++) {
    ring.setPixelColor(i, ring.Color(0, 0, 120));
  }
  for (uint8_t i = (counter >> 1) % 12; i < 12; i++) {
    ring.setPixelColor(i, ring.Color(0, 0, 0));
  }
  ring.show();
}

void updateShiftRegister(byte leds)
{
  digitalWrite(SR_latch, LOW);
  shiftOut(SR_data, SR_clk, LSBFIRST, ~leds);
  digitalWrite(SR_latch, HIGH);
}

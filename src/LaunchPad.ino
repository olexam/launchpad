#include "FastLED.h"
#include <AceButton.h>
using namespace ace_button;

#define LEDSTRIP_PIN 4
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define BRIGHTNESS 32

CRGB leds[NUM_LEDS];


#define TRANSITION_TIMEOUT 1500

#define IGNITOR_PIN 5

#define LAUNCH_BTN 6
#define ARM_BTN 2

AceButton armBtn;
AceButton launchBtn;

void handleEvent(AceButton*, uint8_t, uint8_t);

enum Status {NONE, IDLE, ARMED, IGNITION};

Status status;
Status prevStatus;
boolean ignitorConn;
boolean inTransition;

int transitionStart;

void setup() {
  delay(1000); // some microcontrollers reboot twice
  Serial.begin(115200);
  while (! Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ARM_BTN, INPUT_PULLUP);
  armBtn.init(ARM_BTN);
  ButtonConfig* armBtnConfig = armBtn.getButtonConfig();
  armBtnConfig->setEventHandler(handleEvent);

  pinMode(LAUNCH_BTN, INPUT_PULLUP);
  launchBtn.init(LAUNCH_BTN);
  ButtonConfig* launchBtnConfig = launchBtn.getButtonConfig();
  // launchBtnConfig->setEventHandler(launchEvntHandle);

  pinMode(IGNITOR_PIN, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE,LEDSTRIP_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // initializes LED strip
  FastLED.setBrightness(BRIGHTNESS);// global brightness

  if (armBtn.isPressedRaw()) {
    Serial.println(F("setup(): armBtn was pressed while booting"));
  }

  status = IDLE;
  prevStatus = NONE;

  displayStatus();
  Serial.println(F("setup(): ready"));
}

void loop() {
  launchBtn.check();
  armBtn.check();
  ignitorConn = digitalRead(IGNITOR_PIN) == LOW;
  if (status != IGNITION && !ignitorConn) {
    Serial.println(F("Ignitor not connected"));
    status = IDLE;
  }
  if (prevStatus != status) {
    prevStatus = status;
    displayStatus();
  }
  if (inTransition) {
    displayProgress();
  }
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (button->getPin() == ARM_BTN) {
    handleArmEvent(button, eventType, buttonState);
  }
  if (button->getPin() == LAUNCH_BTN) {
    handleLaunchEvent(button, eventType, buttonState);
  }
}

void handleArmEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  if (!ignitorConn) {
    return;
  }

  switch (eventType) {
    case AceButton::kEventPressed:
      if (status == IDLE) {
        inTransition = true;
        transitionStart = millis();
      } else {
        status = IDLE;
      }
      break;
    case AceButton::kEventReleased:
      if (status == IDLE && transitionStart > 0 && (millis() - transitionStart) > TRANSITION_TIMEOUT) {
        status = ARMED;
      } else {
        status = IDLE;
      }
      inTransition = false;
      transitionStart = 0;
      break;
  }
  prevStatus = NONE;
}

void handleLaunchEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println(F("launchEvntHandle(): enter"));
  if (!ignitorConn || status != ARMED) {
    return;
  }
  switch (eventType) {
    case AceButton::kEventPressed:
      if (status == ARMED) {
        inTransition = true;
        transitionStart = millis();
      }
      break;
    case AceButton::kEventReleased:
      if (status == ARMED && transitionStart > 0 && (millis() - transitionStart) > TRANSITION_TIMEOUT) {
        status = IGNITION;
      }
      inTransition = false;
      transitionStart = 0;
      break;
  }
  prevStatus = NONE;
}


void displayStatus() {
  switch (status) {
    case IDLE:
      ledShow(CRGB::White);
      break;
    case ARMED:
      ledShow(CRGB::Orange);
      break;
    case IGNITION:
      ledShow(CRGB::Red);
      break;
  }
}

void displayProgress() {
      int timeFromPrearm = millis() - transitionStart;
      int progres = map(timeFromPrearm, 0, TRANSITION_TIMEOUT, 1, NUM_LEDS);
      progres = progres < NUM_LEDS ? progres : NUM_LEDS;
      if (status == IDLE) {
        ledShow(CRGB::Orange, CRGB::White, progres-1);
      }
      if (status == ARMED) {
        ledShow(CRGB::Red, CRGB::Orange, progres-1);
      }
}

void ledShow(CRGB color) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = color;
  }
  FastLED.show();
}

void ledShow(CRGB color1, CRGB color2, int p) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = i <= p ? color1 : color2;
  }
  FastLED.show();
}

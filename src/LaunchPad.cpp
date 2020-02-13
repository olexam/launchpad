#include <Arduino.h>
#include <FastLED.h>
#include <AceButton.h>
using namespace ace_button;

#define ARM_BTN 2
#define LAUNCH_BTN 3
#define CONNECTIVITY_PIN 4
// #define IGNITOR_PIN LED_BUILTIN// 7
#define IGNITOR_PIN 6

#define LEDSTRIP_PIN 5
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define BRIGHTNESS 32
#define LED_LONG_PAUSE 1500
#define LED_SHORT_PAUSE 100
#define LED_BLINK_TIME 100

#define TRANSITION_TIMEOUT 1500

// #define IDLE_COL CRGB::White
#define IDLE_COL CRGB::GhostWhite
#define ARM_COL CRGB::Green
#define IGN_COL CRGB::Red

void handleEvent(AceButton*, uint8_t, uint8_t);
void handleArmEvent(AceButton*, uint8_t, uint8_t);
void handleLaunchEvent(AceButton*, uint8_t, uint8_t);
void displayStatus();
void displayProgress();
void ledShow(CRGB);
void ledShow(CRGB, CRGB, int);
void printState(const char*);
void initIgnitor();
void initButtons();
void initLedStrip();
void readInputs();
void proceedIgnition();
void updateStatus();
void blinkStrip();

enum Status {NONE, IDLE, ARMED, IGNITION};

struct Seq
{
  long pause;
  void* func;
};


CRGB leds[NUM_LEDS];
AceButton* armBtn;
AceButton* launchBtn;

Status status = NONE;
Status prevStatus = NONE;
boolean ignitorConn = false;
boolean inTransition = false;

long transitionStart = 0;
long cTime = 0;

long bTime = 0;
long blinkTimes[] = {LED_BLINK_TIME, LED_SHORT_PAUSE, LED_BLINK_TIME, LED_LONG_PAUSE};
long blinkVals[] = {255, BRIGHTNESS, 255, BRIGHTNESS};
int bIndex = 0;

void setup() {
  Serial.begin(115200);
  while (! Serial); // Wait until Serial is ready - Leonardo/Micro

  printState("setup(), state begin");

  initButtons();
  initLedStrip();
  initIgnitor();

  status = IDLE;

  delayMicroseconds(500); // wait a state with black strip
  displayStatus();
  printState("setup(), state on finish");
}

void loop() {
  readInputs();
  updateStatus();
  proceedIgnition();

  blinkStrip();

  if (inTransition) {
    displayProgress();
  } else {
      displayStatus();
  }
  if (prevStatus != status) {
    prevStatus = status;
    printState("MainLoop");
  }
  
}

void blinkStrip() {
  long diff = cTime - bTime;
  FastLED.setBrightness(blinkVals[bIndex]);
  if (diff > blinkTimes[bIndex]) {
    bTime = cTime;
    bIndex++;
    if (bIndex > 3) bIndex = 0;
  } 

}

void proceedIgnition() {
  if (status == IGNITION) {
    if (prevStatus != IGNITION) {
      inTransition = true;
      transitionStart = cTime;
    } else {
      if (transitionStart > 0 && (cTime - transitionStart) > TRANSITION_TIMEOUT) {
        status = IDLE;
        inTransition = false;
        transitionStart = 0;
      }
    }
  }
  if (status == IGNITION) {
      digitalWrite(IGNITOR_PIN, LOW);
  } else {
      digitalWrite(IGNITOR_PIN, HIGH);
  }
}

void updateStatus() {
  if (!ignitorConn && status != IGNITION) {
    Serial.println(F("Ignitor not connected"));
    status = IDLE;
    inTransition = false;
    transitionStart = 0;
  }
}

void readInputs() {
  cTime = millis();
  launchBtn->check();
  armBtn->check();
  ignitorConn = digitalRead(CONNECTIVITY_PIN) == LOW;
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.print(F("handleEvent(): button pin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  printState("HandleEvent");

/* 
  if (!ignitorConn) {
    return;
  }
 */
  if (button->getPin() == ARM_BTN) {
    handleArmEvent(button, eventType, buttonState);
  }
  if (button->getPin() == LAUNCH_BTN) {
    handleLaunchEvent(button, eventType, buttonState);
  }
}

void handleArmEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      if (status == IDLE) {
        inTransition = true;
        transitionStart = cTime;
      } else {
        status = IDLE;
      }
      break;
    case AceButton::kEventReleased:
      if (status == IDLE && transitionStart > 0 && (cTime - transitionStart) > TRANSITION_TIMEOUT) {
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
  switch (eventType) {
    case AceButton::kEventPressed:
      if (status == ARMED) {
        inTransition = true;
        transitionStart = cTime;
      }
      break;
    case AceButton::kEventReleased:
      if (status == ARMED) {
        if (transitionStart > 0 && (cTime - transitionStart) > TRANSITION_TIMEOUT) {
          status = IGNITION;
        }
      }
      inTransition = false;
      transitionStart = 0;
      break;
  }
  prevStatus = NONE;
}


void displayStatus() {
  switch (status) {
    case IGNITION:
      ledShow(IGN_COL);
      break;
    case ARMED:
      ledShow(ARM_COL);
      break;
    case NONE:
    case IDLE:
      ledShow(IDLE_COL);
      break;
  }
}

void displayProgress() {
    int timeFromPrearm = cTime - transitionStart;
    int progres = map(timeFromPrearm, 0, TRANSITION_TIMEOUT, 1, NUM_LEDS);
    progres = progres < NUM_LEDS ? progres : NUM_LEDS;
    if (status == IDLE) {
      ledShow(ARM_COL, IDLE_COL, progres-1);
    }
    if (status == ARMED) {
      ledShow(IGN_COL, ARM_COL, progres-1);
    }
}

void ledShow(CRGB color) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = color;
  }
  FastLED.show();
}

void ledShow(CRGB colorTo, CRGB colorFrom, int p) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = i <= p ? colorTo : colorFrom;
  }
  FastLED.show();
}

void printState(const char* pref) {
  Serial.print(pref);
  Serial.print(F(" : prevStatus="));
  Serial.print(prevStatus);
  Serial.print(F(" : status="));
  Serial.print(status);
  Serial.print(F("; inTransition="));
  Serial.print(inTransition);
  Serial.print(F("; transitionStart="));
  Serial.print(transitionStart);
  Serial.print(F("; ignitorConn="));
  Serial.print(ignitorConn);
  Serial.println();
}

void initIgnitor() {
  pinMode(IGNITOR_PIN, OUTPUT);
  digitalWrite(IGNITOR_PIN, HIGH);
}

void initButtons() {
  pinMode(ARM_BTN, INPUT_PULLUP);
  armBtn = new AceButton(ARM_BTN);
  ButtonConfig* btnConfig = armBtn->getButtonConfig();
  btnConfig->setEventHandler(handleEvent);

  pinMode(LAUNCH_BTN, INPUT_PULLUP);
  launchBtn = new AceButton(LAUNCH_BTN);
  btnConfig = launchBtn->getButtonConfig();
  btnConfig->setEventHandler(handleEvent);

  if (armBtn->isPressedRaw()) {
    Serial.println(F("armBtn was pressed while booting"));
  }
}

void initLedStrip() {
  pinMode(LEDSTRIP_PIN, OUTPUT);
  FastLED.addLeds<LED_TYPE,LEDSTRIP_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalPixelString); // initializes LED strip
  FastLED.setBrightness(BRIGHTNESS);// global brightness
  ledShow(CRGB::White);
  delay(100);
  ledShow(CRGB::Black);
}

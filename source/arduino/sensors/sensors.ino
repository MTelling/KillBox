#include <Wire.h>
#include <Adafruit_MPR121.h>

#define ENABLE_TILT
// #define ENABLE_MOTION
#define ENABLE_CAP
#define ENABLE_LID

#define STARTUP_DELAY 10 * 1000 // 10 secs
#define LID_CAP 15
#define BUZZER_TONE 440
#define LONG_BEEP 2
#define SHORT_BEEP 8

//#define DEBUG_LID
//#define DEBUG_RELAY
#define DEBUG_TRIGGERED

// constants won't change. They're used here to
// set pin numbers:
const int pin_tilt1 = 3;
const int pin_tilt2 = 5;
const int pin_motion = 2;
const int pin_relay = 6;
const int pin_lid_button = 4;
const int pin_buzzer = 5;
const int pin_cap = 3;

// set buzzer tones.
#ifdef DEBUG_TRIGGERED
int told_tilt = 0, told_motion = 0, told_cap = 0, told_lid = 0;
const int music_tilt[] = {SHORT_BEEP, SHORT_BEEP};
const int music_motion[] = {SHORT_BEEP, LONG_BEEP};
const int music_lid[] = {LONG_BEEP, LONG_BEEP};
const int music_cap[] = {LONG_BEEP, SHORT_BEEP};
const int music_start[] = {SHORT_BEEP, SHORT_BEEP, LONG_BEEP};
const int music_before_start[] = {SHORT_BEEP, SHORT_BEEP, SHORT_BEEP};
const int music_lid_put_on[] = {LONG_BEEP};
#endif

// set leds:
const int pin_led1 = 8;
const int pin_led2 = 10;
const int pin_led3 = 12;

const int pin_tilt_led = pin_led1;
const int pin_motion_led = pin_led2;
const int pin_cap_led = pin_led3;

//Capacity check limit. 
const int cap_limit = 1;

//Remember the last tilts
int tilt1_last;
int tilt2_last;

//Remember if sensors have been activated. 
int triggered_tilt1 = 0;
int triggered_tilt2 = 0;
int triggered_cap = 0;
int triggered_motion = 0;
int lid_is_open = 0;

//Setup the capacitive sensor object
Adafruit_MPR121 cap = Adafruit_MPR121();

void setup() {
  while (!Serial); // needed to keep leonardo/micro from starting too fast for cap. sensor.
  
  Serial.begin(9600);
  // initialize the pushbutton pin as an input:
  setupPins();

  //Read the first pins from the tilt sensors. 
  tilt1_last = digitalRead(pin_tilt1);
  tilt2_last = digitalRead(pin_tilt2);

  //Turn on relay.
  digitalWrite(pin_relay, HIGH);

  waitForReady();

  // Connect to capacity sensor. 
  setupCapacitySensor();
  delay(50);
}

void waitForReady() {
  // Wait for lid to close
  lid_is_open = 1;
  playTune(music_before_start, sizeof(music_before_start)/sizeof(music_before_start[0]));
  Serial.println("Waiting for lid");
  while (lid_is_open) {
    readLidButton();
    delay(100);
  }
  playTune(music_lid_put_on, sizeof(music_lid_put_on)/sizeof(music_lid_put_on[0]));
  initLED();
  Serial.println("Waiting startup delay");
  delay(STARTUP_DELAY);
  playTune(music_start, sizeof(music_start)/sizeof(music_start[0]));
  Serial.println("Lets go");
}

void initLED() {
  const int pause = 80;
  Serial.println("Flashing LEDs");
  digitalWrite(pin_led1, HIGH);
  delay(pause);
  digitalWrite(pin_led2, HIGH);
  delay(pause);
  digitalWrite(pin_led3, HIGH);
  delay(pause);
  digitalWrite(pin_led3, LOW);
  delay(pause);
  digitalWrite(pin_led2, LOW);
  delay(pause);
  digitalWrite(pin_led1, LOW);
  setLEDs();
}

void loop() {

  #ifdef ENABLE_TILT
  readTiltSensors();
  #endif

  #ifdef ENABLE_CAP
  readCapSensor();
  #endif

  #ifdef ENABLE_MOTION
  readMotionDetector();
  #endif 

  #ifdef ENABLE_LID
  readLidButton();
  #endif
  
  setLEDs();
  setRelayState();  
}

void setupPins() {
  pinMode(pin_tilt1, INPUT);
  pinMode(pin_tilt2, INPUT);
  pinMode(pin_motion, INPUT);
  pinMode(pin_lid_button, INPUT);
  
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_tilt_led, OUTPUT);
  pinMode(pin_motion_led, OUTPUT);
  pinMode(pin_cap_led, OUTPUT);
  pinMode(pin_buzzer, OUTPUT);
}

void readLidButton() {
  lid_is_open = !digitalRead(pin_lid_button);

  #ifdef DEBUG_LID
  Serial.println(digitalRead(pin_lid_button));
  //Serial.println(lid_button_count);
  #endif
}

void setupCapacitySensor() {
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
}

void setRelayState() {
  // Toggles relay to on or off. 
  if (triggered_motion 
  || lid_is_open 
  || triggered_cap
  || triggered_tilt1
  || triggered_tilt2) {
    digitalWrite(pin_relay, LOW);
    #ifdef DEBUG_RELAY
    Serial.println("Cutting power!");
    #endif
  }

  #ifdef DEBUG_TRIGGERED
  if (!told_motion && triggered_motion) {
    Serial.println("Triggered motion!");
    told_motion = 1;
    playTune(music_motion, sizeof(music_motion)/sizeof(music_motion[0]));
  }
  if (!told_lid && lid_is_open) {
    Serial.println("Triggered lid!");
    told_lid = 1;
    playTune(music_lid, sizeof(music_lid)/sizeof(music_lid[0]));
  }
  if (!told_cap && triggered_cap) {
    Serial.println("Triggered cap!");
    told_cap = 1;
    playTune(music_cap, sizeof(music_cap)/sizeof(music_cap[0]));
  }
  if (!told_tilt && (triggered_tilt1 || triggered_tilt2)) {
    Serial.println("Triggered tilt!");
    told_tilt = 1;
    playTune(music_tilt, sizeof(music_tilt)/sizeof(music_tilt[0]));
  }
  #endif

}

void playTune(const int* noteDurations, int size) {
 
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < size; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(pin_buzzer, BUZZER_TONE, noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(pin_buzzer);
  }
}

void readCapSensor() {
  //Serial.println(cap.touched());
  // if (cap.touched() > cap_limit) {
  //   triggered_cap = 1;
  // }
  int base = cap.baselineData(pin_cap);
  int filter = cap.filteredData(pin_cap);
  if ((base - filter) > (base / 7.0) || cap.touched()) {
    // #ifdef DEBUG
    Serial.println("Cap, debug:");
    Serial.println(base / 7.0);
    Serial.println(filter);
    Serial.println(base);
    Serial.println("\n");
    // #endif
    triggered_cap = 1;
  }
}

void readTiltSensors() {
  //Read tilt data. 
  int tilt1_current = digitalRead(pin_tilt1);
  int tilt2_current = digitalRead(pin_tilt2);

  //Remember if a sensor was tilted. 
  if (tilt1_current != tilt1_last) {
    triggered_tilt1 = 1;
  }
  if (tilt2_current != tilt2_last) {
    triggered_tilt2 = 1;
  }

  //Save last value. 
  tilt1_last = tilt1_current;
  tilt2_last = tilt2_current;
 
}

void readMotionDetector() {
  //Set the motion flag to 1 if triggered. 
  triggered_motion = triggered_motion || digitalRead(pin_motion) & 1;
}


void setLEDs() {
  
  digitalWrite(pin_motion_led, triggered_motion);
  digitalWrite(pin_cap_led, triggered_cap);
  
  if (triggered_tilt1 || triggered_tilt2) digitalWrite(pin_tilt_led, HIGH);
  else digitalWrite(pin_tilt_led, LOW);
  
}

void resetFlags() {
  triggered_tilt1 = 0;
  triggered_tilt2 = 0;
  triggered_cap = 0;
  triggered_motion = 0;
}
  


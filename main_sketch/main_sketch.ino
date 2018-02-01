// ------------------------------
// INCLUDES
// ------------------------------
#include<TimerOne.h>

// ------------------------------
// PIN DEFINITIONS
// ------------------------------
#define ledPin 13

#define distanceEchoPin 11
#define distanceTrigPin 12


// ------------------------------
// GLOBS
// ------------------------------
volatile int distance = 0;


// ------------------------------
// EVENT DEFINITIONS
// ------------------------------
#define DOOR_OPEN 0


// ------------------------------
// ISRs
// ------------------------------
void isr_read_distance() {
  // Invert LED state.
  digitalWrite(ledPin, !digitalRead(ledPin));
  
  digitalWrite(distanceTrigPin, LOW);
  delayMicroseconds(2);
  // Ping
  digitalWrite(distanceTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distanceTrigPin, LOW);
  // Read
  int echo_timing = pulseIn(distanceEchoPin, HIGH);
  distance = echo_timing / 58;
}


// ------------------------------
// Setup
// ------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(distanceEchoPin, INPUT);
  pinMode(distanceTrigPin, OUTPUT);

  Timer1.initialize();
  Timer1.setPeriod(1000000); // The timeout period is set in microseconds where 100 000 = 0.1 seconds
  Timer1.attachInterrupt( isr_read_distance );
}


// ------------------------------
// Main loop
// ------------------------------
void loop() {
  delay(1000);
  check_distance();
}


// ------------------------------
// Utility functions
// ------------------------------
void check_distance() {
  if (distance < 5) {
    send_message(DOOR_OPEN);
  }
}


void send_message(int msg) {
  switch (msg) {
    case DOOR_OPEN:
      Serial.println(DOOR_OPEN);
      break;
    default:
      break;
  }
}


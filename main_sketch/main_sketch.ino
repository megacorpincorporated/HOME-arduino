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

char buffer[10];

boolean ALARM_RAISED = false;


// ------------------------------
// EVENT DEFINITIONS
// ------------------------------

// GENERAL
#define OFF 0
#define ON 1
#define ERR 666

// OUT
#define PROXIMITY_ALARM 0

// IN
#define GET_DISTANCE 10
#define DEACTIVATE_ALARM 11


// ------------------------------
// ISRs
// ------------------------------
void isr_read_distance() {
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
// Utility functions
// ------------------------------
void check_input() {
  if (Serial.available()) {
    Serial.readBytes(buffer, Serial.available());
    incoming_command(buffer);
    // Reset reading buffer.
    memset(&buffer[0], 0, sizeof(buffer));
  }
}


void incoming_command(char *cmd) {
  Serial.println("Incoming command:");
  Serial.println(cmd);

  int main = atoi(&cmd[0]);
  int sub = 0;

  if (&cmd[2] > 0) {
    sub = atoi(&cmd[2]);
  }
  execute_command(&main, &sub);
}


void execute_command(int *main, int *sub) {
  switch (*main) {
    case PROXIMITY_ALARM:
      alarm_callback(*sub);
      break;
    default:
      Serial.println("ERROR >> command not identified!");
      break;
  }
}


void alarm_callback(int state) {
  switch (state) {
    case ON:
      ALARM_RAISED = true;
      digitalWrite(ledPin, HIGH);
      Serial.println("Alarm is activated!");
      break;
    case OFF:
      ALARM_RAISED = false;
      digitalWrite(ledPin, LOW);
      Serial.println("Alarm is deactivated!");
      break;
    default:
      Serial.println("ERROR >> Could not read alarm sub cause!");
      break;
  }
}


void check_distance() {
  if (distance < 5 && !ALARM_RAISED) {
    send_message(PROXIMITY_ALARM, ON);
  } else if (distance > 5 && ALARM_RAISED) {
    send_message(PROXIMITY_ALARM, OFF);
  }
}


void send_message(int msg, int sub) {
  Serial.println();
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
  Timer1.setPeriod(100000); // The timeout period is set in microseconds where 100 000 = 0.1 seconds
  Timer1.attachInterrupt( isr_read_distance );
}


// ------------------------------
// Main loop
// ------------------------------
void loop() {
  delay(1000);
  check_input();
  check_distance();
}

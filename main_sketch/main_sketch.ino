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
volatile unsigned int pos = 0;
volatile unsigned int samples[10];

char buffer[50];

boolean ALARM_RAISED = false;


// ------------------------------
// EVENT DEFINITIONS
// ------------------------------

// GENERAL (sub cause codes)
#define OFF 0
#define ON 1
#define ERR 9

// OUT
#define PROXIMITY_ALARM 0

// IN
#define GET_ALARM_STATUS 1


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
  int distance = echo_timing / 58;
  // Discard negative values as the read was distorted.
  if (distance > 0) {
    samples[pos] = distance;
    pos++;
  }
  if (pos == 10) {
    pos = 0;
  }
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
  int main = cmd[0] - '0';
  int sub = 9; // ERR

  if ((cmd[2] - '0') > -1) {
    sub = cmd[2] - '0';
  }
  execute_command(&main, &sub);
}


void execute_command(int *main, int *sub) {
  switch (*main) {
    case PROXIMITY_ALARM:
      alarm_callback(*sub);
      break;
    case GET_ALARM_STATUS:
      send_alarm_status();
      break;
    default:
      Serial.println(ERR);
      break;
  }
}


int parse_samples() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += samples[i];
  }

  return total / 10;
}


void send_alarm_status() {
  
  int alarm_status;
  if (ALARM_RAISED) {
    alarm_status = ON;
  } else {
    alarm_status = OFF;
  }
  send_message(GET_ALARM_STATUS, alarm_status);
}


void alarm_callback(int state) {
  switch (state) {
    case ON:
      ALARM_RAISED = true;
      digitalWrite(ledPin, HIGH);
      break;
    case OFF:
      ALARM_RAISED = false;
      digitalWrite(ledPin, LOW);
      break;
    default:
      Serial.println(ERR);
      break;
  }
}


void check_distance() {
  if (parse_samples() > 5 && !ALARM_RAISED) {
    send_message(PROXIMITY_ALARM, ON);
  } else if (parse_samples() < 5 && ALARM_RAISED) {
    send_message(PROXIMITY_ALARM, OFF);
  }
}


void send_message(int main, int sub) {
  char m, s, f;
  m = main + '0';
  s = sub + '0';
  f = ' ';
  char message[4] = {m, f, s}; // including string terminator!
  Serial.println(message);
}


// ------------------------------
// Setup
// ------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {} // Wait for leonardo
  delay(600); // wait for serial initialisation

  if (Serial.available()) {
      Serial.readBytes(buffer, Serial.available()); // attempt to read any trash available
  }

  memset(&buffer[0], 0, sizeof(buffer));
  
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
  delay(250);
  check_input();
  check_distance();
}

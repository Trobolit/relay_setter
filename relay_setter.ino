#include <elapsedMillis.h>

#define BAUDRATE 9600 // Serial baudrate
#define SIZE 6 // Size of incoming data in bytes

#define number_of_relays 4
#define relay_1_pin 2
#define relay_2_pin 3
#define relay_3_pin 4
#define relay_4_pin 5
#define relay_1_index 0
#define relay_2_index 1
#define relay_3_index 2
#define relay_4_index 3

void changeAllRelayStates(bool state);
void updateAllRelays();
void blinks(int n, int d);

const byte relay_pins[] = {relay_1_pin,relay_2_pin,relay_3_pin,relay_4_pin};
bool relay_states[] = {false,false,false,false};

// Serial data in will be 6 bytes: SOM, R1,R2,R3,R4, EOM
// where: Ri = 0 means off, 1 on.
// special bytes:
// SOM (start of message): 250, EOM (end of message): 251

// Byte array to hold incoming serial data
byte incoming_bytes[SIZE];

const int timeout_ms = 500;

// Timer for communications problems check
elapsedMillis timer0;

void setup(void)
{
  // Blink to show that arduino is starting as expected.
  pinMode(LED_BUILTIN, OUTPUT);
  blinks(5, 100);
  
  // Initialize relay pins
  for(int i=0;i<number_of_relays;i++){
    pinMode(relay_pins[i],OUTPUT);
  }
  // Initialize serial
  Serial.begin(BAUDRATE);
}

bool badMessage = false;

void loop(void)
{
  // Clear bytes array
  for (int i = 0; i < SIZE; i++) {
    incoming_bytes[i] = 0;
  }

  // Check for connection problems, if nothing in for more than .5 secs turn off all relays.
  while (true) {
    if (timer0 > timeout_ms) { // check that we haven't run out of time.
      timer0 = 0;
      changeAllRelayStates(false);
      updateAllRelays();
    }
    if(Serial.available() > 0){ // search for SOM
      if(Serial.read() == 250){
        incoming_bytes[0] = 250;
        break;
      } 
    }
  }

  // We now have SOM. Wait for complete message to arrive.
  while (Serial.available() < SIZE-1){
    if (timer0 > timeout_ms) { // check that we haven't run out of time.
      timer0 = 0;
      changeAllRelayStates(false);
      updateAllRelays();
    }
  }
  // Buffer ready to parse!
  for (int i = 1; i < SIZE; i++) {
      incoming_bytes[i] = Serial.read();
  }

  // If serial data looks ok, fill data array
  if (incoming_bytes[SIZE - 1] == 251) {
      for(int i=0;i<number_of_relays;i++){
        if(incoming_bytes[i+1] == 0)          // offset because of SOM
          relay_states[i] = false;
        else if(incoming_bytes[i+1] == 1)     // offset because of SOM
          relay_states[i] = true;
        else{                                 // part of message is bad, force all to off.
          changeAllRelayStates(false);        // then break out of loop to update pins.
          break;
        }
      }
      blinks(3, 50);
      updateAllRelays();

  } else {
    //turn off all relays, message bad.
    changeAllRelayStates(false);
    updateAllRelays();
  }
  timer0 = 0;
}


void blinks(int n, int d) {
  for(int i=0;i<n;i++){
      digitalWrite(LED_BUILTIN, HIGH);
      delay(d);
      digitalWrite(LED_BUILTIN, LOW);
      delay(d);
  }
}

void changeAllRelayStates(bool state){
  //blinks(1, 300);
  for(int i=0;i<number_of_relays;i++){
    relay_states[i]=state;
  }
}

void updateAllRelays() {
  ///blinks(3, 50);
  for(int i=0;i<number_of_relays;i++){
    if(relay_states[i])
      digitalWrite(relay_pins[i], HIGH);
    else
      digitalWrite(relay_pins[i], LOW);
  }
}

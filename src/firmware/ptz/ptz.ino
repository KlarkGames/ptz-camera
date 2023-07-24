#define LEFT 1
#define RIGHT 0
#define UP 1
#define DOWN 0

const int enPin=8;
const int stepXPin = 2; //X.STEP
const int dirXPin = 5; // X.DIR
const int stepYPin = 3; //Y.STEP
const int dirYPin = 6; // Y.DIR
const int stepZPin = 4; //Z.STEP
const int dirZPin = 7; // Z.DIR

const byte numChars = 4;
char receivedChars[numChars];
static byte index = 0;

byte horizontal_dir = 0;
byte horizontal_power = 0;
byte vertical_dir = 0;
byte vertical_power = 0;

char s;

int millisBtwnSteps = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  pinMode(stepXPin, OUTPUT);
  pinMode(dirXPin, OUTPUT);
  pinMode(stepYPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
 
  Serial.println(F("CNC Shield Initialized"));
}

void writeRegisters() {
  if (horizontal_power != 0) {
    if (horizontal_dir == 0) {
      PORTD &= ~_BV(PD5);
    } else {
      PORTD |= _BV(PD5);
    }
    PORTD |= _BV(PD2);
  } else {
    PORTD &= ~_BV(PD2);
    PORTD &= ~_BV(PD5);
  }

  if (vertical_power != 0) {
    if (vertical_dir == 0) {
      PORTD &= ~_BV(PD6);
    } else {
      PORTD |= _BV(PD6);
    }
    PORTD |= _BV(PD3);
  } else {
    PORTD &= ~_BV(PD3);
    PORTD &= ~_BV(PD6);
  }
}

void rotateFunc() {
  writeRegisters();
  delayMicroseconds(millisBtwnSteps);
  PORTD &= ~_BV(PD2);
  PORTD &= ~_BV(PD3);
  delayMicroseconds(millisBtwnSteps);
}


// Example message: "UL" 
void loop() {
  if (index == numChars) {
    index = 0;
    horizontal_dir = (byte)receivedChars[0] - 48;
    horizontal_power = (byte)receivedChars[1] - 48;
    vertical_dir = (byte)receivedChars[2] - 48;
    vertical_power = (byte)receivedChars[3] - 48;
  }
  
  if (Serial.available() > 0) {
    s = Serial.read();    

    if (s != '\n' && (int)s != -1 && s != '\0') {
      receivedChars[index] = s;
      index++;
    }
  } 
  
  rotateFunc();
}

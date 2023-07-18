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

bool left = false;
bool right = false;
bool up = false;
bool down = false;

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

void rotateFunc(int stepPin) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(millisBtwnSteps);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(millisBtwnSteps);
}


// Example message: "UL" 
void loop() {
  if (Serial.available() > 0) {

    String line = Serial.readString();
    int direction = line.charAt(0);
    int command = line.charAt(1);

    int value = (command == 'L') ? true : false;

    switch(direction)
    {
      case 'L': {
        left = value;
        if (value == true) right = false;
        break;
      }
      case 'R': {
        right = value;
        if (value == true) left = false;
        break;
      }
      case 'U': {
        up = value;
        if (value == true) down = false;
        break;
      }
      case 'D': {
        down = value;
        if (value == true) up = false;
        break;
      }
    }

} else {
    if (left == true) {
      digitalWrite(dirXPin, LEFT);
      rotateFunc(stepXPin);
    }
    if (right == true) {
      digitalWrite(dirXPin, RIGHT);
      rotateFunc(stepXPin);
    }
    if (up == true) {
      digitalWrite(dirYPin, UP);
      rotateFunc(stepYPin);
    }
    if (down == true) {
      digitalWrite(dirYPin, DOWN);
      rotateFunc(stepYPin);
    }
  }
}

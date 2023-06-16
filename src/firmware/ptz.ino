const int enPin=8;
const int stepXPin = 2; //X.STEP
const int dirXPin = 5; // X.DIR
const int stepYPin = 3; //Y.STEP
const int dirYPin = 6; // Y.DIR
const int stepZPin = 4; //Z.STEP
const int dirZPin = 7; // Z.DIR

const char separator = ':';
int pulseWidthMicros = 100;  // microseconds
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

boolean isValidNumber(String str){
  for(byte i=0;i<str.length();i++)
  {
    if(isDigit(str.charAt(i))) return true;
  }

  return false;
}

int getValue(String data, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }

    String strValue = data.substring(strIndex[0], strIndex[1]);
    bool isNumericString = isValidNumber(strValue);
    Serial.println("Value:" + strValue + ".");
    Serial.println("IsNumericString:" + isNumericString);
    return found && isNumericString > index ? strValue.toInt() : -1;
}

void rotateFunc(int stepPin) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(millisBtwnSteps);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(millisBtwnSteps);
}

void rotate(int dirPin, int stepPin, int direction, int steps) {
  Serial.println(F("Running clockwise"));
  digitalWrite(dirPin, direction); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for (int i = 0; i < steps; i++) {
    rotateFunc(stepPin);
  }
  delay(1000); // One second delay
}

// Example message: "5 2 1 200" 
void loop() {
  if (Serial.available() > 0) {
    String dirPinValue = Serial.readStringUntil(' ');
    if (!isValidNumber(dirPinValue)){
      Serial.println("Direction pin value is not a number: " + dirPinValue + ".");
      return;
    }
    Serial.read();
    int dirPin = dirPinValue.toInt();

    String stepPinValue = Serial.readStringUntil(' ');
    if (!isValidNumber(stepPinValue)){
      Serial.println("Step pin value is not a number: " + stepPinValue + ".");
      return;
    }
    Serial.read();
    int stepPin = stepPinValue.toInt();

    String directionValue = Serial.readStringUntil(' ');
    if (!isValidNumber(directionValue)){
      Serial.println("Direction value is not a number: " + directionValue + ".");
      return;
    }
    Serial.read();
    int direction = directionValue.toInt();

    String stepsValue = Serial.readStringUntil(' ');
    if (!isValidNumber(stepsValue)){
      Serial.println("Steps value is not a number: " + stepsValue + ".");
      return;
    }
    Serial.read();
    int steps = stepsValue.toInt();

    Serial.println(dirPin);
    Serial.println(stepPin);
    Serial.println(direction);
    Serial.println(steps);

    rotate(dirPin, stepPin, (direction==0)?HIGH:LOW, steps);
  }
}
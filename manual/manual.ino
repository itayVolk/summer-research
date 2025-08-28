const int pins[2] = {13, 12};

void setup() {
    Serial.begin(9600);
    for (int j = 0; j < 2; j++) {
      pinMode(pins[j], OUTPUT);
    }
}

// rotate the motor
void move(int dir, int step, int motor[2]) {
  digitalWrite(motor[1],dir); // Enables the motor to move in a particular direction
  for(int x = 0; x < step; x++) {
    digitalWrite(motor[0],HIGH); 
    delayMicroseconds(500); 
    digitalWrite(motor[0],LOW); 
    delayMicroseconds(500); 
  }
}

int dir = 0;
void loop() {
    if (Serial.available()) {
        char cur;
        do {
            cur = Serial.read();
        } while(cur == '\n' || cur == 0xFFFFFFFF);
        switch(cur) {
            case 'u':
                dir = 1;
                break;
            case 'd':
                dir = -1;
                break;
            default:
                dir = 0;
        }
    }
    if (dir) {
        move((dir+1)/2, 200, pins);
    }
}
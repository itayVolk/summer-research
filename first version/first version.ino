const int pins[3][2] = {{13, 12}, {11, 10}, {9, 8}};
const int sensors[3] = {3, 4, 5};
const int clock = 2;

long * lengths;
int index = 0;

void setup() {
  Serial.begin(9600);
  pinMode(clock, OUTPUT);
  lengths = calloc(sizeof(sensors)/sizeof(int), sizeof(long));
  for (int i = 0; i < sizeof(sensors)/sizeof(int); i++) {
    for (int j = 0; j < 2; j++) {
      pinMode(pins[i][j], OUTPUT);
    }
    pinMode(sensors[i], INPUT);
    lengths[i] = 0;
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

long* read(int pins[], int count) {
  long * out = calloc(count, sizeof(long));

  //read 24 bits
  for (int i = 0; i < 24; i++) {
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
    for (int i = 0; i < count; i++) {
      out[i] = out[i]*2 + digitalRead(sensors[i]);
    }
  }

  // convert the binary format
  for (int i = 0; i < count; i++) {
    out[i] ^= 0x800000;
  }

  for (char i = 0; i < 3; i++) {
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
  }
  return out;
}

const int step = 200;
const int thres = 100;

void loop() {
  while(Serial.available()) {
    long val = 0;
    char cur = Serial.read();
    while (cur != '\n') {
      val = val*10 + cur - '0';
      cur = Serial.read();
    }
    lengths[index++] = val;
    Serial.println(lengths[index-1]);
    if (index == sizeof(sensors)/sizeof(int)) {
      index = 0;
    }
  }

  long * out = read(sensors, sizeof(sensors)/sizeof(int));
  for (int i = 0; i < sizeof(sensors)/sizeof(int); i++) {
    if (!lengths[i]) {
      delay(250);
      break;
    }

    if (out[i] > lengths[i]+thres) {
      move(LOW, step, pins[i]);
    } else if (out[i] < lengths[i]-thres) {
      move(HIGH, step, pins[i]);
    } else {
      delay(250);
    }
  }
}
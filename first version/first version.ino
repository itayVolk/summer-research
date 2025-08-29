const int pins[1][2] = {{13, 12}};
const int sensors[1] = {3};
const int clock = 2;

long * lengths;
long * prev;
int index = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("on");
  pinMode(clock, OUTPUT);
  lengths = (long *)calloc(sizeof(sensors)/sizeof(int), sizeof(long));
  prev = (long *)calloc(sizeof(sensors)/sizeof(int), sizeof(long));
  for (int i = 0; i < sizeof(sensors)/sizeof(int); i++) {
    for (int j = 0; j < 2; j++) {
      pinMode(pins[i][j], OUTPUT);
    }
    pinMode(sensors[i], INPUT);
    lengths[i] = 0;
  }
}

// rotate the motor
void move(int dir, int step, const int motor[2]) {
  digitalWrite(motor[1],dir); // Enables the motor to move in a particular direction
  for(int x = 0; x < step; x++) {
    digitalWrite(motor[0],HIGH); 
    delayMicroseconds(500); 
    digitalWrite(motor[0],LOW); 
    delayMicroseconds(500); 
  }
}

long* read(const int pins[], int count) {
  long * out = (long *)calloc(count, sizeof(long));

  //read 24 bits
  for (int j = 0; j < 24; j++) {
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
    for (int i = 0; i < count; i++) {
      out[i] = out[i]*2 + digitalRead(sensors[i]);
    }
  }

  // convert the binary format
  for (int i = 0; i < count; i++) {
    out[i] ^= 0x800000;
    if (out[i] == 8388608) {
      out[i] = prev[i];
    }
  }

  for (char i = 0; i < 3; i++) {
    digitalWrite(clock, HIGH);
    digitalWrite(clock, LOW);
  }
  prev = out;
  return out;
}

const int step = 200;
const int thres = 500;

void loop() {
  long * out = read(sensors, sizeof(sensors)/sizeof(int));
  while(Serial.available()) {
    long val = 0;
    char cur = Serial.read();
    while (cur != '\n') {
      if (cur) {
        val = val*10 + cur - '0';
      }
      cur = Serial.read();
    }
    lengths[index++] = val;
    Serial.println(lengths[index-1]);
    Serial.println(out[index-1]);
    if (index == sizeof(sensors)/sizeof(int)) {
      index = 0;
    }
  }

  for (int i = 0; i < sizeof(sensors)/sizeof(int); i++) {
    // Serial.println(out[i]);
    if (!lengths[i] || !out[i]) {
      delay(50);
      break;
    }
    // Serial.println(lengths[i]);

    if (out[i] > lengths[i]+thres) {
      // Serial.println("for");
      move(LOW, step, pins[i]);
    } else if (out[i] < lengths[i]-thres) {
      // Serial.println("back");
      move(HIGH, step, pins[i]);
    }
  }
}
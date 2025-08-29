const int motorsData = 13;
const int motorsEn = 12;
const int motorsLatch = 11;
const int motorsClock = 10;

const int sensorsData = 2;
const int sensorsEn = 3;
const int sensorsLatch = 4;
const int sensorsClock = 5;
const int data = 6;

const int count = 2;
const int step = 200;
const int thres = 100;

long * lengths = (long *) calloc(count*3, sizeof(long));
int index = 0;

void setup() {
    Serial.begin(9600);
    pinMode(motorsData, OUTPUT);
    pinMode(motorsEn, OUTPUT);
    digitalWrite(motorsEn, HIGH);
    pinMode(motorsLatch, OUTPUT);
    pinMode(motorsClock, OUTPUT);

    pinMode(sensorsData, OUTPUT);
    pinMode(sensorsEn, OUTPUT);
    digitalWrite(sensorsEn, HIGH);
    pinMode(sensorsLatch, OUTPUT);
    pinMode(sensorsClock, OUTPUT);
    pinMode(data, INPUT);
}

// rotate the motor
void move(int dirs[], int steps[]) {
    for (int x = 1; x <= step; x++) {
        int pattern = 1;
        for (int i = 0; i < count*3; i++) {
          pattern = pattern*4 + !(x%steps[i])*2 + dirs[i];
          if (i%3 == 2) {
              digitalWrite(motorsLatch, LOW);
              shiftOut(motorsData, motorsClock, MSBFIRST , pattern*2);
              digitalWrite(motorsLatch, HIGH);
              delayMicroseconds(50);
              pattern = 1;
          }
        }
        digitalWrite(motorsEn, LOW);
        delayMicroseconds(250);
        digitalWrite(motorsEn,HIGH);
        delayMicroseconds(250);
    }
}

long* read() {
    long * out = (long *) calloc(count*3, sizeof(long));
    digitalWrite(sensorsLatch, LOW);
    digitalWrite(sensorsData, HIGH);
    digitalWrite(sensorsClock, HIGH);
    digitalWrite(sensorsClock, LOW);
    digitalWrite(sensorsData, LOW);
    digitalWrite(sensorsLatch, HIGH);

    for (int i = 0; i < count*3; i++) {
        //read 24 bits
        for (int j = 0; j < 24; j++) {
            digitalWrite(sensorsEn, HIGH);
            digitalWrite(sensorsEn, LOW);
            out[i] = out[i]*2 + digitalRead(data);
        }

        // convert the binary format
        out[i] ^= 0x800000;

        // prep for next read
        for (int j = 0; j < 3; j++) {
            digitalWrite(sensorsEn, HIGH);
            digitalWrite(sensorsEn, LOW);
        }

        //move sensor
        digitalWrite(sensorsLatch, LOW);
        digitalWrite(sensorsClock, HIGH);
        digitalWrite(sensorsClock, LOW);
        digitalWrite(sensorsLatch, HIGH);
    }
    return out;
}

void loop() {
    long * out = read();
    while (Serial.available()) {
        long val = 0;
        char cur = Serial.read();
        while (cur != '\n') {
            if (cur) {
                val = val*10 + cur - '0';
            }
            cur = Serial.read();
        }
        lengths[index++] = val;
        Serial.println(val);
        if (index == count*3) {
            index = 0;
        }
    }

    
    int dirs[count*3];
    int steps[count*3];
    for (int i = 0; i < count*3; i++) {
        steps[i] = step+1;
        if (!lengths[i]) {
            dirs[i] = 0;
            continue;
        }

        if (out[i] > lengths[i]+thres) {
            dirs[i] = LOW;
            steps[i] = 1;
        } else if (out[i] < lengths[i]-thres) {
            dirs[i] = HIGH;
            steps[i] = 1;
        }
    }
    move(dirs, steps);
}
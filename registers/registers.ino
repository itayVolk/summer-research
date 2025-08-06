const int motorsData = 13;
const int motorsEn = 12;
const int motorsLatch = 11;
const int motorsClock = 10;
const int clocks[4] = {4, 3, 2, 1};
const int clockEn = 5;
const int data = 6;

const int count = 3;
const int step = 200;
const int thres = 100;

long * lengths = calloc(count, sizeof(long));
int index = 0;

void setup() {
    Serial.begin(9600);
    pinMode(motorsData, OUTPUT);
    pinMode(motorsEn, OUTPUT);
    digitalWrite(motorsEn, HIGH);
    pinMode(motorsLatch, OUTPUT);
    pinMode(motorsClock, OUTPUT);

    pinMode(clockEn, OUTPUT);
    pinMode(data, INPUT);
    for (int i = 0; i < 4; i++) {
        pinMode(clocks[i], OUTPUT);
    }

    long * out = read();
    int dirs[count];
    int steps[count];
    for (int i = 0; i < count; i++) {
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

// rotate the motor
void move(int dirs[], int steps[]) {
    for (int x = 0; x < step; x++) {
        int pattern = 1;
        for (int i = 0; i < count; i++) {
        pattern = pattern*4 + dirs[i]*2 + !(x%steps[i]);
        if (i%3 == 2) {
            Serial.println(pattern*2, BIN);
            digitalWrite(motorsLatch, LOW);
            shiftOut(motorsData, motorsClock, MSBFIRST , pattern*2);
            digitalWrite(motorsLatch, HIGH);
            pattern = 1;
        }
        }

        digitalWrite(motorsEn, LOW);
        // Serial.println("on");
        delayMicroseconds(250); 
        digitalWrite(motorsEn,HIGH);
        // Serial.println("off");
        delayMicroseconds(250);
    }
}

long* read() {
    long * out = calloc(count, sizeof(long));

    for (int i = 0; i < count; i++) {
            //select which sensor we're on
            int temp = i+1;
            for (int j = 0; j < 4; j++) {
            digitalWrite(clocks[j], temp%2);
            temp /= 2;
        }

        //read 24 bits
        for (int i = 0; i < 24; i++) {
            digitalWrite(clockEn, HIGH);
            digitalWrite(clockEn, LOW);
            for (int i = 0; i < count; i++) {
                out[i] = out[i]*2 + digitalRead(data);
            }
        }

        // convert the binary format
        for (int i = 0; i < count; i++) {
            out[i] ^= 0x800000;
        }

        // prep for next read
        for (char i = 0; i < 3; i++) {
            digitalWrite(clockEn, HIGH);
            digitalWrite(clockEn, LOW);
        }
    }
    return out;
}

void loop() {
    while (Serial.available()) {
        long val = 0;
        char cur = Serial.read();
        while (cur != '\n') {
            if (cur != 0xFFFFFFFF) {
                val = val*10 + cur - '0';
            }
            cur = Serial.read();
        }
        lengths[index++] = val;
        Serial.println(lengths[index - 1]);
        if (index == count) {
            index = 0;
        }
    }

    long * out = read();
    int dirs[count];
    int steps[count];
    for (int i = 0; i < count; i++) {
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
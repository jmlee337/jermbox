// I/O
#define LED       13
#define PIN       23

// Right side buttons
#define S_START   0
#define S_L       1
#define S_Y       2
#define S_R       3
#define S_TILT    4
#define S_B       5
#define S_A       6
#define S_X       7
#define S_Z       8
#define S_CUP     9
#define S_CLEFT   10
#define S_CDOWN   11
#define S_CRIGHT  12

// Left side buttons
#define S_TAUNT   22
#define S_AUP     21
#define S_ALEFT   20
#define S_ADOWN   19
#define S_ARIGHT  18
#define S_TUP     17
#define S_TLEFT   16
#define S_TDOWN   15
#define S_TRIGHT  14

// Bitmasks
#define bit0(x) x & (1 << 0)
#define bit1(x) x & (1 << 1)
#define bit2(x) x & (1 << 2)
#define bit3(x) x & (1 << 3)
#define bit4(x) x & (1 << 4)
#define bit5(x) x & (1 << 5)
#define bit6(x) x & (1 << 6)
#define bit7(x) x & (1 << 7)

// more bitmasks
#define setBit0(x) x | (1 << 0)
#define setBit1(x) x | (1 << 1)
#define setBit2(x) x | (1 << 2)
#define setBit3(x) x | (1 << 3)
#define setBit4(x) x | (1 << 4)
#define setBit5(x) x | (1 << 5)
#define setBit6(x) x | (1 << 6)
#define setBit7(x) x | (1 << 7)

// Minimum analog stick axis value
#define A_MIN 48

// Middle analog stick axis value
#define A_MID 128

// Maximum analog stick axis value
#define A_MAX 208

// The lightest shield value
#define A_TRIGGER 43

// Smallest tilt offset possible
#define O_MIN 25

// Middle analog offset
// Jump/Shield drop on y-axis
// Walk/tilt on x-axis
#define O_MID 53 // 53, 54, or 55

// Max analog offset
// basically only used for angles
#define O_MAX 70

// Period length is 5 us
// 72 MHz * 2.5 us
#define CYCLES_HALF 180

// Period length is 5 us
// 72 MHz * 10 us
#define CYCLES_TWO 720

bool passedAll = false;

uint32_t fallTime = 0;
uint32_t buffer = 0;
uint8_t bitsRead = 0;

void setup() {
  cli();
  pinMode(PIN, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  pinMode(S_START, INPUT_PULLUP);
  pinMode(S_L, INPUT_PULLUP);
  pinMode(S_Y, INPUT_PULLUP);
  pinMode(S_R, INPUT_PULLUP);
  pinMode(S_TILT, INPUT_PULLUP);
  pinMode(S_B, INPUT_PULLUP);
  pinMode(S_A, INPUT_PULLUP);
  pinMode(S_X, INPUT_PULLUP);
  pinMode(S_Z, INPUT_PULLUP);
  pinMode(S_CUP, INPUT_PULLUP);
  pinMode(S_CLEFT, INPUT_PULLUP);
  pinMode(S_CDOWN, INPUT_PULLUP);
  pinMode(S_CRIGHT, INPUT_PULLUP);

  pinMode(S_TAUNT, INPUT_PULLUP);
  pinMode(S_AUP, INPUT_PULLUP);
  pinMode(S_ALEFT, INPUT_PULLUP);
  pinMode(S_ADOWN, INPUT_PULLUP);
  pinMode(S_ARIGHT, INPUT_PULLUP);
  pinMode(S_TUP, INPUT_PULLUP);
  pinMode(S_TLEFT, INPUT_PULLUP);
  pinMode(S_TDOWN, INPUT_PULLUP);
  pinMode(S_TRIGHT, INPUT_PULLUP);

  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}

void loop() {
  while (1) {
    // Wait for falling edge
    while (digitalReadFast(PIN) == HIGH);
    uint32_t newFallTime = ARM_DWT_CYCCNT;
    if (newFallTime - fallTime > CYCLES_TWO) {
      // start of new command
      buffer = 0;
      bitsRead = 0;
    }
    fallTime = newFallTime;

    // Wait for rising edge
    while (digitalReadFast(PIN) == LOW);
    uint32_t cycles = ARM_DWT_CYCCNT - fallTime;
    buffer = (buffer << 1) | (cycles > CYCLES_HALF ? 0 : 1);
    bitsRead++;
    if (bitsRead == 9) {
      if (buffer & 0x1) {
        handleShort(buffer >> 1);
      }
    } else if (bitsRead == 25) {
      if (buffer & 0x1) {
        handleLong(buffer >> 1);
      }
    }
  }
}

void handleShort(uint32_t buffer) {
  if (buffer == 0x0) {
    respondWithId();
  } else if (buffer == 0x41) {
    respondWithOrigins();
  }
}

void handleLong(uint32_t buffer) {
  // the bottom two bits of the command control rumble.
  // mask them out cuz we don't care, but for reference:
  // XXXXXX00, brake off, motor off
  // XXXXXX01, brake off, motor on
  // XXXXXX10, brake on, motor off
  // XXXXXX11, brake on, motor on? idk
  buffer &= 0xFFFFFFFC;

  if (buffer == 0x00400000) {
    respondWithCalibration();
  } else {
    // Technically, we should check for the right commands
    // before responding, but we don't actually know what
    // all the commands are
    if (!passedAll) {
      digitalWriteFast(LED, HIGH);
      passedAll = true;
    }
    respondWithData();
  }
}

void respondWithId() {
  pinMode(PIN, OUTPUT);
  digitalWriteFast(PIN, HIGH);

  // 0x09 0x00 0x00 STOP
  send0(); send0(); send0(); send0(); send1(); send0(); send0(); send1();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1();

  pinMode(PIN, INPUT);
}

void respondWithOrigins() {
  pinMode(PIN, OUTPUT);
  digitalWriteFast(PIN, HIGH);

  // 0x00 0x80 0x80 0x80 0x80 0x80 0x00 0x00 0x02 0x02 STOP
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send1(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send1(); send0();
  send1();

  pinMode(PIN, INPUT);
}

void respondWithCalibration() {
  pinMode(PIN, OUTPUT);
  digitalWriteFast(PIN, HIGH);

  // 0x00 0x80 0x80 0x80 0x80 0x80 0x00 0x00 STOP
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send0(); send0(); send0(); send0(); send0(); send0(); send0(); send0();
  send1();

  pinMode(PIN, INPUT);
}

void respondWithData() {
  bool pStart = digitalReadFast(S_START) == LOW;
  bool pL = digitalReadFast(S_L) == LOW;
  bool pY = digitalReadFast(S_Y) == LOW;
  bool pR = digitalReadFast(S_R) == LOW;
  bool pTilt = digitalReadFast(S_TILT) == LOW;
  bool pB = digitalReadFast(S_B) == LOW;
  bool pA = digitalReadFast(S_A) == LOW;
  bool pX = digitalReadFast(S_X) == LOW;
  bool pZ = digitalReadFast(S_Z) == LOW;
  bool pCUp = digitalReadFast(S_CUP) == LOW;
  bool pCLeft = digitalReadFast(S_CLEFT) == LOW;
  bool pCDown = digitalReadFast(S_CDOWN) == LOW;
  bool pCRight = digitalReadFast(S_CRIGHT) == LOW;
  bool pTaunt = digitalReadFast(S_TAUNT) == LOW;
  bool pAUp = digitalReadFast(S_AUP) == LOW;
  bool pALeft = digitalReadFast(S_ALEFT) == LOW;
  bool pADown = digitalReadFast(S_ADOWN) == LOW;
  bool pARight = digitalReadFast(S_ARIGHT) == LOW;
  bool pTUp = digitalReadFast(S_TUP) == LOW;
  bool pTLeft = digitalReadFast(S_TLEFT) == LOW;
  bool pTDown = digitalReadFast(S_TDOWN) == LOW;
  bool pTRight = digitalReadFast(S_TRIGHT) == LOW;

  uint8_t data[8];
  
  // 0x07: [0]
  // 0x06: [0]
  // 0x05: [0]
  // 0x04: Start
  // 0x03: Y
  // 0x02: X
  // 0x01: B
  // 0x00: A
  data[0] = 0x00;
  if (pStart)   data[0] = setBit4(data[0]);
  if (pY)       data[0] = setBit3(data[0]);
  if (pX)       data[0] = setBit2(data[0]);
  if (pB)       data[0] = setBit1(data[0]);
  if (pA)       data[0] = setBit0(data[0]);

  // 0x07: [1]
  // 0x06: L
  // 0x05: R
  // 0x04: Z
  // 0x03: DPad Up
  // 0x02: DPad Down
  // 0x01: DPad Right
  // 0x00: Dpad Left
  data[1] = 0x80;
  if (pL && !pTilt) data[1] = setBit6(data[1]);
  if (pR && !pTilt) data[1] = setBit5(data[1]);
  if (pZ)           data[1] = setBit4(data[1]);
  if (pTaunt)       data[1] = setBit3(data[1]);

  // Analog X
  if (pALeft && pARight) {
    // TODO: conflict resolution
    data[2] = A_MID;
  } else if (pALeft) {
    if (pTLeft && pTDown) {
      data[2] = A_MID - O_MAX;
    } else if (pTLeft) {
      data[2] = A_MID - O_MIN;
    } else if (pTDown) {
      data[2] = A_MID - O_MID;
    } else {
      data[2] = A_MIN;
    }
  } else if (pARight) {
    if (pTLeft && pTDown) {
      data[2] = A_MID + O_MAX;
    } else if (pTLeft) {
      data[2] = A_MID + O_MIN;
    } else if (pTDown) {
      data[2] = A_MID + O_MID;
    } else {
      data[2] = A_MAX;
    }
  } else {
    data[2] = A_MID;
  }

  // Analog Y
  if (pADown && pAUp) {
    // TODO: conflict resolution
    data[3] = A_MID;
  } else if (pADown) {
    if (pTUp && pTRight) {
      data[3] = A_MID - O_MAX;
    } else if (pTUp) {
      data[3] = A_MID - O_MIN;
    } else if (pTRight) {
      data[3] = A_MID - O_MID;
    } else {
      data[3] = A_MIN;
    }
  } else if (pAUp) {
    if (pTUp && pTRight) {
      data[3] = A_MID + O_MAX;
    } else if (pTUp) {
      data[3] = A_MID + O_MIN;
    } else if (pTRight) {
      data[3] = A_MID + O_MID;
    } else {
      data[3] = A_MAX;
    }
  } else {
    data[3] = A_MID;
  }

  // CStick X
  if (pCLeft && pCRight) {
    // TODO: conflict resolution
    data[4] = A_MID;
  } else if (pCLeft) {
    data[4] = A_MIN;
  } else if (pCRight) {
    data[4] = A_MAX;
  } else {
    data[4] = A_MID;
  }

  // CStick Y
  if (pCDown && pCUp) {
    // TODO: conflict resolution
    data[5] = A_MID;
  } else if (pCDown) {
    data[5] = A_MIN;
  } else if (pCUp) {
    data[5] = A_MAX;
  } else {
    data[5] = A_MID;
  }

  // Left Trigger
  if (pL && pTilt) {
    data[6] = A_TRIGGER;
  } else {
    data[6] = 0x00;
  }

  // Right Trigger
  if (pR && pTilt) {
    data[7] = A_TRIGGER;
  } else {
    data[7] = 0x00;
  }

  pinMode(PIN, OUTPUT);
  digitalWriteFast(PIN, HIGH);
  for(int i = 0; i < 8; i++) {
    if (bit7(data[i])) send1(); else send0();
    if (bit6(data[i])) send1(); else send0();
    if (bit5(data[i])) send1(); else send0();
    if (bit4(data[i])) send1(); else send0();
    if (bit3(data[i])) send1(); else send0();
    if (bit2(data[i])) send1(); else send0();
    if (bit1(data[i])) send1(); else send0();
    if (bit0(data[i])) send1(); else send0();
  }
  send1();

  pinMode(PIN, INPUT);
}

// pinMode must be set to OUTPUT
void send0() {
  digitalWriteFast(PIN, LOW);
  delayMicroseconds(3);
  digitalWriteFast(PIN, HIGH);
  delayMicroseconds(1);
}

// pinMode must be set to OUTPUT
void send1() {
  digitalWriteFast(PIN, LOW);
  delayMicroseconds(1);
  digitalWriteFast(PIN, HIGH);
  delayMicroseconds(3);
}


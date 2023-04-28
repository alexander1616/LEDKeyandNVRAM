#include <Arduino.h>
#define STB 10
#define CLK 11
#define DIO 12
#define TRIG 7
#define ECHO 6

extern void setLCDPosVal(byte, byte);
extern unsigned long measureDistance();

class LCDArray{
public:
  LCDArray(){
    init();
  }
  void init(){
    for (int i = 0; i<8; i++){
      valArray[i] = 0;
    }
    futureC = millis()+250;
  }
  void resetCount(){
    for (int i = 0; i < 4; i++){
      valArray[i] = 0;
    }
  }
  void redrawLCD(){
    for (int i =0; i<8; i++){
      setLCDPosVal(i, valArray[i]);
    }
  }
  void keyPressedVal(byte val){
    byte mask = 0x1;
    byte result = 0;
    // for (byte i = 0; i<8; i++){
    //   result = val&(mask<<i);
    //   if (result){
    //     addPosition(i);
    //   }
    // }
    for (byte i = 4; i<8; i++){
      result = val&(mask<<i);
      if (result){
        addPosition(i);
      }
    }
    result = val&mask;
    if (result){
      resetCount();
      redrawLCD();
    }
  }
  void sonarCounter(){
//    unsigned long future;
//    future = millis() + 250;
//    if (millis() > futureC){
//    while (millis() < future){
//      ;
//    }
    if (millis() < futureC){
      return;
    }
    futureC = millis() + 250;
    word x;
    unsigned long y;
    y = measureDistance();
    x = numGenerator();
    Serial.print("sonar counter y: ");
    Serial.print(y);
    Serial.print(" x: ");
    Serial.println(x);
    Serial.print("Move flag: ");
    Serial.println(moveFlag);
    if (y <= x){
      if (!moveFlag){
        addPosition(3);
        moveFlag = 1;
        Serial.print("Moving! ");
        Serial.println(moveFlag);
      }
    } else {
      moveFlag = 0;
      Serial.print("Not Moving ");
      Serial.println(moveFlag);
    }
//    }
  }
private:
  byte valArray[8];
  void addPosition(byte i){
    valArray[i]++;
    if (valArray[i]>9){
      //valArray[i] = 0;
      valArray[i] = 0;
      if (i == 4){
        init();
        redrawLCD();
        return;
      }
      addPosition(i-1); 
    }
    setLCDPosVal(i, valArray[i]);
  }
  word numGenerator(){
    word val;
    val = valArray[4]*1000 + valArray[5]*100 + valArray[6]*10 + valArray[7];
    return val;
  }
  byte moveFlag = 0;
  unsigned long futureC;
};

LCDArray LCDArray;

void setBit(byte b){
  digitalWrite(DIO, b); //works for rising and falling edge
  digitalWrite(CLK, LOW);
  digitalWrite(CLK, HIGH);
  //digitalWrite(DIO, LOW);
}

void setByte(byte b, byte s){
  byte mask;
  mask = 0x1;

  for (byte i = 0; i < s; i++){
    setBit(b&mask);
    b >>= 1;
  }
}

byte readBit(){
  byte val;
  pinMode(DIO, INPUT);
  digitalWrite(CLK, LOW);
  val = digitalRead(DIO);
  digitalWrite(CLK, HIGH);
  pinMode(DIO, OUTPUT);
  return val;
}

//reading lsb first
byte readByte(){
  byte val = 0;
  byte rval = 0;
  for (int i = 0; i < 8; i++){
    rval=readBit();
    val|=(rval<<i);
  }
  return val;
}

void sendCommand(word value) {
  digitalWrite(STB, LOW);
  setByte(value, 8);
  digitalWrite(STB, HIGH);
}

byte readButtons(){
  byte buttons = 0;
  digitalWrite(STB, LOW);
  setByte(0x42, 8);
  pinMode(DIO, INPUT);
  for (byte i = 0; i < 4; i++){
    byte v = (readByte() << i);
    buttons |= v;
  }
  pinMode(DIO, OUTPUT);
  digitalWrite(STB, HIGH);
  return buttons;
}

byte getButtons(){
  byte buttons;
  word bArray[256]= {0};
  buttons = readButtons();
  if (buttons == 0){
    return buttons;
  }
//  Serial.println("Start button count");
//  Serial.println(buttons, HEX);
  bArray[buttons]++;
  unsigned long cur;
  cur = millis();
  unsigned long future = cur + 150;
  int counter = 0;
  while (cur < future){
    buttons = readButtons();
//    Serial.println(cur);
//    Serial.println(buttons, HEX);
    bArray[buttons]++;
    cur = millis();
    counter++;
  }
//  Serial.println(counter);
  byte store = 0;
  int n;
  for (n = 1; n < 256; n++){
    if (bArray[n] > bArray[store]){
      store = n;
    }
  }
  buttons = store;
//  Serial.println(buttons, HEX);
  return buttons;
}

void setLed(byte value, byte pos){
  pinMode(DIO, OUTPUT);
  sendCommand(0x44);
  digitalWrite(STB, LOW);
  setByte(0xC1 + (pos<<1), 8);
  setByte(value, 8);
  digitalWrite(STB, HIGH);
}

void buttons2Led(byte buttons){
  byte p;
  for (p = 0; p < 8; p++){
    byte mask = 0x1 << p;
    setLed(buttons&mask?1:0, p);
  }
}

void reset() {
  sendCommand(0x40); // set auto increment mode
  digitalWrite(STB, LOW);
  setByte(0xc0, 8);
  for (byte i = 0; i< 16; i++){
    setByte(0x00, 8);
  }
  digitalWrite(STB, HIGH);
}

long microsecondsToCentimeters(long ms){
  return ms / 29 / 2;
}

unsigned long measureDistance(){
  unsigned long duration, cm;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH);
  cm = microsecondsToCentimeters(duration);
  Serial.print("measure Distance ");
  Serial.println(cm);
  return cm;
}

void setup() {
  pinMode(STB, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  sendCommand(0x8f); // activate
  reset();
  LCDArray.redrawLCD();
  Serial.begin(115200);
}

byte LED_POS = 0;
                    /*0*/ /*1*/ /*2*/ /*3*/ /*4*/ /*5*/ /*6*/ /*7*/ /*8*/ /*9*/
byte digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

void setLCDPosVal(byte pos, byte val){
  sendCommand(0x44);
  digitalWrite(STB, LOW);
  setByte(0xc0 | (pos<<1), 8);
  if (pos <4){
    setByte(digits[val], 8);
  } else {
    setByte(digits[val], 8);
  }
  digitalWrite(STB, HIGH);
}

void loop (){
  byte pressedButtons = 0;
  pressedButtons = getButtons();
//  if (pressedButtons == 0){
//    return;
//  }
  if (pressedButtons != 0){
    buttons2Led(pressedButtons);
    LCDArray.keyPressedVal(pressedButtons);
  }
  LCDArray.sonarCounter();
  //delay(100);
}
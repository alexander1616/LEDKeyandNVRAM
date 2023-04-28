#include <Arduino.h>
#define CS 7
#define SK 6
#define DI 5
#define DO 4
#define ORG 8

byte mode = 8; //or 16

void setBit(byte b){
  digitalWrite(DI, b);
  digitalWrite(SK, HIGH);
  digitalWrite(SK, LOW);
  digitalWrite(DI, LOW);
}

void setByte(byte b, byte s){
  byte mask;
  mask = 0x1;
  mask = mask << (s-1);
  while(mask){
    setBit(b&mask);
    mask >>= 1;
  }
}

void setWord(word b, byte s){
  word mask;
  mask = 0x1;
  mask = mask << (s-1);
  while(mask){
    if (b&mask){
      setBit(1);
    } else {
      setBit(0);
    }
    mask >>= 1;
  }
}

byte readBit(){
  byte val;
  digitalWrite(SK, HIGH);
  val = digitalRead(DO);
  digitalWrite(SK, LOW);
  return val;
}

byte readByte(){
  byte val = 0;
  for(int i = 0; i < 8; i++){
     val<<=1;
     val|=readBit();
  }
  return val;
}

word readWord(){
  word val = 0;
  for(int i=0; i<16; i++){
    val<<=1;
    val|=readBit();
  }
  return val;
}

byte readByteAdr(byte adr){
  byte val;
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(1);
  setBit(0);
  setByte(adr, 7);
  val = readByte();
  digitalWrite(CS,LOW);
  return val;
}

word readWordAdr(byte adr){
  word val;
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(1);
  setBit(0);
  setByte(adr, 6);
  val = readWord();
  digitalWrite(CS,LOW);
  return val;
}

void paddedByte(byte val){
  if (val < 16){
    Serial.print('0');
  }
  Serial.print(val, HEX);
}

void paddedWord(word val){
  if (val < 0x10){
    Serial.print("000");
  } else if (val < 0x100){
    Serial.print("00");
  } else if (val < 0x1000){
    Serial.print('0');
  }
  Serial.print(val, HEX);
}

void dumpBuffer(byte adr){
  Serial.print("  ");
  byte val;
  for (int j = 0; j < 16; j++){
    val = readByteAdr(adr+j);
    if((val <' ')||(val >'~')){
      Serial.print('.');
    } else {
      Serial.print((char)val);
    }
  }
}

void ramByteDump(){
  byte val;
  for (int i = 0; i < 128; i+=16){
    for (int j = 0; j < 16; j++){
      val = readByteAdr(i+j);
      paddedByte(val);
      Serial.print(' ');
    }
    dumpBuffer(i);
    Serial.println();
  }
}

void ramWordDump(){
  word val;
  for (int i = 0; i < 64; i+=8){
    for (int j = 0; j < 8; j++){
      val = readWordAdr(i+j);
      paddedWord(val);
      Serial.print(' ');
    }
    digitalWrite(ORG, LOW);
    dumpBuffer(i*2);
    digitalWrite(ORG, HIGH);
    Serial.println();
  }
}

void a_ewen(){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(0);
  if (mode == 8){
    setByte(0b1100000, 7);
  } else {
    setByte(0b110000, 6);
  }
  digitalWrite(CS, LOW);
}

void writeByteAdr(byte adr, byte b){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(1);
  setByte(adr, 7);
  setByte(b, 8);
  digitalWrite(CS, LOW);
  digitalWrite(CS, HIGH);
  delay(10);
  digitalWrite(CS, LOW);
}

void writeWordAdr(byte adr, word b){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(1);
  setByte(adr, 6);
  setWord(b, 16);
  digitalWrite(CS, LOW);
  digitalWrite(CS, HIGH);
  delay(10);
  digitalWrite(CS, LOW);
}

void a_erase(byte adr){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(1);
  setBit(1);
  if (mode == 8){
    setByte(adr, 7);
  } else {
    setByte(adr, 6);
  }
  digitalWrite(CS, LOW);
  delay(10);
}

void a_eral(){
  digitalWrite(CS,HIGH);
  setBit(1);
  setBit(0);
  setBit(0);
  if (mode == 8){
    setByte(0b1000000, 7);
  } else {
    setByte(0b100000, 6);
  }
  digitalWrite(CS,LOW);
  delay(10);
}

void a_byteWral(byte val){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(0);
  setByte(0b0100000, 7);
  //setBit(0x10&val);
  setByte(val, 8);
  digitalWrite(CS, LOW);
  delay(10);
}

void a_wordWral(word val){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(0);
  setByte(0b010000, 6);
  //setBit(0x10&val);
  setWord(val, 16);
  digitalWrite(CS, LOW);
  delay(10);
}

void a_ewds(){
  digitalWrite(CS, HIGH);
  setBit(1);
  setBit(0);
  setBit(0);
  if (mode == 8){
    setByte(0, 7);
  } else {
    setByte(0, 6);
  }
  digitalWrite(CS, LOW);
  
}

void setup() {
  Serial.begin(115200);
  pinMode(CS, OUTPUT);
  pinMode(SK, OUTPUT);
  pinMode(DI, OUTPUT);
  pinMode(DO, INPUT);
  pinMode(ORG, OUTPUT);
  digitalWrite(ORG, LOW);
  mode = 8;
  //Serial.println(readByteAdr(0), HEX);
  a_ewen();
  writeByteAdr(0, 'f');
  writeByteAdr(1, 'l');
  writeByteAdr(2, 'a');
  writeByteAdr(3, 'g');
  ramByteDump();
}

void writeByteStringAdr(byte adr, char* s){
  byte c;
  while (c = *s++){
    writeByteAdr(adr++, c);
  }
}

void writeWordStringAdr(byte adr, char* s){
  byte c;
  word val;
  while (c = *s++){
    val = (*s<<8)|c;
    writeWordAdr(adr++, val);
    if (*s == 0){
      return;
    }
    s++;
  }
}

void loop() {
  Serial.println("NVRAM Hex Dump:      1");
  Serial.println("EWEN Write Enable:   2");
  Serial.println("ERAL Erase All:      3");
  Serial.println("EWDS Write Disable:  4");
  Serial.println("ERASE (Locations):   5");
  Serial.println("WRITE SEQ ASCII:     6");
  Serial.println("SWITCH MODE:         7");
  Serial.println("WRITE ALL (First):   8");
  Serial.println("WRITE ALL (Second):  9");
  Serial.println("WRITE (String 1):    a");
  Serial.println("WRITE (String 2):    b");
  Serial.println("WRITE (String 3):    c");
  
  byte ch;
  while(Serial.available()==0);
  ch = Serial.read();
  switch (ch){
  case '1':
    Serial.println("***************");
    Serial.println("Dumping...");
    Serial.println("***************");
    if (mode == 8){
      ramByteDump();
    } else {
      ramWordDump();
    }
    break;
  case '2':
    Serial.println("***************");
    Serial.println("Write Enabled");
    Serial.println("***************");
    a_ewen();
    break;
  case '3':
    Serial.println("***************");
    Serial.println("Erasing ALL");
    Serial.println("***************");
    a_eral();
    break;
  case '4':
    Serial.println("***************");
    Serial.println("Write Disabled");
    Serial.println("***************");
    a_ewds();
    break;
  case '5':
    Serial.println("***************");
    Serial.println("Erase (Location)");
    Serial.println("***************");
    a_erase(5);
    break;
  case '6':
    Serial.println("***************");
    Serial.println("Writing SA");
    Serial.println("***************");
    byte i;
    word j;
    if (mode == 8){
      for (i = 0; i < 128; i++){
        writeByteAdr(i, i);
      }
    } else {
      for (j = 0; j < 128; j+=2){
        word val;
        //val = (j<<8)|(j+1);
        val = ((j+1)<<8)|j;
        writeWordAdr((byte)(j/2), val);
      }
    }
    break;
  case '7':
    Serial.println("***************");
    Serial.println("Switching Mode");
    Serial.println("***************");
    if (mode == 8){
      digitalWrite(ORG, HIGH);
      mode = 16;     
    } else {
      digitalWrite(ORG, LOW);
      mode = 8;
    }
    Serial.print("Current mode: ");
    Serial.println(mode);
    break;
  case '8':
    Serial.println("***************");
    Serial.println("Writing All 1:");
    Serial.println("***************");
    if (mode == 8){
      a_byteWral(0x99);
    } else {
      a_wordWral(0x8888);
    }
    break;
  case '9':
    Serial.println("***************");
    Serial.println("Writing All 2");
    Serial.println("***************");
    if (mode == 8){
      a_byteWral(0xdd);
    } else {
      a_wordWral(0xffff);
    }
    break;
  case 'a':
    Serial.println("***************");
    Serial.println("Writing String 1");
    Serial.println("***************");
    if (mode == 8){
      writeByteStringAdr(0, "Digitize Engineer");
    } else {
      writeWordStringAdr(0, "Coding Practice");
    }
    break;
  case 'b':
    Serial.println("***************");
    Serial.println("Writing String 2");
    Serial.println("***************");
    if (mode == 8){
      writeByteStringAdr(20, "Alex BitBang");
    } else {
      writeWordStringAdr(20, "Boss Berry");
    }
    break;
  case 'c':
    Serial.println("***************");
    Serial.println("Writing String 3");
    Serial.println("***************");
    if (mode == 8){
      writeByteStringAdr(40, "Study Hex Code");
    } else {
      writeWordStringAdr(40, "Arduino Fun");
    }
    break;
  case '\n':
    break;
  default:
    Serial.println("***************");
    Serial.println("Bad Command");
    Serial.println("***************");
    break;
  }
}
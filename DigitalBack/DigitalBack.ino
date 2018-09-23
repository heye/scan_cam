/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
#include "SdFat.h"
#include <ADC.h>

 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;



#define endstop_bot 26
#define endstop_top 27
#define m_dir 25
#define m_step 24
#define m_enable 12

#define rs 2
#define sh1 3
#define cp 4
#define sh2 5
#define sh3 6
#define clk 7
#define sp 8
#define clk_fin 9

#define LINEBUFF_SIZE 3825

int values[LINEBUFF_SIZE];
int *values_start;
int *values_it;
bool doSample;
ADC *adc = new ADC(); // adc object;


// the setup routine runs once when you press reset:
void setup() {                
  // STEPPER
  pinMode(endstop_bot, INPUT_PULLUP);    
  pinMode(endstop_top, INPUT_PULLUP);     
  pinMode(m_dir, OUTPUT);
  pinMode(m_step, OUTPUT); 
  pinMode(m_enable, OUTPUT); 

  // SENSOR
  pinMode(rs, OUTPUT); 
  pinMode(sh1, OUTPUT); 
  pinMode(cp, OUTPUT); 
  pinMode(sh2, OUTPUT); 
  pinMode(sh3, OUTPUT); 
  pinMode(clk, OUTPUT); 
  pinMode(sp, OUTPUT); 
  pinMode(clk_fin, OUTPUT); 

  // LED
  pinMode(led, OUTPUT); 

  // ADC
  pinMode(A3, INPUT);  
  adc->setAveraging(1); // set number of averages
  adc->setResolution(16); // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);
  Serial.begin(9600);

  
  
  values_start = values;
  values_it = values;
  
  *values_it = 4;
}

//################################# SD

bool sdEnabled = false;
bool sdOpenFile = false;
SdFatSdioEX sdEx;
File file;

void yield() {
  if (!sdEx.card()->isBusy()) {return;}
  while (sdEx.card()->isBusy()) {}
}

void createFile(){
  if(sdOpenFile){
    return;
  }
  sdEx.remove("values.bin");
  if (!file.open("values.bin", O_WRITE | O_CREAT)) {
    Serial.println("SD FILE CREATE ERR");
    return;
  }
  sdOpenFile = true;
}

void openFile(){
  if(sdOpenFile){
    return;
  }
  if (!file.open("values.bin", O_WRITE | O_AT_END | O_APPEND)) {
    Serial.println("SD FILE OPEN ERR");
    return;
  }
  sdOpenFile = true;
}

void closeFile(){
  if(sdOpenFile){
    file.close();
    sdOpenFile = false;
  }
}

void pushBuffer(){
  if(sdOpenFile){
    //file.truncate(0);
    file.write(values, LINEBUFF_SIZE*sizeof(int));
  }
}

void appendFile(){
  if(sdOpenFile){
    //file.truncate(0);
    file.write("test\n", 5);
  }
}

void writeBuffer(){
  
}

void initSD(){
  static bool isInit = false;

  if(!isInit){
    isInit = true;
    
    if (!sdEx.begin()) {
      Serial.println("SD INIT FAIL");
      return;
    }
    // make sdEx the current volume.
    sdEx.chvol();

    sdEnabled = true;

    Serial.println("SD INIT SUCCESS");
  }
}


//############################## STEPPER

void disableStepper(){
  digitalWrite(m_enable, HIGH);
}

void enableStepper(){
  digitalWrite(m_enable, LOW);
}


bool endstopBot(){
  return digitalRead(endstop_bot) == LOW;
}

bool endstopTop(){
  return digitalRead(endstop_top) == LOW;
}



void doStep(){
  static bool state = false;
  if(state){
    digitalWrite(m_step, HIGH);
  }
  else{
    digitalWrite(m_step, LOW);
  }
  state = !state; 
}

void dirUp(){
  digitalWrite(m_dir, HIGH);
}

void dirDown(){
  digitalWrite(m_dir, LOW);
}


void stepUp(){
  //return if top is reached
  if(endstopTop()){
    return;
  }

  digitalWrite(m_dir, HIGH);
  
  digitalWrite(m_step, HIGH);
  digitalWrite(m_step, LOW);
}


void stepDown(){
  //return if top is reached
  if(endstopBot()){
    return;
  }

  digitalWrite(m_dir, LOW);
  
  digitalWrite(m_step, HIGH);
  digitalWrite(m_step, LOW);
}

//############################## SENSOR
//################ CLK
inline void clkLow(){
  digitalWrite(clk, LOW);
  digitalWrite(clk_fin, LOW);
}
inline void clkHigh(){
  digitalWrite(clk, HIGH);
  digitalWrite(clk_fin, HIGH);
}

//################ SP
inline void spLow(){
  digitalWrite(sp, LOW);
}
inline void spHigh(){
  digitalWrite(sp, HIGH);
}

//################ SH
inline void shLow(){
  digitalWrite(sh1, LOW);
  digitalWrite(sh2, LOW);
  digitalWrite(sh3, LOW);
}
inline void shHigh(){
  digitalWrite(sh1, HIGH);
  digitalWrite(sh2, HIGH);
  digitalWrite(sh3, HIGH);
}

//################ RS
inline void rsLow(){
  digitalWrite(rs, LOW);
}
inline void rsHigh(){
  digitalWrite(rs, HIGH);
}

//################ CP
inline void cpLow(){
  digitalWrite(cp, LOW);
}
inline void cpHigh(){
  digitalWrite(cp, HIGH);
}


void sample(){
  if(doSample){  
    uint32_t x = adc->analogRead(A3);
    *values_it = x;
    values_it++;
  }
}


//################################ START
void startSH(){

  // 1 high
  clkHigh();

  
  spLow();
  //2000ns 

  shHigh();
  //3000ns 

  shLow();
  //2000ns 



  // 1 low
  clkLow();
  //100ns

  spHigh();
  //100ns

  rsHigh();
  //100ns - rs pulse width
  rsLow();

  cpHigh();
  //100ns - cp pulse width
  cpLow();
}


// ################################# NORMAL
void normal() {
  clkHigh();
  //50ns - video delay

  sample();

  clkLow();

  rsHigh();
  // 100ns - rs pulse width
  rsLow();

  cpHigh();
  //100ns - cp pulse width
  cpLow();
  
}



void printline(){
  static bool printed = false;

  if(!printed){
    Serial.println("#");
    values_it = values_start;
    for(int i = 0; i < 3825; i ++){
      if(i % 100 == 0){
        Serial.println((int)*values_it);
        //SerialUSB.print("\t");
        delay(1);
      }
      values_it++;
    }
    //Serial.println(" ");
    //SerialUSB.flush();
    //delay(100);
  }
  //printed = true;
}


void readLine(){
  startSH();
  
  values_it = values_start;
  for(int i = 0; i < 3822; i++){
    normal();
  }
}

void rewind(){
  while(!endstopBot()){
    stepDown();
    delay(2); 
  }
}

void capture(){
  initSD();
  createFile();
  closeFile();
  enableStepper();
  Serial.println("CAPUTRE START");

  //LINES
  for(int i=0; i < 500; i++){
    Serial.print("LINE ");
    Serial.println(i);

    //dummy read out
    doSample = false;
    readLine();
    readLine();
    readLine();
    readLine();
    delay(30);
    doSample = true;
    readLine();
    
    Serial.println("#");

    uint32_t t = micros();
    openFile();
    pushBuffer();
    closeFile();
    t = micros() - t;
    if(t > 50000){
      Serial.println("HANG");
      readLine();
      readLine();
      readLine();
      readLine();
      readLine();
      readLine();
      readLine();
      readLine();
    }
    
    //printline();

    //stepUp();
    //TODO: move stepper
    for(int j=0; j<20; j++){
      stepUp();
      delay(4);
    }
  }
  disableStepper();

  Serial.println("CAPUTRE DONE");
  
}

// the loop routine runs over and over again forever:
void loop() {
  disableStepper();

  static bool doRewind = true;
  if(doRewind){
    enableStepper();
    rewind();
    doRewind = false;
    disableStepper();
  }


  static bool doCapture = true;
  //static bool runCapture = false;
  //static int line = 0;
  if(doCapture){
    //Serial.println("CAPUTRE START");
    //initSD();
    //createFile();
    //closeFile();
    capture();
    doCapture = false;
    //runCapture = true;
  }

  //delay(100);

  /*if(runCapture){
    Serial.print("LINE ");
    Serial.println(line);
    
    doSample = false;
    readLine();
    Serial.println("#");
    //TODO: integration time delay?
    
    doSample = true;
    readLine();
    Serial.println("#");

    pushBuffer();
    line ++;
  }

  if(line == 127){
    Serial.println("CAPUTRE END");
    runCapture = false;
  }*/


  //################# STEPPER TEST
  //stepUp();
  //delay(1000);
  
  /*static bool disabled = false;

  disabled |= endstopBot() || endstopTop();
  if(disabled){
    digitalWrite(led, LOW);
  }
  else{
    digitalWrite(led, HIGH);
  }  

  delay(500);

  if(!disabled){
    dirUp();
    doStep();
  }

  if(disabled){
    disableStepper();
  }
  else{
    enableStepper();
  }*/

  //################# SENSOR TEST
    
  static int iterations = 0;
  //if (iterations > 9){
    //doSample = true;
    //integration time
  //}

  //uint32_t t = micros();
  doSample = false;
  //dummy read outs to clear any overflow in the sensor
  readLine();
  readLine();
  readLine();
  readLine();
  readLine();

  //integration time
  delay(30);

  //sample
  doSample = true;
  readLine();

  
  //t = micros() - t;
  //Serial.print("sample line micros: ");
  //Serial.println(t);
  
  iterations++;
  //Serial.println(iterations);
  
  if(iterations > 10){
    printline();
    //initSD();
    //openFile();
    //t = micros();
    //pushBuffer();
    //t = micros() - t;
    //Serial.print("write line micros: ");
    //Serial.println(t);
    //closeFile();
    
    iterations = 0;
    //doSample = false;
  }


  //################### SD TEST
  /*static bool doTest = true;
  if(doTest){
    Serial.println("buffer push start");
    initSD();
    createFile();
    pushBuffer();
    closeFile();
    
    doTest = false;
    Serial.println("buffer push done");
  }*/

}


















#define nop asm volatile ("nop")

//pin 46 = ø2 = !ø1

#define p_rs          0b000000000000000000000000000000010
#define p_cp          0b000000000000000000000000000000100
#define p_sh1         0b000000000000000000000000000001000
#define p_sh2         0b000000000000000100000000000000000
#define p_sh3         0b000000000000000010000000000000000
#define p_sh          0b00000000000000011000000000000100x0
#define p_clk         0b000000000000000001010000000000000
#define p_sp          0b000000000000000000100000000000000

#define initial_set   0b000000000000000111010000000001110
#define initial_unset 0b000000000000000000100000000000000

#define p_pi_all      0b000000000000000000000011111111111
#define pi_clk        0b000000000000000000000000000000001
#define line_clk      0b000000000000000000000000000000010

 
uint16_t values[3825];
uint16_t *values_start;
uint16_t *values_it;

bool doSample;

inline void wait1000ns(){
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;
  nop;nop;nop;nop;
  //80 nops = 1000ns
}

void init_io_sensor(){
  pinMode (48, OUTPUT) ; //clk
  pinMode (50, OUTPUT) ; //clk
  pinMode (49, OUTPUT) ; //2l - sp
  pinMode (33, OUTPUT) ; //r - rs
  pinMode (34, OUTPUT) ; //cp - cp
  pinMode (35, OUTPUT) ; //tg1 - sh1
  pinMode (46, OUTPUT) ; //tg2 - sh2
  pinMode (47, OUTPUT) ; //tg3 - sh3
  pinMode (A0, INPUT) ; //tg3 - sh3
  pinMode (A1, INPUT) ; //tg3 - sh3
  pinMode (A2, INPUT) ; //tg3 - sh3
  pinMode (A3, INPUT) ; //tg3 - sh3
}

void init_output_pi(){
  //D.0 - D.10
  pinMode (25, OUTPUT) ; 
  pinMode (26, OUTPUT) ; 
  pinMode (27, OUTPUT) ; 
  pinMode (28, OUTPUT) ; 
  pinMode (14, OUTPUT) ; 
  pinMode (15, OUTPUT) ; 
  pinMode (29, OUTPUT) ; 
  pinMode (11, OUTPUT) ; 
  pinMode (12, OUTPUT) ; 
  pinMode (30, OUTPUT) ; 
  pinMode (32, OUTPUT) ; 
  
  pinMode (68, OUTPUT) ; //line_clk
  pinMode (69, OUTPUT) ; //pi_clk
}

void setup() {
  // put your setup code here, to run once:
  init_io_sensor();
  init_output_pi();
  
  //TODO: output for other port (B?, D?)
  
  REG_ADC_MR = (REG_ADC_MR & 0xFFF0FFFF) | 0x00020000;
  REG_ADC_MR = (REG_ADC_MR & 0xFFFFFF0F) | 0x00000080; //enable FREERUN mode
  ADC->ADC_CHER = 0x80; //enable ADC on pin A0
  
  analogReadResolution(12);

  //set all to zero
  REG_PIOC_SODR = initial_set;
  //unset nothing
  REG_PIOC_CODR = initial_unset;
  
  SerialUSB.begin(9600);
  while (!Serial) {;}
  SerialUSB.println("start");
  SerialUSB.flush();
  
  values_start = values;
  values_it = values;
  
  *values_it = 4;
}

inline void clockPi(){
  //toggle px_clk
  static uint32_t i = 0;
  i = (i+1)%2;
  if (i==0){
    REG_PIOA_CODR = pi_clk;
  }
  else{
    REG_PIOA_SODR = pi_clk;
  }
}

inline void sample(){
  //uint8_t it = 0;
  //static uint16_t x = 0;
  
  //it++;
  
  //if(it % 8 == 0){
  if(doSample){
    //it = 0;
    //while((ADC->ADC_ISR & 0x80)==0); // wait for conversion
  
    uint32_t x = analogRead(A0);
    //uint16_t x = ADC->ADC_CDR[7];
  
    *values_it = x;
    values_it++;
  }
  
  //TODO: set port 
  //REG_PIOD_CODR = p_pi_all;
  //REG_PIOD_SODR = ((x>>2) & p_pi_all);
  
  //clockPi();
}

inline void rsPulseWidth(){
  //typ. 100ns
  nop;nop;nop;nop;nop;nop;nop;nop;
  //nop;nop;nop;nop;nop;nop;nop;nop;
  //nop;nop;nop;nop;nop;nop;nop;nop;
}

inline void cpPulseWidth(){
  nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  //nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  //nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
}

inline void spPulseWidth(){
  //typ. 100ns
  nop;nop;nop;nop;nop;nop;nop;nop;
  //nop;nop;nop;nop;nop;nop;nop;nop;
}


inline void videoDelay(){
  //typ. 15ns
  nop;nop;nop;nop;//nop;nop;nop;nop;
  //nop;nop;nop;nop;nop;nop;nop;nop;
  //nop;nop;nop;nop;nop;nop;nop;nop;
}

inline void startSH(){
  //pre: sp = high

  //1 = high
  REG_PIOC_SODR = p_clk;
  nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  
  //sp = low
  REG_PIOC_SODR = p_sp;
  
  wait1000ns();
  wait1000ns(); //t1 = tpy. 1000ns
  
  //sh = high
  REG_PIOC_CODR = p_sh;
  wait1000ns();wait1000ns();wait1000ns(); //t1 = tpy. 2000ns
  //sh = low
  REG_PIOC_SODR = p_sh;
  
  wait1000ns(); //t5 = tpy. 1000ns
  wait1000ns();
  
  
  
  //1 = low
  //REG_PIOC_SODR = p_clk;
  REG_PIOC_CODR = p_clk;
  nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  
  //sp = high
  REG_PIOC_CODR = p_sp;
  nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  
  //RS = high
  REG_PIOC_CODR = p_rs;
  rsPulseWidth();
  //RS = low
  REG_PIOC_SODR = p_rs;
  nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  
  //CP = high
  REG_PIOC_CODR = p_cp;
  cpPulseWidth();
  //CP = low
  REG_PIOC_SODR = p_cp;
}

inline void normal(){
  //pre: sp = high
  //post: sp = high

  //1 = high
  REG_PIOC_SODR = p_clk;
  
  videoDelay();
  
  sample();
  
  //sp = high
  //REG_PIOC_CODR = p_sp;
  //spPulseWidth();
  //nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns 
  //sample();
    //nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
    
  //Test - no sp low for non-sample-hold adc
  //sp = low
  //REG_PIOC_SODR = p_sp;
  
  //1 = low
  REG_PIOC_CODR = p_clk;
  
  //RS = high
  REG_PIOC_CODR = p_rs;
  rsPulseWidth();
  //RS = low
  REG_PIOC_SODR = p_rs;
  //nop;nop;nop;nop;nop;nop;nop;nop;//typ. 100ns
  
  //CP = high
  REG_PIOC_CODR = p_cp;
  cpPulseWidth();
  //CP = low
  REG_PIOC_SODR = p_cp;
  nop;nop;nop;nop;nop;nop;nop;//typ. 80ns
}

void printline(){
  SerialUSB.println("#");
  values_it = values_start;
  for(int i = 0; i < 3825; i ++){
    if(i % 100 == 0){
      SerialUSB.println((int)*values_it);
      //SerialUSB.print("\t");
      SerialUSB.flush();
    }
    values_it++;
  }
  //Serial.println(" ");
  //SerialUSB.flush();
  //delay(100);
}

void loop() {
  static uint8_t iterations = 0;
  //set line clock high
  REG_PIOA_SODR = line_clk;
  //clock pi => reads line clock
  clockPi();
  
  //start sh
  startSH();
  
  //unset line clock for next pi-clock
  REG_PIOA_CODR = line_clk;
  
  if (iterations > 9){
    doSample = true;
  }
  
  values_it = values_start;
  for(int i = 0; i < 3822; i++){
    normal();
  }
  
  //integration time
  //delay(10);
  
  iterations++;
  
  if(iterations > 10){
    printline();
    iterations = 0;
    doSample = false;
  }
}

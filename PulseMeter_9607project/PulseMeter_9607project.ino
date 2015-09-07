#define FREQ 500                            // Sampling rate
volatile boolean state;

int pulsePin = 54;                          // Analog pin 0
int beatPin = 13;                           // Heart beat led

// Variables for BPM calculation
volatile int rate[10];                      // Array to store last ten IBI values
volatile unsigned long sampleCounter = 0;   // Used to determine pulse timing
volatile unsigned long lastBeatTime = 0;    // Used to find IBI
volatile int P =512;                        // Peak of waveform
volatile int T = 512;                       // Trough of waveform
volatile int thresh = 525;                  // Tresh of waveform when beat happens
volatile int amp = 100;                     // Amplitude of waveform
volatile boolean firstBeat = true;          // Used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = false;        // Used to seed rate array so we startup with reasonable BPM

// Variables for interrupt service routine 
volatile int BPM;                           // BPM (beats per minute)
volatile int Signal;                        // Pulse data collecting from sensor
volatile int IBI = 600;                     // IBI (inter beat interval) 
volatile boolean Pulse = false;             // Heart beat is detected or not 
volatile boolean QS = false;                // Quantified Self flag 

void setup(){
  pinMode(beatPin,OUTPUT);                
  Serial.begin(115200);                    // Serial tranmission speed
  startTimer(TC1, 0, TC3_IRQn, FREQ);       // Timer counter setup
  setupBlueToothConnection();               // Bluetooth module setup
  delay(20000);
}

void loop(){
  int recv;                                 // Serial input from Android app through Bluetooth serial
  if(Serial2.available()){                 // Check if there's any data sent from the remote bluetooth shield
    recv = Serial2.parseInt();             // Message(number '1') received from Android app
    if(recv){  
      Serial.println(recv);
          if (QS == true){
            digitalWrite(beatPin,HIGH);     
            SerialOutput();                 // Serial output the BPM     
            QS = false;                     // Reset Quantified Self flag    
          } 
          else{ 
            digitalWrite(beatPin,LOW);      // No beat tested
          }
          delay(20);                        
    }
  }
}

// Serial output
void SerialOutput(){    
    Serial.println(BPM);                
    Serial2.print(BPM);                    //Without linebreak, because the Android input stream has to deal with the socket stream data 
}

// Timer setup
void startTimer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency) {
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)irq);
        TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
        uint32_t rc = VARIANT_MCK/2/frequency;           // 2 because we selected TIMER_CLOCK1 above
        TC_SetRA(tc, channel, rc/2);                     // 50% high, 50% low
        TC_SetRC(tc, channel, rc);
        TC_Start(tc, channel);
        
        tc->TC_CHANNEL[channel].TC_IER=TC_IER_CPCS;     // TC Interrupt Enable Register RC Compare Interrupt
        tc->TC_CHANNEL[channel].TC_IDR=~TC_IER_CPCS;    // TC Interrupt Disable Register
        NVIC_EnableIRQ(irq);
}

// Timer counter handler 
void TC3_Handler()
{
  TC_GetStatus(TC1,0);
  state =! state;
       
  __disable_irq();                          // Disable interrupt
  Signal = analogRead(pulsePin);            // Read the analogue data 
  sampleCounter += 2;                       // Keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;     // Monitor the time since the last beat to avoid noise

  // Trough
  if(Signal < thresh && N > (IBI/5)*3){     // Avoid dichrotic interference by waiting 0.6 IBI
    if (Signal < T){                        // T is the trough
      T = Signal;                           // Keep track of lowest point in pulse wave 
    }
  }
  
  // Peak
  if(Signal > thresh && Signal > P){        // Thresh condition helps avoid noise
    P = Signal;                             // P is the peak
  }                                         // Keep track of highest point in pulse wave

  
  // Calculate BPM when heart beat happens
  if (N > 250){                             // Avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
      Pulse = true;                         // Set the Pulse flag when the beats happen
      IBI = sampleCounter - lastBeatTime;   // Measure time between beats in mS
      lastBeatTime = sampleCounter;         // Keep track of time for next pulse

      if(secondBeat){                       // If this is the second beat
        secondBeat = false;                 // Clear secondBeat flag
        for(int i=0; i<=9; i++){            // Seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;                      
        }
      }

      if(firstBeat){                        // If it's the first time we found a beat
        firstBeat = false;                  // Clear firstBeat flag
        secondBeat = true;                  // Set the second beat flag
        __enable_irq();                     // Enable interrupts again
        return;                             // IBI value is unreliable so discard it
      }   

      // Keep a sum of the last 10 IBI values
      word runningTotal = 0;                 // Clear the runningTotal variable    

      for(int i=0; i<=8; i++){               // Shift data in the rate array
        rate[i] = rate[i+1];                 // Drop the oldest IBI value 
        runningTotal += rate[i];             // Add up the 9 oldest IBI values
      }

      rate[9] = IBI;                         // Add the latest IBI to the rate array
      runningTotal += rate[9];               // Add the latest IBI to runningTotal
      runningTotal /= 10;                    // Average the last 10 IBI values 
      BPM = 60000/runningTotal;              // BPM calculation
      QS = true;                             // Set Quantified Self flag 
    }                       
  }
  
  // Heart beat is over
  if (Signal < thresh && Pulse == true){   
    Pulse = false;                           // Reset the Pulse flag
    amp = P - T;                             // Get amplitude of the pulse wave 
    thresh = amp/2 + T;                      // Set thresh at 50% of the amplitude
    P = thresh;                              // Reset these for next test
    T = thresh;
  }
  
  // Without heart beat for a long time
  if (N > 2500){                             // If 2.5 seconds go by without a beat, set the variables to default
    thresh = 512;
    P = 512;
    T = 512;
    lastBeatTime = sampleCounter;
    firstBeat = true;
    secondBeat = false;
  }

  __enable_irq();                            // Enable interrupt
        
}

// Set Bluetooth module
void setupBlueToothConnection()
{
    Serial2.begin(115200);                           // set serial2 to baud rate 115200  
    Serial2.print("\r\n+STWMOD=0\r\n");              // set the bluetooth work in slave mode
    Serial2.print("\r\n+STBD=115200\r\n");           // set bluetooth module baud rate to 115200
    Serial2.print("\r\n+STNA=Group9project\r\n");    // set the bluetooth name
    Serial2.print("\r\n+STOAUT=1\r\n");              // Permit Paired device to connect me
    Serial2.print("\r\n+STAUTO=0\r\n");              // Auto-connection is forbidden
    Serial2.print("\r\n+STPIN=0000\r\n");            // set pin code
    delay(2000);                                      // This delay is required.
    Serial2.print("\r\n+INQ=1\r\n");                 // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                      // This delay is required.
    Serial2.flush();                                 // flush serial2 
}

// FFT library options
// See http://wiki.openmusiclabs.com/wiki/Defines 
#define FFT_N 16 
#define WINDOW 0
#define OCTAVE 1
#define OCT_NORM 0
#include <FFT.h>

#define SAMPLE_PERIOD_MICROSECONDS 250

bool DEBUG = true;

int numInputs = 3;
const byte cvIn[] = {A2, A1, A0};
int numPots = 3;
const byte potIn[]= {A5, A4, A3};
const byte audioPin = A1;
const byte buttonPin =  9;

void setup() {
  analogReference(DEFAULT);
  Serial.begin(115200);
  
  // Setup CV inputs
  pinMode(cvIn[0], INPUT);
  pinMode(cvIn[1], INPUT);
  pinMode(cvIn[2], INPUT);
  pinMode(buttonPin, INPUT);

}

/* Function: getSamples
 * --------------------
 * Populates every other element of an array with FFT_N samples, sampled at
 * 1000/SAMPLE_PERIOD_MICROSECONDS kHz. Takes about 6.7ms to run.
 */
void getSamples(int samples[FFT_N*2]) {
  unsigned long next_sample_time = micros();
  for (byte i = 0; i < FFT_N*2; i+=2) {
    while (micros() < next_sample_time); // wait till next sample time
    samples[i] = analogRead(audioPin) - 512;
    samples[i+1] = 0;
    next_sample_time += SAMPLE_PERIOD_MICROSECONDS;
  }
}



int last[] = {0,0,0}; // needs global scope


bool on = false;
void sampleCV(){

  int sample, in, scaled;
  for(int x = 0; x < numInputs; ++x){
    //read and smooth
    in = analogRead(cvIn[x]);
    sample = smooth(in, last[x]);

    if(sample != -1 && sample != last[x]){
      //save raw value for next smoothing comparison
      last[x] = sample;

      
      //output value using serial ( rescale val/1023.0 for 0. to 1. on the other end)
      String out = String(x) + String(" ");
      Serial.println(out + sample);

    }
    else{
      //work around for 1023 pull up readout ( no jack which should be 0 )
      if( in == 1023 && last[x] != 1023){
          last[x] = 1023;
          String out = String(x) + String(" ");
          Serial.println(out + String("0"));
      }
    }
  }
}

int lastPot[] = {0,0,0}; // needs global scope

void samplePot(){
  int sample,in, scaled;
  for( int x = 0; x < numPots; ++x){
    in = analogRead(potIn[x]);
    sample = smooth(in, lastPot[x]);
    if(sample != -1  && sample != lastPot[x]){
      //save raw value for next smoothing comparison
      lastPot[x] = sample;

      //output value using serial ( rescale val/1023.0 for 0. to 1. on the other end)
      String out = String(x + 10) + String(" ");
      Serial.println(out + sample );
    } 
   }
}

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 10;    // the debounce time; increase if the output flickers

void sampleButton(){
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      //send button state over serial ( message number 30 )
      if(buttonState == HIGH){
         Serial.println( "30 1");
      }
      else{
         Serial.println( "30 0");
      }
    }
  }

  lastButtonState = reading;
}



int smooth(int in, int PotPrevVal){ 
    int margin = PotPrevVal * .00001; //  get 2% of the raw value.  Tune for lowest non-jitter value.
    /*
     * Next we add (or subtract...) the 'standard' fixed value to the previous reading. (PotPrevVal needs to be declared outside the function so it persists.)
     * Here's the twist: Since the jitter seems to be worse at high raw vals, we also add/subtract the 2% of total raw. Insignificantat on low 
     * raw vals, but enough to remove the jitter at raw >900 without wrecking linearity or adding 'lag', or slowing down the loop, etc.
     */
    if (in > PotPrevVal + (4 + margin) || in < PotPrevVal - (5 + margin)) { // a 'real' change in value? Tune the two numeric values for best results
      //average last 2 values ofr smoothing
      in = (PotPrevVal + in) / 2.0;
      //PotPrevVal = in; // store valid raw val  for next comparison
      return in;
    }  
    return -1;  
}

//sends FFT data over serial, 255 staring int means 4 ints/bins of fft follow
void sendFFT( byte bins[LOG_N] ){
    Serial.println(String(100) + ' ' + bins[0]);
    Serial.println(String(101) + ' ' + bins[1]);
    Serial.println(String(102) +' ' + bins[2]);
    Serial.println(String(103) +' ' + bins[3]);
}


void loop() {
  // put your main code here, to run repeatedly:
  sampleButton();
  sampleCV();
  samplePot();

  //FFT 
  //getSamples(fft_input);
//  fft_reorder();
//  fft_run();
//  fft_mag_octave();

//  sendFFT(fft_oct_out);
  

}




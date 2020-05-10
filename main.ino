#include "arduinoFFT.h"
#include "FastLED.h"

#define BUTTON_PIN 7
bool buttonState;
//variables for LEDs
#define NUM_LEDS 200
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

int normCoef = 1;

//variables for FFT
#define SAMPLES 16              //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be less than 10000 due to ADC

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

//functions
void runFFT();
void visualize();
double avg(double* nums, int start, int last);
double findMax(double* nums, int start, int last);
bool hasChanged();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN,INPUT);

  buttonState = digitalRead(BUTTON_PIN);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));

  for(int i=0; i<3;i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);

  }
}

void loop() {
  visualize_1();
  visualize_2();
}

void runFFT(){

    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
      microseconds = micros();    //Overflows after around 70 minutes!

      vReal[i] = analogRead(0);
      vImag[i] = 0;

      while(micros() < (microseconds + sampling_period_us)){
      }
    }

    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
}

void visualize_1(){
  int bassMax,trebMax,bassTemp,trebTemp;
  uint8_t hue;
  runFFT();
  FastLED.clear();
  //bass visualization
  bassMax = findMax(vReal,0,SAMPLES/4); //largest number in the first half of vReal[]
  if(bassTemp>NUM_LEDS/2) bassMax = NUM_LEDS/4;

  for(int i=0; i<bassMax*normCoef*2; i++){
    leds[i] = CRGB::Blue;
    leds[NUM_LEDS-1-i] = CRGB::Red;
  };

  trebMax = findMax(vReal,SAMPLES/4,SAMPLES/2); //largest number in the first half of vReal[]
  if(trebTemp>NUM_LEDS/2) trebMax = NUM_LEDS/4;

  for(int i=0; i<trebMax*normCoef; i++){
    leds[NUM_LEDS/2+i] = CRGB::Green;
    leds[NUM_LEDS/2-i] = CRGB::Green;
  };
  FastLED.show();

  while(1){
    bassMax--;
    trebMax--;
    hue++;

    runFFT();

    FastLED.clear();

    //bass visualization
    //    bassTemp = findMax(vReal,0,SAMPLES/4); //largest number in the first half of vReal[]
    bassTemp = vReal[0];
    if(bassTemp>NUM_LEDS/4) bassTemp = NUM_LEDS/4;
    if(bassTemp>bassMax) bassMax = bassTemp;

    for(int i=0; i<bassMax*normCoef*2; i++){

      leds[i] = CHSV(hue,255-bassMax*5,255);
      leds[NUM_LEDS-1-i] = CHSV(hue,255-bassMax*5,255);
    }

    //    //treble visualization
    //    trebTemp = avg(vReal,SAMPLES/4,SAMPLES/2); //largest number in the second half of vReal[]
    //    if(trebTemp>NUM_LEDS/4) trebTemp = NUM_LEDS/4;
    //    if(trebTemp>trebMax) trebMax = trebTemp;
    //
    //    for(int i=0; i<trebMax*normCoef; i++){
    //      leds[NUM_LEDS/2+i] = CRGB::Green;
    //      leds[NUM_LEDS/2-i] = CRGB::Green;
    //    }
    FastLED.show();
    if(hasChanged()) return;
  }
}

void visualize_2(){
  FastLED.clear();
  runFFT();
  for(int i=0; i<SAMPLES/2; i++){
  //    Serial.print(i*NUM_LEDS/(SAMPLES/2));
  //    Serial.print(", ");
  //    Serial.println(vReal[i]);
    for(int k=i*NUM_LEDS/(SAMPLES/2); k<i*NUM_LEDS/(SAMPLES/2)+vReal[i]; k++){
      leds[k] = CRGB::Blue;
    }
  }
  FastLED.show();
  if(hasChanged()) return;
}

//finds the average from start to last inclusive
double avg(double* nums, int start, int last){
  double sum;
  for(int i = start; i < last+1; i++){
    sum += nums[i];
  }
  return sum/(last-start+1);
}

//finds the max from start to last inclusive
double findMax(double* nums, int start, int last){
  double temp=0;
  for(int i = start; i < last+1; i++){
    if(temp < nums[i]) temp = nums[i];
  }
  return temp;
}

//returns true if the state of BUTTON_PIN is different than buttonState.
// Changes buttonState to equal BUTTON_PIN
bool hasChanged(){
  bool temp = digitalRead(BUTTON_PIN);
  if(temp==buttonState) return 0;
  buttonState = temp;
  return 1;
}

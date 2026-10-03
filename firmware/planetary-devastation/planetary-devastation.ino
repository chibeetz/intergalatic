#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <StevesAwesomeButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sinewave.h"
#include "square.h"
#include "pulse.h"
#include "sample_hold.h"
#include "triangle.h"
#include "sawtooth.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

// GUItool: begin automatically generated code
AudioInputI2S i2s2;                 //xy=214.57142639160156,252.7142791748047
AudioSynthWaveform waveform1;       //xy=216.71426391601562,285.0389709472656
AudioMixer4 mixer2;                 //xy=410.0000228881836,278.5714340209961
AudioSynthWaveform waveform2;       //xy=451.1428527832031,402
AudioEffectMultiply multiply1;      //xy=593.1428527832031,395.9999942779541
AudioEffectBitcrusher bitcrusher1;  //xy=786.7142639160156,366.1428413391113
AudioMixer4 mixer1;                 //xy=986.7141723632812,398.5714111328125
AudioEffectDelay delay1;            //xy=1132,201
AudioMixer4 mixer3;                 //xy=1141,603
AudioFilterBiquad biquad1;          //xy=1315.5,177
AudioMixer4 mixer4;                 //xy=1351.5,390
AudioOutputUSB usb1;                //xy=1536.28564453125,301.4285583496094
AudioOutputI2S i2s1;                //xy=1556.28564453125,383.4285583496094
AudioConnection patchCord1(i2s2, 0, mixer2, 0);
AudioConnection patchCord2(i2s2, 1, mixer2, 1);
AudioConnection patchCord3(waveform1, 0, mixer2, 2);
AudioConnection patchCord4(mixer2, 0, multiply1, 0);
AudioConnection patchCord5(waveform2, 0, multiply1, 1);
AudioConnection patchCord6(multiply1, 0, mixer1, 2);
AudioConnection patchCord7(multiply1, 0, mixer1, 3);
AudioConnection patchCord8(multiply1, bitcrusher1);
AudioConnection patchCord9(bitcrusher1, 0, mixer1, 0);
AudioConnection patchCord10(bitcrusher1, 0, mixer1, 1);
AudioConnection patchCord11(mixer1, 0, mixer3, 0);
AudioConnection patchCord12(mixer1, 0, mixer4, 2);
AudioConnection patchCord13(mixer1, 0, mixer4, 3);
AudioConnection patchCord14(delay1, 0, mixer3, 1);
AudioConnection patchCord15(delay1, 0, mixer4, 0);
AudioConnection patchCord16(delay1, 0, biquad1, 0);
AudioConnection patchCord17(mixer3, delay1);
AudioConnection patchCord18(biquad1, 0, mixer4, 1);
AudioConnection patchCord19(mixer4, 0, usb1, 0);
AudioConnection patchCord20(mixer4, 0, usb1, 1);
AudioConnection patchCord21(mixer4, 0, i2s1, 0);
AudioConnection patchCord22(mixer4, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  //xy=780.1428527832031,292.9999942779541
// GUItool: end automatically generated code

StevesAwesomeButton button1(32, 0, INPUT_PULLUP);
StevesAwesomeButton button2(33, 1, INPUT_PULLUP);
StevesAwesomeButton button3(31, 2, INPUT_PULLUP);
StevesAwesomeButton button4(34, 3, INPUT_PULLUP);
StevesAwesomeButton button5(35, 4, INPUT_PULLUP);

StevesAwesomeButton* buttons[5] = { &button1, &button2, &button3, &button4, &button5 };

int totalButtons = 5;

int bcPin = A17;
int bitcrushAmount = 0;

int ringPin = A10;
int ringModcarrier = 0;

int lfoPin = A11;
int lfoVal = 0;

int potControl[2] = { ringPin, bcPin };

bool bitcrushON = false;
bool userIN = false;
bool delayON = false;
bool changeWAV1 = false;
bool changeWAV2 = false;

short waveforms1[] = {
  WAVEFORM_SINE,
  WAVEFORM_TRIANGLE,
  WAVEFORM_SAWTOOTH,
  WAVEFORM_SQUARE,
  WAVEFORM_PULSE,
  WAVEFORM_SAMPLE_HOLD
};

const char* waveformNames1[] = {
  "Sine", "Triangle", "Sawtooth", "Square", "Pulse", "Sample & Hold"
};

const int NUM_WAVEFORMS1 = sizeof(waveforms1) / sizeof(waveforms1[0]);
int currentIdx_w1 = 0;

short waveforms2[] = {
  WAVEFORM_SINE,
  WAVEFORM_TRIANGLE,
  WAVEFORM_SAWTOOTH,
  WAVEFORM_SQUARE,
  WAVEFORM_PULSE,
  WAVEFORM_SAMPLE_HOLD
};

const char* waveformNames2[] = {
  "Sine", "Triangle", "Sawtooth", "Square", "Pulse", "Sample & Hold"
};

const int NUM_WAVEFORMS2 = sizeof(waveforms2) / sizeof(waveforms2[0]);
int currentIdx_w2 = 0;

const float BASE_DELAY_MS = 250.0;  // Center delay time (ms)
const float FEEDBACK_GAIN = 0.55;   // Echo repeat decay (0.0 to 0.95)
float smoothedDelayMs = BASE_DELAY_MS;

int tempo = 1000;
unsigned long lastStepTime = 0;

void setup() {
  AudioMemory(200);
  Serial.begin(9600);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  if (!display2.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //can't call SCREEN_ADDRESS, just name the address you want to assign
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }  
  Wire1.setClock(1000000);

  for (int i = 0; i < totalButtons; i++) {
    buttons[i]->pressHandler(onButtonPress);
    // buttons[i]->releaseHandler(onButtonRelease);
  }

  

  biquad1.setLowpass(0, 2500, 0.707);
  // use pinMode for Digital Pins

  waveform1.begin(waveforms1[currentIdx_w1]);
  waveform1.frequency(110);
  waveform1.amplitude(0.5);

  waveform2.begin(waveforms2[currentIdx_w2]);
  waveform2.frequency(90);
  waveform2.amplitude(0.5);

  Serial.print("Initial Carrier: ");
  Serial.println(waveformNames2[currentIdx_w2]);

  mixer1.gain(0, 0.0);
  mixer1.gain(1, 0.0);
  mixer1.gain(2, 1.0);
  mixer1.gain(3, 1.0);

  mixer2.gain(0, 0.0);
  mixer2.gain(1, 0.0);
  mixer2.gain(2, 0.0);

  mixer3.gain(0, 1.0);
  mixer3.gain(1, 1.0);

  mixer4.gain(0, 0.0);
  mixer4.gain(1, 0.0);
  mixer4.gain(2, 1.0);
  mixer4.gain(3, 1.0);


  delay1.delay(0, BASE_DELAY_MS);

  // display2.setRotation(0);
  // display2.clearDisplay();
  // display2.display();

  updateDisplay();
  updateDisplay_pic();
  sinewave_gif();
  square_gif();
  pulse_gif();
  sampleHold_gif();
  triangle_gif();
  sawtooth_gif();
}

void loop() {

  for (int i = 0; i < totalButtons; i++) {
    buttons[i]->process();
  }


  lfoVal = analogRead(lfoPin);
  // Map Fader 3 to LFO Speed (0.1 Hz slow drift to 4.0 Hz fast warble)
  // float lfoRateHz = map(lfoVal, 0, 1023, 1, 40) / 10.0;
  float lfoRateHz = map(lfoVal, 0, 1023, 1, 15) / 10.0f;  // 0.1 Hz to 1.5 Hz

  // Map Fader 3 to Modulation Depth (0 ms to 15 ms delay swing)
  // float modDepthMs = map(lfoVal, 0, 1023, 0, 150) / 10.0;
  float modDepthMs = map(lfoVal, 0, 1023, 0, 40) / 10.0f;  // 0.0 ms to 4.0 ms

  // Calculate Software LFO Sine Wave
  unsigned long currentMillis = millis();
  float lfoPeriodMs = 1000.0 / lfoRateHz;
  float lfoPhase = fmod(currentMillis, lfoPeriodMs) / lfoPeriodMs;  // Normalize 0.0 to 1.0
  float lfoValue = sin(lfoPhase * 2.0 * M_PI);

  // Calculate Warbled Delay Time
  float currentDelayMs = BASE_DELAY_MS + (lfoValue * modDepthMs);
  smoothedDelayMs = (smoothedDelayMs * 0.95) + (currentDelayMs * 0.05);

  // Update Delay Tap 0 Read Head dynamically
  delay1.delay(0, smoothedDelayMs);

  delay(10);  // Small loop pacing


  ringModcarrier = analogRead(ringPin);
  float freq = map(ringModcarrier, 0, 1023, 2000, 20);
  // waveform2.frequency(freq);
  waveform2.frequency(freq);
  delay(10);

  bitcrushAmount = map(analogRead(bcPin), 0, 1023, 1, 10);
  Serial.println(bitcrushAmount);
  bitcrusher1.bits(bitcrushAmount);
  delay(50);
  
  updateDisplay();
  updateDisplay_pic();
}

void onButtonPress(int _buttonNum) {

  if (_buttonNum == 1) {
    bitcrushON = !bitcrushON;
    if (bitcrushON == true) {
      Serial.println("bitcrush on");
      mixer1.gain(0, 1.0);  // turn bitcrush channel on
      mixer1.gain(1, 1.0);  // turn bitcrush channel on
      mixer1.gain(2, 0.0);  // turn dry channel off
      mixer1.gain(3, 0.0);  // turn dry channel off

    } else if (bitcrushON == false) {
      Serial.println("bitcrush off");
      mixer1.gain(0, 0.0);  // turn bitcrush channel off
      mixer1.gain(1, 0.0);  // turn bitcrush channel off
      mixer1.gain(2, 1.0);  // turn dry channel on
      mixer1.gain(3, 1.0);  // turn dry channel on
    }
  }

  if (_buttonNum == 0) {
    userIN = !userIN;
    if (userIN == true) {
      Serial.println("user input on");
      mixer2.gain(0, 1.0);  // turn user input channel on
      mixer2.gain(1, 1.0);  // turn user input channel on
      mixer2.gain(2, 0.0);  // turn osclliator channel off

    } else if (userIN == false) {
      Serial.println("user input off");
      mixer2.gain(0, 0.0);  // turn user input channel off
      mixer2.gain(1, 0.0);  // turn user input channel off
      mixer2.gain(2, 1.0);  // turn osclliator channel on
    }
  }

  if (_buttonNum == 2) {
    changeWAV1 = !changeWAV1;
    if (changeWAV1 == true) {
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames1[currentIdx_w1]);
      currentIdx_w1 = (currentIdx_w1 + 1) % NUM_WAVEFORMS1;  // Cycle index
      waveform1.begin(waveforms1[currentIdx_w1]);

    } else if (changeWAV1 == false) {
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames1[currentIdx_w1]);
      currentIdx_w1 = (currentIdx_w1 + 1) % NUM_WAVEFORMS1;  // Cycle index
      waveform1.begin(waveforms1[currentIdx_w1]);
    }
  }

  if (_buttonNum == 3) {
    changeWAV2 = !changeWAV2;



    if (changeWAV2 == true) {
      currentIdx_w2 = (currentIdx_w2 + 1) % NUM_WAVEFORMS2;
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames2[currentIdx_w2]);
        // Cycle index
      waveform2.begin(waveforms2[currentIdx_w2]);

    } else if (changeWAV2 == false) {
      currentIdx_w2 = (currentIdx_w2 + 1) % NUM_WAVEFORMS2;
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames2[currentIdx_w2]);
        // Cycle index
      waveform2.begin(waveforms2[currentIdx_w2]);
    }
    updateDisplay();
    updateDisplay_pic();


  }

  if (_buttonNum == 4) {
    delayON = !delayON;
    if (delayON == true) {
      Serial.println("delay on");
      mixer3.gain(0, 1.0);
      mixer3.gain(1, 1.0);
      mixer4.gain(0, FEEDBACK_GAIN);
      mixer4.gain(1, FEEDBACK_GAIN);
      mixer4.gain(2, 0.0);  // keep dry channel on
      mixer4.gain(3, 0.0);  // turn feedback channel on

    } else if (delayON == false) {
      Serial.println("delay off");
      mixer3.gain(0, 0.0);
      mixer4.gain(0, 0.0);
      mixer4.gain(1, 0.0);
      mixer4.gain(2, 1.0);  // keep dry channel on
      mixer4.gain(3, 1.0);  // turn feedback channel on
    }
  }
}

void updateDisplay(){
    display.clearDisplay();  // Clear display buffer
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(25, 10);
    // display.println(currentIdx_w2);
    if(currentIdx_w2 == 0) {
      display.println("SINE");
    } 
    if(currentIdx_w2 == 1) {
      display.println("TRIANGLE");
    }
    if(currentIdx_w2 == 2) {
      display.println("SAWTOOTH");
    }
    if(currentIdx_w2 == 3) {
      display.println("SQUARE");
    }
    if(currentIdx_w2 == 4) {
      display.println("PULSE");
    } 
    if(currentIdx_w2 == 5) {
      display.println("SAMPLE HOLD");
    }               
    // display.fillRect(0, 15, 60, 40, WHITE);
    display.display();
}

void updateDisplay_pic(){
    display2.setRotation(0);
    display2.clearDisplay();  // Clear display buffer
    display2.setTextSize(3);
    display2.setTextColor(WHITE);
    display2.setCursor(0, 0);
    // display.println(currentIdx_w2);
    if(currentIdx_w2 == 0) {
      // display2.println("FML");
      sinewave_gif();
    } 
    if(currentIdx_w2 == 1) {
      triangle_gif();
    }
    if(currentIdx_w2 == 2) {
      sawtooth_gif();
    }
    if(currentIdx_w2 == 3) {
      square_gif();
    }
    if(currentIdx_w2 == 4) {
      pulse_gif();
    } 
    if(currentIdx_w2 == 5) {
      sampleHold_gif();
    }               
    // display2.fillRect(0, 15, 60, 40, WHITE);
    display2.display();
}

void sinewave_gif() {
  static uint8_t frameIdx = 0;
  static unsigned long lastMs = 0;
  const uint8_t* frames[] = {frame0, frame1, frame2, frame3, frame4, frame5, frame6, frame7};
  const uint8_t numFrames = 8;
  if (millis() - lastMs >= 83) {
    lastMs = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames[frameIdx], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx = (frameIdx + 1) % numFrames;
  }
}

void square_gif() {
  static uint8_t frameIdx_s = 0;
  static unsigned long lastMs_s = 0;
  const uint8_t* frames_s[] = {frame0_s, frame1_s, frame2_s, frame3_s, frame4_s, frame5_s, frame6_s, frame7_s, frame8_s, frame9_s};
  const uint8_t numFrames_s = 10;
  if (millis() - lastMs_s >= 100) {
    lastMs_s = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames_s[frameIdx_s], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx_s = (frameIdx_s + 1) % numFrames_s;
  }
}

void pulse_gif() {
  static uint8_t frameIdx_p = 0;
  static unsigned long lastMs_p = 0;
  const uint8_t* frames_p[] = {frame0_p, frame1_p, frame2_p, frame3_p, frame4_p, frame5_p, frame6_p, frame7_p, frame8_p, frame9_p};
  const uint8_t numFrames_p = 10;
  if (millis() - lastMs_p >= 83) {
    lastMs_p = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames_p[frameIdx_p], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx_p = (frameIdx_p + 1) % numFrames_p;
  }
}

void sampleHold_gif() {
  static uint8_t frameIdx_sh = 0;
  static unsigned long lastMs_sh = 0;
  const uint8_t* frames_sh[] = {frame0_sh, frame1_sh, frame2_sh, frame3_sh, frame4_sh, frame5_sh, frame6_sh, frame7_sh, frame8_sh, frame9_sh, frame10_sh, frame11_sh, frame12_sh, frame13_sh, frame14_sh, frame15_sh, frame16_sh, frame17_sh, frame18_sh, frame19_sh};
  const uint8_t numFrames_sh = 20;
  if (millis() - lastMs_sh >= 50) {
    lastMs_sh = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames_sh[frameIdx_sh], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx_sh = (frameIdx_sh + 1) % numFrames_sh;
  }
}

void triangle_gif() {
  static uint8_t frameIdx_tri = 0;
  static unsigned long lastMs_tri = 0;
  const uint8_t* frames_tri[] = {frame0_tri, frame1_tri, frame2_tri, frame3_tri, frame4_tri, frame5_tri, frame6_tri, frame7_tri, frame8_tri, frame9_tri, frame10_tri, frame11_tri, frame12_tri, frame13_tri, frame14_tri, frame15_tri, frame16_tri, frame17_tri, frame18_tri, frame19_tri};
  const uint8_t numFrames_tri = 20;
  if (millis() - lastMs_tri >= 100) {
    lastMs_tri = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames_tri[frameIdx_tri], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx_tri = (frameIdx_tri + 1) % numFrames_tri;
  }
}

void sawtooth_gif() {
  static uint8_t frameIdx_saw = 0;
  static unsigned long lastMs_saw = 0;
  const uint8_t* frames_saw[] = {frame0_saw, frame1_saw, frame2_saw, frame3_saw, frame4_saw, frame5_saw, frame6_saw, frame7_saw, frame8_saw, frame9_saw, frame10_saw, frame11_saw, frame12_saw, frame13_saw, frame14_saw, frame15_saw};
  const uint8_t numFrames_saw = 16;
  if (millis() - lastMs_saw >= 100) {
    lastMs_saw = millis();
    display2.clearDisplay();
    display2.drawBitmap(0, 0, frames_saw[frameIdx_saw], 128, 64, SSD1306_WHITE);
    display2.display();
    frameIdx_saw = (frameIdx_saw + 1) % numFrames_saw;
  }
}
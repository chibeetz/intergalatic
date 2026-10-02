#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <StevesAwesomeButton.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=214.57142639160156,252.7142791748047
AudioSynthWaveform       waveform1;      //xy=216.71426391601562,285.0389709472656
AudioMixer4              mixer2; //xy=410.0000228881836,278.5714340209961
AudioSynthWaveform       waveform2;      //xy=451.1428527832031,402
AudioEffectMultiply      multiply1;      //xy=593.1428527832031,395.9999942779541
AudioEffectBitcrusher    bitcrusher1;    //xy=786.7142639160156,366.1428413391113
AudioMixer4              mixer1;         //xy=986.7141723632812,398.5714111328125
AudioEffectDelay         delay1;         //xy=1132,201
AudioMixer4              mixer3;         //xy=1141,603
AudioFilterBiquad        biquad1;        //xy=1315.5,177
AudioMixer4              mixer4;         //xy=1351.5,390
AudioOutputUSB           usb1;           //xy=1536.28564453125,301.4285583496094
AudioOutputI2S           i2s1;           //xy=1556.28564453125,383.4285583496094
AudioConnection          patchCord1(i2s2, 0, mixer2, 0);
AudioConnection          patchCord2(i2s2, 1, mixer2, 1);
AudioConnection          patchCord3(waveform1, 0, mixer2, 2);
AudioConnection          patchCord4(mixer2, 0, multiply1, 0);
AudioConnection          patchCord5(waveform2, 0, multiply1, 1);
AudioConnection          patchCord6(multiply1, 0, mixer1, 2);
AudioConnection          patchCord7(multiply1, 0, mixer1, 3);
AudioConnection          patchCord8(multiply1, bitcrusher1);
AudioConnection          patchCord9(bitcrusher1, 0, mixer1, 0);
AudioConnection          patchCord10(bitcrusher1, 0, mixer1, 1);
AudioConnection          patchCord11(mixer1, 0, mixer3, 0);
AudioConnection          patchCord12(mixer1, 0, mixer4, 2);
AudioConnection          patchCord13(mixer1, 0, mixer4, 3);
AudioConnection          patchCord14(delay1, 0, mixer3, 1);
AudioConnection          patchCord15(delay1, 0, mixer4, 0);
AudioConnection          patchCord16(delay1, 0, biquad1, 0);
AudioConnection          patchCord17(mixer3, delay1);
AudioConnection          patchCord18(biquad1, 0, mixer4, 1);
AudioConnection          patchCord19(mixer4, 0, usb1, 0);
AudioConnection          patchCord20(mixer4, 0, usb1, 1);
AudioConnection          patchCord21(mixer4, 0, i2s1, 0);
AudioConnection          patchCord22(mixer4, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=780.1428527832031,292.9999942779541
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

const float BASE_DELAY_MS = 250.0; // Center delay time (ms)
const float FEEDBACK_GAIN  = 0.55;  // Echo repeat decay (0.0 to 0.95)
float smoothedDelayMs = BASE_DELAY_MS;


void setup() {
  AudioMemory(200);
  Serial.begin(9600);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);

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

  // waveform2.begin(WAVEFORM_SINE);
  // waveform2.frequency(440);
  // waveform2.amplitude(0.5);

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

}

void loop() {
  for (int i = 0; i < totalButtons; i++) {
    buttons[i]->process();
  }


  lfoVal = analogRead(lfoPin);
  // Map Fader 3 to LFO Speed (0.1 Hz slow drift to 4.0 Hz fast warble)
  // float lfoRateHz = map(lfoVal, 0, 1023, 1, 40) / 10.0;
  float lfoRateHz  = map(lfoVal, 0, 1023, 1, 15) / 10.0f;  // 0.1 Hz to 1.5 Hz

  // Map Fader 3 to Modulation Depth (0 ms to 15 ms delay swing)
  // float modDepthMs = map(lfoVal, 0, 1023, 0, 150) / 10.0;
  float modDepthMs = map(lfoVal, 0, 1023, 0, 40) / 10.0f;  // 0.0 ms to 4.0 ms

  // Calculate Software LFO Sine Wave
  unsigned long currentMillis = millis();
  float lfoPeriodMs = 1000.0 / lfoRateHz;
  float lfoPhase = fmod(currentMillis, lfoPeriodMs) / lfoPeriodMs; // Normalize 0.0 to 1.0
  float lfoValue = sin(lfoPhase * 2.0 * M_PI);

  // Calculate Warbled Delay Time
  float currentDelayMs = BASE_DELAY_MS + (lfoValue * modDepthMs);
  smoothedDelayMs = (smoothedDelayMs * 0.95) + (currentDelayMs * 0.05);

  // Update Delay Tap 0 Read Head dynamically
  delay1.delay(0, smoothedDelayMs);

  delay(10); // Small loop pacing


  ringModcarrier = analogRead(ringPin);
  float freq = map(ringModcarrier, 0, 1023, 2000, 20);
  // waveform2.frequency(freq);
  waveform2.frequency(freq);
  delay(10);

  bitcrushAmount = map(analogRead(bcPin), 0, 1023, 1, 10);
  Serial.println(bitcrushAmount);
  bitcrusher1.bits(bitcrushAmount);
  delay(50);

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
      currentIdx_w1 = (currentIdx_w1 + 1) % NUM_WAVEFORMS1; // Cycle index
      waveform1.begin(waveforms1[currentIdx_w1]);

    } else if (changeWAV1 == false) {
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames1[currentIdx_w1]);
      currentIdx_w1 = (currentIdx_w1 + 1) % NUM_WAVEFORMS1; // Cycle index
      waveform1.begin(waveforms1[currentIdx_w1]);
    }
  }

    if (_buttonNum == 3) {
    changeWAV2 = !changeWAV2;
    if (changeWAV2 == true) {
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames2[currentIdx_w2]);
      currentIdx_w2 = (currentIdx_w2 + 1) % NUM_WAVEFORMS2; // Cycle index
      waveform2.begin(waveforms2[currentIdx_w2]);

    } else if (changeWAV2 == false) {
      Serial.print("Switched Modulating Waveform to: ");
      Serial.println(waveformNames2[currentIdx_w2]);
      currentIdx_w2 = (currentIdx_w2 + 1) % NUM_WAVEFORMS2; // Cycle index
      waveform2.begin(waveforms2[currentIdx_w2]);
    }
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
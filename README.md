# Planetary Devastation: Space-Themed Hardware Audio FX Processor

A low-latency, real-time embedded DSP audio processor built on the **Teensy 4.1 (ARM Cortex-M7 @ 600 MHz)** platform. The device processes real-time instrument or USB audio through three chained space-themed digital effects controlled via tactile linear faders and software-debounced hardware inputs.

---

## Technical Specifications

* **Microcontroller:** Teensy 4.1 (NXP i.MXRT1062, 32-Bit ARM Cortex-M7)
* **Audio CODEC:** SGTL5000 Audio Shield ($24\text{-bit} / 44.1\text{ kHz}$ I2S I/O)
* **Target Latency:** $< 1.5\text{ ms}$ (buffer size: 128 samples)
* **Controls:** 3x Linear Slide Potentiometers ("Orbital Controls"), 1x Pushbutton
* **I/O Options:** Dual-channel 3.5mm Stereo Jack, Line Out pads, and Class-Compliant USB Audio Interface for Ableton Live / DAW integration.

---

## Audio Signal Architecture

```text
[ Audio Input ] ──> [ Solar Flare ] ──> [ Black Hole ] ──> [ Gravity Drift ] ──> [ Output ]
 (Line-In / USB)     (Ring Modulator)     (Bitcrusher)     (Modulated Delay)    (I2S / USB)
                          ▲                   ▲                   ▲
                          │                   │                   │
                     [ Button ]          [ Fader A1 ]        [ Fader A2 ]
                   (Wave Selection)      (Resolution)       (LFO Warble)
```

1. **Solar Flare (Ring Modulator):** Multiplies audio input with an array of carrier waveforms (Sine, Sawtooth, Triangle, Square, Pulse, Sample & Hold) switched via software-debounced hardware button.
2. **Black Hole (Bitcrusher):** Dynamic resolution reduction down to 1-bit quantization and sample-rate decimation for arcade digital destruction.
3. **Gravity Drift (Modulated Delay):** Variable read-pointer delay buffer with exponential moving average (EMA) parameter smoothing and a low-pass filtered feedback loop for analog tape warble without digital zipper noise.

---

## Physical Controls & Parameter Mapping

| Control | Hardware Pin | Parameter Target | Range / Options |
| :--- | :--- | :--- | :--- |
| **Pushbutton** | Pin 2 (`INPUT_PULLUP`) | Solar Flare Waveform Select | Sine $\rightarrow$ Tri $\rightarrow$ Saw $\rightarrow$ Square $\rightarrow$ S&H |
| **Fader 1** | Pin A0 | Solar Flare Carrier Pitch | $20\text{ Hz} - 2000\text{ Hz}$ |
| **Fader 2** | Pin A1 | Black Hole Bit Depth | $16\text{-bit} \rightarrow 1\text{-bit}$ resolution |
| **Fader 3** | Pin A2 | Gravity Drift LFO Speed & Depth | $0.1\text{ Hz} - 1.5\text{ Hz}$ rate / $0\text{ ms} - 4\text{ ms}$ swing |

---

## Hardware Setup & Build Instructions

### Prerequisites
* [Arduino IDE](https://www.arduino.cc/en/software) with [Teensyduino add-on installed](https://www.pjrc.com/teensy/td_download.html) OR [VS Code](https://code.visualstudio.com/) with **PlatformIO**.

### IDE Configuration
1. Connect Teensy 4.1 via Micro-USB.
2. Select **Board:** `Teensy 4.1`.
3. Select **USB Type:** `Audio` (or `Serial + MIDI + Audio`).
4. Set **CPU Speed:** `600 MHz`.
5. Compile and flash `src/main.cpp`.

---

## Automated Verification Suite

An automated Python testing script (`test/verify_audio.py`) profiles signal fidelity using `numpy` and `scipy`:
* **Latency Profile:** Impulse response timing measuring total buffer transit delay.
* **Frequency Response:** Logarithmic sweep profiling low-pass feedback attenuation.
* **THD+N:** Total Harmonic Distortion measurement across dry and ring-modulated outputs.
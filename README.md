

# Teensy Audio Library Extras

This library is a small suite of useful modules designed to work with and extend the Teensy audio library (https://github.com/PaulStoffregen/Audio) for the Teensy3.x and Teensy 4.x family of ARM based microcontrollers.

## Description

Currently the Teensy Audio Library allows different modules (AudioStream objects) to be patched together - IE the output of one module can be connected to the input of another. However, only audio rate signals can be patched between modules. This library will allow for data types other than 16 bit integers to be passed and received by downstream AudioStream objects. The idea is for the patching capabilities to shift towards a Pure Data paradigm where different data types can be connected and form a complete data flow using AudioStream objects.

I've introduced 2 data types that can be passed to specific modules. The first pertains to midi note information. 2 bytes are passed for a midi note's pitch and velocity. This data is only updated once at every Update call (every 2.9 ms roughly). The second is a boolean type use to represent gates and trigger events.

Here's list of the modules so far:
- midiNoteInput: reads usb midi data and transmits midi note data downstream.
- New ADSR envelope with logarithmic and exponential curve shapes / responsive to gate input for note on/off events.
- Clock pulse generator. Sends trigger events to downstream objects based on tempo and PPQN set.
- Euclidean generator. Sends euclidean rhythm trigger events using clock trigger input.
- Random sequence generator. Generates random pitches at each clock trigger event received.
- Looping Sequencer. Midi note data sequence can be recorded and played back.
- Modified synth waveform class. Can now read midi note information at input to set waveform frequency.
- Scale Quantizer. Force midi data onto selected scale.


## Getting Started

### Dependencies

All set up steps detailed on the teensy website should be taken first.
https://www.pjrc.com/teensy/tutorial.html 

### Installing


* Clone this directory.
* In Teensyduino IDE go to Sketch -> Include Library -> Add .zip Library



// #include <Arduino.h>
// #include <Audio.h>
// #include <ADSR_envelope.h>

// #define SIXTEENTH_NOTE_DURATION 125


// AudioSynthWaveformModulated wave;
// AudioOutputI2S i2s;
// AudioEffectMultiply vca2;


// // ExpLogCurveTable testCurveTable[5] =   {ExpLogCurveTable (2,  0), ExpLogCurveTable (1.5,  0), ExpLogCurveTable (1,  0), ExpLogCurveTable (1.5,  1), ExpLogCurveTable (2,  1)};
// // AudioADSREnvelope newEnvelope(&testCurveTable[0], 1);
// AudioADSREnvelope newEnvelope;

// AudioConnection patch2(wave, 0, vca2, 0);
// AudioConnection patch1(newEnvelope , 0, vca2, 1 );
// AudioConnection patch3(vca2, 0, i2s, 0);
// AudioConnection patch4(vca2, 0, i2s, 1);  

// int seed;

// void setup() {
//   // put your setup code here, to run once:

//   // TGA_PRO_MKII_REV1(); // Declare the version of the TGA Pro you are using.
//   //TGA_PRO_REVB(x);
//   //TGA_PRO_REVA(x);

//   delay(5); // wait a few ms to make sure the GTA Pro is fully powered up
//   AudioMemory(48); // Provide an arbitrarily large number of audio buffers (48 blocks) for the effects (delays use a lot more than others)

//   // If the codec was already powered up (due to reboot) power it down first
//   // codecControl.disable();
//   delay(100);
//   // codecControl.enable();
//   delay(100);

  

//   wave.begin(1.0f, 60, WAVEFORM_TRIANGLE);

//   newEnvelope.attack(5);
//   newEnvelope.decay(200);
//   newEnvelope.sustain(0.0f);
//   newEnvelope.release(1000);
//   newEnvelope.noteRetriggerRelease(5);

//   seed = analogRead(1);
//   // randomSeed(seed);


// }


// void oldloop()
// {
  
//   newEnvelope.attack(5);
//   newEnvelope.release(random(1000));
//   newEnvelope.noteOn();
//   delay(100);
//   newEnvelope.noteOff();  
//   delay(2000);

// }

// void loop() {
//   // put your main code here, to run repeatedly:
  
//   seed = analogRead(1);
  

//   for(int i = 0; i < 4; i++)
//   {
//     randomSeed(seed);

//     for (int j = 0; j < 2; j++)
//     { 
      
//       int delay1 = SIXTEENTH_NOTE_DURATION * random(5);
//       int delay2 = SIXTEENTH_NOTE_DURATION * random(5);
//       int delay3 = SIXTEENTH_NOTE_DURATION *  8 - delay1 - delay2;

//       newEnvelope.attack(random(5, 100));
//       newEnvelope.decay(random(50, 800));
//       newEnvelope.noteOn();
//       wave.frequency(random(50,80));
//       delay(delay1);

//       newEnvelope.attack(random(5, 100));
//       newEnvelope.decay(random(50, 800));
//       newEnvelope.noteOn();
//       wave.frequency(random(50,80));
//       delay(delay2);

//       newEnvelope.attack(random(5, 100));
//       newEnvelope.decay(random(50, 800));
//       newEnvelope.noteOn();
//       wave.frequency(random(50,80));
//       delay(delay3);

//     }
//   }
// }
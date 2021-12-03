/*
Live sequencer recorder AUdiostream derived class

Records midi note input in a sequence. It then plays back as a loop.
Resolution of the sequencer is 3ms.

This module is at the proof of concept stage.


Connections:
Port    Purpose
In 0	Midi Note Data
Out 0	Midi Note Data



*/


#include <Arduino.h>
#include <Audio.h>
#include <MidiData.h>


#define BEATSPERBAR 4
#define SECSPERMINUTE 60
#define SAMPLINGRATE 44100/128.0f

#define POLYPHONY 2
#define SEQUENCE_STEPS 1024


#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif


#ifndef SEQUENCER_H
#define SEQUENCER_H

enum sequenceStates {
    stopped,
    playing,
    armed,
    recording,
    overdubbing
};

enum sequenceType {
    events,
    continuous
};


typedef struct midi_data
{
    midiVoice_t midiVoice[POLYPHONY];
}midi_data_t;


class LiveSequencer : public AudioStream
{
    public:
    LiveSequencer()
    : sequencerState(stopped),
    nbOfVoices(2),
    AudioStream(1, inputQueueArray),
    lowerBPMRange(70)
    {};
    
    void update(void) override;

    void updateSequence();

    void record(){recordFlag = !recordFlag;}
    void record(bool rec){recordFlag = rec;}

    void play(){playFlag = !playFlag;}
    void play(bool play){playFlag = play;}

    bool firstInputReceived(){};

    void resetSequence();

    void setTempo();

    void setSequenceParameters();

    void checkForClockPulse();

    void checkFunctionFlags();




    private:


    audio_block_t *inputQueueArray[1];

    uint8_t sequencerType;
    uint16_t lowerBPMRange;
    uint8_t sequenceLengthBars;
    uint8_t sequencerState;

    float *externalTempo;
    float tempoRatio;

    //global sequencer variables
    int nbOfVoices;



    //saved sequence variables
    float sequenceTempo;

    midi_data_t sequenceData [SEQUENCE_STEPS];


    // midi_data_t sequenceData;
    uint32_t timeStamp[SEQUENCE_STEPS] ;
    uint32_t sequenceLength;


    bool recordFlag, playFlag;
    int step;
    uint32_t sequenceClock;
    int clockPulsePPQN = 1;
    float clockPulseExpectedInterval;
    bool externalClockSync;


    
    //update should be called by the audiostream and run every 2.9ms
    uint32_t bufferUpdateCounter;
    uint32_t fineTick;
    uint8_t tickMultiplier = 128;
    



};



#endif
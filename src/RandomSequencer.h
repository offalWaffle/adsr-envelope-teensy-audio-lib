/*

Random pitch generator module to work with the AudioSynthWaveformSineModulated 
and AudioSynthWaveformModulated modules. 

A new random pitch is generate when a trigger is received at the input. Random voltage 
are quantized to specified scale.

Connections:
Port	Purpose
In 0	trigger
Out 0	Envelope 



*/

#include <Audio.h>

#define SEMITONE_STEP 455.111111111111111
#define OCTAVE_STEP 5461.333333333333333
#define MIDDLE_C_MIDI_NOTE 60
#define MIDDLE_C_FREQ 261.626

enum scales
{
    chromatic,
    major,
    minor

};

enum modes
{  
    forward,
    reverse,
    randomstep, 
    skip

};

class RandomSequencer : public AudioStream
{
    public:
    RandomSequencer();
    void update(void) override;
    void nextSeqStep();
    void randomiseRegister();
    void setSeed(int s){seed = s;}
    void setScale(int scale);
    int quantizeNote(int note);


    private:
    audio_block_t *inputQueueArray[1];
    bool inletState;

    int16_t registerStep[32];
    int16_t quantizedOutput;
    int currentStep;
    int sequencerMode;
    int sequenceLength;
    int seed;


    int majorScale[12] = {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11};
    int minorScale[12] = {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10};
    int chromaticScale[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

    int currentScale[12];

    int getOctave(int note);
    int getPitch(int note);


};
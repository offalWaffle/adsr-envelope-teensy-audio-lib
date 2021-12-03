/*
Audiostream derived class to transform midi note data and quantize it to defined scale

Connections:
Port	Purpose
In 0	Midi Note Data
Out 0	Midi Note Data

*/

#include <Audio.h>
#include <MidiData.h>

enum scales
{
    chromatic,
    major,
    minor

};

class ScaleQuantize : public AudioStream
{
    public:
    ScaleQuantize();

    void update(void) override;
    void updateMidiInput();
    int quantizeMidiPitch(int midiNote);
    void setScale(int scale);
    

    private:
    audio_block_t inputQueueArray[1];
    midiVoice_t *midiData;

    int polyphony;

    int majorScale[12] = {0, 0, 2, 2, 4, 5, 5, 7, 7, 9, 9, 11};
    int minorScale[12] = {0, 0, 2, 3, 3, 5, 5, 7, 8, 8, 10, 10};
    int chromaticScale[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

    int currentScale[12];

};
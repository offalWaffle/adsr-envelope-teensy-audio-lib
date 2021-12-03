
/*

This module gets midi usb from the USB host port on the Teensy and outputs midi note data
to downstream audiostream objects.

Reading midi usb data every time the audiocallback is called means any midi data received in the
previous buffer processing window are used in the current audio block. This does mean there is
likely to be some midi jitter (from 0 to 3ms) but it is fairly minimal.

Connections:
Port	Purpose
Out 0	Midi Note Data


*/


#include <Audio.h>
#include <USBHost_t36.h>
#include <MidiData.h>


class MidiNoteIn : public AudioStream
{
    public:
    MidiNoteIn(int nbOfVoices = 4);

    ~MidiNoteIn()
    {
      delete[] midiVoice;
    }

    void update();
    int getNextAvailableMidiVoice();
    void turnCorrespondingVoiceOff(int pitch);


    private:
    USBHost myusb;
    USBHub hub1;
    USBHub hub2;
    MIDIDevice midi1;
    midiVoice_t * midiVoice;

    int polyphony;


};
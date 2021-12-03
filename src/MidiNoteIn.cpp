#include <MidiNoteIn.h>

#define CIRCULAR_ARRAY_SIZE 8

static bool gate[CIRCULAR_ARRAY_SIZE];
static byte notes[CIRCULAR_ARRAY_SIZE];
static byte velocities[CIRCULAR_ARRAY_SIZE];
static int circularArrayIndex = 0;


void myNoteOn(byte channel, byte note, byte velocity) 
{
  gate[circularArrayIndex] = 1;
  notes[circularArrayIndex] = note;
  velocities[circularArrayIndex] = velocity;
  circularArrayIndex = (circularArrayIndex + 1) % 8;
}

void myNoteOff(byte channel, byte note, byte velocity) {

  gate[circularArrayIndex] = 0;
  notes[circularArrayIndex] = note;
  velocities[circularArrayIndex] = 0;

  circularArrayIndex = (circularArrayIndex + 1) % 8;

}


MidiNoteIn::MidiNoteIn(int nbOfVoices = 4)
: hub1(myusb),
hub2(myusb),
midi1(myusb),
AudioStream(0, NULL),
polyphony(nbOfVoices)
{


  midiVoice = new midiVoice_t[nbOfVoices]();

  // Wait 1.5 seconds before turning on USB Host.  If connected USB devices
  // use too much power, Teensy at least completes USB enumeration, which
  // makes isolating the power issue easier.
  delay(1500);
  Serial.println("USB Host InputFunctions example");
  delay(10);
  myusb.begin();

  midi1.setHandleNoteOn(myNoteOn);
  midi1.setHandleNoteOff(myNoteOff);




}

void MidiNoteIn::update()
{

  int8_t *m;
  int voiceNb;

  for(int i = 0; i < 4; i++)
  {
    midi1.read() ;
    
  } 


  audio_block_t *midiNoteData;

  midiNoteData = allocate();

  if(!midiNoteData) return;



  //save gate data into midivoice
  if(circularArrayIndex)
  {

    m = (int8_t *) midiNoteData->data;
  
    //cycle through midi note on / off messages received
    for (int i = 0; i < circularArrayIndex; i++)
    {
      
      if (!velocities[i]) turnCorrespondingVoiceOff(notes[i]);
      voiceNb = getNextAvailableMidiVoice();
      
      midiVoice[voiceNb].noteNumber = notes[i];
      midiVoice[voiceNb].velocity = gate[i] ? velocities[i] : 0;
  

    }

    //using the data array in audio_block_t, transmit 2 bytes per voice (note pitch + velocity)
    // saved serially in the array
    for (int k = 0; k < polyphony; k++)
    {
      *m++ = midiVoice[k].noteNumber;
      *m++ = midiVoice[k].velocity;
    }

    transmit(midiNoteData, 0);




  }


  circularArrayIndex = 0;

  release(midiNoteData);


}

void MidiNoteIn::turnCorrespondingVoiceOff(int pitch)
{
  for (int j = 0; j < 2; j++)
  {
    if(midiVoice[j].noteNumber == pitch) midiVoice[j].velocity = 0;
  }
}


int MidiNoteIn::getNextAvailableMidiVoice()
{
  //return next available free note
  for (int j = 0; j < polyphony; j++)
  {
    if(!midiVoice[j].velocity) return j;
  }

  // or steal highest note
  int highestNote, highestVoice;
  highestNote = midiVoice[0].noteNumber;
  highestVoice = 0;
  for (int j = 1; j < polyphony; j++)
  {
    if(midiVoice[j].noteNumber > highestNote) 
    {
      highestNote = midiVoice[j].noteNumber;
      highestVoice = j;
    }
  }
  return highestVoice;

}




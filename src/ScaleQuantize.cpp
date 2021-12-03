#include <ScaleQuantize.h>

ScaleQuantize::ScaleQuantize()
: AudioStream(1, inputQueueArray)
{

}

void ScaleQuantize::update()
{
    audio_block_t *midiData;

	midiData = receiveWritable(0);
	if(!midiData) return;

    midiVoice_t *midiVoiceData = (midiVoice_t *) midiData->data;

	for (int l = 0; l < polyphony; l++)
	{
        midiVoiceData.noteNumber = quantizeMidiPitch(midiVoiceData.noteNumber);
        midiVoiceData++;
	}

    transmit(midiData);
	release(midiData);

}


int ScaleQuantize::quantizeMidiPitch(int midiNote)
{

    Serial.print("note: "); Serial.println(note);
    int tmp = currentScale[note % 12] + 12 * ((note < 0 ? note - 12 : note) / 12) ;
    Serial.print("quantized: "); Serial.println(tmp);
    return tmp;

}

void ScaleQuantize::setScale(int scale)
{
    switch (scale)
    {
        case chromatic:
        memcpy(currentScale,chromaticScale , sizeof(int [12]) );
        break;
        case major:
        memcpy(currentScale,majorScale , sizeof(int [12]) );
        break;
        case minor:
        memcpy(currentScale, minorScale , sizeof(int [12]) );
        break;    
    }

}
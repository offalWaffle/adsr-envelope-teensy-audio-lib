#include <RandomSequencer.h>

RandomSequencer::RandomSequencer()
: AudioStream(1, inputQueueArray),
seed(1),
sequenceLength(8)
{

    setScale(minor);
    randomiseRegister();

}

void RandomSequencer::update()
{
    audio_block_t * clockPulse;
    audio_block_t * out;

    clockPulse = receiveReadOnly(0);
    if(!clockPulse) return;

    out = allocate();
    if(!out) 
    {   
        release(clockPulse);
        return;
    }

    int16_t * trig = clockPulse->data;
    int16_t * o = out->data;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if(inletState == LOW && *trig > 16384 ) 
        {
            inletState = HIGH;
            nextSeqStep();
            
            quantizedOutput = quantizeNote (registerStep[currentStep]) * SEMITONE_STEP;
        }
        else if (inletState == HIGH && *trig < 16384)
        {
            inletState = LOW;

        }

        *o++ = quantizedOutput; 
        trig++;
    }
    
    transmit(out);
    release(clockPulse);
    release(out);


  
}

void RandomSequencer::randomiseRegister()
{
    randomSeed(seed);
    for(int i = 0; i < 32; i++)
    {

        int tmpRnd = random (0 , 12);
        registerStep[i] = (tmpRnd);// * SEMITONE_STEP;

    }

}


int RandomSequencer::getPitch(int note)
{
    
    return (note) % 12;


}

int RandomSequencer::getOctave(int note)
{
    return ((note - MIDDLE_C_MIDI_NOTE) / 12) + 3;
}


int RandomSequencer::quantizeNote(int note)
{
    Serial.print("note: "); Serial.println(note);
    int tmp = currentScale[note % 12] + 12 * ((note < 0 ? note - 12 : note) / 12) ; 
    Serial.print("quantized: "); Serial.println(tmp);
    return tmp;
}

void RandomSequencer::setScale(int scale)
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

void RandomSequencer::nextSeqStep()
{
    

    switch (sequencerMode)
    {
        case forward:
        currentStep = (currentStep + 1) % sequenceLength;
        break;

    }

}
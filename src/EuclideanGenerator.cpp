#include <EuclideanGenerator.h>

EuclideanGenerator::EuclideanGenerator()
: AudioStream(1, inputQueueArray),
seed(9),
sequenceLength(8),
nbEuclideanTriggers(5)

{

    setEuclideanSequence();

}

void EuclideanGenerator::update()
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
        }
        else if (inletState == HIGH && *trig < 16384)
        {
            inletState = LOW;
        }

        *o++ = registerStep[currentStep] ? *trig : 0; 
        trig++;
    }
    
    transmit(out);
    release(clockPulse);
    release(out);


  
}

void EuclideanGenerator::randomiseRegister()
{
    randomSeed(seed);
    int factor = 2;
    for(int i = 0; i < 32; i++)
    {
        registerStep[i] =  random (1);//random(32768.0f / );
    }

}

void EuclideanGenerator::nextSeqStep()
{

    switch (sequencerMode)
    {
        case egforward:
        currentStep = (currentStep + 1) % sequenceLength;
        break;

    }

}

void EuclideanGenerator::clearSequence()
{
    for(int i = 0; i < sizeof(registerStep); i++)
    {
        registerStep[0];
    }
}

void EuclideanGenerator::setEuclideanSequence()
{

    clearSequence();

    if (!nbEuclideanTriggers) return;

    registerStep[0] = HIGH;

    float accurateEuclideanInterval =  (float)sequenceLength / nbEuclideanTriggers;
    
    int j = 1;
    int nextEuclideanTrigger = j * accurateEuclideanInterval;
    while (nextEuclideanTrigger < sequenceLength)
    {
        registerStep[nextEuclideanTrigger] = HIGH;
        nextEuclideanTrigger = ++j * accurateEuclideanInterval;
    }


    for (int i = 0; i < sequenceLength; i++)
    {
        Serial.println(registerStep[i]);

    }
    

}
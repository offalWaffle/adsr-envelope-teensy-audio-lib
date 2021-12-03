#include "Sequencer.h"



void LiveSequencer::checkFunctionFlags()


{
    switch (sequencerState)
    {
    case stopped:
        
        if(recordFlag) 
        {
            sequencerState = armed;
            Serial.println("seq set to armed");
        }

        if(playFlag)
        {
            sequencerState = playing;
            step = 0;
            Serial.println("seq set to play");
        } 
        break;

    case armed:

        if(!recordFlag) 
        {
            sequencerState = stopped;
        }
        break;

    case recording:

        if(playFlag) 
        {
            sequencerState = playing;
            Serial.println("seq finish record and set to play");
            delay(1);

            timeStamp[step] = bufferUpdateCounter;
            
            //sequenceLength + bpm
            sequenceLength = timeStamp[step];
            recordFlag = 0;
            step = 0;

            bufferUpdateCounter = 0;


        }
        break;
    case playing:

        if (!playFlag)
        {
            sequencerState = stopped;
        }
        break;

    }

}


void LiveSequencer::update()
{

    bufferUpdateCounter++;
    checkFunctionFlags();

    audio_block_t *inputBlock, *playbackBlock, *outputBlock;
    inputBlock = receiveReadOnly(0);

    midi_data_t *inputData = (midi_data_t *) inputBlock->data;

    switch (sequencerState)
    {
    case stopped:
        
        //set output pointer to input for through data
        outputBlock = inputBlock;
        break;
    
    case armed:


        delay(10);

        


        // how do we tell that the first note is played?
        if(inputBlock)
        {
            Serial.println("seq set to record");
            delay(10);
            sequencerState = recording;
            // record data
            step = 0;
            sequenceData[step] = *inputData;
            timeStamp[step] = 0;
            bufferUpdateCounter = 0;
            step++;
            
            outputBlock = inputBlock;
            transmit(outputBlock);
            release(inputBlock);

        }


        break;

    case recording:
 
        // record data    
        if(inputBlock)
        {
            //record data
            sequenceData[step] = *inputData;
            timeStamp[step] = bufferUpdateCounter;
            // pass through data while recording
            outputBlock = inputBlock;

            transmit(outputBlock);
            release(inputBlock);
            setSequenceParameters();
            step++;
        }

        break;
    
    case playing:

        if (bufferUpdateCounter >= timeStamp[step])
        {

            if (bufferUpdateCounter >= sequenceLength)
            {
                step = 0;
                bufferUpdateCounter = 0;

            }
            
            playbackBlock = allocate();
            midi_data_t *playbackData = (midi_data_t *) playbackBlock->data;

            *playbackData = sequenceData[step];

            transmit(playbackBlock);
            release(playbackBlock);

            step++;

        }
        

        break;


        
    
    default:
        break;
    }


    // to do reset coarsetick before overflow

}


void LiveSequencer::setSequenceParameters()
{
    sequenceLength = timeStamp[step];

    int oneBarLengthAtLowerRange = SAMPLINGRATE * SECSPERMINUTE * BEATSPERBAR / lowerBPMRange;
    sequenceLengthBars = sequenceLength / oneBarLengthAtLowerRange; 
    
    sequenceTempo = SECSPERMINUTE * sequenceLengthBars * BEATSPERBAR * SAMPLINGRATE / sequenceLength ;

    clockPulseExpectedInterval = SECSPERMINUTE * clockPulsePPQN / sequenceTempo;

    
}
/* Audio Library for Teensy 3.X
 * Copyright (c) 2017, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "ADSR_envelope.h"

#define STATE_IDLE	0
#define STATE_ATTACK	1
#define STATE_DECAY	2
#define STATE_SUSTAIN	3
#define STATE_RELEASE	4
#define STATE_FORCED	5

#define LOG_CURVE 0
#define EXP_CURVE 1



// TO DO retrigger noteon should have 2 options: start attack from current amplitude 
//												or reset to 0 then retrigger from 0 amplitude

// TO DO exponential scaler (in a different module) for setting time values (small increments at low values, bigger steps higher up)
// TO DO global time factor to change attack/decay/release time with one knob. (like ableton time param)
// TO DO measure performance vs teensy lib envelope
// TO DO noteon + noteoff changes to change member variables only at start of audio block processing (when update is called)
// this allows for potential synchronicity between audio block updates (when retriggering samplegrain object for example)



AudioADSREnvelope::AudioADSREnvelope() 
	: AudioStream(0, NULL)

	
{

	curveShapeTable = new ExpLogCurveTable[3] {ExpLogCurveTable(1.5,  LOG_CURVE), ExpLogCurveTable(1,  LOG_CURVE), ExpLogCurveTable(1.5, EXP_CURVE)};
	setAttackCurve(0);
	setDecayCurve(2);
	setReleaseCurve(2);




	state = 0;
	attack(10.5f);
	decay(35.0f);
	sustain(0.5f);
	release(300.0f);
	noteRetriggerRelease(5.0f);




	// Serial.println("what??");
	delay(10);
	
}


AudioADSREnvelope::AudioADSREnvelope(ExpLogCurveTable* ELCT, int nbOfCurveShapes) 
	: AudioStream(0, NULL) ,
	curveShapeTable(ELCT) ,
	totalCurveShapes(nbOfCurveShapes)
	{
		state = 0;
		attack(10.5f);
		decay(35.0f);
		sustain(0.5f);
		release(300.0f);
		noteRetriggerRelease(5.0f);

		setAttackCurve(0);
		setDecayCurve(1);
		setReleaseCurve(1);
		delay(10);
		
	}




void AudioADSREnvelope::noteOn(void)
{
	__disable_irq();
	if (state == STATE_IDLE || release_forced_count == 0) {
		startAmplitude = 0;
		peakAmplitude = MAXAMPLITUDE;
		// sustainAmplitude = level * peakAmplitude;

		Serial.println("Note On!!  ");
		
		
		state = STATE_ATTACK;
		count = attack_count;
		
		
	} else if (state != STATE_FORCED) {
		state = STATE_FORCED;
		count = release_forced_count;
		inc_hires = (-mult_hires) / (int32_t)count;
	}
	__enable_irq();
}

void AudioADSREnvelope::noteOff(void)
{
	Serial.println("Note Off!!  ");
	__disable_irq();
	if (state != STATE_IDLE && state != STATE_FORCED) {

		

		state = STATE_RELEASE;
		count = release_count;
		inc_hires = (-mult_hires) / (int32_t)count;

		releaseStartAmplitude = targetAmplitude;
	}
	__enable_irq();
}

void AudioADSREnvelope::update(void)
{
	audio_block_t *block;
	uint16_t *p, *end;
	uint32_t sample12, sample34, sample56, sample78, tmp1, tmp2;

	uint16_t sample[8];


	
	// Serial.println("Still idle!!  ");

	block = allocate();
	if (!block) return;
	if (state == STATE_IDLE) {
		
		release(block);
		return;
	}
	p = (uint16_t *)(block->data);
	end = p + AUDIO_BLOCK_SAMPLES;

	

	while (p < end) {
		// we only care about the state when completing a region
		
		if (state == STATE_ATTACK) {
			

			

			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = peakAmplitude * curveShapeTable[attackCurve].getCurveLookupValue((count / (float)attack_count));

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;


			// Serial.println(count);
			// Serial.print(0);Serial.print(", "); Serial.print(targetAmplitude); Serial.print(", "); Serial.println(MAXAMPLITUDE);



			if (count == 0) 
			{
				// Serial.println("Progress check ");
				// Serial.print("Decay!!  ");  
				state = STATE_DECAY;
				count = decay_count;
			}

			// continue;

		} else if (state == STATE_DECAY) {

			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = sustainAmplitude + (peakAmplitude - sustainAmplitude) * curveShapeTable[decayCurve].getCurveLookupValue(count / (float)decay_count);

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;

			// Serial.print(0);Serial.print(", "); Serial.print(targetAmplitude); Serial.print(", "); Serial.println(MAXAMPLITUDE);


			if (count == 0) 
			{
				state = STATE_SUSTAIN;
				// Serial.println("Sustain!!  ");
				count = 0xFFFF;
			}
			
		} else if (state == STATE_SUSTAIN) {
			
			targetAmplitude = sustainAmplitude;
			perSampleIncrement = 0;
			count = 0xFFFF;

			// Serial.print(0);Serial.print(", "); Serial.print(targetAmplitude); Serial.print(", "); Serial.println(MAXAMPLITUDE);


		} else if (state == STATE_RELEASE) {

			// Serial.println("release!!  ");
			
			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = releaseStartAmplitude * curveShapeTable[releaseCurve].getCurveLookupValue(count / (float)release_count);

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;

			// Serial.print(0);Serial.print(", "); Serial.print(targetAmplitude); Serial.print(", "); Serial.println(MAXAMPLITUDE);


			if (count == 0) 
			{
				state = STATE_IDLE;
				count = 0xFFFF;

				// Serial.println("Idle!!  ");
			}


			
			
		} else if (state == STATE_IDLE) {
			
			
			targetAmplitude = 0;
			perSampleIncrement = 0;
			count = 0xFFFF;
			

			// Serial.println("Idle case!!  ");
			

		} else if (state == STATE_FORCED) {
			
			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = releaseStartAmplitude * curveShapeTable[releaseCurve].getCurveLookupValue(count / (float)release_forced_count);

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;
			
			
			if (count == 0)
			{
				startAmplitude = 0;
				peakAmplitude = MAXAMPLITUDE;
				state = STATE_ATTACK;
				count = attack_count;

			}
		} 
		

		// sample[0] = startAmplitude * maxAmplitude;

		
		*p++ = startAmplitude;

		Serial.println(startAmplitude);

		uint16_t mult = startAmplitude;

		for (int i = 1; i < 8; i++)
		{
			mult = mult + perSampleIncrement;

			*p++ = mult; 
			Serial.println(mult);

		}

		startAmplitude = targetAmplitude;



		count--;
	}
	transmit(block);
	release(block);
}

bool AudioADSREnvelope::isActive()
{
	uint8_t current_state = *(volatile uint8_t *)&state;
	if (current_state == STATE_IDLE) return false;
	return true;
}

bool AudioADSREnvelope::isSustain()
{
	uint8_t current_state = *(volatile uint8_t *)&state;
	if (current_state == STATE_SUSTAIN) return true;
	return false;
}




ExpLogCurveTable::ExpLogCurveTable (float base, bool curveType)
: powBase (base),
multFactor (MULTIPLICATIONFACTOR),
curveType (curveType)
{

    generateLookupValues();


}

void ExpLogCurveTable::generateLookupValues()
{
    //curveLookupTable;


	float ceiling = pow(powBase, multFactor - 1) +  (multFactor - 1) * pow(powBase, -1);

    if (curveType == LOG_CURVE)
    {
        for (int i = 0; i < TABLERESOLUTION + 1; i++)
        {

            // curveLookupTable[i] = 1 - ( pow(powBase, i/1 - tableResolution - 1) +  (i/1 - tableResolution - 1) * pow(powBase, -1));
            
            
            // curveLookupTable[i] = 1 - ( pow(powBase, (tableResolution - i)/tableResolution - 1) +  ((tableResolution - i)   /tableResolution - 1) * pow(powBase, -1));
        
			curveLookupTable[i] =  1 - ((pow(powBase, (i * multFactor / TABLERESOLUTION) - 1) +  ((i * multFactor / TABLERESOLUTION) - 1) * pow(powBase, -1)) / ceiling);
			

            // Serial.println("curveLookupTable[i]");
            // Serial.println(curveLookupTable[i]);
        
        }
            
            


            

        

    }
    else if (curveType == EXP_CURVE)
    {
        for (int i = 0; i < TABLERESOLUTION + 1; i++)
        {

        
			// curveLookupTable[i] = ( pow(powBase, i/tableResolution - 1) +  (i/tableResolution - 1) * pow(powBase, -1));

			curveLookupTable[i] = (pow(powBase, (i * multFactor / TABLERESOLUTION) - 1) +  ((i * multFactor / TABLERESOLUTION) - 1) * pow(powBase, -1)) / ceiling;
			

                // Serial.println("curveLookupTable[i]");
            // Serial.println(curveLookupTable[i]);

        }




    }


// tablelookup - 1 dimensional array with results from exponential curve formula generated in constructor





}



float ExpLogCurveTable::getCurveLookupValue (float input)
{






    // input value expected between 0-1. scaledinput = multiply input by the table resolution

    float scaledInput = input * TABLERESOLUTION;

    // convert scaledinput to an int intScaledinput

    int intScaledInput = (int)scaledInput;

    // float map function. use scaledinput as input. lowerinput =  intScaledinput, upperinput = intScaledinput + 1
    // use tablelookup(intScaledinput) and tablelookup(intScaledinput + 1) as lower and upper output


    return mapFloat(scaledInput, (float) intScaledInput, (float) (intScaledInput + 1), curveLookupTable[intScaledInput], intScaledInput + 1 > TABLERESOLUTION ? curveLookupTable[intScaledInput] : curveLookupTable[intScaledInput + 1]);

        



}

float ExpLogCurveTable::getRateAtLookupValue (float input)
{






    // input value expected between 0-1. scaledinput = multiply input by the table resolution

    float scaledInput = input * TABLERESOLUTION;
// convert scaledinput to an int intScaledinput

    int intScaledInput = (int)scaledInput;

    // float map function. use scaledinput as input. lowerinput =  intScaledinput, upperinput = intScaledinput + 1
    // use tablelookup(intScaledinput) and tablelookup(intScaledinput + 1) as lower and upper output

    return (intScaledInput + 1 > TABLERESOLUTION ? curveLookupTable[intScaledInput] : curveLookupTable[intScaledInput + 1] - curveLookupTable[intScaledInput]) * TABLERESOLUTION;


        



}



float ExpLogCurveTable::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
  
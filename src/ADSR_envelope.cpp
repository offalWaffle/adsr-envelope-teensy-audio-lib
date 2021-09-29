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

#define DEFAULT_CURVE_BASE 1.6

// TO DO retrigger noteon should have 2 options: start attack from current amplitude 
//												or reset to 0 then retrigger from 0 amplitude

// TO DO exponential scaler (in a different module) for setting time values (small increments at low values, bigger steps higher up)
// TO DO global time factor to change attack/decay/release time with one knob. (like ableton time param)
// TO DO measure performance vs teensy lib envelope
// TO DO noteon + noteoff changes to change member variables only at start of audio block processing (when update is called)
// this allows for potential synchronicity between audio block updates (when retriggering samplegrain object for example)




AudioADSREnvelope::AudioADSREnvelope(uint8_t voice_number = 0) 
	: AudioStream(1, inputQueueArray),
	totalCurveShapes(3), voiceNumber(voice_number)


	
{

	curveShapeTable = new ExpLogCurveTable[totalCurveShapes] {ExpLogCurveTable(DEFAULT_CURVE_BASE,  LOG_CURVE), //logarithmic
											   ExpLogCurveTable(1,  EXP_CURVE), //linear
											   ExpLogCurveTable(DEFAULT_CURVE_BASE, EXP_CURVE)}; //exponential
	
	curveTablesOnTheHeap = true;

	setAttackCurve(0);
	setDecayCurve(2);
	setReleaseCurve(2);

	state = 0;
	attack(10.5f);
	decay(35.0f);
	sustain(0.5f);
	release(300.0f);
	noteRetriggerRelease(5.0f);

	Serial.println("built adsr");

	
}
AudioADSREnvelope::~AudioADSREnvelope()
{
	if(curveTablesOnTheHeap) delete[] curveShapeTable;
}

AudioADSREnvelope::AudioADSREnvelope(ExpLogCurveTable* ELCT, int nbOfCurveShapes, uint8_t voice_number = 0) 
	: AudioStream(0, NULL) ,
	curveShapeTable(ELCT) ,
	totalCurveShapes(nbOfCurveShapes),
	voiceNumber(voice_number)
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
		state = STATE_ATTACK;
		count = attack_count;
		
		
	} else if (state != STATE_FORCED) {
		state = STATE_FORCED;
		count = release_forced_count;
		releaseStartAmplitude = targetAmplitude;
		
	}
	__enable_irq();
}

void AudioADSREnvelope::noteOff(void)
{

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


	updateMidiInput();

	// delay(1);

	//************
	//set audioblock pointer and allocate

	// triggerInlet = receiveReadOnly(0);
    // if(!triggerInlet) return;
	// trig = triggerInlet->data;


	

	block = allocate();
	if (!block) return;
	if (state == STATE_IDLE) {
		
		// release(block);
		// return;
	}
	p = (uint16_t *)(block->data);
	end = p + AUDIO_BLOCK_SAMPLES;



	while (p < end) {

		//************



		if (state == STATE_ATTACK) {
		
			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = peakAmplitude * curveShapeTable[attackCurve].getCurveLookupValue(((attack_count - count) / (float)attack_count));

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;


			if (count == 0) 
			{ 
				state = STATE_DECAY;
				count = decay_count;
			}


		} else if (state == STATE_DECAY) {

			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = sustainAmplitude + (peakAmplitude - sustainAmplitude) * curveShapeTable[decayCurve].getCurveLookupValue(count / (float)decay_count);

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;

			if (count == 0) 
			{
				state = STATE_SUSTAIN;
				count = 0xFFFF;
			}
			
		} else if (state == STATE_SUSTAIN) {
			
			targetAmplitude = sustainAmplitude;
			perSampleIncrement = 0;
			count = 0xFFFF;

		} else if (state == STATE_RELEASE) {

			// get target value for the next 8 samples using the curve lookup table
			targetAmplitude = releaseStartAmplitude * curveShapeTable[releaseCurve].getCurveLookupValue(count / (float)release_count);

			//increase rate linear over 8 samples
			perSampleIncrement = (targetAmplitude - startAmplitude) / 8;

			if (count == 0) 
			{
				state = STATE_IDLE;
				count = 0xFFFF;

			}

			
		} else if (state == STATE_IDLE) {
			
			
			targetAmplitude = 0;
			perSampleIncrement = 0;
			count = 0xFFFF;

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
		

		*p++ = startAmplitude;
		uint16_t mult = startAmplitude;

		for (int i = 1; i < 8; i++)
		{
			mult = mult + perSampleIncrement;
			*p++ = mult; 
		}

		startAmplitude = targetAmplitude;
		count--;
	}


	transmit(block);

	release(block);
}


void AudioADSREnvelope::updateMidiInput()
{
	
	
  
	audio_block_t *midiData;

	midiData = receiveReadOnly(0);
	if(!midiData) return;

	
	


	int8_t *md = (int8_t *) midiData->data;

	int pitch;
	int velocity;

	for (int l = 0; l < voiceNumber; l++)
	{
		md++;
		md++;
	}
	pitch = *md++;
	velocity = *md;

	Serial.print("voiceNumber");Serial.println(voiceNumber);
	Serial.print("pitch");Serial.println(pitch);
	Serial.print("velocity");Serial.println(velocity);

	if (velocity) noteOn();
	else noteOff();

	// noteNumber = (int8_t *) midiData->data;
	// Serial.print("mididata: ");
	// Serial.println(*noteNumber++);
	// Serial.println(*noteNumber++);
	// Serial.println(*noteNumber++);
	// Serial.println(*noteNumber++);

	release(midiData);
	
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

	float ceiling = pow(powBase, multFactor - 1) +  (multFactor - 1) * pow(powBase, -1);

    if (curveType == LOG_CURVE)
    {
		
        for (int i = 0; i < TABLERESOLUTION + 1; i++)
        {
			curveLookupTable[TABLERESOLUTION - i] =  1 - ((pow(powBase, (i * multFactor / TABLERESOLUTION) - 1) +  ((i * multFactor / TABLERESOLUTION) - 1) * pow(powBase, -1)) / ceiling);
		}

    }
    else if (curveType == EXP_CURVE)
    {

        for (int i = 0; i < TABLERESOLUTION + 1; i++)
        {
			curveLookupTable[i] = (pow(powBase, (i * multFactor / TABLERESOLUTION) - 1) +  ((i * multFactor / TABLERESOLUTION) - 1) * pow(powBase, -1)) / ceiling;
        }

    }




}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


float ExpLogCurveTable::getCurveLookupValue (float input)
{

    // input value expected between 0-1. scaledinput = multiply input by the table resolution
    float scaledInput = input * TABLERESOLUTION;

    // convert scaledinput to an int intScaledinput
    int intScaledInput = (int)scaledInput;

    return mapFloat(scaledInput, (float) intScaledInput, (float) (intScaledInput + 1), curveLookupTable[intScaledInput], intScaledInput + 1 > TABLERESOLUTION ? curveLookupTable[intScaledInput] : curveLookupTable[intScaledInput + 1]);

}



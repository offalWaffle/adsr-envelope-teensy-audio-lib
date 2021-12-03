#include <Arduino.h>
#include "ADSR_envelope.h"



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
		releaseStartAmplitude = targetAmplitude;
	}
	__enable_irq();
}

void AudioADSREnvelope::update(void)
{
	audio_block_t *block;
	uint16_t *p, *end;

	//get midi input data
	updateMidiInput();

	block = allocate();
	if (!block) return;
	if (state == STATE_IDLE) {
		
		release(block);
		return;
	}

	p = (uint16_t *)(block->data);
	end = p + AUDIO_BLOCK_SAMPLES;

	//iterate through samples in buffer
	while (p < end) {


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


	//read midi data from input buffer
	int8_t *md = (int8_t *) midiData->data;

	int pitch;
	int velocity;

	//skip to relevant voice number
	for (int l = 0; l < voiceNumber; l++)
	{
		md++;
		md++;
	}

	pitch = *md++;
	velocity = *md;

	if (velocity) noteOn();
	else noteOff();

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



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

#ifndef adsr_envelope_h_
#define adsr_envelope_h_
#include "Arduino.h"
#include "AudioStream.h"
#include "utility/dspinst.h"

#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)
#define MAXAMPLITUDE 0b0111111111111111 //2's complement max value

#define TABLERESOLUTION 100
#define MULTIPLICATIONFACTOR 10 //multiplication factor used to keep exponential function from overflowing



class ExpLogCurveTable
{
    public:
	ExpLogCurveTable ();
    ExpLogCurveTable (float base, bool curveType );


    void generateLookupValues ();
    void setBase(float base){powBase = base;}
    

    float getCurveLookupValue (float input);
    

    private:
	float powBase;
    float multFactor;
    float curveLookupTable[TABLERESOLUTION];

    bool curveType; //0 is exponential, 1 is logarithmic
    


};

class AudioADSREnvelope : public AudioStream
{
public:
	AudioADSREnvelope(); 
	AudioADSREnvelope(ExpLogCurveTable* ELCT, int nbOfCurveShapes);

	~AudioADSREnvelope();

	void noteOn();
	void noteOff();
	
	void attack(float milliseconds) {
		attack_count = milliseconds2count(milliseconds);
		if (attack_count == 0) attack_count = 1;
	}
	void decay(float milliseconds) {
		decay_count = milliseconds2count(milliseconds);
		if (decay_count == 0) decay_count = 1;
	}
	void sustain(float level) {
		if (level < 0.0) level = 0;
		else if (level > 1.0) level = 1.0;
		sustain_mult = level * 1073741824.0;
		sustainAmplitude = level * MAXAMPLITUDE;
	}
	void release(float milliseconds) {
		release_count = milliseconds2count(milliseconds);
		if (release_count == 0) release_count = 1;
	}
	void noteRetriggerRelease(float milliseconds) {
		release_forced_count = milliseconds2count(milliseconds);
		if (release_count == 0) release_count = 1;
	}

	void setAttackCurve(uint8_t curve) {
		attackCurve = curve;
	}

	void setDecayCurve(uint8_t curve) {
		decayCurve = curve;
	}

	void setReleaseCurve(uint8_t curve) {
		releaseCurve = curve;
	}

	bool isActive();
	bool isSustain();
	using AudioStream::release;
	virtual void update(void);
private:
	uint16_t milliseconds2count(float milliseconds) {
		if (milliseconds < 0.0) milliseconds = 0.0;
		uint32_t c = ((uint32_t)(milliseconds*SAMPLES_PER_MSEC)+7)>>3;
		if (c > 65535) c = 65535; // allow up to 11.88 seconds
		return c;
	}
	audio_block_t *inputQueueArray[1];
	// state
	uint8_t  state;      // idle, attack, decay, sustain, release, forced
	uint16_t count;      // how much time remains in this state, in 8 sample units
	int32_t  mult_hires; // attenuation, 0=off, 0x40000000=unity gain
	int32_t  inc_hires;  // amount to change mult_hires every 8 samples

	// settings
	uint16_t attack_count;
	uint16_t decay_count;
	int32_t  sustain_mult;
	uint16_t release_count;
	uint16_t release_forced_count;


	ExpLogCurveTable *curveShapeTable;
	uint8_t totalCurveShapes;
	uint8_t attackCurve, decayCurve, releaseCurve;
	uint16_t targetAmplitude, startAmplitude, perSampleIncrement;
	uint16_t peakAmplitude, releaseStartAmplitude, sustainAmplitude;
	bool curveTablesOnTheHeap;


	audio_block_t * triggerInlet;
	int16_t * trig;
	bool inletState;
	void checkInletState();
	




};


#undef SAMPLES_PER_MSEC
#endif

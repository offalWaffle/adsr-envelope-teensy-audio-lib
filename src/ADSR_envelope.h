/* 
ADSR envelope audiostream derived class
This is an enhancement to the envelope class in the Teensy Audio Library as it allows users to set
the type of curve for the attack, decay and release phases of the envelope to either logarithmic, linear or exponential.
This makes for more natural sounding attack and decay characteristics.

Audio signal input has been removed and only generates an audio rate control signal. To apply this envelope
to an audio rate signal, use a AudioEffectMultiply object with the envelope signal in 1 input and the
audio signal in the other. This matches more closely the EG -> VCA paradigm used in synth design. 

The ExpLogCurveTable class is used to generate a lookup table that is generated at instantiation. 

Connections:
Port	Purpose
In 0	Midi Note Data
Out 0	Envelope 



 */

#ifndef adsr_envelope_h_
#define adsr_envelope_h_
#include "Arduino.h"
#include "AudioStream.h"
#include "utility/dspinst.h"

#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)
#define MAXAMPLITUDE 0b0111111111111111 //2's complement max value

#define TABLERESOLUTION 100 // curve lookup table resolution
#define MULTIPLICATIONFACTOR 10 //multiplication factor used to keep exponential function from overflowing

#define STATE_IDLE	0
#define STATE_ATTACK	1
#define STATE_DECAY	2
#define STATE_SUSTAIN	3
#define STATE_RELEASE	4
#define STATE_FORCED	5

#define LOG_CURVE 0
#define EXP_CURVE 1

#define DEFAULT_CURVE_BASE 1.6 // this base number provides a 

class ExpLogCurveTable
{
    public:
    ExpLogCurveTable (float base, bool curveType ); //constructor

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
	// this constructor generates 3 default curves (log, lin and exp) using DEFAULT_CURVE_BASE value
	AudioADSREnvelope(uint8_t voice_number = 0); 

	//this contructor allows the user to pass any number of user defined ExpLogCurveTable objects
	AudioADSREnvelope(ExpLogCurveTable* ELCT, int nbOfCurveShapes, uint8_t voice_number = 0);

	~AudioADSREnvelope();

	void noteOn();
	void noteOff();

	void updateMidiInput();
	
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

	// counters
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

	void checkInletState();
	uint8_t voiceNumber; 

};


#undef SAMPLES_PER_MSEC
#endif

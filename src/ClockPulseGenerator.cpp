#include <ClockPulseGenerator.h>

#define SAMPLE_INTERVAL AUDIO_SAMPLE_RATE_EXACT/AUDIO_BLOCK_SAMPLES

ClockPulseGenerator::ClockPulseGenerator()
:AudioStream(0, NULL),
tempoBpm(120),
PPQN(1),
pulseDurationSmpl(300)
{
    sampleInterval = 1 / AUDIO_SAMPLE_RATE_EXACT;
    setClockInterval();

}

ClockPulseGenerator::ClockPulseGenerator(float bpm, int pulsePerQuarterNote)
:AudioStream(0, NULL),
tempoBpm(bpm),
PPQN(pulsePerQuarterNote),
pulseDurationSmpl(300)
{

    sampleInterval = 1 / AUDIO_SAMPLE_RATE_EXACT;
    setClockInterval();



}

void ClockPulseGenerator::setTempo(float bpm)
{
    tempoBpm = bpm;
    setClockInterval();
}

void ClockPulseGenerator::setClockInterval()
{
    clockIntervalSmpl = (((30.0f / tempoBpm) / sampleInterval) / PPQN);
    Serial.print("tempoBpm: ");Serial.println(tempoBpm);
    Serial.print("sampleInterval: ");Serial.println(sampleInterval);
    Serial.print("PPQN: ");Serial.println(PPQN);
    Serial.print("clockIntervalSmpl: ");Serial.println(clockIntervalSmpl);

}

void ClockPulseGenerator::update()
{

    audio_block_t *block;
    block = allocate();

    if(!block) return;

    int16_t *p = block->data;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if (sampleCounter > clockIntervalSmpl) sampleCounter = 0;

        if(sampleCounter < pulseDurationSmpl) *p++ = MAXAMPLITUDE;
        else *p++ = 0;

        sampleCounter++;

    }

    transmit(block);
    release(block);

}

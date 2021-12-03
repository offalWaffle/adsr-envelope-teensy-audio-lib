/*
Clock pulse generator Audiostream class

Outputs a clock pulse based on tempo and PPQN. Used to control sequencing modules like 
EuclideanGenerator and RandomSequencer

Connections:
Port	Purpose
Out 0	Clock out 

*/

#include <Audio.h>

#define MAXAMPLITUDE 0b0111111111111111 //2's complement max value

class ClockPulseGenerator : public AudioStream
{
    public:
    ClockPulseGenerator();
    ClockPulseGenerator(float bpm, int pulsePerQuarterNote);
    void update(void) override;
    void setClockInterval();
    void setTempo(float bpm);
    void resetClock();


    private:
    float tempoBpm;
    int PPQN;
    int clockIntervalSmpl;
    int pulseDurationSmpl;
    int sampleCounter;
    float sampleInterval;



};
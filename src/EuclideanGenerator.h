/*
Euclidean clock generator derived Audiostream class

Generates euclidean rhythm clock pulse based on settings


Connections:
Port	Purpose
In 0	Clock input
Out 0	trigger output
*/

#include <Audio.h>

enum egModes
{  
    egforward,
    egreverse,
    egrandomstep, 
    egskip

};



class EuclideanGenerator : public AudioStream
{
    public:
    EuclideanGenerator();
    void update(void) override;
    void nextSeqStep();
    void randomiseRegister();
    void setEuclideanSequence();
    void clearSequence();
    void setSeed(int s){seed = s;}


    private:
    audio_block_t *inputQueueArray[1];
    bool inletState;

    bool registerStep[32];
    int currentStep;
    int sequencerMode;
    int sequenceLength;
    int seed;
    int nbEuclideanTriggers;



};
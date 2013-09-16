/*
 *  gabor.h
 *  mp
 *
 *  Created by Chris on 01/11/2011.
 *  Copyright 2011 Goldsmiths Creative Computing. All rights reserved.
 *
 */


#include <iostream>
#include "maximilian.h"
#include <list>
#include <vector>
#include <valarray>
#include "maxiGrains.h"
#include <map>
#ifndef TARGET_OS_IPHONE
#include "maxiAtomKernel.h"
#endif
#include <set>


using namespace std;

//
class maxiAtomKernel;

enum maxiAtomTypes {
	GABOR
};

struct maxiAtom {
	maxiAtomTypes atomType;
	maxiType length;
	maxiType position;
	maxiType amp;
	static bool atomSortPositionAsc(maxiAtom* a, maxiAtom* b) {return a->position < b->position;}
};

struct maxiGaborAtom : maxiAtom {
	maxiType frequency;
	maxiType phase;
};

//create atoms
class maxiCollider {
public:
	static inline void createGabor(valarray<maxiType> &atom, const maxiType freq, const maxiType sampleRate, const unsigned int length,
                                maxiType phase, const maxiType kurtotis, const maxiType amp);
    static maxiGrainWindowCache<gaussianWinFunctor> envCache;
};

class maxiAtomWindowCache {
public:
    maxiType * getWindow(int length);
protected:
    map<int, valarray<maxiType> > windows;
};

//queue atoms into an audio stream
class maxiAccelerator {
public:
	maxiAccelerator();
//	void addAtom(valarray<maxiType> &atom, long offset=0);
    void addAtom(const maxiType freq, const maxiType phase, const maxiType sampleRate, const unsigned int length, const maxiType amp, const unsigned int offset);
	void fillNextBuffer(float *buffer, unsigned int bufferLength);
#ifndef TARGET_OS_IPHONE
	void fillNextBuffer_OpenCL(float *buffer, unsigned int bufferLength);
	void fillNextBuffer_OpenCLBatch(float *buffer, unsigned int bufferLength);
	void fillNextBuffer_OpenCLBatch2(float *buffer, unsigned int bufferLength);
#endif
	inline long getSampleIdx(){return sampleIdx;}
    inline maxiAccelerator& setShape(maxiType val) {shape = val;}
    inline maxiAccelerator& setAtomCountLimit(int val){atomCountLimit = val;}
    void precacheWindows(set<int> &windowSizes);
private:
	long sampleIdx;
	struct queuedAtom {
//        valarray<float> atom;
		long startTime;
		maxiType phase;
        unsigned int length;
        maxiType amp;
        float pos;
        float freq;
//        int offset;
        maxiType *env;
        maxiType phaseInc;
        maxiType maxPhase;
	};
	typedef list<queuedAtom> queuedAtomList;
	queuedAtomList atomQueue;
    valarray<maxiType> gabor;
    maxiAtomWindowCache winCache;
    maxiType shape;
    int atomCountLimit;
#ifndef TARGET_OS_IPHONE
    maxiAtomKernel clKernel;
    std::vector<structAtomData> atomDataBlock;
#endif
    std::vector<float> atomAmps, atomPhases, atomPhaseIncs;
    std::vector<int> atomPositions, atomLengths;
};

/*load a book in MPTK XML format
 http://mptk.irisa.fr/
 
 how to create a book:
 mpd -n 1000 -R 10 -d ./dic_gabor_two_scales.xml glockenspiel.wav book.xml
*/
class maxiAtomBook {
public:
	~maxiAtomBook();
	typedef vector<maxiAtom*> maxiAtomBookData;
    int numSamples;
	int sampleRate;
    float maxAmp;
	maxiAtomBookData atoms;
    set<int> windowSizes;
	static bool loadMPTKXmlBook(string filename, maxiAtomBook &book, maxiAccelerator &accel, bool verbose = false);
	
};

class maxiAtomBookPlayer {
public:
	maxiAtomBookPlayer();
	void play(maxiAtomBook &book, maxiAccelerator &atomStream);
    inline maxiAtomBookPlayer &setLengthMod(maxiType val) {lengthMod = val;}
    inline maxiAtomBookPlayer &setFreqMod(maxiType val) {freqMod = val;}
    inline maxiAtomBookPlayer &setProbability(maxiType val) {probability = val;}
    inline maxiAtomBookPlayer &setLowFreq(maxiType val) {lowFreq = val;}
    inline maxiAtomBookPlayer &setHighFreq(maxiType val) {highFreq = val;}
    inline maxiAtomBookPlayer &setLowAmp(maxiType val) {lowAmp = val;}
    inline maxiAtomBookPlayer &setHighAmp(maxiType val) {highAmp = val;}
    inline maxiAtomBookPlayer &setPlaybackSpeed(maxiType val) {playbackSpeed = val;}
    inline maxiAtomBookPlayer &setGap(maxiType val) {gap = val;}
    inline maxiAtomBookPlayer &setSnapRange(maxiType val){snapRange = val; snapInvRange = 1.0 / snapRange;}
    inline maxiAtomBookPlayer &setSnapFreqs(vector<maxiType> &freqs){snapFreqs = freqs;}
protected:
    void queueAtomsBetween(maxiAtomBook &book, maxiAccelerator &atomStream, long start, long end, int blockOffset);
	maxiType atomIdx;
    maxiType lengthMod;
    maxiType probability;
    maxiType lowFreq, highFreq;
    maxiType lowAmp, highAmp;
    maxiType freqMod;
    maxiType playbackSpeed;
    maxiType gap;
    double loopedSamplePos;
    vector<maxiType> snapFreqs;
    maxiType snapRange, snapInvRange;
};


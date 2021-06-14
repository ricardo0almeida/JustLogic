#pragma once
#include "JL_components.hpp"
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelProtoA;
extern Model* modelNoteToLogic;
extern Model* modelIfStatement;
extern Model* modelSwitchStatement;


struct JL_signals
{
	int nActiveChannels;
	float cv[16] = {};
};

enum ComparationType {
	LT,		// <
	LE,		// <=
	EQ,		// ==
	GE,		// >=
	GT		// >
};

void calcNoteOctave(float ctrlVolt, int* pitchNote, int* octave);

bool CompareCvToNote(float ctrlVolt, int selectedPitchNote, int selectedOctave, ComparationType selectedComparation);


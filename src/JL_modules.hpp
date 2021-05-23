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

void calcNoteOctave(float ctrlVolt, int* pitchNote, int* octave);


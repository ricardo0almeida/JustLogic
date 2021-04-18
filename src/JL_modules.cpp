#include "JL_modules.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelProtoA);
	p->addModel(modelNoteToLogic);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loadSing assets and lookup tables when your module is created to reduce startup times of Rack.
}

void calcNoteOctave(float ctrlVolt, int* pitchNote, int* octave) {
	eucDivMod((int)std::round(ctrlVolt * 12.0f), 12, octave, pitchNote);
}
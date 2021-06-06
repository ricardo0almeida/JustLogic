#include "JL_modules.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelProtoA);
	p->addModel(modelNoteToLogic);
	p->addModel(modelIfStatement);
	p->addModel(modelSwitchStatement);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loadSing assets and lookup tables when your module is created to reduce startup times of Rack.
}

void calcNoteOctave(float ctrlVolt, int* pitchNote, int* octave) {
	eucDivMod((int)std::round(ctrlVolt * 12.0f), 12, octave, pitchNote);
}

bool CompareCvToNote(float ctrlVolt, int selectedPitchNote, int selectedOctave, ComparationType selectedComparation){
		int inPitchNote;
		int inOctave;
		int CVWeight;
		int selectedNoteWeight;
		calcNoteOctave(ctrlVolt, &inPitchNote, &inOctave);
		inOctave += 4;

		if (selectedOctave == 10)
		{
			CVWeight = inPitchNote;
			selectedNoteWeight = selectedPitchNote;
		}else{
			CVWeight = (100 * inOctave) + inPitchNote;
			selectedNoteWeight = (100 * selectedOctave) +  selectedPitchNote;
		}

		switch (selectedComparation)
		{
		case LT:
			return (CVWeight < selectedNoteWeight);
			break;
		case LE:
			return (CVWeight <= selectedNoteWeight);
			break;
		case EQ:
			return (CVWeight == selectedNoteWeight);
			break;
		case GE:
			return (CVWeight >= selectedNoteWeight);
			break;
		case GT:
			return (CVWeight > selectedNoteWeight);
			break;
		
		default:
			return false;
			break;
		}

		
	}
#include "JL_modules.hpp"

static const char noteLetters[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
static const char isSharpNote[12] = { 0,   1,   0,   1,   0,   0,   1,   0,   1,   0,   1,   0 };


struct ProtoA : Module {
	enum ParamIds {
		NOTE1_PARAM,
		OCTAVE1_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LOGICOUT1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int inPitchNote;
	int inOctave;
	int selectedPitchNote;
	int selectedOctave;
	bool result = false;

	bool showDisplay = true;
	char noteStr[4] = {'\0', '\0', '\0', '\0'};

	const float trueValue = 10.f;
	const float falseValue = 0.f;

	

	ProtoA() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NOTE1_PARAM, 0.f, 11.f, 0.f, "Note");
		configParam(OCTAVE1_PARAM, 0.f, 10.f, 4.f, "Octave");
	}

	void process(const ProcessArgs& args) override {
		selectedPitchNote = (int)(params[NOTE1_PARAM].getValue());
		selectedOctave = (int)(params[OCTAVE1_PARAM].getValue());

		// display selected note
		noteStr[0] = noteLetters[selectedPitchNote];
		int i = 1;
		if (selectedOctave < 10)
		{
			noteStr[i] = selectedOctave + 48;	//ASCII char offset
			i++;
		}
		if (isSharpNote[selectedPitchNote])
		{
			noteStr[i] = '"';
		}else{
			noteStr[i] = '\0';
		}
		i++;
		noteStr[i] = '\0';
		
		// Logic calculation
		if (inputs[PITCH1_INPUT].isConnected())
		{

			calcNoteOctave(inputs[PITCH1_INPUT].getVoltage(), &inPitchNote, &inOctave);
			inOctave += 4;

			if (selectedOctave == 10)
			{
				if (selectedPitchNote == inPitchNote)
				{
					result = true;
				}else{
					result = false;
				}
				
			}else{
				if ((selectedPitchNote == inPitchNote) && (selectedOctave == inOctave))
				{
					result = true;
				}else{
					result = false;
				}
			}
			
			if (result)
			{
				outputs[LOGICOUT1_OUTPUT].setVoltage(trueValue);
			}else{
				outputs[LOGICOUT1_OUTPUT].setVoltage(falseValue);
			}

		}
		
	}
};


struct ProtoAWidget : ModuleWidget {
	ProtoAWidget(ProtoA* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ProtoA.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(28.046, 30.919)), module, ProtoA::NOTE1_PARAM));
		addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(47.89, 30.919)), module, ProtoA::OCTAVE1_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.953, 30.919)), module, ProtoA::PITCH1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(69.813, 30.919)), module, ProtoA::LOGICOUT1_OUTPUT));

		JLThreeDigitDisplayWidget *display = new JLThreeDigitDisplayWidget();
		display->box.pos = Vec(85,120);
		display->box.size = Vec(55, 24);
		if(module){
			display->text = module->noteStr;
			display->on = &module->showDisplay;
		} 
		addChild(display);
	}
};


Model* modelProtoA = createModel<ProtoA, ProtoAWidget>("ProtoA");
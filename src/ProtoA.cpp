#include "JL_modules.hpp"


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

	int pitchNote;
	int octave;
	bool result = false;

	const float trueValue = 10.f;
	const float falseValue = 0.f;

	ProtoA() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NOTE1_PARAM, 0.f, 11.f, 0.f, "Note");
		configParam(OCTAVE1_PARAM, 0.f, 10.f, 4.f, "Octave");
	}

	void process(const ProcessArgs& args) override {
		if (inputs[PITCH1_INPUT].isConnected())
		{

			calcNoteOctave(inputs[PITCH1_INPUT].getVoltage(), &pitchNote, &octave);
			octave += 4;

			if (params[OCTAVE1_PARAM].getValue() == 10.f)
			{
				if (params[NOTE1_PARAM].getValue() == (float)pitchNote)
				{
					result = true;
				}else{
					result = false;
				}
				
			}else{
				if ((params[NOTE1_PARAM].getValue() == (float)pitchNote) && (params[OCTAVE1_PARAM].getValue() == (float)octave))
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
	}
};


Model* modelProtoA = createModel<ProtoA, ProtoAWidget>("ProtoA");
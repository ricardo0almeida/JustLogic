#include "JL_modules.hpp"
#define N_COMP 5

static const char noteLetters[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
static const char isSharpNote[12] = { 0,   1,   0,   1,   0,   0,   1,   0,   1,   0,   1,   0 };

struct NoteToLogic : Module {
	enum ParamIds {
		/*NOTE1_PARAM,
		OCTAVE1_PARAM,
		COMP1_PARAM,*/
		ENUMS(NOTE_PARAM, N_COMP),
		ENUMS(OCTAVE_PARAM, N_COMP),
		ENUMS(COMP_PARAM, N_COMP),
		NUM_PARAMS
	};
	enum InputIds {
		//PITCH1_INPUT,
		ENUMS(PITCH_INPUT, N_COMP),
		NUM_INPUTS
	};
	enum OutputIds {
		//LOGICOUT1_OUTPUT,
		//LOGICOUTNOT1_OUTPUT,
		ENUMS(LOGICOUT_OUTPUT, N_COMP),
		ENUMS(LOGICOUTNOT_OUTPUT, N_COMP),
		NUM_OUTPUTS
	};
	enum LightIds {
		//LEDACTIV1_LIGHT,
		ENUMS(LEDACTIV_LIGHT, N_COMP),
		NUM_LIGHTS
	};

	bool showDisplay = true;
	//char noteStr[6] = {'\0', '\0', '\0', '\0'};
	char noteStr[N_COMP][6];

	NoteToLogic() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		/*configParam(NOTE1_PARAM, 0.f, 11.f, 0.f, "Note");
		configParam(OCTAVE1_PARAM, 0.f, 10.f, 4.f, "Octave");
		configParam(COMP1_PARAM, 0.f, 4.f, 2.f, "Comparation");*/
		for (int i = 0; i < N_COMP; i++)
		{
			configParam(NOTE_PARAM + i, 0.f, 11.f, 0.f, "Note");
			configParam(OCTAVE_PARAM + i, 0.f, 10.f, 4.f, "Octave");
			configParam(COMP_PARAM + i, 0.f, 4.f, 2.f, "Comparation");
		}
		
	}

	void process(const ProcessArgs& args) override {
		// display selected note
		for (int i = 0; i < N_COMP; i++)
		{
			// Set comparation type {<, <=, =, >=, >}
			int CompType = (int)(params[COMP_PARAM + i].getValue());
			switch (CompType)
			{
			case 0:
				noteStr[i][0] = ' ';
				noteStr[i][1] = '<';
				break;
			case 1:
				noteStr[i][0] = '<';
				noteStr[i][1] = '=';
				break;
			case 2:
				noteStr[i][0] = ' ';
				noteStr[i][1] = '=';
				break;
			case 3:
				noteStr[i][0] = '>';
				noteStr[i][1] = '=';
				break;
			case 4:
				noteStr[i][0] = ' ';
				noteStr[i][1] = '>';
				break;
			
			default:
				noteStr[i][0] = ' ';
				noteStr[i][1] = '=';
				break;
			}
			// Set pitch note
			noteStr[i][2] = noteLetters[(int)(params[NOTE_PARAM + i].getValue())];

			// Set octave
			int k = 3;
			if (params[OCTAVE_PARAM + i].getValue() < 10.f)
			{
				noteStr[i][k] = ((int)(params[OCTAVE_PARAM + i].getValue())) + 48;	//ASCII char offset
				k++;
			}

			// Set sharp char
			if (isSharpNote[(int)(params[NOTE_PARAM + i].getValue())])
			{
				noteStr[i][k] = '"';
			}else{
				noteStr[i][k] = '\0';
			}

			// Set end of string
			k++;
			noteStr[i][k] = '\0';
			
		}
		

	}
};


struct NoteToLogicWidget : ModuleWidget {
	NoteToLogicWidget(NoteToLogic* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NoteToLogic.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		/*addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(21.0, 31.0)), module, NoteToLogic::NOTE1_PARAM));
		addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(38.0, 31.0)), module, NoteToLogic::OCTAVE1_PARAM));
		addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(55.0, 31.0)), module, NoteToLogic::COMP1_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.307, 30.919)), module, NoteToLogic::PITCH1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(100.556, 30.919)), module, NoteToLogic::LOGICOUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(114.556, 30.919)), module, NoteToLogic::LOGICOUTNOT1_OUTPUT));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(12.588, 28.088)), module, NoteToLogic::LEDACTIV1_LIGHT));

		// mm2px(Vec(29.8, 8.0))
		//addChild(createWidget<Widget>(mm2px(Vec(63.0, 27.0))));
		JLDisplayWidget *display = new JLDisplayWidget();
		display->box.pos = mm2px(Vec(63,27));
		display->box.size = Vec(88, 24);
		if(module){
			display->text = module->noteStr;
			display->on = &module->showDisplay;
		} 
		addChild(display);*/

		static const int topFirstRow = 31;
		static const int rowGap = 19;

		for (int i = 0; i < N_COMP; i++)
		{
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(21.0, topFirstRow + (i * rowGap))), module, NoteToLogic::NOTE_PARAM + i));
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(38.0, topFirstRow + (i * rowGap))), module, NoteToLogic::OCTAVE_PARAM + i));
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(55.0, topFirstRow + (i * rowGap))), module, NoteToLogic::COMP_PARAM + i));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.307, topFirstRow + (i * rowGap))), module, NoteToLogic::PITCH_INPUT + i));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(101.5, topFirstRow + (i * rowGap))), module, NoteToLogic::LOGICOUT_OUTPUT + i));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.5, topFirstRow + (i * rowGap))), module, NoteToLogic::LOGICOUTNOT_OUTPUT + i));

			addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(12.588, topFirstRow + (i * rowGap) - 3)), module, NoteToLogic::LEDACTIV_LIGHT + i));

			// mm2px(Vec(29.8, 8.0))
			//addChild(createWidget<Widget>(mm2px(Vec(63.0, 27.0))));
			JLDisplayWidget *display = new JLDisplayWidget();
			display->box.pos = mm2px(Vec(63,topFirstRow + (i * rowGap) - 4));
			display->box.size = Vec(88, 24);
			if(module){
				display->text = module->noteStr[i];
				display->on = &module->showDisplay;
			} 
			addChild(display);
		}

	}
};


Model* modelNoteToLogic = createModel<NoteToLogic, NoteToLogicWidget>("NoteToLogic");
#include "JL_modules.hpp"
#define N_COMP 4	// Set the number of switch cases

static const char noteLetters[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
static const char isSharpNote[12] = { 0,   1,   0,   1,   0,   0,   1,   0,   1,   0,   1,   0 };

struct SwitchStatement : Module {

	enum ParamIds {
		ENUMS(NOTE_PARAM, N_COMP),
		ENUMS(OCTAVE_PARAM, N_COMP),
		ENUMS(COMP_PARAM, N_COMP),
		NUM_PARAMS
	};
	enum InputIds {
		CV1_INPUT,
		IN1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CASEOUT_OUTPUT, N_COMP),
		CASEDEFAULTOUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEDINT1_LIGHT,
		LEDEXT1_LIGHT,
		ENUMS(LEDCASE_OUTPUT, N_COMP),
		LEDCASEDEFAULT_LIGHT,
		LEDDEFAULTENABLE_LIGHT,
		NUM_LIGHTS
	};

	bool showDisplay = true;
	char noteStr[N_COMP][6];

	// Expander
	JL_signals signalMessages[2][2] = {}; // Messages from SwitchStatement placed to the left (CV, IN)

	SwitchStatement() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i = 0; i < N_COMP; i++)
		{
			configParam(NOTE_PARAM + i, 0.f, 11.f, 0.f, "Note");
			configParam(OCTAVE_PARAM + i, 0.f, 10.f, 4.f, "Octave");
			configParam(COMP_PARAM + i, 0.f, 4.f, 2.f, "Comparation");
		}

		leftExpander.producerMessage = signalMessages[0];
		leftExpander.consumerMessage = signalMessages[1];
	}

	void process(const ProcessArgs& args) override {
		// ---display selected note---
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
		
		// ---Logic calculation---

		int cvActiveChannels = 0;					// Get the number of active chennals in the CV signal (internal or external) - Only de first channel is used for comparasions
		float cv[16] = {};							// Get the voltages the CV signal (internal or external)
		int inActiveChannels = 1;					// Get the number of active chennals in the IN signal (internal or external)
		float in[16] = {};							// Get the voltages the IN signal (internal or external)

		int selectedPitchNote;						// Gets the pitch note selected by the user (NOTE knobs)
		int selectedOctave;							// Gets the octave selected by the user (OCTAVE knobs)
		ComparationType selectedComparation;		// Gets the Comparation type selected by the user (COMP knobs)
		
		// Get signals from previous SwitchStatement to the left, or, from input ports of this module
		if (leftExpander.module && leftExpander.module->model == modelSwitchStatement) {
			// From previous SwitchStatement...

			// Get signals
			JL_signals *inSignal = (JL_signals*) leftExpander.consumerMessage;

			cvActiveChannels = inSignal[0].nActiveChannels;
			for (int i = 0; i < cvActiveChannels; i++)
			{
				cv[i] = inSignal[0].cv[i];
			}

			inActiveChannels = inSignal[1].nActiveChannels;
			for (int i = 0; i < inActiveChannels; i++)
			{
				in[i] = inSignal[1].cv[i];
			}

			// Set source LED's
			lights[LEDINT1_LIGHT].setBrightness(0.f);
			lights[LEDEXT1_LIGHT].setBrightness(10.f);

		}else{
			// From input ports...

			// Set source LED's
			lights[LEDINT1_LIGHT].setBrightness(10.f);
			lights[LEDEXT1_LIGHT].setBrightness(0.f);

			if (inputs[CV1_INPUT].isConnected())
			{
				cvActiveChannels = 1;	// Only the first channel will be used for the switch case
				// Get input
				inputs[CV1_INPUT].readVoltages(cv);
			}
			
			if (inputs[IN1_INPUT].isConnected())
			{
				inActiveChannels = std::max(inActiveChannels, inputs[IN1_INPUT].getChannels());
				// Get input
				inputs[IN1_INPUT].readVoltages(in);
			}
		}

		// If there is anything to compare...
		if (cvActiveChannels > 0)
		{
			for (int i = 0; i < N_COMP; i++)
			{
				selectedPitchNote = (int)(params[NOTE_PARAM + i].getValue());
				selectedOctave = (int)(params[OCTAVE_PARAM + i].getValue());
				selectedComparation = (ComparationType)((int)(params[COMP_PARAM + i].getValue()));

				if ((CompareCvToNote(cv[0], selectedPitchNote, selectedOctave, selectedComparation) == true) && (cvActiveChannels > 0))
				{
					cvActiveChannels = 0;	// A valid comparison has been found -> nothing else will be compared
					lights[LEDCASE_OUTPUT + i].setBrightness(10.f);
					// Write on Output
					outputs[CASEOUT_OUTPUT + i].setChannels(inActiveChannels);
					outputs[CASEOUT_OUTPUT + i].writeVoltages(in);
				}else
				{
					// Shutdown output
					outputs[CASEOUT_OUTPUT + i].setChannels(1);
					outputs[CASEOUT_OUTPUT + i].setVoltage(0.f);
					lights[LEDCASE_OUTPUT + i].setBrightness(0.f);
				}
				
				
			}
		}else{
			// If not, shutdown all outputs
			for (int i = 0; i < (N_COMP + 1); i++)
			{
				outputs[CASEOUT_OUTPUT + i].setChannels(1);
				outputs[CASEOUT_OUTPUT + i].setVoltage(0.f);
				lights[LEDCASE_OUTPUT + i].setBrightness(0.f);
			}
		}
		
		// To next SwitchStatement module to the right
		if (rightExpander.module && rightExpander.module->model == modelSwitchStatement) {
			JL_signals *outSignal = (JL_signals*)(rightExpander.module->leftExpander.producerMessage);
			outSignal[0].nActiveChannels = cvActiveChannels;
			for (int i = 0; i < cvActiveChannels; i++)
			{
				outSignal[0].cv[i] = cv[i];
			}

			outSignal[1].nActiveChannels = inActiveChannels;
			for (int i = 0; i < inActiveChannels; i++)
			{
				outSignal[1].cv[i] = in[i];
			}
			rightExpander.module->leftExpander.messageFlipRequested = true;
			lights[LEDDEFAULTENABLE_LIGHT].setBrightness(0.f);
			cvActiveChannels = 0;	// Because there is a SwitchStatement module to the right, the default output port must be ignored
		}else{
			lights[LEDDEFAULTENABLE_LIGHT].setBrightness(10.f);
		}

		// If there is still an active channel, it means that the IN signal must be routed to the Default port
		if (cvActiveChannels > 0)
		{
			lights[LEDCASEDEFAULT_LIGHT].setBrightness(10.f);
			// Write on Output
			outputs[CASEDEFAULTOUT_OUTPUT].setChannels(inActiveChannels);
			outputs[CASEDEFAULTOUT_OUTPUT].writeVoltages(in);
		}else
		{	
			// Shutdown output
			lights[LEDCASEDEFAULT_LIGHT].setBrightness(0.f);
			outputs[CASEDEFAULTOUT_OUTPUT].setChannels(1);
			outputs[CASEDEFAULTOUT_OUTPUT].setVoltage(0.f);
		}

	}
};


struct SwitchStatementWidget : ModuleWidget {
	SwitchStatementWidget(SwitchStatement* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SwitchStatement.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.919, 19.977)), module, SwitchStatement::CV1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.419, 19.977)), module, SwitchStatement::IN1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(112.229, 109.253)), module, SwitchStatement::CASEDEFAULTOUT_OUTPUT));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(25.088, 18.139)), module, SwitchStatement::LEDINT1_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(25.088, 22.146)), module, SwitchStatement::LEDEXT1_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(106.088, 106.422)), module, SwitchStatement::LEDCASEDEFAULT_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(72.588, 109.639)), module, SwitchStatement::LEDDEFAULTENABLE_LIGHT));

		JLDisplayWidget *display1 = new JLDisplayWidget();
		display1->box.pos = mm2px(Vec(71.467,39.202 - 4));
		display1->box.size = Vec(88, 24);
		if(module){
			display1->text = module->noteStr[0];
			display1->on = &module->showDisplay;
		} 
		addChild(display1);

		static const int topFirstRow = 39;
		static const int rowGap = 17;

		for (int i = 0; i < N_COMP; i++)
		{
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(29.467, topFirstRow + (i * rowGap))), module, SwitchStatement::NOTE_PARAM + i));
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(46.467, topFirstRow + (i * rowGap))), module, SwitchStatement::OCTAVE_PARAM + i));
			addParam(createParamCentered<SnapJLKnob>(mm2px(Vec(63.467, topFirstRow + (i * rowGap))), module, SwitchStatement::COMP_PARAM + i));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(112.229, topFirstRow + (i * rowGap))), module, SwitchStatement::CASEOUT_OUTPUT + i));

			addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(106.088, topFirstRow + (i * rowGap) - 3)), module, SwitchStatement::LEDCASE_OUTPUT + i));
			JLDisplayWidget *display = new JLDisplayWidget();
			display->box.pos = mm2px(Vec(71.467,topFirstRow + (i * rowGap) - 4));
			display->box.size = Vec(88, 24);
			if(module){
				display->text = module->noteStr[i];
				display->on = &module->showDisplay;
			} 
			addChild(display);
		}

	}
};


Model* modelSwitchStatement = createModel<SwitchStatement, SwitchStatementWidget>("SwitchStatement");
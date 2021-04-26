#include "JL_modules.hpp"
#define N_COMP 5	// Set the number of comparators

static const char noteLetters[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
static const char isSharpNote[12] = { 0,   1,   0,   1,   0,   0,   1,   0,   1,   0,   1,   0 };

struct NoteToLogic : Module {
	enum ParamIds {
		ENUMS(NOTE_PARAM, N_COMP),
		ENUMS(OCTAVE_PARAM, N_COMP),
		ENUMS(COMP_PARAM, N_COMP),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(PITCH_INPUT, N_COMP),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(LOGICOUT_OUTPUT, N_COMP),
		ENUMS(LOGICOUTNOT_OUTPUT, N_COMP),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LEDACTIV_LIGHT, N_COMP),
		NUM_LIGHTS
	};

	enum ComparationType {
		LT,		// <
		LE,		// <=
		EQ,		// ==
		GE,		// >=
		GT		// >
	};

	const float trueValue = 10.f;
	const float falseValue = 0.f;

	bool showDisplay = true;
	char noteStr[N_COMP][6];

	bool isCommonLogic = true;				// Polyphony mode - Selected by context menu
	int nChannelToFirstComparator = -1;		// If isCommonLogic == false, gets the channel index to be assigned to the first comparator - Selected by context menu

	NoteToLogic() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < N_COMP; i++)
		{
			configParam(NOTE_PARAM + i, 0.f, 11.f, 0.f, "Note");
			configParam(OCTAVE_PARAM + i, 0.f, 10.f, 4.f, "Octave");
			configParam(COMP_PARAM + i, 0.f, 4.f, 2.f, "Comparation");
		}
		
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// Polyphony mode
		json_object_set_new(rootJ, "isCommonLogic", json_integer(isCommonLogic));
		json_object_set_new(rootJ, "nChannelToFirstComparator", json_integer(nChannelToFirstComparator));
		
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Get Polyphony mode
		json_t *isCommonLogicJ = json_object_get(rootJ, "isCommonLogic");
		if (isCommonLogicJ)
			isCommonLogic = json_integer_value(isCommonLogicJ);

		// Get Channel to by assigned to the first comparator
		json_t *nChannelToFirstComparatorJ = json_object_get(rootJ, "nChannelToFirstComparator");
		if (nChannelToFirstComparatorJ)
			nChannelToFirstComparator = json_integer_value(nChannelToFirstComparatorJ);

		
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
		int nActiveChannels;						// Gets the number of active channels
		int _nChannelToFirstComparator = -1;		// Gets the channel index to be assigned to the first comparator
		bool _isCommonLogic = true;					// Defines the polyphony mode

		int selectedPitchNote;						// Gets the pitch note selected by the user (NOTE knobs)
		int selectedOctave;							// Gets the octave selected by the user (OCTAVE knobs)
		ComparationType selectedComparation;		// Gets the Comparation type selected by the user (COMP knobs)

		// Set the operation mode:
		//	Monophonic signal --> _isCommonLogic = true
		//	Polyphonic signal --> isCommonLogic == true ? _isCommonLogic = true : _isCommonLogic = false
		if (inputs[0].isConnected())
		{
			nActiveChannels = inputs[0].getChannels();
			if (nActiveChannels > 1)
			{
				_isCommonLogic = isCommonLogic;
				if (!isCommonLogic)
				{
					_nChannelToFirstComparator = nChannelToFirstComparator;

				}
			}else{
				_isCommonLogic = true;
			}
		}else{
			_isCommonLogic = true;
		}

		// Signal Processing
		if (_isCommonLogic)
		{
			// Common Logic - The same comparison is applied to all active channels of all connected inputs
			for (int i = 0; i < N_COMP; i++)
			{
				if (inputs[i].isConnected())
				{
					lights[LEDACTIV_LIGHT + i].setBrightness(trueValue);
					selectedPitchNote = (int)(params[NOTE_PARAM + i].getValue());
					selectedOctave = (int)(params[OCTAVE_PARAM + i].getValue());
					selectedComparation = (ComparationType)((int)(params[COMP_PARAM + i].getValue()));
					nActiveChannels = inputs[i].getChannels();
					
					for (int j = 0; j < nActiveChannels; j++)
					{
						bool result = CompareCvToNote(inputs[i].getPolyVoltage(j), selectedPitchNote, selectedOctave, selectedComparation);
						outputs[LOGICOUT_OUTPUT + i].setVoltage( result ? trueValue : falseValue , j);
						outputs[LOGICOUTNOT_OUTPUT + i].setVoltage( result ? falseValue : trueValue , j);
					}
					
					outputs[LOGICOUT_OUTPUT + i].setChannels(nActiveChannels);
					outputs[LOGICOUTNOT_OUTPUT + i].setChannels(nActiveChannels);
				}else{
					// Set outpus of unconnected comparators
					outputs[LOGICOUT_OUTPUT + i].setVoltage(falseValue, 0);
					outputs[LOGICOUT_OUTPUT + i].setChannels(1);
					outputs[LOGICOUTNOT_OUTPUT + i].setVoltage(trueValue, 0);
					outputs[LOGICOUTNOT_OUTPUT + i].setChannels(1);
					lights[LEDACTIV_LIGHT + i].setBrightness(falseValue);
				}
			}
		}else{
			// Split channels by comparators - Each channel of the MAIN input is assigned to a separate comparator (in sequence)
			int comapratorIndex = 0;
			for (int i = _nChannelToFirstComparator; i < nActiveChannels && comapratorIndex < N_COMP; i++, comapratorIndex++)
			{
				selectedPitchNote = (int)(params[NOTE_PARAM + comapratorIndex].getValue());
				selectedOctave = (int)(params[OCTAVE_PARAM + comapratorIndex].getValue());
				selectedComparation = (ComparationType)((int)(params[COMP_PARAM + comapratorIndex].getValue()));
				lights[LEDACTIV_LIGHT + comapratorIndex].setBrightness(trueValue);			
				bool result = CompareCvToNote(inputs[0].getPolyVoltage(i), selectedPitchNote, selectedOctave , selectedComparation);
				outputs[LOGICOUT_OUTPUT + comapratorIndex].setVoltage(result ? trueValue : falseValue, 0);
				outputs[LOGICOUTNOT_OUTPUT + comapratorIndex].setVoltage(result ? falseValue : trueValue, 0);
				
				outputs[LOGICOUT_OUTPUT + comapratorIndex].setChannels(1);
				outputs[LOGICOUTNOT_OUTPUT + comapratorIndex].setChannels(1);
			}

			for (int j = comapratorIndex; j < N_COMP; j++)
			{	
				// Set outpus of unassigned comparators
				outputs[LOGICOUT_OUTPUT + j].setVoltage(falseValue, 0);
				outputs[LOGICOUT_OUTPUT + j].setChannels(1);
				outputs[LOGICOUTNOT_OUTPUT + j].setVoltage(trueValue, 0);
				outputs[LOGICOUTNOT_OUTPUT + j].setChannels(1);
				lights[LEDACTIV_LIGHT + j].setBrightness(falseValue);
			}
		}
		

	}
};


struct NoteToLogicWidget : ModuleWidget {

	struct CommonLogicMenuItem : MenuItem {
		NoteToLogic *module;
		void onAction(const event::Action &e) override {
			module->isCommonLogic = true;
			module->nChannelToFirstComparator = -1;
		}
	};
	struct SplitChannelsMenuItem : MenuItem {
		struct AssignChannelToComparatorMenuItem : MenuItem {
			NoteToLogic *module;
			int nChannelToFirstComparator;
			void onAction(const event::Action &e) override {
				module->isCommonLogic = false;
				module->nChannelToFirstComparator = nChannelToFirstComparator;
			}
		};

		NoteToLogic *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			AssignChannelToComparatorMenuItem *itemChannel0 = createMenuItem<AssignChannelToComparatorMenuItem>("#1 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 0));
			itemChannel0->module = module;
			itemChannel0->nChannelToFirstComparator = 0;
			menu->addChild(itemChannel0);		
			
			AssignChannelToComparatorMenuItem *itemChannel1 = createMenuItem<AssignChannelToComparatorMenuItem>("#2 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 1));
			itemChannel1->module = module;
			itemChannel1->nChannelToFirstComparator = 1;
			menu->addChild(itemChannel1);	

			AssignChannelToComparatorMenuItem *itemChannel2 = createMenuItem<AssignChannelToComparatorMenuItem>("#3 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 2));
			itemChannel2->module = module;
			itemChannel2->nChannelToFirstComparator = 2;
			menu->addChild(itemChannel2);	

			AssignChannelToComparatorMenuItem *itemChannel3 = createMenuItem<AssignChannelToComparatorMenuItem>("#4 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 3));
			itemChannel3->module = module;
			itemChannel3->nChannelToFirstComparator = 3;
			menu->addChild(itemChannel3);	

			AssignChannelToComparatorMenuItem *itemChannel4 = createMenuItem<AssignChannelToComparatorMenuItem>("#5 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 4));
			itemChannel4->module = module;
			itemChannel4->nChannelToFirstComparator = 4;
			menu->addChild(itemChannel4);	

			AssignChannelToComparatorMenuItem *itemChannel5 = createMenuItem<AssignChannelToComparatorMenuItem>("#6 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 5));
			itemChannel5->module = module;
			itemChannel5->nChannelToFirstComparator = 5;
			menu->addChild(itemChannel5);	

			AssignChannelToComparatorMenuItem *itemChannel6 = createMenuItem<AssignChannelToComparatorMenuItem>("#7 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 6));
			itemChannel6->module = module;
			itemChannel6->nChannelToFirstComparator = 6;
			menu->addChild(itemChannel6);	

			AssignChannelToComparatorMenuItem *itemChannel7 = createMenuItem<AssignChannelToComparatorMenuItem>("#8 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 7));
			itemChannel7->module = module;
			itemChannel7->nChannelToFirstComparator = 7;
			menu->addChild(itemChannel7);	

			AssignChannelToComparatorMenuItem *itemChannel8 = createMenuItem<AssignChannelToComparatorMenuItem>("#9 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 8));
			itemChannel8->module = module;
			itemChannel8->nChannelToFirstComparator = 8;
			menu->addChild(itemChannel8);	

			AssignChannelToComparatorMenuItem *itemChannel9 = createMenuItem<AssignChannelToComparatorMenuItem>("#10 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 9));
			itemChannel9->module = module;
			itemChannel9->nChannelToFirstComparator = 9;
			menu->addChild(itemChannel9);	

			AssignChannelToComparatorMenuItem *itemChannel10 = createMenuItem<AssignChannelToComparatorMenuItem>("#11 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 10));
			itemChannel10->module = module;
			itemChannel10->nChannelToFirstComparator = 10;
			menu->addChild(itemChannel10);	

			AssignChannelToComparatorMenuItem *itemChannel11 = createMenuItem<AssignChannelToComparatorMenuItem>("#12 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 11));
			itemChannel11->module = module;
			itemChannel11->nChannelToFirstComparator = 11;
			menu->addChild(itemChannel11);	

			AssignChannelToComparatorMenuItem *itemChannel12 = createMenuItem<AssignChannelToComparatorMenuItem>("#13 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 12));
			itemChannel12->module = module;
			itemChannel12->nChannelToFirstComparator = 12;
			menu->addChild(itemChannel12);	

			AssignChannelToComparatorMenuItem *itemChannel13 = createMenuItem<AssignChannelToComparatorMenuItem>("#14 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 13));
			itemChannel13->module = module;
			itemChannel13->nChannelToFirstComparator = 13;
			menu->addChild(itemChannel13);	

			AssignChannelToComparatorMenuItem *itemChannel14 = createMenuItem<AssignChannelToComparatorMenuItem>("#15 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 14));
			itemChannel14->module = module;
			itemChannel14->nChannelToFirstComparator = 14;
			menu->addChild(itemChannel14);	

			AssignChannelToComparatorMenuItem *itemChannel15 = createMenuItem<AssignChannelToComparatorMenuItem>("#16 Channel to first comparator", CHECKMARK(module->nChannelToFirstComparator == 15));
			itemChannel15->module = module;
			itemChannel15->nChannelToFirstComparator = 15;
			menu->addChild(itemChannel15);	

			return menu;
		}
	};	

	void appendContextMenu(Menu *menu) override {
		NoteToLogic *module = dynamic_cast<NoteToLogic*>(this->module);
		assert(module);
	
		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		MenuLabel *themeLabel = new MenuLabel();
		themeLabel->text = "Polyphony mode";
		menu->addChild(themeLabel);

		CommonLogicMenuItem *isCommunLogicMenuItem = createMenuItem<CommonLogicMenuItem>("Commun logic", CHECKMARK(module->isCommonLogic));
		isCommunLogicMenuItem->module = module;
		menu->addChild(isCommunLogicMenuItem);

		SplitChannelsMenuItem *splitChannelsMenuItem = createMenuItem<SplitChannelsMenuItem>("Split channels by comparators", RIGHT_ARROW);
		splitChannelsMenuItem->module = module;
		menu->addChild(splitChannelsMenuItem);		

	}	

	NoteToLogicWidget(NoteToLogic* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/NoteToLogic.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

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
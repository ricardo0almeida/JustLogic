#include "JL_modules.hpp"


struct IfStatement : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		LOGICGATE1_INPUT,
		IN1_INPUT,
		CV1_INPUT,
		CV2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		INCV1_LIGHT,
		INCV2_LIGHT,
		NUM_LIGHTS
	};

	float trueValue = 10.f;
	float falseValue = 0.f;

	IfStatement() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();

		// Set Voltage Levels
		json_object_set_new(rootJ, "trueValue", json_integer((int) trueValue));
		json_object_set_new(rootJ, "falseValue", json_integer((int) falseValue));
		
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Get Voltage Levels
		json_t *json_trueValue = json_object_get(rootJ, "trueValue");
		if (json_trueValue)
			trueValue = (float) json_integer_value(json_trueValue);

		json_t *json_falseValue = json_object_get(rootJ, "falseValue");
		if (json_falseValue)
			falseValue = (float) json_integer_value(json_falseValue);

	}

	void process(const ProcessArgs& args) override {

		if (inputs[LOGICGATE1_INPUT].isConnected())
		{
			
			if(inputs[LOGICGATE1_INPUT].getVoltage() > (((trueValue - falseValue) / 2.f) + falseValue)){
				// LOGIC GATE == true.

				// Set LED's
				lights[INCV1_LIGHT].setBrightness(10.f);
				lights[INCV2_LIGHT].setBrightness(0.f);

				// Add CV1 to IN in all active channels 
				int nActiveChannelsIN = inputs[IN1_INPUT].getChannels();
				int nActiveChannelsCV1 = inputs[CV1_INPUT].getChannels();
				int nActiveChannels = std::max(std::max(1, nActiveChannelsIN) , nActiveChannelsCV1);
				for (int i = 0; i < nActiveChannels; i++)
				{
					outputs[OUT1_OUTPUT].setVoltage( (i < nActiveChannelsIN ? inputs[IN1_INPUT].getPolyVoltage(i) : 0.f) + (i < nActiveChannelsCV1 ? inputs[CV1_INPUT].getPolyVoltage(i) : 0.f), i);
				}
				outputs[OUT1_OUTPUT].setChannels(nActiveChannels);
			}else{
				// LOGIC GATE == false.

				// Set LED's
				lights[INCV1_LIGHT].setBrightness(0.f);
				lights[INCV2_LIGHT].setBrightness(10.f);

				// Add CV2 to IN in all active channels 
				int nActiveChannelsIN = inputs[IN1_INPUT].getChannels();
				int nActiveChannelsCV2 = inputs[CV2_INPUT].getChannels();
				int nActiveChannels = std::max(std::max(1, nActiveChannelsIN) , nActiveChannelsCV2);
				for (int i = 0; i < nActiveChannels; i++)
				{
					outputs[OUT1_OUTPUT].setVoltage( (i < nActiveChannelsIN ? inputs[IN1_INPUT].getPolyVoltage(i) : 0.f) + (i < nActiveChannelsCV2 ? inputs[CV2_INPUT].getPolyVoltage(i) : 0.f), i);
				}
				outputs[OUT1_OUTPUT].setChannels(nActiveChannels);
			}
			
		}else{
			// If LOGIC GATE is not connected, then, swith off output and LED's
			outputs[OUT1_OUTPUT].setChannels(1);
			outputs[OUT1_OUTPUT].setVoltage(0.f, 0);
			lights[INCV1_LIGHT].setBrightness(0.f);
			lights[INCV2_LIGHT].setBrightness(0.f);
		}
	}
};


struct IfStatementWidget : ModuleWidget {

	struct VoltageLevelMenuItem : MenuItem {
		IfStatement *module;
		float trueValue;
		float falseValue;
		void onAction(const event::Action &e) override {
			module->trueValue = trueValue;
			module->falseValue = falseValue;
		}
	};

	void appendContextMenu(Menu *menu) override {
		IfStatement *module = dynamic_cast<IfStatement*>(this->module);
		assert(module);
	
		MenuLabel *spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);

		MenuLabel *voltageLabel = new MenuLabel();
		voltageLabel->text = "Voltage levels for Logic Gate";
		menu->addChild(voltageLabel);

		VoltageLevelMenuItem *ZeroToTenVoltsMenuItem = createMenuItem<VoltageLevelMenuItem>("False = 0v; True = 10v", CHECKMARK(module->trueValue == 10.f));
		ZeroToTenVoltsMenuItem->module = module;
		ZeroToTenVoltsMenuItem->trueValue = 10.f;
		ZeroToTenVoltsMenuItem->falseValue = 0.f;
		menu->addChild(ZeroToTenVoltsMenuItem);

		VoltageLevelMenuItem *MinusFiveToFiveVoltsMenuItem = createMenuItem<VoltageLevelMenuItem>("False = -5v; True = 5v", CHECKMARK(module->trueValue == 5.f));
		MinusFiveToFiveVoltsMenuItem->module = module;
		MinusFiveToFiveVoltsMenuItem->trueValue = 5.f;
		MinusFiveToFiveVoltsMenuItem->falseValue = -5.f;
		menu->addChild(MinusFiveToFiveVoltsMenuItem);		

	}	


	IfStatementWidget(IfStatement* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/IfStatement.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.756, 18.147)), module, IfStatement::LOGICGATE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.756, 32.15)), module, IfStatement::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.756, 46.343)), module, IfStatement::CV1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.756, 60.469)), module, IfStatement::CV2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.756, 84.692)), module, IfStatement::OUT1_OUTPUT));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(2.587, 72.931)), module, IfStatement::INCV1_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(2.587, 77.788)), module, IfStatement::INCV2_LIGHT));
	}
};


Model* modelIfStatement = createModel<IfStatement, IfStatementWidget>("IfStatement");
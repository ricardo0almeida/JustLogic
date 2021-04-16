#include <rack.hpp>

using namespace rack;

extern Plugin *pluginInstance;

struct SnapJLKnob : RoundKnob {
        SnapJLKnob();
};

struct JLThreeDigitDisplayWidget : TransparentWidget {

	bool *on;
	char *text;
	std::shared_ptr<Font> font;

	JLThreeDigitDisplayWidget();

	void draw(const DrawArgs &args) override;
};
#include <rack.hpp>

using namespace rack;

extern Plugin *pluginInstance;

struct SnapJLKnob : RoundKnob {
        SnapJLKnob();
};

struct JLDisplayWidget : TransparentWidget {

	bool *on = 0;
	char *text;
	std::shared_ptr<Font> font;

	JLDisplayWidget();

	void draw(const DrawArgs &args) override;
};
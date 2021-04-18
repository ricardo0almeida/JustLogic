#include "JL_components.hpp"


SnapJLKnob::SnapJLKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/RoundSmallKnob.svg")));
	snap = true;
};



JLDisplayWidget::JLDisplayWidget() {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Fonts/DSEG14Modern-Regular.ttf"));
};

void JLDisplayWidget::draw(const DrawArgs &args){

    NVGcolor backgroundColor = nvgRGB(0x38, 0x38, 0x38);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 3.0);
    nvgFillColor(args.vg, backgroundColor);
    nvgFill(args.vg);
    nvgStrokeWidth(args.vg, 1.0);
    nvgStrokeColor(args.vg, borderColor);
    nvgStroke(args.vg);

    nvgFontSize(args.vg, 16);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 2.5);

    Vec textPos = Vec(5.0f, 20.0f);

    NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
    nvgFillColor(args.vg, nvgTransRGBA(textColor, 16));
    nvgText(args.vg, textPos.x, textPos.y, "~~~~~", NULL);

    if(on && *on) {
        nvgFillColor(args.vg, textColor);
        nvgText(args.vg, textPos.x, textPos.y, text, NULL);
    }
};

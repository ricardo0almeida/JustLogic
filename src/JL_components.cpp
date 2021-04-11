#include "JL_components.hpp"


SnapJLKnob::SnapJLKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/RoundSmallKnob.svg")));
	snap = true;
};
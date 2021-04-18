#pragma once
#include "WickedEngine.h"

class CyRender;


class VisualsWindow : public wiWindow {
public:
	void Create(CyRender* renderer);
	wiCheckBox renderChunksCheckBox;
	wiCheckBox renderTreesCheckBox;
	wiCheckBox renderMeshesCheckBox;
	wiCheckBox renderTorchesCheckBox;
	wiCheckBox renderLightsCheckBox;
	wiCheckBox renderGizmosCheckBox;
	wiCheckBox renderEmittersCheckBox;
	wiCheckBox soundCheckBox;
};

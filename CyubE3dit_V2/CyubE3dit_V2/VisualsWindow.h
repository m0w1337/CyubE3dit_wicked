#pragma once
#include "WickedEngine.h"

class CyRender;


class VisualsWindow : public wiWindow {
public:
	void Create(CyRender* renderer);
	wiCheckBox renderChunksCheckBox;
	wiCheckBox renderTreesCheckBox;
	wiCheckBox renderMeshesCheckBox;
	wiCheckBox renderFoilageCheckBox;
	wiCheckBox renderTorchesCheckBox;
	wiCheckBox renderLightsCheckBox;
	wiCheckBox renderGizmosCheckBox;
	wiCheckBox renderEmittersCheckBox;
	wiCheckBox pickTypeChunkCheckBox;
	wiCheckBox pickTypeTreeCheckBox;
	wiCheckBox clipUndergroundCheckBox;
	wiCheckBox temporalAACheckBox;
	wiCheckBox volCloudsCheckBox;
	wiCheckBox soundCheckBox;
	wiSlider musicVol;
	wiSlider effectVol;
};

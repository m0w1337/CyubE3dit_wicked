#pragma once
#include "WickedEngine.h"

class CyRender;





class RendererWindow : public wiWindow {
public:
	void Create(CyRender* renderer);

	wiCheckBox vsyncCheckBox;
	wiCheckBox occlusionCullingCheckBox;
	wiSlider resolutionScaleSlider;
	wiSlider gammaSlider;
	wiCheckBox voxelRadianceCheckBox;
	wiCheckBox voxelRadianceDebugCheckBox;
	wiCheckBox voxelRadianceSecondaryBounceCheckBox;
	wiCheckBox voxelRadianceReflectionsCheckBox;
	wiSlider voxelRadianceVoxelSizeSlider;
	wiSlider voxelRadianceConeTracingSlider;
	wiSlider voxelRadianceRayStepSizeSlider;
	wiSlider voxelRadianceMaxDistanceSlider;
	wiCheckBox partitionBoxesCheckBox;
	wiCheckBox boneLinesCheckBox;
	wiCheckBox debugEmittersCheckBox;
	wiCheckBox debugForceFieldsCheckBox;
	wiCheckBox debugRaytraceBVHCheckBox;
	wiCheckBox wireFrameCheckBox;
	wiCheckBox variableRateShadingClassificationCheckBox;
	wiCheckBox variableRateShadingClassificationDebugCheckBox;
	wiCheckBox advancedLightCullingCheckBox;
	wiCheckBox debugLightCullingCheckBox;
	wiCheckBox tessellationCheckBox;
	wiCheckBox envProbesCheckBox;
	wiCheckBox gridHelperCheckBox;
	wiCheckBox cameraVisCheckBox;
	wiCheckBox pickTypeChunkCheckBox;
	wiCheckBox pickTypeTreeCheckBox;
	wiCheckBox TorchlightsCheckBox;
	wiCheckBox pickTypeLightCheckBox;
	wiCheckBox pickTypeDecalCheckBox;
	wiCheckBox pickTypeForceFieldCheckBox;
	wiCheckBox pickTypeEmitterCheckBox;
	wiCheckBox pickTypeHairCheckBox;
	wiCheckBox pickTypeCameraCheckBox;
	wiCheckBox pickTypeArmatureCheckBox;
	wiCheckBox pickTypeSoundCheckBox;
	wiSlider speedMultiplierSlider;
	wiCheckBox transparentShadowsCheckBox;
	wiComboBox shadowTypeComboBox;
	wiComboBox shadowProps2DComboBox;
	wiComboBox shadowPropsCubeComboBox;
	wiComboBox MSAAComboBox;
	wiSlider raytracedShadowsSlider;
	wiCheckBox temporalAACheckBox;
	wiCheckBox temporalAADebugCheckBox;
	wiComboBox textureQualityComboBox;
	wiSlider mipLodBiasSlider;
	wiSlider raytraceBounceCountSlider;

	wiCheckBox freezeCullingCameraCheckBox;
	wiCheckBox disableAlbedoMapsCheckBox;

	uint32_t GetPickType() const;
};

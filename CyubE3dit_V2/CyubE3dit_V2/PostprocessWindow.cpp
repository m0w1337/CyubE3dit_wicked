#include "stdafx.h"
#include "PostprocessWindow.h"
#include "cyRender.h"

#include <thread>

using namespace std;
using namespace wiGraphics;


void PostprocessWindow::Create(CyRender* renderer) {
	wiWindow::Create("PostProcess Window");
	SetSize(XMFLOAT2(450, 500));

	float x = 150;
	float y = 10;
	float hei = 18;
	float step = hei + 2;

	exposureSlider.Create(0.0f, 3.0f, 1, 10000, "Exposure: ");
	exposureSlider.SetTooltip("Set the tonemap exposure value");
	exposureSlider.SetScriptTip("RenderPath3D::SetExposure(float value)");
	exposureSlider.SetSize(XMFLOAT2(100, hei));
	exposureSlider.SetPos(XMFLOAT2(x, y += step));
	exposureSlider.SetValue(renderer->getExposure());
	exposureSlider.OnSlide([=](wiEventArgs args) {
		renderer->setExposure(args.fValue);
	});
	AddWidget(&exposureSlider);

	lensFlareCheckBox.Create("LensFlare: ");
	lensFlareCheckBox.SetTooltip("Toggle visibility of light source flares. Additional setup needed per light for a lensflare to be visible.");
	lensFlareCheckBox.SetScriptTip("RenderPath3D::SetLensFlareEnabled(bool value)");
	lensFlareCheckBox.SetSize(XMFLOAT2(hei, hei));
	lensFlareCheckBox.SetPos(XMFLOAT2(x, y += step));
	lensFlareCheckBox.SetCheck(renderer->getLensFlareEnabled());
	lensFlareCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setLensFlareEnabled(args.bValue);
	});
	AddWidget(&lensFlareCheckBox);

	lightShaftsCheckBox.Create("LightShafts: ");
	lightShaftsCheckBox.SetTooltip("Enable light shaft for directional light sources.");
	lightShaftsCheckBox.SetScriptTip("RenderPath3D::SetLightShaftsEnabled(bool value)");
	lightShaftsCheckBox.SetSize(XMFLOAT2(hei, hei));
	lightShaftsCheckBox.SetPos(XMFLOAT2(x, y += step));
	lightShaftsCheckBox.SetCheck(renderer->getLightShaftsEnabled());
	lightShaftsCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setLightShaftsEnabled(args.bValue);
	});
	AddWidget(&lightShaftsCheckBox);

	volumetricCloudsCheckBox.Create("Volumetric clouds: ");
	volumetricCloudsCheckBox.SetTooltip("Enable volumetric cloud rendering.");
	volumetricCloudsCheckBox.SetSize(XMFLOAT2(hei, hei));
	volumetricCloudsCheckBox.SetPos(XMFLOAT2(x, y += step));
	volumetricCloudsCheckBox.SetCheck(renderer->getVolumetricCloudsEnabled());
	volumetricCloudsCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setVolumetricCloudsEnabled(args.bValue);
		});
	AddWidget(&volumetricCloudsCheckBox);

	aoComboBox.Create("AO: ");
	aoComboBox.SetTooltip("Choose Ambient Occlusion type. RTAO is only available if hardware supports ray tracing");
	aoComboBox.SetScriptTip("RenderPath3D::SetAO(int value)");
	aoComboBox.SetSize(XMFLOAT2(150, hei));
	aoComboBox.SetPos(XMFLOAT2(x, y += step));
	aoComboBox.AddItem("Disabled");
	aoComboBox.AddItem("SSAO");
	aoComboBox.AddItem("HBAO");
	aoComboBox.AddItem("MSAO");
	if (wiRenderer::GetDevice()->CheckCapability(GRAPHICSDEVICE_CAPABILITY_RAYTRACING))
	{
		aoComboBox.AddItem("RTAO");
	}
	aoComboBox.SetSelected(renderer->getAO());
	aoComboBox.OnSelect([=](wiEventArgs args) {
		renderer->setAO((RenderPath3D::AO)args.iValue);

		switch (renderer->getAO())
		{
		case RenderPath3D::AO_SSAO:
			aoRangeSlider.SetEnabled(true); 
			aoRangeSlider.SetValue(2.0f);
			aoSampleCountSlider.SetEnabled(true); 
			aoSampleCountSlider.SetValue(9.0f);
			break;
		case RenderPath3D::AO_RTAO:
			aoRangeSlider.SetEnabled(true); 
			aoRangeSlider.SetValue(10.0f);
			aoSampleCountSlider.SetEnabled(true); 
			aoSampleCountSlider.SetValue(2.0f);
			break;
		default:
			aoRangeSlider.SetEnabled(false);
			aoSampleCountSlider.SetEnabled(false);
			break;
		}

		renderer->setAORange(aoRangeSlider.GetValue());
		renderer->setAOSampleCount((uint32_t)aoSampleCountSlider.GetValue());
	});
	AddWidget(&aoComboBox);

	aoPowerSlider.Create(0.25f, 8.0f, 2, 1000, "Power: ");
	aoPowerSlider.SetTooltip("Set SSAO Power. Higher values produce darker, more pronounced effect");
	aoPowerSlider.SetSize(XMFLOAT2(100, hei));
	aoPowerSlider.SetPos(XMFLOAT2(x + 100, y += step));
	aoPowerSlider.SetValue((float)renderer->getAOPower());
	aoPowerSlider.OnSlide([=](wiEventArgs args) {
		renderer->setAOPower(args.fValue);
		});
	AddWidget(&aoPowerSlider);

	aoRangeSlider.Create(1.0f, 100.0f, 1, 1000, "Range: ");
	aoRangeSlider.SetTooltip("Set AO ray length. Only for SSAO and RTAO");
	aoRangeSlider.SetSize(XMFLOAT2(100, hei));
	aoRangeSlider.SetPos(XMFLOAT2(x + 100, y += step));
	aoRangeSlider.SetValue((float)renderer->getAOPower());
	aoRangeSlider.OnSlide([=](wiEventArgs args) {
		renderer->setAORange(args.fValue);
		});
	AddWidget(&aoRangeSlider);

	aoSampleCountSlider.Create(1, 16, 9, 15, "Sample Count: ");
	aoSampleCountSlider.SetTooltip("Set AO ray count. Only for SSAO and RTAO");
	aoSampleCountSlider.SetSize(XMFLOAT2(100, hei));
	aoSampleCountSlider.SetPos(XMFLOAT2(x + 100, y += step));
	aoSampleCountSlider.SetValue((float)renderer->getAOPower());
	aoSampleCountSlider.OnSlide([=](wiEventArgs args) {
		renderer->setAOSampleCount(args.iValue);
		});
	AddWidget(&aoSampleCountSlider);

	ssrCheckBox.Create("SSR: ");
	ssrCheckBox.SetTooltip("Enable Screen Space Reflections.");
	ssrCheckBox.SetScriptTip("RenderPath3D::SetSSREnabled(bool value)");
	ssrCheckBox.SetSize(XMFLOAT2(hei, hei));
	ssrCheckBox.SetPos(XMFLOAT2(x, y += step));
	ssrCheckBox.SetCheck(renderer->getSSREnabled());
	ssrCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setSSREnabled(args.bValue);
	});
	AddWidget(&ssrCheckBox);

	raytracedReflectionsCheckBox.Create("Ray Traced Reflections: ");
	raytracedReflectionsCheckBox.SetTooltip("Enable Ray Traced Reflections. Only if GPU supports raytracing.");
	raytracedReflectionsCheckBox.SetScriptTip("RenderPath3D::SetRaytracedReflectionsEnabled(bool value)");
	raytracedReflectionsCheckBox.SetSize(XMFLOAT2(hei, hei));
	raytracedReflectionsCheckBox.SetPos(XMFLOAT2(x + 200, y));
	raytracedReflectionsCheckBox.SetCheck(renderer->getRaytracedReflectionEnabled());
	raytracedReflectionsCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setRaytracedReflectionsEnabled(args.bValue);
		});
	AddWidget(&raytracedReflectionsCheckBox);
	raytracedReflectionsCheckBox.SetEnabled(wiRenderer::GetDevice()->CheckCapability(GRAPHICSDEVICE_CAPABILITY_RAYTRACING));

	rtrCheckBox.Create("RTR: ");
	rtrCheckBox.SetTooltip("Enable real time Reflections (expensive).");
	rtrCheckBox.SetScriptTip("");
	rtrCheckBox.SetSize(XMFLOAT2(hei, hei));
	rtrCheckBox.SetPos(XMFLOAT2(x, y += step));
	rtrCheckBox.OnClick([=](wiEventArgs args) {
		if (CyMainComponent::m_probe != 0) {
			wiScene::EnvironmentProbeComponent* probe = wiScene::GetScene().probes.GetComponent(CyMainComponent::m_probe);
			probe->SetRealTime(args.bValue);
			probe->SetDirty();
		}
	});
	AddWidget(&rtrCheckBox);

	eyeAdaptionCheckBox.Create("EyeAdaption: ");
	eyeAdaptionCheckBox.SetTooltip("Enable eye adaption for the overall screen luminance");
	eyeAdaptionCheckBox.SetSize(XMFLOAT2(hei, hei));
	eyeAdaptionCheckBox.SetPos(XMFLOAT2(x, y += step));
	eyeAdaptionCheckBox.SetCheck(renderer->getEyeAdaptionEnabled());
	eyeAdaptionCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setEyeAdaptionEnabled(args.bValue);
	});
	AddWidget(&eyeAdaptionCheckBox);

	screenSpaceShadowsCheckBox.Create("SS Shadows: ");
	screenSpaceShadowsCheckBox.SetTooltip("Enable screen space contact shadows. This can add small shadows details to shadow maps in screen space.");
	screenSpaceShadowsCheckBox.SetSize(XMFLOAT2(hei, hei));
	screenSpaceShadowsCheckBox.SetPos(XMFLOAT2(x, y += step));
	screenSpaceShadowsCheckBox.SetCheck(wiRenderer::GetScreenSpaceShadowsEnabled());
	screenSpaceShadowsCheckBox.OnClick([=](wiEventArgs args) {
		wiRenderer::SetScreenSpaceShadowsEnabled(args.bValue);
	});
	AddWidget(&screenSpaceShadowsCheckBox);

	screenSpaceShadowsRangeSlider.Create(0.1f, 10.0f, 1, 1000, "Range: ");
	screenSpaceShadowsRangeSlider.SetTooltip("Range of contact shadows");
	screenSpaceShadowsRangeSlider.SetSize(XMFLOAT2(100, hei));
	screenSpaceShadowsRangeSlider.SetPos(XMFLOAT2(x + 100, y));
	screenSpaceShadowsRangeSlider.SetValue((float)renderer->getScreenSpaceShadowRange());
	screenSpaceShadowsRangeSlider.OnSlide([=](wiEventArgs args) {
		renderer->setScreenSpaceShadowRange(args.fValue);
	});
	AddWidget(&screenSpaceShadowsRangeSlider);

	screenSpaceShadowsStepCountSlider.Create(4, 128, 16, 128 - 4, "Sample Count: ");
	screenSpaceShadowsStepCountSlider.SetTooltip("Sample count of contact shadows. Higher values are better quality but slower.");
	screenSpaceShadowsStepCountSlider.SetSize(XMFLOAT2(100, hei));
	screenSpaceShadowsStepCountSlider.SetPos(XMFLOAT2(x + 100, y += step));
	screenSpaceShadowsStepCountSlider.SetValue((float)renderer->getScreenSpaceShadowSampleCount());
	screenSpaceShadowsStepCountSlider.OnSlide([=](wiEventArgs args) {
		renderer->setScreenSpaceShadowSampleCount(args.iValue);
	});
	AddWidget(&screenSpaceShadowsStepCountSlider);


	motionBlurCheckBox.Create("MotionBlur: ");
	motionBlurCheckBox.SetTooltip("Enable motion blur for camera movement and animated meshes.");
	motionBlurCheckBox.SetScriptTip("RenderPath3D::SetMotionBlurEnabled(bool value)");
	motionBlurCheckBox.SetSize(XMFLOAT2(hei, hei));
	motionBlurCheckBox.SetPos(XMFLOAT2(x, y += step));
	motionBlurCheckBox.SetCheck(renderer->getMotionBlurEnabled());
	motionBlurCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setMotionBlurEnabled(args.bValue);
	});
	AddWidget(&motionBlurCheckBox);

	motionBlurStrengthSlider.Create(0.1f, 400, 100, 10000, "Strength: ");
	motionBlurStrengthSlider.SetTooltip("Set the camera shutter speed for motion blur (higher value means stronger blur).");
	motionBlurStrengthSlider.SetScriptTip("RenderPath3D::SetMotionBlurStrength(float value)");
	motionBlurStrengthSlider.SetSize(XMFLOAT2(100, hei));
	motionBlurStrengthSlider.SetPos(XMFLOAT2(x + 100, y));
	motionBlurStrengthSlider.SetValue(renderer->getMotionBlurStrength());
	motionBlurStrengthSlider.OnSlide([=](wiEventArgs args) {
		renderer->setMotionBlurStrength(args.fValue);
		});
	AddWidget(&motionBlurStrengthSlider);
	/*
	depthOfFieldCheckBox.Create("DepthOfField: ");
	depthOfFieldCheckBox.SetTooltip("Enable Depth of field effect. Additional focus and strength setup required.");
	depthOfFieldCheckBox.SetScriptTip("RenderPath3D::SetDepthOfFieldEnabled(bool value)");
	depthOfFieldCheckBox.SetSize(XMFLOAT2(hei, hei));
	depthOfFieldCheckBox.SetPos(XMFLOAT2(x, y += step));
	depthOfFieldCheckBox.SetCheck(renderer->getDepthOfFieldEnabled());
	depthOfFieldCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setDepthOfFieldEnabled(args.bValue);
	});
	AddWidget(&depthOfFieldCheckBox);

	depthOfFieldFocusSlider.Create(1.0f, 100, 10, 10000, "Focus: ");
	depthOfFieldFocusSlider.SetTooltip("Set the focus distance from the camera. The picture will be sharper near the focus, and blurrier further from it.");
	depthOfFieldFocusSlider.SetScriptTip("RenderPath3D::SetDepthOfFieldFocus(float value)");
	depthOfFieldFocusSlider.SetSize(XMFLOAT2(100, hei));
	depthOfFieldFocusSlider.SetPos(XMFLOAT2(x + 100, y));
	depthOfFieldFocusSlider.SetValue(renderer->getDepthOfFieldFocus());
	depthOfFieldFocusSlider.OnSlide([=](wiEventArgs args) {
		renderer->setDepthOfFieldFocus(args.fValue);
	});
	AddWidget(&depthOfFieldFocusSlider);

	depthOfFieldScaleSlider.Create(1.0f, 20, 100, 1000, "Scale: ");
	depthOfFieldScaleSlider.SetTooltip("Set depth of field scale/falloff.");
	depthOfFieldScaleSlider.SetScriptTip("RenderPath3D::SetDepthOfFieldStrength(float value)");
	depthOfFieldScaleSlider.SetSize(XMFLOAT2(100, hei));
	depthOfFieldScaleSlider.SetPos(XMFLOAT2(x + 100, y += step));
	depthOfFieldScaleSlider.SetValue(renderer->getDepthOfFieldStrength());
	depthOfFieldScaleSlider.OnSlide([=](wiEventArgs args) {
		renderer->setDepthOfFieldStrength(args.fValue);
	});
	AddWidget(&depthOfFieldScaleSlider);

	depthOfFieldAspectSlider.Create(0.01f, 2, 1, 1000, "Aspect: ");
	depthOfFieldAspectSlider.SetTooltip("Set depth of field bokeh aspect ratio (width/height).");
	depthOfFieldAspectSlider.SetScriptTip("RenderPath3D::SetDepthOfFieldAspect(float value)");
	depthOfFieldAspectSlider.SetSize(XMFLOAT2(100, hei));
	depthOfFieldAspectSlider.SetPos(XMFLOAT2(x + 100, y += step));
	depthOfFieldAspectSlider.SetValue(renderer->getDepthOfFieldAspect());
	depthOfFieldAspectSlider.OnSlide([=](wiEventArgs args) {
		renderer->setDepthOfFieldAspect(args.fValue);
		});
	AddWidget(&depthOfFieldAspectSlider);
	*/
	bloomCheckBox.Create("Bloom: ");
	bloomCheckBox.SetTooltip("Enable bloom. The effect adds color bleeding to the brightest parts of the scene.");
	bloomCheckBox.SetScriptTip("RenderPath3D::SetBloomEnabled(bool value)");
	bloomCheckBox.SetSize(XMFLOAT2(hei, hei));
	bloomCheckBox.SetPos(XMFLOAT2(x, y += step));
	bloomCheckBox.SetCheck(renderer->getBloomEnabled());
	bloomCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setBloomEnabled(args.bValue);
	});
	AddWidget(&bloomCheckBox);

	bloomStrengthSlider.Create(0.0f, 10, 1, 1000, "Threshold: ");
	bloomStrengthSlider.SetTooltip("Set bloom threshold. The values below this will not glow on the screen.");
	bloomStrengthSlider.SetSize(XMFLOAT2(100, hei));
	bloomStrengthSlider.SetPos(XMFLOAT2(x + 100, y));
	bloomStrengthSlider.SetValue(renderer->getBloomThreshold());
	bloomStrengthSlider.OnSlide([=](wiEventArgs args) {
		renderer->setBloomThreshold(args.fValue);
	});
	AddWidget(&bloomStrengthSlider);

	fxaaCheckBox.Create("FXAA: ");
	fxaaCheckBox.SetTooltip("Fast Approximate Anti Aliasing. A fast antialiasing method, but can be a bit too blurry.");
	fxaaCheckBox.SetScriptTip("RenderPath3D::SetFXAAEnabled(bool value)");
	fxaaCheckBox.SetSize(XMFLOAT2(hei, hei));
	fxaaCheckBox.SetPos(XMFLOAT2(x, y += step));
	fxaaCheckBox.SetCheck(renderer->getFXAAEnabled());
	fxaaCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setFXAAEnabled(args.bValue);
	});
	AddWidget(&fxaaCheckBox);

	colorGradingCheckBox.Create("Color Grading: ");
	colorGradingCheckBox.SetTooltip("Enable color grading of the final render. An additional lookup texture must be set in the Weather!");
	colorGradingCheckBox.SetSize(XMFLOAT2(hei, hei));
	colorGradingCheckBox.SetPos(XMFLOAT2(x, y += step));
	colorGradingCheckBox.SetCheck(renderer->getColorGradingEnabled());
	colorGradingCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setColorGradingEnabled(args.bValue);
	});
	AddWidget(&colorGradingCheckBox);

	ditherCheckBox.Create("Dithering: ");
	ditherCheckBox.SetTooltip("Toggle the full screen dithering effect. This helps to reduce color banding.");
	ditherCheckBox.SetSize(XMFLOAT2(hei, hei));
	ditherCheckBox.SetPos(XMFLOAT2(x, y += step));
	ditherCheckBox.SetCheck(renderer->getDitherEnabled());
	ditherCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setDitherEnabled(args.bValue);
		});
	AddWidget(&ditherCheckBox);

	sharpenFilterCheckBox.Create("Sharpen Filter: ");
	sharpenFilterCheckBox.SetTooltip("Toggle sharpening post process of the final image.");
	sharpenFilterCheckBox.SetScriptTip("RenderPath3D::SetSharpenFilterEnabled(bool value)");
	sharpenFilterCheckBox.SetSize(XMFLOAT2(hei, hei));
	sharpenFilterCheckBox.SetPos(XMFLOAT2(x, y += step));
	sharpenFilterCheckBox.SetCheck(renderer->getSharpenFilterEnabled());
	sharpenFilterCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setSharpenFilterEnabled(args.bValue);
	});
	AddWidget(&sharpenFilterCheckBox);

	sharpenFilterAmountSlider.Create(0, 4, 1, 1000, "Amount: ");
	sharpenFilterAmountSlider.SetTooltip("Set sharpness filter strength.");
	sharpenFilterAmountSlider.SetScriptTip("RenderPath3D::SetSharpenFilterAmount(float value)");
	sharpenFilterAmountSlider.SetSize(XMFLOAT2(100, hei));
	sharpenFilterAmountSlider.SetPos(XMFLOAT2(x + 100, y));
	sharpenFilterAmountSlider.SetValue(renderer->getSharpenFilterAmount());
	sharpenFilterAmountSlider.OnSlide([=](wiEventArgs args) {
		renderer->setSharpenFilterAmount(args.fValue);
	});
	AddWidget(&sharpenFilterAmountSlider);

	outlineCheckBox.Create("Cartoon Outline: ");
	outlineCheckBox.SetTooltip("Toggle the full screen cartoon outline effect.");
	outlineCheckBox.SetSize(XMFLOAT2(hei, hei));
	outlineCheckBox.SetPos(XMFLOAT2(x, y += step));
	outlineCheckBox.SetCheck(renderer->getOutlineEnabled());
	outlineCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setOutlineEnabled(args.bValue);
	});
	AddWidget(&outlineCheckBox);

	outlineThresholdSlider.Create(0, 1, 0.1f, 1000, "Threshold: ");
	outlineThresholdSlider.SetTooltip("Outline edge detection threshold. Increase if not enough otlines are detected, decrease if too many outlines are detected.");
	outlineThresholdSlider.SetSize(XMFLOAT2(100, hei));
	outlineThresholdSlider.SetPos(XMFLOAT2(x + 100, y));
	outlineThresholdSlider.SetValue(renderer->getOutlineThreshold());
	outlineThresholdSlider.OnSlide([=](wiEventArgs args) {
		renderer->setOutlineThreshold(args.fValue);
	});
	AddWidget(&outlineThresholdSlider);

	outlineThicknessSlider.Create(0, 4, 1, 1000, "Thickness: ");
	outlineThicknessSlider.SetTooltip("Set outline thickness.");
	outlineThicknessSlider.SetSize(XMFLOAT2(100, hei));
	outlineThicknessSlider.SetPos(XMFLOAT2(x + 100, y += step));
	outlineThicknessSlider.SetValue(renderer->getOutlineThickness());
	outlineThicknessSlider.OnSlide([=](wiEventArgs args) {
		renderer->setOutlineThickness(args.fValue);
	});
	AddWidget(&outlineThicknessSlider);

	chromaticaberrationCheckBox.Create("Chromatic Aberration: ");
	chromaticaberrationCheckBox.SetTooltip("Toggle the full screen chromatic aberration effect. This simulates lens distortion at screen edges.");
	chromaticaberrationCheckBox.SetSize(XMFLOAT2(hei, hei));
	chromaticaberrationCheckBox.SetPos(XMFLOAT2(x, y += step));
	chromaticaberrationCheckBox.SetCheck(renderer->getOutlineEnabled());
	chromaticaberrationCheckBox.OnClick([=](wiEventArgs args) {
		renderer->setChromaticAberrationEnabled(args.bValue);
		});
	AddWidget(&chromaticaberrationCheckBox);

	chromaticaberrationSlider.Create(0, 4, 1.0f, 1000, "Amount: ");
	chromaticaberrationSlider.SetTooltip("The lens distortion amount.");
	chromaticaberrationSlider.SetSize(XMFLOAT2(100, hei));
	chromaticaberrationSlider.SetPos(XMFLOAT2(x + 100, y));
	chromaticaberrationSlider.SetValue(renderer->getChromaticAberrationAmount());
	chromaticaberrationSlider.OnSlide([=](wiEventArgs args) {
		renderer->setChromaticAberrationAmount(args.fValue);
		});
	AddWidget(&chromaticaberrationSlider);


	Translate(XMFLOAT3((float)wiRenderer::GetDevice()->GetScreenWidth() - 500, 80, 0));
	SetVisible(false);

}

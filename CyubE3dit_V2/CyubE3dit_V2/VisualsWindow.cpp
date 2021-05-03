#include "stdafx.h"
#include "VisualsWindow.h"
#include "RenderPath3D.h"
#include "CyRender.h"
#include "settings.h"

void VisualsWindow::Create(CyRender* renderer) {
	wiWindow::Create("Visual settings");

	SetSize(XMFLOAT2(580, 530));

	float x = 150, y = 5, step = 20, itemheight = 18;

	renderChunksCheckBox.Create("Show chunks: ");
	renderChunksCheckBox.SetTooltip("Enable, if you want to see the chunks");
	renderChunksCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderChunksCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderChunksCheckBox.SetCheck(true);
	renderChunksCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_CHUNKMESH);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_CHUNKMESH);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderChunksCheckBox);

	renderTreesCheckBox.Create("Show Trees: ");
	renderTreesCheckBox.SetTooltip("Enable, if you want to see the trees");
	renderTreesCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderTreesCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderTreesCheckBox.SetCheck((settings::rendermask & LAYER_TREE));
	renderTreesCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_TREE);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_TREE);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderTreesCheckBox);

	renderMeshesCheckBox.Create("Show Meshes: ");
	renderMeshesCheckBox.SetTooltip("Enable, if you want to see the Meshes (like chairs, chests, ...)");
	renderMeshesCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderMeshesCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderMeshesCheckBox.SetCheck((settings::rendermask & LAYER_MESH));
	renderMeshesCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_MESH);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_MESH);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderMeshesCheckBox);

	renderFoilageCheckBox.Create("Show Foilage: ");
	renderFoilageCheckBox.SetTooltip("Enable, if you want to see the Foilage (like flowers, grass, ...)");
	renderFoilageCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderFoilageCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderFoilageCheckBox.SetCheck((settings::rendermask & LAYER_FOILAGE));
	renderFoilageCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_FOILAGE);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_FOILAGE);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderFoilageCheckBox);

	renderTorchesCheckBox.Create("show Torches: ");
	renderTorchesCheckBox.SetTooltip("Enable, if you want to see torches");
	renderTorchesCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderTorchesCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderTorchesCheckBox.SetCheck(settings::rendermask & LAYER_TORCH);
	renderTorchesCheckBox.OnClick([renderer, this](wiEventArgs args) {
		if (args.bValue) {
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_TORCH);
		} else {
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_TORCH);
		}
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderTorchesCheckBox);

	renderLightsCheckBox.Create("show Torchlights: ");
	renderLightsCheckBox.SetTooltip("Enable, if you want the torches to emit light (little performance impact)");
	renderLightsCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderLightsCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderLightsCheckBox.SetCheck(settings::torchlights);
	renderLightsCheckBox.OnClick([](wiEventArgs args) {
		settings::torchlights = args.bValue;
		wiScene::Scene& scn	  = wiScene::GetScene();
		for (uint32_t i = 0; i < scn.lights.GetCount(); i++) {
			if (scn.lights[i].GetType() == wiScene::LightComponent::LightType::POINT) {
				scn.lights[i].SetStatic(!args.bValue);
			}
		}
	});
	AddWidget(&renderLightsCheckBox);
	/*
	renderEmittersCheckBox.Create("Show Emitters: ");
	renderEmittersCheckBox.SetTooltip("Enable, if you want to see nice fire for the torches (high performance impact)");
	renderEmittersCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderEmittersCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderEmittersCheckBox.SetEnabled(false);
	renderEmittersCheckBox.SetCheck(renderer->getLayerMask() & LAYER_EMITTER);
	renderEmittersCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_EMITTER);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_EMITTER);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderEmittersCheckBox);
	*/
	renderGizmosCheckBox.Create("Show Gizmos: ");
	renderGizmosCheckBox.SetTooltip("Enable, if you want to see the schematic interaction objects (Drag planes, axes and origin as well as buttons).");
	renderGizmosCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderGizmosCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderGizmosCheckBox.SetCheck(renderer->getLayerMask() & LAYER_GIZMO);
	renderGizmosCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_GIZMO);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_GIZMO);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderGizmosCheckBox);
	clipUndergroundCheckBox.Create("Render underground: ");
	clipUndergroundCheckBox.SetTooltip("Enable if you want to see all caves down to bedrock.");
	clipUndergroundCheckBox.SetPos(XMFLOAT2(x, y += step));
	clipUndergroundCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	clipUndergroundCheckBox.SetCheck(!settings::clipUnderground);
	clipUndergroundCheckBox.OnClick([](wiEventArgs args) {
		if (args.bValue) {
			settings::clipUnderground = false;
		} else {
			settings::clipUnderground = true;
		}
	});
	AddWidget(&clipUndergroundCheckBox);

	soundCheckBox.Create("Sound: ");
	soundCheckBox.SetTooltip("Enable, to hear some music and maybe basic ambient noise.");
	soundCheckBox.SetPos(XMFLOAT2(x, y += step * 2));
	soundCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	soundCheckBox.SetCheck(settings::sound);
	if (!renderer->bgSound.IsValid() || !renderer->windSound.IsValid() || !renderer->fireSound.IsValid()) {
		soundCheckBox.SetVisible(false);
	}
	soundCheckBox.OnClick([this, renderer](wiEventArgs args) {
		if (args.bValue) {
			settings::sound = true;
			wiAudio::Play(&renderer->bgSoundinstance);
			wiAudio::Play(&renderer->windSoundinstance);
		} else {
			settings::sound = false;
			wiAudio::Stop(&renderer->bgSoundinstance);
			wiAudio::Stop(&renderer->windSoundinstance);
		}
		musicVol.SetVisible(settings::sound);
		effectVol.SetVisible(settings::sound);
	});
	AddWidget(&soundCheckBox);

	musicVol.Create(0.0, 100.0, settings::musicVol * 100, 100, "Music volume:");
	musicVol.SetTooltip("Adjust the music volume.");
	musicVol.SetPos(XMFLOAT2(x, y += step * 2));
	musicVol.SetSize(XMFLOAT2(100, itemheight));
	musicVol.SetEnabled(settings::sound);
	if (!renderer->bgSound.IsValid() || !renderer->windSound.IsValid() || !renderer->fireSound.IsValid()) {
		musicVol.SetVisible(false);
	}
	musicVol.OnSlide([renderer](wiEventArgs args) {
		if (args.fValue > 100)
			args.fValue = 100;
		else if (args.fValue < 0.)
			args.fValue = 0.;
		args.fValue *= 0.01;
		wiAudio::SetSubmixVolume(wiAudio::SUBMIX_TYPE_MUSIC, args.fValue);
		settings::musicVol = args.fValue;
	});
	AddWidget(&musicVol);

	effectVol.Create(0.0, 100.0, settings::effectVol * 100, 100, "Ambient volume:");
	effectVol.SetTooltip("Adjust the music volume.");
	effectVol.SetPos(XMFLOAT2(x, y += step * 2));
	effectVol.SetSize(XMFLOAT2(100, itemheight));
	effectVol.SetEnabled(settings::sound);
	if (!renderer->bgSound.IsValid() || !renderer->windSound.IsValid() || !renderer->fireSound.IsValid()) {
		effectVol.SetVisible(false);
	}
	effectVol.OnSlide([renderer](wiEventArgs args) {
		if (args.fValue > 100)
			args.fValue = 100;
		else if (args.fValue < 0.)
			args.fValue = 0.;
		args.fValue *= 0.01;
		wiAudio::SetSubmixVolume(wiAudio::SUBMIX_TYPE_SOUNDEFFECT, args.fValue);
		settings::effectVol = args.fValue;
	});
	AddWidget(&effectVol);

	x = 450;
	y = 5;

	pickTypeChunkCheckBox.Create("hover chunks: ");
	pickTypeChunkCheckBox.SetTooltip("Enable if you want to display chunk IDs and bounding box on hovering");
	pickTypeChunkCheckBox.SetPos(XMFLOAT2(x, y += step));
	pickTypeChunkCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	pickTypeChunkCheckBox.SetCheck(settings::pickType & PICK_CHUNK);
	pickTypeChunkCheckBox.OnClick([](wiEventArgs args) {
		if (args.bValue) {
			settings::pickType |= PICK_CHUNK;
		} else {
			settings::pickType &= ~PICK_CHUNK;
		}
	});
	AddWidget(&pickTypeChunkCheckBox);

	pickTypeTreeCheckBox.Create("hover trees: ");
	pickTypeTreeCheckBox.SetTooltip("Enable if you want to display tree bounding box on hovering");
	pickTypeTreeCheckBox.SetPos(XMFLOAT2(x, y += step));
	pickTypeTreeCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	pickTypeTreeCheckBox.SetCheck(settings::pickType & PICK_TREE);
	pickTypeTreeCheckBox.OnClick([](wiEventArgs args) {
		if (args.bValue) {
			settings::pickType |= PICK_TREE;
		} else {
			settings::pickType &= ~PICK_TREE;
		}
	});
	AddWidget(&pickTypeTreeCheckBox);

	temporalAACheckBox.Create("Temporal Antialiasing: ");
	temporalAACheckBox.SetTooltip("Switches the antialiasing method from supersampling to Temporal AA");
	temporalAACheckBox.SetPos(XMFLOAT2(x, y += step * 2));
	temporalAACheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	temporalAACheckBox.SetCheck(settings::volClouds);
	temporalAACheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue) {
			wiRenderer::SetTemporalAAEnabled(true);
			renderer->resolutionScale = 1.f;
			renderer->setFXAAEnabled(false);
			renderer->setSharpenFilterEnabled(true);
		} else {
			wiRenderer::SetTemporalAAEnabled(false);
			renderer->resolutionScale = 1.3f;
			renderer->setSharpenFilterEnabled(false);
			renderer->setFXAAEnabled(true);
		}
		settings::volClouds = args.bValue;
		renderer->ResizeBuffers();
	});
	AddWidget(&temporalAACheckBox);

	volCloudsCheckBox.Create("Volumetric Clouds: ");
	volCloudsCheckBox.SetTooltip("Wanna see nice volumetric clouds?");
	volCloudsCheckBox.SetPos(XMFLOAT2(x, y += step));
	volCloudsCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	volCloudsCheckBox.SetCheck(settings::volClouds);
	volCloudsCheckBox.OnClick([renderer](wiEventArgs args) {
		settings::volClouds = args.bValue;
		renderer->setVolumetricCloudsEnabled(args.bValue);
	});
	AddWidget(&volCloudsCheckBox);

	Translate(XMFLOAT3(50, 50, 0));
	SetVisible(false);
}

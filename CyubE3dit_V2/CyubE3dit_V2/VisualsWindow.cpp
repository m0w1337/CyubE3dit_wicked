#include "stdafx.h"
#include "VisualsWindow.h"
#include "RenderPath3D.h"
#include "CyRender.h"
#include "settings.h"

void VisualsWindow::Create(CyRender* renderer) {
	wiWindow::Create("Visual settings");

	SetSize(XMFLOAT2(580, 530));

	float x = 220, y = 5, step = 20, itemheight = 18;

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
	renderTreesCheckBox.SetCheck(renderer->getLayerMask() & LAYER_TREE);
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
	renderMeshesCheckBox.SetCheck(renderer->getLayerMask() & LAYER_MESH);
	renderMeshesCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_MESH);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_MESH);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderMeshesCheckBox);

	renderTorchesCheckBox.Create("show Torches: ");
	renderTorchesCheckBox.SetTooltip("Enable, if you want to see torches");
	renderTorchesCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderTorchesCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderTorchesCheckBox.SetCheck(renderer->getLayerMask() & LAYER_TORCH);
	renderTorchesCheckBox.OnClick([renderer, this](wiEventArgs args) {
		if (args.bValue) {
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_TORCH);
		}
		else {
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

	renderEmittersCheckBox.Create("Show Emitters: ");
	renderEmittersCheckBox.SetTooltip("Enable, if you want to see nice fire for the torches (high performance impact)");
	renderEmittersCheckBox.SetPos(XMFLOAT2(x, y += step));
	renderEmittersCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderEmittersCheckBox.SetCheck(renderer->getLayerMask() & LAYER_EMITTER);
	renderEmittersCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_EMITTER);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_EMITTER);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&renderEmittersCheckBox);

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

	renderGizmosCheckBox.Create("Ambient Sound: ");
	renderGizmosCheckBox.SetTooltip("Enable, to hear some basic ambient noise.");
	renderGizmosCheckBox.SetPos(XMFLOAT2(x, y += step*2));
	renderGizmosCheckBox.SetSize(XMFLOAT2(itemheight, itemheight));
	renderGizmosCheckBox.SetCheck(settings::sound);
	renderGizmosCheckBox.OnClick([renderer](wiEventArgs args) {
		if (args.bValue)
			renderer->setlayerMask(renderer->getLayerMask() | LAYER_GIZMO);
		else
			renderer->setlayerMask(renderer->getLayerMask() & ~LAYER_GIZMO);
		settings::rendermask = renderer->getLayerMask();
	});
	AddWidget(&soundCheckBox);



	Translate(XMFLOAT3(100, 50, 0));
	SetVisible(false);
}

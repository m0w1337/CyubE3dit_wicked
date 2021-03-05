#pragma once
#include "RenderPath3D_PathTracing.h"
#include "RendererWindow.h"
#include "PostprocessWindow.h"
#include "meshGen.h"
#include "cyBlocks.h"


class CyLoadingScreen : public LoadingScreen {
private:
	wiSprite sprite;
	wiSpriteFont font;

public:
	void Load() override;
	void Update(float dt) override;
};
class CyMainComponent;
class CyRender : public RenderPath3D {
private:
	std::unique_ptr<RenderPath3D> renderPath;
	wiGraphics::RenderPass renderpass_selectionOutline[2];
	wiGraphics::RenderPass renderpass_composeOutline;
	wiGraphics::Texture rt_selectionOutline_MSAA;
	wiGraphics::Texture rt_selectionOutline[2];

public:

		enum EDITORSTENCILREF {
		EDITORSTENCILREF_CLEAR				= 0x00,
		EDITORSTENCILREF_HIGHLIGHT_OBJECT	= 0x01,
		EDITORSTENCILREF_HIGHLIGHT_MATERIAL = 0x02,
		EDITORSTENCILREF_LAST				= 0x0F,
	};
	float selectionOutlineTimer	   = 0;
	const XMFLOAT4 selectionColor  = XMFLOAT4(0.8f, 0.6f, 0, 1.f);
	const XMFLOAT4 selectionColor2 = XMFLOAT4(0, 0.6f, 0.8f, 1.f);

	float lasttime;
	float sinepulse;
	wiComboBox worldSelector;
	wiComboBox camModeSelector;
	RendererWindow rendererWnd;
	PostprocessWindow postprocessWnd;
	wiButton rendererWnd_Toggle;
	wiButton postprocessWnd_Toggle;
	wiButton loadSchBtn;
	wiButton PauseChunkLoading;
	wiSlider viewDist;
	wiLabel label;

	
	CyMainComponent* main;
	wiScene::PickResult hovered;
	void Load() override;
	void Update(float dt) override;
	void ResizeBuffers() override;
	void ResizeLayout() override;
	void Render() const override;
	void Compose(wiGraphics::CommandList cmd) const override;
};
/*
class CyPathRender : public RenderPath3D_PathTracing
{
public:
	void Load() override;

	void Update(float dt) override;
	void ResizeLayout() override;
};
*/
class CyMainComponent : public MainComponent
{
	
public:
	enum EDITORSTENCILREF {
		EDITORSTENCILREF_CLEAR				= 0x00,
		EDITORSTENCILREF_HIGHLIGHT_OBJECT	= 0x01,
		EDITORSTENCILREF_HIGHLIGHT_MATERIAL = 0x02,
		EDITORSTENCILREF_LAST				= 0x0F,
	};

	
	float selectionOutlineTimer	   = 0;
	const XMFLOAT4 selectionColor  = XMFLOAT4(0.4f, 0.6f, 0.1f, 0.5f);
	const XMFLOAT4 selectionColor2 = XMFLOAT4(0.f, 1.f, 0.6f, 0.35f);

	static wiECS::Entity m_headLight;
	static wiECS::Entity m_probe;
	static wiECS::Entity m_dust;
	CyRender renderer;
	CyLoadingScreen loader;
	//CyPathRender pathRenderer;
	void Initialize() override;
	void CreateScene(void);
	void Compose(wiGraphics::CommandList cmd) override;
};


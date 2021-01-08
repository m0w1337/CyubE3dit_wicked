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
public:
	float lasttime;
	float sinepulse;
	wiComboBox worldSelector;
	RendererWindow rendererWnd;
	PostprocessWindow postprocessWnd;
	wiButton rendererWnd_Toggle;
	wiButton postprocessWnd_Toggle;
	wiButton loadSchBtn;
	wiSlider viewDist;
	wiLabel label;
	
	CyMainComponent* main;
	wiScene::PickResult hovered;
	void Load() override;
	void Update(float dt) override;
	void ResizeLayout() override;
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
	static wiECS::Entity m_headLight;
	static wiECS::Entity m_probe;
	CyRender renderer;
	CyLoadingScreen loader;
	//CyPathRender pathRenderer;
	void Initialize() override;
	void CreateScene(void);
	void Compose(wiGraphics::CommandList cmd) override;
};


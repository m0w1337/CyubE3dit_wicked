#pragma once
#include "RenderPath3D_PathTracing.h"
#include "meshGen.h"
class CyRender : public RenderPath3D
{
public:
	void Load() override;
	
	void Update(float dt) override;
	void ResizeLayout() override;
};

class CyPathRender : public RenderPath3D_PathTracing
{
public:
	void Load() override;

	void Update(float dt) override;
	void ResizeLayout() override;
};

class CyMainComponent : public MainComponent
{
	
public:
	CyRender renderer;
	CyPathRender pathRenderer;
	void Initialize() override;
	void CreateScene(void);
};


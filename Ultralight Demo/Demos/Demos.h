#pragma once
#include <PCH.h>
#include "DemoAntiAliasTest/DemoAntiAliasTest.h"
#include "DemoBasic/DemoBasic.h"
#include "DemoInspector/DemoInspector.h"
#include "DemoBorderlessResizable/DemoBorderlessResizable.h"
#include "DemoBorderlessResizableMovable/DemoBorderlessResizableMovable.h"
#include "DemoTransparent/DemoTransparent.h"
#include "DemoCPPTextureInBrowser/DemoCPPTextureInBrowser.h"
#include "DemoOpenFileDialog/DemoOpenFileDialog.h"
#include "DemoJSCppCommunication/DemoJSCPPCommunication.h"
#include "DemoOverlayedCPPTextureOnDiv/DemoOverlayedCPPTextureOnDiv.h"
#include "DemoCubeDraw/DemoCubeDraw.h"
#include "DemoGLTFViewer/DemoGLTFViewer.h"
#include "DemoSelector/DemoSelector.h"

shared_ptr<Engine> GenerateEngineInstanceForDemo(DemoSelector::DemoId demoId)
{
	shared_ptr<Engine> demoInstance = nullptr;
	switch (demoId)
	{
	case DemoSelector::DemoId::DemoAntiAliasTest:
		demoInstance = make_shared<DemoAntiAliasTest>();
		break;
	case DemoSelector::DemoId::DemoBasic:
		demoInstance = make_shared<DemoBasic>();
		break;
	case DemoSelector::DemoId::DemoBorderlessResizable:
		demoInstance = make_shared<DemoBorderlessResizable>();
		break;
	case DemoSelector::DemoId::DemoBorderlessResizableMovable:
		demoInstance = make_shared<DemoBorderlessResizableMovable>();
		break;
	case DemoSelector::DemoId::DemoCPPTextureInBrowser:
		demoInstance = make_shared<DemoCPPTextureInBrowser>();
		break;
	case DemoSelector::DemoId::DemoCubeDraw:
		demoInstance = make_shared<DemoCubeDraw>();
		break;
	case DemoSelector::DemoId::DemoGLTFViewer:
		demoInstance = make_shared<DemoGLTFViewer>();
		break;
	case DemoSelector::DemoId::DemoInspector:
		demoInstance = make_shared<DemoInspector>();
		break;
	case DemoSelector::DemoId::DemoJSCPPCommunication:
		demoInstance = make_shared<DemoJSCPPCommunication>();
		break;
	case DemoSelector::DemoId::DemoOpenFileDialog:
		demoInstance = make_shared<DemoOpenFileDialog>();
		break;
	case DemoSelector::DemoId::DemoTransparent:
		demoInstance = make_shared<DemoTransparent>();
		break;
	case DemoSelector::DemoId::DemoOverlayedCPPTexture:
		demoInstance = make_shared<DemoOverlayedCPPTextureOnDiv>();
		break;
	}
	return demoInstance;
}
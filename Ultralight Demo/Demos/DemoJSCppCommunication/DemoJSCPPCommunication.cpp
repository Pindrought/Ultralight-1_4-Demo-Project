#include "PCH.h"
#include "DemoJSCPPCommunication.h"
#include "../Misc/CursorManager.h"

bool DemoJSCPPCommunication::Startup()
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	WindowCreationParameters windowParms;
	windowParms.Width = screenWidth - 100;
	windowParms.Height = screenHeight - 100;
	windowParms.Style = WindowStyle::Resizable | WindowStyle::ExitButton | WindowStyle::MaximizeAvailable;
	windowParms.Title = "Default Title - Primary Window";
	m_PrimaryWindow = WindowManager::SpawnWindow(windowParms);
	if (m_PrimaryWindow.expired())
	{
		FatalError("Failed to initialize primary window. Program must now abort.");
		return false;
	}

	UltralightViewCreationParameters parms;
	parms.Width = m_PrimaryWindow->GetWidth();
	parms.Height = m_PrimaryWindow->GetHeight();
	parms.IsAccelerated = false;
	parms.ForceMatchWindowDimensions = true;
	parms.IsTransparent = true;

	WeakWrapper<UltralightView> pView = m_UltralightMgr->CreateUltralightView(parms);
	pView->LoadURL("file:///Samples/JSCPPCommunication/JSCPPCommunication.html");
	m_UltralightMgr->SetViewToWindow(pView->GetId(), m_PrimaryWindow->GetId());

	return true;
}

EZJSParm DemoJSCPPCommunication::OnEventCallbackFromUltralight(int32_t viewId, string eventName, vector<EZJSParm> parameters)
{
	if (eventName == "JS_CPP1")
	{ //One number
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Number);
		double num = parameters[0].AsDouble();
		string text = strfmt("Number: %f", num);
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP2")
	{ //One string
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::String);
		string str = parameters[0].AsString();
		string text = strfmt("String: %s", str.c_str());
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP3")
	{ //One boolean
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Boolean);
		bool bValue = parameters[0].AsBool();
		string text;
		if (bValue == true)
		{
			text = "Bool: true";
		}
		else
		{
			text = "Bool: false";
		}
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP4")
	{ //null
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Null);
		MessageBoxA(m_PrimaryWindow->GetHWND(), "Null", eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP5")
	{ //array of numbers
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::Array);
		vector<EZJSParm> entries = parameters[0].AsArray();
		string text = "";
		for (auto& entry : entries)
		{
			assert(entry.GetType() == EZJSParm::Number);
			if (text != "")
			{
				text += "\n";
			}
			text += std::to_string(entry.AsDouble()); 
		}
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP6")
	{ //key value pairs object
		assert(parameters.size() == 1);
		assert(parameters[0].GetType() == EZJSParm::TableKeyValuePair);
		vector<pair<string, EZJSParm>> kvpTable = parameters[0].AsKeyValuePairsTable();
		string text = "";
		for (auto& kvp : kvpTable)
		{
			if (text != "")
			{
				text += "\n";
			}
			//First add key to text
			text += kvp.first;
			text += ": ";
			//Next determine what type value is and add it
			auto& val = kvp.second;
			switch (val.GetType())
			{
			case EZJSParm::Array:
				text += "[ARRAYDATA]";
				break;
			case EZJSParm::Boolean:
				if (val.AsBool() == true)
				{
					text += "TRUE";
				}
				else
				{
					text += "FALSE";
				}
				break;
			case EZJSParm::Null:
				text += "NULL";
				break;
			case EZJSParm::Number:
				text += std::to_string(val.AsDouble());
				break;
			case EZJSParm::String:
				text += val.AsString();
				break;
			case EZJSParm::TableKeyValuePair:
				text += "[TABLEDATA]";
				break;
			default:
				assert(false && "This shouldn't be happening.");
			}
		}
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP7")
	{ //string first parameter, bool 2nd parameter
		assert(parameters.size() == 2);
		assert(parameters[0].GetType() == EZJSParm::String);
		assert(parameters[1].GetType() == EZJSParm::Boolean);
		string s = parameters[0].AsString();
		bool bValue = parameters[1].AsBool();
		string text = "Parm1: " + s + "\n";
		text += "Parm2: ";
		if (bValue == true)
		{
			text += "TRUE";
		}
		else
		{
			text += "FALSE";
		}
		MessageBoxA(m_PrimaryWindow->GetHWND(), text.c_str(), eventName.c_str(), MB_OK);
		return EZJSParm();
	}
	if (eventName == "JS_CPP8")
	{ //Return the current tick count back to JS
		return (int)GetTickCount();
	}
	/////////////////
	//For these below events, we'll need to CallJSFnc on the view, so go ahead and get the view ptr from the view id
	WeakWrapper<UltralightView> pView = m_UltralightMgr->GetViewFromId(viewId);

	if (eventName == "CPP_JS1")
	{ //Send a string
		EZJSParm outReturnVal;
		string outException;
		bool success = pView->CallJSFnc("CPP_JS1",
									   { "This is a string from C++!"},
									   outReturnVal,
									   outException);
		assert(success == true);
		return EZJSParm();
	}

	if (eventName == "CPP_JS2")
	{ //Send a string
		EZJSParm outReturnVal;
		string outException;
		bool success = pView->CallJSFnc("CPP_JS2",
										{ "HelloFromC++", 1593 },
										outReturnVal,
										outException);
		assert(success == true);
		return EZJSParm();
	}

	if (eventName == "CPP_JS3")
	{ //Send a string
		EZJSParm outReturnVal;
		string outException;
		vector<EZJSParm> randomNumbers;
		int numberCount = rand() % 5 + 5;
		for (int i = 0; i < numberCount; i++)
		{
			randomNumbers.push_back(rand());
		}

		EZJSParm randomNumbersArray(randomNumbers);

		bool success = pView->CallJSFnc("CPP_JS3",
										{ randomNumbersArray },
										outReturnVal,
										outException);
		assert(success == true);
		return EZJSParm();
	}

	return EZJSParm();
}

void DemoJSCPPCommunication::OnWindowDestroyStartCallback(int32_t windowId)
{
	WeakWrapper<Window> pWindow = WindowManager::GetWindow(windowId);
	auto pViews = pWindow->GetSortedUltralightViews();
	for (auto pView : pViews)
	{
		m_UltralightMgr->DestroyView(pView);
	}
}

void DemoJSCPPCommunication::OnWindowDestroyEndCallback(int32_t windowId)
{
	if (WindowManager::GetWindowCount() == 0)
	{
		SetRunning(false);
	}
}

void DemoJSCPPCommunication::OnWindowResizeCallback(Window* pWindow)
{
}


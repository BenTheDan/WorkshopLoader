#pragma once
#pragma comment( lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "./imgui/imgui.h"
#include "curl/curl.h"
#include <filesystem>

namespace fs = std::filesystem;

class WsLoader : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
public:
	std::string menuTitle = "Workshop Loader";

	virtual void onLoad();
	virtual void onUnload();

	void SwitchChanged(std::string prev, CVarWrapper curr);
	void EGChanged(std::string prev, CVarWrapper curr);
	void WsChanged(std::string prev, CVarWrapper curr);
	void LoadMap(fs::path map);
	std::string UnloadMap();
	void OpenMenu();
	bool CheckPath(fs::path path, bool logError, std::string warningCvar);
	std::string GenerateBackupName(fs::path path);
	
	fs::path switchFile;
	fs::path eg_path;
	fs::path ws_path;
	std::string status;
	std::string pressed_btn;
public:
	virtual void OnOpen();
	virtual void OnClose();
	virtual void Render();
	virtual void SetImGuiContext(uintptr_t ctx);
	virtual std::string GetMenuTitle();
	virtual std::string GetMenuName();
	virtual bool ShouldBlockInput();
	virtual bool IsActiveOverlay();
};


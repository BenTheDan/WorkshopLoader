#include "WsLoader.h"
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>

BAKKESMOD_PLUGIN(WsLoader, "Workshop Loader", "0.1", 0)

namespace fs = std::filesystem;

std::vector<std::filesystem::path> maps;

void WsLoader::onLoad() {
	cvarManager->registerCvar("switch_map", "Labs_Underpass_P", "The mapfile to replace with workshop map" , true, false, 0.0f, false, 0.0f, true).addOnValueChanged(std::bind(&WsLoader::SwitchChanged, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("eg_path", "C:/Program Files/Epic Games/rocketleague/", "Rocket League path", true, false, 0.0f, false, 0.0f, true).addOnValueChanged(std::bind(&WsLoader::EGChanged, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("ws_path", "C:/Program Files (x86)/Steam/steamapps/workshop/content/252950/", "Workshop maps path", true, false, 0.0f, false, 0.0f, true).addOnValueChanged(std::bind(&WsLoader::WsChanged, this, std::placeholders::_1, std::placeholders::_2));
	cvarManager->registerCvar("sw_notfound", "", "error message", false);
	cvarManager->registerCvar("eg_notfound", "", "error message", false);
	cvarManager->registerCvar("ws_notfound", "", "error message", false);
	cvarManager->registerCvar("loaded_ws", "", "The filename of the last loaded workshop.", false);
	cvarManager->registerNotifier("wsl_openMenu", std::bind(&WsLoader::OpenMenu, this), "Opens workshop menu", 0);

	eg_path = fs::path(fs::absolute(cvarManager->getCvar("eg_path").getStringValue())) / "TAGame\\CookedPCConsole\\";
	ws_path = fs::absolute(cvarManager->getCvar("ws_path").getStringValue());
	switchFile = eg_path / (cvarManager->getCvar("switch_map").getStringValue() + ".upk");

	//Write warnings if any of the default paths are invalid
	CheckPath(eg_path, false, "eg_notfound");
	CheckPath(ws_path, false, "ws_notfound");
	CheckPath(switchFile, false, "sw_notfound");

	pressed_btn = "";
}

void WsLoader::OpenMenu() {
	cvarManager->executeCommand("togglemenu wsloader");
}

void WsLoader::onUnload() {

}

void WsLoader::LoadMap(fs::path map) {
	if (!CheckPath(map, true, "") || !CheckPath(eg_path, true, "") || !CheckPath(switchFile, true, "")) { 
		return;
		status = "Wrong path! Check plugin settings!";
	}
	std::string backup = GenerateBackupName(switchFile);
	if (!CheckPath(backup, false, "")) {
		std::filesystem::copy(switchFile, backup, std::filesystem::copy_options::overwrite_existing);
		cvarManager->log("Created backup: " + fs::path(backup).filename().string());
	}
	std::filesystem::copy(map, switchFile, std::filesystem::copy_options::overwrite_existing);
	cvarManager->log("Loaded: " + map.filename().string());
	status = "Loaded: " + map.filename().string();
	cvarManager->getCvar("loaded_ws").setValue(map.filename().string());
}

std::string WsLoader::UnloadMap() {
	std::string backup = GenerateBackupName(switchFile);
	if (CheckPath(backup, false, "")) {
		std::filesystem::copy(backup, switchFile, std::filesystem::copy_options::overwrite_existing);
		remove(backup.c_str());
		cvarManager->log("Map unloaded.");
		return "Unloaded successfully";
	}
	else {
		return "Couldn't restore backup.";
	}
}

void WsLoader::Render() {
	bool pressed = false;
	ImGui::Begin("wsloader");
	ImGui::Text(("status: " + status).c_str());
	if (maps.empty()) {
		ImGui::Text("No workshop maps found");
	}
	if (ImGui::Button("Unload")) {
		if (pressed_btn != "Unload") { status = UnloadMap(); }
		else { pressed_btn = "Unload"; }
		pressed = true;
	}
	int id = 10;
	for (const fs::path i : maps) {
		ImGui::PushID(id);
		if (ImGui::Button(i.filename().string().c_str())) {
			if (pressed_btn != std::to_string(id)) { LoadMap(i); }
			else { pressed_btn = std::to_string(id); }
			pressed = true;
		}
		ImGui::PopID();
		id++;
	}
	if (!pressed) { pressed_btn = ""; }
	ImGui::End();
}

void WsLoader::OnOpen() {
	if (CheckPath(switchFile, false, "sw_notfound")) {
		//Check whether a workshop is still loaded by searching for a backup of the original map
		if (CheckPath(GenerateBackupName(switchFile), false, "")) {
			status = "Loaded: " + cvarManager->getCvar("loaded_ws").getStringValue();
		}
		else {
			status = "No map loaded";
		}
	}
	else {
		status = "No map loaded";
	}
	maps.clear();
	for (const auto& entry : std::filesystem::directory_iterator(ws_path)) {
		std::string path_str = entry.path().string();
		if (entry.is_directory()) {
			for (const auto& entryy : std::filesystem::directory_iterator(entry.path())) {
				std::string path_strr = entryy.path().string();
				if (!entryy.is_directory() && path_strr.substr(path_strr.length() - 4, 4) == ".udk") {
					maps.push_back(entryy.path());
					break;
				}
			}
		}
		else if (path_str.substr(path_str.length() - 4, 4) == ".udk") {
			maps.push_back(entry.path());
		}
	}
}

void WsLoader::OnClose() {

}

void WsLoader::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool WsLoader::ShouldBlockInput() {
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool WsLoader::IsActiveOverlay() {
	return true;
}

std::string WsLoader::GetMenuName()
{
	return "wsloader";
}

std::string WsLoader::GetMenuTitle()
{
	return menuTitle;
}

//Called when ws_path cvar changes
void WsLoader::WsChanged(std::string prev, CVarWrapper curr) {
	fs::path new_path = fs::absolute(curr.getStringValue());
	if (CheckPath(new_path, false, "ws_notfound")) {
		ws_path = new_path;
	}
}

//Called when switch_map cvar changes
void WsLoader::SwitchChanged(std::string prev, CVarWrapper curr) {
	fs::path new_path = eg_path / (curr.getStringValue() + ".upk");
	if (CheckPath(new_path, false, "sw_notfound")) {
		if (CheckPath(switchFile, false, "") && prev != curr.getStringValue()) { if (GenerateBackupName(switchFile), false, "") { UnloadMap(); } }
		switchFile = new_path;
	}
}

//Called when eg_path cvar changes
void WsLoader::EGChanged(std::string prev, CVarWrapper curr) {
	fs::path path = fs::absolute(curr.getStringValue()) / "TAGame\\CookedPCConsole\\";
	if (CheckPath(path, false, "eg_notfound")) {
		if (CheckPath(switchFile, false, "") && prev != curr.getStringValue()) { if (GenerateBackupName(switchFile), false, "") { UnloadMap(); } }
		eg_path = path;
		if (CheckPath(eg_path / switchFile.filename(), false, "sw_notfound")) {
			switchFile = eg_path / switchFile.filename();
		}
	}
}

//Adds "_BACKUP" to the file name
std::string WsLoader::GenerateBackupName(fs::path path) {
	std::string path_str{ path.string() };
	std::string backup = path_str.substr(0, path_str.length() - 4) + "_BACKUP.upk";
	return backup;
}

//Checks wether the path exists 
bool WsLoader::CheckPath(fs::path path, bool logError = false, std::string warningCvar = "") {
	struct stat info;
	if (stat(path.string().c_str(), &info) != 0) {
		//Log to console
		if (logError) {
			cvarManager->log(path.string() + " not found!");
		}
		//Log to cvar 
		if (warningCvar != "") {
			cvarManager->getCvar(warningCvar).setValue("Warning: " + path.string() + " could not be found!");
		}
		return false;
	}
	else {
		if (warningCvar != "") {
			cvarManager->getCvar(warningCvar).setValue("");
		}
		return true;
	}
}
#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include "SaltyNX.h"
#include <dirent.h>

bool* def = 0;
bool* isDocked = 0;
bool* pluginActive = 0;
bool _isDocked = false;
bool _def = true;
bool PluginRunning = false;
bool state = false;
bool closed = false;
bool check = false;
bool SaltySD = false;
bool bak = false;
bool plugin = true;
char saveChar[32];
char DockedChar[32];
char SystemChar[32];
char HandheldDDR[32];
char DockedDDR[32];
uint64_t PID = 0;
Handle remoteSharedMemory = 1;
SharedMemory _sharedmemory = {};
bool SharedMemoryUsed = false;

enum res_mode {
	res_mode_default = 0,
	res_mode_480p = 1,
	res_mode_540p = 2,
	res_mode_630p = 3,
	res_mode_720p = 4,
	res_mode_810p = 5,
	res_mode_900p = 6,
	res_mode_1080p = 7,
	res_mode_amount = 8
};

struct resolutionMode {
	res_mode handheld_res: 4;
	res_mode docked_res: 4;
} PACKED;
bool* _wasDDRused = 0;

resolutionMode* res_mode_ptr = 0;

std::pair<int, int> resolutions[] = {{0 ,0}, {854, 480}, {960, 540}, {1120, 630}, {1280, 720}, {1440, 810}, {1600, 900}, {1920, 1080}};

bool writeSave() {
	uint64_t titid = 0;
	if (R_FAILED(pmdmntGetProgramId(&titid, PID))) {
		return false;
	}
	char path[128];
	DIR* dir = opendir("sdmc:/SaltySD/plugins/ReverseNX-RT/");
	if (!dir) {
		mkdir("sdmc:/SaltySD/plugins/", 777);
		mkdir("sdmc:/SaltySD/plugins/ReverseNX-RT/", 777);
	}
	else closedir(dir);
	snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/ReverseNX-RT/%016lX.dat", titid);
	if (_def) {
		remove(path);
		return true;
	}
	FILE* save_file = fopen(path, "wb");
	if (!save_file)
		return false;
	fprintf(save_file, "NXRT");
	uint8_t version = 2;
	fwrite(&version, 1, 1, save_file);
	fwrite(&_isDocked, 1, 1, save_file);
	uint8_t resolutionModeH = (uint8_t)(res_mode_ptr->handheld_res);
	uint8_t resolutionModeD = (uint8_t)(res_mode_ptr->docked_res);
	fwrite(&resolutionModeH, 1, 1, save_file);
	fwrite(&resolutionModeD, 1, 1, save_file);
	fclose(save_file);
	return true;
}

bool LoadSharedMemory() {
	if (SaltySD_Connect())
		return false;

	SaltySD_GetSharedMemoryHandle(&remoteSharedMemory);
	SaltySD_Term();

	shmemLoadRemote(&_sharedmemory, remoteSharedMemory, 0x1000, Perm_Rw);
	if (!shmemMap(&_sharedmemory)) {
		SharedMemoryUsed = true;
		return true;
	}
	return false;
}

ptrdiff_t searchSharedMemoryBlock(uintptr_t base) {
	ptrdiff_t search_offset = 0;
	while(search_offset < 0x1000) {
		uint32_t* MAGIC_shared = (uint32_t*)(base + search_offset);
		if (*MAGIC_shared == 0x5452584E) {
			return search_offset;
		}
		else search_offset += 4;
	}
	return -1;
}

bool CheckPort () {
	Handle saltysd;
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			break;
		}
		else {
			if (i == 66) return false;
			svcSleepThread(1'000'000);
		}
	}
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			return true;
		}
		else svcSleepThread(1'000'000);
	}
	return false;
}

class ResolutionModeMenu : public tsl::Gui {
public:
	bool _isDocked = false;
	ResolutionModeMenu (bool isDocked) {
		_isDocked = isDocked;
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("ReverseNX-RT", _isDocked ? "Change Docked Default Display Resolution" : "Change Handheld Default Display Resolution");

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		auto *clickableListItem2 = new tsl::elm::ListItem("Default");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				if (_isDocked) res_mode_ptr->docked_res = res_mode_default;
				else res_mode_ptr->handheld_res = res_mode_default;
				tsl::goBack();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);
		
		for (uint32_t i = 1; i < res_mode_amount; i++) {
			char Hz[] = "1920x1080";
			snprintf(Hz, sizeof(Hz), "%dx%d", resolutions[i].first, resolutions[i].second);
			auto *clickableListItem = new tsl::elm::ListItem(Hz);
			clickableListItem->setClickListener([this, i](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (_isDocked) res_mode_ptr->docked_res = (res_mode)i;
					else res_mode_ptr->handheld_res = (res_mode)i;
					tsl::goBack();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem);
		}

		frame->setContent(list);

		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (!PluginRunning) {
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class GuiTest : public tsl::Gui {
public:
	GuiTest(u8 arg1, u8 arg2, bool arg3) {}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("ReverseNX-RT", APP_VERSION);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();
		
		list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			if (!SaltySD) {
				renderer->drawString("SaltyNX is not working!", false, x, y+50, 20, renderer->a(0xF33F));
			}
			else if (!check) {
				if (closed) {
					renderer->drawString("Game was closed! Overlay disabled!", false, x, y+20, 19, renderer->a(0xF33F));
				}
				else {
					renderer->drawString("Game is not running! Overlay disabled!", false, x, y+20, 19, renderer->a(0xF33F));
				}
			}
			else if (!PluginRunning) {
				renderer->drawString("Game is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString("ReverseNX-RT is not running!", false, x, y+40, 20, renderer->a(0xF33F));
			}
			else {
				renderer->drawString("ReverseNX-RT is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				if (!*pluginActive) renderer->drawString("Game didn't check any mode!", false, x, y+40, 18, renderer->a(0xF33F));
				else {
					renderer->drawString(SystemChar, false, x, y+42, 20, renderer->a(0xFFFF));
					renderer->drawString(DockedChar, false, x, y+64, 20, renderer->a(0xFFFF));
					if (!*def) {
						if (*_wasDDRused) {
							renderer->drawString(HandheldDDR, false, x, y+86, 20, renderer->a(0xFFFF));
							renderer->drawString(DockedDDR, false, x, y+108, 20, renderer->a(0xFFFF));
						}
						else {
							renderer->drawString("Default Display Resolution", false, x, y+86, 20, renderer->a(0xFFFF));
							renderer->drawString("was not checked!", false, x, y+108, 20, renderer->a(0xFFFF));							
						}
					}
				}
				renderer->drawString(saveChar, false, x, y+130, 20, renderer->a(0xFFFF));
			}
	}), 150);

		if (PluginRunning && *pluginActive) {

			auto *clickableListItem = new tsl::elm::ListItem("Change system control");
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					*def = !*def;
					tsl::goBack();
					tsl::changeTo<GuiTest>(1, 2, true);
					return true;
				}

				return false;
			});

			list->addItem(clickableListItem);

			if (!*def) {

				auto *clickableListItem2 = new tsl::elm::ListItem("Change mode");
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						*isDocked = !*isDocked;
						return true;
					}
					
					return false;
				});
				list->addItem(clickableListItem2);

				if (*_wasDDRused) {
					auto *clickableListItem3 = new tsl::elm::ListItem("Change Handheld DDR");
					clickableListItem3->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning) {
							tsl::changeTo<ResolutionModeMenu>(false);
							return true;
						}
						
						return false;
					});
					list->addItem(clickableListItem3);

					auto *clickableListItem4 = new tsl::elm::ListItem("Change Docked DDR");
					clickableListItem4->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning) {
							tsl::changeTo<ResolutionModeMenu>(true);
							return true;
						}
						
						return false;
					});
					list->addItem(clickableListItem4);
				}
			}

			auto *clickableListItem3 = new tsl::elm::ListItem("Save current settings");
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (writeSave())
						snprintf(saveChar, sizeof(saveChar), "Settings saved successfully!");
					else snprintf(saveChar, sizeof(saveChar), "Saving settings failed!");
					return true;
				}
				
				return false;
			});
			list->addItem(clickableListItem3);
		}

		// Add the list to the frame for it to be drawn
		frame->setContent(list);
        
		// Return the frame to have it become the top level element of this Gui
		return frame;
	}

	// Called once every frame to update values
	virtual void update() override {
		static uint8_t i = 10;
		Result rc = pmdmntGetApplicationProcessId(&PID);
		if (R_FAILED(rc) && PluginRunning) {
			PluginRunning = false;
			check = false;
			closed = true;
		}

		if (PluginRunning) {
			if (i > 9) {
				_def = *def;
				_isDocked = *isDocked;
				i = 0;
				
				if (_def) sprintf(SystemChar, "Controlled by system: Yes");
				else sprintf(SystemChar, "Controlled by system: No");

				if (_def) {
					if (_isDocked) sprintf(DockedChar, "Mode: Docked");
					else sprintf(DockedChar, "Mode: Handheld");
				}
				else {
					if (_isDocked) sprintf(DockedChar, "Mode: Fake Docked");
					else sprintf(DockedChar, "Mode: Fake Handheld");
				}

				if (!res_mode_ptr->handheld_res) strcpy(HandheldDDR, "Handheld DDR: Default");
				else snprintf(HandheldDDR, sizeof(HandheldDDR), "Handheld DDR: %dx%d", resolutions[res_mode_ptr->handheld_res].first, resolutions[res_mode_ptr->handheld_res].second);
				if (!res_mode_ptr->docked_res) strcpy(DockedDDR, "Docked DDR: Default");
				else snprintf(DockedDDR, sizeof(DockedDDR), "Docked DDR: %dx%d", resolutions[res_mode_ptr->docked_res].first, resolutions[res_mode_ptr->docked_res].second);
			}
			else i++;
		}
	
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & HidNpadButton_B) {
			tsl::goBack();
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class Dummy : public tsl::Gui {
public:
	Dummy(u8 arg1, u8 arg2, bool arg3) {}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame("ReverseNX-RT", APP_VERSION);
		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		tsl::changeTo<GuiTest>(0, 1, true);
		return true;   // Return true here to singal the inputs have been consumed
	}
};

class OverlayTest : public tsl::Overlay {
public:
	// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
	virtual void initServices() override {

		tsl::hlp::doWithSmSession([]{
			
			fsdevMountSdmc();
			SaltySD = CheckPort();
			if (!SaltySD) return;

			if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) return;
			check = true;
			
			if(!LoadSharedMemory()) return;

			if (!PluginRunning) {
				uintptr_t base = (uintptr_t)shmemGetAddr(&_sharedmemory);
				ptrdiff_t rel_offset = searchSharedMemoryBlock(base);
				if (rel_offset > -1) {
					isDocked = (bool*)(base + rel_offset + 4);
					def = (bool*)(base + rel_offset + 5);
					pluginActive = (bool*)(base + rel_offset + 6);
					res_mode_ptr = (resolutionMode*)(base + rel_offset + 7);
					_wasDDRused = (bool*)(base + rel_offset + 8);
					PluginRunning = true;
				}		
			}
		
		});
	
	}  // Called at the start to initialize all services necessary for this Overlay
	
	virtual void exitServices() override {
		shmemClose(&_sharedmemory);
		fsdevUnmountDevice("sdmc");
	}  // Callet at the end to clean up all services previously initialized

	virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
	
	virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

	virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
		return initially<Dummy>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
	}
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}

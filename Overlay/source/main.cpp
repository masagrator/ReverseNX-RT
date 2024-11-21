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
uint64_t PID = 0;
Handle remoteSharedMemory = 1;
SharedMemory _sharedmemory = {};
bool SharedMemoryUsed = false;

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
	uint8_t version = 1;
	fwrite(&version, 1, 1, save_file);
	fwrite(&_isDocked, 1, 1, save_file);
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

class GuiTest : public tsl::Gui {
public:
	GuiTest(u8 arg1, u8 arg2, bool arg3) { }

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
					renderer->drawString(SystemChar, false, x, y+40, 20, renderer->a(0xFFFF));
					renderer->drawString(DockedChar, false, x, y+60, 20, renderer->a(0xFFFF));
				}
				renderer->drawString(saveChar, false, x, y+80, 20, renderer->a(0xFFFF));
			}
	}), 120);

		if (PluginRunning && *pluginActive) {
			auto *clickableListItem = new tsl::elm::ListItem("Change system control");
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					_def = !_def;
					*def = _def;
					return true;
				}

				return false;
			});

			list->addItem(clickableListItem);
			
			auto *clickableListItem2 = new tsl::elm::ListItem("Change mode");
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning && !_def) {
					_isDocked = !_isDocked;
					*isDocked = _isDocked;
					return true;
				}
				
				return false;
			});
			list->addItem(clickableListItem2);

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

				if (_isDocked) sprintf(DockedChar, "Mode: Docked");
				else sprintf(DockedChar, "Mode: Handheld");
				
				if (_def) sprintf(SystemChar, "Controlled by system: Yes");
				else sprintf(SystemChar, "Controlled by system: No");
			}
			else i++;
		}
	
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		return false;   // Return true here to singal the inputs have been consumed
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
		return initially<GuiTest>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
	}
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}

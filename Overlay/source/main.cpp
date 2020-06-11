#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include "libstratosphere/dmntcht.hpp"

bool def = true;
bool isDocked = false;
bool PluginRunning = false;
bool closed = false;
Handle debug;
uintptr_t docked_address = 0x0;
uintptr_t def_address = 0x0;
uintptr_t MAGIC_address = 0x0;
uint64_t PID = 0;
uint32_t MAGIC = 0x0;
bool check = false;
bool dmntcht = false;
bool SaltySD = false;
bool bak = false;
bool plugin = false;
char DockedChar[32];
char SystemChar[32];
char PluginChar[36];

bool CheckPort () {
	Result ret;
	Handle saltysd;
	for (int i = 0; i < 200; i++) {
		ret = svcConnectToNamedPort(&saltysd, "InjectServ");
		svcSleepThread(1'000'000);
		
		if (R_SUCCEEDED(ret)) {
			svcCloseHandle(saltysd);
			break;
		}
	}
	if (R_FAILED(ret)) return false;
	for (int i = 0; i < 200; i++) {
		ret = svcConnectToNamedPort(&saltysd, "InjectServ");
		svcSleepThread(1'000'000);
		
		if (R_SUCCEEDED(ret)) {
			svcCloseHandle(saltysd);
			break;
		}
	}
	if (R_FAILED(ret)) return false;
	else return true;
}

bool isServiceRunning(const char *serviceName) {	
	Handle handle;	
	SmServiceName service_name = smEncodeName(serviceName);	
	bool running = R_FAILED(smRegisterService(&handle, service_name, false, 1));	

	svcCloseHandle(handle);	

	if (!running) smUnregisterService(service_name);	

	return running;	
}

class GuiTest : public tsl::Gui {
public:
	GuiTest(u8 arg1, u8 arg2, bool arg3) { }

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("ReverseNX-RT", "v1.0.1");

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();
		
		list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			if (SaltySD == false) renderer->drawString("SaltyNX is not working!", false, x, y+50, 20, renderer->a(0xF33F));
			else if (plugin == false) renderer->drawString("Can't detect ReverseNX-RT plugin!", false, x, y+50, 20, renderer->a(0xF33F));
			else if (check == false) {
				if (closed == true) renderer->drawString("Game was closed! Overlay disabled!\nExit from overlay and run game first!", false, x, y+20, 19, renderer->a(0xF33F));
				else renderer->drawString("Game is not running! Overlay disabled!\nExit from overlay and run game first!", false, x, y+20, 19, renderer->a(0xF33F));
				renderer->drawString(PluginChar, false, x, y+60, 20, renderer->a(0xFFFF));
			}
			else if (PluginRunning == false) {
				renderer->drawString("Game is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString("ReverseNX-RT is not running!", false, x, y+40, 20, renderer->a(0xF33F));
				renderer->drawString(PluginChar, false, x, y+60, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString("ReverseNX-RT plugin is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				if (MAGIC == 0x16BA7E39) renderer->drawString("Game doesn't support changing modes!", false, x, y+40, 18, renderer->a(0xF33F));
				else if (MAGIC != 0x06BA7E39) renderer->drawString("WRONG MAGIC!", false, x, y+40, 20, renderer->a(0xF33F));
				else {
					renderer->drawString(SystemChar, false, x, y+40, 20, renderer->a(0xFFFF));
					renderer->drawString(DockedChar, false, x, y+60, 20, renderer->a(0xFFFF));
				}
			}
	}), 100);

		if (MAGIC == 0x06BA7E39) {
			auto *clickableListItem = new tsl::elm::ListItem("Change system control");
			clickableListItem->setClickListener([](u64 keys) { 
				if (keys & KEY_A) {
					if (PluginRunning == true) {
						def = !def;
						if (dmntcht == true) {
							dmntchtWriteCheatProcessMemory(def_address, &def, 0x1);
							dmntchtReadCheatProcessMemory(def_address, &def, 0x1);
							return true;
						}
						else if (R_SUCCEEDED(svcDebugActiveProcess(&debug, PID))) {
							svcWriteDebugProcessMemory(debug, &def, def_address, 0x1);
							svcReadDebugProcessMemory(&def, debug, def_address, 0x1);
							svcCloseHandle(debug);
							return true;
						}
					}
					else return false;
				}

				return false;
			});

			list->addItem(clickableListItem);
			
			auto *clickableListItem2 = new tsl::elm::ListItem("Change mode");
			clickableListItem2->setClickListener([](u64 keys) { 
				if (keys & KEY_A) {
					if (PluginRunning == true && def == false) {
						isDocked =! isDocked;
						if (dmntcht == true) {
							dmntchtWriteCheatProcessMemory(docked_address, &isDocked, 0x1);
							dmntchtReadCheatProcessMemory(docked_address, &isDocked, 0x1);
							return true;
						}
						else if (R_SUCCEEDED(svcDebugActiveProcess(&debug, PID))) {
							svcWriteDebugProcessMemory(debug, &isDocked, docked_address, 0x1);
							svcReadDebugProcessMemory(&isDocked, debug, docked_address, 0x1);
							svcCloseHandle(debug);
							return true;
						}
					}
					else return false;
				}
				
				return false;
			});
			list->addItem(clickableListItem2);
		}
		else if (SaltySD == true && plugin == true && check == false) {
			auto *clickableListItem = new tsl::elm::ListItem("(De)activate plugin");
			clickableListItem->setClickListener([](u64 keys) { 
				if (keys & KEY_A) {
					if (bak == false) {
						rename("sdmc:/SaltySD/plugins/ReverseNX-RT.elf", "sdmc:/SaltySD/plugins/ReverseNX-RT.elf.bak");
						bak = true;
					}
					else {
						rename("sdmc:/SaltySD/plugins/ReverseNX-RT.elf.bak", "sdmc:/SaltySD/plugins/ReverseNX-RT.elf");
						bak = false;
					}
					return true;
				}
				
				return false;
			});
			list->addItem(clickableListItem);
		}

		// Add the list to the frame for it to be drawn
		frame->setContent(list);
        
		// Return the frame to have it become the top level element of this Gui
		return frame;
	}

	// Called once every frame to update values
	virtual void update() override {
		static uint8_t i = 0;
		if (R_FAILED(pmdmntGetApplicationProcessId(&PID)) && PluginRunning == true) {
			remove("sdmc:/SaltySD/ReverseNX-RT.hex");
			PluginRunning = false;
			check = false;
			closed = true;
		}
		if (PluginRunning == true) {
			if (i > 59) {
				if (dmntcht == true) dmntchtReadCheatProcessMemory(docked_address, &isDocked, 0x1);
				else if (R_SUCCEEDED(svcDebugActiveProcess(&debug, PID))) {
					svcReadDebugProcessMemory(&isDocked, debug, docked_address, 0x1);
					svcCloseHandle(debug);
				}
				i = 0;
			}
			else i++;
		}
		
		if (isDocked == true) sprintf(DockedChar, "Mode: Docked");
		else sprintf(DockedChar, "Mode: Handheld");
		
		if (def == true) sprintf(SystemChar, "Controlled by system: Yes");
		else sprintf(SystemChar, "Controlled by system: No");
		
		if (bak == false) sprintf(PluginChar, "ReverseNX-RT plugin is activated.");
		else sprintf(PluginChar, "ReverseNX-RT plugin is deactivated.");
	
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class OverlayTest : public tsl::Overlay {
public:
	// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
	virtual void initServices() override {
		smInitialize();
		fsdevMountSdmc();
		
		SaltySD = CheckPort();
		if (SaltySD == false) return;
		
		FILE* temp = fopen("sdmc:/SaltySD/plugins/ReverseNX-RT.elf", "r");
		if (temp != NULL) {
			fclose(temp);
			plugin = true;
			sprintf(PluginChar, "ReverseNX-RT plugin is activated.");
		}
		else {
			temp = fopen("sdmc:/SaltySD/plugins/ReverseNX-RT.elf.bak", "r");
			if (temp != NULL) {
				fclose(temp);
				plugin = true;
				bak = true;
				sprintf(PluginChar, "ReverseNX-RT plugin is deactivated.");
			}
			else return;
		}

		if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) remove("sdmc:/SaltySD/ReverseNX-RT.hex");
		else {
			check = true;
			svcSleepThread(1'000'000'000);
			FILE* offset = fopen("sdmc:/SaltySD/ReverseNX-RT.hex", "rb");
			if (offset != NULL) {
				fread(&docked_address, 0x5, 1, offset);
				fread(&def_address, 0x5, 1, offset);
				fread(&MAGIC_address, 0x5, 1, offset);
				fclose(offset);
				
				dmntcht = isServiceRunning("dmnt:cht");
				
				if (dmntcht == true) {
					if (R_SUCCEEDED(dmntchtInitialize())) {
						bool out = false;
						dmntchtHasCheatProcess(&out);
						if (out == false) dmntchtForceOpenCheatProcess();
						dmntchtReadCheatProcessMemory(docked_address, &isDocked, 0x1);
						dmntchtReadCheatProcessMemory(def_address, &def, 0x1);
						dmntchtReadCheatProcessMemory(MAGIC_address, &MAGIC, 0x4);
						PluginRunning = true;
					}
					else dmntcht = false;
				}

				if (dmntcht == false) {
					svcSleepThread(1'000'000'000);
					if (R_SUCCEEDED(svcDebugActiveProcess(&debug, PID))) {
						svcReadDebugProcessMemory(&isDocked, debug, docked_address, 0x1);
						svcReadDebugProcessMemory(&def, debug, def_address, 0x1);
						svcReadDebugProcessMemory(&MAGIC, debug, MAGIC_address, 0x4);
						svcCloseHandle(debug);
						PluginRunning = true;
					}
				}
			}
		}
		
		if (isDocked == true) sprintf(DockedChar, "Mode: Docked");
		else sprintf(DockedChar, "Mode: Handheld");
		
		if (def == true) sprintf(SystemChar, "Controlled by system: Yes");
		else sprintf(SystemChar, "Controlled by system: No");
	
	}  // Called at the start to initialize all services necessary for this Overlay
	
	virtual void exitServices() override {
		dmntchtExit();
		if (dmntcht == false) svcCloseHandle(debug);
		fsdevUnmountDevice("sdmc");
		smExit();
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

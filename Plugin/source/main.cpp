#include <switch_min.h>

#include "saltysd/SaltySD_core.h"
#include "saltysd/SaltySD_ipc.h"
#include "saltysd/SaltySD_dynamic.h"

extern "C" {
	extern u32 __start__;

	static char g_heap[0x4000];

	void __libnx_init(void* ctx, Handle main_thread, void* saved_lr);
	void __attribute__((weak)) NORETURN __libnx_exit(int rc);
	void __nx_exit(int, void*);
	void __libc_fini_array(void);
	void __libc_init_array(void);
	extern u32 _ZN2nn2oe18GetPerformanceModeEv() LINKABLE;
	extern u8 _ZN2nn2oe16GetOperationModeEv() LINKABLE;
	extern bool _ZN2nn2oe25TryPopNotificationMessageEPj (int &Message) LINKABLE;
	extern int _ZN2nn2oe22PopNotificationMessageEv() LINKABLE;
}

u32 __nx_applet_type = AppletType_None;

Handle orig_main_thread;
void* orig_ctx;
void* orig_saved_lr;
bool def = true;
uint8_t isDocked = 0;
uint32_t MAGIC = 0x16BA7E39;
const char* ver = "1.0";

void __libnx_init(void* ctx, Handle main_thread, void* saved_lr) {
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = &g_heap[0];
	fake_heap_end   = &g_heap[sizeof g_heap];
	
	orig_ctx = ctx;
	orig_main_thread = main_thread;
	orig_saved_lr = saved_lr;
	
	// Call constructors.
	//void __libc_init_array(void);
	__libc_init_array();
}

void __attribute__((weak)) NORETURN __libnx_exit(int rc) {
	// Call destructors.
	//void __libc_fini_array(void);
	__libc_fini_array();

	SaltySD_printf("SaltySD Plugin: jumping to %p\n", orig_saved_lr);

	__nx_exit(0, orig_saved_lr);
	while (true);
}

bool TryPopNotificationMessage(int &msg) {

	static bool check1 = true;
	static bool check2 = true;
	static bool compare = false;
	static bool compare2 = false;
	
	if (MAGIC == 0x16BA7E39) MAGIC = 0x06BA7E39;

	if (def == true) {
		if (check1 == false) {
			check1 = true;
			msg = 0x1f;
			return true;
		}
		else if (check2 == false) {
			check2 = true;
			msg = 0x1e;
			return true;
		}
		else return _ZN2nn2oe25TryPopNotificationMessageEPj(msg);
	}
	
	check1 = false;
	check2 = false;
	if (compare2 != isDocked) {
		compare2 = isDocked;
		msg = 0x1f;
		return true;
	}
	if (compare != isDocked) {
		compare = isDocked;
		msg = 0x1e;
		return true;
	}
	
	return _ZN2nn2oe25TryPopNotificationMessageEPj(msg);
}

int PopNotificationMessage() {
	
	static bool check1 = true;
	static bool check2 = true;
	static bool compare = false;
	static bool compare2 = false;
	
	if (MAGIC == 0x16BA7E39) MAGIC = 0x06BA7E39;
	
	if (def == true) {
		if (check1 == false) {
			check1 = true;
			return 0x1e;
		}
		else if (check2 == false) {
			check2 = true;
			return 0x1f;
		}
		else return _ZN2nn2oe22PopNotificationMessageEv();
	}
	
	check1 = false;
	check2 = false;

	if (compare2 != isDocked) {
		compare2 = isDocked;
		return 0x1e;
	}
	else if (compare != isDocked) {
		compare = isDocked;
		return 0x1f;
	}
	
	return _ZN2nn2oe22PopNotificationMessageEv();
}

uint32_t GetPerformanceMode() {
	if (def == true) isDocked = _ZN2nn2oe18GetPerformanceModeEv();
	
	return isDocked;
}

uint8_t GetOperationMode() {
	if (def == true) isDocked = _ZN2nn2oe16GetOperationModeEv();
	
	return isDocked;
}

int main(int argc, char *argv[]) {
	SaltySD_printf("SaltySD ReverseNX-RT %s: alive\n", ver);
	
	FILE* offset = SaltySDCore_fopen("sdmc:/SaltySD/ReverseNX-RT.hex", "wb");
	uint64_t ptr = (uint64_t)&isDocked;
	SaltySDCore_fwrite(&ptr, 0x5, 1, offset);
	ptr = (uint64_t)&def;
	SaltySDCore_fwrite(&ptr, 0x5, 1, offset);
	ptr = (uint64_t)&MAGIC;
	SaltySDCore_fwrite(&ptr, 0x5, 1, offset);
	SaltySDCore_fclose(offset);
	
	SaltySDCore_ReplaceImport("_ZN2nn2oe25TryPopNotificationMessageEPj", (void*)TryPopNotificationMessage);
	SaltySDCore_ReplaceImport("_ZN2nn2oe22PopNotificationMessageEv", (void*)PopNotificationMessage);
	SaltySDCore_ReplaceImport("_ZN2nn2oe18GetPerformanceModeEv", (void*)GetPerformanceMode);
	SaltySDCore_ReplaceImport("_ZN2nn2oe16GetOperationModeEv", (void*)GetOperationMode);
	
	SaltySD_printf("SaltySD ReverseNX-RT %s: injection finished\n", ver);
}

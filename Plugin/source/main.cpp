#include <switch_min.h>

#include "saltysd/saltysd_core.h"
#include "saltysd/saltysd_ipc.h"
#include "saltysd/saltysd_dynamic.h"

extern "C" {
	struct SystemEvent {
		const char reserved[16];
		bool flag;
	};

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
	extern void _ZN2nn2oe27GetDefaultDisplayResolutionEPiS1_(int* width, int* height) LINKABLE;
	extern void _ZN2nn2oe38GetDefaultDisplayResolutionChangeEventEPNS_2os11SystemEventE(SystemEvent* systemEvent) LINKABLE;
	extern bool _ZN2nn2os18TryWaitSystemEventEPNS0_15SystemEventTypeE(SystemEvent* systemEvent) LINKABLE;
}

u32 __nx_applet_type = AppletType_None;

Handle orig_main_thread;
void* orig_ctx;
void* orig_saved_lr;
bool* def_shared = 0;
bool* isDocked_shared = 0;
bool* pluginActive_shared = 0;
const char* ver = "1.1";

SharedMemory _sharedmemory = {};
Handle remoteSharedMemory = 0;
ptrdiff_t SharedMemoryOffset = -1;

SystemEvent* systemEventCopy = 0;

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
	virtmemSetup();
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

	*pluginActive_shared = true;

	if (*def_shared) {
		if (!check1) {
			check1 = true;
			msg = 0x1f;
			return true;
		}
		else if (!check2) {
			check2 = true;
			msg = 0x1e;
			return true;
		}
		else return _ZN2nn2oe25TryPopNotificationMessageEPj(msg);
	}
	
	check1 = false;
	check2 = false;
	if (compare2 != *isDocked_shared) {
		compare2 = *isDocked_shared;
		msg = 0x1f;
		return true;
	}
	if (compare != *isDocked_shared) {
		compare = *isDocked_shared;
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
	
	*pluginActive_shared = true;
	
	if (*def_shared) {
		if (!check1) {
			check1 = true;
			return 0x1e;
		}
		else if (!check2) {
			check2 = true;
			return 0x1f;
		}
		else return _ZN2nn2oe22PopNotificationMessageEv();
	}
	
	check1 = false;
	check2 = false;

	if (compare2 != *isDocked_shared) {
		compare2 = *isDocked_shared;
		return 0x1e;
	}
	else if (compare != *isDocked_shared) {
		compare = *isDocked_shared;
		return 0x1f;
	}
	
	return _ZN2nn2oe22PopNotificationMessageEv();
}

uint32_t GetPerformanceMode() {
	if (*def_shared) *isDocked_shared = _ZN2nn2oe18GetPerformanceModeEv();
	
	return *isDocked_shared;
}

uint8_t GetOperationMode() {
	if (*def_shared) *isDocked_shared = _ZN2nn2oe16GetOperationModeEv();
	
	return *isDocked_shared;
}

void GetDefaultDisplayResolution(int* width, int* height) {
	if (*def_shared) {
		_ZN2nn2oe27GetDefaultDisplayResolutionEPiS1_(width, height);
		if (*width == 1920) *isDocked_shared = true;
		else *isDocked_shared = false;
	}
	else if (*isDocked_shared) {
		*width = 1920;
		*height = 1080;
	}
	else {
		*width = 1280;
		*height = 720;
	}
}

void GetDefaultDisplayResolutionChangeEvent(SystemEvent* systemEvent) {
	_ZN2nn2oe38GetDefaultDisplayResolutionChangeEventEPNS_2os11SystemEventE(systemEvent);
	systemEventCopy = systemEvent;
}

bool TryWaitSystemEvent(SystemEvent* systemEvent) {
	static bool check = true;
	static bool compare = false;

	if (systemEvent != systemEventCopy || *def_shared) {
		bool ret = _ZN2nn2os18TryWaitSystemEventEPNS0_15SystemEventTypeE(systemEvent);
		compare = *isDocked_shared;
		if (!check) {
			check = true;
			return true;
		}
		return ret;
	}
	check = false;

	if (compare != *isDocked_shared) {
		compare = *isDocked_shared;
		return true;
	}
	return false;
}

int main(int argc, char *argv[]) {
	SaltySDCore_printf("ReverseNX-RT %s: alive\n", ver);
	Result ret = SaltySD_CheckIfSharedMemoryAvailable(&SharedMemoryOffset, 7);
	SaltySDCore_printf("ReverseNX-RT: SharedMemory ret: 0x%X\n", ret);
	if (!ret) {
		SaltySDCore_printf("ReverseNX-RT: SharedMemory MemoryOffset: %d\n", SharedMemoryOffset);
		SaltySD_GetSharedMemoryHandle(&remoteSharedMemory);
		shmemLoadRemote(&_sharedmemory, remoteSharedMemory, 0x1000, Perm_Rw);
		Result rc = shmemMap(&_sharedmemory);
		if (R_SUCCEEDED(rc)) {
			uintptr_t base = (uintptr_t)shmemGetAddr(&_sharedmemory) + SharedMemoryOffset;
			uint32_t* MAGIC = (uint32_t*)base;
			*MAGIC = 0x5452584E;
			isDocked_shared = (bool*)(base + 4);
			def_shared = (bool*)(base + 5);
			pluginActive_shared = (bool*)(base + 6);
			*isDocked_shared = false;
			*def_shared = true;
			*pluginActive_shared = false;
			SaltySDCore_ReplaceImport("_ZN2nn2oe25TryPopNotificationMessageEPj", (void*)TryPopNotificationMessage);
			SaltySDCore_ReplaceImport("_ZN2nn2oe22PopNotificationMessageEv", (void*)PopNotificationMessage);
			SaltySDCore_ReplaceImport("_ZN2nn2oe18GetPerformanceModeEv", (void*)GetPerformanceMode);
			SaltySDCore_ReplaceImport("_ZN2nn2oe16GetOperationModeEv", (void*)GetOperationMode);
			SaltySDCore_ReplaceImport("_ZN2nn2oe27GetDefaultDisplayResolutionEPiS1_", (void*)GetDefaultDisplayResolution);
			SaltySDCore_ReplaceImport("_ZN2nn2oe38GetDefaultDisplayResolutionChangeEventEPNS_2os11SystemEventE", (void*)GetDefaultDisplayResolutionChangeEvent);
			SaltySDCore_ReplaceImport("_ZN2nn2os18TryWaitSystemEventEPNS0_15SystemEventTypeE", (void*)TryWaitSystemEvent);

			SaltySDCore_printf("SaltySD ReverseNX-RT %s: injection finished correctly\n", ver);
		}
		else {
			SaltySDCore_printf("SaltySD ReverseNX-RT %s: error 0x%X. Couldn't map shmem\n", ver, rc);
		}
	}
	
	SaltySDCore_printf("SaltySD ReverseNX-RT %s: injection finished\n", ver);
}

enum {
#define Proxy(name) Func_##name,
#include "proxied.inc"
#undef  Proxy
	Func_Count
};

static FARPROC   funcs[Func_Count];
static HINSTANCE lib_handle;

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
	(void) inst;
	(void) reserved;

	if (reason == DLL_PROCESS_ATTACH) {
		wchar_t buffer[32768];
		GetSystemDirectoryW(buffer, 32768 - sizeof HIJACKED_DLL - 2);
		// NOTE(geni): No PathCchAppend since it's only available on Windows 8 and later, while Ripcord is still supported on 7
		PathAppendW(buffer, L"\\" HIJACKED_DLL);
		lib_handle = LoadLibraryExW(buffer, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (!lib_handle) {
			// NOTE(geni): Since we're hijacking UxTheme, MessageBox will probably silently fail, so..
			return 0;
		}

#define Proxy(name) funcs[Func_##name] = GetProcAddress(lib_handle, #name);
#include "proxied.inc"
#undef  Proxy

		return LoadHooks();
	} else if (reason == DLL_PROCESS_DETACH) {
		FreeLibrary(lib_handle);
	}

	return 1;
}

//~ ASM proxy

FARPROC ProcAddr;
extern int JMPToProc();

#define Proxy(name) void PROXY_##name() { ProcAddr = funcs[Func_##name]; JMPToProc(); }
#include "proxied.inc"
#undef  Proxy

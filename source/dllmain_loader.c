enum {
	Func_NetServerStatisticsGet,
	Func_NetpsNameCanonicalize,
	Func_NetpsNameCompare,
	Func_NetpsNameValidate,
	Func_NetpsPathCanonicalize,
	Func_NetpsPathCompare,
	Func_NetpsPathType,

	Func_Count
};

static FARPROC funcs[Func_Count];
static HINSTANCE lib_handle = 0;

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
	(void) inst;
	(void) reserved;

	if (reason == DLL_PROCESS_ATTACH) {
		LoadLibraryA("patch.dll");

		wchar_t buffer[32768];
		GetSystemDirectoryW(buffer, 32768 - 11);
		// NOTE(geni): No PathCchAppend since it's only available on Windows 8 and later, while Ripcord is still supported on 7
		PathAppendW(buffer, L"\\srvcli.dll");
		lib_handle = LoadLibraryExW(buffer, 0, LOAD_WITH_ALTERED_SEARCH_PATH);

		funcs[Func_NetServerStatisticsGet] = GetProcAddress(lib_handle, "NetServerStatisticsGet");
		funcs[Func_NetpsNameCanonicalize]  = GetProcAddress(lib_handle, "NetpsNameCanonicalize");
		funcs[Func_NetpsNameCompare]       = GetProcAddress(lib_handle, "NetpsNameCompare");
		funcs[Func_NetpsNameValidate]      = GetProcAddress(lib_handle, "NetpsNameValidate");
		funcs[Func_NetpsPathCanonicalize]  = GetProcAddress(lib_handle, "NetpsPathCanonicalize");
		funcs[Func_NetpsPathCompare]       = GetProcAddress(lib_handle, "NetpsPathCompare");
		funcs[Func_NetpsPathType]          = GetProcAddress(lib_handle, "NetpsPathType");
	} else if (reason == DLL_PROCESS_DETACH) {
		FreeLibrary(lib_handle);
	}

	return 1;
}

//~ ASM proxy

FARPROC ProcAddr = NULL;
extern int JMPToProc();

void PROXY_NetServerStatisticsGet() { ProcAddr = funcs[Func_NetServerStatisticsGet]; JMPToProc(); }
void PROXY_NetpsNameCanonicalize()  { ProcAddr = funcs[Func_NetpsNameCanonicalize]; JMPToProc(); }
void PROXY_NetpsNameCompare()       { ProcAddr = funcs[Func_NetpsNameCompare]; JMPToProc(); }
void PROXY_NetpsNameValidate()      { ProcAddr = funcs[Func_NetpsNameValidate]; JMPToProc(); }
void PROXY_NetpsPathCanonicalize()  { ProcAddr = funcs[Func_NetpsPathCanonicalize]; JMPToProc(); }
void PROXY_NetpsPathCompare()       { ProcAddr = funcs[Func_NetpsPathCompare]; JMPToProc(); }
void PROXY_NetpsPathType()          { ProcAddr = funcs[Func_NetpsPathType]; JMPToProc(); }
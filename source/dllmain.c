BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
	(void) inst;
	(void) reserved;

	if (reason == DLL_PROCESS_ATTACH) {
		LoadHooks();
	}

	return 1;
}

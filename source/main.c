BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
	(void) inst;
	(void) reserved;

	if (reason == DLL_PROCESS_ATTACH) {
		return LoadHooks();
	}

	return 1;
}

#include <flame/foundation/foundation.h>

#include <windows.h>
#include <wininet.h>
#include <process.h>
#include <commctrl.h>
#include <shellapi.h>
#include <DbgHelp.h>

using namespace flame;

void* load_exe_as_dll(const std::filesystem::path& path)
{
	DWORD dw;

	auto addr = LoadLibraryW(path.c_str());

	dw = 0;
	auto import_desc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(addr, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &dw);
	if (!import_desc)
		return nullptr;

	while (import_desc->Name)
	{
		auto name = (char*)((uchar*)addr + import_desc->Name);
		if (!name) 
			break;

		auto dll = LoadLibraryA(name);
		assert(dll);

		auto thunk = (PIMAGE_THUNK_DATA)((uchar*)addr + import_desc->FirstThunk);
		while (thunk->u1.Function)
		{
			FARPROC fn_new = 0;
			size_t rva = 0;

			if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
			{
				auto ord = IMAGE_ORDINAL64(thunk->u1.Ordinal);
				
				auto ppfn = (PROC*)&thunk->u1.Function;
				assert(ppfn);

				rva = (size_t)thunk;

				// purpose??
				char fe[100] = { 0 };
				sprintf_s(fe, 100, "#%u", ord);
				fn_new = GetProcAddress(dll, (char*)ord);
			}
			else
			{
				auto ppfn = (PROC*)&thunk->u1.Function;
				assert(ppfn);

				rva = (size_t)thunk;
				auto fname = (char*)dll + thunk->u1.Function + 2;
				if (!fname)
					break;

				fn_new = GetProcAddress(dll, fname);
				assert(fn_new);
			}

			auto hp = GetCurrentProcess();
			if (!WriteProcessMemory(hp, (void**)rva, &fn_new, sizeof(fn_new), nullptr) &&
				GetLastError() == ERROR_NOACCESS)
			{
				DWORD old_protect;
				if (VirtualProtect((void*)rva, sizeof(fn_new), PAGE_WRITECOPY, &old_protect))
				{
					if (!WriteProcessMemory(hp, (void**)rva, &fn_new, sizeof(fn_new), nullptr))
						assert(0);
					if (!VirtualProtect((void*)rva, sizeof(fn_new), old_protect, &old_protect))
						assert(0);
				}
			}

			thunk++;
		}

		import_desc++;
	}
}

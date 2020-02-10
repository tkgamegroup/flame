#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

#include <Windows.h>
#include <ImageHlp.h>

using namespace flame;

static PIMAGE_SECTION_HEADER get_enclosing_section_header(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader)
{
	auto section = IMAGE_FIRST_SECTION(pNTHeader);

	for (auto i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
	{
		auto size = section->Misc.VirtualSize;
		if (0 == size)
			size = section->SizeOfRawData;

		if (rva >= section->VirtualAddress && rva < section->VirtualAddress + size)
			return section;
	}

	return 0;
}

static LPVOID get_ptr_from_rva(DWORD rva, PIMAGE_NT_HEADERS64 pNTHeader, PBYTE imageBase)
{
	auto pSectionHdr = get_enclosing_section_header(rva, pNTHeader);
	if (!pSectionHdr)
		return 0;

	auto delta = (INT)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
	return (PVOID)(imageBase + rva - delta);
}

int main(int argc, char **args)
{
	if (argc < 2)
	{
		printf("argc is less than 2, exit\n");
		return 0;
	}

	std::filesystem::path filename(args[1]);
	std::filesystem::path typeinfo_filename = std::filesystem::path(filename).replace_extension(L".typeinfo");
	std::vector<std::wstring> dependencies;
	std::wstring pdb_filename;
	for (auto i = 2; i < argc; i++)
	{
		auto arg = args[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'd':
				dependencies.push_back(s2w(arg + 2));
				break;
			case 'p':
				pdb_filename = s2w(arg + 2);
				break;
			}
		}
	}

	{
		auto image = ImageLoad(filename.string().c_str(), filename.parent_path().string().c_str());

		if (image->FileHeader->OptionalHeader.NumberOfRvaAndSizes >= 2)
		{
			auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)get_ptr_from_rva(
				image->FileHeader->OptionalHeader.DataDirectory[1].VirtualAddress,
				image->FileHeader, image->MappedAddress);
			while (true)
			{
				if (importDesc->TimeDateStamp == 0 && importDesc->Name == 0)
					break;

				std::wstring d = s2w((char*)get_ptr_from_rva(importDesc->Name,
					image->FileHeader, image->MappedAddress));
				if (d != L"flame_type.dll" && SUW::starts_with(d, L"flame_"))
				{
					auto found = false;
					for (auto& _d : dependencies)
					{
						if (d == _d)
						{
							found = true;
							break;
						}
					}
					if (!found)
						dependencies.push_back(d);
				}

				importDesc++;
			}
		}
		ImageUnload(image);

	}

	if (!std::filesystem::exists(typeinfo_filename) || std::filesystem::last_write_time(typeinfo_filename) < std::filesystem::last_write_time(filename))
	{
		printf("generating typeinfo");

		auto last_curr_path = get_curr_path();
		set_curr_path(get_app_path().v);
		for (auto& d : dependencies)
			TypeinfoDatabase::load(std::filesystem::path(d).replace_extension(L".typeinfo").c_str(), true, false);
		set_curr_path(last_curr_path.v);

		TypeinfoDatabase::collect(filename.c_str(), !pdb_filename.empty() ? pdb_filename.c_str() : nullptr);

		printf(" - done\n");
	}
	else
		printf("typeinfo up to data\n");

	return 0;
}

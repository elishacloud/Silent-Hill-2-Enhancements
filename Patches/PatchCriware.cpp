#include <windows.h>
#include "Criware\criware.h"
#include "Logging\Logging.h"
#include "Common/Utils.h"
#include "External/Hooking.Patterns/Hooking.Patterns.h"
#include <External/injector/include/injector/injector.hpp>

void PatchCriware()
{
	Logging::Log() << "Enabling Criware...";

	// ADXFIC
	auto pattern = hook::pattern("E9 ? ? ? ? 90 90 90 90 90 90 90 90 90 90 90 8B 44 24 ? 85 C0 74");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXFIC_Create = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 85 C0 75 ? 83 C8 ? C3 89 44 24 ? E9 ? ? ? ? 90");
	if (pattern.size() != 2)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXFIC_GetNumFiles = pattern.count(2).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 85 C0 75 ? C3 89 44 24 ? E9 ? ? ? ? 90");
	if (pattern.size() != 2)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXFIC_GetFileName = pattern.count(2).get(1).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 85 C0 74 ? 89 44 24 ? E9 ? ? ? ? C3 90");
	if (pattern.size() != 2)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXFIC_Destroy = pattern.count(2).get(0).get<uint32_t>(0);

	// ADXWIN
	pattern = hook::pattern("E8 ? ? ? ? 6A ? 68 ? ? ? ? E8 ? ? ? ? 6A ? 68 ? ? ? ? 68");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXWIN_SetupDvdFs = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E8 ? ? ? ? 85 C0 7e ? E8 ? ? ? ? E8 ? ? ? ? E9 ? ? ? ? 90");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXWIN_ShutdownDvdFs = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 50 6A ? E8 ? ? ? ? 83 C4 ? C3");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXWIN_SetupSound = pattern.count(1).get(0).get<uint32_t>(0);

	// ADXM
	pattern = hook::pattern("A1 ? ? ? ? 81 EC ? ? ? ? 53 33 DB 56 3B C3 57 0F 85 ? ? ? ? 89 1D");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXM_SetupThrd = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E9 ? ? ? ? 90 90 90 90 90 90 90 90 90 90 90 A1 ? ? ? ? 6A");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXM_ExecMain = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("51 A1 ? ? ? ? 48 A3 ? ? ? ? 0F 85");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXM_ShutdownThrd = pattern.count(1).get(0).get<uint32_t>(0);

	// ADXF
	pattern = hook::pattern("53 55 8B 6C 24 ? 56 8B 74 24 ? 57 56 55 E8 ? ? ? ? 83 C4 ? 85 C0 0F 8C");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXF_LoadPartitionNw = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("A1 ? ? ? ? 83 Ec ? 53 56 8B 74 24 ? 57 3B f0 74 ? 68 ? ? ? ? E8");
	if (pattern.size() != 2)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXF_GetPtStat = pattern.count(2).get(0).get<uint32_t>(0);

	// ADXT
	pattern = hook::pattern("83 EC ? 53 56 8B 74 24 ? 57 85 F6 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 5F 5E 5B 83 C4");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_StartAfs = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 85 F6 57 74 ? 8B 7C 24 ? 85 FF 74 ? 56 E8 ? ? ? ? E8 ? ? ? ? 8B 46");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_StartFname = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 8B 4c 24 ? 53 8B 5c 24 ? 55");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_Create = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E9 ? ? ? ? 90 90 90 56 8B 74 24 ? 85 F6 75 ? 5E C7 44 24 ? ? ? ? ? E9");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_Stop = pattern.count(1).get(0).get<uint32_t>(8);

	pattern = hook::pattern("8B 44 24 ? 85 C0 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 83 C8 ? C3 0f be 40 ? C3 90 90 8b 44 24 ? 83 F8");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_GetStat = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 85 C0 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? C3 8b 4c 24 ? 66 89 48");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_SetOutVol = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("A1 ? ? ? ? A1 ? ? ? ? 85 C0 0f 85 ? ? ? ? 57 E8 ? ? ? ? E8");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_Init = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("A1 ? ? ? ? 48 A3 ? ? ? ? 75 ? E8 ? ? ? ? E8");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_ADXT_Finish = pattern.count(1).get(0).get<uint32_t>(0);

	// AIXP
	pattern = hook::pattern("A1 ? ? ? ? A1 ? ? ? ? 85 C0 75 ? 57 B9 ? ? ? ? 33 C0 BF ? ? ? ? F3");
	if (pattern.size() != 2)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_Init = pattern.count(2).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 8B 4c 24 ? 53 55 8B 54 24 ? 8D 68");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_Create = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 85 F6 0f 84 ? ? ? ? 55");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_Destroy = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("A1 ? ? ? ? 56 8B 74 24 ? 56 E8");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_StartFname = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 57 33 FF 8A 46");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_Stop = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 0F BE 40 ? C3 90 90 90 90 90 90 90 8B 44 24 ? 8B 4C 24");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_GetStat = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 44 24 ? 8B 4c 24 ? 8B 44 81 ? C3");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_GetAdxt = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 4c 24 ? 8A 44 24 ? 88 81 ? ? ? ? C3 90 8b 4c 24 ? 8A 44 24");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uint32_t* ptr_AIXP_SetLpSw = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? 52 E8 ? ? ? ? 83 C4 ? 83 F8");
	if (pattern.size() != 1)
	{
		Logging::Log() << __FUNCTION__ " Error: failed to find memory address!";
		return;
	}
	uintptr_t ptr_AIXP_ExecServer = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int();

	// Write new JMPs
	WriteJMPtoMemory((BYTE*)ptr_ADXFIC_Create, ADXFIC_Create);
	WriteJMPtoMemory((BYTE*)ptr_ADXFIC_GetNumFiles, ADXFIC_GetNumFiles);
	WriteJMPtoMemory((BYTE*)ptr_ADXFIC_GetFileName, ADXFIC_GetFileName);
	WriteJMPtoMemory((BYTE*)ptr_ADXFIC_Destroy, ADXFIC_Destroy);

	WriteJMPtoMemory((BYTE*)ptr_ADXWIN_SetupDvdFs, ADXWIN_SetupDvdFs);
	WriteJMPtoMemory((BYTE*)ptr_ADXWIN_ShutdownDvdFs, ADXWIN_ShutdownDvdFs);
	WriteJMPtoMemory((BYTE*)ptr_ADXWIN_SetupSound, ADXWIN_SetupSound);

	WriteJMPtoMemory((BYTE*)ptr_ADXM_SetupThrd, ADXM_SetupThrd);
	WriteJMPtoMemory((BYTE*)ptr_ADXM_ExecMain, ADXM_ExecMain);
	WriteJMPtoMemory((BYTE*)ptr_ADXM_ShutdownThrd, ADXM_ShutdownThrd);

	WriteJMPtoMemory((BYTE*)ptr_ADXF_LoadPartitionNw, ADXF_LoadPartitionNw);
	WriteJMPtoMemory((BYTE*)ptr_ADXF_GetPtStat, ADXF_GetPtStat);

	WriteJMPtoMemory((BYTE*)ptr_ADXT_StartAfs, ADXT_StartAfs);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_StartFname, ADXT_StartFname);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_Create, ADXT_Create);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_Stop, ADXT_Stop);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_GetStat, ADXT_GetStat);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_SetOutVol, ADXT_SetOutVol);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_Init, ADXT_Init);
	WriteJMPtoMemory((BYTE*)ptr_ADXT_Finish, ADXT_Finish);

	WriteJMPtoMemory((BYTE*)ptr_AIXP_Init, AIXP_Init);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_Create, AIXP_Create);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_Destroy, AIXP_Destroy);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_StartFname, AIXP_StartFname);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_Stop, AIXP_Stop);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_GetStat, AIXP_GetStat);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_GetAdxt, AIXP_GetAdxt);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_SetLpSw, AIXP_SetLpSw);
	WriteJMPtoMemory((BYTE*)ptr_AIXP_ExecServer, AIXP_ExecServer);
}

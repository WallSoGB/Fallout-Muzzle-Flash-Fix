#include "nvse/PluginAPI.h"

NVSEInterface* g_nvseInterface{};

void __forceinline SafeWrite32(UInt32 addr, UInt32 data)
{
	UInt32	oldProtect;

	VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect);
	*((UInt32*)addr) = data;
	VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
}

void __forceinline ReplaceCall(UInt32 jumpSrc, UInt32 jumpTgt)
{
	SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

template <typename T_Ret = UInt32, typename ...Args>
__forceinline T_Ret ThisStdCall(UInt32 _addr, const void* _this, Args ...args)
{
	return ((T_Ret(__thiscall*)(const void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "MuzzleFlashFix";
	info->version = 1;

	return true;
}

struct MuzzleFlash {
	bool bEnabled;
	DWORD useless[3];
	DWORD* light;
};

void __fastcall MuzzleLightFix(MuzzleFlash* apThis) {
	if (apThis->light) {
		if (!apThis->bEnabled) [[likely]] {
			apThis->light[12] |= 1;
			}
		else {
			apThis->light[12] &= ~1;
		}
	}
	ThisStdCall(0x9BB8A0, apThis);
}

bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		ReplaceCall(0x9BB158, (UInt32)MuzzleLightFix);
	}

	return true;
}
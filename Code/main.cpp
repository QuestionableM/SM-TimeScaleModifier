#include "GraphicsOptionsMenu.hpp"
#include "Utils/Console.hpp"

#include <MinHook.h>

static bool ms_mhInitialized = false;
static bool ms_mhHooksAttached = false;

#define DEFINE_HOOK(address, detour, original) \
	MH_CreateHook((LPVOID)(v_mod_base + address), (LPVOID)detour, (LPVOID*)&original)

#define EASY_HOOK(address, func_name) \
	MH_CreateHook((LPVOID)(v_mod_base + address), (LPVOID)h_ ## func_name, (LPVOID*)&o_ ## func_name)

#define EASY_CLASS_HOOK(address, class_name, func_name) \
	MH_CreateHook((LPVOID)(v_mod_base + address), (LPVOID)class_name::h_##func_name, (LPVOID*)&class_name::o_##func_name)

static void (*o_ContraptionUpdate)(void*, float) = nullptr;
static void (*o_ContraptionRender)(void*, float) = nullptr;
static void (*o_GameInstanceUpdate)(void*, float) = nullptr;
static void (*o_PlayStateUpdate)(void*, float) = nullptr;

static void h_ContraptionUpdate(void* self, float delta_time)
{
	o_ContraptionUpdate(self, delta_time * GraphicsOptionsMenu::TimeScale);
}

static void h_ContraptionRender(void* self, float delta_time)
{
	o_ContraptionRender(self, delta_time * GraphicsOptionsMenu::TimeScale);
}

static void h_GameInstanceUpdate(void* self, float delta_time)
{
	o_GameInstanceUpdate(self, delta_time * GraphicsOptionsMenu::TimeScale);
}

static void h_PlayStateUpdate(void* self, float delta_time)
{
	o_PlayStateUpdate(self, delta_time * GraphicsOptionsMenu::TimeScale);
}

static void process_attach(HMODULE hMod)
{
	AttachDebugConsole();

	if (MH_Initialize() != MH_OK)
	{
		DebugErrorL("Couldn't initialize minHook");
		return;
	}

	ms_mhInitialized = true;

	const std::uintptr_t v_mod_base = std::uintptr_t(GetModuleHandle(NULL));
	if (DEFINE_HOOK(0x2E4C40, h_ContraptionUpdate, o_ContraptionUpdate) != MH_OK) return;
	if (DEFINE_HOOK(0x2E30B0, h_ContraptionRender, o_ContraptionRender) != MH_OK) return;
	if (DEFINE_HOOK(0x34EBA0, h_GameInstanceUpdate, o_GameInstanceUpdate) != MH_OK) return;
	if (DEFINE_HOOK(0x43E770, h_PlayStateUpdate, o_PlayStateUpdate) != MH_OK) return;

	// Gui hook
	if (DEFINE_HOOK(0x30EB70, GraphicsOptionsMenu::h_CreateWidgets, GraphicsOptionsMenu::o_CreateWidgets)) return;

	ms_mhHooksAttached = MH_EnableHook(MH_ALL_HOOKS) == MH_OK;
}

static void process_detach(HMODULE hmod)
{
	if (ms_mhInitialized)
	{
		if (ms_mhHooksAttached)
			MH_DisableHook(MH_ALL_HOOKS);

		MH_Uninitialize();
	}

	FreeLibrary(hmod);
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		process_attach(hModule);
		break;
	case DLL_PROCESS_DETACH:
		process_detach(hModule);
		break;
	}

	return TRUE;
}
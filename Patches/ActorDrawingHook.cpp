#include "ActorDrawingHook.h"
#include "Patches.h"
#include "Common\Utils.h"

static std::vector<ActorDrawTopCallbackFn> sPrologueFunctions;
static std::vector<ActorDrawTopCallbackFn> sEpilogueFunctions;

static void(__cdecl* ActorDrawTop)(ModelOffsetTable*, void*) = nullptr;

static bool sActorDrawTopFnPatched = false;

static void __cdecl PatchedActorDrawTop(ModelOffsetTable* pOffsetTable, void* arg2) {
    bool shouldSkipActualDraw = false;
    for (auto& fn : sPrologueFunctions) {
        if (fn(pOffsetTable, arg2)) {
            shouldSkipActualDraw = true;
        }
    }

    if (!shouldSkipActualDraw) {
        ActorDrawTop(pOffsetTable, arg2);
    }

    for (auto& fn : sEpilogueFunctions) {
        fn(pOffsetTable, arg2);
    }
}

static void PatchActorDrawTop() {
    if (sActorDrawTopFnPatched) {
        return;
    }

    switch (GameVersion) {
        case SH2V_10:
            ActorDrawTop = reinterpret_cast<decltype(ActorDrawTop)>(0x501F90);
            WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50EB2B), PatchedActorDrawTop, 5);
        break;

        case SH2V_11:
            ActorDrawTop = reinterpret_cast<decltype(ActorDrawTop)>(0x5022C0);
            WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50EE5B), PatchedActorDrawTop, 5);
        break;

        case SH2V_DC:
            ActorDrawTop = reinterpret_cast<decltype(ActorDrawTop)>(0x501BE0);
            WriteCalltoMemory(reinterpret_cast<BYTE*>(0x50E77B), PatchedActorDrawTop, 5);
        break;
    }

    sActorDrawTopFnPatched = true;
}

void RegisterActorDrawTopPrologue(const ActorDrawTopCallbackFn& prologueFn) {
    PatchActorDrawTop();

    sPrologueFunctions.push_back(prologueFn);
}

void RegisterActorDrawTopEpilogue(const ActorDrawTopCallbackFn& epilogueFn) {
    PatchActorDrawTop();

    sEpilogueFunctions.push_back(epilogueFn);
}

#include "GravestoneBoardsFix.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"

static DWORD GravestoneBoardsRetAddr = 0;
static UIElement* uiElement = nullptr;

__declspec(naked) void __stdcall GravestoneBoardsASM()
{
    __asm {
        mov uiElement, esi
    }

    // Check we're in the grave room and examining the gravestone
    if (GetRoomID() == R_MAN_GRAVE_ROOM && IsInFullScreenImageEvent())
    {
        // Step 1: Adjust the texture coordinates for each board to more closely match eachother

        // Black board
        if (uiElement->texCoords.u1 == 0.0f && uiElement->texCoords.v1 == 0.0f &&
            uiElement->texCoords.u2 != 0.0f && uiElement->texCoords.v2 != 0.0f)
        {
            uiElement->texCoords.u1 = 3.0f;
            uiElement->texCoords.v1 = 1.0f;
            uiElement->texCoords.u2 = 2548.0f;
            uiElement->texCoords.v2 = 3320.0f;
        }
        // White board
        else if (uiElement->texCoords.u1 > 2000.0f && uiElement->texCoords.v1 == 0.0f)
        {
            uiElement->texCoords.u1 = 2581.0f;
            uiElement->texCoords.v1 = -5.0f;
            uiElement->texCoords.u2 = 5126.0f;
            uiElement->texCoords.v2 = 3320.0f;
        }
        // Red board
        else if (uiElement->texCoords.u1 == 0.0f && uiElement->texCoords.v1 > 3000.0f)
        {
            uiElement->texCoords.u1 = 3.0f;
            uiElement->texCoords.v1 = 3316.0f;
            uiElement->texCoords.u2 = 2542.0f;
            uiElement->texCoords.v2 = 6636.0f;
        }

        // Step 2: Adjust the vertices of the boards for each orientation to properly fill the tombstone cavity

        // Rotated right 90 deg
        if (uiElement->verts[0].x > 0 && uiElement->verts[0].y < 0 &&
            uiElement->verts[1].x > 0 && uiElement->verts[1].y > 0 &&
            uiElement->verts[2].x < 0 && uiElement->verts[2].y < 0 &&
            uiElement->verts[3].x < 0 && uiElement->verts[3].y > 0)
        {
            uiElement->verts[0].x = 1355;
            uiElement->verts[0].y = -2030;
            uiElement->verts[1].x = 1355;
            uiElement->verts[1].y = 1158;
            uiElement->verts[2].x = -1313;
            uiElement->verts[2].y = -2030;
            uiElement->verts[3].x = -1313;
            uiElement->verts[3].y = 1158;
        }
        // Rotated 180 deg
        else if (uiElement->verts[0].x > 0 && uiElement->verts[0].y > 0 &&
                 uiElement->verts[1].x < 0 && uiElement->verts[1].y > 0 &&
                 uiElement->verts[2].x > 0 && uiElement->verts[2].y < 0 &&
                 uiElement->verts[3].x < 0 && uiElement->verts[3].y < 0)
        {
            uiElement->verts[0].x = 1287;
            uiElement->verts[0].y = 1262;
            uiElement->verts[1].x = -1267;
            uiElement->verts[1].y = 1262;
            uiElement->verts[2].x = 1287;
            uiElement->verts[2].y = -2066;
            uiElement->verts[3].x = -1267;
            uiElement->verts[3].y = -2066;
        }
        // Rotated left 90 deg
        else if (uiElement->verts[0].x < 0 && uiElement->verts[0].y > 0 &&
                 uiElement->verts[1].x < 0 && uiElement->verts[1].y < 0 &&
                 uiElement->verts[2].x > 0 && uiElement->verts[2].y > 0 &&
                 uiElement->verts[3].x > 0 && uiElement->verts[3].y < 0)
        {
            uiElement->verts[0].x = -1353;
            uiElement->verts[0].y = 1176;
            uiElement->verts[1].x = -1353;
            uiElement->verts[1].y = -2005;
            uiElement->verts[2].x = 1315;
            uiElement->verts[2].y = 1176;
            uiElement->verts[3].x = 1315;
            uiElement->verts[3].y = -2005;
        }
    }

    __asm {
        // original instructions
        mov cx, word ptr ds : [esi + 0x2E]
        fild dword ptr ds : [0xA33480]

        // return to real function
        jmp GravestoneBoardsRetAddr
    }
}

void PatchGravestoneBoardsFix()
{
    constexpr BYTE SearchBytes[]{ 0x90, 0x90, 0x66, 0x8B, 0x4E, 0x2E, 0xDB, 0x05 };
    DWORD GravestoneBoardsAddr = SearchAndGetAddresses(0x0049EFDE, 0x0049F28E, 0x0049EB4E, SearchBytes, sizeof(SearchBytes), 0x02, __FUNCTION__);
    if (!GravestoneBoardsAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    GravestoneBoardsRetAddr = GravestoneBoardsAddr + 0xA;

    Logging::Log() << "Patching Gravestone Boards Fix...";
    WriteJMPtoMemory((BYTE*)GravestoneBoardsAddr, *GravestoneBoardsASM);
}

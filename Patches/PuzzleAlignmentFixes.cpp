#include "PuzzleAlignmentFixes.h"
#include "Common\Utils.h"
#include "Logging\Logging.h"
#include "Patches\Patches.h"

using DrawUserInterfaceFunc = void (*)(/* UIElement* uiElement */);
static DrawUserInterfaceFunc DrawUserInterface = nullptr;

void GravestoneBoardsFix(UIElement* uiElement)
{
    // Check we're in the grave room and examining the gravestone or reading memo 54
    if ((GetRoomID() == R_MAN_GRAVE_ROOM && IsInFullScreenImageEvent()) ||
        (GetReadingMemoFlag() && GetEventIndex() == EVENT_MEMO_LIST && GetMemoUniqueId() == 54))
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
        // Anything else
        else
        {
            return;
        }

        // Step 2: Adjust the vertices of the boards for each orientation to properly fill the gravestone cavity

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
}

void CoinPuzzleFix(UIElement* uiElement)
{
    // Check we're in Apts room 105 and examining the coin puzzle
    if (GetRoomID() == R_APT_W_RM_105 && IsInFullScreenImageEvent())
    {
        // Step 1: Adjust the texture coordinates for each coin to more closely match eachother

        // Old man coin
        if (uiElement->texCoords.u1 == 0.0f && uiElement->texCoords.v1 == 0.0f &&
            uiElement->texCoords.u2 == 752.0f && uiElement->texCoords.v2 == 1008.0f)
        {
            uiElement->texCoords.u1 = 0.0f;
            uiElement->texCoords.v1 = 0.0f;
            uiElement->texCoords.u2 = 1024.0f;
            uiElement->texCoords.v2 = 1024.0f;
        }
        // Snake coin
        else if (uiElement->texCoords.u1 == 0.0f && uiElement->texCoords.v1 == 1024.0f &&
            uiElement->texCoords.u2 == 752.0f && uiElement->texCoords.v2 == 2032.0f)
        {
            uiElement->texCoords.u1 = 0.0f;
            uiElement->texCoords.v1 = 1055.0f;
            uiElement->texCoords.u2 = 1024.0f;
            uiElement->texCoords.v2 = 2075.0f;
        }
        // Prisoner coin
        else if (uiElement->texCoords.u1 == 0.0f && uiElement->texCoords.v1 == 2048.0f &&
            uiElement->texCoords.u2 == 752.0f && uiElement->texCoords.v2 == 3056.0f)
        {
            uiElement->texCoords.u1 = -4.0f;
            uiElement->texCoords.v1 = 2082.0f;
            uiElement->texCoords.u2 = 1028.0f;
            uiElement->texCoords.v2 = 3108.0f;
        }
        // Anything else
        else
        {
            return;
        }

        // Step 2: Adjust the vertices of the coins for each position to properly fill the cabinet slot

        // Position 1
        if (uiElement->verts[0].x > -3200 && uiElement->verts[0].x < -3100)
        {
            uiElement->verts[0].x = -3160;
            uiElement->verts[0].y = -708;
            uiElement->verts[1].x = -2090;
            uiElement->verts[1].y = 362;
        }
        // Position 2
        else if (uiElement->verts[0].x > -1800 && uiElement->verts[0].x < -1700)
        {
            uiElement->verts[0].x = -1790;
            uiElement->verts[0].y = -682;
            uiElement->verts[1].x = -720;
            uiElement->verts[1].y = 388;
        }
        // Position 3
        else if (uiElement->verts[0].x > -500 && uiElement->verts[0].x < -400)
        {
            uiElement->verts[0].x = -414;
            uiElement->verts[0].y = -650;
            uiElement->verts[1].x = 656;
            uiElement->verts[1].y = 420;
        }
        // Position 4
        else if (uiElement->verts[0].x > 900 && uiElement->verts[0].x < 1000)
        {
            uiElement->verts[0].x = 962;
            uiElement->verts[0].y = -612;
            uiElement->verts[1].x = 2032;
            uiElement->verts[1].y = 458;
        }
        // Position 5
        else if (uiElement->verts[0].x > 2300 && uiElement->verts[0].x < 2400)
        {
            uiElement->verts[0].x = 2348;
            uiElement->verts[0].y = -580;
            uiElement->verts[1].x = 3418;
            uiElement->verts[1].y = 490;
        }
    }
}

void PuzzleAlignmentFixesHook(/* UIElement* uiElement */)
{
    static UIElement* uiElement = nullptr;

    __asm {
        mov uiElement, esi
    }

    GravestoneBoardsFix(uiElement);
    CoinPuzzleFix(uiElement);

    __asm {
        mov esi, uiElement
    }

    DrawUserInterface(/* uiElement */);
}

void PatchPuzzleAlignmentFixes()
{
    switch (GameVersion)
    {
    case SH2V_10:
        DrawUserInterface = reinterpret_cast<DrawUserInterfaceFunc>(0x49EFE0);
        break;
    case SH2V_11:
        DrawUserInterface = reinterpret_cast<DrawUserInterfaceFunc>(0x49F290);
        break;
    case SH2V_DC:
        DrawUserInterface = reinterpret_cast<DrawUserInterfaceFunc>(0x49EB50);
        break;
    case SH2V_UNKNOWN:
        Logging::Log() << __FUNCTION__ << " Error: unknown game version!";
        return;
    }

    constexpr BYTE SearchBytes[]{ 0xE8, 0x64, 0xF6, 0xFF, 0xFF, 0x8B, 0x06, 0x85 };
    DWORD DrawUserInterfaceAddr = SearchAndGetAddresses(0x49F977, 0x49FC27, 0x49F4E7, SearchBytes, sizeof(SearchBytes), 0, __FUNCTION__);
    if (!DrawUserInterfaceAddr)
    {
        Logging::Log() << __FUNCTION__ << " Error: failed to find memory address!";
        return;
    }

    Logging::Log() << "Patching Gravestone Boards Fix...";
    WriteCalltoMemory((BYTE*)DrawUserInterfaceAddr, PuzzleAlignmentFixesHook);
}

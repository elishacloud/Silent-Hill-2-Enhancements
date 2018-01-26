#pragma once

void UpdateTexAddr();

struct TexAddrStruct
{
	char *name;
	BYTE vDC[5];
	BYTE v10[5];
	BYTE v11[5];
};

constexpr TexAddrStruct TexAddrList[] = {
	"\\data\\pic\\etc\\start00.tex",  { 0xB8, 0x40, 0xEC, 0xDB, 0x01 }, { 0xB8, 0x40, 0xC0, 0xDB, 0x01 }, { 0xB8, 0x40, 0xFC, 0xDB, 0x01 }
};

// Search line in DC
// 0044A1B6   . 68 88706300    PUSH sh2pcDC.00637088                    ; /mode = "rb"

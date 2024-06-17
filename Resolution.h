#pragma once

#include <vector>

struct RESOLUTONTEXT
{
	char resStrBuf[27];
};

const std::vector<RESOLUTONTEXT>& GetResolutionText();

void WSFDynamicChangeWithResIndex(BYTE NewIndex);

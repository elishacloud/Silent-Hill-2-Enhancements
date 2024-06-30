#pragma once

#include <vector>

struct RESOLUTONTEXT
{
	char resStrBuf[27];
};

const std::vector<RESOLUTONTEXT>& GetResolutionText();

// Updates WSF and updates the text resolution index.
void WSFDynamicChangeWithIndex(BYTE NewIndex);

// Returns whether the resolution option is locked.
bool IsResolutionLocked();

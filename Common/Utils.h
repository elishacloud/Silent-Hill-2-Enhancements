#pragma once

void *GetAddressOfData(const void *data, size_t len);
void *GetAddressOfData(const void *data, size_t len, DWORD step, DWORD start = 0);

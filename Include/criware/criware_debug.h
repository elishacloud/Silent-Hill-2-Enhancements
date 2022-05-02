/*
*/
#pragma once

void ADXD_Error(const char* caption, const char* fmt, ...);
void ADXD_Warning(const char* caption, const char* fmt, ...);
void ADXD_Log(const char* fmt, ...);

void ADXD_SetLevel(int level);

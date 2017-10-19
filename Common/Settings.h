#pragma once

typedef void(__stdcall* NV)(char* name, char* value);

bool SetValue(char* name);
bool IsValidSettings(char* name, char* value);
char* Read(char* szFileName);
void Parse(char* str, NV NameValueCallback);

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#include <string>
#include "CConfig.h"

#undef CreateWindow

class CWnd
{
public:
	CWnd();

	operator HWND();

	void CreateWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance);
	void Destroy();

	void SetWnd(HWND wnd);
	void SetText(LPCWSTR lpString);
	void Enable(bool enable = true);
	WNDPROC Subclass(WNDPROC new_proc);
	LRESULT CallProcedure(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetID(UINT uId);

	enum Type
	{
		TYPE_WND,
		TYPE_CHECK,
		TYPE_LIST,
		TYPE_TEXT,
	};
	Type tType;

private:
	HWND hWnd, hWndParent;
	HINSTANCE hInst;
	WNDPROC old_proc;
};

class CCtrlGroup : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
};

class CCtrlButton : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
};

class CCtrlTab : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	void InsertItem(int index, LPCWSTR lpString);
	void InsertItem(int index, LPCSTR lpString);
	int GetCurSel();
	void GetRect(RECT& rect);
};

class CCtrlStatic : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, UINT Align = 1);
	bool SetText(LPCWSTR lpText);
	void OnPaint(HDC hdc);

	std::wstring szText;
	HFONT hFont;
	UINT uAlign;

	static LRESULT CALLBACK proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
};

class CCtrlCheckBox : public CWnd
{
public:
	void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	bool GetCheck();
	void SetCheck(bool check);
};

class CCtrlDropBox : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);

	void Reset();
	void AddString(LPCWSTR lpString);

	int GetSelection();
	void SetSelection(int sel);
};

class CCtrlTextBox : public CWnd
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	void SetString(std::wstring lpString);
	std::wstring GetString();
};

class CCtrlDescription
{
public:
	void CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hBold);

	void SetCaption(LPCWSTR lpCaption);
	void SetText(LPCWSTR lpText);

	CCtrlStatic hCaption, hText;
	CWnd hBack, hStrike;
};

// combined controls
class CCombined
{
public:
	CCombined() : cCtrl(nullptr),
		cValue(nullptr),
		uType(TYPE_DEFAULT)
	{}

	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
	{
		UNREFERENCED_PARAMETER(lpName);
		UNREFERENCED_PARAMETER(X);
		UNREFERENCED_PARAMETER(Y);
		UNREFERENCED_PARAMETER(Width);
		UNREFERENCED_PARAMETER(Height);
		UNREFERENCED_PARAMETER(hParent);
		UNREFERENCED_PARAMETER(hInstance);
		UNREFERENCED_PARAMETER(hFont);
	}
	virtual void Release() {}
	virtual void SetHover(std::wstring Text, std::wstring Desc, CCtrlDescription* ctrl) { szText = Text; szDesc = Desc; cCtrl = ctrl; }
	// lists
	virtual void Reset() {}
	virtual void AddString(LPCWSTR lpString)
	{
		UNREFERENCED_PARAMETER(lpString);
	}
	virtual int GetSelection() { return 0; }
	virtual void SetSelection(int sel)
	{
		UNREFERENCED_PARAMETER(sel);
	}
	// checkboxes
	virtual bool GetCheck() { return false; }
	virtual void SetCheck(bool check)
	{
		UNREFERENCED_PARAMETER(check);
	}
	// textboxes
	virtual std::wstring GetString() { return std::wstring(); };
	virtual void SetString(std::wstring lpString)
	{
		UNREFERENCED_PARAMETER(lpString);
	}

	// random configuration
	void SetConfigPtr(CConfigOption* c) { cValue = c; }
	int GetConfigValue()
	{
		return cValue->cur_val;
	}
	void SetConfigValue(int val)
	{
		if (cValue)
		{
			cValue->cur_val = val;
		}
	}
	void SetConfigValue(std::wstring lpString)
	{
		if (cValue)
		{
			cValue->value[cValue->cur_val].val = WideToMulti_s(lpString);
		}
	}

	enum TYPE
	{
		TYPE_DEFAULT,
		TYPE_CHECK,
		TYPE_LIST,
		TYPE_TEXT
	};

	UINT uType;
	std::wstring szText, szDesc;
	CCtrlDescription* cCtrl;
	CConfigOption* cValue;
};

class CFieldCheck : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	virtual void Release();

	virtual bool GetCheck();
	virtual void SetCheck(bool check);

	static LRESULT CALLBACK proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	CCtrlCheckBox box;
	WNDPROC old;
};

class CFieldList : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	virtual void Release();

	virtual void Reset();
	virtual void AddString(LPCWSTR lpString);

	virtual int GetSelection();
	virtual void SetSelection(int sel);

	static LRESULT CALLBACK proc_list(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK proc_static(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	void Enable(bool enable = true) { this->hList.Enable(enable); this->hStatic.Enable(enable); }

private:
	CCtrlStatic hStatic;
	CCtrlDropBox hList;
};

class CFieldText : public CCombined
{
public:
	virtual void CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont);
	virtual void Release();

	virtual void SetString(std::wstring lpString);
	virtual std::wstring GetString();

	static LRESULT CALLBACK proc_text(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK proc_static(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	
	void Enable(bool enable = true) { this->hList.Enable(enable); this->hStatic.Enable(enable); }

private:
	CCtrlStatic hStatic;
	CCtrlTextBox hList;
};

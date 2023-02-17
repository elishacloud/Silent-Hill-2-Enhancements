/**
* Copyright (C) 2022 Gemini
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*
* Code taken from: https://github.com/Gemini-Loboto3/SH2config
*
* Updated by Elisha Riedlinger 2023
*/

#include "CWnd.h"

///////////////////////////////////////////////////////
CWnd::CWnd() :
	hWnd(0),
	hWndParent(0),
	hInst(0),
	tType(TYPE_WND),
	old_proc(nullptr)
{}

CWnd::operator HWND() { return hWnd; }

void CWnd::CreateWindow(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance)
{
	hInst = hInstance;
	hWndParent = hParent;
	hWnd = ::CreateWindowExW(0, lpClassName, lpWindowName, dwStyle, X, Y, Width, Height, hParent, nullptr, hInstance, (LPVOID)this);
}

void CWnd::Destroy()
{
	DestroyWindow(hWnd);
	hWnd = 0;
}

void CWnd::SetWnd(HWND wnd) { hWnd = wnd; }

void CWnd::SetText(LPCWSTR lpString) { SetWindowTextW(hWnd, lpString); }

void CWnd::Enable(bool enable) { EnableWindow(hWnd, enable); }

WNDPROC CWnd::Subclass(WNDPROC new_proc)
{
	WNDPROC ret = (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)new_proc);
	// preserve only the original procedure
	if (old_proc == nullptr)
	{
		old_proc = ret;
	}

	return ret;
}

LRESULT CWnd::CallProcedure(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProcW(old_proc, wnd, msg, wParam, lParam);
}

void CWnd::SetID(UINT uId)
{
	SetWindowLongPtrW(hWnd, GWLP_ID, uId);
}

///////////////////////////////////////////////////////
void CCtrlGroup::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_GROUPBOX | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
}

///////////////////////////////////////////////////////
void CCtrlButton::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_BUTTONW, lpName, WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
}

///////////////////////////////////////////////////////
void CCtrlTab::CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_TABCONTROLW, L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
}

void CCtrlTab::InsertItem(int index, LPCWSTR lpString)
{
	TCITEMW item;
	item.mask = TCIF_TEXT;
	item.pszText = (LPWSTR)lpString;
	SendMessageW(*this, TCM_INSERTITEMW, (WPARAM)index, (LPARAM)&item);
}

void CCtrlTab::InsertItem(int index, LPCSTR lpString)
{
	TCITEMA item;
	item.mask = TCIF_TEXT;
	item.pszText = (LPSTR)lpString;
	SendMessageW(*this, TCM_INSERTITEMA, (WPARAM)index, (LPARAM)&item);
}

int CCtrlTab::GetCurSel()
{
	return (int)SendMessageW(*this, TCM_GETCURSEL, 0, 0);
}

void CCtrlTab::GetRect(RECT& rect)
{
	GetClientRect(*this, &rect);
	SendMessageW(*this, TCM_ADJUSTRECT, (WPARAM)false, (LPARAM)&rect);
}

///////////////////////////////////////////////////////
void CCtrlStatic::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFontt, UINT Align)
{
	szText = lpName;
	this->hFont = hFontt;
	uAlign = Align;
	CWnd::CreateWindow(WC_STATICW, lpName, SS_LEFT | WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, X, Y, Width, Height, hParent, hInstance);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);
	Subclass(proc);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFontt, TRUE);
}

bool CCtrlStatic::SetText(LPCWSTR lpText)
{
	if (szText.compare(lpText) == 0)
		return false;
	szText = lpText;
	return true;
}

void CCtrlStatic::OnPaint(HDC hdc)
{
	RECT rc;
	GetClientRect(*this, &rc);

	FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, hFont);
	SIZE size;
	GetTextExtentPoint32W(hdc, szText.c_str(), (int)szText.size(), &size);
#if 0
	int Y = 0;
	switch (c->uAlign)
	{
	case 0: Y = rc.top; break;
	case 1: Y = (rc.bottom - size.cy) / 2; break;
	case 2: Y = rc.bottom - size.cy; break;
	}

	ExtTextOutW(hdc, 0, Y, ETO_CLIPPED, &rc, c->szText.c_str(), c->szText.size(), nullptr);
#else
	int Y;
	switch (uAlign)
	{
	case 1: Y = (rc.bottom - size.cy) / 2; rc.top += Y; rc.bottom -= Y; break;
	case 2: Y = rc.bottom - size.cy; rc.top += Y; rc.bottom -= Y; break;
	}
	DrawTextExW(hdc, (LPWSTR)szText.c_str(), (int)szText.size(), &rc, DT_LEFT | DT_WORDBREAK, nullptr);
#endif
}

LRESULT CCtrlStatic::proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CCtrlStatic* c = reinterpret_cast<CCtrlStatic*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (Msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		c->OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 0;
	default:
		return c->CallProcedure(hWnd, Msg, wParam, lParam);
	}
}

///////////////////////////////////////////////////////
void CCtrlCheckBox::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_BUTTONW, lpName, BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | BS_VCENTER, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);

	tType = TYPE_CHECK;
}

bool CCtrlCheckBox::GetCheck() { return SendMessageW(*this, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false; }

void CCtrlCheckBox::SetCheck(bool check) { SendMessageW(*this, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, 0); }

///////////////////////////////////////////////////////
void CCtrlDropBox::CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_VISIBLE | WS_CHILD, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);

	tType = TYPE_LIST;
}

void CCtrlDropBox::Reset() { SendMessageW(*this, CB_RESETCONTENT, 0, 0); }

void CCtrlDropBox::AddString(LPCWSTR lpString) { SendMessageW(*this, CB_ADDSTRING, 0, (LPARAM)lpString); }

int CCtrlDropBox::GetSelection() { return (int)SendMessageW(*this, CB_GETCURSEL, 0, 0); }

void CCtrlDropBox::SetSelection(int sel) { SendMessageW(*this, CB_SETCURSEL, sel, 0); }

///////////////////////////////////////////////////////
void CCtrlTextBox::CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	CWnd::CreateWindow(WC_EDITW, L"", ES_LEFT | WS_VISIBLE | WS_CHILD | WS_BORDER, X, Y, Width, Height, hParent, hInstance);
	SendMessageW(*this, WM_SETFONT, (LPARAM)hFont, TRUE);
	SetWindowLongPtrW(*this, GWLP_USERDATA, (LONG_PTR)this);

	tType = TYPE_TEXT;
}

void CCtrlTextBox::SetString(std::wstring lpString) { SendMessageW(*this, WM_SETTEXT, 0, (LPARAM)lpString.c_str()); }

std::wstring CCtrlTextBox::GetString()
{
	wchar_t text[MAX_PATH] = { NULL };
	SendMessageW(*this, WM_GETTEXT, MAX_PATH, LPARAM(text));
	return std::wstring(text);
}

///////////////////////////////////////////////////////
void CCtrlDescription::CreateWindow(int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont, HFONT hBold)
{
	hBack.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_WHITERECT | SS_RIGHTJUST | WS_BORDER, X, Y, Width, Height, hParent, hInstance);
	hStrike.CreateWindow(WC_STATICW, L"", WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 4, 22, Width - 8, Height, hBack, hInstance);
	hCaption.CreateWindow(L"", 4, 2, Width - 8, 20, hBack, hInstance, hBold, 0);
	hText.CreateWindow(L"", 4, 24, Width - 8, Height - 32, hBack, hInstance, hFont, 0);
}

void CCtrlDescription::SetCaption(LPCWSTR lpCaption)
{
	if (hCaption.SetText(lpCaption))
	{
		InvalidateRect(hCaption, nullptr, true);
		UpdateWindow(hCaption);
	}
}

void CCtrlDescription::SetText(LPCWSTR lpText)
{
	if (hText.SetText(lpText))
	{
		InvalidateRect(hText, nullptr, true);
		UpdateWindow(hText);
	}
}

///////////////////////////////////////////////////////
// COMBINED CONTROLS
///////////////////////////////////////////////////////
void CFieldCheck::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	uType = TYPE_CHECK;
	box.CreateWindow(lpName, X, Y, Width, Height, hParent, hInstance, hFont);
	SetWindowLongPtrW(box, GWLP_USERDATA, (LONG_PTR)this);
	box.Subclass(proc);
}

void CFieldCheck::Release()
{
	box.Destroy();
}

bool CFieldCheck::GetCheck() { return box.GetCheck(); }

void CFieldCheck::SetCheck(bool check) { box.SetCheck(check); }

LRESULT CFieldCheck::proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CFieldCheck* check = (CFieldCheck*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (Msg)
	{
	case WM_MOUSEMOVE:
		if (check->cCtrl)
		{
			check->cCtrl->SetCaption(check->szText.c_str());
			check->cCtrl->SetText(check->szDesc.c_str());
		}
		break;
	}

	return check->box.CallProcedure(hWnd, Msg, wParam, lParam);
}

///////////////////////////////////////////////////////
void CFieldList::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	uType = TYPE_LIST;
	const int sub = 140;
	hStatic.CreateWindow(lpName, X, Y, Width - sub, Height, hParent, hInstance, hFont);
	hList.CreateWindow(X + Width - sub, Y, sub, Height, hParent, hInstance, hFont);

	SetWindowLongPtrW(hStatic, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtrW(hList, GWLP_USERDATA, (LONG_PTR)this);
	hStatic.Subclass(proc_static);
	hList.Subclass(proc_list);
}

void CFieldList::Release()
{
	hStatic.Destroy();
	hList.Destroy();
}

void CFieldList::Reset() { hList.Reset(); }

void CFieldList::AddString(LPCWSTR lpString) { hList.AddString(lpString); }

int CFieldList::GetSelection() { return hList.GetSelection(); }

void CFieldList::SetSelection(int sel) { hList.SetSelection(sel); }

LRESULT CFieldList::proc_list(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CFieldList* list = (CFieldList*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (Msg)
	{
	case WM_MOUSEMOVE:
		if (list->cCtrl)
		{
			list->cCtrl->SetCaption(list->szText.c_str());
			list->cCtrl->SetText(list->szDesc.c_str());
		}
		break;
	}

	return list->hList.CallProcedure(hWnd, Msg, wParam, lParam);
}

LRESULT CFieldList::proc_static(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CFieldList* st = (CFieldList*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (Msg)
	{
	case WM_NCHITTEST:
		if (st->cCtrl)
		{
			st->cCtrl->SetCaption(st->szText.c_str());
			st->cCtrl->SetText(st->szDesc.c_str());
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		st->hStatic.OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 0;
	}

	return st->hStatic.CallProcedure(hWnd, Msg, wParam, lParam);
}

///////////////////////////////////////////////////////
void CFieldText::CreateWindow(LPCWSTR lpName, int X, int Y, int Width, int Height, HWND hParent, HINSTANCE hInstance, HFONT hFont)
{
	uType = TYPE_TEXT;
	const int sub = 140;
	hStatic.CreateWindow(lpName, X, Y, Width - sub, Height, hParent, hInstance, hFont);
	hList.CreateWindow(X + Width - sub, Y, sub, Height, hParent, hInstance, hFont);

	SetWindowLongPtrW(hStatic, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtrW(hList, GWLP_USERDATA, (LONG_PTR)this);
	hStatic.Subclass(proc_static);
	hList.Subclass(proc_text);
}

void CFieldText::Release()
{
	hStatic.Destroy();
	hList.Destroy();
}

void CFieldText::SetString(std::wstring lpString) { hList.SetString(lpString); }

std::wstring CFieldText::GetString() { return hList.GetString(); }

LRESULT CFieldText::proc_text(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CFieldText* text = (CFieldText*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (Msg)
	{
	case WM_MOUSEMOVE:
		if (text->cCtrl)
		{
			text->cCtrl->SetCaption(text->szText.c_str());
			text->cCtrl->SetText(text->szDesc.c_str());
		}
		break;
	}

	return text->hList.CallProcedure(hWnd, Msg, wParam, lParam);
}

LRESULT CFieldText::proc_static(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CFieldText* st = (CFieldText*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

	switch (Msg)
	{
	case WM_NCHITTEST:
		if (st->cCtrl)
		{
			st->cCtrl->SetCaption(st->szText.c_str());
			st->cCtrl->SetText(st->szDesc.c_str());
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		st->hStatic.OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 0;
	}

	return st->hStatic.CallProcedure(hWnd, Msg, wParam, lParam);
}

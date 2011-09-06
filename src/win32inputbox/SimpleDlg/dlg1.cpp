#include <windows.h>
#include "resource.h"
#include <stdio.h>

LRESULT CALLBACK dlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
    case WM_CLOSE:
      ::EndDialog(hDlg, IDOK);
      return TRUE;
  }
  return 0;
}

#include "n.h"

void show_calc()
{
  ::DialogBoxIndirectParam((HINSTANCE) ::GetModuleHandle(0), 
        (LPCDLGTEMPLATE) dlg_14, 
        0, 
        (DLGPROC)dlgProc, 0);
  ::GetLastError();
}

int main()
{
  show_calc();

  HMODULE hModule = ::GetModuleHandle(0);
  HINSTANCE hInst = hModule;

  HRSRC hrsrc = ::FindResource(hModule, MAKEINTRESOURCE(IDD_MYDLG), RT_DIALOG);

  HGLOBAL hglobal = ::LoadResource(hModule, hrsrc);

  ::DialogBoxIndirectParam(hInst, (LPCDLGTEMPLATE) hglobal, 0, (DLGPROC)dlgProc, 0);

  return 0;
}
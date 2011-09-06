// dlgX.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "dlgX.h"
#include "Win32InputBox.h"

int test_resource_dialog(bool bMultiLine = false)
{
  //testInputDialog();
  char buf[MAX_PATH] = "";
  WIN32INPUTBOX_PARAM param;

  param.szTitle = "[RESOURCE] What's up doc?";
  param.szPrompt = "Enter your name:";
  param.szResult = buf;
  param.nResultSize = MAX_PATH;
  param.DlgTemplateName = MAKEINTRESOURCE(IDD_INPUTBOX);
  param.bMultiline = bMultiLine;
  CWin32InputBox::InputBoxEx(&param);

  OutputDebugString(buf);

  return 0;
}

void test_inputbox(bool bMultiLine = false)
{
  char buf[100]= {0};
  CWin32InputBox::InputBox("hello", "what?", buf, 100, bMultiLine);
  OutputDebugString(buf);
}

int APIENTRY _tWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPTSTR    lpCmdLine,
  int       nCmdShow)
{
  test_resource_dialog(false);
  //test_inputbox(true);
	return 0;
}
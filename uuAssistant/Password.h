// Password.h
// The C# like Form class
// Author: Shu.KK
// 2006-11-01
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CC_WIN_PASSWORD_H_
#define CC_WIN_PASSWORD_H_

#include "..\cc\cc.All.h"
#include "resource.h"
#include "NoteForm.h"

using namespace cc;
using namespace cc::win;

class Password;
class MainForm;

class PWDLoginDlg : public Dialog
{
public:
	PWDLoginDlg(const TCHAR* sPWD = NULL, bool isPWDMayEmpty = false, const TCHAR* sTitle = NULL);
	virtual ~PWDLoginDlg();

	EventHandler OKClick;
	EventHandler CancelClick;

protected:
	bool _isPWDMayEmpty;
	TCHAR* _tcPWD;
	Label* label1;
	Label* label2;
	Label* label3;
	TextBox* _editText1;
	TextBox* _editText2;
	TextBox* _editText3;
	Button* _btnOK;
	Button* _btnCancel;

	void ok_Click(Object* sender, EventArgs* e);
	void cancel_Click(Object* sender, EventArgs* e);
};

class Password : public NoteBaseForm
{
protected:
	TextBox* editURL;
	ToolItem* toolrefresh;
	ToolItem* toolstop;
	ToolItem* toolgo;
	cc::Str sSaveUrl;
	bool isBlock;
	HWND hWndIE;
	TCHAR tcBuf[MAX_PATH + 1];

	MenuItem* menuClose;
	MenuItem* menuNewWin;
	MenuItem* menuFavorites;
	MenuItem* menuFavAdd;
	MenuItem* menuFavMan;
	MenuItem* menuOpenFile;
	//MenuItem* menuSource;
	MenuItem* menuReload;
	MenuItem* menuPrint;
	MenuItem* menuSaveAs;
	MenuItem* menuIsBlock;
	MenuItem* menuSaveURL;
	MenuItem* menuOption;
	MenuItem* menuSaveSize;
	MenuItem* menuBreak1;
	MenuItem* menuBreak2;

	ToolItem* toolexit;
	ToolItem* toolcopy;
	ToolItem* toolpaste;
	ToolItem* toolcut;
	ToolItem* toolbreak1;
	ToolItem* toolbreak2;
	ToolBar* toolbar;
protected:
	void InitializeComponent();
	void OnResize(EventArgs* e);
	void Form_Deactivate(Object* sender, EventArgs* e);
	void Form_Load(Object* sender, EventArgs* e);
	void Form_Closed(Object* sender, EventArgs* e);
	void move_DoubleClick(Object* sender, EventArgs* e);
	void close_Click(Object* sender, EventArgs* e);
	void NewWin_Click(Object* sender, EventArgs* e);
	void print_Click(Object* sender, EventArgs* e);
	void savedefault_Click(Object* sender, EventArgs* e);
	void editURL_KeyDown(Object* sender, KeyEventArgs* e);

	void menuFavorites_Popup(Object* sender, EventArgs* e);
	void menuFavorites_Click(Object* sender, EventArgs* e);
	void Favorites2Menu(MenuItem* item, cc::Str path);

	void tool_Click(Object* sender, EventArgs* e);
	void SaveConfig(bool isExit);

public:
	Password();

	virtual void LoadFromEntity(NoteEntity* entity);
	virtual void SaveToEntity(NoteEntity* entity);
};

#endif //#ifndef CC_WIN_PASSWORD_H_

// NoteForm.h
// The C# like Form class
// Author: Shu.KK
// 2006-11-01
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CC_WIN_NOTEFORM_H_
#define CC_WIN_NOTEFORM_H_

#include "..\cc\cc.All.h"
#include "resource.h"

using namespace cc;
using namespace cc::win;

class NoteBaseForm;
class NoteForm;
class MainForm;

#define IE_URL_LEN       200

#define MODE_NOTE        0x0000
#define MODE_TIME        0x0001
#define MODE_IE          0x0002
#define MODE_PASS        0x0003

#define NOTE_FLAG_MIN           0x00000001
#define NOTE_FLAG_FIX           0x00000002
#define NOTE_FLAG_CRYPT         0x00000004
#define NOTE_FLAG_CLOSE         0x00000008
#define NOTE_FLAG_BLOCK         0x00000010
#define NOTE_FLAG_SCRIPT        0x00000020
#define NOTE_FLAG_EXTFUN        0x00000040
#define NOTE_FLAG_HASFONT       0x00000080

#define NOTE_PWD_MAXLEN   40
#define NOTE_VER_1        20140701
typedef struct tagNoteEntityHeader
{
    char headerflag[2];
	unsigned int version;
	unsigned int noteCount;
	TCHAR notePwd[NOTE_PWD_MAXLEN+1];
	TCHAR tcDummy1[50];
	TCHAR tcDummy2[50];
	int nDummy1;
	int nDummy2;
	tagNoteEntityHeader()
	{
		headerflag[0] = 'U';
		headerflag[1] = 'W';
		version = NOTE_VER_1;
		noteCount = 0;
		memset(notePwd, _T('\0'), NOTE_PWD_MAXLEN+1);
		memset(tcDummy1, _T('\0'), 50);
		memset(tcDummy2, _T('\0'), 50);
		nDummy1 = 0;
		nDummy2 = 0;
	};
}NoteEntityHeader;

typedef struct tagNoteEntity
{
	NoteBaseForm* note;
	DWORD dwFlag;
	int nMode;
	COLORREF corText;
	COLORREF corBack;
	RECT rcPos;

	int nFontSize;
	int nFontWeight;
	BYTE isFontItalic; //while MiniIE is this->GetIsBlock()
	BYTE isFontUnderline;
	BYTE isFontStrikeOut;
	TCHAR tcURL[1];//while MiniIE is URL(also connect with tcFileName&tcFontName,len is IE_URL_LEN)
	TCHAR tcFileName[40];
	TCHAR tcFontName[40];
	TCHAR tcDummy1[50];
	TCHAR tcDummy2[50];
	TCHAR tcDummy3[20];//if MiniIE,then tcURL is to here
	TCHAR tcDummy4[10];
	int nDummy1;
	int nDummy2;
	int nDummy3;
	int nDummy4;
	tagNoteEntity()
	{
		note = NULL;
		dwFlag = 0;
		nMode = 0;
		corText = (COLORREF)-1;
		corBack = (COLORREF)-1;
		::SetRect(&rcPos, 0, 0, 0, 0);
		nFontSize = -1;
		nFontWeight = -1;
		isFontItalic = false;
		isFontUnderline = false;
		isFontStrikeOut = false;
		memset(tcURL, _T('\0'), 1);
		memset(tcFileName, _T('\0'), 40);
		memset(tcFontName, _T('\0'), 40);
		memset(tcDummy1, _T('\0'), 50);
		memset(tcDummy2, _T('\0'), 50);
		memset(tcDummy3, _T('\0'), 20);
		memset(tcDummy4, _T('\0'), 10);
		nDummy1 = 0;
		nDummy2 = 0;
		nDummy3 = 0;
		nDummy4 = 0;
	};
}NoteEntity;

#define NOTEFILE_PATH_KEY _T("NoteFilePath")
#define NOTEFILE_PATH_DEFAULT _T("Note\\")
#define NOTEFILE_COUNT_MAX -1

#define CC_NOTE_CFG_NORMAL 0
#define CC_NOTE_CFG_FONT 1
#define CC_NOTE_CFG_CLOSED 2
#define CC_NOTE_CFG_URL 3
class NoteBaseForm : public Form
{
protected:
	bool isModal;
	bool isMin;
	bool isCrypt;
	int nMode; //0:note,1:edit file,2:miniIE
	cc::Str filePath;
	RECT rcNormal;
	MainForm* mainForm;
	ImageList* imageList;
	ContextMenu* contMenu;
	ToolItem* moveForm;
	ToolItem* toolsysmenu;
	ToolItem* toolkey;
	ToolItem* toolattach;
	ToolItem* toollock;
	TextBox* editPWD;
	Button* btnPWD;
	Control* ctlMain;//while note is editText,and while miniie is ie

	//keyArr is param address,but sThisHead is by value
	virtual void DoSetLang(KeyArr& keyArr, cc::Str sThisHead);
	void AdjustLocation();

public:
	NoteBaseForm();
	virtual void SetIsMin(bool isMin);
	virtual bool GetIsMin();
	virtual void SetIsFix(bool isMin);
	virtual bool GetIsFix();
	virtual void SetIsCrypt(bool isCrypt, bool isSetPushBtn = false);
	virtual bool GetIsCrypt();
	virtual RECT GetNormalBounds();

	virtual void LoadFromEntity(NoteEntity* entity) = 0;
	virtual void SaveToEntity(NoteEntity* entity, int cfgType) = 0;
	virtual void SetMode(int nMode);
	virtual int GetMode();
};

#define CC_TIMERID_SENDKEY CC_TIMERID_USER+1
#define CC_TIMERID_SAVECFG CC_TIMERID_USER+2

class NoteForm : public NoteBaseForm
{
protected:
	TextBox* editText;

	MenuItem* menuClose;
	MenuItem* menuClear;
	MenuItem* menuNewWin;
	MenuItem* menuSave;
	MenuItem* menuReload;
	MenuItem* menuFont;
	MenuItem* menuBKColor;
	MenuItem* menuPrint;
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
	virtual void OnTimer(EventArgs *e);
	void Form_Deactivate(Object* sender, EventArgs* e);
	void Form_Load(Object* sender, EventArgs* e);
	void Form_Closed(Object* sender, EventArgs* e);
	void Form_Shutdown(Object* sender, EventArgs* e);
	void Form_SaveCfg(Object* sender, EventArgs* e);
	void move_DoubleClick(Object* sender, EventArgs* e);
	void close_Click(Object* sender, EventArgs* e);
	void reload_Click(Object* sender, EventArgs* e);
	void NewWin_Click(Object* sender, EventArgs* e);
	void clear_Click(Object* sender, EventArgs* e);
	void print_Click(Object* sender, EventArgs* e);
	void font_Click(Object* sender, EventArgs* e);
	void bkColor_Click(Object* sender, EventArgs* e);
	void savedefault_Click(Object* sender, EventArgs* e);
	virtual LRESULT WndProc(EventArgs *e);

	void save_Click(Object* sender, EventArgs* e);
	void key_Click(Object* sender, EventArgs* e);
	void copy_Click(Object* sender, EventArgs* e);
	void paste_Click(Object* sender, EventArgs* e);
	void cut_Click(Object* sender, EventArgs* e);
	void lock_Click(Object* sender, EventArgs* e);
	void decrypt_Click(Object* sender, EventArgs* e);

	void SaveToFile();

public:
	NoteForm();

	virtual void LoadFromEntity(NoteEntity* entity);
	virtual void SaveToEntity(NoteEntity* entity, int cfgType);
};

#endif //#ifndef CC_WIN_NOTEFORM_H_

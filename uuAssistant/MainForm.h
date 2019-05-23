//http://www.codeproject.com/editctrl/brainchild.asp
//http://www.vczx.com/tutorial/mfc/mfc12.php
// MainForm.h
// The C# like Form class
// Author: Shu.KK
// 2006-11-01
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CC_WIN_MAINFORM_H_
#define CC_WIN_MAINFORM_H_

#include "..\cc\cc.All.h"
#include "..\ccExt\ShowImage.h"
#include "resource.h"
#include "NoteForm.h"
#include "MiniIEForm.h"
#include "../uuSpy/uuSpyDll.h"
//#include "Password.h"
//#include "vistavol.h"
#include "../ccSample/DesktopCalendar.h"
//for ::OleLoadPicture
#include <olectl.h>
#include "initguid.h"

#include "../ccExt/zlib/zlib.h"
#include "../ccExt/png/png.h"

#include "../ccExt/jpeg/jpeglib.h"
#include "../ccExt/jpeg/jerror.h"

#include "../ccExt/tiff/tiffio.h"

#include "../ccExt/CxImage/ximage.h"
#include "../ccExt/CxImage/ximagif.h"
#include "../ccExt/CxImage/ximatif.h"
#include "../ccExt/CxImage/xfile.h"

using namespace cc;
using namespace cc::win;

#define BUF_LEN 256

//volume
//http://sourceforge.net/projects/hotkeyp/

//timer to notice mainform to restore system time
#define UU_TIMERID_UPDATE   CC_TIMERID_USER + 3
//flash window, and refresh notice window at app start
#define UU_TIMERID_WFLASH   CC_TIMERID_USER + 4
#define UU_TIMERID_AUTORUNCHK   CC_TIMERID_USER + 5
#define UU_TIMERID_CALENDAR   CC_TIMERID_USER + 6
#define UU_TIMERID_PICKCOLOR   CC_TIMERID_USER + 7
//next is used by Calendar!( at desktopcalendar.h)
//#define UU_TIMERID_CALENDAR_EXIT   CC_TIMERID_USER + 7
#define UU_TIMERID_CAPTURE_EXIT   CC_TIMERID_USER + 8
#define UU_TIMERID_ONTOP          CC_TIMERID_USER + 9
#define UU_TIMERID_ONTOP2         CC_TIMERID_USER + 10
#define UU_TIMERID_MIN_HIDE       CC_TIMERID_USER + 11
#define UU_TIMERID_MIN_HIDE2      CC_TIMERID_USER + 12

class ShowImageDialog : public Form
{
public:
	ShowImageDialog();
	ScrollContainer* scrWork;
	ShowImage* showImgBig;

	void scrWork_UpdateClientPosition(Object* sender, EventArgs* e);
};

class CaptureDialog : public Form
{
public:
	CaptureDialog(HWND mainFormHwnd);

protected:
	HWND mainFormHwnd;
	ImageList* imageList;
	ToolItem* toolMove;
	ToolItem* toolCapWin;
	ToolItem* toolCapScreen;
	TextBox* editScroll;
	HWND hWndActive;
	//CheckBox* chkSave;
	TextBox* editPath;
	Button* btnPath;
	ComboBox* comboList;

	ContextMenu* contMenu;
	MenuItem* menuClose;
	MenuItem* menuBreak1;
	MenuItem* menuCopy;
	MenuItem* menuSave;
	MenuItem* menuEdit;
	ToolItem* toolsysmenu;

	void Form_Load(Object* sender, EventArgs* e);
	void Form_Closed(Object* sender, EventArgs* e);
	void Form_Shutdown(Object* sender, EventArgs* e);
	void Control_Click(Object* sender, EventArgs* e);
	void toolCapWin_MouseMove(Object* sender, MouseEventArgs* e);
	void DoSetLang(KeyArr& keyArr, cc::Str sThisHead);
	LRESULT WndProc(EventArgs *e);
};

class StepDialog : public Form
{
public:
	StepDialog(int maxStep = 255);
	void SetStep(int index, int delay = 2);

protected:
	ProgressBar* progBar;
	void OnInnerTimer(EventArgs *e);
};

#define UU_MAINFLAG_EXIT         0x00000001
#define UU_MAINFLAG_UPDNOTE      0x00000002
#define UU_MAINFLAG_GETHWND      0x00000004
#define UU_MAINFLAG_REFRESHMENU      0x00000008
#define UU_MAINFLAG_PASTE      0x00000010
class MainForm : public Form
{
protected:
	WindowsVersion::Type winVer;
	List<void*>* lstNote;
	NoteEntity* noteDefault;
	NoteEntity* noteWKTime;
	cc::Str* notePWD;
	NoteEntity* noteTmp;
	//bool isDoClosed;
	DWORD m_MainFlag;
	CaptureDialog* captureDialog;
	NoteBaseForm* lastNote;
	//Password* password;
	CMainForm* formCalendar;

	ImageList* imageList;
	cc::RegistryKey* registryKey1;

	MainMenuBar* mainMenuBar;
	MainMenu* mainMenu;
	MenuItem* menuFile;
	MenuItem* menuExit;

	MenuItem* menuEdit;
	MenuItem* menuUnDo;
	MenuItem* menuReDo;
	MenuItem* menuEditBreak1;
	MenuItem* menuCut;
	MenuItem* menuCopy;
	MenuItem* menuPaste;
	MenuItem* menuDel;
	MenuItem* menuEditBreak2;
	MenuItem* menuSelAll;

	MenuItem* menuView;
	MenuItem* menuToolBar;
	MenuItem* menuCommTool;
	MenuItem* menuEditTool;
	MenuItem* menuViewBreak1;
	MenuItem* menuFixTools;
	MenuItem* menuStatusBar;
	MenuItem* menuTips;

	MenuItem* menuHelp;
	MenuItem* menuAbout;

	ContextMenu* contMenu;
	MenuItem* menuShowNote;
	MenuItem* menuNewIE;
	MenuItem* menuNewNote;
	MenuItem* menuCloseAll;
	MenuItem* menuBreak1;
	MenuItem* menuCapture;
	//MenuItem* menuPassword;
	MenuItem* menuWorkTime;
	MenuItem* menuCalendar;
	MenuItem* menuShow;
	MenuItem* menuExit2;
	NotifyIcon* notify1;
	Hotkey* hotkey1;

	ToolItem* toolOnTop;
	ToolItem* toolSetBack;
	ToolBar* commTool;

	ToolItem* toolCopy;
	ToolItem* toolPaste;
	ToolItem* toolCut;
	ToolBar* editTool;

	Label* label;
	TabControlEx* tab1;

	//pageNote
	TabPage* pageNote;
	ListBox* ltbNoeForm;
	Button* btnNoteNew;
	Button* btnMiniIENew;
	Button* btnNoteShowHide;
	Button* btnNoteRemove;
	Button* btnNoteSetPWD;
	Button* btnNoteTimer;
	Button* btnNoteUp;
	Button* btnNoteDown;
	CheckBox* chkMirco;
	CheckBox* chkCalendar;

	//pageWorkTime
	//TabPage* pageWorkTime;
	//TextBox* editTime;
	//Button* btnRefreshTime;
	Button* btnOpenTime;

	//pageTool
	TabPage* pageTool;
	//ComboBox* comboWinList;
	//Button* btnRefresh;
	//HScrollBar* hScrollBar1;
	PickPercentCtl* pickPercent;
	//Button* btnChgWin;
	//Button* btnResetWin;
	//Button* btnOnTop;
	//Button* btnNotOnTop;
	CheckBox* chkShowCapture;
	PickColorCtl* toolSelectColor;
	TextBox* labToolColor;
	int toolColorStart;
	COLORREF toolColorMin;
	COLORREF toolColorMax;
	TextBox* editRGB;
	Button* btnRunPrg;
	Button* btnRunCancel;
	ComboBox* comboPrg;
	//PathButton* btnSelFrom;
	ToolItem* btnRunAdd;
	ToolItem* btnRunDel;
	//TextBox* editRunTime;
	//TextBox* editRunTime2;
	TextBox* editRunTime3;
	//Button* btnShowPassword;
	SYSTEMTIME stSaveTime;
	SYSTEMTIME stNewTime;
	virtual void OnInnerTimer(EventArgs *e);

	//volume
	StepDialog* volumeDialog;
	StepDialog* transparentDialog;

	//get password
	HWND hWndPrev;
	HINSTANCE glhInstance;
	TextBox* editPassword;
	Button* btnGetPWD;
	void btnGetPWD_Click(HWND hWnd);
	HANDLE pShareMemoryMap;
	ShareEntity* pShareMemory;

	//pageHotkey
	TabPage* pageHotkey;
	RichTextBox* editHotkeyInfo;

	//pageHide
	TabPage* pageHide;
	//CheckListBox* chklWinList;
	//Button* btnHideRefresh;
	Button* btnShowAll;
	TextBox* editFilter[15];
	CheckBox* chkHideNote;
	Button* btnGetHwnd[5];

	//pageOption
	TabPage* pageOption;
	CheckBox* chkAutoRun;
	ComboBox* comboLang;
	Button* btnOK;
	VersionInfo* verInfo;

	StatusItem* statusitem1;
	StatusBar* statusbar;

	HWND hWndHide;
	List<HWND>* lstWin;
	List<HWND>* lstWinHide;
	List<HWND>* lstWinHideSave;
	List<HWND>* lstWinTrans;
	TCHAR buf[BUF_LEN];
	int nFlashTime;
	HWND hWndFlash;
	clock_t tmVolumeDbClick;
	//HWND hClipboard;
	List<cc::Str>* lstClipboard;
	ContextMenu* menuClipboard;

	void InitializeComponent();
	//keyArr is param address,but sThisHead is by value
	virtual void DoSetLang(KeyArr& keyArr, cc::Str sThisHead);

	//do case with sender
	void Control_Click(Object* sender, EventArgs* e);
	LRESULT WndProc(EventArgs *e);

	void LoadAllNoteW();
	void SetLangAllNoteW();
	cc::Str GetNewNoteFile();
	int GetNoteIndex(NoteBaseForm* note);
	void contMenu_Popup(Object* sender, EventArgs* e);
	void NoteShow_Click(Object* sender, EventArgs* e);
	void NoteNew_Click(Object* sender, EventArgs* e);
	void NoteShowHide_Click(Object* sender, EventArgs* e);
	void NoteSetPWD_Click(Object* sender, EventArgs* e);
	//void NoteTimer_Click(Object* sender, EventArgs* e);
	void NoteRemove_Click(Object* sender, EventArgs* e);
	void NoteUp_Click(Object* sender, EventArgs* e);
	void NoteDown_Click(Object* sender, EventArgs* e);
	void Tool_MouseDown(Object* sender, MouseEventArgs* e);
	void Tool_ValueChanged(Object* sender, EventArgs* e);
	//void btnGetPWD_Click(Object* sender, EventArgs* e);
	void GetHwnd_MouseDown(Object* sender, MouseEventArgs* e);
	void GetHwnd_MouseMove(Object* sender, MouseEventArgs* e);
	void GetHwnd_MouseUp(Object* sender, MouseEventArgs* e);
	void GetHwnd_MouseDown2(Object* sender, MouseEventArgs* e);
	void GetHwnd_MouseMove2(Object* sender, MouseEventArgs* e);
	void GetHwnd_MouseUp2(Object* sender, MouseEventArgs* e);

	//void Note_DoubleClick(Object* sender, EventArgs* e);
	void Form_Load(Object* sender, EventArgs* e);
	void Form_Closing(Object* sender, CancelEventArgs* e);
	void Form_Closed(Object* sender, EventArgs* e);
	void Form_Resize(Object* sender, EventArgs* e);
	void Form_Shutdown(Object* sender, EventArgs* e);
	void menuExit_Click(Object* sender, EventArgs* e);
	void OptionOK_Click(Object* sender, EventArgs* e);
	void showhide_Click(Object* sender, EventArgs* e);
	//void refresh_Click(Object* sender, EventArgs* e);
	//void refreshHide_Click(Object* sender, EventArgs* e);
	//bool CheckHwnd();
	//void ontop_Click(Object* sender, EventArgs* e);
	//void notontop_Click(Object* sender, EventArgs* e);
	//void chgwin_Click(Object* sender, EventArgs* e);
	void hotkey_Press(Object* sender, EventArgs* e);
	//void refreshtime_Click(Object* sender, EventArgs* e);
	void autorun_Click(Object* sender, EventArgs* e);
	void opentime_Click(Object* sender, EventArgs* e);
	void ShowAllHideW(DWORD dwShowOrHide);
	cc::Str GetCurTime(const TCHAR* sPre);
	void closeall_Click(Object* sender, EventArgs* e);
	void menuAbout_Click(Object* sender, EventArgs* e);
	void toolOnTop_Click(Object* sender, EventArgs* e);
	void toolSetBack_Click(Object* sender, EventArgs* e);
	void menuCommTool_Click(Object* sender, EventArgs* e);
	void menuEditTool_Click(Object* sender, EventArgs* e);
	void menuFixTools_Click(Object* sender, EventArgs* e);
	void menuTips_Click(Object* sender, EventArgs* e);
	void menuStatusBar_Click(Object* sender, EventArgs* e);
	void menuClipboard_Click(Object* sender, EventArgs* e);

	//for note to mircosize
	void chkMirco_Click(Object* sender, EventArgs* e);

public:
	MainForm();
	virtual ~MainForm();

	NoteForm* NewNote(cc::Str path = cc::Str());
	void NoteChangeTitle(NoteBaseForm* note, cc::Str& sTxt);
	NoteEntity* GetNoteEntity(NoteBaseForm* note);
	NoteEntity* GetNoteDefaultEntity();
	NoteEntity* GetNoteWKTimeEntity();
	const cc::Str& GetNotePWD();
	void SaveAllNoteW(bool isExit);

	TextBox* GetCurControl();
	void SetEditMenuEnable(bool enable);
	void ChgEdit_Focus(Object* sender, EventArgs* e);
	void menuUnDo_Click(Object* sender, EventArgs* e);
	void menuReDo_Click(Object* sender, EventArgs* e);
	void menuCut_Click(Object* sender, EventArgs* e);
	void menuCopy_Click(Object* sender, EventArgs* e);
	void menuPaste_Click(Object* sender, EventArgs* e);
	void menuDel_Click(Object* sender, EventArgs* e);
	void menuSelAll_Click(Object* sender, EventArgs* e);

	//for note to mircosize
	bool GetIsMirco();

	MiniIEForm* NewMiniIE();
	void RemoveBlankIE(NoteBaseForm* note);
	void NewIE_Click(Object* sender, EventArgs* e);

};

#endif //#ifndef CC_WIN_MAINFORM_H_

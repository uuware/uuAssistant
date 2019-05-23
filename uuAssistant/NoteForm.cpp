#include "NoteForm.h"
#include "MainForm.h"
using namespace cc;
using namespace cc::win;

NoteBaseForm::NoteBaseForm()
{
	ctlMain = NULL;
}

RECT NoteBaseForm::GetNormalBounds()
{
	if(isMin)
	{
		int nW = rcNormal.right - rcNormal.left;
		int nH = rcNormal.bottom - rcNormal.top;
		RECT rc = GetBounds();
		rcNormal.left = rc.left;
		rcNormal.top = rc.top;
		rcNormal.right = rc.left + nW;
		rcNormal.bottom = rc.top + nH;
		return rcNormal;
	}
	return GetBounds();
}

void NoteBaseForm::DoSetLang(KeyArr& keyArr, cc::Str sThisHead)
{
	Form::DoSetLang(keyArr, sThisHead);
	contMenu->SetLang(keyArr, sThisHead);
}

void NoteBaseForm::SetIsMin(bool isMin)
{
	if(this->isMin != isMin)
	{
		this->isMin = isMin;
		if(!isMin)
		{
			RECT rcTmp = GetBounds();
			this->SetBoundsNotOriginal(rcTmp.left, rcTmp.top, rcNormal.right - rcNormal.left, rcNormal.bottom - rcNormal.top);
			this->SetStyle(StyleType::Style, WS_DLGFRAME|WS_BORDER, WS_THICKFRAME, true);
			toolsysmenu->SetEnabled(true);
			::SetWindowPos(this->m_hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
			moveForm->SetLocation(-1, 0);
		}
		else
		{
			rcNormal = GetBounds();
			toolsysmenu->SetEnabled(false);
			if(mainForm->GetIsMirco())
			{
				this->SetStyle(StyleType::Style, WS_THICKFRAME, WS_BORDER, true);
				this->SetBoundsNotOriginal(rcNormal.left, rcNormal.top, 6, 6);
			}
			else
			{
				this->SetStyle(StyleType::Style, WS_THICKFRAME, WS_DLGFRAME, true);
				//this->SetBoundsNotOriginal(rcNormal.left, rcNormal.top, 20, 21);
				this->SetBoundsNotOriginal(rcNormal.left, rcNormal.top, 14, 15);
			}
			//moveForm->SetLocation(-4, -3);
			moveForm->SetLocation(-7, -5);
		}
	}
}

bool NoteBaseForm::GetIsMin()
{
	return isMin;
}

void NoteBaseForm::SetIsFix(bool isFix)
{
	toolattach->SetPushed(isFix);
}

bool NoteBaseForm::GetIsFix()
{
	return toolattach->GetPushed();
}

void NoteBaseForm::SetMode(int nMode)
{
	this->nMode = nMode;
	if(nMode == MODE_TIME)
	{
		toollock->SetVisible(false);
		moveForm->SetIcon(imageList, IDI_ICON_NOTE);
	}
}

int NoteBaseForm::GetMode()
{
	return nMode;
}

void NoteBaseForm::SetIsCrypt(bool isCrypt, bool isSetPushBtn)
{
	if(!isCrypt && !ctlMain->GetVisible())
	{
		cc::Str sPWD = mainForm->GetNotePWD();
		cc::Str sTxt = cc::Str(editPWD->GetText());
		if(sPWD.GetLength() > 0 && !sTxt.Equals(sPWD))
		{
			isModal = true;
			::MessageBox(m_hWnd, CC_APP_GetLang()->Get(_T("Lang.U_NOTE_MSG_PWD_Error")), NULL, MB_OK);
			isModal = false;
			return;
		}
		editPWD->SetText(NULL);
	}
	ctlMain->SetVisible(!isCrypt);
	editPWD->SetVisible(isCrypt);
	btnPWD->SetVisible(isCrypt);
	if(isSetPushBtn)
	{
		toollock->SetPushed(isCrypt);
		if(nMode == MODE_NOTE)
		{
			if(isCrypt)
			{
				moveForm->SetIcon(imageList, IDI_ICON_NOTEMIN2);
			}
			else
			{
				moveForm->SetIcon(imageList, IDI_ICON_NOTEMIN);
			}
		}
	}
}

bool NoteBaseForm::GetIsCrypt()
{
	return toollock->GetPushed();
}

void NoteBaseForm::AdjustLocation()
{
	if(GetVisible() && this->GetWindowState() == FormWindowState::Normal) {
		//int w = LOWORD(e->LParam);
		//int h = HIWORD(e->LParam);
		RECT rect = {0, 0, 0, 0};
		
		HMONITOR hm = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
		if(hm != NULL) {
			MONITORINFO mix;
			mix.cbSize = sizeof(mix);
			if(GetMonitorInfo(hm, &mix)) {
				CopyRect(&rect, &mix.rcWork);
			}
			else {
				hm = NULL;
			}
		}
		if(hm == NULL) {
			if(!::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0))
			{
				rect.right = (LONG)::GetSystemMetrics(SM_CXSCREEN);
				rect.bottom = (LONG)::GetSystemMetrics(SM_CYSCREEN);
			}
		}

		RECT rect2 = GetNormalBounds();
		if(rect2.top < rect.top) {
			rect2.top = rect.top;
		}
		if(rect2.left < rect.left) {
			rect2.left = rect.left;
		}
		if(rect2.top > rect.bottom) {
			rect2.top = rect.bottom - 32;
		}
		if(rect2.left > rect.right) {
			rect2.left = rect.right - 32;
		}
		::SetWindowPos(m_hWnd, NULL, rect2.left, rect2.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	}
	::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetAlwaysOnTop(true);
}

NoteForm::NoteForm()
{
	InitializeComponent();
}

void NoteForm::InitializeComponent()
{
	mainForm = (MainForm*)CC_APP_GetVarious(_T("MainForm"));
	imageList = (ImageList*)CC_APP_GetVarious(_T("ImageList"));
	isMin = false;
	isCrypt = false;
	isModal = false;
	nMode = MODE_NOTE;

	//context menu
	contMenu = new ContextMenu();
	contMenu->SetName(_T("contMenu"));
	contMenu->SetAllOwnerDraw(true);

	menuClose = new MenuItem();
	menuClose->SetName(_T("menuClose"));
	menuClose->Click += EventHandler((Object*)this, (EventFun)&NoteForm::close_Click);
	menuClose->SetIcon(imageList, IDI_ICON_EXIT);
	menuClose->SetShortcut(Shortcut::CtrlQ);
	contMenu->ItemAdd(menuClose);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuClear = new MenuItem();
	menuClear->SetName(_T("menuClear"));
	menuClear->Click += EventHandler((Object*)this, (EventFun)&NoteForm::clear_Click);
	menuClear->SetIcon(imageList, IDI_ICON_CLEAR);
	contMenu->ItemAdd(menuClear);

	menuNewWin = new MenuItem();
	menuNewWin->SetName(_T("menuNewWin"));
	menuNewWin->Click += EventHandler((Object*)this, (EventFun)&NoteForm::NewWin_Click);
	menuNewWin->SetIcon(imageList, IDI_ICON_NEWWIN);
	menuNewWin->SetShortcut(Shortcut::CtrlN);
	contMenu->ItemAdd(menuNewWin);

	menuSave = new MenuItem();
	menuSave->SetName(_T("menuSave"));
	menuSave->Click += EventHandler((Object*)this, (EventFun)&NoteForm::save_Click);
	menuSave->SetIcon(imageList, IDI_ICON_SAVE);
	menuSave->SetShortcut(Shortcut::CtrlS);
	contMenu->ItemAdd(menuSave);

	menuReload = new MenuItem();
	menuReload->SetName(_T("menuReload"));
	menuReload->Click += EventHandler((Object*)this, (EventFun)&NoteForm::reload_Click);
	menuReload->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuReload);

	menuBreak2 = new MenuItem();
	menuBreak2->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak2);

	menuFont = new MenuItem();
	menuFont->SetName(_T("menuFont"));
	menuFont->Click += EventHandler((Object*)this, (EventFun)&NoteForm::font_Click);
	menuFont->SetIcon(imageList, IDI_ICON_FONT);
	contMenu->ItemAdd(menuFont);

	menuBKColor = new MenuItem();
	menuBKColor->SetName(_T("menuBKColor"));
	menuBKColor->Click += EventHandler((Object*)this, (EventFun)&NoteForm::bkColor_Click);
	menuBKColor->SetIcon(imageList, IDI_ICON_COLOR);
	contMenu->ItemAdd(menuBKColor);

	menuPrint = new MenuItem();
	menuPrint->SetName(_T("menuPrint"));
	menuPrint->Click += EventHandler((Object*)this, (EventFun)&NoteForm::print_Click);
	menuPrint->SetIcon(imageList, IDI_ICON_PRINT);
	menuPrint->SetShortcut(Shortcut::CtrlP);
	contMenu->ItemAdd(menuPrint);

	menuSaveSize = new MenuItem();
	menuSaveSize->SetName(_T("menuSaveSize"));
	menuSaveSize->Click += EventHandler((Object*)this, (EventFun)&NoteForm::savedefault_Click);
	menuSaveSize->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveSize);

	moveForm = new ToolItem();
	moveForm->SetName(_T("moveForm"));
	//moveForm->SetMoveForm(this);
	moveForm->SetLocation(-1, 0);
	moveForm->SetSize(20, 18);
	moveForm->SetText(NULL);
	moveForm->SetIcon(imageList, IDI_ICON_NOTEMIN);
	moveForm->SetStyle(StyleType::Outer, 0, OuterStyle::AlwaysNoEdge|OuterStyle::AutoSize);
	moveForm->DoubleClick += EventHandler((Object*)this, (EventFun)&NoteForm::move_DoubleClick);
	moveForm->SetMouseDownMoveForm(this);
	this->AddControl(moveForm);

	toolsysmenu = new ToolItem();
	toolsysmenu->SetName(_T("toolsysmenu"));
	toolsysmenu->SetContextMenu(contMenu);
	toolsysmenu->SetText(NULL);
	toolsysmenu->SetLocation(20, 2);
	toolsysmenu->SetSize(11, 19);
	this->AddControl(toolsysmenu);

	//toolbar
	toolbar = new ToolBar();
	toolbar->SetName(_T("editTool"));
	toolbar->SetLocation(31, 0);
	toolbar->SetMinSize(0, 23);
	this->AddControl(toolbar);

	toolexit = new ToolItem();
	toolexit->SetName(_T("toolExit"));
	toolexit->SetMargin(2, 1, 2, 1);
	toolexit->SetIcon(imageList, IDI_ICON_EXIT);
	toolexit->Click += EventHandler((Object*)this, (EventFun)&NoteForm::close_Click);
	toolbar->AddControl(toolexit);

	toolbreak1 = new ToolItem();
	toolbreak1->SetBreak(true);
	toolbar->AddControl(toolbreak1);

	toolkey = new ToolItem();
	toolkey->SetName(_T("toolKey"));
	toolkey->SetMargin(2, 1, 2, 1);
	toolkey->SetIcon(imageList, IDI_ICON_KEY);
	toolkey->Click += EventHandler((Object*)this, (EventFun)&NoteForm::key_Click);
	toolbar->AddControl(toolkey);

	toolbreak1 = new ToolItem();
	toolbreak1->SetBreak(true);
	toolbar->AddControl(toolbreak1);

	toolcopy = new ToolItem();
	toolcopy->SetName(_T("toolCopy"));
	toolcopy->SetMargin(2, 1, 2, 1);
	toolcopy->SetIcon(imageList, IDI_ICON_COPY);
	toolcopy->Click += EventHandler((Object*)this, (EventFun)&NoteForm::copy_Click);
	toolbar->AddControl(toolcopy);

	toolcut = new ToolItem();
	toolcut->SetName(_T("toolCut"));
	toolcut->SetMargin(2, 1, 2, 1);
	toolcut->SetIcon(imageList, IDI_ICON_CUT);
	toolcut->Click += EventHandler((Object*)this, (EventFun)&NoteForm::cut_Click);
	toolbar->AddControl(toolcut);

	toolpaste = new ToolItem();
	toolpaste->SetName(_T("toolPaste"));
	toolpaste->SetMargin(2, 1, 2, 1);
	toolpaste->SetIcon(imageList, IDI_ICON_PASTE);
	toolpaste->Click += EventHandler((Object*)this, (EventFun)&NoteForm::paste_Click);
	toolbar->AddControl(toolpaste);

	toolbreak2 = new ToolItem();
	toolbreak2->SetBreak(true);
	toolbar->AddControl(toolbreak2);

	toolattach = new ToolItem();
	toolattach->SetName(_T("toolAttach"));
	toolattach->SetMargin(2, 1, 2, 1);
	toolattach->SetIcon(imageList, IDI_ICON_ATTACH);
	toolattach->SetIsPushButton(true);
	toolattach->SetPushed(true);
	toolattach->Click += EventHandler((Object*)this, (EventFun)&NoteForm::Form_SaveCfg);
	toolbar->AddControl(toolattach);

	toollock = new ToolItem();
	toollock->SetName(_T("toolLock"));
	toollock->SetMargin(2, 1, 2, 1);
	toollock->SetIcon(imageList, IDI_ICON_LOCK);
	toollock->SetIsPushButton(true);
	toollock->Click += EventHandler((Object*)this, (EventFun)&NoteForm::lock_Click);
	toolbar->AddControl(toollock);

	editText = new TextBox();
	editText->SetName(_T("editText1"));
	editText->SetText(_T(""));
	editText->SetStyle(StyleType::Style, 0, ES_NOHIDESEL|WS_TABSTOP);
	editText->SetMultiline(true);
	editText->SetLocation(0, 23);
	editText->SetSize(600, 480 - 23);
	editText->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	this->AddControl(editText);
	ctlMain = editText;

	editPWD = new TextBox();
	editPWD->SetText(_T(""));
	editPWD->SetStyle(StyleType::Style, 0, ES_NOHIDESEL|ES_PASSWORD);
	editPWD->SetLocation(2, 23);
	editPWD->SetSize(100, 17);
	editPWD->SetVisible(false);
	this->AddControl(editPWD);

	btnPWD = new Button();
	btnPWD->SetName(_T("btnPWD"));
	btnPWD->SetLocation(104, 23);
	btnPWD->SetSize(60, 17);
	btnPWD->SetVisible(false);
	btnPWD->Click += EventHandler((Object*)this, (EventFun)&NoteForm::decrypt_Click);
	this->AddControl(btnPWD);

	this->DisposeItemAdd(contMenu);
	this->SetName(_T("Note"));
	this->SetText(App::GetName());
	this->SetConfigName(_T(""));
	this->SetContextMenu(contMenu);
	this->SetStyle(StyleType::Style, 0xFFFFFFFF, WS_VISIBLE|WS_CHILD|WS_THICKFRAME);
	this->SetStyle(StyleType::Style, WS_CAPTION, 0);
	this->SetStyle(StyleType::ExStyle, 0xFFFFFFFF, WS_EX_WINDOWEDGE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
	this->SetReBarStyle(ReBarStyle::Fix);
	this->SetMoveFormOnClick(true);
	this->SetLocation(100, 100);
	this->SetClientSize(600, 480);
	this->SetSize(230, 100);
	this->SetVisible(true);
	this->Closed += EventHandler((Object*)this, (EventFun)&NoteForm::Form_Closed);
	this->Deactivate += EventHandler((Object*)this, (EventFun)&NoteForm::Form_Deactivate);
	this->Load += EventHandler((Object*)this, (EventFun)&NoteForm::Form_Load);
	this->Move += EventHandler((Object*)this, (EventFun)&NoteForm::Form_SaveCfg);
	this->Resize += EventHandler((Object*)this, (EventFun)&NoteForm::Form_SaveCfg);
	::CopyRect(&rcNormal, &_Rect);

	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
	SetAlwaysOnTop(true);
}

LRESULT NoteForm::WndProc(EventArgs *e)
{
	switch(e->Msg)
	{
	case WM_DISPLAYCHANGE:
	case WM_ACTIVATEAPP:
	case WM_ACTIVATE:
		{
			AdjustLocation();
		}
		break;
	}
	return NoteBaseForm::WndProc(e);
}

void NoteForm::OnTimer(EventArgs* e)
{
	if(e->WParam == CC_TIMERID_SENDKEY)
	{
		::KillTimer(m_hWnd, CC_TIMERID_SENDKEY);
		int nStartChar;
		int nEndChar;
		editText->GetSel(nStartChar, nEndChar);
		if(nStartChar < 0 || nEndChar <= nStartChar)
		{
			this->SetVisible(true);
			isModal = false;
			return;
		}
		editText->Copy();
		cc::Str str = cc::Str(LangCode::UNICODE16);
		cc::Util::GetClipboardText(str);
		if(str.GetLength() < 1)
		{
			this->SetVisible(true);
			isModal = false;
			return;
		}

		//clock_t tmStart = ::clock();
		//while((int)(clock() - tmStart) < 800)
		//{
		//	App::DoEvents();
		//	Sleep(50);
		//}
		//
		HWND hWnd = ::GetForegroundWindow();
		if(hWnd != NULL)
		{
			DWORD id = 0;
			AttachThreadInput(GetCurrentThreadId(), GetWindowThreadProcessId(hWnd, &id), TRUE);

			HWND hGocus = ::GetFocus();
			if(hGocus != NULL)
			{
				//special for ie
				TCHAR buf[31];
				::GetClassName(hGocus, (LPTSTR)&buf, 30);
				if(_tcscmp(buf, _T("Shell DocObject View")) == 0)
				{
					HWND hwndtmp = ::GetNextWindow(hGocus, GW_CHILD);
					::GetClassName(hwndtmp, (LPTSTR)&buf, 30);
					if(_tcscmp(buf, _T("Internet Explorer_Server")) == 0)
					{
						::SetFocus(hwndtmp);
						hGocus = hwndtmp;
					}
				}

				const wchar_t* c = (wchar_t*)str.GetBuf();
				wchar_t ch;
				while(c != NULL && (ch = *c) != L'\0')
				{
					if(ch == 13 || ch == 10 || ch == 9)
					{
						SendMessage(hGocus, WM_IME_KEYDOWN, ch, 0);
					}
					else
					{
						SendMessage(hGocus, WM_IME_CHAR, ch, 0);
					}
					c++;
				}
				//SendMessage(hGocus, WM_PASTE, 0, 0);
			}
			//const TCHAR* tc = (TCHAR*)str.GetBuf();
			//TCHAR ch;
			//WORD MKey;
			//BYTE VKey;
			//while(tc != NULL && (ch = *tc) != '\0')
			//{
			//	tc++;
			//	MKey = ::VkKeyScan(ch);
			//	VKey = LOBYTE(MKey);
			//	// Get scancode
			//	BYTE ScanCode = LOBYTE(::MapVirtualKey(VKey, 0));
			//	keybd_event(VKey, ScanCode, 0, 0);
			//	keybd_event(VKey, ScanCode, KEYEVENTF_KEYUP, 0);
			//}
		}
		this->SetVisible(true);
		isModal = false;
		return;
	}
	else if(e->WParam == CC_TIMERID_SAVECFG)
	{
		::KillTimer(m_hWnd, CC_TIMERID_SAVECFG);
		SaveToEntity(NULL, CC_NOTE_CFG_NORMAL);
		return;
	}
	OnTimer(e);
}

void NoteForm::Form_SaveCfg(Object* sender, EventArgs* e)
{
	::KillTimer(m_hWnd, CC_TIMERID_SAVECFG);
	::SetTimer(m_hWnd, CC_TIMERID_SAVECFG, 1000, NULL);
}

void NoteForm::Form_Deactivate(Object* sender, EventArgs* e)
{
	if(!isModal && !toolattach->GetPushed() && !GetIsMin())
	{
		SetIsMin(true);
		if(toollock->GetPushed())
		{
			SetIsCrypt(true);
		}
	}
}

void NoteForm::Form_Load(Object* sender, EventArgs* e)
{
	NoteEntity* entity;
	if(nMode == MODE_TIME)
	{
		entity = mainForm->GetNoteWKTimeEntity();
	}
	else
	{
		entity = mainForm->GetNoteEntity(this);
	}
	LoadFromEntity(entity);
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CLOSE, false);

	ConfigBackDlg::ConfigResult(this->GetClient(), 0);
	reload_Click(NULL, NULL);
}

void NoteForm::SaveToFile()
{
	if(!editText->GetIsDirty())
	{
		return;
	}
	if(filePath.GetLength() > 0)
	{
		cc::Str buf = cc::Str(LangCode::TCHAR, editText->GetText());
		cc::Util::fWrite(buf, (TCHAR*)filePath.GetBuf());
		editText->SetIsDirty(false);
		if(nMode == MODE_NOTE)
		{
			mainForm->NoteChangeTitle(this, buf);
		}
	}
}

void NoteForm::Form_Closed(Object* sender, EventArgs* e)
{
	//is mode is 1(edit of worktime),megbox for save it if edited
	//if(editText->GetIsDirty() && (nMode == MODE_NOTE ||
	//	(IDYES == (int)::MessageBox(m_hWnd, CC_APP_GetLang()->Get(_T("Lang.U_NOTE_MSG_CloseSave")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Question")), MB_YESNO | MB_ICONQUESTION))))
	//{
	if(editText->GetIsDirty() && nMode == MODE_NOTE)
	{
		SaveToFile();
	}
	//}
	//SaveConfig(true);
	if(e != NULL && e->WParam == 1 && e->LParam == 1)
	{
		//save config as form closed(not show while next restart)
		SaveToEntity(NULL, CC_NOTE_CFG_CLOSED);
		mainForm->SaveAllNoteW(false);
	}
}

void NoteForm::SaveToEntity(NoteEntity* entity, int cfgType)
{
	if(entity == NULL) {
		if(nMode == MODE_TIME)
		{
			entity = mainForm->GetNoteWKTimeEntity();
		}
		else
		{
			entity = mainForm->GetNoteEntity(this);
		}
	}
	entity->nMode = nMode;
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_FIX, this->GetIsFix());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_MIN, this->GetIsMin());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CRYPT, this->GetIsCrypt());

	RECT rc = this->GetNormalBounds();
	::CopyRect(&entity->rcPos, &rc);
	::CopyRect(&rcNormal, &rc);

	//entity->tcFormType[0] = _T('n');
	entity->corText = editText->GetTextColor();
	if(editText->HasStyle(StyleType::Outer, OuterStyle::TransparentParentBrush))
	{
		entity->corBack = (COLORREF)-1;
	}
	else
	{
		entity->corBack = editText->GetBackground();
	}

	if(cfgType == CC_NOTE_CFG_CLOSED) {
		CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CLOSE, true);
	}
	if(cfgType == CC_NOTE_CFG_FONT)
	{
		HFONT hFont = editText->GetFont();
		if(hFont != NULL) {
			LOGFONT lf;
			memset(&lf, '\0', sizeof(LOGFONT));
			::GetObject(hFont, sizeof(lf), &lf);
			entity->nFontSize = lf.lfHeight;
			entity->nFontWeight = lf.lfWeight;
			entity->isFontItalic = lf.lfItalic;
			entity->isFontUnderline = lf.lfUnderline;
			entity->isFontStrikeOut = lf.lfStrikeOut;
			cc::Buf::Cpy(entity->tcFontName, 40, lf.lfFaceName);
			CC_BIT_SET(entity->dwFlag, NOTE_FLAG_HASFONT, true);
		}
		else {
			memset(entity->tcFontName, _T('\0'), 40);
		}
	}
}

void NoteForm::LoadFromEntity(NoteEntity* entity)
{
	this->SetBoundsNotOriginal(entity->rcPos);
	this->filePath = cc::Util::getStartupRelatePath(entity->tcFileName);
	if((int)entity->corText < 0)
	{
		return;
	}

	//if(entity->isCrypt)
	if(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_CRYPT))
	{
		this->SetIsCrypt(true, true);
	}
	//this->SetIsFix(entity->isFix);
	//this->SetIsMin(entity->isMin);
	this->SetIsFix(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_FIX));
	this->SetIsMin(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_MIN));
	editText->SetTextColor(entity->corText);
	if(entity->corBack == (COLORREF)-1)
	{
		editText->SetStyle(StyleType::Outer, 0, OuterStyle::TransparentParentBrush);
	}
	else
	{
		editText->SetBackground(entity->corBack);
		editText->SetStyle(StyleType::Outer, OuterStyle::TransparentParentBrush, 0);
		::InvalidateRect(editText->m_hWnd, NULL, TRUE);
	}

	if(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_HASFONT))
	{
		LOGFONT lf;
		memset(&lf, '\0', sizeof(LOGFONT));
		lf.lfHeight = entity->nFontSize;
		lf.lfWeight = entity->nFontWeight;
		lf.lfItalic = entity->isFontItalic;
		lf.lfUnderline = entity->isFontUnderline;
		lf.lfStrikeOut = entity->isFontStrikeOut;
		cc::Buf::Cpy(lf.lfFaceName, LF_FACESIZE, entity->tcFontName);

		HFONT hFont = ::CreateFontIndirect(&lf);
		editText->SetFont(hFont);
		::InvalidateRect(editText->m_hWnd, NULL, FALSE);
	}
}

void NoteForm::move_DoubleClick(Object* sender, EventArgs* e)
{
	bool isMin = this->GetIsMin();
	this->SetIsMin(!isMin);
	if(isMin)
	{
		editText->Focus();
	}
}

void NoteForm::close_Click(Object* sender, EventArgs* e)
{
	//notice thar wparam is 1 for saveconfig while Form_Closed
	this->SetTips(NULL);
	//SaveToEntity(NULL);
	::SendMessage(m_hWnd, WM_CLOSE, 1, 1);
}

void NoteForm::reload_Click(Object* sender, EventArgs* e)
{
	cc::Str sTxt(LangCode::TCHAR);
	if(cc::Util::fExist((TCHAR*)filePath.GetBuf()))
	{
		cc::Util::fRead(sTxt, (TCHAR*)filePath.GetBuf(), LangCode::TCHAR);
	}
	editText->SetText((TCHAR*)sTxt.GetBuf());
	editText->SetIsDirty(false);
	if(nMode == MODE_NOTE)
	{
		mainForm->NoteChangeTitle(this, sTxt);
	}
}

void NoteForm::clear_Click(Object* sender, EventArgs* e)
{
	editText->Clear();
}

void NoteForm::NewWin_Click(Object* sender, EventArgs* e)
{
	mainForm->NewNote();
}

void NoteForm::print_Click(Object* sender, EventArgs* e)
{
	isModal = true;

	//cc::Util::selectPrint(m_hWnd, CC_APP_GetInstance());
	cc::Str sHtml = cc::Str(LangCode::UNICODE16, _T("<html><head><title>"));
	sHtml.Append(filePath);
	sHtml.Append(_T("</title></head><body><pre>"));
	cc::Str str = cc::Str(editText->GetText());
	str.Replace(_T("<"), _T("&lt;"));
	sHtml.Append(str);
	sHtml.Append(_T("</pre></body></html>"));
	cc::win::WinUtil::PrintHtml(sHtml, this);

	//BSTR bstrCode = ::SysAllocString((wchar_t*)sHtml.GetBuf());
	//ie1->Write(bstrCode);
	//::SysFreeString(bstrCode);
	//ie1->GetWebBrowser()->ExecWB((ie_OLECMDID)7, (ie_OLECMDEXECOPT)1, NULL, NULL);

	isModal = false;
}

void NoteForm::font_Click(Object* sender, EventArgs* e)
{
	isModal = true;
	COLORREF corInit = editText->GetTextColor();
	HFONT hFont = cc::Util::selectFont(editText->GetFont(), corInit, 0, m_hWnd);
	if(hFont != NULL)
	{
		editText->SetFont(hFont);
		editText->SetTextColor(corInit);
		::InvalidateRect(editText->m_hWnd, NULL, FALSE);
		SaveToEntity(NULL, CC_NOTE_CFG_FONT);
	}
	isModal = false;
}

void NoteForm::bkColor_Click(Object* sender, EventArgs* e)
{
	isModal = true;
	int nRet = (int)::MessageBox(m_hWnd, CC_APP_GetLang()->Get(_T("Lang.U_NOTE_MSG_BKColor")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Question")), MB_YESNOCANCEL | MB_ICONQUESTION);
	if(nRet == IDCANCEL)
	{
		isModal = false;
		return;
	}
	if(nRet == IDNO)
	{
		isModal = false;
		editText->SetStyle(StyleType::Outer, 0, OuterStyle::TransparentParentBrush);
		return;
	}

	COLORREF corText = editText->GetBackground();
	if(cc::Util::selectColor(corText, true, NULL, 0, m_hWnd))
	{
		editText->SetBackground(corText);
		editText->SetStyle(StyleType::Outer, OuterStyle::TransparentParentBrush, 0);
		::InvalidateRect(editText->m_hWnd, NULL, TRUE);
		SaveToEntity(NULL, CC_NOTE_CFG_NORMAL);
	}
	isModal = false;
}

void NoteForm::savedefault_Click(Object* sender, EventArgs* e)
{
	SaveToEntity(mainForm->GetNoteDefaultEntity(), CC_NOTE_CFG_FONT);
	SaveToEntity(mainForm->GetNoteDefaultEntity(), CC_NOTE_CFG_NORMAL);
}

void NoteForm::save_Click(Object* sender, EventArgs* e)
{
	SaveToFile();
}

void NoteForm::key_Click(Object* sender, EventArgs* e)
{
	if(toollock->GetPushed())
	{
		return;
	}

	int nStartChar;
	int nEndChar;
	editText->GetSel(nStartChar, nEndChar);
	if(nStartChar < 0 || nEndChar <= nStartChar)
	{
		return;
	}
	isModal = true;
	this->SetVisible(false);
	::SetTimer(m_hWnd, CC_TIMERID_SENDKEY, 400, NULL);
}

void NoteForm::copy_Click(Object* sender, EventArgs* e)
{
	editText->Copy();
}

void NoteForm::paste_Click(Object* sender, EventArgs* e)
{
	editText->Paste();
}

void NoteForm::cut_Click(Object* sender, EventArgs* e)
{
	editText->Cut();
}

void NoteForm::lock_Click(Object* sender, EventArgs* e)
{
	if(toollock->GetPushed())
	{
		SetIsCrypt(true, true);
	}
	else
	{
		toollock->SetPushed(true);
		SetIsCrypt(false, true);
	}
	mainForm->SaveAllNoteW(false);
}

void NoteForm::decrypt_Click(Object* sender, EventArgs* e)
{
	SetIsCrypt(false, true);
}

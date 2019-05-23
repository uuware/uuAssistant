#include "Password.h"
#include "MainForm.h"
using namespace cc;
using namespace cc::win;

//===========================================================cc::win::PWDLoginDlg
///////////////////////////////////////////////////////////////////////////////
// constructors/destructor
PWDLoginDlg::PWDLoginDlg(const TCHAR* sPWD, bool isPWDMayEmpty, const TCHAR* sTitle)
{
	_tcPWD = NULL;
	CC_StrCopy(&_tcPWD, sPWD);
	_isPWDMayEmpty = isPWDMayEmpty;
	label1 = new Label();
	label1->SetText(CC_APP_GetLang()->Get("Lang.S_PWDLoginDlg_OldPWD"));
	label1->SetLocation(6, 5);
	label1->SetSize(90, 17);

	label2 = new Label();
	label2->SetText(CC_APP_GetLang()->Get("Lang.S_PWDLoginDlg_NewPWD1"));
	label2->SetLocation(6, 25);
	label2->SetSize(90, 17);

	label3 = new Label();
	label3->SetText(CC_APP_GetLang()->Get("Lang.S_PWDLoginDlg_NewPWD2"));
	label3->SetLocation(6, 45);
	label3->SetSize(90, 17);

	_editText1 = new TextBox();
	_editText1->SetName(_T("editText1"));
	_editText1->SetStyle(StyleType::Style, 0, ES_PASSWORD);
	_editText1->SetText(_T(""));
	_editText1->SetLocation(100, 4);
	_editText1->SetSize(120, 18);

	_editText2 = new TextBox();
	_editText2->SetName(_T("editText2"));
	_editText2->SetStyle(StyleType::Style, 0, ES_PASSWORD);
	_editText2->SetText(_T(""));
	_editText2->SetLocation(100, 24);
	_editText2->SetSize(120, 18);

	_editText3 = new TextBox();
	_editText3->SetName(_T("editText3"));
	_editText3->SetStyle(StyleType::Style, 0, ES_PASSWORD);
	_editText3->SetText(_T(""));
	_editText3->SetLocation(100, 44);
	_editText3->SetSize(120, 18);

	_btnOK = new Button();
	_btnOK->SetLocation(37, 65);
	_btnOK->SetSize(70, 19);
	_btnOK->Click += EventHandler((Object*)this, (EventFun)&PWDLoginDlg::ok_Click);
	_btnOK->SetText((TCHAR*)CC_APP_GetLang()->Get(_T("Lang.S_Dialog_OK")).GetBuf());

	_btnCancel = new Button();
	_btnCancel->SetLocation(109, 65);
	_btnCancel->SetSize(70, 19);
	_btnCancel->Click += EventHandler((Object*)this, (EventFun)&PWDLoginDlg::cancel_Click);
	_btnCancel->SetText((TCHAR*)CC_APP_GetLang()->Get(_T("Lang.S_Dialog_Cancel")).GetBuf());

	this->AddControl(label1);
	this->AddControl(label2);
	this->AddControl(label3);
	this->AddControl(_editText1);
	this->AddControl(_editText2);
	this->AddControl(_editText3);
	this->AddControl(_btnOK);
	this->AddControl(_btnCancel);
	if(sTitle == NULL)
	{
		this->SetText((TCHAR*)CC_APP_GetLang()->Get(_T("Lang.S_PWDLoginDlg_Title")).GetBuf());
	}
	else
	{
		this->SetText(sTitle);
	}
	this->SetSize(232, 117);
#ifdef _DEBUG_
	this->SetName(_T("PWDLoginDlg"));
#endif
}

PWDLoginDlg::~PWDLoginDlg()
{
}
// constructors/destructor
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// private/protected/internal used methods
void PWDLoginDlg::ok_Click(Object* sender, EventArgs* e)
{
	if(OKClick.IsEmpty())
	{
		cc::Str p0 = cc::Str(_tcPWD).Trim();
		cc::Str p1 = cc::Str(_editText1->GetText()).Trim();
		cc::Str p2 = cc::Str(_editText2->GetText()).Trim();
		cc::Str p3 = cc::Str(_editText3->GetText()).Trim();
		if(p2 != p3)
		{
			::MessageBox(m_hWnd, CC_APP_GetLang()->GetFormat(_T("Lang.S_PWDLoginDlg_NewPWD_NotSame")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Critical")), MB_OK);
			return;
		}
		if(!_isPWDMayEmpty && p2.GetLength() < 1)
		{
			::MessageBox(m_hWnd, CC_APP_GetLang()->GetFormat(_T("Lang.S_PWDLoginDlg_NewPWD_IsEmpty")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Critical")), MB_OK);
			return;
		}
		if(p0 == p1 || (p0.GetLength() == 0 && p1.GetLength() == 0))
		{
		}
		else
		{
			::MessageBox(m_hWnd, CC_APP_GetLang()->GetFormat(_T("Lang.S_PWDLoginDlg_OldPWD_NotSame")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Critical")), MB_OK);
			return;
		}
		EndDialog(DialogResult::OK);
	}
	else
	{
		e->Handled = true;
		OKClick.Invoke((Object*)this, e);
		if(e->Handled)
		{
			EndDialog(DialogResult::OK);
		}
	}
}

void PWDLoginDlg::cancel_Click(Object* sender, EventArgs* e)
{
	e->Handled = true;
	CancelClick.Invoke((Object*)this, e);
	if(e->Handled)
	{
		EndDialog(DialogResult::Cancel);
	}
}
// private/protected/internal used methods
///////////////////////////////////////////////////////////////////////////////

Password::Password()
{
	InitializeComponent();
}

void Password::InitializeComponent()
{
	mainForm = (MainForm*)CC_APP_GetVarious(_T("MainForm"));
	imageList = (ImageList*)CC_APP_GetVarious(_T("ImageList"));
	isMin = false;
	isModal = false;
	nMode = MODE_PASS;
	toollock = NULL;//not use
	this->isBlock = true;
	hWndIE = NULL;
	memset(tcBuf, 0, MAX_PATH);

	//context menu
	contMenu = new ContextMenu();
	contMenu->SetName(_T("contMenu"));
	contMenu->SetAllOwnerDraw(true);
	contMenu->Popup += EventHandler((Object*)this, (EventFun)&Password::menuFavorites_Popup);

	menuClose = new MenuItem();
	menuClose->SetName(_T("menuClose"));
	menuClose->Click += EventHandler((Object*)this, (EventFun)&Password::close_Click);
	menuClose->SetIcon(imageList, IDI_ICON_EXIT);
	menuClose->SetShortcut(Shortcut::CtrlQ);
	contMenu->ItemAdd(menuClose);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuNewWin = new MenuItem();
	menuNewWin->SetName(_T("menuNewIE"));
	menuNewWin->Click += EventHandler((Object*)this, (EventFun)&Password::NewWin_Click);
	menuNewWin->SetIcon(imageList, IDI_ICON_NEWWIN);
	menuNewWin->SetShortcut(Shortcut::CtrlN);
	contMenu->ItemAdd(menuNewWin);

	menuFavorites = new MenuItem();
	menuFavorites->SetName(_T("menuFavorites"));
	//menuFavorites->Popup += EventHandler((Object*)this, (EventFun)&Password::menuFavorites_Popup);
	//menuFavorites->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuFavorites);

	menuFavAdd = new MenuItem();
	menuFavAdd->SetName(_T("menuFavAdd"));
	menuFavAdd->Click += EventHandler((Object*)this, (EventFun)&Password::menuFavorites_Click);
	//menuFavAdd->SetIcon(imageList, IDI_ICON_REFRESH);
	menuFavorites->ItemAdd(menuFavAdd);

	menuFavMan = new MenuItem();
	menuFavMan->SetName(_T("menuFavMan"));
	menuFavMan->Click += EventHandler((Object*)this, (EventFun)&Password::menuFavorites_Click);
	//menuFavMan->SetIcon(imageList, IDI_ICON_REFRESH);
	menuFavorites->ItemAdd(menuFavMan);

	menuBreak2 = new MenuItem();
	menuBreak2->SetText(_T("-"));
	menuFavorites->ItemAdd(menuBreak2);

	menuOpenFile = new MenuItem();
	menuOpenFile->SetName(_T("menuOpenFile"));
	menuOpenFile->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	//menuOpenFile->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuOpenFile);

	//menuSource = new MenuItem();
	//menuSource->SetName(_T("menuSource"));
	//menuSource->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	////menuSource->SetIcon(imageList, IDI_ICON_REFRESH);
	//contMenu->ItemAdd(menuSource);

	menuReload = new MenuItem();
	menuReload->SetName(_T("menuReload"));
	menuReload->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	menuReload->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuReload);

	menuBreak2 = new MenuItem();
	menuBreak2->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak2);

	menuPrint = new MenuItem();
	menuPrint->SetName(_T("menuPrint"));
	menuPrint->Click += EventHandler((Object*)this, (EventFun)&Password::print_Click);
	menuPrint->SetIcon(imageList, IDI_ICON_PRINT);
	menuPrint->SetShortcut(Shortcut::CtrlP);
	contMenu->ItemAdd(menuPrint);

	menuSaveAs = new MenuItem();
	menuSaveAs->SetName(_T("menuSaveAs"));
	menuSaveAs->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	//menuSaveAs->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveAs);

	menuSaveURL = new MenuItem();
	menuSaveURL->SetName(_T("menuSaveURL"));
	menuSaveURL->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	//menuSaveURL->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveURL);

	menuIsBlock = new MenuItem();
	menuIsBlock->SetName(_T("menuIsBlock"));
	menuIsBlock->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	menuIsBlock->SetChecked(this->isBlock);
	//menuIsBlock->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuIsBlock);

	menuOption = new MenuItem();
	menuOption->SetName(_T("menuOption"));
	menuOption->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	//menuOption->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuOption);

	menuSaveSize = new MenuItem();
	menuSaveSize->SetName(_T("menuSaveSize"));
	menuSaveSize->Click += EventHandler((Object*)this, (EventFun)&Password::savedefault_Click);
	menuSaveSize->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveSize);

	moveForm = new ToolItem();
	moveForm->SetName(_T("moveForm"));
	//moveForm->SetMoveForm(this);
	moveForm->SetLocation(-1, 0);
	moveForm->SetSize(20, 18);
	moveForm->SetText(NULL);
	moveForm->SetIcon(imageList, IDI_ICON_IE);
	moveForm->SetStyle(StyleType::Outer, 0, OuterStyle::AlwaysNoEdge);
	moveForm->DoubleClick += EventHandler((Object*)this, (EventFun)&Password::move_DoubleClick);
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
	toolexit->Click += EventHandler((Object*)this, (EventFun)&Password::close_Click);
	toolbar->AddControl(toolexit);

	toolbreak1 = new ToolItem();
	toolbreak1->SetBreak(true);
	toolbar->AddControl(toolbreak1);

	toolattach = new ToolItem();
	toolattach->SetName(_T("toolAttach"));
	toolattach->SetMargin(2, 1, 2, 1);
	toolattach->SetIcon(imageList, IDI_ICON_ATTACH);
	toolattach->SetIsPushButton(true);
	toolattach->SetPushed(true);
	toolbar->AddControl(toolattach);

	toollock = new ToolItem();
	toollock->SetName(_T("toolLock"));
	toollock->SetMargin(2, 1, 2, 1);
	toollock->SetIcon(imageList, IDI_ICON_LOCK);
	toollock->SetIsPushButton(true);
	toollock->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	toolbar->AddControl(toollock);

	toolrefresh = new ToolItem();
	toolrefresh->SetName(_T("toolrefresh"));
	toolrefresh->SetMargin(2, 1, 2, 1);
	toolrefresh->SetIcon(imageList, IDI_ICON_REFRESH);
	toolrefresh->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	toolbar->AddControl(toolrefresh);

	toolstop = new ToolItem();
	toolstop->SetName(_T("toolstop"));
	toolstop->SetMargin(2, 1, 2, 1);
	toolstop->SetIcon(imageList, IDI_ICON_STOP);
	toolstop->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	toolbar->AddControl(toolstop);

	//126 - 148
	editURL = new TextBox();
	editURL->SetName(_T("editURL"));
	editURL->SetText(_T("about:blank"));
	editURL->SetStyle(StyleType::Style, 0, ES_NOHIDESEL|WS_TABSTOP);
	editURL->SetLocation(148, 3);
	editURL->SetSize(600 - 148 - 21, 17);
	editURL->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
	editURL->KeyDown += KeyEventHandler((Object*)this, (KeyEventFun)&Password::editURL_KeyDown);
	this->AddControl(editURL);

	toolgo = new ToolItem();
	toolgo->SetName(_T("toolgo"));
	toolgo->SetLocation(600 - 20, 2);
	//toolgo->SetMargin(2, 1, 2, 1);
	toolgo->SetSize(16, 18);
	toolgo->SetIcon(imageList, IDI_ICON_GO);
	toolgo->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	toolgo->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	toolgo->MinPosition.x = 148 + 2;
	this->AddControl(toolgo);

	//ctlMain = ie;
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
	btnPWD->Click += EventHandler((Object*)this, (EventFun)&Password::tool_Click);
	this->AddControl(btnPWD);

	this->DisposeItemAdd(contMenu);
	this->SetName(_T("MiniIE"));
	this->SetText(App::GetName());
	this->SetConfigName(_T(""));
	this->SetContextMenu(contMenu);
	this->SetStyle(StyleType::Style, 0xFFFFFFFF, WS_VISIBLE|WS_CHILD|WS_THICKFRAME);
	this->SetStyle(StyleType::Style, WS_CAPTION, 0);
	this->SetStyle(StyleType::Style, 0xFFFFFFFF, WS_EX_WINDOWEDGE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
	this->SetReBarStyle(ReBarStyle::Fix);
	this->SetMoveFormOnClick(true);
	this->SetLocation(100, 100);
	this->SetClientSize(600, 480);
	this->SetSize(230, 100);
	this->SetVisible(true);
	this->Closed += EventHandler((Object*)this, (EventFun)&Password::Form_Closed);
	this->Deactivate += EventHandler((Object*)this, (EventFun)&Password::Form_Deactivate);
	this->Load += EventHandler((Object*)this, (EventFun)&Password::Form_Load);
	::CopyRect(&rcNormal, &_Rect);

	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
}

void Password::OnResize(EventArgs* e)
{
	NoteBaseForm::OnResize(e);
	toolgo->SetVisible(false);
	toolgo->SetVisible(true);
}

void Password::Form_Deactivate(Object* sender, EventArgs* e)
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

void Password::Form_Load(Object* sender, EventArgs* e)
{
	//NoteEntity* entity;
	//entity = mainForm->GetNoteEntity(this);
	//LoadFromEntity(entity);
}

void Password::Form_Closed(Object* sender, EventArgs* e)
{
	SaveConfig(true);
}

void Password::SaveConfig(bool isExit)
{
	if(!isExit)
	{
	//	mainForm->SaveAllNoteW(false, false);
	}
	else
	{
	//	NoteEntity* entity = mainForm->GetNoteEntity(this);
	//	SaveToEntity(entity);
		//entity->isClose = true;
	//	CC_BITON(entity->dwFlag, NOTE_FLAG_CLOSE);
	}
}

void Password::SaveToEntity(NoteEntity* entity)
{
	//entity->isFix = this->GetIsFix();
	//entity->isMin = this->GetIsMin();
	//entity->isCrypt = this->GetIsCrypt();
	//entity->isFontItalic = this->GetIsBlock();
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_FIX, this->GetIsFix());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_MIN, this->GetIsMin());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CRYPT, this->GetIsCrypt());
	entity->nMode = nMode;
	memset(entity->tcURL, _T('\0'), IE_URL_LEN);
	if(sSaveUrl.GetLength() > 0)
	{
		//cc::Str url = cc::Str(cc::LangCode::TCHAR, sSaveUrl, 0, IE_URL_LEN);
		//_tcsncpy(entity->tcURL, (TCHAR*)sSaveUrl.GetBuf(), IE_URL_LEN);
		cc::Buf::Mov(entity->tcURL, IE_URL_LEN, (TCHAR*)sSaveUrl.GetBuf(), sSaveUrl.GetLength());
	}

	RECT rc = this->GetNormalBounds();
	::CopyRect(&entity->rcPos, &rc);
	::CopyRect(&rcNormal, &rc);
}

void Password::LoadFromEntity(NoteEntity* entity)
{
	this->SetBoundsNotOriginal(entity->rcPos);
	if(entity->tcURL[0] == _T('\0') || _tcscmp(_T(""), entity->tcURL) == 0)
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
}

void Password::move_DoubleClick(Object* sender, EventArgs* e)
{
	this->SetIsMin(!this->GetIsMin());
}

void Password::close_Click(Object* sender, EventArgs* e)
{
	//notice thar wparam is 1 for saveconfig while Form_Closed
	this->SetTips(NULL);
	::SendMessage(m_hWnd, WM_CLOSE, 1, NULL);
}

void Password::NewWin_Click(Object* sender, EventArgs* e)
{
	mainForm->NewMiniIE();
}

void Password::print_Click(Object* sender, EventArgs* e)
{
}

void Password::savedefault_Click(Object* sender, EventArgs* e)
{
	NoteEntity* entity = mainForm->GetNoteDefaultEntity();
	RECT rc = this->GetNormalBounds();
	::CopyRect(&entity->rcPos, &rc);
	::CopyRect(&rcNormal, &rc);
}

void Password::editURL_KeyDown(Object* sender, KeyEventArgs* e)
{
	if(e != NULL && e->baseArgs->WParam == VK_RETURN)
	{
		tool_Click(toolgo, e->baseArgs);
	}
}

void Password::Favorites2Menu(MenuItem* item, cc::Str path)
{
#ifdef _DEBUG_
//	cc::Log::debug(_T("Favorites2Menu:%s"), (TCHAR*)path.GetBuf());
#endif
	if(!path.EndsWith(_T("\\")))
	{
		path.Append(_T("\\"));
	}
	//load all text file from dir of note
	cc::List<cc::Str> arrFullName;
	cc::Str filefilter = path + _T("*.url");
	path.Append(_T("*.*"));
	int nCnt = cc::Util::getDirs(arrFullName, (TCHAR*)path.GetBuf(), false);
	for(int i = 0; i < nCnt; i++)
	{
		MenuItem* subitem = new MenuItem();
		cc::Str nameonly = cc::Util::getLastSubDirName((const TCHAR*)arrFullName.GetKey(i).GetBuf());
		subitem->SetText((const TCHAR*)nameonly.GetBuf());
		subitem->SetIcon(imageList, IDI_ICON_FOLDER);
		item->ItemAdd(subitem);
		Favorites2Menu(subitem, arrFullName.GetKey(i));
	}
	arrFullName.Clear();
	nCnt = cc::Util::getFiles(arrFullName, (TCHAR*)filefilter.GetBuf(), false);
	for(int i = 0; i < nCnt; i++)
	{
		MenuItem* subitem = new MenuItem();
		cc::Str nameonly = cc::Util::getFileNameNoExt((const TCHAR*)arrFullName.GetKey(i).GetBuf());
		subitem->SetText((const TCHAR*)nameonly.GetBuf());

		memset(tcBuf, 0, MAX_PATH);
		// an .URL file is formatted just like an .INI file, so we can use GetPrivateProfileString() to get the information we want
 		::GetPrivateProfileString(_T("InternetShortcut"), _T("URL"), _T(""), tcBuf, MAX_PATH, (const TCHAR*)arrFullName.GetKey(i).GetBuf());
		subitem->SetTips(tcBuf);

		subitem->Click += EventHandler((Object*)this, (EventFun)&Password::menuFavorites_Click);
		subitem->SetIcon(imageList, IDI_ICON_PAGE);
		item->ItemAdd(subitem);
	}
}

void Password::menuFavorites_Popup(Object* sender, EventArgs* e)
{
#ifdef _DEBUG_
//	cc::Log::debug(_T("menuFavorites_Popup"));
#endif
	int nCnt = menuFavorites->ItemCount();
	if(nCnt > 3)
	{
		for(int i = nCnt - 1; i >= 3; i--)
		{
			//delete menuFavorites->ItemGet(i);
			Menu* menu = menuFavorites->ItemRemove(i);
			delete menu;
		}
	}

	memset(tcBuf, 0, MAX_PATH);
	if(::SHGetSpecialFolderPath(m_hWnd, tcBuf, CSIDL_FAVORITES, FALSE))
	{
		Favorites2Menu(menuFavorites, cc::Str(tcBuf));
	}
}
void Password::menuFavorites_Click(Object* sender, EventArgs* e)
{
}

void Password::tool_Click(Object* sender, EventArgs* e)
{
	if(sender == toolrefresh || sender == menuReload)
	{
	}
	if(sender == toolstop)
	{
	}
	if(sender == menuOpenFile)
	{
	}
	if(sender == menuSaveAs)
	{
		isModal = true;
		isModal = false;
	}
	else if(sender == toollock)
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
	}
	else if(sender == btnPWD)
	{
		SetIsCrypt(false);
	}
	else if(sender == menuIsBlock)
	{
		this->isBlock = !this->isBlock;
		menuIsBlock->SetChecked(this->isBlock);
	}
}

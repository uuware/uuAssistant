#include "MiniIEForm.h"
#include "MainForm.h"
using namespace cc;
using namespace cc::win;

// Map(bool isSort, bool bIsDESC = false, bool isDistinct = false, int nMinCapacity = 0, double nIncrement = 1.3);
Map<HWND, Control*>* mapHWND = NULL;
HHOOK gMsgHook = NULL;
LRESULT CALLBACK WatchMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code >= 0 && mapHWND != NULL)
	{
		int nIndex;
		MSG* data = (MSG*)lParam;
		if(data->message == WM_LBUTTONDOWN || data->message == WM_KEYDOWN)
		{
			if((nIndex = mapHWND->GetIndex(data->hwnd)) >= 0)
			{
				MiniIEForm* ieForm = (MiniIEForm*)mapHWND->GetAt(nIndex);
				ieForm->UpdateIEEvent();
				::SetWindowPos(ieForm->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
		}
	}
	return ::CallNextHookEx(gMsgHook, code, wParam, lParam);
}
long StartMsgHook(HINSTANCE hInstance)
{
	gMsgHook = SetWindowsHookEx(WH_GETMESSAGE, WatchMsgProc, hInstance, ::GetCurrentThreadId());
	//cc::Log::debug(_T("StartMsgHook:%d"), gMsgHook);
	if(gMsgHook == NULL)
	{
		return -1;
	}
	return 0;
}
long StopMsgHook()
{
	//cc::Log::debug(_T("StopMsgHook:%d"), gMsgHook);
	if(gMsgHook != NULL)
	{
		::UnhookWindowsHookEx(gMsgHook);
		gMsgHook = NULL;
	}
	return 0;
}

void miniIE_IEEvent::NewWindow(LPDISPATCH* ppDisp, VARIANT_BOOL* Cancel)
{
	if(!miniIE->GetIsBlock() || miniIE->IsIEEvent())
	{
		MiniIEForm* ieForm = mainForm->NewMiniIE();
		if(ieForm != NULL)
		{
			IEControl* ie = ieForm->GetIEControl();
			*ppDisp = ie->GetWebBrowser();
		}
	}
	else
	{
		*Cancel = TRUE;
	}
}
void miniIE_IEEvent::BeforeNavigate2(const BSTR url, IDispatch *ppDisp, short *cancel)
{
	if(webBrower == NULL)
	{
		webBrower = miniIE->GetIEControl()->GetWebBrowser();
	}
	if(ppDisp == webBrower)
	{
		cc::Str url2 = cc::Str(cc::LangCode::TCHAR, url);
		if(url2.IndexOf(_T("script:"), true) < 0)
		{
			editURL->SetText((TCHAR*)url2.GetBuf());
		}
	}
}
void miniIE_IEEvent::DocumentComplete(const BSTR url)
{
	miniIE->GetIEHwnd();
}

MiniIEForm::MiniIEForm()
{
	InitializeComponent();
}

//destructor
MiniIEForm::~MiniIEForm()
{
	if(mapHWND != NULL)
	{
		delete mapHWND;
		mapHWND = NULL;
	}
}

void MiniIEForm::InitializeComponent()
{
	mainForm = (MainForm*)CC_APP_GetVarious(_T("MainForm"));
	imageList = (ImageList*)CC_APP_GetVarious(_T("ImageList"));
	isMin = false;
	isModal = false;
	nMode = MODE_IE;
	toollock = NULL;//not use
	this->isBlock = true;
	hWndIE = NULL;
	memset(tcBuf, 0, MAX_PATH);

	//context menu
	contMenu = new ContextMenu();
	contMenu->SetName(_T("contMenu"));
	contMenu->SetAllOwnerDraw(true);
	contMenu->Popup += EventHandler((Object*)this, (EventFun)&MiniIEForm::menuFavorites_Popup);

	menuClose = new MenuItem();
	menuClose->SetName(_T("menuClose"));
	menuClose->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::close_Click);
	menuClose->SetIcon(imageList, IDI_ICON_EXIT);
	menuClose->SetShortcut(Shortcut::CtrlQ);
	contMenu->ItemAdd(menuClose);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuNewWin = new MenuItem();
	menuNewWin->SetName(_T("menuNewIE"));
	menuNewWin->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::NewWin_Click);
	menuNewWin->SetIcon(imageList, IDI_ICON_NEWWIN);
	menuNewWin->SetShortcut(Shortcut::CtrlN);
	contMenu->ItemAdd(menuNewWin);

	menuFavorites = new MenuItem();
	menuFavorites->SetName(_T("menuFavorites"));
	//menuFavorites->Popup += EventHandler((Object*)this, (EventFun)&MiniIEForm::menuFavorites_Popup);
	//menuFavorites->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuFavorites);

	menuFavAdd = new MenuItem();
	menuFavAdd->SetName(_T("menuFavAdd"));
	menuFavAdd->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::menuFavorites_Click);
	//menuFavAdd->SetIcon(imageList, IDI_ICON_REFRESH);
	menuFavorites->ItemAdd(menuFavAdd);

	menuFavMan = new MenuItem();
	menuFavMan->SetName(_T("menuFavMan"));
	menuFavMan->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::menuFavorites_Click);
	//menuFavMan->SetIcon(imageList, IDI_ICON_REFRESH);
	menuFavorites->ItemAdd(menuFavMan);

	menuBreak2 = new MenuItem();
	menuBreak2->SetText(_T("-"));
	menuFavorites->ItemAdd(menuBreak2);

	menuOpenFile = new MenuItem();
	menuOpenFile->SetName(_T("menuOpenFile"));
	menuOpenFile->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	//menuOpenFile->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuOpenFile);

	//menuSource = new MenuItem();
	//menuSource->SetName(_T("menuSource"));
	//menuSource->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	////menuSource->SetIcon(imageList, IDI_ICON_REFRESH);
	//contMenu->ItemAdd(menuSource);

	menuReload = new MenuItem();
	menuReload->SetName(_T("menuReload"));
	menuReload->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	menuReload->SetIcon(imageList, IDI_ICON_REFRESH);
	contMenu->ItemAdd(menuReload);

	menuBreak2 = new MenuItem();
	menuBreak2->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak2);

	menuPrint = new MenuItem();
	menuPrint->SetName(_T("menuPrint"));
	menuPrint->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::print_Click);
	menuPrint->SetIcon(imageList, IDI_ICON_PRINT);
	menuPrint->SetShortcut(Shortcut::CtrlP);
	contMenu->ItemAdd(menuPrint);

	menuSaveAs = new MenuItem();
	menuSaveAs->SetName(_T("menuSaveAs"));
	menuSaveAs->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	//menuSaveAs->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveAs);

	menuSaveURL = new MenuItem();
	menuSaveURL->SetName(_T("menuSaveURL"));
	menuSaveURL->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	//menuSaveURL->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveURL);

	menuIsBlock = new MenuItem();
	menuIsBlock->SetName(_T("menuIsBlock"));
	menuIsBlock->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	menuIsBlock->SetChecked(this->isBlock);
	//menuIsBlock->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuIsBlock);

	menuOption = new MenuItem();
	menuOption->SetName(_T("menuOption"));
	menuOption->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	//menuOption->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuOption);

	menuSaveSize = new MenuItem();
	menuSaveSize->SetName(_T("menuSaveSize"));
	menuSaveSize->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::savedefault_Click);
	menuSaveSize->SetIcon(imageList, IDI_ICON_OPTION);
	contMenu->ItemAdd(menuSaveSize);

	moveForm = new ToolItem();
	moveForm->SetName(_T("moveForm"));
	//moveForm->SetMoveForm(this);
	moveForm->SetLocation(-1, 0);
	moveForm->SetSize(20, 18);
	moveForm->SetText(NULL);
	moveForm->SetStyle(StyleType::Outer, 0, OuterStyle::AlwaysNoEdge);
	moveForm->SetIcon(imageList, IDI_ICON_IE);
	moveForm->DoubleClick += EventHandler((Object*)this, (EventFun)&MiniIEForm::move_DoubleClick);
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
	toolexit->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::close_Click);
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
	toollock->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	toolbar->AddControl(toollock);

	toolrefresh = new ToolItem();
	toolrefresh->SetName(_T("toolrefresh"));
	toolrefresh->SetMargin(2, 1, 2, 1);
	toolrefresh->SetIcon(imageList, IDI_ICON_REFRESH);
	toolrefresh->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	toolbar->AddControl(toolrefresh);

	toolstop = new ToolItem();
	toolstop->SetName(_T("toolstop"));
	toolstop->SetMargin(2, 1, 2, 1);
	toolstop->SetIcon(imageList, IDI_ICON_STOP);
	toolstop->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	toolbar->AddControl(toolstop);

	//126 - 148
	editURL = new TextBox();
	editURL->SetName(_T("editURL"));
	editURL->SetText(_T("about:blank"));
	editURL->SetStyle(StyleType::Style, 0, ES_NOHIDESEL|WS_TABSTOP);
	editURL->SetLocation(148, 3);
	editURL->SetSize(600 - 148 - 21, 17);
	editURL->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
	editURL->KeyDown += KeyEventHandler((Object*)this, (KeyEventFun)&MiniIEForm::editURL_KeyDown);
	this->AddControl(editURL);

	toolgo = new ToolItem();
	toolgo->SetName(_T("toolgo"));
	toolgo->SetLocation(600 - 20, 2);
	//toolgo->SetMargin(2, 1, 2, 1);
	toolgo->SetSize(16, 18);
	toolgo->SetIcon(imageList, IDI_ICON_GO);
	toolgo->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	toolgo->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	toolgo->MinPosition.x = 148 + 2;
	this->AddControl(toolgo);

	ie = new IEControl();
	ie->SetName(_T("ie"));
	ie->SetLocation(0, 23);
	ie->SetSize(600, 480 - 23);
	ie->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	this->AddControl(ie);
	ctlMain = ie;

	//ie_DocHandler* docHandler; //be deleted at ie
	//docHandler = new ie_DocHandler();
	//ie->SetOleObject(docHandler, NULL, NULL, NULL);
	miniIE = new miniIE_IEEvent();
	miniIE->SetParent(mainForm, this, moveForm, editURL);
	ie->SetEventHandler(miniIE); //be deleted at ie

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
	btnPWD->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::tool_Click);
	this->AddControl(btnPWD);

	this->DisposeItemAdd(contMenu);
	this->SetName(_T("MiniIE"));
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
	this->Closed += EventHandler((Object*)this, (EventFun)&MiniIEForm::Form_Closed);
	this->Deactivate += EventHandler((Object*)this, (EventFun)&MiniIEForm::Form_Deactivate);
	this->Load += EventHandler((Object*)this, (EventFun)&MiniIEForm::Form_Load);
	::CopyRect(&rcNormal, &_Rect);

	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
	SetAlwaysOnTop(true);
}

LRESULT MiniIEForm::WndProc(EventArgs *e)
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

void MiniIEForm::OnResize(EventArgs* e)
{
	NoteBaseForm::OnResize(e);
	toolgo->SetVisible(false);
	toolgo->SetVisible(true);
}

void MiniIEForm::Form_Deactivate(Object* sender, EventArgs* e)
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

void MiniIEForm::Form_Load(Object* sender, EventArgs* e)
{
	NoteEntity* entity;
	entity = mainForm->GetNoteEntity(this);
	LoadFromEntity(entity);
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CLOSE, false);
}

void MiniIEForm::Form_Closed(Object* sender, EventArgs* e)
{
	if(mapHWND != NULL)
	{
		if(mapHWND->GetSize() == 1)
		{
			StopMsgHook();
		}
		mapHWND->Remove(hWndIE);
	}
	//if(sSaveUrl.GetLength() < 1 || sSaveUrl.Equals(_T("about:blank"), true))
	//{
	//	mainForm->RemoveBlankIE(this);
	//}
	if(e != NULL && e->WParam == 1 && e->LParam == 1)
	{
		//save config as form closed(not show while next restart)
		SaveToEntity(NULL, CC_NOTE_CFG_CLOSED);
		mainForm->SaveAllNoteW(false);
	}
}

void MiniIEForm::SaveToEntity(NoteEntity* entity, int cfgType)
{
	if(entity == NULL)
	{
		entity = mainForm->GetNoteEntity(this);
	}
	entity->nMode = nMode;
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_FIX, this->GetIsFix());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_MIN, this->GetIsMin());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CRYPT, this->GetIsCrypt());
	CC_BIT_SET(entity->dwFlag, NOTE_FLAG_BLOCK, this->GetIsBlock());

	RECT rc = this->GetNormalBounds();
	::CopyRect(&entity->rcPos, &rc);
	::CopyRect(&rcNormal, &rc);

	if(cfgType == CC_NOTE_CFG_CLOSED) {
		CC_BIT_SET(entity->dwFlag, NOTE_FLAG_CLOSE, true);
	}
	if(cfgType == CC_NOTE_CFG_URL) {
		memset(entity->tcURL, _T('\0'), IE_URL_LEN);
		if(sSaveUrl.GetLength() > 0)
		{
			//cc::Str url = cc::Str(cc::LangCode::TCHAR, sSaveUrl, 0, IE_URL_LEN);
			//_tcsncpy(entity->tcURL, (TCHAR*)sSaveUrl.GetBuf(), IE_URL_LEN);
			cc::Buf::Mov(entity->tcURL, IE_URL_LEN, (TCHAR*)sSaveUrl.GetBuf(), sSaveUrl.GetLength());
		}
	}
}

void MiniIEForm::LoadFromEntity(NoteEntity* entity)
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
	//this->SetIsBlock(entity->isFontItalic);
	this->SetIsBlock(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_BLOCK));
	//sSaveUrl = cc::Str(cc::LangCode::UNICODE16, entity->tcURL);
	sSaveUrl = cc::Str(entity->tcURL);
	editURL->SetText(entity->tcURL);
	cc::Str url = cc::Str(cc::LangCode::UNICODE16, entity->tcURL);
	ie->Navigate((wchar_t*)url.GetBuf());
}

void MiniIEForm::move_DoubleClick(Object* sender, EventArgs* e)
{
	this->SetIsMin(!this->GetIsMin());
}

void MiniIEForm::close_Click(Object* sender, EventArgs* e)
{
	//notice thar wparam is 1 for saveconfig while Form_Closed
	this->SetTips(NULL);
	::SendMessage(m_hWnd, WM_CLOSE, 1, 1);
}

void MiniIEForm::NewWin_Click(Object* sender, EventArgs* e)
{
	mainForm->NewMiniIE();
}

void MiniIEForm::print_Click(Object* sender, EventArgs* e)
{
	ie->GetWebBrowser()->ExecWB((ie_OLECMDID)7, (ie_OLECMDEXECOPT)1, NULL, NULL);
}

void MiniIEForm::savedefault_Click(Object* sender, EventArgs* e)
{
	NoteEntity* entity = mainForm->GetNoteDefaultEntity();
	this->SetIsBlock(CC_BIT_HAS(entity->dwFlag, NOTE_FLAG_BLOCK));
	RECT rc = this->GetNormalBounds();
	::CopyRect(&entity->rcPos, &rc);
	::CopyRect(&rcNormal, &rc);
}

void MiniIEForm::editURL_KeyDown(Object* sender, KeyEventArgs* e)
{
	if(e != NULL && e->baseArgs->WParam == VK_RETURN)
	{
		tool_Click(toolgo, e->baseArgs);
	}
}

void MiniIEForm::Favorites2Menu(MenuItem* item, cc::Str path)
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

		subitem->Click += EventHandler((Object*)this, (EventFun)&MiniIEForm::menuFavorites_Click);
		subitem->SetIcon(imageList, IDI_ICON_PAGE);
		item->ItemAdd(subitem);
	}
}

void MiniIEForm::menuFavorites_Popup(Object* sender, EventArgs* e)
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
void MiniIEForm::menuFavorites_Click(Object* sender, EventArgs* e)
{
	if(sender == menuFavAdd)
	{
		#define ID_IE_ID_ADDFAV 2261
		GetIEHwnd();
		::SendMessage(hWndIE, WM_COMMAND, MAKEWPARAM(LOWORD(ID_IE_ID_ADDFAV), 0x0), 0);
	}
	else if(sender == menuFavMan)
	{
		memset(tcBuf, 0, MAX_PATH);
		if(!::SHGetSpecialFolderPathA(m_hWnd, (char*)tcBuf, CSIDL_FAVORITES, TRUE))
		{
			MessageBox(m_hWnd, _T("Error SHGetSpecialFolderPath of CSIDL_FAVORITES."), NULL, MB_ICONERROR);
			return;
		}

		typedef UINT (CALLBACK* LPFNORGFAV)(HWND, char*);
		bool bIsLoadDll = false;
		HMODULE hMod = ::GetModuleHandle(_T("shdocvw.dll"));
		if(hMod == NULL)
		{
			hMod = ::LoadLibrary(_T("shdocvw.dll"));
			bIsLoadDll = true;
			if(hMod == NULL)
			{
				MessageBox(m_hWnd, _T("Error load shdocvw.dll."), NULL, MB_ICONERROR);
				return;
			}
		}
		LPFNORGFAV lpfnDoOrganizeFavDlg = (LPFNORGFAV)::GetProcAddress(hMod, "DoOrganizeFavDlg");
		if(lpfnDoOrganizeFavDlg == NULL)
		{
			MessageBox(m_hWnd, _T("Error GetProcAddress of DoOrganizeFavDlg."), NULL, MB_ICONERROR);
			return;
		}

		(*lpfnDoOrganizeFavDlg)(m_hWnd, (char*)tcBuf);
		if(bIsLoadDll)
		{
			::FreeLibrary(hMod);
		}
	}
	else if(sender != NULL)
	{
		Menu* menu = (Menu*)sender;
		const TCHAR* tips = menu->GetTips();
		if(tips != NULL)
		{
			editURL->SetText(tips);
			cc::Str url = cc::Str(cc::LangCode::UNICODE16, tips);
			ie->Navigate((wchar_t*)url.GetBuf());
			ie->Focus();
		}
	}
}

void MiniIEForm::tool_Click(Object* sender, EventArgs* e)
{
	if(sender == toolrefresh || sender == menuReload)
	{
		ie->GetWebBrowser()->ExecWB((ie_OLECMDID)22, (ie_OLECMDEXECOPT)1, NULL, NULL);
	}
	if(sender == toolstop)
	{
		ie->GetWebBrowser()->ExecWB((ie_OLECMDID)23, (ie_OLECMDEXECOPT)1, NULL, NULL);
	}
	if(sender == menuOpenFile)
	{
		isModal = true;
		cc::Str path = cc::Util::selectOpenFile(_T(""), _T("HTML Files (*.HTML,*.HTM,*.TXT)\0*.HTML;*.HTM;*.TXT\0\0"), _T(".html"), m_hWnd, _T("select html file:"));
		if(path.GetLength() > 0)
		{
			editURL->SetText((TCHAR*)path.GetBuf());
			cc::Str url = cc::Str(cc::LangCode::UNICODE16, path);
			ie->Navigate((wchar_t*)url.GetBuf());
			ie->Focus();
		}
		isModal = false;
	}
	//if(sender == menuSource)
	//{
	//}
	if(sender == menuSaveAs)
	{
		isModal = true;
		ie->GetWebBrowser()->ExecWB((ie_OLECMDID)4, (ie_OLECMDEXECOPT)0, NULL, NULL);
		isModal = false;
	}
	if(sender == menuOption)
	{
		typedef BOOL (WINAPI *LAUNCHCPL) (HWND);
		HMODULE hInetcpl = LoadLibrary(_T("inetcpl.cpl"));
		if(hInetcpl != NULL)
		{
			LAUNCHCPL cpl = (LAUNCHCPL)GetProcAddress(hInetcpl, "LaunchInternetControlPanel");
			if(cpl)
			{
				cpl(m_hWnd);
			}
			FreeLibrary(hInetcpl);
		}
	}
	else if(sender == toolgo)
	{
		editURL->SetReadOnly(true);
		cc::Str url = cc::Str(cc::LangCode::UNICODE16, editURL->GetText());
		if(url.GetLength() > 0)
		{
			ie->Navigate((wchar_t*)url.GetBuf());
			ie->Focus();
		}
		editURL->SetReadOnly(false);

		SaveToEntity(NULL, CC_NOTE_CFG_URL);
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
	else if(sender == menuSaveURL)
	{
		const TCHAR* tcUrl = editURL->GetText();
		if(_tcscmp(_T(""), tcUrl) == 0 || _tcscmp(_T("about:blank"), tcUrl) == 0)
		{
			sSaveUrl.Clear();
			mainForm->NoteChangeTitle(this, cc::Str(_T("about:blank")));
		}
		else
		{
			sSaveUrl = cc::Str(tcUrl);
			mainForm->NoteChangeTitle(this, sSaveUrl);
		}
		SaveToEntity(NULL, CC_NOTE_CFG_URL);
	}
	else if(sender == menuIsBlock)
	{
		this->isBlock = !this->isBlock;
		menuIsBlock->SetChecked(this->isBlock);
	}
}

void MiniIEForm::GetIEHwnd()
{
	if(hWndIE == NULL)
	{
		//start hook for ie
		hWndIE = ie->GetWebBrowserHWND(); //Shell Embedding
		hWndIE = ::GetWindow(hWndIE, GW_CHILD); //Shell DocObject View
		hWndIE = ::GetWindow(hWndIE, GW_CHILD); //Internet Explorer_Server
		if(hWndIE != NULL)
		{
			if(mapHWND == NULL)
			{
				mapHWND = new Map<HWND, Control*>(true, false, true);
			}
			//cc::Log::debug(_T("ie->GetWebBrowser()->get_HWND():%x"), hWndIE);
			mapHWND->Add(hWndIE, this);
			if(gMsgHook == NULL)
			{
				StartMsgHook(CC_APP_GetInstance());
			}
		}
	}
}

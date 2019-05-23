#include "MainForm.h"
#include <stdio.h>
#include <windows.h>
#include <Winuser.h>
#include <Wtsapi32.h>

const TCHAR* tcKey_RUN = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\");
//for compile under winnt(otherwise need define[WINVER=0x0601])
#if (NTDDI_VERSION < NTDDI_WINXPSP1)
typedef enum
{
    eConnTypeUnknown,
    eConnType3Point5mm,
    eConnTypeQuarter,
    eConnTypeAtapiInternal,
    eConnTypeRCA,
    eConnTypeOptical,
    eConnTypeOtherDigital,
    eConnTypeOtherAnalog,
    eConnTypeMultichannelAnalogDIN,
    eConnTypeXlrProfessional,
    eConnTypeRJ11Modem,
    eConnTypeCombination
} EPcxConnectionType;

typedef enum
{
    eGeoLocRear = 0x1,
    eGeoLocFront,
    eGeoLocLeft,
    eGeoLocRight,
    eGeoLocTop,
    eGeoLocBottom,
    eGeoLocRearPanel,
    eGeoLocRiser,
    eGeoLocInsideMobileLid,
    eGeoLocDrivebay,
    eGeoLocHDMI,
    eGeoLocOutsideMobileLid,
    eGeoLocATAPI,
    eGeoLocReserved5,
    eGeoLocReserved6,
    EPcxGeoLocation_enum_count
} EPcxGeoLocation;

typedef enum
{
    eGenLocPrimaryBox = 0,
    eGenLocInternal,
    eGenLocSeparate,
    eGenLocOther,
    EPcxGenLocation_enum_count
} EPcxGenLocation;

typedef enum
{
    ePortConnJack = 0,
    ePortConnIntegratedDevice,
    ePortConnBothIntegratedAndJack,
    ePortConnUnknown
} EPxcPortConnection;

// structure for KSPROPERTY_JACK_DESCRIPTION pin property
typedef struct 
{
    DWORD                 ChannelMapping;
    COLORREF              Color;   // use RGB() macro to generate these
    EPcxConnectionType    ConnectionType;
    EPcxGeoLocation       GeoLocation;
    EPcxGenLocation       GenLocation;
    EPxcPortConnection    PortConnection;
    BOOL                  IsConnected;
} KSJACK_DESCRIPTION, *PKSJACK_DESCRIPTION;

typedef struct _tagKSJACK_DESCRIPTION2
{
  DWORD              DeviceStateInfo; // Top 16 bits: Report current device state, active, streaming, idle, or hardware not ready
                                      // Bottom 16 bits: detailed reason to further explain state in top 16 bits
  DWORD              JackCapabilities; // Report jack capabilities such as jack presence detection capability 
                                       // or dynamic format changing capability         
} KSJACK_DESCRIPTION2, *PKSJACK_DESCRIPTION2;

typedef enum 
{
    KSJACK_SINK_CONNECTIONTYPE_HDMI = 0,            // HDMI
    KSJACK_SINK_CONNECTIONTYPE_DISPLAYPORT,         // DisplayPort
} KSJACK_SINK_CONNECTIONTYPE;

#define MAX_SINK_DESCRIPTION_NAME_LENGTH 32
typedef struct _tagKSJACK_SINK_INFORMATION
{
  KSJACK_SINK_CONNECTIONTYPE ConnType;              // Connection Type
  WORD  ManufacturerId;                             // Sink manufacturer ID
  WORD  ProductId;                                  // Sink product ID
  WORD  AudioLatency;                                    // Sink audio latency
  BOOL  HDCPCapable;                                // HDCP Support
  BOOL  AICapable;                                  // ACP Packet, ISRC1, and ISRC2 Support
  UCHAR SinkDescriptionLength;                      // Monitor/Sink name length
  WCHAR SinkDescription[MAX_SINK_DESCRIPTION_NAME_LENGTH];   // Monitor/Sink name
  LUID  PortId;                                     // Video port identifier
}  KSJACK_SINK_INFORMATION, *PKSJACK_SINK_INFORMATION;
#endif

#include <mmdeviceapi.h>
#include <endpointvolume.h>
using namespace cc;
using namespace cc::win;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//when add manifest.xml for xp theme, also need set controls to have default teansparent
	//so while some controls need backcolor, need remove  (OuterStyle::TransparentParentBrush|OuterStyle::TransparentParentErase)
	cc::win::App::SetDefaultTransparent(true);
	cc::win::App::SetInstance(hInstance);
	//should set this before cc::Config::GetLang();
	cc::win::App::GetConfig()->LoadLang(CC_CONFIG_LANGENG);
	//the name is used for CreateMutex"s flages,cannot change to other lang
	//#ifdef _DEBUG_
	//cc::win::App::SetNameVersion(_T("uuAssistant[uuware.com](debug)"), _T("Ver 2.02 build 20080325"));
	//#else
	//cc::win::App::SetNameVersion(_T("uuAssistant[uuware.com]"), _T("Ver 2.02 build 20080325"));
	//#endif
	cc::win::App::SetNameVersion(_T(APPNAME), _T(APPVERSION));
	if(!cc::win::App::IsFirstInstance(NULL))
	{
		return 0;
	}

	//set Transparent for TabControlEx
	cc::win::TabPageConfig::nOuterStyle = OuterStyle::TransparentParentErase;
	cc::win::FormConfig::nInnerStyleClient = InnerStyle::BackgroundTheme;
	cc::win::TabControlExConfig::nOuterStyle = OuterStyle::TransparentParentErase;
	cc::win::TabPageConfig::corBackground = ::GetSysColor(COLOR_MENU);
	cc::win::TabPageConfig::corHotBackColor = ::GetSysColor(COLOR_MENU);
	cc::win::TabPageConfig::corTextground = RGB(255, 0, 0);
	cc::win::TextBoxConfig::nOuterStyle |= OuterStyle::TransparentParentBrush;
	//should set next for RichTextBox
	//rich->SetStyle(StyleType::ExStyle, 0, WS_EX_TRANSPARENT);
	//rich->SetOuterStyle(OuterStyle::TransparentHollowBrush, true);

	cc::win::App::SetAlwaysTips(true);
	MainForm* form = new MainForm();
	cc::win::App::Run(form);
	form->Dispose();
	delete form;
	return 0;
}

// code from SmallestWindowFromPoint
HWND CC_SmallestWindowFromPoint(const POINT &point)
{
	// Find the smallest "window" still containing the point
	// Doing this prevents us from stopping at the first window containing the point
	RECT rect, rectSearch;
	HWND hParentWnd, hWnd, hSearchWnd;

	hWnd = ::WindowFromPoint(point);
	if(hWnd != NULL)
	{
		// Get the size and parent for compare later
		::GetWindowRect(hWnd, &rect);
		hParentWnd = ::GetParent(hWnd);

		// We only search further if the window has a parent
		if(hParentWnd != NULL)
		{
			// Search from the window down in the Z-Order
			hSearchWnd = hWnd;
			do
			{
				hSearchWnd = ::GetWindow(hSearchWnd, GW_HWNDNEXT);

				// Does the search window also contain the point, have the same parent, and is visible?
				::GetWindowRect(hSearchWnd, &rectSearch);
				if(::PtInRect(&rectSearch, point) && ::IsChild(hParentWnd, hSearchWnd) && ::IsWindowVisible(hSearchWnd))
				{
					// It does, but is it smaller?
					if(((rectSearch.right - rectSearch.left) * (rectSearch.bottom - rectSearch.top)) < ((rect.right - rect.left) * (rect.bottom - rect.top)))
					{
						// Found new smaller window, update compare window
						hWnd = hSearchWnd;
						::GetWindowRect(hWnd, &rect);
					}
				}
			}while(hSearchWnd != NULL);
		}
	}

	return hWnd;
}

void CC_InvertBorder(const HWND hWnd, int width = 0);
void CC_InvertBorder(const HWND hWnd, int width)
{
	if(!IsWindow(hWnd))
	{
		return;
	}

	RECT rect;
	// Get the coordinates of the window on the screen
	::GetWindowRect(hWnd, &rect);
	// Get a handle to the window's device context
	HDC hDC = ::GetWindowDC(hWnd);
	if(width < 1) {
		width = 3 * GetSystemMetrics(SM_CXBORDER);
	}
	CC_APP_GetDrawActor()->InvertBorder(0, 0, rect.right - rect.left, rect.bottom - rect.top, hDC, width);
	::ReleaseDC(hWnd, hDC);
}

int StartHook(HINSTANCE& glhInstance, HWND m_hWnd)
{
	if(glhInstance != NULL)
	{
		::FreeLibrary(glhInstance);
		glhInstance = NULL;
	}

	cc::Str sDll = cc::Util::getStartupPath(true).Append(_T("uuAssistant.dll"));
	glhInstance = ::LoadLibrary((TCHAR*)sDll.GetBuf());
	if(glhInstance != NULL)
	{
		long (*pStartMsgHook)(HINSTANCE, HWND) = NULL;
		pStartMsgHook = (long(*)(HINSTANCE, HWND))::GetProcAddress(glhInstance, "StartMsgHook");
		if(pStartMsgHook)
		{
			if(pStartMsgHook(glhInstance, m_hWnd) == 0)
			{
				return 0;
			}
		}
	}
	else
	{
		return -1;
	}
	return -2;
}
void FreeHook(HINSTANCE& glhInstance)
{
	if(glhInstance != NULL)
	{
		long (*pStopMsgHook)() = NULL;
		pStopMsgHook = (long(*)())::GetProcAddress(glhInstance, "StopMsgHook");
		if(pStopMsgHook)
		{
			pStopMsgHook();
		}

		::FreeLibrary(glhInstance);
		glhInstance = NULL;
	}
}

bool PutToClipboard(HBITMAP bmp, cc::Str sPath, int ind)
{
	bool ret = true;
	if(sPath.GetLength() > 0 && !cc::Util::dExist(sPath))
	{
		cc::Util::dCreate(sPath);
	}
	if(sPath.GetLength() > 0 && cc::Util::dExist(sPath))
	{
		CxImage img;
		img.CreateFromHBITMAP(bmp);

		cc::Str ext;
		int ntype = 0;
		if(ind <= 0)
		{
			ext = _T(".png");
			ntype = CXIMAGE_FORMAT_PNG;
		}
		else if(ind == 1)
		{
			ext = _T(".jpg");
			ntype = CXIMAGE_FORMAT_JPG;
			int nQuality = CC_APP_GetIni()->Get(_T("Main.JpegQuality"), 75);
			if(nQuality < 1 || nQuality > 100)
			{
				nQuality = 75;
			}
			img.SetJpegQuality(nQuality);
		}
		else if(ind == 2)
		{
			ext = _T(".tif");
			ntype = CXIMAGE_FORMAT_TIF;
		}
		else if(ind == 3)
		{
			ext = _T(".gif");
			ntype = CXIMAGE_FORMAT_GIF;
			img.CreateFromHBITMAP(bmp, 0, 8);
		}
		else //if(ind == 4)
		{
			ext = _T(".bmp");
			ntype = CXIMAGE_FORMAT_BMP;
		}

		cc::Time time = cc::Time::GetCurrentTime();
		cc::Str sFile = sPath;
		sFile.AppendFormat(_T("%d%02d%02d_%02d%02d%02d"), time.GetYear(), time.GetMonth(), time.GetDay(),
			time.GetHour(), time.GetMinute(), time.GetSecond()).Append(ext);
		while(cc::Util::fExist(sFile))
		{
			sFile = sPath;
			sFile.AppendFormat(_T("%d%02d%02d_%02d%02d%02d"), time.GetYear(), time.GetMonth(), time.GetDay(),
				time.GetHour(), time.GetMinute(), time.GetSecond()).Append(ext);
		}

		img.Save(sFile, ntype);
		DeleteObject(bmp);
	}
	else
	{
		if(!cc::Util::SetClipboardBitmap(bmp))
		{
			DeleteObject(bmp);
		}
	}
	return ret;
}

ShowImageDialog::ShowImageDialog()
{
	scrWork = new ScrollContainer();
	scrWork->SetName(_T("scrWork"));
	scrWork->SetLocation(5, 5);
	scrWork->SetSize(400-10, 400-10);
	scrWork->EnableScrollBar(SB_BOTH, true);
	scrWork->SetClientBounds(0, 0, 400-10, 400-10);
	scrWork->SetMinSize(100 - 9, 100 - 16);
	scrWork->SetMinMoveLimit(6);
	scrWork->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	scrWork->UpdatePositionEvent += EventHandler((Object*)this, (EventFun)&ShowImageDialog::scrWork_UpdateClientPosition);
	this->AddControl(scrWork);

	//add showImgBig to scrWork
	showImgBig = new ShowImage();
	showImgBig->SetLocation(5, 5);
	showImgBig->SetSize(400-10, 400-10);
	showImgBig->SetBackground(RGB(40, 80, 80));
	showImgBig->SetEnlarge(1);
	//showImgBig->MouseDown += MouseEventHandler((Object*)this, (MouseEventFun)&ShowImageDialog::ImgBig_MouseDown);
	//showImgBig->MouseMove += MouseEventHandler((Object*)this, (MouseEventFun)&ShowImageDialog::ImgBig_MouseMove);
	//showImgBig->MouseUp += MouseEventHandler((Object*)this, (MouseEventFun)&ShowImageDialog::ImgBig_MouseUp);
	//showImgBig->MouseLeave += EventHandler((Object*)this, (EventFun)&ShowImageDialog::ImgBig_MouseLeave);
	scrWork->AddControl(showImgBig);

	this->SetClientSize(400, 400);
	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
}


void ShowImageDialog::scrWork_UpdateClientPosition(Object* sender, EventArgs* e)
{
	//if(ptOldImgBig.x != -1 && ptOldImgBig.y != -1)
	//{
	//	CC_InvertBorder(ptOldImgBig.x, ptOldImgBig.y, nEnlargeIndex, nEnlargeIndex);
	//	ptOldImgBig.x = -1;
	//	ptOldImgBig.y = -1;
	//}
}

CaptureDialog::CaptureDialog(HWND mainFormHwnd)
{
	this->mainFormHwnd = mainFormHwnd;
	hWndActive = NULL;
	//SetMoveFormOnClick(true);
	imageList = (ImageList*)CC_APP_GetVarious(_T("ImageList"));
	this->SetSize(122, 37);
	this->SetMaxSize(122, 37);
	this->SetMinSize(122, 37);
	this->SetConfigName(_T("CaptureDialog"));
	this->SetStyle(StyleType::Style, WS_VISIBLE, 0);
	this->SetStyle(StyleType::ExStyle, 0, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
	this->Load += EventHandler((Object*)this, (EventFun)&CaptureDialog::Form_Load);
	this->Closed += EventHandler((Object*)this, (EventFun)&CaptureDialog::Form_Closed);
	this->Shutdown += EventHandler((Object*)this, (EventFun)&CaptureDialog::Form_Shutdown);
	this->SetStyle(StyleType::Style, WS_CAPTION, 0);
	this->SetMoveFormOnClick(true);

	//context menu
	contMenu = new ContextMenu();
	contMenu->SetName(_T("contCap"));
	contMenu->SetAllOwnerDraw(true);
	this->DisposeItemAdd(contMenu);
	this->SetContextMenu(contMenu);

	menuClose = new MenuItem();
	menuClose->SetName(_T("menuClose"));
	menuClose->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	menuClose->SetIcon(imageList, IDI_ICON_EXIT);
	contMenu->ItemAdd(menuClose);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuEdit = new MenuItem();
	menuEdit->SetName(_T("menuEdit"));
	menuEdit->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	//menuSave->SetIcon(imageList, IDI_ICON_CAPW);
	contMenu->ItemAdd(menuEdit);

	menuCopy = new MenuItem();
	menuCopy->SetName(_T("menuCopy"));
	menuCopy->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	//menuCopy->SetIcon(imageList, IDI_ICON_CAPW);
	contMenu->ItemAdd(menuCopy);
	menuCopy->SetChecked(true);

	menuSave = new MenuItem();
	menuSave->SetName(_T("menuSave"));
	menuSave->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	//menuSave->SetIcon(imageList, IDI_ICON_CAPW);
	contMenu->ItemAdd(menuSave);

	toolMove = new ToolItem();
	toolMove->SetName(_T("toolMove"));
	toolMove->SetIcon(imageList, IDI_ICON_MAIN);
	toolMove->SetLocation(1, 1);
	toolMove->SetSize(20, 18);
	toolMove->SetMouseDownMoveForm(this);
	toolMove->SetText(NULL);
	toolMove->SetStyle(StyleType::Outer, OuterStyle::ContextMenuButton, OuterStyle::AlwaysNoEdge|OuterStyle::AutoSize);
	this->AddControl(toolMove);
	toolMove->SetContextMenu(contMenu);

	toolsysmenu = new ToolItem();
	toolsysmenu->SetName(_T("toolsysmenu"));
	toolsysmenu->SetContextMenu(contMenu);
	toolsysmenu->SetText(NULL);
	toolsysmenu->SetLocationOffset(toolMove, false, 1, 0);
	toolsysmenu->SetSize(11, 19);
	this->AddControl(toolsysmenu);

	toolCapWin = new ToolItem();
	toolCapWin->SetName(_T("toolCapWin"));
	toolCapWin->SetIcon(imageList, IDI_ICON_CAPW);
	toolCapWin->SetLocationOffset(toolsysmenu, false, 1, 0);
	toolCapWin->SetSize(20, 18);
	toolCapWin->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	toolCapWin->MouseMove += MouseEventHandler((Object*)this, (MouseEventFun)&CaptureDialog::toolCapWin_MouseMove);
	this->AddControl(toolCapWin);

	toolCapScreen = new ToolItem();
	toolCapScreen->SetName(_T("toolCapScreen"));
	toolCapScreen->SetIcon(imageList, IDI_ICON_CAPS);
	toolCapScreen->SetLocationOffset(toolCapWin, false, 1, 0);
	toolCapScreen->SetSize(20, 18);
	toolCapScreen->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	this->AddControl(toolCapScreen);

	editScroll = new TextBox();
	editScroll->SetName(_T("editScroll"));
	//for minus
	//editScroll->SetStyleNumber(true);
	editScroll->SetText(_T(""));
	editScroll->SetLocationOffset(toolCapScreen, false, 1, 2);
	editScroll->SetSize(30, 17);
	editScroll->SetMaxLength(4);
	editScroll->SetOuterStyle(OuterStyle::AutoConfig, true);
	//if set this flage,the select all text while GotFocus
	editScroll->SetOuterStyle(OuterStyle::PrivateFlag1, true);
	this->AddControl(editScroll);

	//chkSave = new CheckBox();
	//chkSave->SetName(_T("chkSave"));
	//chkSave->SetText(_T(""));
	//chkSave->SetLocationOffset(editScroll, false, 2, 0);
	//chkSave->SetSize(18, 18);
	//chkSave->SetOuterStyle(OuterStyle::AutoSize, true);
	//chkSave->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	//this->AddControl(chkSave);

	editPath = new TextBox();
	editPath->SetName(_T("editPath"));
	editPath->SetText(_T(""));
	editPath->SetLocation(1, 21);
	editPath->SetSize(60, 17);
	editPath->SetOuterStyle(OuterStyle::AutoConfig, true);
	this->AddControl(editPath);

	btnPath = new Button();
	btnPath->SetName(_T("btnPath"));
	btnPath->SetText(_T("..."));
	btnPath->SetLocationOffset(editPath, false, 0, 0);
	btnPath->SetSize(15, 17);
	btnPath->Click += EventHandler((Object*)this, (EventFun)&CaptureDialog::Control_Click);
	this->AddControl(btnPath);
	editPath->SetVisible(false);
	btnPath->SetVisible(false);

	comboList = new ComboBox();
	comboList->SetName(_T("comboList"));
	comboList->SetComboBoxStyle(ComboBoxStyle::DropDownList);
	comboList->SetLocationOffset(btnPath, false, 1, 0);
	comboList->SetSize(30, 145);
	comboList->SetText(_T(".png;\t.jpg;\t.tif;\t.gif;\t.bmp;\t"));
	this->AddControl(comboList);
	comboList->SetVisible(false);

	this->SetVisible(false);
}

void CaptureDialog::DoSetLang(KeyArr& keyArr, cc::Str sThisHead)
{
	Form::DoSetLang(keyArr, sThisHead);

	contMenu->SetLang(*CC_APP_GetLang(), CC_CONFIG_LANGSECTION);
}

void CaptureDialog::Form_Load(Object* sender, EventArgs* e)
{
	//load lang
	SetLang(*CC_APP_GetLang(), CC_CONFIG_LANGSECTION);

	//load config
	this->Config(true, *CC_APP_GetIni(), cc::Str(_T("")));

	int nWTop = CC_APP_GetIni()->Get(_T("Main.CapTop"), 100);
	int nWLeft = CC_APP_GetIni()->Get(_T("Main.CapLeft"), 100);
	this->SetLocation(nWLeft, nWTop);

	int ind = CC_APP_GetIni()->Get(_T("Main.CapType"), 0);
	comboList->SetSelectedIndex(ind);

	int nSType = CC_APP_GetIni()->GetBool(_T("Main.SaveType"), false);
	if(nSType == 1)
	{
		this->Control_Click(menuCopy, NULL);
	}
	else if(nSType == 2)
	{
		this->Control_Click(menuSave, NULL);
	}
	else
	{
		this->Control_Click(menuEdit, NULL);
	}
}

void CaptureDialog::Form_Closed(Object* sender, EventArgs* e)
{
	//do some save
	Form_Shutdown(sender, e);
}

void CaptureDialog::Form_Shutdown(Object* sender, EventArgs* e)
{
	//save config
	this->Config(false, *CC_APP_GetIni(), cc::Str(_T("")));

	RECT rect = this->GetBounds();
	CC_APP_GetIni()->Set(_T("Main.CapTop"), rect.top);
	CC_APP_GetIni()->Set(_T("Main.CapLeft"), rect.left);

	int ind = comboList->GetSelectedIndex();
	CC_APP_GetIni()->Set(_T("Main.CapType"), ind);

	int nSType = 0;
	if(menuCopy->GetChecked())
	{
		nSType = 1;
	}
	else if(menuSave->GetChecked())
	{
		nSType = 2;
	}
	CC_APP_GetIni()->Set(_T("Main.SaveType"), nSType);
}

HWND MainActivateWindow()
{
	HWND hWnd = NULL;
	//GUITHREADINFO info;
	//info.cbSize = sizeof(GUITHREADINFO);
	//GetGUIThreadInfo(NULL, &info);
	//return info.hwndFocus;
	hWnd = ::GetForegroundWindow();
	if(hWnd == ::GetDesktopWindow())
	{
		hWnd = NULL;
	}
	//else
	//{
	//	::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
	//	cc::Util::FourceWindowTop(hWnd);
	//}
	return hWnd;
}

void CaptureDialog::Control_Click(Object* sender, EventArgs* e)
{
	if(sender == menuSave || sender == menuCopy || sender == menuEdit)
	{
		int nSType = 0;
		if(sender == menuCopy)
		{
			nSType = 1;
			menuCopy->SetChecked(true);
			menuEdit->SetChecked(false);
			menuSave->SetChecked(false);
		}
		else if(sender == menuSave)
		{
			nSType = 2;
			menuSave->SetChecked(true);
			menuEdit->SetChecked(false);
			menuCopy->SetChecked(false);
		}
		else
		{
			menuEdit->SetChecked(true);
			menuSave->SetChecked(false);
			menuCopy->SetChecked(false);
		}
		if(nSType == 2)
		{
			this->SetMaxSize(122, 37 + 18);
			this->SetMinSize(122, 37 + 18);
			this->SetSize(122, 37 + 18);
		}
		else
		{
			this->SetMaxSize(122, 37);
			this->SetMinSize(122, 37);
			this->SetSize(122, 37);
		}
		editPath->SetVisible(nSType == 2);
		btnPath->SetVisible(nSType == 2);
		comboList->SetVisible(nSType == 2);
		return;
	}
	else if(sender == menuClose)
	{
		this->SetVisible(false);
		::SetTimer(this->mainFormHwnd, UU_TIMERID_CAPTURE_EXIT, 100, NULL);
		return;
	}
	else if(sender == btnPath)
	{
		cc::Str sPath = editPath->GetText();
		cc::Str selectedPath = cc::Util::selectDir(sPath, m_hWnd, _T("input the path for save images:"));
		if(selectedPath.GetLength() > 0 && !selectedPath.Equals(_T("\\")))
		{
			editPath->SetText(selectedPath);
		}
		return;
	}
	//RECT rect = this->GetBounds();
	this->SetTips(NULL);
	//this->SetVisible(false);
	//this->SetSize(1, 1);
	//this->SetLocation(-100, 1000);
	//::ShowWindow(m_hWnd, SW_MINIMIZE);
	//this->SetVisible(false);
	::SetWindowPos(this->m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_HIDEWINDOW);
	HWND hWnd = hWndActive;
	if(hWnd != NULL)
	{
		::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
		cc::Util::FourceWindowTop(hWnd);
		//::SetForegroundWindow(hWnd);
		//::BringWindowToTop(hWnd);
	}

	clock_t tmStart = ::clock();
	while((int)(clock() - tmStart) < 800)
	{
		App::DoEvents();
		Sleep(50);
	}

	int ww = GetSystemMetrics(SM_CXSCREEN);
	int hh = GetSystemMetrics(SM_CYSCREEN);
	if(hWnd == NULL)
	{
		hWnd = MainActivateWindow();
	}
//cc::Log::debug(_T("my333:%d, hWnd:%d"), m_hWnd, hWnd);
	if(sender == toolCapWin)
	{
		RECT rect2;
		GetWindowRect(hWnd, &rect2);
		if(rect2.top < 0)
		{
			rect2.top = 0;
		}
		if(rect2.left < 0)
		{
			rect2.left = 0;
		}
		if(rect2.bottom > hh)
		{
			rect2.bottom = hh;
		}
		if(rect2.right > ww)
		{
			rect2.right = ww;
		}
		int w = rect2.right - rect2.left;
		int h = rect2.bottom - rect2.top;
		HBITMAP bmp = cc::win::DrawActor::GetBmpFromHDC(rect2.left, rect2.top, w, h, w, h, NULL);
		//cannot delete bmp
		cc::Str sPath;
		if(menuSave->GetChecked())
		{
			sPath = editPath->GetText();
			if(sPath.GetLength() > 0 && !sPath.EndsWith(_T("\\")))
			{
				sPath.Append(_T("\\"));
			}
		}
		PutToClipboard(bmp, sPath, comboList->GetSelectedIndex());

		int nScroll = editScroll->GetTextInt();
		if(editScroll != 0)
		{
			POINT point;
			GetCursorPos(&point);

			SetCursorPos(rect2.right - 13, (rect2.bottom + rect2.top) / 2);
			Sleep(100);
			int nMove = -10;
			if(nScroll < 0)
			{
				nMove = 10;
				nScroll *= -1;
			}
			for(int n = 0; n < nScroll; n++)
			{
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, nMove, 0);
			}
			SetCursorPos(point.x, point.y);
		}
	}
	else if(sender == toolCapScreen)
	{
		HBITMAP bmp = cc::win::DrawActor::GetBmpFromHDC(0, 0, ww, hh, ww, hh, NULL);
		//cannot delete bmp
		cc::Str sPath;
		if(menuSave->GetChecked())
		{
			sPath = editPath->GetText();
		}
		PutToClipboard(bmp, sPath, comboList->GetSelectedIndex());
	}
	::SetWindowPos(this->m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	//::ShowWindow(m_hWnd, SW_RESTORE);
	//this->SetVisible(false);
	//this->SetLocation(rect.left, rect.top);
	//this->SetSize(90, 47);
	//this->SetVisible(true);
	//::SetForegroundWindow(hWnd);
	//::BringWindowToTop(hWnd);
	//cc::Log::debug(_T("my333:%d, act:%d"), m_hWnd, hWnd);
	if(menuEdit->GetChecked())
	{
		ShowImageDialog image = ShowImageDialog();
		image.ShowDialog();
	}
}
void CaptureDialog::toolCapWin_MouseMove(Object* sender, MouseEventArgs* e)
{
	//if(!Focused() && this->GetVisible())
	{
		HWND hWnd2 = MainActivateWindow();
		if(hWnd2 != m_hWnd && hWnd2 != NULL)
		{
			hWndActive = hWnd2;
			//cc::Log::debug(_T("my222:%d, act:%d"), m_hWnd, hWnd2);
		}
	}
}
LRESULT CaptureDialog::WndProc(EventArgs *e)
{
	if(e->Msg == WM_NCMOUSEMOVE || e->Msg == WM_MOUSEMOVE)
	{
		//if(!Focused() && this->GetVisible())
		{
			HWND hWnd2 = MainActivateWindow();
			if(hWnd2 != m_hWnd && hWnd2 != NULL)
			{
				hWndActive = hWnd2;
				//cc::Log::debug(_T("my:%d, act:%d"), m_hWnd, hWnd2);
			}
		}
	}
	return Form::WndProc(e);
}


StepDialog::StepDialog(int maxStep)
{
	if(maxStep < 1) {
		maxStep = 1;
	}
	progBar = new ProgressBar();
	progBar->SetName(_T("progBar"));
	progBar->SetLocation(10, 10);
	progBar->SetSize(maxStep, 17);
	progBar->SetRange(0, 255);
	this->AddControl(progBar);

	SetMoveFormOnClick(true);
	this->SetSize(280, 45);
	this->SetConfigName(_T("StepDialog"));
	this->SetStyle(StyleType::Style, WS_CAPTION, 0);
	this->SetStyle(StyleType::Style, WS_VISIBLE, 0);
	this->SetStyle(StyleType::ExStyle, 0, WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
	this->SetVisible(false);
}

void StepDialog::SetStep(int index, int delay)
{
	//this->SetVisible(true);
	if(!this->GetVisible()) {
		RECT rcScreen = {0, 0, 0, 0};
		if(!::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0))
		{
			rcScreen.right = (LONG)::GetSystemMetrics(SM_CXSCREEN);
			rcScreen.bottom = (LONG)::GetSystemMetrics(SM_CYSCREEN);
		}
		RECT rect = this->GetBounds();
		rect.left = (rcScreen.left + rcScreen.right) / 2 - (rect.right - rect.left) / 2;
		rect.top = rcScreen.bottom - 80;
		this->SetLocation(rect.left, rect.top);
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	}
	if(delay <= 3)
	{
		delay = 3;
	}
	if(delay > 10)
	{
		delay = 10;
	}
	if(index < 0)
	{
		index = 0;
	}
	progBar->SetPos(index);
	::SetTimer(this->m_hWnd, UU_TIMERID_UPDATE, delay * 1000, NULL);
}

void StepDialog::OnInnerTimer(EventArgs *e)
{
	//e->WParam is ID
	if(e == NULL || e->WParam == UU_TIMERID_UPDATE)
	{
		::KillTimer(m_hWnd, UU_TIMERID_UPDATE);
		this->SetVisible(false);
	}
}

MainForm::MainForm()
{
	int bit64 = 0;
	winVer = cc::Util::GetWindowsVersion(&bit64);
	volumeDialog = NULL;
	transparentDialog = NULL;
	captureDialog = NULL;
	formCalendar = NULL;
	const TCHAR* shareFlg = SPY_HOOK_KEY;
	int nShareMemorySize = sizeof(ShareEntity);
	pShareMemoryMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shareFlg);
	if(pShareMemoryMap == NULL)
	{
		pShareMemoryMap = ::CreateFileMapping( (HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nShareMemorySize, shareFlg );
	}
	pShareMemory = (ShareEntity*)::MapViewOfFile(pShareMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, nShareMemorySize);

	hWndPrev = NULL;
	glhInstance = NULL;
	m_MainFlag = 0;
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
	//isDoClosed = false;
	noteDefault = new NoteEntity();
	noteDefault->rcPos.left = 100;
	noteDefault->rcPos.top = 100;
	noteDefault->rcPos.right = noteDefault->rcPos.left + 230;
	noteDefault->rcPos.bottom = noteDefault->rcPos.top + 100;
	noteWKTime = new NoteEntity();
	noteWKTime->rcPos.left = 200;
	noteWKTime->rcPos.top = 200;
	noteWKTime->rcPos.right = noteWKTime->rcPos.left + 300;
	noteWKTime->rcPos.bottom = noteWKTime->rcPos.top + 200;
	notePWD = new cc::Str();
	lastNote = NULL;
	ZeroMemory(&stSaveTime, sizeof(SYSTEMTIME));
	ZeroMemory(&stNewTime, sizeof(SYSTEMTIME));
	//password = NULL;
	nFlashTime = 0;
	hWndFlash = NULL;
	tmVolumeDbClick = ::clock();
	toolColorStart = 0;

	InitializeComponent();

	CC_APP_SetVarious(_T("ImageList"), imageList);
	//CC_APP_SetVarious(_T("ContextMenu"), contMenu);
	//CC_APP_SetVarious(_T("NotifyIcon"), notify1);
	CC_APP_SetVarious(_T("MainForm"), this);
}

MainForm::~MainForm()
{
}

void MainForm::InitializeComponent()
{
	const TCHAR* tcKeySub = cc::win::App::GetName();
	hWndHide = NULL;
	memset(buf, 0, BUF_LEN * sizeof(TCHAR));
	//isSort, bIsDESC=false, isDistinct=false, nMinCapacity=0, nIncrement=1.3)
	//lstWin = new List<HWND>(false, false, true);
	//lstWinHide = new List<HWND>(false, false, true);
	lstWinHideSave = new List<HWND>(false, false, true);
	lstWinTrans = new List<HWND>(false, false, true);
	lstNote = new List<void*>(false, false, true);
	noteTmp = NULL;
	//hClipboard = NULL;
	lstClipboard = new List<cc::Str>(false, false, true);
	//context menu
	menuClipboard = new ContextMenu();
	menuClipboard->SetName(_T("contMenu"));
	menuClipboard->SetAllOwnerDraw(true);
	this->DisposeItemAdd(menuClipboard);

	imageList = new ImageList();
	imageList->SetIconSize(-1, -1);
	imageList->SetIcon(IDI_ICON_MAIN, IDI_ICON_MAIN);
	imageList->SetIcon(IDI_ICON_COPY, IDI_ICON_COPY);
	imageList->SetIcon(IDI_ICON_CUT, IDI_ICON_CUT);
	imageList->SetIcon(IDI_ICON_PASTE, IDI_ICON_PASTE);
	imageList->SetIcon(IDI_ICON_ATTACH, IDI_ICON_ATTACH);
	imageList->SetIcon(IDI_ICON_MOVE1, IDI_ICON_MOVE1);
	imageList->SetIcon(IDI_ICON_MOVE2, IDI_ICON_MOVE2);
	imageList->SetIcon(IDI_ICON_EXIT, IDI_ICON_EXIT);
	imageList->SetIcon(IDI_ICON_LOCK, IDI_ICON_LOCK);
	imageList->SetIcon(IDI_ICON_SAVE, IDI_ICON_SAVE);
	imageList->SetIcon(IDI_ICON_FONT, IDI_ICON_FONT);
	imageList->SetIcon(IDI_ICON_REFRESH, IDI_ICON_REFRESH);
	imageList->SetIcon(IDI_ICON_PRINT, IDI_ICON_PRINT);
	imageList->SetIcon(IDI_ICON_CLEAR, IDI_ICON_CLEAR);
	imageList->SetIcon(IDI_ICON_NEWWIN, IDI_ICON_NEWWIN);
	imageList->SetIcon(IDI_ICON_COLOR, IDI_ICON_COLOR);
	imageList->SetIcon(IDI_ICON_NOTE, IDI_ICON_NOTE);
	imageList->SetIcon(IDI_ICON_REDO, IDI_ICON_REDO);
	imageList->SetIcon(IDI_ICON_UNDO, IDI_ICON_UNDO);
	imageList->SetIcon(IDI_ICON_DELETE, IDI_ICON_DELETE);
	imageList->SetIcon(IDI_ICON_CAPW, IDI_ICON_CAPW);
	imageList->SetIcon(IDI_ICON_CAPS, IDI_ICON_CAPS);
	imageList->SetIcon(IDI_ICON_OPTION, IDI_ICON_OPTION);
	imageList->SetIcon(IDI_ICON_IE, IDI_ICON_IE);
	imageList->SetIcon(IDI_ICON_STOP, IDI_ICON_STOP);
	imageList->SetIcon(IDI_ICON_GO, IDI_ICON_GO);
	imageList->SetIcon(IDI_ICON_FOLDER, IDI_ICON_FOLDER);
	imageList->SetIcon(IDI_ICON_PAGE, IDI_ICON_PAGE);
	imageList->SetIcon(IDI_ICON_KEY, IDI_ICON_KEY);
	imageList->SetIcon(IDI_ICON_SETBACK, IDI_ICON_SETBACK);
	imageList->SetIcon(IDI_ICON_NOTEMIN, IDI_ICON_NOTEMIN);
	imageList->SetIcon(IDI_ICON_NOTEMIN2, IDI_ICON_NOTEMIN2);
	imageList->SetIcon(IDI_ICON_NOTEORG, IDI_ICON_NOTEORG);
	imageList->SetIcon(IDI_ICON_CALENDAR, IDI_ICON_CALENDAR);
	this->DisposeItemAdd(imageList);

	registryKey1 = cc::Registry::GetLocalMachine();
	this->DisposeItemAdd(registryKey1);

	//main menu
	mainMenuBar = new MainMenuBar();
	mainMenuBar->SetName(_T("mainMenuBar"));
	//only set ConfigName for one of mainMenu or mainMenuBar
	mainMenuBar->SetConfigName(_T("mainMenu"));
	mainMenuBar->SetMinSize(0, 23);
	mainMenuBar->SetCloseButtonEnable(false);
	mainMenuBar->SetDockable((DockStyles::Type)(DockStyles::Top|DockStyles::Bottom|DockStyles::None));
	mainMenuBar->SetBreak(true);
	this->AddToolBar(mainMenuBar);

	mainMenu = new MainMenu();
	mainMenu->SetName(_T("mainMenu"));
	mainMenuBar->SetConfigName(_T(""));
	mainMenu->SetAllOwnerDraw(true);
	mainMenuBar->SetMainMenu(mainMenu);
	//this->SetMainMenu(mainMenu);
	this->DisposeItemAdd(mainMenu);

	menuFile = new MenuItem();
	menuFile->SetName(_T("menuFile"));
	mainMenu->ItemAdd(menuFile);
	menuExit = new MenuItem();
	menuExit->SetName(_T("menuExit"));
	menuExit->SetIcon(imageList, IDI_ICON_EXIT);
	menuExit->SetShortcut(Shortcut::CtrlQ);
	menuExit->SetTips(NULL, imageList, IDI_ICON_EXIT);
	menuExit->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuExit_Click);
	menuFile->ItemAdd(menuExit);

	menuEdit = new MenuItem();
	menuEdit->SetName(_T("menuEdit"));
	//menuEdit->Popup += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	mainMenu->ItemAdd(menuEdit);
	menuUnDo = new MenuItem();
	menuUnDo->SetName(_T("menuUnDo"));
	menuUnDo->SetIcon(imageList, IDI_ICON_UNDO);
	menuUnDo->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuUnDo_Click);
	menuEdit->ItemAdd(menuUnDo);
	menuReDo = new MenuItem();
	menuReDo->SetName(_T("menuReDo"));
	menuReDo->SetIcon(imageList, IDI_ICON_REDO);
	menuReDo->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuReDo_Click);
	menuEdit->ItemAdd(menuReDo);
	menuEditBreak1 = new MenuItem();
	menuEditBreak1->SetText(_T("-"));
	menuEdit->ItemAdd(menuEditBreak1);
	menuCut = new MenuItem();
	menuCut->SetName(_T("menuCut"));
	menuCut->SetIcon(imageList, IDI_ICON_CUT);
	menuCut->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuCut_Click);
	menuEdit->ItemAdd(menuCut);
	menuCopy = new MenuItem();
	menuCopy->SetName(_T("menuCopy"));
	menuCopy->SetIcon(imageList, IDI_ICON_COPY);
	menuCopy->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuCopy_Click);
	menuEdit->ItemAdd(menuCopy);
	menuPaste = new MenuItem();
	menuPaste->SetName(_T("menuPaste"));
	menuPaste->SetIcon(imageList, IDI_ICON_PASTE);
	menuPaste->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuPaste_Click);
	menuEdit->ItemAdd(menuPaste);
	menuDel = new MenuItem();
	menuDel->SetName(_T("menuDel"));
	menuDel->SetIcon(imageList, IDI_ICON_DELETE);
	menuDel->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuDel_Click);
	menuEdit->ItemAdd(menuDel);
	menuEditBreak2 = new MenuItem();
	menuEditBreak2->SetText(_T("-"));
	menuEdit->ItemAdd(menuEditBreak2);
	menuSelAll = new MenuItem();
	menuSelAll->SetName(_T("menuSelAll"));
	menuSelAll->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuSelAll_Click);
	menuEdit->ItemAdd(menuSelAll);

	menuView = new MenuItem();
	menuView->SetName(_T("menuView"));
	mainMenu->ItemAdd(menuView);
	menuToolBar = new MenuItem();
	menuToolBar->SetName(_T("menuToolBar"));
	menuView->ItemAdd(menuToolBar);
	menuCommTool = new MenuItem();
	menuCommTool->SetName(_T("menuCommTool"));
	menuCommTool->SetChecked(true);
	menuCommTool->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuCommTool_Click);
	menuToolBar->ItemAdd(menuCommTool);
	menuEditTool = new MenuItem();
	menuEditTool->SetName(_T("menuEditTool"));
	menuEditTool->SetChecked(true);
	menuEditTool->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuEditTool_Click);
	menuToolBar->ItemAdd(menuEditTool);
	menuViewBreak1 = new MenuItem();
	menuViewBreak1->SetText(_T("-"));
	menuToolBar->ItemAdd(menuViewBreak1);
	menuFixTools = new MenuItem();
	menuFixTools->SetName(_T("menuFixTools"));
	menuFixTools->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuFixTools_Click);
	menuToolBar->ItemAdd(menuFixTools);
	menuStatusBar = new MenuItem();
	menuStatusBar->SetName(_T("menuStatusBar"));
	menuStatusBar->SetChecked(true);
	menuStatusBar->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuStatusBar_Click);
	menuView->ItemAdd(menuStatusBar);
	menuTips = new MenuItem();
	menuTips->SetName(_T("menuTips"));
	menuTips->SetChecked(true);
	menuTips->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuTips_Click);
	menuView->ItemAdd(menuTips);

	menuHelp = new MenuItem();
	menuHelp->SetName(_T("menuHelp"));
	mainMenu->ItemAdd(menuHelp);
	menuAbout = new MenuItem();
	menuAbout->SetName(_T("menuAbout"));
	menuAbout->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuAbout_Click);
	menuHelp->ItemAdd(menuAbout);

	//contextMenu
	contMenu = new ContextMenu();
	contMenu->SetName(_T("contMenu"));
	contMenu->SetAllOwnerDraw(true);
	contMenu->Popup += EventHandler((Object*)this, (EventFun)&MainForm::contMenu_Popup);
	this->DisposeItemAdd(contMenu);
	this->SetContextMenu(contMenu);

	menuShowNote = new MenuItem();
	menuShowNote->SetName(_T("menuShowNote"));
	//menuShowNote->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteShow_Click);
	//menuShowNote->SetIcon(imageList, IDI_ICON_NEWWIN);
	contMenu->ItemAdd(menuShowNote);

	menuNewNote = new MenuItem();
	menuNewNote->SetName(_T("menuNewNote"));
	menuNewNote->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteNew_Click);
	menuNewNote->SetIcon(imageList, IDI_ICON_NEWWIN);
	contMenu->ItemAdd(menuNewNote);

	menuNewIE = new MenuItem();
	menuNewIE->SetName(_T("menuNewIE"));
	menuNewIE->Click += EventHandler((Object*)this, (EventFun)&MainForm::NewIE_Click);
	menuNewIE->SetIcon(imageList, IDI_ICON_IE);
	contMenu->ItemAdd(menuNewIE);

	menuCloseAll = new MenuItem();
	menuCloseAll->SetName(_T("menuCloseAll"));
	menuCloseAll->Click += EventHandler((Object*)this, (EventFun)&MainForm::closeall_Click);
	menuCloseAll->SetIcon(imageList, IDI_ICON_EXIT);
	contMenu->ItemAdd(menuCloseAll);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuCapture = new MenuItem();
	menuCapture->SetName(_T("menuCapture"));
	menuCapture->SetIcon(imageList, IDI_ICON_CAPW);
	menuCapture->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	contMenu->ItemAdd(menuCapture);

	menuWorkTime = new MenuItem();
	menuWorkTime->SetName(_T("menuWorkTime"));
	menuWorkTime->SetIcon(imageList, IDI_ICON_NOTE);
	menuWorkTime->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	contMenu->ItemAdd(menuWorkTime);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuCalendar = new MenuItem();
	menuCalendar->SetName(_T("menuCalendar"));
	menuCalendar->SetIcon(imageList, IDI_ICON_CALENDAR);
	menuCalendar->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	contMenu->ItemAdd(menuCalendar);

	menuBreak1 = new MenuItem();
	menuBreak1->SetText(_T("-"));
	contMenu->ItemAdd(menuBreak1);

	menuShow = new MenuItem();
	menuShow->SetName(_T("menuShow"));
	menuShow->Click += EventHandler((Object*)this, (EventFun)&MainForm::showhide_Click);
	contMenu->ItemAdd(menuShow);

	menuExit2 = new MenuItem();
	menuExit2->SetName(_T("menuExit"));
	menuExit2->SetIcon(imageList, IDI_ICON_EXIT);
	menuExit2->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuExit_Click);
	contMenu->ItemAdd(menuExit2);

	commTool = new ToolBar();
	commTool->SetName(_T("commTool"));
	commTool->SetMinSize(0, 25);
	commTool->SetBreak(true);
	this->AddToolBar(commTool);

	toolOnTop = new ToolItem();
	toolOnTop->SetName(_T("toolOnTop"));
	toolOnTop->SetIsPushButton(true);
	toolOnTop->SetIcon(imageList, IDI_ICON_ATTACH);
	toolOnTop->SetOuterStyle(OuterStyle::AutoConfig, true);
	toolOnTop->Click += EventHandler((Object*)this, (EventFun)&MainForm::toolOnTop_Click);
	commTool->AddControl(toolOnTop);

	toolSetBack = new ToolItem();
	toolSetBack->SetName(_T("toolSetBack"));
	toolSetBack->SetIcon(imageList, IDI_ICON_SETBACK);
	toolSetBack->Click += EventHandler((Object*)this, (EventFun)&MainForm::toolSetBack_Click);
	commTool->AddControl(toolSetBack);

	editTool = new ToolBar();
	editTool->SetName(_T("editTool"));
	editTool->SetMinSize(0, 25);
	this->AddToolBar(editTool);

	toolCopy = new ToolItem();
	toolCopy->SetName(_T("toolCopy"));
	toolCopy->SetIcon(imageList, IDI_ICON_COPY);
	toolCopy->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuCopy_Click);
	editTool->AddControl(toolCopy);

	toolCut = new ToolItem();
	toolCut->SetName(_T("toolCut"));
	toolCut->SetIcon(imageList, IDI_ICON_CUT);
	toolCut->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuCut_Click);
	editTool->AddControl(toolCut);

	toolPaste = new ToolItem();
	toolPaste->SetName(_T("toolPaste"));
	toolPaste->SetIcon(imageList, IDI_ICON_PASTE);
	toolPaste->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuPaste_Click);
	editTool->AddControl(toolPaste);

	tab1 = new TabControlEx();
	tab1->SetName(_T("tab1"));
	tab1->SetConfigName(_T(""));
	tab1->SetLocation(2, 1);
	tab1->SetSize(597, 477);
	tab1->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	tab1->SetOuterStyle(OuterStyle::AutoConfig, true);
	tab1->SetContextMenu(contMenu);
	tab1->SetTabStyle(0, TabStyle::NextShowMenu|TabStyle::NoBodyRect);
	this->AddControl(tab1);

	int nTop = 1;

	//pageNote
	pageNote = new TabPage();
	pageNote->SetName(_T("pageNote"));
	pageNote->SetClientSize(347, 154);
	pageNote->SetContextMenu(contMenu);
	tab1->AddPage(pageNote);

	ltbNoeForm = new ListBox();
	ltbNoeForm->SetName(_T("ltbNoeForm"));
	ltbNoeForm->SetLocation(2, nTop);
	ltbNoeForm->SetSize(242, 152);
	ltbNoeForm->ItemDoubleClick += EventHandler((Object*)this, (EventFun)&MainForm::NoteShowHide_Click);
	ltbNoeForm->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	pageNote->AddControl(ltbNoeForm);

	btnNoteNew = new Button();
	btnNoteNew->SetName(_T("btnNoteNew"));
	btnNoteNew->SetLocation(247, nTop);
	btnNoteNew->SetSize(100, 17);
	btnNoteNew->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteNew_Click);
	btnNoteNew->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteNew);

	nTop += 19;
	btnMiniIENew = new Button();
	btnMiniIENew->SetName(_T("btnMiniIENew"));
	btnMiniIENew->SetLocation(247, nTop);
	btnMiniIENew->SetSize(100, 17);
	btnMiniIENew->Click += EventHandler((Object*)this, (EventFun)&MainForm::NewIE_Click);
	btnMiniIENew->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnMiniIENew);

	nTop += 19;
	btnNoteShowHide = new Button();
	btnNoteShowHide->SetName(_T("btnNoteShowHide"));
	btnNoteShowHide->SetLocation(247, nTop);
	btnNoteShowHide->SetSize(100, 17);
	btnNoteShowHide->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteShowHide_Click);
	btnNoteShowHide->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteShowHide);

	nTop += 19;
	btnNoteRemove = new Button();
	btnNoteRemove->SetName(_T("btnNoteRemove"));
	btnNoteRemove->SetLocation(247, nTop);
	btnNoteRemove->SetSize(100, 17);
	btnNoteRemove->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteRemove_Click);
	btnNoteRemove->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteRemove);

	nTop += 19;
	btnNoteSetPWD = new Button();
	btnNoteSetPWD->SetName(_T("btnNoteSetPWD"));
	btnNoteSetPWD->SetLocation(247, nTop);
	btnNoteSetPWD->SetSize(100, 17);
	btnNoteSetPWD->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteSetPWD_Click);
	btnNoteSetPWD->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteSetPWD);

	nTop += 19;
	btnNoteUp = new Button();
	btnNoteUp->SetName(_T("btnNoteUp"));
	btnNoteUp->SetLocation(247, nTop);
	btnNoteUp->SetSize(49, 17);
	btnNoteUp->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteUp_Click);
	btnNoteUp->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteUp);

	btnNoteDown = new Button();
	btnNoteDown->SetName(_T("btnNoteDown"));
	btnNoteDown->SetLocation(247+51, nTop);
	btnNoteDown->SetSize(49, 17);
	btnNoteDown->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteDown_Click);
	btnNoteDown->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(btnNoteDown);

	nTop += 20;
	chkMirco = new CheckBox();
	chkMirco->SetName(_T("chkMirco"));
	chkMirco->SetLocation(247, nTop);
	chkMirco->SetSize(170, 17);
	chkMirco->SetOuterStyle(OuterStyle::AutoSize, true);
	chkMirco->SetOuterStyle(OuterStyle::AutoConfig, true);
	chkMirco->Click += EventHandler((Object*)this, (EventFun)&MainForm::chkMirco_Click);
	chkMirco->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageNote->AddControl(chkMirco);

	//btnNoteTimer = new Button();
	//btnNoteTimer->SetName(_T("btnNoteTimer"));
	//btnNoteTimer->SetLocation(247, 96);
	//btnNoteTimer->SetSize(100, 17);
	//btnNoteTimer->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteTimer_Click);
	//btnNoteTimer->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	//pageNote->AddControl(btnNoteTimer);

	//pageWorkTime
	//pageWorkTime = new TabPage();
	//pageWorkTime->SetName(_T("pageWorkTime"));
	//pageWorkTime->SetClientSize(347, 154);
	//pageWorkTime->SetContextMenu(contMenu);
	//tab1->AddPage(pageWorkTime);

	//label = new Label();
	//label->SetName(_T("labTime"));
	//label->SetLocation(3, 1);
	//label->SetSize(300, 15);
	//pageWorkTime->AddControl(label);

	//editTime = new TextBox();
	//editTime->SetText(_T(""));
	//editTime->SetMultiline(true);
	//editTime->SetLocation(2, 16);
	//editTime->SetSize(345, 119);
	//editTime->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editTime->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editTime->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	//pageWorkTime->AddControl(editTime);

	//btnRefreshTime = new Button();
	//btnRefreshTime->SetName(_T("btnRefreshTime"));
	//btnRefreshTime->SetLocation(2, 137);
	//btnRefreshTime->SetSize(100, 17);
	//btnRefreshTime->Click += EventHandler((Object*)this, (EventFun)&MainForm::refreshtime_Click);
	//btnRefreshTime->Anchor = (AnchorStyles::Type)(AnchorStyles::Left | AnchorStyles::Bottom);
	//pageWorkTime->AddControl(btnRefreshTime);

	nTop = 1;
	//pageTool
	pageTool = new TabPage();
	pageTool->SetName(_T("pageTool"));
	pageTool->SetClientSize(347, 154);
	pageTool->SetContextMenu(contMenu);
	tab1->AddPage(pageTool);

	/*
	label = new Label();
	label->SetName(_T("labWinList"));
	label->SetLocation(3, nTop);
	label->SetSize(100, 15);
	pageTool->AddControl(label);

	nTop += 16;
	comboWinList = new ComboBox();
	comboWinList->SetName(_T("comboWinList"));
	comboWinList->SetComboBoxStyle(ComboBoxStyle::DropDownList);
	comboWinList->SetLocation(2, nTop);
	comboWinList->SetSize(295, 145);
	comboWinList->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
	pageTool->AddControl(comboWinList);

	btnRefresh = new Button();
	btnRefresh->SetName(_T("btnRefresh"));
	btnRefresh->SetLocation(297, nTop);
	btnRefresh->SetSize(50, 17);
	btnRefresh->Click += EventHandler((Object*)this, (EventFun)&MainForm::refresh_Click);
	btnRefresh->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageTool->AddControl(btnRefresh);

	nTop += 25;
	btnChgWin = new Button();
	btnChgWin->SetName(_T("btnTransparent"));
	btnChgWin->SetLocation(2, nTop);
	btnChgWin->SetSize(90, 17);
	btnChgWin->Click += EventHandler((Object*)this, (EventFun)&MainForm::chgwin_Click);
	pageTool->AddControl(btnChgWin);

	btnResetWin = new Button();
	btnResetWin->SetName(_T("btnRTransparent"));
	btnResetWin->SetLocationOffset(btnChgWin, false, 2, 0);
	btnResetWin->SetSize(40, 17);
	btnResetWin->Click += EventHandler((Object*)this, (EventFun)&MainForm::chgwin_Click);
	pageTool->AddControl(btnResetWin);

	//hScrollBar1 = new HScrollBar();
	//hScrollBar1->SetLocation(2, 48);
	//hScrollBar1->SetSize(255, 17);
	//hScrollBar1->SetMinMaxValue(0, 255);
	//hScrollBar1->SetValue(159);
	//pageTool->AddControl(hScrollBar1);
	pickPercent = new PickPercentCtl();
	pickPercent->SetLocationOffset(btnResetWin, false, 4, 0);
	pickPercent->SetSize(40, 20);
	pickPercent->SetMinMaxValue(0, 255);
	pickPercent->SetValue(159);
	pickPercent->SetOuterStyle(OuterStyle::AutoConfig, true);
	pageTool->AddControl(pickPercent);

	btnOnTop = new Button();
	btnOnTop->SetName(_T("btnOnTop"));
	btnOnTop->SetLocationOffset(pickPercent, false, 2, 0);
	btnOnTop->SetSize(90, 17);
	btnOnTop->Click += EventHandler((Object*)this, (EventFun)&MainForm::ontop_Click);
	pageTool->AddControl(btnOnTop);

	btnNotOnTop = new Button();
	btnNotOnTop->SetName(_T("btnNotOnTop"));
	btnNotOnTop->SetLocationOffset(btnOnTop, false, 2, 0);
	btnNotOnTop->SetSize(90, 17);
	btnNotOnTop->Click += EventHandler((Object*)this, (EventFun)&MainForm::notontop_Click);
	pageTool->AddControl(btnNotOnTop);

	//btnShowPassword = new Button();
	//btnShowPassword->SetName(_T("btnShowPassword"));
	//btnShowPassword->SetLocation(2, 48);
	//btnShowPassword->SetSize(100, 34);
	//btnShowPassword->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	//pageTool->AddControl(btnShowPassword);

	nTop += 25;
	*/
	chkCalendar = new CheckBox();
	chkCalendar->SetName(_T("chkCalendar"));
	chkCalendar->SetText(_T("Desktop Calendar"));
	chkCalendar->SetLocation(2, nTop);
	chkCalendar->SetSize(150, 17);
	chkCalendar->SetOuterStyle(OuterStyle::AutoSize, true);
	chkCalendar->SetOuterStyle(OuterStyle::AutoConfig, true);
	chkCalendar->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	pageTool->AddControl(chkCalendar);

	nTop += 21;
	chkShowCapture = NULL;
	chkShowCapture = new CheckBox();
	chkShowCapture->SetName(_T("chkShowCapture"));
	chkShowCapture->SetLocation(2, nTop);
	chkShowCapture->SetSize(150, 17);
	chkShowCapture->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	pageTool->AddControl(chkShowCapture);

	//nTop += 21;
	btnOpenTime = new Button();
	btnOpenTime->SetName(_T("btnOpenTime"));
	btnOpenTime->SetLocationOffset(chkShowCapture, false, 2, 0);
	btnOpenTime->SetSize(150, 17);
	btnOpenTime->Click += EventHandler((Object*)this, (EventFun)&MainForm::opentime_Click);
	//btnOpenTime->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Right);
	pageTool->AddControl(btnOpenTime);

	nTop += 21;
	Label* label = new Label();
	label->SetName(_T("labRGB"));
	label->SetLocation(3, nTop + 3);
	label->SetSize(150, 17);
	pageTool->AddControl(label);

	toolSelectColor = new PickColorCtl();
	toolSelectColor->SetName(_T("toolSelectColor"));
	toolSelectColor->SetText(_T(" "));
	toolSelectColor->SetOuterStyle(OuterStyle::AutoSize, false);
	//toolSelectColor->SetOuterStyle(OuterStyle::AlwaysNoEdge, true);
	//toolSelectColor->SetOuterStyle(OuterStyle::TransparentParentErase, false);
	toolSelectColor->SetOuterStyle(OuterStyle::OuterEdge, false);
	toolSelectColor->SetLocationOffset(label, false, 0, -2);
	toolSelectColor->SetSize(48, 18);
	toolSelectColor->SetStyle(StyleType::Outer, 0, OuterStyle::AutoConfig);
	//toolSelectColor->SetBackground(colorFront);
	//toolSelectColor->SetHotBackColor(colorFront);
	toolSelectColor->MouseDown += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::Tool_MouseDown);
	//toolSelectColor->MouseMove += MouseEventHandler((Object*)this, (MouseEventFun)&IconPage::Tool_MouseMove);
	//toolSelectColor->MouseUp += MouseEventHandler((Object*)this, (MouseEventFun)&IconPage::Tool_MouseUp);
	toolSelectColor->ValueChanged += EventHandler((Object*)this, (EventFun)&MainForm::Tool_ValueChanged);
	pageTool->AddControl(toolSelectColor);

	editRGB = new TextBox();
	editRGB->SetName(_T("editRGB"));
	editRGB->SetText(_T(""));
	editRGB->SetLocationOffset(toolSelectColor, false, 2, 0);
	editRGB->SetSize(58, 17);
	editRGB->SetReadOnly(true);
	pageTool->AddControl(editRGB);
	labToolColor = new TextBox();
	labToolColor->SetName(_T("labToolColor"));
	labToolColor->SetText(_T(""));
	labToolColor->SetLocationOffset(editRGB, false, 2, 2);
	labToolColor->SetSize(120, 17);
	pageTool->AddControl(labToolColor);

	nTop += 21;
	btnRunPrg = new Button();
	btnRunPrg->SetName(_T("btnRunPrg"));
	btnRunPrg->SetLocation(2, nTop);
	btnRunPrg->SetSize(150, 17);
	btnRunPrg->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	pageTool->AddControl(btnRunPrg);

	comboPrg = new ComboBox();
	comboPrg->SetName(_T("comboPrg"));
	//comboPrg->SetComboBoxStyle(ComboBoxStyle::DropDownList);
	comboPrg->SetLocationOffset(btnRunPrg, false, 1, 0);
	comboPrg->SetSize(120, 145);
	comboPrg->SetStyle(StyleType::Outer, 0, OuterStyle::AutoConfig);
	comboPrg->SetStyle(StyleType::Inner2, 0, Inner2Style::AutoConfigListData);
	pageTool->AddControl(comboPrg);
	//btnSelFrom = new PathButton();
	//btnSelFrom->SetLocationOffset(comboPrg, false, 0, 1);
	//btnSelFrom->SetSize(15, 17);
	//btnSelFrom->IsFile = true;
	//btnSelFrom->PathControl = comboPrg;
	//btnSelFrom->tcCaption = _T("select DLL file:");
	//btnSelFrom->tcFilter = _T("EXE Files (*.exe)\0*.exe;\0\0\0\0");
	//btnSelFrom->tcDefaultExt = _T("exe");
	//pageTool->AddControl(btnSelFrom);

	btnRunAdd = new ToolItem();
	btnRunAdd->SetName(_T("btnRunAdd"));
	btnRunAdd->SetText(_T("+"));
	btnRunAdd->SetLocationOffset(comboPrg, false, 2, 1);
	btnRunAdd->SetSize(12, 10);
	btnRunAdd->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	btnRunAdd->SetOuterStyle(OuterStyle::AutoSize, false);
	pageTool->AddControl(btnRunAdd);
	btnRunDel = new ToolItem();
	btnRunDel->SetName(_T("btnRunDel"));
	btnRunDel->SetText(_T("-"));
	btnRunDel->SetLocationOffset(btnRunAdd, true, 0, 1);
	btnRunDel->SetSize(12, 10);
	btnRunDel->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	btnRunDel->SetOuterStyle(OuterStyle::AutoSize, false);
	pageTool->AddControl(btnRunDel);
	label = new Label();
	label->SetName(_T("labRunFmt"));
	label->SetText(_T("(YYYY/mm/DD HH:MM)"));
	label->SetLocationOffset(btnRunAdd, false, 4, 2);
	label->SetSize(300, 17);
	pageTool->AddControl(label);

	//editRunTime = new TextBox();
	//editRunTime->SetName(_T("editRunTime"));
	//editRunTime->SetText(_T(""));
	//editRunTime->SetLocation(104, 122);
	//editRunTime->SetSize(70, 17);
	//editRunTime->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editRunTime->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editRunTime->SetOuterStyle(OuterStyle::AutoConfig, true);
	//pageTool->AddControl(editRunTime);
	//label = new Label();
	//label->SetName(_T("labRunTime"));
	//label->SetText(_T("(YYYY/mm/DD)"));
	//label->SetLocation(176, 124);
	//label->SetSize(90, 17);

	//editRunTime2 = new TextBox();
	//editRunTime2->SetName(_T("editRunTime2"));
	//editRunTime2->SetText(_T(""));
	//editRunTime2->SetLocation(280, 122);
	//editRunTime2->SetSize(40, 17);
	//editRunTime2->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editRunTime2->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	//editRunTime2->SetOuterStyle(OuterStyle::AutoConfig, true);
	//pageTool->AddControl(editRunTime2);
	//pageTool->AddControl(label);
	//label = new Label();
	//label->SetName(_T("labRunTime"));
	//label->SetText(_T("(HH:MM)"));
	//label->SetLocation(322, 124);
	//label->SetSize(80, 17);
	//pageTool->AddControl(label);

	nTop += 21;
	label = new Label();
	label->SetName(_T("labRunTime3"));
	label->SetLocation(2, nTop + 3);
	label->SetSize(150, 17);
	pageTool->AddControl(label);
	editRunTime3 = new TextBox();
	editRunTime3->SetName(_T("editRunTime3"));
	editRunTime3->SetText(_T(""));
	editRunTime3->SetLocationOffset(label, false, 0, 0);
	editRunTime3->SetSize(40, 17);
	editRunTime3->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editRunTime3->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editRunTime3->SetOuterStyle(OuterStyle::AutoConfig, true);
	editRunTime3->SetStyleNumber(true);
	editRunTime3->SetMaxLength(3);
	pageTool->AddControl(editRunTime3);

	btnRunCancel = new Button();
	btnRunCancel->SetName(_T("btnRunCancel"));
	btnRunCancel->SetLocationOffset(editRunTime3, false, 10, 0);
	btnRunCancel->SetSize(80, 17);
	btnRunCancel->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	pageTool->AddControl(btnRunCancel);

	nTop += 21;
	Button* btnGetPWD = new Button();
	btnGetPWD->SetName(_T("btnGetPWD"));
	btnGetPWD->SetText(_T("Get PassWord"));
	btnGetPWD->SetLocation(2, nTop);
	btnGetPWD->SetSize(150, 17);
	//btnGetPWD->Click += EventHandler((Object*)this, (EventFun)&MainForm::btnGetPWD_Click);
	btnGetPWD->MouseDown += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseDown);
	btnGetPWD->MouseMove += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseMove);
	btnGetPWD->MouseUp += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseUp);
	pageTool->AddControl(btnGetPWD);

	editPassword = new TextBox();
	editPassword->SetName(_T("editPassword"));
	editPassword->SetText(_T(""));
	editPassword->SetLocationOffset(btnGetPWD, false, 1, 0);
	editPassword->SetSize(200, 17);
	editPassword->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editPassword->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editPassword->SetReadOnly(true);
	pageTool->AddControl(editPassword);

	//nTop += 25;
	//label = new Label();
	//label->SetName(_T("labVolume"));
	//label->SetLocation(2, nTop + 3);
	//label->SetSize(400, 17);
	//pageTool->AddControl(label);

	nTop = 1;
	//pageHotkey
	pageHotkey = new TabPage();
	pageHotkey->SetName(_T("pageHotkey"));
	pageHotkey->SetClientSize(347, 154);
	pageHotkey->SetContextMenu(contMenu);
	tab1->AddPage(pageHotkey);

	nTop = 1;
	editHotkeyInfo = new RichTextBox();
	editHotkeyInfo->SetName(_T("editHotkeyInfo"));
	editHotkeyInfo->SetText(_T(""));
	//editHotkeyInfo->SetTextColor(UU_INFO_COLOR);
	editHotkeyInfo->SetLocation(2, nTop);
	editHotkeyInfo->SetSize(345, 140);
	editHotkeyInfo->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editHotkeyInfo->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
	editHotkeyInfo->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	editHotkeyInfo->SetReadOnly(true);
	editHotkeyInfo->SetMultiline(true);
	editHotkeyInfo->SetStyle(StyleType::ExStyle, WS_EX_CLIENTEDGE, WS_EX_TRANSPARENT);
	pageHotkey->AddControl(editHotkeyInfo);

	nTop = 1;
	//pageHide
	pageHide = new TabPage();
	pageHide->SetName(_T("pageHide"));
	pageHide->SetClientSize(347, 154);
	pageHide->SetContextMenu(contMenu);
	tab1->AddPage(pageHide);

	label = new Label();
	label->SetName(_T("labFilter"));
	label->SetLocation(2, nTop + 2);
	label->SetSize(100, 17);
	pageHide->AddControl(label);

	//btnHideRefresh = new Button();
	//btnHideRefresh->SetName(_T("btnHideRefresh"));
	//btnHideRefresh->SetLocation(2, 20);
	//btnHideRefresh->SetSize(100, 17);
	//btnHideRefresh->Click += EventHandler((Object*)this, (EventFun)&MainForm::refreshHide_Click);
	//pageHide->AddControl(btnHideRefresh);

	btnShowAll = new Button();
	btnShowAll->SetName(_T("btnShowAll"));
	btnShowAll->SetLocation(102, nTop);
	btnShowAll->SetSize(100, 17);
	btnShowAll->Click += EventHandler((Object*)this, (EventFun)&MainForm::Control_Click);
	pageHide->AddControl(btnShowAll);

	int nLeft = 0;
	for(int i = 0; i < 15; i++)
	{
		if(i % 5 == 0)
		{
			nLeft = 2;
			nTop += 20;
		}
		else
		{
			nLeft += 72;
		}
		editFilter[i] = new TextBox();
		editFilter[i]->SetName(cc::Str(_T("editFilter")) + i);
		editFilter[i]->SetText(_T(""));
		editFilter[i]->SetLocation(nLeft, nTop);
		editFilter[i]->SetSize(70, 17);
		if(i < 5)
		{
			editFilter[i]->SetTextColor(RGB(0, 0, 255));
		}
		editFilter[i]->GotFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
		editFilter[i]->LostFocus += EventHandler((Object*)this, (EventFun)&MainForm::ChgEdit_Focus);
		editFilter[i]->SetOuterStyle(OuterStyle::AutoConfig, true);
		pageHide->AddControl(editFilter[i]);
	}

	nTop += 20;
	nLeft = 0;
	for(int i = 0; i < 5; i++)
	{
		btnGetHwnd[i] = new Button();
		btnGetHwnd[i]->SetName(cc::Str(_T("btnGetHwnd")) + i);
		btnGetHwnd[i]->SetText(_T("Pick Win"));
		btnGetHwnd[i]->SetTips(_T("Pick dynamic window."));
		btnGetHwnd[i]->SetLocation(nLeft, nTop);
		btnGetHwnd[i]->SetSize(70, 19);
		btnGetHwnd[i]->TabIndex = 17;
		btnGetHwnd[i]->MouseDown += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseDown2);
		btnGetHwnd[i]->MouseMove += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseMove2);
		btnGetHwnd[i]->MouseUp += MouseEventHandler((Object*)this, (MouseEventFun)&MainForm::GetHwnd_MouseUp2);
		pageHide->AddControl(btnGetHwnd[i]);
		nLeft += 72;
	}

	nTop += 20;
	chkHideNote = new CheckBox();
	chkHideNote->SetName(_T("chkHideNote"));
	chkHideNote->SetLocation(2, nTop);
	chkHideNote->SetSize(170, 17);
	chkHideNote->SetChecked(true);
	chkHideNote->SetOuterStyle(OuterStyle::AutoSize, true);
	chkHideNote->SetOuterStyle(OuterStyle::AutoConfig, true);
	pageHide->AddControl(chkHideNote);

	nTop += 20;
	label = new Label();
	label->SetName(_T("labHideInfo"));
	//label->SetLocation(3, 65);
	label->SetLocation(2, nTop);
	//label->SetSize(100, 145);
	label->SetSize(242, 194 - nTop);
	label->SetTextColor( RGB(255, 0, 0) );
	//label->SetStyle(StyleType::OwnerDrawText, 0xFFFFFFFF, DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS|DT_EDITCONTROL|DT_WORDBREAK);
	//label->SetStyle(StyleType::Style, 0, SS_OWNERDRAW);
	label->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	pageHide->AddControl(label);

	//chklWinList = new CheckListBox();
	//chklWinList->SetName(_T("chklWinList"));
	//chklWinList->SetLocation(105, 60);
	//chklWinList->SetSize(242, 92);
	//chklWinList->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	//pageHide->AddControl(chklWinList);

	//pageOption
	pageOption = new TabPage();
	pageOption->SetName(_T("pageOption"));
	pageOption->SetClientSize(347, 154);
	pageOption->SetContextMenu(contMenu);
	tab1->AddPage(pageOption);

	chkAutoRun = new CheckBox();
	chkAutoRun->SetName(_T("chkAutoRun"));
	chkAutoRun->SetLocation(2, 1);
	chkAutoRun->SetSize(170, 17);
	chkAutoRun->CheckedChanged += EventHandler((Object*)this, (EventFun)&MainForm::autorun_Click);
	pageOption->AddControl(chkAutoRun);

	label = new Label();
	label->SetName(_T("labLang"));
	label->SetLocation(3, 22);
	label->SetSize(70, 17);
	pageOption->AddControl(label);

	comboLang = new ComboBox();
	comboLang->SetName(_T("comboLang"));
	comboLang->SetComboBoxStyle(ComboBoxStyle::DropDownList);
	comboLang->SetLocation(73, 20);
	comboLang->SetSize(200, 145);
	//comboLang->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right);
	pageOption->AddControl(comboLang);

	btnOK = new Button();
	btnOK->SetName(_T("btnOK"));
	btnOK->SetLocationOffset(comboLang, false, 2, 0);
	btnOK->SetSize(80, 19);
	btnOK->Click += EventHandler((Object*)this, (EventFun)&MainForm::OptionOK_Click);
	pageOption->AddControl(btnOK);

	//LabelLink* lnkDonate = new LabelLink();
	//lnkDonate->SetName(_T("lnkDonate"));
	//lnkDonate->SetLocationOffset(btnOK, false, 5, 2);
	//lnkDonate->SetSize(100, 17);
	//lnkDonate->SetOuterStyle(OuterStyle::AutoSize, true);
	//pageOption->AddControl(lnkDonate);

	verInfo = new VersionInfo();
	verInfo->SetLocation(1, 44);
	verInfo->SetSize(343, 110);
	verInfo->Anchor = (AnchorStyles::Type)(AnchorStyles::Top | AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Bottom);
	pageOption->AddControl(verInfo);

	hotkey1 = new Hotkey();
	hotkey1->HotKeyDown += EventHandler((Object*)this, (EventFun)&MainForm::hotkey_Press);
	this->AddControl(hotkey1);

	statusitem1 = new StatusItem();
	statusitem1->SetName(_T("item1"));
	statusitem1->SetText(_T("Ready."));
	statusitem1->SetAutoSize(StatusItem::AutoSize::Sprint);

	statusbar = new StatusBar();
	statusbar->SetName(_T("statusbar"));
	statusbar->AddControl(statusitem1);
	this->SetStatusBar(statusbar);

	notify1 = NULL;

	//here need be 32X32
	this->SetOuterStyle(OuterStyle::AutoConfig, true);
#ifdef _DEBUG_
	this->SetIcon(imageList, IDI_ICON_ATTACH);
#else
	this->SetIcon(imageList, IDI_ICON_MAIN);
#endif
	this->Load += EventHandler((Object*)this, (EventFun)&MainForm::Form_Load);
	this->Closing += CancelEventHandler((Object*)this, (CancelEventFun)&MainForm::Form_Closing);
	this->Closed += EventHandler((Object*)this, (EventFun)&MainForm::Form_Closed);
	this->Resize += EventHandler((Object*)this, (EventFun)&MainForm::Form_Resize);
	this->Shutdown += EventHandler((Object*)this, (EventFun)&MainForm::Form_Shutdown);
	this->SetName(App::GetName());
	this->SetText(App::GetNameVersion());
	this->SetClientSize(600, 480);
	this->SetSize(450, 300);
	this->SetStyle(StyleType::Style, 0, DS_CENTER);
	this->SetConfigName(_T(""));
	this->SetMaxButtonEnable(false);

	notify1 = new NotifyIcon();
	notify1->SetText(App::GetNameVersion());
	notify1->SetIcon(imageList, IDI_ICON_MAIN);
	notify1->SetContextMenu(contMenu);
	notify1->SetShowMenu(LeftRight::Left, false);
	notify1->SetVisible(true);
	notify1->DoubleClick += EventHandler((Object*)this, (EventFun)&MainForm::showhide_Click);
	this->DisposeItemAdd(notify1);
	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
}

LRESULT MainForm::WndProc(EventArgs *e)
{
	if(e->Msg == WM_POWERBROADCAST)
	{
		cc::Str strTimeFile = cc::Util::getStartupPath();
		strTimeFile.Append(_T(".WorkTime.txt"));
		if((int)e->WParam == PBT_APMRESUMEAUTOMATIC)
		{
			cc::Str sTime = GetCurTime(_T("Work ReStart(Resume):"));
			cc::Util::fAppend(sTime, (TCHAR*)strTimeFile.GetBuf());
		}
		else if((int)e->WParam == PBT_APMSUSPEND)
		{
			cc::Str sTime = GetCurTime(_T("Work Stop(Suspend)  :"));
			cc::Util::fAppend(sTime, (TCHAR*)strTimeFile.GetBuf());
		}
	}
	/*
	else if(e->Msg == WM_DRAWCLIPBOARD)
	{
		if(hClipboard)
		{
			::SendMessage(hClipboard, WM_DRAWCLIPBOARD, 0, 0);
			
			cc::Str str;
			cc::Util::GetClipboardText(str, m_hWnd);
			if(str.GetLength() > 0)
			{
				if(lstClipboard->GetSize() > 5)
				{
				}
				//::MessageBox(m_hWnd, str, TCHAR_LANG("Lang.S_MsgBox_Title_Info"), MB_OK);
				lstClipboard.Add(str);
			}
		}
	}
	else if(e->Msg == WM_CHANGECBCHAIN)
	{
		HWND hWndRemove = (HWND)e->WParam;
		HWND hWndAfter = (HWND)e->LParam;
		if(hClipboard == hWndRemove)
		{
			hClipboard = hWndAfter;
		}
		else if(hClipboard)
		{
			::SendMessage(hClipboard, WM_CHANGECBCHAIN, (WPARAM)hWndRemove, (LPARAM)hWndAfter);
		}
	}
	*/
	else if(e->Msg == WM_WTSSESSION_CHANGE && (e->WParam == WTS_SESSION_LOGON || e->WParam == WTS_SESSION_UNLOCK))
	{
		::SetTimer(this->m_hWnd, UU_TIMERID_UPDATE, 1000 * 6, NULL);
	}
	return Form::WndProc(e);
}

void MainForm::Tool_MouseDown(Object* sender, MouseEventArgs* e)
{
	if(e->Button == MouseButtons::Left)
	{
		toolColorStart = 0;
		labToolColor->SetText(_T("wait a while..."));
		::SetTimer(this->m_hWnd, UU_TIMERID_PICKCOLOR, 3000, NULL);
	}
}
void MainForm::Tool_ValueChanged(Object* sender, EventArgs* e)
{
	COLORREF cor = toolSelectColor->GetBackground();
	cc::Str str;
	str.Format(_T("#%02x%02x%02x"), GetRValue(cor), GetGValue(cor), GetBValue(cor));
	editRGB->SetText((TCHAR*)str.GetBuf());

	//wait for a while to start
	if(toolColorStart == 0)
	{
		return;
	}
	if(toolColorStart == 1)
	{
		toolColorStart = 2;
		toolColorMin = cor;
		toolColorMax = cor;
	}
	int r = GetRValue(toolColorMin);
	int g = GetGValue(toolColorMin);
	int b = GetBValue(toolColorMin);
	if(r > GetRValue(cor))
	{
		r = GetRValue(cor);
	}
	if(g > GetRValue(cor))
	{
		g = GetRValue(cor);
	}
	if(b > GetRValue(cor))
	{
		b = GetRValue(cor);
	}
	toolColorMin = RGB(r, g, b);

	r = GetRValue(toolColorMax);
	g = GetGValue(toolColorMax);
	b = GetBValue(toolColorMax);
	if(r < GetRValue(cor))
	{
		r = GetRValue(cor);
	}
	if(g < GetRValue(cor))
	{
		g = GetRValue(cor);
	}
	if(b < GetRValue(cor))
	{
		b = GetRValue(cor);
	}
	toolColorMax = RGB(r, g, b);
	str.Format(_T("(Min:#%02x%02x%02x, Max:#%02x%02x%02x)"), GetRValue(toolColorMin), GetGValue(toolColorMin), GetBValue(toolColorMin)
		, GetRValue(toolColorMax), GetGValue(toolColorMax), GetBValue(toolColorMax));
	labToolColor->SetText(str);
}

void MainForm::DoSetLang(KeyArr& keyArr, cc::Str sThisHead)
{
	Form::DoSetLang(keyArr, sThisHead);
	contMenu->SetLang(keyArr, sThisHead);
	if(formCalendar != NULL)
	{
		formCalendar->SetLang(keyArr, CC_CONFIG_LANGSECTION);
	}
	if(captureDialog != NULL)
	{
		captureDialog->SetLang(keyArr, CC_CONFIG_LANGSECTION);
	}
	cc::Str s1 = keyArr.Get(_T("Lang.pageHide.editFilter1Tips"));
	cc::Str s2 = keyArr.Get(_T("Lang.pageHide.editFilter2Tips"));
	for(int i = 0; i < 15; i++)
	{
		if(i < 5)
		{
			editFilter[i]->SetTips(s1);
		}
		else
		{
			editFilter[i]->SetTips(s2);
		}
	}

}

void MainForm::Form_Load(Object* sender, EventArgs* e)
{
	//has flash, not good
	//if not visible,minimize win,so has min memory need.
	//if(!CC_APP_GetIni()->GetBool(_T("Main.Visible"), true))
	//{
	//	SetVisible(true);
	//	::ShowWindow(m_hWnd, SW_MINIMIZE);
	//	::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE, 300, NULL);
	//	::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE2, 3000, NULL);
	//}

	if(volumeDialog == NULL)
	{
		volumeDialog = new StepDialog();
		volumeDialog->SetVisible(false);
		volumeDialog->CreateControl();
	}
	if(transparentDialog == NULL)
	{
		transparentDialog = new StepDialog();
		transparentDialog->SetVisible(false);
		transparentDialog->CreateControl();
	}
	CC_TRACE((_T("3, Form_Load, volumeDialog:%d, transparentDialog:%d"), volumeDialog, transparentDialog));

	hotkey1->Register(MOD_SHIFT, VK_ESCAPE);//ESC
	hotkey1->Register(MOD_WIN, VK_ESCAPE);//ESC
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_UP);//Up
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_DOWN);//Down
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_LEFT);//Left
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_RIGHT);//Right
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_SPACE);//space
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_F11);
	hotkey1->Register(MOD_WIN | MOD_CONTROL, VK_F12);
	hotkey1->Register(MOD_SHIFT | MOD_CONTROL, 'V');
	
	//hClipboard = ::SetClipboardViewer(m_hWnd);

	cc::Str strTimeFile = cc::Util::getStartupPath();
	strTimeFile.Append(_T(".WorkTime.txt"));
	cc::Str sTime = GetCurTime(_T("Start:"));
	cc::Util::fAppend(sTime, (TCHAR*)strTimeFile.GetBuf());

	//refreshtime_Click(NULL, NULL);
	const TCHAR* tcKeySub = cc::win::App::GetName();
	registryKey1->OpenSubKey(tcKey_RUN, true);
	cc::Str val = registryKey1->GetValue(tcKeySub);
	registryKey1->Close();
	cc::Str sStartPath = cc::Util::getStartupPath();
	if(val.Equals(sStartPath))
	{
		chkAutoRun->SetChecked(true);
	}

	//get all lang(res and files) and set to item
	cc::win::WinUtil::AddLangItems(comboLang, true, true);

	if(CC_APP_GetIni()->GetBool(_T("Main.pageNote.chkMirco"), false))
	{
		chkMirco->SetChecked(true);
	}
	LoadAllNoteW();

	//load config
	this->Config(true, *CC_APP_GetIni(), cc::Str(_T("Main")));
	ConfigBackDlg::ConfigResult(this->GetClient(), 0);
	toolOnTop_Click(this, NULL);
	SetEditMenuEnable(false);
	if(!CC_APP_GetIni()->GetBool(_T("Main.ShowCommTool"), true))
	{
		menuCommTool_Click(this, NULL);
	}
	if(!CC_APP_GetIni()->GetBool(_T("Main.ShowEditTool"), true))
	{
		menuEditTool_Click(this, NULL);
	}
	if(CC_APP_GetIni()->GetBool(_T("Main.FixTools"), false))
	{
		menuFixTools_Click(this, NULL);
	}
	if(!CC_APP_GetIni()->GetBool(_T("Main.ShowTips"), true))
	{
		menuTips_Click(this, NULL);
	}
	if(!CC_APP_GetIni()->GetBool(_T("Main.ShowStatusBar"), true))
	{
		menuStatusBar_Click(this, NULL);
	}
	btnRunCancel->SetEnabled(false);
	if(chkCalendar->GetChecked())
	{
		Control_Click(chkCalendar, NULL);
	}
	//UpdateMenuNoteList();

	//WTSRegisterSessionNotification(m_hWnd, NOTIFY_FOR_THIS_SESSION);
	typedef BOOL (WINAPI *WTSREGISTERSESSIONNOTIFICATION)(HWND, DWORD);
	HANDLE hWtsLib = LoadLibrary(_T("wtsapi32.dll"));
	WTSREGISTERSESSIONNOTIFICATION funReg;
    funReg = (WTSREGISTERSESSIONNOTIFICATION)GetProcAddress((HMODULE)hWtsLib, "WTSRegisterSessionNotification");
    if(funReg != NULL)
    {
        funReg(m_hWnd, NOTIFY_FOR_THIS_SESSION);
    }
}

void MainForm::Form_Resize(Object* sender, EventArgs* e)
{
	//hide main window
	if(e != NULL && e->WParam == SIZE_MINIMIZED)
	{
		//refresh
		if(notify1 != NULL)
		{
			notify1->SetText(App::GetNameVersion());
			notify1->SetVisible(true);
		}
		::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE, 300, NULL);
		::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE2, 3000, NULL);
	}
}

void MainForm::Form_Closing(Object* sender, CancelEventArgs* e)
{
	if(e->baseArgs->LParam == 1 && e->baseArgs->WParam == 1) {
		//exit from menu
		return;
	}
	::ShowWindow(m_hWnd, SW_MINIMIZE);
	::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE, 300, NULL);
	::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE2, 3000, NULL);
	//SetVisible(false);
	//not close but goto NotifyIcon
	e->Cancel = true;

	//save position
	//this->Config(false, *CC_APP_GetIni(), cc::Str(_T("Main")));
	//save config while window down
	//CC_APP_GetConfig()->SaveIni();
}

void MainForm::Form_Closed(Object* sender, EventArgs* e)
{
	CC_TRACE((_T("MainForm::Form_Closed Start.")));

	OnInnerTimer(NULL);
	//ChangeClipboardChain(m_hWnd, hClipboard);
	if(hWndHide != NULL)
	{
		::ShowWindow(hWndHide, SW_SHOW);
		hWndHide = NULL;
	}
	if(volumeDialog != NULL)
	{
		volumeDialog->Dispose();
		delete volumeDialog;
		volumeDialog = NULL;
	}
	if(transparentDialog != NULL)
	{
		transparentDialog->Dispose();
		delete transparentDialog;
		transparentDialog = NULL;
	}

	//do some save
	Form_Shutdown(sender, e);

	if(lastNote != NULL)
	{
		delete lastNote;
		lastNote = NULL;
	}
	ShowAllHideW(SW_SHOW);
	for(int n = 0; n < lstWinTrans->GetSize(); n++)
	{
		HWND hWnd = lstWinTrans->GetKey(n);
		if(::IsWindow(hWnd))
		{
			cc::Util::SetTransparent(hWnd, 255);
		}
	}
	lstWinTrans->Clear();
	//delete lstWin;
	//lstWin = NULL;
	//delete lstWinHide;
	//lstWinHide = NULL;
	delete lstWinHideSave;
	lstWinHideSave = NULL;
	delete lstWinTrans;
	lstWinTrans = NULL;
	delete noteDefault;
	noteDefault = NULL;
	delete noteWKTime;
	noteWKTime = NULL;
	for(int i = lstClipboard->GetSize() - 1; i>= 0; i--)
	{
		cc::Str str = lstClipboard->GetKey(i);
		str.Clear();
		lstClipboard->RemoveAt(i);
	}
	delete lstClipboard;
	lstClipboard = NULL;
	delete notePWD;
	notePWD = NULL;

	cc::Str sTime = GetCurTime(_T("End  :"));
	cc::Str strTimeFile = cc::Util::getStartupPath();
	strTimeFile.Append(_T(".WorkTime.txt"));
	cc::Util::fAppend(sTime, (TCHAR*)strTimeFile.GetBuf());

	if(pShareMemory != NULL)
	{
		::UnmapViewOfFile(pShareMemory);
		::CloseHandle(pShareMemoryMap);
		pShareMemory = NULL;
		pShareMemoryMap = NULL;
	}

	CC_TRACE((_T("MainForm::Form_Closed End.")));
}

void MainForm::Form_Shutdown(Object* sender, EventArgs* e)
{
	//save sth.
	if(captureDialog != NULL)
	{
		RECT rect = captureDialog->GetBounds();
		CC_APP_GetIni()->Set(_T("CaptureDlg.Left"), rect.left);
		CC_APP_GetIni()->Set(_T("CaptureDlg.Top"), rect.top);
		captureDialog->Dispose();
		delete captureDialog;
		captureDialog = NULL;
	}
	if(formCalendar != NULL)
	{
		formCalendar->Dispose();
		delete formCalendar;
		formCalendar = NULL;
	}

	CC_TRACE((_T("UU_MAINFLAG_EXIT:%d"), CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_EXIT)));
	if(CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_EXIT))
	{
		return;
	}

	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_EXIT);
	SaveAllNoteW(true);

	//save position
	this->Config(false, *CC_APP_GetIni(), cc::Str(_T("Main")));

	//save config while window down
	CC_APP_GetConfig()->SaveIni();

	int nCnt = lstNote->GetSize();
	for(int i = 0; i < nCnt; i++)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(noteTmp->note != NULL && ::IsWindow(noteTmp->note->m_hWnd))
		{
			::SendMessage(noteTmp->note->m_hWnd, WM_CLOSE, NULL, NULL);
		}
		if(noteTmp->note != NULL)
		{
			noteTmp->note->Dispose();
			delete noteTmp->note;
			noteTmp->note = NULL;
		}
		delete noteTmp;
	}
	delete lstNote;
	lstNote = NULL; 
	CC_TRACE((_T("CC_APP_GetConfig()->SaveIni()")));
}

void MainForm::Control_Click(Object* sender, EventArgs* e)
{
	if(sender == btnShowAll)
	{
		ShowAllHideW(SW_SHOW);
		chkHideNote->SetEnabled(true);
		//btnHideRefresh->SetEnabled(true);
		//chklWinList->SetEnabled(true);
	}
	//else if(sender == btnShowPassword || sender == menuPassword)
	//{
	//	if(password == NULL)
	//	{
	//		password = new Password();
	//		password->SetLocation(CC_APP_GetIni()->Get(_T("PasswordDlg.Left"), 300), CC_APP_GetIni()->Get(_T("PasswordDlg.Top"), 300));
	//	}
	//	password->Show();
	//}
	else if(sender == menuCapture)
	{
		chkShowCapture->SetChecked(true);
		Control_Click(chkShowCapture, NULL);
	}
	else if(sender == chkShowCapture)
	{
		if(captureDialog != NULL)
		{
			captureDialog->Dispose();
			delete captureDialog;
			captureDialog = NULL;
		}
		if(chkShowCapture->GetChecked())
		{
			captureDialog = new CaptureDialog(this->m_hWnd);
			captureDialog->SetLocation(CC_APP_GetIni()->Get(_T("CaptureDlg.Left"), 300), CC_APP_GetIni()->Get(_T("CaptureDlg.Top"), 300));
			captureDialog->CreateControl();
			::SetWindowPos(captureDialog->m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}
	}
	else if(sender == menuCalendar)
	{
		chkCalendar->SetChecked(true);
		Control_Click(chkCalendar, NULL);
		::SetTimer(this->m_hWnd, UU_TIMERID_CALENDAR, 100, NULL);
	}
	else if(sender == chkCalendar)
	{
		if(formCalendar != NULL)
		{
			formCalendar->Dispose();
			delete formCalendar;
			formCalendar = NULL;
		}
		if(chkCalendar->GetChecked())
		{
			formCalendar = new CMainForm(this->m_hWnd);
			//formCalendar->SetVisible(chkCalendar->GetChecked());
			formCalendar->Show();
		}
	}
	else if(sender == menuWorkTime)
	{
		opentime_Click(sender, e);
	}
	else if(sender == btnRunAdd)
	{
		cc::Str str = comboPrg->GetText();
		if(str.GetLength() > 0 && comboPrg->FindStringExact(0, str) < 0)
		{
			comboPrg->ItemAdd(str);
		}
	}
	else if(sender == btnRunDel)
	{
		cc::Str str = comboPrg->GetText();
		comboPrg->ItemDel(str);
	}
	else if(sender == btnRunPrg)
	{
		cc::Str str = comboPrg->GetText();
		str.Trim();
		if(str.GetLength() < 16)
		{
			::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_TimeErrFmt"), TCHAR_LANG("Lang.S_MsgBox_Title_Info"), MB_OK);
			return;
		}
		int nReTime = editRunTime3->GetTextInt();
		if(nReTime < 1 || nReTime > 1000)
		{
			::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_TimeErrFmt2"), TCHAR_LANG("Lang.S_MsgBox_Title_Info"), MB_OK);
			return;
		}
		const TCHAR* tcstr = (const TCHAR*)str.GetBuf();
		//YYYY/mm/DD HH:MM
		if(tcstr[4] != _T('/') || tcstr[7] != _T('/') || tcstr[10] != _T(' ') || tcstr[13] != _T(':'))
		{
			::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_TimeErrFmt"), TCHAR_LANG("Lang.S_MsgBox_Title_Info"), MB_OK);
			return;
		}
		SYSTEMTIME st;
		st.wSecond = 0;
		st.wMilliseconds = 0;
		st.wYear = str.Substring(0, 4).ToInt();
		st.wMonth = str.Substring(5, 7).ToInt();
		st.wDay = str.Substring(8, 10).ToInt();
		st.wHour = str.Substring(11, 13).ToInt();
		st.wMinute = str.Substring(14, 16).ToInt();
		CC_TRACE((_T("nY:%d, nm:%d, nD:%d, nH:%d, nM:%d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute));
		if(st.wYear < 1600 || st.wMonth < 1 || st.wMonth > 12 || st.wDay < 1 || st.wDay > 31 || st.wHour < 0 || st.wHour > 23 || st.wMinute < 0 || st.wMinute > 59)
		{
			::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_TimerErrFmt"), TCHAR_LANG("Lang.S_MsgBox_Title_Info"), MB_OK);
			return;
		}
		if(IDYES == (int)::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_SetTime"), TCHAR_LANG("Lang.S_MsgBox_Title_Question"), MB_YESNO | MB_ICONQUESTION))
		{
			//save system time
			::GetLocalTime(&stSaveTime);
			::SetLocalTime(&st);
			::GetLocalTime(&stNewTime);
			::SetTimer(this->m_hWnd, UU_TIMERID_UPDATE, nReTime * 1000, NULL);
			SetStatus(TCHAR_LANG("Lang.U_TimeSet"));
			btnRunPrg->SetEnabled(false);
			btnRunCancel->SetEnabled(true);
		}
	}
	else if(sender == btnRunCancel)
	{
		::KillTimer(m_hWnd, UU_TIMERID_UPDATE);
		::SetTimer(this->m_hWnd, UU_TIMERID_UPDATE, 10, NULL);
	}
}

void MinusFileTime(FILETIME *pFileTime, FILETIME *pFileTimeMinus)
{
	if(pFileTime->dwLowDateTime >= pFileTimeMinus->dwLowDateTime && pFileTime->dwHighDateTime >= pFileTimeMinus->dwHighDateTime)
	{
		//High and Low is biger than second,-
		pFileTime->dwHighDateTime -= pFileTimeMinus->dwHighDateTime;
		pFileTime->dwLowDateTime -= pFileTimeMinus->dwLowDateTime;
	}
	else if(pFileTime->dwLowDateTime < pFileTimeMinus->dwLowDateTime && pFileTime->dwHighDateTime > pFileTimeMinus->dwHighDateTime)
	{
		//Low is small than second but High is biger than second,High--
		DWORD dwTmpLowDataTime = pFileTimeMinus->dwLowDateTime - pFileTime->dwLowDateTime;
		pFileTime->dwHighDateTime -= pFileTimeMinus->dwHighDateTime;
		pFileTime->dwHighDateTime -= 1;
		pFileTime->dwLowDateTime = 0xFFFFFFFF - dwTmpLowDataTime + 1;
	}
}
void AddFileTime(FILETIME *pFileTime, FILETIME *pFileTimeAdd)
{
	DWORD dwTmpLowDataTime = pFileTime->dwLowDateTime;
	pFileTime->dwLowDateTime += pFileTimeAdd->dwLowDateTime;
	if(pFileTime->dwLowDateTime < dwTmpLowDataTime)
	{
		pFileTime->dwHighDateTime += 1;
	}
}

void MainForm::OnInnerTimer(EventArgs *e)
{
	//e->WParam is ID
	if(e != NULL && e->WParam == UU_TIMERID_MIN_HIDE)
	{
		if(GetWindowState() == FormWindowState::Minimized)
		{
			::KillTimer(m_hWnd, UU_TIMERID_MIN_HIDE);
			SetVisible(false);
			//save state
			GetWindowState();
		}
	}
	if(e != NULL && e->WParam == UU_TIMERID_MIN_HIDE2)
	{
		::KillTimer(m_hWnd, UU_TIMERID_MIN_HIDE);
		::KillTimer(m_hWnd, UU_TIMERID_MIN_HIDE2);
	}
	if(e == NULL || e->WParam == UU_TIMERID_UPDATE)
	{
		if(stSaveTime.wYear != 0 && stSaveTime.wMinute != 0)
		{
			if(e != NULL)
			{
				::KillTimer(m_hWnd, UU_TIMERID_UPDATE);
			}
			//restore system time
			SYSTEMTIME stNewTime2;
			FILETIME ftSaveTime;
			FILETIME ftNewTime;
			FILETIME ftNewTime2;
			SystemTimeToFileTime(&stSaveTime, &ftSaveTime);
			SystemTimeToFileTime(&stNewTime, &ftNewTime);
			::GetLocalTime(&stNewTime2);
			SystemTimeToFileTime(&stNewTime2, &ftNewTime2);

			//ftSaveTime = ftSaveTime + (ftNewTime2 - ftNewTime);
			MinusFileTime(&ftNewTime2, &ftNewTime);
			AddFileTime(&ftSaveTime, &ftNewTime2);

			FileTimeToSystemTime(&ftSaveTime, &stSaveTime);
			CC_TRACE((_T("nY:%d, nm:%d, nD:%d, nH:%d, nM:%d"), stSaveTime.wYear, stSaveTime.wMonth, stSaveTime.wDay, stSaveTime.wHour, stSaveTime.wMinute));
			::SetLocalTime(&stSaveTime);
			if(e != NULL)
			{
				SetStatus(TCHAR_LANG("Lang.U_TimeRestore"));
				btnRunPrg->SetEnabled(true);
				btnRunCancel->SetEnabled(false);
			}
			ZeroMemory(&stSaveTime, sizeof(SYSTEMTIME));
			ZeroMemory(&stNewTime, sizeof(SYSTEMTIME));
		}
		return;
	}
	if(e->WParam == UU_TIMERID_WFLASH)
	{
		nFlashTime -= 50;
		nFlashTime--;
		if(nFlashTime <= 0)
		{
			FlashWindow(hWndFlash, TRUE);
			::KillTimer(m_hWnd, UU_TIMERID_WFLASH);
			hWndFlash = NULL;
		}
		if(hWndFlash != NULL) {
			FlashWindow(hWndFlash, nFlashTime%2==0 ? TRUE : FALSE);
		}
		return;
	}
	if(e->WParam == UU_TIMERID_AUTORUNCHK)
	{
		::KillTimer(m_hWnd, UU_TIMERID_AUTORUNCHK);
		cc::Str strPathFile = cc::Util::getStartupPath();
		const TCHAR* tcKeySub = cc::win::App::GetName();
		registryKey1->OpenSubKey(tcKey_RUN, true);
		cc::Str val = registryKey1->GetValue(tcKeySub);
		registryKey1->Close();
		bool isAuto = chkAutoRun->GetChecked();
		if(isAuto)
		{
			if(!val.Equals(strPathFile))
			{
				::MessageBox(m_hWnd, CC_APP_GetLang()->Get(_T("Lang.U_NOTE_MSG_Need_Sup"), _T("Can't access registry, Need run as administrator")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Info")), MB_OK|MB_ICONSTOP);
				chkAutoRun->SetChecked(false);
			}
		}
		else
		{
			if(val.Equals(strPathFile))
			{
				::MessageBox(m_hWnd, CC_APP_GetLang()->Get(_T("Lang.U_NOTE_MSG_Need_Sup"), _T("Can't access registry, Need run as administrator")), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Info")), MB_OK|MB_ICONSTOP);
				chkAutoRun->SetChecked(true);
			}
		}
		return;
	}
	if(e->WParam == UU_TIMERID_CALENDAR)
	{
		::KillTimer(m_hWnd, UU_TIMERID_CALENDAR);
		SendMessage(formCalendar->m_hWnd, WM_ACTIVATE, WA_ACTIVE, 0);
		return;
	}
	if(e->WParam == UU_TIMERID_PICKCOLOR)
	{
		::KillTimer(m_hWnd, UU_TIMERID_PICKCOLOR);
		toolColorStart = 1;
		labToolColor->SetText(_T("Start..."));
		return;
	}
	if(e->WParam == UU_TIMERID_CALENDAR_EXIT)
	{
		::KillTimer(m_hWnd, UU_TIMERID_CALENDAR_EXIT);
		if(formCalendar != NULL)
		{
			formCalendar->Dispose();
			delete formCalendar;
			formCalendar = NULL;
		}
		chkCalendar->SetChecked(false);
		return;
	}
	if(e->WParam == UU_TIMERID_CAPTURE_EXIT)
	{
		::KillTimer(m_hWnd, UU_TIMERID_CAPTURE_EXIT);
		if(captureDialog != NULL)
		{
			captureDialog->Dispose();
			delete captureDialog;
			captureDialog = NULL;
		}
		chkShowCapture->SetChecked(false);
		return;
	}
	if(e->WParam == UU_TIMERID_ONTOP)
	{
		::KillTimer(m_hWnd, UU_TIMERID_ONTOP);

		int nCnt = lstNote->GetSize();
		for(int i = 0; i < nCnt; i++)
		{
			noteTmp = (NoteEntity*)lstNote->GetKey(i);
			if(noteTmp->note != NULL && ::IsWindow(noteTmp->note->m_hWnd))
			{
				::SetWindowPos(noteTmp->note->m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
		}
		::SetTimer(this->m_hWnd, UU_TIMERID_ONTOP2, 100, NULL);
		return;
	}
	if(e->WParam == UU_TIMERID_ONTOP2)
	{
		::KillTimer(m_hWnd, UU_TIMERID_ONTOP2);

		int nCnt = lstNote->GetSize();
		for(int i = 0; i < nCnt; i++)
		{
			noteTmp = (NoteEntity*)lstNote->GetKey(i);
			if(noteTmp->note != NULL && ::IsWindow(noteTmp->note->m_hWnd))
			{
				//::SendMessage(noteTmp->note->m_hWnd, WM_ACTIVATE, NULL, NULL);
				::SetWindowPos(noteTmp->note->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
		}
		return;
	}
	Form::OnInnerTimer(e);
}

void MainForm::NoteChangeTitle(NoteBaseForm* note, cc::Str& sTxt)
{
	if(CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_EXIT) || !IsWindow(note->m_hWnd))
	{
		//may from SaveToFile while shutdown!
		return;
	}
	int nInd = GetNoteIndex(note);
	if(nInd < 0)
	{
		return;
	}
	cc::Str str = cc::Str(LangCode::TCHAR, sTxt, 0, 100);
	str.Replace(_T("\r"), _T("")).Replace(_T("\n"), _T("")).Trim();
	if(str.GetLength() < 1)
	{
		if(note->GetMode() == MODE_IE)
		{
			str = cc::Str(_T("about:blank"));
		}
		else
		{
			str = CC_APP_GetLang()->Get(_T("Lang.U_NOTE_CTL_NoTitle"));
		}
	}
	ltbNoeForm->ItemSet(nInd, (TCHAR*)str.GetBuf());
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
}

void MainForm::SaveAllNoteW(bool isExit)
{
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
	int nCnt = lstNote->GetSize();
	CC_TRACE((_T("MainForm::SaveAllNoteW Start. isExit:%d, nCnt:%d"), isExit, nCnt));

	int size = (int)sizeof(NoteEntityHeader) + (int)(sizeof(NoteEntity) * (nCnt+2));
	void *buf = malloc(size);
	memset(buf, '\0', size);
	NoteEntityHeader *header = (NoteEntityHeader*)buf;

	header->headerflag[0] = 'U';
	header->headerflag[1] = 'W';
	//save count
	header->noteCount = nCnt;
	//save PWD
	if(notePWD->GetLength() > 0)
	{
		//_tcscpy(allEntity->tcFileName, (TCHAR*)notePWD->GetBuf());
		cc::Buf::Cpy(header->notePwd, NOTE_PWD_MAXLEN, (TCHAR*)notePWD->GetBuf());
	}

	memcpy((char*)buf + sizeof(NoteEntityHeader), noteDefault, sizeof(NoteEntity));
	memcpy((char*)buf + sizeof(NoteEntityHeader) + sizeof(NoteEntity), noteWKTime, sizeof(NoteEntity));

	for(int i = 0; i < nCnt; i++)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		memcpy((char*)buf + sizeof(NoteEntityHeader) + sizeof(NoteEntity) * (i+2), noteTmp, sizeof(NoteEntity));
	}
	cc::Str path = CC_APP_GetIni()->Get(NOTEFILE_PATH_KEY, NOTEFILE_PATH_DEFAULT);
	path = cc::Util::getStartupRelatePath((TCHAR*)path.GetBuf());
	path.Append(_T("note.bin"));
	cc::Util::fWrite((const void*)buf, size, LangCode::SYSTEM, (TCHAR*)path.GetBuf());
	free(buf);
	CC_TRACE((_T("MainForm::SaveAllNoteW End.")));
}

void MainForm::LoadAllNoteW()
{
	CC_TRACE((_T("MainForm::LoadAllNoteW Start.")));
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
	cc::Str path = CC_APP_GetIni()->Get(NOTEFILE_PATH_KEY, NOTEFILE_PATH_DEFAULT);
	if(path.GetLength() < 1)
	{
		path = cc::Str(NOTEFILE_PATH_DEFAULT);
		CC_APP_GetIni()->Set(NOTEFILE_PATH_KEY, (TCHAR*)path.GetBuf());
	}
	if(!path.EndsWith(_T("\\")))
	{
		path.Append(_T("\\"));
		CC_APP_GetIni()->Set(NOTEFILE_PATH_KEY, (TCHAR*)path.GetBuf());
	}
	path = cc::Util::getStartupRelatePath((TCHAR*)path.GetBuf());
	if(!cc::Util::dExist((TCHAR*)path.GetBuf()))
	{
		cc::Util::dCreate((TCHAR*)path.GetBuf());
	}
	cc::Str savePath = path;
	path.Append(_T("note.bin"));

	int nCnt = 0;
	void* buf = NULL;
	int nRead = cc::Util::fRead(&buf, (TCHAR*)path.GetBuf());
	if(nRead >= (int)sizeof(NoteEntityHeader) + (int)(sizeof(NoteEntity) * 2) && ((char*)buf)[0] == 'U' && ((char*)buf)[1] == 'W')
	{
		//Get count
		NoteEntityHeader *header = (NoteEntityHeader*)buf;
		nCnt = header->noteCount;
		if(nCnt > 0 && nRead == (int)sizeof(NoteEntityHeader) + (int)(sizeof(NoteEntity) * (nCnt + 2)))
		{
			//Get PWD
			notePWD->ValueOf(header->notePwd);

			memcpy(noteDefault, ((char*)buf) + sizeof(NoteEntityHeader), sizeof(NoteEntity));
			noteDefault->note = NULL;
			memcpy(noteWKTime, ((char*)buf) + sizeof(NoteEntityHeader) + sizeof(NoteEntity), sizeof(NoteEntity));
			noteWKTime->note = NULL;

			for(int i = 0; i < nCnt; i++)
			{
				noteTmp = new NoteEntity();
				memcpy(noteTmp, ((char*)buf) + sizeof(NoteEntityHeader) + sizeof(NoteEntity) * (i+2), sizeof(NoteEntity));
				noteTmp->note = NULL;
				if(noteTmp->nMode != MODE_IE || (noteTmp->tcURL[0] != _T('\0') && _tcscmp(_T(""), noteTmp->tcURL) != 0 && _tcscmp(_T("about:blank"), noteTmp->tcURL) != 0))
				{
					lstNote->Add(noteTmp);
				}
			}
		}
		else
		{
			nCnt = 0;
		}
	}
	if(buf != NULL)
	{
		free(buf);
		buf = NULL;
	}

	nCnt = lstNote->GetSize();
	CC_TRACE((_T("MainForm::LoadAllNoteW, nCnt:%d"), nCnt));
	for(int i = 0; i < nCnt; i++)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(!CC_BIT_HAS(noteTmp->dwFlag, NOTE_FLAG_CLOSE))
		{
			if(noteTmp->nMode == MODE_IE)
			{
				ltbNoeForm->ItemAdd(noteTmp->tcURL);
				noteTmp->note = new MiniIEForm();
			}
			else
			{
				ltbNoeForm->ItemAdd(CC_APP_GetLang()->Get(_T("Lang.U_NOTE_CTL_NoTitle")));
				noteTmp->note = new NoteForm();
			}
			noteTmp->note->SetVisible(false);
			noteTmp->note->CreateControl();
			noteTmp->note->SetVisible(true);
		}
		else
		{
			if(noteTmp->nMode == MODE_IE)
			{
				ltbNoeForm->ItemAdd(noteTmp->tcURL);
			}
			else
			{
				cc::Str sTxt(LangCode::TCHAR);
				path = cc::Util::getStartupRelatePath(noteTmp->tcFileName);
				if(cc::Util::fExist((TCHAR*)path.GetBuf()))
				{
					cc::Util::fRead(sTxt, (TCHAR*)path.GetBuf(), LangCode::TCHAR, 300);
				}
				sTxt = cc::Str(LangCode::TCHAR, sTxt, 0, 100);
				sTxt.Replace(_T("\r"), _T("")).Replace(_T("\n"), _T("")).Trim();
				if(sTxt.GetLength() > 0)
				{
					ltbNoeForm->ItemAdd((TCHAR*)sTxt.GetBuf());
				}
				else
				{
					ltbNoeForm->ItemAdd(CC_APP_GetLang()->Get(_T("Lang.U_NOTE_CTL_NoTitle")));
				}
			}
		}
	}
	if(nCnt == 0)
	{
		//load all text file from dir of note
		cc::List<cc::Str> arrFullName;
		savePath.Append(_T("*.txt"));
		int nCnt2 = cc::Util::getFiles(arrFullName, (TCHAR*)savePath.GetBuf(), true);
		cc::Str notepath = CC_APP_GetIni()->Get(NOTEFILE_PATH_KEY, NOTEFILE_PATH_DEFAULT);
		for(int i = 0; i < nCnt2; i++)
		{
			NewNote(notepath + arrFullName.GetKey(i));
		}
	}
	if(ltbNoeForm->GetSelectedIndex() < 0)
	{
		ltbNoeForm->SetSelectedIndex(0);
	}
	CC_TRACE((_T("MainForm::LoadAllNoteW End.")));
}

void MainForm::SetLangAllNoteW()
{
	for(int i = lstNote->GetSize() - 1; i >= 0; i--)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(noteTmp->note != NULL && !noteTmp->note->GetIsDispose())
		{
			noteTmp->note->SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
		}
	}
	if(noteWKTime->note != NULL && !noteWKTime->note->GetIsDispose())
	{
		noteWKTime->note->SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
	}
}

int MainForm::GetNoteIndex(NoteBaseForm* note)
{
	for(int i = 0; i < lstNote->GetSize(); i++)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(noteTmp->note == note)
		{
			return i;
		}
	}
	return -1;
}

NoteEntity* MainForm::GetNoteEntity(NoteBaseForm* note)
{
	int nIndex = GetNoteIndex(note);
	return (NoteEntity*)lstNote->GetKey(nIndex);
}

NoteEntity* MainForm::GetNoteDefaultEntity()
{
	return noteDefault;
}

NoteEntity* MainForm::GetNoteWKTimeEntity()
{
	return noteWKTime;
}

const cc::Str& MainForm::GetNotePWD()
{
	return *notePWD;
}

cc::Str MainForm::GetNewNoteFile()
{
	struct tm *tm;
	time_t t;
	t = time(NULL);
#ifdef __STDC_SECURE_LIB__
	struct tm ptmTemp1;
	localtime_s(&ptmTemp1, &t);
	tm = &ptmTemp1;
#else
	tm = localtime(&t);
#endif
	cc::Str path = CC_APP_GetIni()->Get(NOTEFILE_PATH_KEY, NOTEFILE_PATH_DEFAULT);
	int n = 0;
	cc::Str fileName;
	cc::Str fileTxt;
	fileTxt.Format(_T("%04d/%02d/%02d %02d:%02d\r\n\r\n"),
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
	fileName.Format(_T("%s%04d%02d%02d_%02d%02d.txt"), (TCHAR*)path.GetBuf(),
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);

	while(  cc::Util::fExist( (TCHAR*)cc::Util::getStartupRelatePath((TCHAR*)fileName.GetBuf()).GetBuf() )  )
	{
		n++;
		fileName.Format(_T("%s%04d%02d%02d_%02d%02d_%d.txt"), (TCHAR*)path.GetBuf(),
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, n);
	}
	cc::Util::fAppend(fileTxt, (TCHAR*)cc::Util::getStartupRelatePath((TCHAR*)fileName.GetBuf()).GetBuf());
	return fileName;
}

void MainForm::NoteNew_Click(Object* sender, EventArgs* e)
{
	NewNote();
}

void MainForm::contMenu_Popup(Object* sender, EventArgs* e)
{
	if(!CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_UPDNOTE))
	{
		return;
	}
	menuShowNote->ItemClear();
	int nCnt = ltbNoeForm->ItemCount();
	for(int i = 0; i < nCnt; i++)
	{
		cc::Str str = ltbNoeForm->ItemGet(i);
		MenuItem* item = new MenuItem();
		item->SetName(_T("menuShowNote"));
		item->SetText(str);
		item->Click += EventHandler((Object*)this, (EventFun)&MainForm::NoteShow_Click);
		menuShowNote->ItemAdd(item);
	}
}
void MainForm::NoteShow_Click(Object* sender, EventArgs* e)
{
	MenuItem* item = (MenuItem*)sender;
	int nInd = item->GetIndex();
	ltbNoeForm->SetSelectedIndex(nInd);
	NoteShowHide_Click(sender, NULL);
}

void MainForm::closeall_Click(Object* sender, EventArgs* e)
{
	//SaveAllNoteW(false, true);
	int nCnt = lstNote->GetSize();
	for(int i = 0; i < nCnt; i++)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(noteTmp->note != NULL)
		{
			noteTmp->note->Dispose();
			::SendMessage(noteTmp->note->m_hWnd, WM_CLOSE, 1, 1);
		}
	}
}

void MainForm::NoteShowHide_Click(Object* sender, EventArgs* e)
{
	int nIndex = ltbNoeForm->GetSelectedIndex();
	if(nIndex < 0)
	{
		return;
	}
	noteTmp = (NoteEntity*)lstNote->GetKey(nIndex);
	if(noteTmp->note != NULL && noteTmp->note->GetIsDispose())
	{
		delete noteTmp->note;
		noteTmp->note = NULL;
	}
	if(noteTmp->note == NULL)
	{
		if(noteTmp->nMode == MODE_IE)
		{
			noteTmp->note = new MiniIEForm();
		}
		else
		{
			noteTmp->note = new NoteForm();
		}
		noteTmp->note->SetVisible(false);
		noteTmp->note->CreateControl();
		noteTmp->note->SetVisible(true);
		noteTmp->note->SetIsMin(false);
	}
	else
	{
		if(e == NULL)
		{
			//from contMenu_Popup/Click Show
			noteTmp->note->SetVisible(true);
			cc::Util::FourceWindowTop(noteTmp->note->m_hWnd);
			noteTmp->note->SetIsMin(false);
		}
		else
		{
			noteTmp->note->SaveToEntity(noteTmp, CC_NOTE_CFG_CLOSED);
			noteTmp->note->Dispose();
			delete noteTmp->note;
			noteTmp->note = NULL;
			//::SendMessage(noteTmp->note->m_hWnd, WM_CLOSE, NULL, NULL);
		}
	}
}

void MainForm::NoteSetPWD_Click(Object* sender, EventArgs* e)
{
	//alow empty password
	ChangePWDDlg* changePWD = new ChangePWDDlg((TCHAR*)notePWD->GetBuf(), true);
	changePWD->SetParent(this, true);
	changePWD->SetCenter(true);
	if(changePWD->ShowDialog() == DialogResult::OK)
	{
		Control* ctl = changePWD->GetControl(_T("editText2"));
		cc::Str n = cc::Str(ctl->GetText());
		if(n.GetLength() > NOTE_PWD_MAXLEN) {
			MessageBox(m_hWnd, TCHAR_LANGFMT1("Lang.U_NotePwdMaxLen", NOTE_PWD_MAXLEN), NULL, MB_ICONERROR);
		}
		else {
			notePWD->ValueOf(ctl->GetText());
		}
	}
	changePWD->Dispose();
	delete changePWD;
}

//void MainForm::NoteTimer_Click(Object* sender, EventArgs* e)
//{
//	int nIndex = ltbNoeForm->GetSelectedIndex();
//	if(nIndex < 0)
//	{
//		return;
//	}
//	::MessageBox(m_hWnd, _T("wait for it"), NULL, MB_OK);
//}
//
void MainForm::NoteRemove_Click(Object* sender, EventArgs* e)
{
	int nIndex = ltbNoeForm->GetSelectedIndex();
	if(nIndex < 0)
	{
		return;
	}
	if(IDYES == (int)::MessageBox(m_hWnd, TCHAR_LANG("Lang.U_NOTE_MSG_Remove"), TCHAR_LANG("Lang.S_MsgBox_Title_Question"), MB_YESNO | MB_ICONQUESTION))
	{
		CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
		noteTmp = (NoteEntity*)lstNote->GetKey(nIndex);
		if(noteTmp->note != NULL)
		{
			::SendMessage(noteTmp->note->m_hWnd, WM_CLOSE, NULL, NULL);
			delete noteTmp->note;
		}
		if(noteTmp->nMode == MODE_NOTE)
		{
			cc::Str path = cc::Util::getStartupRelatePath(noteTmp->tcFileName);
			cc::Util::fDel(path);
		}
		delete noteTmp;
		lstNote->RemoveAt(nIndex);
		ltbNoeForm->ItemDel(nIndex);

		int nCnt = ltbNoeForm->ItemCount();
		if(nCnt <= nIndex)
		{
			nIndex = nCnt - 1;
		}
		ltbNoeForm->SetSelectedIndex(nIndex);
	}
}

void MainForm::NoteUp_Click(Object* sender, EventArgs* e)
{
	int nIndex = ltbNoeForm->GetSelectedIndex();
	if(nIndex < 1)
	{
		return;
	}
	ltbNoeForm->ItemUp(nIndex);
	lstNote->Swap(nIndex, nIndex - 1);
}

void MainForm::NoteDown_Click(Object* sender, EventArgs* e)
{
	int nIndex = ltbNoeForm->GetSelectedIndex();
	int nCnt = ltbNoeForm->ItemCount();
	if(nIndex < 0 || nIndex == nCnt - 1)
	{
		return;
	}
	ltbNoeForm->ItemDown(nIndex);
	lstNote->Swap(nIndex, nIndex + 1);
}

void MainForm::menuExit_Click(Object* sender, EventArgs* e)
{
	::SendMessage(m_hWnd, WM_CLOSE, 1, 1);
	//App::Exit();
}

void MainForm::OptionOK_Click(Object* sender, EventArgs* e)
{
	//get selected lang and set to ini
	cc::win::WinUtil::SetLangToConfig(comboLang);
	SetStatus(NULL);
	SetTips(NULL);
	cc::win::App::GetConfig()->LoadLang(CC_CONFIG_LANGENG);
	SetLang(*CC_APP_GetLang(), cc::Str(CC_CONFIG_LANGSECTION));
	SetLangAllNoteW();
}

void MainForm::showhide_Click(Object* sender, EventArgs* e)
{
	if(GetVisible() && sender != notify1)
	{
		//SetWindowState(FormWindowState::Minimized);
		::ShowWindow(m_hWnd, SW_MINIMIZE);
		//SetVisible(false);
		::SetTimer(this->m_hWnd, UU_TIMERID_MIN_HIDE, 300, NULL);
	}
	else
	{
		//save state
		FormWindowState::Type sType = GetWindowState();
		if(!GetVisible() && sType != FormWindowState::Minimized)
		{
			SetWindowState(FormWindowState::Minimized);
		}
		SetVisible(true);
		::ShowWindow(m_hWnd, SW_RESTORE);
	}
}

/*
void MainForm::refresh_Click(Object* sender, EventArgs* e)
{
	//comboWinList->ItemClear();
	lstWin->Clear();
	HWND hwndtmp = ::GetDesktopWindow();
	hwndtmp = GetNextWindow(hwndtmp, GW_CHILD);
	HWND hwndend = hwndtmp;
	while(hwndtmp != 0)
	{
		int style = (DWORD)GetWindowLong(hwndtmp, GWL_STYLE);
		if(::IsWindowVisible(hwndtmp) && ((style & WS_CAPTION) != 0 || ::GetParent(hwndtmp) == NULL))
		{
			::GetWindowText(hwndtmp, (LPTSTR)&buf, BUF_LEN - 1);
			//comboWinList->ItemAdd((const TCHAR *)buf);
			lstWin->Add(hwndtmp);
		}
		hwndtmp = GetNextWindow(hwndtmp, GW_HWNDNEXT);
		if(hwndtmp == hwndend)
		{
			break;
		}
	}
}

void MainForm::refreshHide_Click(Object* sender, EventArgs* e)
{
	chklWinList->ItemClear();
	lstWinHide->Clear();
	HWND hwndtmp = ::GetDesktopWindow();
	hwndtmp = GetNextWindow(hwndtmp, GW_CHILD);
	HWND hwndend = hwndtmp;
	while(hwndtmp != 0)
	{
		int style = (DWORD)GetWindowLong(hwndtmp, GWL_STYLE);
		if(::IsWindowVisible(hwndtmp) && ((style & WS_CAPTION) != 0 || ::GetParent(hwndtmp) == NULL))
		{
			::GetWindowText(hwndtmp, (LPTSTR)&buf, BUF_LEN - 1);
			chklWinList->ItemAdd((const TCHAR *)buf);
			lstWinHide->Add(hwndtmp);
		}
		hwndtmp = GetNextWindow(hwndtmp, GW_HWNDNEXT);
		if(hwndtmp == hwndend)
		{
			break;
		}
	}
}

bool MainForm::CheckHwnd()
{
	int nIndex;// = comboWinList->GetSelectedIndex();
	if(nIndex < 0)
	{
		MessageBox(m_hWnd, TCHAR_LANG("Lang.U_NeedSelWin"), NULL, MB_ICONERROR);
		return false;
	}
	HWND hWnd = lstWin->GetKey(nIndex);
	if(!::IsWindow(hWnd))
	{
		MessageBox(m_hWnd, TCHAR_LANG("Lang.U_WinNotValid"), NULL, MB_ICONERROR);
		return false;
	}
	return true;
}

void MainForm::ontop_Click(Object* sender, EventArgs* e)
{
	if(!CheckHwnd())
	{
		return;
	}
	int nIndex = comboWinList->GetSelectedIndex();
	HWND hWnd = lstWin->GetKey(nIndex);
	::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void MainForm::notontop_Click(Object* sender, EventArgs* e)
{
	if(!CheckHwnd())
	{
		return;
	}
	int nIndex = comboWinList->GetSelectedIndex();
	HWND hWnd = lstWin->GetKey(nIndex);
	::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void MainForm::chgwin_Click(Object* sender, EventArgs* e)
{
	if(!CheckHwnd())
	{
		return;
	}
	int nIndex = comboWinList->GetSelectedIndex();
	HWND hWnd = lstWin->GetKey(nIndex);
	if(sender == btnResetWin)
	{
		cc::Util::SetTransparent(hWnd, 255);
	}
	else
	{
		int nParcent = pickPercent->GetValue();
		cc::Util::SetTransparent(hWnd, nParcent);
	}
}
*/

void MainForm::GetHwnd_MouseDown(Object* sender, MouseEventArgs* e)
{
	if(e->Button == MouseButtons::Left)
	{
		SetTips(NULL);
		int ret = StartHook(glhInstance, m_hWnd);
		if(ret == -1 || glhInstance == NULL)
		{
			cc::Str msg = TCHAR_LANG("Lang.U_LoadDllErr");
			editPassword->SetText(msg);
			SetStatus(msg);
			return;
		}
		else if(ret == -2)
		{
			cc::Str msg = TCHAR_LANG("Lang.U_StartHookErr");
			editPassword->SetText(msg);
			SetStatus(msg);
			return;
		}

		hWndPrev = NULL;
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		//this->SetWindowState(FormWindowState::Minimized);
		SetCapture(m_hWnd);
		//Sleep(300);
		CC_BIT_ON(m_MainFlag, UU_MAINFLAG_GETHWND);
	}
}

void MainForm::GetHwnd_MouseMove(Object* sender, MouseEventArgs* e)
{
	if(!CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_GETHWND))
	{
		return;
	}
	POINT point;
	GetCursorPos(&point);
	HWND hWnd = CC_SmallestWindowFromPoint(point);
	if(hWnd != NULL && !::IsChild(m_hWnd, hWnd))
	{
		if(hWnd != hWndPrev)
		{
			// New window, remove the old border and draw a new one
			CC_InvertBorder(hWndPrev);
			hWndPrev = hWnd;
			CC_InvertBorder(hWndPrev);

			memset(buf, 0, BUF_LEN);
			::GetWindowText(hWnd, (LPTSTR)&buf, BUF_LEN - 1);
			editPassword->SetText(buf);

			//if(cc::Str::Len(buf) < 1)
			{
				memset(pShareMemory->pwd, 0, BUF_PWD_LEN);
				//send msg to spy for get password
				::SendMessage(hWndPrev, CC_WM_SPYPWD, 1, 2);
				editPassword->SetText(pShareMemory->pwd);
			}
		}
	}
}

void MainForm::GetHwnd_MouseUp(Object* sender, MouseEventArgs* e)
{
	if(!CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_GETHWND))
	{
		return;
	}
	CC_BIT_OFF(m_MainFlag, UU_MAINFLAG_GETHWND);
	FreeHook(glhInstance);
	if(hWndPrev != NULL)
	{
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		CC_InvertBorder(hWndPrev);
		hWndPrev = NULL;
	}
	ReleaseCapture();
	//::ShowWindow(m_hWnd, SW_RESTORE);
	//cc::Util::FourceWindowTop(m_hWnd);
}

//---------------------------------------------------------------------------
//  Windows XP and older - winmm.dll
//---------------------------------------------------------------------------
float GetVolume(WindowsVersion::Type winVer)
{
	float currentVolume = 0;
	if((int)winVer >= (int)WindowsVersion::WindowsVista) {
		GUID m_guidMyContext = GUID_NULL;
		IMMDeviceEnumerator *deviceEnumerator = NULL;
		IMMDevice *defaultDevice = NULL;
		IAudioEndpointVolume *endpointVolume = NULL;

		if(FAILED(CoInitialize(NULL)))
		{
			cc::Log::error(_T("Error : CoInitialize(NULL)."));
			return false;
		}

		HRESULT hr;
		hr = CoCreateGuid(&m_guidMyContext);
		if(FAILED(hr)) {
			return false;
		}
		
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);

		hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

		hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&endpointVolume);

		hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
		cc::Log::debug(_T("Current volume as a scalar is(W7): %0.2f\n"), currentVolume);

		if(endpointVolume != NULL) {
			endpointVolume->Release();
			endpointVolume = NULL;
		}

		if(deviceEnumerator != NULL) {
			deviceEnumerator->Release();
			deviceEnumerator = NULL;
		}

		if(defaultDevice != NULL) {
			defaultDevice->Release();
			defaultDevice = NULL;
		}
		CoUninitialize();
		return currentVolume;
	}

	//WinXP, Win2000
	MMRESULT result;
	HMIXER hMixer;
	MIXERLINE ml = {0};
	MIXERLINECONTROLS mlc = {0};
	MIXERCONTROL mc = {0};
	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdu = {0};

	// get a handle to the mixer device
	result = mixerOpen(&hMixer, 0, 0, 0, MIXER_OBJECTF_HMIXER);
	if(MMSYSERR_NOERROR == result)
	{
		ml.cbStruct = sizeof(MIXERLINE);
		ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		// get the speaker line of the mixer device
		result = mixerGetLineInfo((HMIXEROBJ) hMixer, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE);
		if(MMSYSERR_NOERROR == result)
		{
			mlc.cbStruct = sizeof(MIXERLINECONTROLS);
			mlc.dwLineID = ml.dwLineID;
			mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mlc.cControls = 1;
			mlc.pamxctrl = &mc;
			mlc.cbmxctrl = sizeof(MIXERCONTROL);

			// get the volume controls associated with the speaker line
			result = mixerGetLineControls((HMIXEROBJ) hMixer, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);
			if(MMSYSERR_NOERROR == result)
			{
				mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
				mcd.hwndOwner = 0;
				mcd.dwControlID = mc.dwControlID;
				mcd.paDetails = &mcdu;
				mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
				mcd.cChannels = 1;

				// get the volume
				result = mixerGetControlDetails((HMIXEROBJ) hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
				if(MMSYSERR_NOERROR == result)
				{
					LONG range = ((LONG*)&mc.Bounds)[1] - ((LONG*)&mc.Bounds)[0];
					currentVolume = (float)(mcdu.dwValue/range);
					cc::Log::debug(_T("Current volume as a scalar is: %0.2f\n"), currentVolume);
				}
			}
		}
	
		mixerClose(hMixer);
	}

	return (currentVolume);
}

/*mute:on=1,off=-1,do nothing=0*/
bool SetVolume(WindowsVersion::Type winVer, float scalarVolume, int mute)
{
	if(scalarVolume < 0.005) {
		scalarVolume = 0;
	}
	if(scalarVolume > 9.995) {
		scalarVolume = 1.0;
	}
	if((int)winVer >= (int)WindowsVersion::WindowsVista) {
		GUID m_guidMyContext = GUID_NULL;
		IMMDeviceEnumerator *deviceEnumerator = NULL;
		IMMDevice *defaultDevice = NULL;
		IAudioEndpointVolume *endpointVolume = NULL;

		if(FAILED(CoInitialize(NULL)))
		{
			cc::Log::error(_T("Error : CoInitialize(NULL)."));
			return false;
		}

		HRESULT hr;
		hr = CoCreateGuid(&m_guidMyContext);
		if(FAILED(hr)) {
			return false;
		}
		
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);

		hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

		hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&endpointVolume);

		if(mute != -1 && mute != 1) {
			hr = endpointVolume->SetMasterVolumeLevelScalar(scalarVolume, &m_guidMyContext);
			cc::Log::debug(_T("Current volume as a scalar is(W7): %0.2f\n"), scalarVolume);
		}

		if(mute == -1) {
			endpointVolume->SetMute(TRUE, &m_guidMyContext);
		}
		else if(mute == 1) {
			endpointVolume->SetMute(FALSE, &m_guidMyContext);
		}
		else {
			if(scalarVolume < 0.005) {
				endpointVolume->SetMute(TRUE, &m_guidMyContext);
			}
			else {
				endpointVolume->SetMute(FALSE, &m_guidMyContext);
			}
		}

		if(endpointVolume != NULL) {
			endpointVolume->Release();
			endpointVolume = NULL;
		}

		if(deviceEnumerator != NULL) {
			deviceEnumerator->Release();
			deviceEnumerator = NULL;
		}

		if(defaultDevice != NULL) {
			defaultDevice->Release();
			defaultDevice = NULL;
		}
		CoUninitialize();
		return true;
	}

	//WinXP, Win2000
	MMRESULT result;
	HMIXER hMixer;
	MIXERLINE ml = {0};
	MIXERLINECONTROLS mlc = {0};
	MIXERCONTROL mc = {0};
	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdu = {0};
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute = {0};

	// get a handle to the mixer device
	result = mixerOpen(&hMixer, MIXER_OBJECTF_MIXER, 0, 0, 0);
	if(MMSYSERR_NOERROR == result)
	{
		MIXERCAPS g_mxcaps;
		if(mixerGetDevCaps((UINT) hMixer, &g_mxcaps, sizeof(MIXERCAPS)) != MMSYSERR_NOERROR)
		{
			//SetStatus(_T("InitMasterVolumeControl Error."));
			return false;
		}
		ml.cbStruct = sizeof(MIXERLINE);
		ml.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		// get the speaker line of the mixer device
		result = mixerGetLineInfo((HMIXEROBJ) hMixer, &ml, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE);
		if(MMSYSERR_NOERROR == result)
		{
			mlc.cbStruct = sizeof(MIXERLINECONTROLS);
			mlc.dwLineID = ml.dwLineID;
			mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mlc.cControls = 1;
			mlc.pamxctrl = &mc;
			mlc.cbmxctrl = sizeof(MIXERCONTROL);

			// get the volume controls associated with the speaker line
			result = mixerGetLineControls((HMIXEROBJ) hMixer, &mlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
			if(MMSYSERR_NOERROR == result)
			{
				mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
				mcd.hwndOwner = 0;
				mcd.dwControlID = mc.dwControlID;
				mcd.paDetails = &mcdu;
				mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
				mcd.cChannels = 1;

				// get the volume
				result = mixerGetControlDetails((HMIXEROBJ) hMixer, &mcd, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
				if(MMSYSERR_NOERROR == result)
				{
					LONG range = ((LONG*)&mc.Bounds)[1] - ((LONG*)&mc.Bounds)[0];
					DWORD dwVolume = (DWORD)(scalarVolume * range);
					cc::Log::debug(_T("Set volume rang:%d, volume:%d, scalar: %0.2f\n"), (int)range, (int)dwVolume, scalarVolume);
					//if((LONG)dwVolume < ((LONG*)&mc.Bounds)[0])
					//{
					//	dwVolume = ((LONG*)&mc.Bounds)[0];
					//}
					//if((LONG)dwVolume > ((LONG*)&mc.Bounds)[1])
					//{
					//	dwVolume = ((LONG*)&mc.Bounds)[1];
					//}

					mcdu.dwValue = dwVolume;
					mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
					mcd.hwndOwner = 0;
					mcd.dwControlID = mc.dwControlID;
					mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
					mcd.paDetails = &mcdu;
					mcd.cChannels = 1;
					mcd.cMultipleItems = 0;

					if(mute != -1 && mute != 1) {
						// set the volume
						result = mixerSetControlDetails((HMIXEROBJ) hMixer, &mcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
					}

					if(mute == -1) {
						mxcdMute.fValue = TRUE;
					}
					else if(mute == 1) {
						mxcdMute.fValue = FALSE;
					}
					else {
						if(scalarVolume < 0.005) {
							mxcdMute.fValue = TRUE;
						}
						else {
							mxcdMute.fValue = FALSE;
						}
					}
					mcd.cbDetails   = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
					mcd.paDetails   = &mxcdMute;
					// set the mute
					result = mixerSetControlDetails((HMIXEROBJ) hMixer, &mcd, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
					
					mixerClose(hMixer);
					return true;
				}
			}
		}
	
		mixerClose(hMixer);
	}
	return false;
}

void MainForm::hotkey_Press(Object* sender, EventArgs* e)
{
	if(e->WParam == MAKELONG(VK_ESCAPE, MOD_SHIFT))
	{
		if(hWndHide != NULL)
		{
			::ShowWindow(hWndHide, SW_SHOW);
			hWndHide = NULL;
		}
		else
		{
			HWND hWndTmp = MainActivateWindow();
			if(hWndTmp != ::GetDesktopWindow() && hWndTmp != m_hWnd)
			{
				::ShowWindow(hWndTmp, SW_HIDE);
				hWndHide = hWndTmp;
			}
			else
			{
				MessageBox(m_hWnd, TCHAR_LANG("Lang.U_NoHideThis"), NULL, MB_ICONERROR | MB_TOPMOST);
			}
		}
	}
	else if(e->WParam == MAKELONG(VK_ESCAPE, MOD_WIN))
	{
		//if(chklWinList->GetCheckedCount() < 1)
		//{
		//	MessageBox(NULL, TCHAR_LANG("Lang.U_NoHideDef"), NULL, MB_ICONERROR | MB_TOPMOST);
		//	return;
		//}
		if(chkHideNote->GetEnabled())
		{
			//group window is not hide,try hide all checked win
			chkHideNote->SetEnabled(false);
			//btnHideRefresh->SetEnabled(false);
			//chklWinList->SetEnabled(false);
			ShowAllHideW(SW_HIDE);
		}
		else
		{
			ShowAllHideW(SW_SHOW);
			chkHideNote->SetEnabled(true);
			//btnHideRefresh->SetEnabled(true);
			//chklWinList->SetEnabled(true);
		}
	}
	else if(e->WParam == MAKELONG(VK_LEFT, MOD_WIN | MOD_CONTROL) || e->WParam == MAKELONG(VK_RIGHT, MOD_WIN | MOD_CONTROL))
	{
		HWND hWndTmp = MainActivateWindow();
		if(hWndTmp != NULL)
		{
			//if is first time, default is 255
			int nParcent = 255;
			if(lstWinTrans->GetIndex(hWndTmp) >= 0)
			{
				nParcent = cc::Util::GetTransparent(hWndTmp);
			}
			else
			{
				lstWinTrans->Add(hWndTmp);
			}
			if(nParcent <= 0)
			{
				nParcent = 255;
			}
			if(e->WParam == MAKELONG(VK_LEFT, MOD_WIN | MOD_CONTROL))
			{
				nParcent -= 4;
			}
			else
			{
				nParcent += 4;
			}
			if(nParcent < 1)
			{
				nParcent = 1;
			}
			if(nParcent > 255)
			{
				nParcent = 255;
			}
			transparentDialog->SetStep(nParcent);
			cc::Util::SetTransparent(hWndTmp, nParcent);
			hWndFlash = hWndTmp;
			nFlashTime = 760;
			::SetTimer(this->m_hWnd, UU_TIMERID_WFLASH, 50, NULL);
		}
	}
	else if(e->WParam == MAKELONG(VK_UP, MOD_WIN | MOD_CONTROL) || e->WParam == MAKELONG(VK_DOWN, MOD_WIN | MOD_CONTROL))
	{
		float ind = GetVolume(winVer); //volumeDialog->GetVolume();
		if(ind >= (float)0.0)
		{
			float step = (float)(2.0/255.0);
			if(e->WParam == MAKELONG(VK_UP, MOD_WIN | MOD_CONTROL)) //UP
			{
				//ind = MainForm_volume(1);
				if(ind < 1.0 || !volumeDialog->GetVisible())
				{
					ind += step;
					cc::Log::debug(_T("volume ind:%0.2f, step: %0.2f\n"), ind, step);
					SetVolume(winVer, ind, 0);
				}
			}
			else
			{
				if(ind > 0.0 || !volumeDialog->GetVisible())
				{
					ind -= step;
					cc::Log::debug(_T("volume ind:%0.2f, step: %0.2f\n"), ind, step);
					SetVolume(winVer, ind, 0);
				}
			}
			volumeDialog->SetStep((int)(ind*255));
		}
	}
	else if(e->WParam == MAKELONG(VK_SPACE, MOD_WIN | MOD_CONTROL))
	{
		volumeDialog->SetStep(0);
		if((clock() - tmVolumeDbClick) <= 300) {
			SetVolume(winVer, -1, 1);
		}
		else {
			SetVolume(winVer, -1, -1);
		}
		tmVolumeDbClick = ::clock();
	}
	else if(e->WParam == MAKELONG(VK_F11, MOD_WIN | MOD_CONTROL))
	{
		HWND hWndTmp = MainActivateWindow();
		if(hWndTmp != NULL)
		{
			::SetWindowPos(hWndTmp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hWndFlash = hWndTmp;
			nFlashTime = 760;
			::SetTimer(this->m_hWnd, UU_TIMERID_WFLASH, 50, NULL);
		}
	}
	else if(e->WParam == MAKELONG(VK_F12, MOD_WIN | MOD_CONTROL))
	{
		HWND hWndTmp = MainActivateWindow();
		if(hWndTmp != NULL)
		{
			::SetWindowPos(hWndTmp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			hWndFlash = hWndTmp;
			nFlashTime = 760;
			::SetTimer(this->m_hWnd, UU_TIMERID_WFLASH, 50, NULL);
		}
	}
	else if(e->WParam == MAKELONG('V', MOD_SHIFT | MOD_CONTROL))
	{
		if(CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_REFRESHMENU))
		{
			CC_BIT_OFF(m_MainFlag, UU_MAINFLAG_REFRESHMENU);
			int nCnt = menuClipboard->ItemCount();
			for(int i = nCnt - 1; i >= 0; i--)
			{
				Menu* menu = menuClipboard->ItemRemove(i);
				delete menu;
			}
		}
		if(menuClipboard->ItemCount() < 1)
		{
			MenuItem* subitem = new MenuItem();
			subitem->SetText(_T("Save Current Clipboard Text"));
			subitem->SetIcon(imageList, IDI_ICON_COPY);
			subitem->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuClipboard_Click);
			menuClipboard->ItemAdd(subitem);

			subitem = new MenuItem();
			subitem->SetText(_T("Clean All"));
			//subitem->SetIcon(imageList, IDI_ICON_COPY);
			subitem->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuClipboard_Click);
			menuClipboard->ItemAdd(subitem);

			for(int i = 0; i < lstClipboard->GetSize(); i++)
			{
				MenuItem* subitem = new MenuItem();
				subitem->SetIcon(imageList, IDI_ICON_PASTE);
				cc::Str str = lstClipboard->GetKey(i);
				if(str.GetLength() > 100)
				{
					str = str.Substring(0, 100).Replace("\r\n", "\n").Replace("\r", "\n").Replace("\n", " ").Append(_T("..."));
				}
				else
				{
					str = str.Replace("\r\n", "\n").Replace("\r", "\n").Replace("\n", " ");
				}
				str.Insert(0, _T("Paste: "));
				subitem->SetText(str);
				subitem->SetTips(str);
				subitem->Click += EventHandler((Object*)this, (EventFun)&MainForm::menuClipboard_Click);
				menuClipboard->ItemAdd(subitem);
			}
		}

		HWND hWndTmp = MainActivateWindow();
		POINT pt = {0, 0};
		::GetCursorPos(&pt);
		menuClipboard->Show(NULL, pt);
		if(CC_BIT_HAS(m_MainFlag, UU_MAINFLAG_PASTE))
		{
			CC_BIT_OFF(m_MainFlag, UU_MAINFLAG_PASTE);
			cc::Util::FourceWindowTop(hWndTmp);

			Sleep(10);
			// Create a generic keyboard event structure
			INPUT ip;
			ip.type = INPUT_KEYBOARD;
			ip.ki.wScan = 0;
			ip.ki.time = 0;
			ip.ki.dwExtraInfo = 0;

			// Press the "Ctrl" key
			ip.ki.wVk = VK_CONTROL;
			ip.ki.dwFlags = 0; // 0 for key press
			SendInput(1, &ip, sizeof(INPUT));
 
			// Press the "V" key
			ip.ki.wVk = 'V';
			ip.ki.dwFlags = 0; // 0 for key press
			SendInput(1, &ip, sizeof(INPUT));
 
			// Release the "V" key
			ip.ki.wVk = 'V';
			ip.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
 
			// Release the "Ctrl" key
			ip.ki.wVk = VK_CONTROL;
			ip.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &ip, sizeof(INPUT));
		}
	}
}

void MainForm::menuClipboard_Click(Object* sender, EventArgs* e)
{
	MenuItem* subitem = (MenuItem*)sender;
	int ind = subitem->GetIndex();
	if(ind == 0)
	{
		cc::Str str;
		cc::Util::GetClipboardText(str, m_hWnd);
		if(str.GetLength() > 0)
		{
			if(lstClipboard->GetSize() > 10)
			{
				cc::Str str2 = lstClipboard->GetKey(0);
				str2.Clear();
				lstClipboard->RemoveAt(0);
			}
			lstClipboard->Add(str);
			CC_BIT_ON(m_MainFlag, UU_MAINFLAG_REFRESHMENU);
		}
	}
	else if(ind == 1)
	{
		lstClipboard->Clear();
	}
	else
	{
		cc::Str str = lstClipboard->GetKey(ind - 1);
		cc::Util::SetClipboardText(str, m_hWnd);
		CC_BIT_ON(m_MainFlag, UU_MAINFLAG_PASTE);
	}
}

//void MainForm::refreshtime_Click(Object* sender, EventArgs* e)
//{
//	cc::Str strTimeFile = cc::Util::getStartupPath();
//	strTimeFile.Append(_T(".WorkTime.txt"));
//	cc::Str sTxt(LangCode::TCHAR);
//	if(cc::Util::fExist((TCHAR*)strTimeFile.GetBuf()))
//	{
//		cc::Util::fRead(sTxt, (TCHAR*)strTimeFile.GetBuf(), LangCode::TCHAR);
//	}
//	if(sTxt.GetLength() != 0 && !sTxt.EndsWith(_T("\r\n")))
//	{
//		sTxt.Append(_T("\r\n"));
//	}
//
//	int nLines = 4;
//	int nPos = sTxt.LastIndexOf(_T("\r\n"));
//	while(nPos > 0 && nLines > 0)
//	{
//		nLines--;
//		nPos = sTxt.LastIndexOf(_T("\r\n"), nPos - 1);
//	}
//	cc::Str sShow(LangCode::TCHAR, sTxt.Substring(nPos < 0 ? 0 : nPos + 2));
//	editTime->SetText((TCHAR*)sShow.GetBuf());
//}

//the check status will change to(is different to now status!)
void MainForm::autorun_Click(Object* sender, EventArgs* e)
{
	cc::Str strPathFile = cc::Util::getStartupPath();
	const TCHAR* tcKeySub = cc::win::App::GetName();
	bool isAuto = chkAutoRun->GetChecked();
	registryKey1->OpenSubKey(tcKey_RUN, true);
	if(!isAuto)
	{
		registryKey1->SetValue(tcKeySub, (TCHAR*)strPathFile.GetBuf());
	}
	else
	{
		registryKey1->DeleteValue(tcKeySub);
	}
	registryKey1->Close();
	::SetTimer(this->m_hWnd, UU_TIMERID_AUTORUNCHK, 50, NULL);
}

void MainForm::opentime_Click(Object* sender, EventArgs* e)
{
	cc::Str strTimeFile = cc::Util::getFileName(cc::Util::getStartupPath());
	strTimeFile.Append(_T(".WorkTime.txt"));
	//if(cc::Util::fExist((TCHAR*)strTimeFile.GetBuf()))
	//{
	//	ShellExecute(m_hWnd, _T("open"), _T("notepad.exe"), (TCHAR*)strTimeFile.GetBuf(), NULL, SW_SHOWNORMAL);
	//}
	//else
	//{
	//	MessageBox(m_hWnd, _T("not exist WorkTime text file!"), NULL, MB_ICONERROR);
	//}

	if(noteWKTime->note != NULL && noteWKTime->note->GetIsDispose())
	{
		delete noteWKTime->note;
		noteWKTime->note = NULL;
	}
	if(noteWKTime->note == NULL)
	{
		noteWKTime->note = new NoteForm();
		noteWKTime->note->SetVisible(false);
		noteWKTime->note->SetMode(MODE_TIME);
		//_tcscpy(noteWKTime->tcFileName, (TCHAR*)strTimeFile.GetBuf());
		cc::Buf::Cpy(noteWKTime->tcFileName, 40, (TCHAR*)strTimeFile.GetBuf());
		noteWKTime->note->CreateControl();
		noteWKTime->note->SetVisible(true);
	}
}

void MainForm::ShowAllHideW(DWORD dwShowOrHide)
{
	for(int i = 0; i < 5; i++)
	{
		cc::Str str = btnGetHwnd[i]->GetText();
		HWND hWnd = (HWND)str.ToInt();
		if(hWnd != 0 && IsWindow(hWnd))
		{
			::ShowWindow(hWnd, dwShowOrHide);
		}
	}
	if(dwShowOrHide == SW_SHOW)
	{
		for(int n = 0; n < lstWinHideSave->GetSize(); n++)
		{
			::ShowWindow(lstWinHideSave->GetKey(n), dwShowOrHide);
		}
		lstWinHideSave->Clear();
		return;
	}

	//add win of list
	lstWinHideSave->Clear();
	//int nCnt = chklWinList->ItemCount();
	//for(int n = 0; n < nCnt; n++)
	//{
	//	if(chklWinList->GetCheck(n))
	//	{
	//		HWND hwndtmp = lstWinHide->GetKey(n);
	//		if(hwndtmp != m_hWnd)
	//		{
	//			lstWinHideSave->Add(hwndtmp);
	//		}
	//	}
	//}

	//add win of note if need
	if(chkHideNote->GetChecked())
	{
		for(int i = lstNote->GetSize() - 1; i >= 0; i--)
		{
			noteTmp = (NoteEntity*)lstNote->GetKey(i);
			if(noteTmp->note != NULL && !noteTmp->note->GetIsDispose())
			{
				lstWinHideSave->Add(noteTmp->note->m_hWnd);
			}
		}
	}

	//add win of filter
	HWND hwndtmp = ::GetDesktopWindow();
	hwndtmp = GetNextWindow(hwndtmp, GW_CHILD);
	HWND hwndend = hwndtmp;
	while(hwndtmp != 0)
	{
		int style = (DWORD)GetWindowLong(hwndtmp, GWL_STYLE);
		if(::IsWindowVisible(hwndtmp) && ((style & WS_CAPTION) != 0 || (style & WS_THICKFRAME) != 0 || (style & WS_POPUP) != 0 || ::GetParent(hwndtmp) == NULL))
		{
			::GetWindowText(hwndtmp, (LPTSTR)&buf, BUF_LEN - 1);
			bool isHide = false;
			for(int i = 0; i < 15; i++)
			{
				int nLen = editFilter[i]->GetTextLen();
				if(nLen > 0)
				{
					const TCHAR* tcText = editFilter[i]->GetText();
					if(i < 5)
					{
						//Exact Match
						if(_tcscmp(buf, tcText) == 0)
						{
							isHide = true;
						}
					}
					else
					{
						//Sub Match
						if(_tcsstr(buf, tcText) != NULL)
						{
							isHide = true;
						}
					}
				}
				if(isHide)
				{
					lstWinHideSave->Add(hwndtmp);
					break;
				}
			}
		}
		hwndtmp = GetNextWindow(hwndtmp, GW_HWNDNEXT);
		if(hwndtmp == hwndend)
		{
			break;
		}
	}

	HWND hWndTop = MainActivateWindow();
	for(int n = 0; n < lstWinHideSave->GetSize(); n++)
	{
		HWND wnd = lstWinHideSave->GetKey(n);
		::ShowWindow(wnd, SW_HIDE);
		if(wnd == hWndTop)
		{
			//if not do this,then will be shown while logout&login
			::SetForegroundWindow(::GetDesktopWindow());
			::BringWindowToTop(::GetDesktopWindow());
		}
	}
}

cc::Str MainForm::GetCurTime(const TCHAR* sPre)
{
	struct tm *tm;
	time_t t;
	t = time(NULL);
#ifdef __STDC_SECURE_LIB__
	struct tm ptmTemp1;
	localtime_s(&ptmTemp1, &t);
	tm = &ptmTemp1;
#else
	tm = localtime(&t);
#endif
	cc::Str sTime = cc::Str(LangCode::TCHAR).Format(_T("%s%04d/%02d/%02d %02d:%02d:%02d\r\n"), 
			sPre, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, 
			tm->tm_min, tm->tm_sec);
	return sTime;
}

void MainForm::menuAbout_Click(Object* sender, EventArgs* e)
{
	cc::Str sAbout = App::GetNameVersion();
	sAbout.Append(_T(" ")).Append(APPBUILD);
	AboutDlg* dlg = new AboutDlg(sAbout, imageList, IDI_ICON_MAIN, CC_APP_GetLang()->Get(_T("Lang.U_CopyRightTitle")), 
		420, 36, 36, CC_APP_GetLang()->Get(_T("Lang.U_CopyRight")), true, true);
	dlg->SetParent(this, true);
	dlg->SetCenter();
	dlg->ShowDialog();

	dlg->Dispose();
	delete dlg;
}

void MainForm::toolOnTop_Click(Object* sender, EventArgs* e)
{
	bool chk = toolOnTop->GetPushed();
	::SetWindowPos(m_hWnd, (chk) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void MainForm::toolSetBack_Click(Object* sender, EventArgs* e)
{
	ConfigBackDlg cfgBack;
	cfgBack.SetParent(this, true);
	cfgBack.SetResultControl(this->GetClient(), 0);
	cfgBack.SetCenter();
	//void ConfigBackDlg::AddInnerImage(UINT nResID, const TCHAR *resType, bool isStretch, const TCHAR* info)
	if(cfgBack.ShowDialog() == DialogResult::OK)
	{
		for(int i = lstNote->GetSize() - 1; i >= 0; i--)
		{
			noteTmp = (NoteEntity*)lstNote->GetKey(i);
			if(noteTmp->note != NULL && !noteTmp->note->GetIsDispose())
			{
				ConfigBackDlg::ConfigResult(noteTmp->note->GetClient(), 0);
			}
		}
		if(noteWKTime->note != NULL && !noteWKTime->note->GetIsDispose())
		{
			ConfigBackDlg::ConfigResult(noteWKTime->note->GetClient(), 0);
		}
	}
}

void MainForm::menuCommTool_Click(Object* sender, EventArgs* e)
{
	bool chk = menuCommTool->GetChecked();
	menuCommTool->SetChecked(!chk);
	commTool->SetVisible(!chk);
	if(commTool->GetFloating())
	{
		commTool->GetFloatContainer()->SetVisible(!chk);
	}

	CC_APP_GetIni()->SetBool(_T("Main.ShowCommTool"), !chk);
}

void MainForm::menuEditTool_Click(Object* sender, EventArgs* e)
{
	bool chk = menuEditTool->GetChecked();
	menuEditTool->SetChecked(!chk);
	editTool->SetVisible(!chk);
	if(editTool->GetFloating())
	{
		editTool->GetFloatContainer()->SetVisible(!chk);
	}

	CC_APP_GetIni()->SetBool(_T("Main.ShowEditTool"), !chk);
}

void MainForm::menuFixTools_Click(Object* sender, EventArgs* e)
{
	bool chk = menuFixTools->GetChecked();
	menuFixTools->SetChecked(!chk);
	this->SetReBarStyle(chk ? ReBarStyle::Floatable : ReBarStyle::Fix);

	CC_APP_GetIni()->SetBool(_T("Main.FixTools"), !chk);
}

void MainForm::menuTips_Click(Object* sender, EventArgs* e)
{
	bool chk = menuTips->GetChecked();
	menuTips->SetChecked(!chk);
	App::SetToolTipActive(!chk);

	CC_APP_GetIni()->SetBool(_T("Main.ShowTips"), !chk);
}

void MainForm::menuStatusBar_Click(Object* sender, EventArgs* e)
{
	bool chk = menuStatusBar->GetChecked();
	menuStatusBar->SetChecked(!chk);
	statusbar->SetVisible(!chk);

	CC_APP_GetIni()->SetBool(_T("Main.ShowStatusBar"), !chk);
}

TextBox* MainForm::GetCurControl()
{
	Control* ctl = Control::FromHandle(::GetFocus());
	if(ctl != NULL && (ctl->GetWndClassName() == WndClassName::TextBox || ctl->GetWndClassName() == WndClassName::RichTextBox))
	{
		return (TextBox*)ctl;
	}
	return NULL;
}

void MainForm::SetEditMenuEnable(bool enable)
{
	menuUnDo->SetEnabled(enable);
	menuReDo->SetEnabled(enable);
	menuCut->SetEnabled(enable);
	menuCopy->SetEnabled(enable);
	menuPaste->SetEnabled(enable);
	menuDel->SetEnabled(enable);
	menuSelAll->SetEnabled(enable);

	toolCopy->SetEnabled(enable);
	toolPaste->SetEnabled(enable);
	toolCut->SetEnabled(enable);
}

void MainForm::ChgEdit_Focus(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	SetEditMenuEnable(txtBox != NULL);
}

void MainForm::menuUnDo_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		txtBox->Undo();
	}
}

void MainForm::menuReDo_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		txtBox->Undo();
	}
}

void MainForm::menuCut_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		txtBox->Cut();
	}
}

void MainForm::menuCopy_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		txtBox->Copy();
	}
}

void MainForm::menuPaste_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		txtBox->Paste();
	}
}

void MainForm::menuDel_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		SendMessage(txtBox->m_hWnd, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
	}
}

void MainForm::menuSelAll_Click(Object* sender, EventArgs* e)
{
	TextBox* txtBox = GetCurControl();
	if(txtBox != NULL)
	{
		long nPos = (long)SendMessage(txtBox->m_hWnd, WM_GETTEXTLENGTH, 0, 0);
		SendMessage(txtBox->m_hWnd, EM_SETSEL, 0, nPos);
	}
}

NoteForm* MainForm::NewNote(cc::Str path)
{
	int nCnt = ltbNoeForm->ItemCount();
	if(NOTEFILE_COUNT_MAX > 0 && nCnt >= NOTEFILE_COUNT_MAX)
	{
		::MessageBox(m_hWnd, CC_APP_GetLang()->GetFormat(_T("Lang.U_NOTE_MSG_Already_Max"), NOTEFILE_COUNT_MAX), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Info")), MB_OK);
		return NULL;
	}
	//cc::Str path = GetNewNoteFile();
	if(path.GetLength() < 1)
	{
		path = GetNewNoteFile();
	}
	ltbNoeForm->ItemAdd(CC_APP_GetLang()->Get(_T("Lang.U_NOTE_CTL_NoTitle")));
	noteTmp = new NoteEntity();
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);

	noteDefault->rcPos.right -= noteDefault->rcPos.left;
	noteDefault->rcPos.bottom -= noteDefault->rcPos.top;
	int nScreenX = ::GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	noteDefault->rcPos.left += 20;
	noteDefault->rcPos.top += 20;
	if(noteDefault->rcPos.left > nScreenX - 100)
	{
		noteDefault->rcPos.left = 20;
	}
	if(noteDefault->rcPos.top > nScreenY - 100)
	{
		noteDefault->rcPos.top = 20;
	}
	noteDefault->rcPos.right += noteDefault->rcPos.left;
	noteDefault->rcPos.bottom += noteDefault->rcPos.top;

	//load default
	memcpy(noteTmp, noteDefault, sizeof(NoteEntity));
	noteTmp->nMode = MODE_NOTE;
	lstNote->Add(noteTmp);

	noteTmp->note = new NoteForm();
	noteTmp->note->SetVisible(false);

	//_tcscpy(noteTmp->tcFileName, (TCHAR*)path.GetBuf());
	cc::Buf::Cpy(noteTmp->tcFileName, 40, (TCHAR*)path.GetBuf());
	noteTmp->note->CreateControl();
	noteTmp->note->SetVisible(true);

	//save to ini
	SaveAllNoteW(false);
	return (NoteForm*)noteTmp->note;
}

void MainForm::chkMirco_Click(Object* sender, EventArgs* e)
{
	for(int i = lstNote->GetSize() - 1; i >= 0; i--)
	{
		noteTmp = (NoteEntity*)lstNote->GetKey(i);
		if(noteTmp->note != NULL && !noteTmp->note->GetIsDispose() && noteTmp->note->GetIsMin())
		{
			noteTmp->note->SetIsMin(false);
			noteTmp->note->SetIsMin(true);
		}
	}
	if(noteWKTime->note != NULL && !noteWKTime->note->GetIsDispose() && noteWKTime->note->GetIsMin())
	{
		noteWKTime->note->SetIsMin(false);
		noteWKTime->note->SetIsMin(true);
	}
}

bool MainForm::GetIsMirco()
{
	return chkMirco->GetChecked();
}

////////////////////////////////////////////////////////////////////////////////miniIE
MiniIEForm* MainForm::NewMiniIE()
{
	int nCnt = ltbNoeForm->ItemCount();
	if(NOTEFILE_COUNT_MAX > 0 && nCnt >= NOTEFILE_COUNT_MAX)
	{
		::MessageBox(m_hWnd, CC_APP_GetLang()->GetFormat(_T("Lang.U_NOTE_MSG_Already_Max"), NOTEFILE_COUNT_MAX), CC_APP_GetLang()->Get(_T("Lang.S_MsgBox_Title_Info")), MB_OK);
		return NULL;
	}
	cc::Str path = cc::Str(_T("about:blank"));
	ltbNoeForm->ItemAdd((TCHAR*)path.GetBuf());
	noteTmp = new NoteEntity();
	lstNote->Add(noteTmp);
	CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);

	noteDefault->rcPos.right -= noteDefault->rcPos.left;
	noteDefault->rcPos.bottom -= noteDefault->rcPos.top;
	int nScreenX = ::GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	noteDefault->rcPos.left += 20;
	noteDefault->rcPos.top += 20;
	if(noteDefault->rcPos.left > nScreenX - 100)
	{
		noteDefault->rcPos.left = 20;
	}
	if(noteDefault->rcPos.top > nScreenY - 100)
	{
		noteDefault->rcPos.top = 20;
	}
	noteDefault->rcPos.right += noteDefault->rcPos.left;
	noteDefault->rcPos.bottom += noteDefault->rcPos.top;

	//load default
	memcpy(noteTmp, noteDefault, sizeof(NoteEntity));
	noteTmp->nMode = MODE_IE;
	memset(noteTmp->tcURL, _T('\0'), IE_URL_LEN);

	noteTmp->note = new MiniIEForm();
	noteTmp->note->SetVisible(false);

	//_tcscpy(noteTmp->tcFontName, (TCHAR*)path.GetBuf());
	noteTmp->note->CreateControl();
	noteTmp->note->SetVisible(true);

	//save to ini
	SaveAllNoteW(false);
	return (MiniIEForm*)noteTmp->note;
}

void MainForm::RemoveBlankIE(NoteBaseForm* note)
{
	int nIndex = GetNoteIndex(note);
	if(nIndex >= 0)
	{
		CC_BIT_ON(m_MainFlag, UU_MAINFLAG_UPDNOTE);
		noteTmp = (NoteEntity*)lstNote->GetKey(nIndex);
		//cannot delete noteTmp->note this time,save to lastNote, delete it nexttime
		if(lastNote != NULL)
		{
			delete lastNote;
			lastNote = NULL;
		}
		lastNote = noteTmp->note;
		delete noteTmp;
		lstNote->RemoveAt(nIndex);
		ltbNoeForm->ItemDel(nIndex);

		int nCnt = ltbNoeForm->ItemCount();
		if(nCnt <= nIndex)
		{
			nIndex = nCnt - 1;
		}
		ltbNoeForm->SetSelectedIndex(nIndex);
	}
}

void MainForm::NewIE_Click(Object* sender, EventArgs* e)
{
	NewMiniIE();
}

void MainForm::GetHwnd_MouseDown2(Object* sender, MouseEventArgs* e)
{
	if(e->Button == MouseButtons::Left)
	{
		SetTips(NULL);
		hWndPrev = NULL;
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		//this->SetWindowState(FormWindowState::Minimized);
		SetCapture(m_hWnd);
		//Sleep(300);
	}
}

void MainForm::GetHwnd_MouseMove2(Object* sender, MouseEventArgs* e)
{
	POINT point;
	GetCursorPos(&point);
	HWND hWnd = CC_SmallestWindowFromPoint(point);
	if(hWnd != NULL && !::IsChild(m_hWnd, hWnd))
	{
		// Make sure that the window doesn't belong to us
		if(hWnd != hWndPrev)
		{
			// New window, remove the old border and draw a new one
			CC_InvertBorder(hWndPrev);
			hWndPrev = hWnd;
			CC_InvertBorder(hWndPrev);
		}
	}
}

void MainForm::GetHwnd_MouseUp2(Object* sender, MouseEventArgs* e)
{
	if(hWndPrev != NULL)
	{
		if(hWndPrev != NULL)
		{
			for(int i = 0; i < 5; i++)
			{
				if(btnGetHwnd[i] == sender)
				{
					btnGetHwnd[i]->SetText(cc::Str().Format(_T("%d"), hWndPrev));
					break;
				}
			}
		}

		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		ReleaseCapture();
		CC_InvertBorder(hWndPrev);
		hWndPrev = NULL;
	}
	//::ShowWindow(m_hWnd, SW_RESTORE);
	//cc::Util::FourceWindowTop(m_hWnd);
}

; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCredits
LastTemplate=CStatic
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "xpilot.h"
LastPage=0

ClassCount=12
Class1=CXpilotApp
Class2=CXpilotDoc
Class3=CXpilotView
Class4=CMainFrame

ResourceCount=7
Resource1=IDD_MOTD
Resource2=IDD_SYSINFO
Class5=CAboutDlg
Class6=motd
Resource3=IDR_MAINFRAME
Class7=CTalkWindow
Resource4=IDD_EXPIRE_SOON
Resource5=IDD_EXPIRED
Class8=CExpireSoon
Class9=CExpired
Class10=CSplashWnd
Resource6=IDD_TALKWINDOW
Class11=CSysInfo
Class12=CCredits
Resource7=IDD_ABOUTBOX

[CLS:CXpilotApp]
Type=0
HeaderFile=xpilotnt.h
ImplementationFile=xpilotnt.cpp
BaseClass=CWinApp
LastObject=CXpilotApp

[CLS:CXpilotDoc]
Type=0
HeaderFile=xpilotDoc.h
ImplementationFile=xpilotDoc.cpp
Filter=N
LastObject=CXpilotDoc

[CLS:CXpilotView]
Type=0
HeaderFile=xpilotView.h
ImplementationFile=xpilotView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=CXpilotView

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=CMainFrame
BaseClass=CFrameWnd
VirtualFilter=fWC



[CLS:CAboutDlg]
Type=0
HeaderFile=about.h
ImplementationFile=about.cpp
BaseClass=CDialog
LastObject=CAboutDlg
Filter=D
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=9
Control1=IDC_STATIC,static,1342177283
Control2=IDC_VERSION,static,1342308480
Control3=IDC_STATIC,static,1342308353
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308353
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342308353
Control8=IDC_STATIC,static,1342177283
Control9=IDC_CREDITS,static,1342177287

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
Command14=ID_CONTEXT_HELP
Command15=ID_HELP
CommandCount=15

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_CONTEXT_HELP
CommandCount=9

[DLG:IDD_MOTD]
Type=1
Class=motd
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_EDIT1,edit,1350633668

[CLS:motd]
Type=0
HeaderFile=motd.h
ImplementationFile=motd.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT1

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_CLOSE
Command2=ID_CONFIGURE_GAME
Command3=ID_COLORS
Command4=ID_CONFIGURE_FONTS
Command5=ID_CONFIGURE_SYSINFO
Command6=ID_HELP
Command7=ID_APP_ABOUT
CommandCount=7

[DLG:IDD_TALKWINDOW]
Type=1
Class=CTalkWindow
ControlCount=1
Control1=IDC_EDIT1,edit,1350631552

[CLS:CTalkWindow]
Type=0
HeaderFile=TalkWindow.h
ImplementationFile=TalkWindow.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CTalkWindow

[DLG:IDD_EXPIRE_SOON]
Type=1
Class=CExpireSoon
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,static,1342308353
Control5=IDC_STATIC,static,1342308353
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342177283
Control8=IDC_STATIC,static,1342177283

[DLG:IDD_EXPIRED]
Type=1
Class=CExpired
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,static,1342308353
Control5=IDC_STATIC,static,1342308353
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342177283
Control8=IDC_STATIC,static,1342177283

[CLS:CExpireSoon]
Type=0
HeaderFile=BetaCheck.h
ImplementationFile=BetaCheck.cpp
BaseClass=CDialog
Filter=D
LastObject=CExpireSoon

[CLS:CExpired]
Type=0
HeaderFile=BetaCheck.h
ImplementationFile=BetaCheck.cpp
BaseClass=CDialog
Filter=D
LastObject=CExpired

[CLS:CSplashWnd]
Type=0
HeaderFile=Splash.h
ImplementationFile=Splash.cpp
BaseClass=CWnd
LastObject=CSplashWnd

[DLG:IDD_SYSINFO]
Type=1
Class=CSysInfo
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDC_EDIT1,edit,1350633668
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352

[CLS:CSysInfo]
Type=0
HeaderFile=SysInfo.h
ImplementationFile=SysInfo.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSysInfo

[CLS:CCredits]
Type=0
HeaderFile=winabout.h
ImplementationFile=winabout.cpp
BaseClass=CStatic
LastObject=CCredits




; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CConfigDlg
LastTemplate=CPropertyPage
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "xpilots.h"

ClassCount=15
Class1=CXpilotsApp
Class2=CXpilotsDlg
Class3=CAboutDlg

ResourceCount=16
Resource1=IDD_CONF_ITEMS
Resource2=IDR_MAINFRAME
Resource3=IDD_CONF_SHIPSSHOTS
Class4=CConfigDlg
Resource4=IDD_EXPIRE_SOON
Class5=ExitXpilots
Resource5=IDD_CONF_GENERAL
Class6=CReallyShutdown
Resource6=IDD_REALLYSHUTDOWN
Resource7=IDD_CONF_BOUNCE
Resource8=IDD_CONF_TEAMS
Resource9=IDD_EXPIRED
Class7=CConfGeneral
Class8=CConfItems
Class9=CConfigDialog
Resource10=IDD_CONF_WORLD
Resource11=IDD_CONF_PLAYERS
Resource12=IDD_CONF_ROBOTS
Resource13=IDD_XPILOTS_DIALOG
Resource14=IDD_EXITXPILOTS
Resource15=IDD_ABOUTBOX
Class10=CConfWorld
Class11=CConfRobots
Class12=CConfBounce
Class13=CConfPlayers
Class14=CConfTeams
Class15=CConfShipsShots
Resource16=IDD_CONFIG_DLG

[CLS:CXpilotsApp]
Type=0
HeaderFile=xpilots.h
ImplementationFile=xpilots.cpp
Filter=N

[CLS:CXpilotsDlg]
Type=0
HeaderFile=xpilotsDlg.h
ImplementationFile=xpilotsDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CXpilotsDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=xpilotsDlg.h
ImplementationFile=xpilotsDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=6
Control1=IDC_STATIC,static,1342177283
Control2=IDC_VERSION,static,1342308480
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_BUTTON_WWW_XPILOT_ORG,button,1342242816
Control6=IDC_BUTTON_WWW_BUCKOSOFT_COM,button,1342242816

[DLG:IDD_XPILOTS_DIALOG]
Type=1
Class=CXpilotsDlg
ControlCount=9
Control1=IDC_START_SERVER,button,1342242816
Control2=IDC_CONFIGURE,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_MAPNAME,static,1342308352
Control5=IDC_STATIC,button,1342177287
Control6=IDC_EDIT1,edit,1352730820
Control7=IDC_STATIC,static,1342308352
Control8=IDC_PLAYERCOUNT,static,1342308352
Control9=IDC_STATIC,button,1342177287

[CLS:CConfigDlg]
Type=0
HeaderFile=configdlg.h
ImplementationFile=configdlg.cpp
BaseClass=CDialog
LastObject=IDC_SETOPTIONS
Filter=D
VirtualFilter=dWC

[DLG:IDD_CONFIG_DLG]
Type=1
Class=CConfigDialog
ControlCount=5
Control1=IDC_COMMANDLINE,edit,1352728580
Control2=IDOK,button,1342242817
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_SETOPTIONS,button,1342242816

[DLG:IDD_EXITXPILOTS]
Type=1
Class=ExitXpilots
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308353

[CLS:ExitXpilots]
Type=0
HeaderFile=ExitXpilots.h
ImplementationFile=ExitXpilots.cpp
BaseClass=CDialog
Filter=D
LastObject=ExitXpilots



[DLG:IDD_REALLYSHUTDOWN]
Type=1
Class=CReallyShutdown
ControlCount=4
Control1=IDCANCEL,button,1342242817
Control2=IDC_EDIT1,edit,1484857472
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342242817

[CLS:CReallyShutdown]
Type=0
HeaderFile=ReallyShutdown.h
ImplementationFile=ReallyShutdown.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CReallyShutdown

[DLG:IDD_CONF_GENERAL]
Type=1
Class=CConfGeneral
ControlCount=11
Control1=IDC_CHECK_REPORTTOMETA,button,1342242822
Control2=IDC_SLIDER_FPS,msctls_trackbar32,1342242821
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_BUTTON1,button,1342242816
Control5=IDC_EDIT_FPS,edit,1350633600
Control6=IDC_EDIT2,edit,1350631552
Control7=IDC_BUTTON2,button,1342242816
Control8=IDC_BUTTON_REPORTTOMETA,button,1342275584
Control9=IDC_BUTTON_NOQUIT,button,1342275584
Control10=IDC_CHECK_NOQUIT,button,1342242822
Control11=IDC_BUTTON_FRAMESPERSECOND,button,1342275584

[DLG:IDD_EXPIRED]
Type=1
Class=?
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,static,1342308353
Control5=IDC_STATIC,static,1342308353
Control6=IDC_STATIC,static,1342308353
Control7=IDC_STATIC,static,1342177283
Control8=IDC_STATIC,static,1342177283

[DLG:IDD_EXPIRE_SOON]
Type=1
Class=?
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308353

[DLG:IDD_CONF_ITEMS]
Type=1
Class=CConfItems
ControlCount=61
Control1=IDC_CHECK_MAXMISSILESPERPACK,button,1342246915
Control2=IDC_EDIT_MAXMISSILESPERPACK,edit,1350639744
Control3=IDC_CHECK_NUKEMINSMARTS,button,1342246915
Control4=IDC_EDIT_NUKEMINSMARTS,edit,1350639744
Control5=IDC_CHECK_NUKEMINMINES,button,1342246915
Control6=IDC_EDIT_NUKEMINMINES,edit,1350639744
Control7=IDC_CHECK_MINEFUSETIME,button,1342246915
Control8=IDC_EDIT_MINEFUSETIME,edit,1350639744
Control9=IDC_CHECK_NUKECLUSTERDAMAGE,button,1342246915
Control10=IDC_SLIDER_NUKECLUSTERDAMAGE,msctls_trackbar32,1342242840
Control11=IDC_CHECK_MOVINGITEMPROB,button,1342246915
Control12=IDC_SLIDER_MOVINGITEMPROB,msctls_trackbar32,1342242840
Control13=IDC_CHECK_DROPITEMONKILLPROB,button,1342246915
Control14=IDC_SLIDER_DROPITEMONKILLPROB,msctls_trackbar32,1342242840
Control15=IDC_CHECK_DESTROYITEMINCOLLISIONPROB,button,1342246915
Control16=IDC_SLIDER_DESTROYITEMINCOLLISIONPROB,msctls_trackbar32,1342242840
Control17=IDC_CHECK_ITEMPROBMULT,button,1342246915
Control18=IDC_SLIDER_ITEMPROBMULT,msctls_trackbar32,1342242840
Control19=IDC_CHECK_MAXITEMDENSITY,button,1342246915
Control20=IDC_SLIDER_MAXITEMDENSITY,msctls_trackbar32,1342242840
Control21=IDC_CHECK_ITEMCONCENTRATORRADIUS,button,1342246915
Control22=IDC_SLIDER_ITEMCONCENTRATORRADIUS,msctls_trackbar32,1342242840
Control23=IDC_CHECK_ITEMCONCENTRATORPROB,button,1342246915
Control24=IDC_SLIDER_ITEMCONCENTRATORPROB,msctls_trackbar32,1342242840
Control25=IDC_CHECK_ROGUEHEATPROB,button,1342246915
Control26=IDC_SLIDER_ROGUEHEATPROB,msctls_trackbar32,1342242840
Control27=IDC_CHECK_ROGUEMINEPROB,button,1342246915
Control28=IDC_SLIDER_ROGUEMINEPROB,msctls_trackbar32,1342242840
Control29=IDC_STATIC,static,1342308352
Control30=IDC_CHECK_ITEMENERGYPACK,button,1342246915
Control31=IDC_SLIDER_ITEMENERGYPACK,msctls_trackbar32,1342242840
Control32=IDC_CHECK_ITEMTANKPROB,button,1342246915
Control33=IDC_SLIDER_ITEMTANKPROB,msctls_trackbar32,1342242840
Control34=IDC_CHECK_ITEMECMPROB,button,1342246915
Control35=IDC_SLIDER_ITEMECMPROB,msctls_trackbar32,1342242840
Control36=IDC_CHECK_ITEMMINEPROB,button,1342246915
Control37=IDC_SLIDER_ITEMMINEPROB,msctls_trackbar32,1342242840
Control38=IDC_CHECK_ITEMMISSILEPROB,button,1342246915
Control39=IDC_SLIDER_ITEMMISSILEPROB,msctls_trackbar32,1342242840
Control40=IDC_CHECK_ITEMCLOAKPROB,button,1342246915
Control41=IDC_SLIDER_ITEMCLOAKPROB,msctls_trackbar32,1342242840
Control42=IDC_CHECK_ITEMSENSORPROB,button,1342246915
Control43=IDC_SLIDER_ITEMSENSORPROB,msctls_trackbar32,1342242840
Control44=IDC_CHECK_ITEMWIDEANGLEPROB,button,1342246915
Control45=IDC_SLIDER_ITEMWIDEANGLEPROB,msctls_trackbar32,1342242840
Control46=IDC_CHECK_ITEMREARSHOTPROB,button,1342246915
Control47=IDC_SLIDER_ITEMREARSHOTPROB,msctls_trackbar32,1342242840
Control48=IDC_CHECK_ITEMAFTERBURNERPROB,button,1342246915
Control49=IDC_SLIDER_ITEMAFTERBURNERPROB,msctls_trackbar32,1342242840
Control50=IDC_CHECK_ITEMTRANSPORTERPROB,button,1342246915
Control51=IDC_SLIDER_ITEMTRANSPORTERPROB,msctls_trackbar32,1342242840
Control52=IDC_CHECK_ITEMLASERPROB,button,1342246915
Control53=IDC_SLIDER_ITEMLASERPROB,msctls_trackbar32,1342242840
Control54=IDC_CHECK_ITEMEMERGENCYTHRUSTPROB,button,1342246915
Control55=IDC_SLIDER_ITEMEMERGENCYTHRUSTPROB,msctls_trackbar32,1342242840
Control56=IDC_CHECK_ITEMTRACTORBEAMPROB,button,1342246915
Control57=IDC_SLIDER_ITEMTRACTORBEAMPROB,msctls_trackbar32,1342242840
Control58=IDC_CHECK_ITEMAUTOPILOTPROB,button,1342246915
Control59=IDC_SLIDER_ITEMAUTOPILOTPROB,msctls_trackbar32,1342242840
Control60=IDC_CHECK_ITEMEMERGENCYSHIELDPROB,button,1342246915
Control61=IDC_SLIDER_ITEMEMERGENCYSHIELDPROB,msctls_trackbar32,1342242840

[CLS:CConfGeneral]
Type=0
HeaderFile=ConfGeneral.h
ImplementationFile=ConfGeneral.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfGeneral

[CLS:CConfItems]
Type=0
HeaderFile=ConfItems.h
ImplementationFile=ConfItems.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfItems
VirtualFilter=idWC

[CLS:CConfigDialog]
Type=0
HeaderFile=ConfigDialog.h
ImplementationFile=ConfigDialog.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_TAB1
VirtualFilter=idWC

[DLG:IDD_CONF_BOUNCE]
Type=1
Class=CConfBounce
ControlCount=0

[DLG:IDD_CONF_PLAYERS]
Type=1
Class=CConfPlayers
ControlCount=0

[DLG:IDD_CONF_TEAMS]
Type=1
Class=CConfTeams
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

[DLG:IDD_CONF_ROBOTS]
Type=1
Class=CConfRobots
ControlCount=0

[DLG:IDD_CONF_SHIPSSHOTS]
Type=1
Class=CConfShipsShots
ControlCount=2
Control1=IDC_CHECK_SHIPMASS,button,1342246915
Control2=IDC_EDIT_SHIPMASS,edit,1350639744

[DLG:IDD_CONF_WORLD]
Type=1
Class=CConfWorld
ControlCount=0

[CLS:CConfWorld]
Type=0
HeaderFile=ConfWorld.h
ImplementationFile=ConfWorld.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfWorld

[CLS:CConfBounce]
Type=0
HeaderFile=ConfBounce.h
ImplementationFile=ConfBounce.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfBounce

[CLS:CConfPlayers]
Type=0
HeaderFile=ConfPlayers.h
ImplementationFile=ConfPlayers.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfPlayers

[CLS:CConfTeams]
Type=0
HeaderFile=ConfTeams.h
ImplementationFile=ConfTeams.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfTeams

[CLS:CConfRobots]
Type=0
HeaderFile=ConfRobots.h
ImplementationFile=ConfRobots.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CConfRobots

[CLS:CConfShipsShots]
Type=0
HeaderFile=ConfShipsShots.h
ImplementationFile=ConfShipsShots.cpp
BaseClass=CPropertyPage
Filter=D


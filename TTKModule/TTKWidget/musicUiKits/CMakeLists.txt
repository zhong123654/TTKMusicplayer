cmake_minimum_required(VERSION 2.8.11)

set_property(GLOBAL PROPERTY MUSIC_WIDGET_UI_KITS_HEADERS
    ${MUSIC_WIDGET_UI_DIR}/musicuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musiccontextuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicdesktoplrcuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicfunctionlistuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicfunctionuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicinteriorfloatuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicinteriorlrcuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicleftitemlistuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicmagicwidgetuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicqualitywidgetuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musictinyuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musictoolsetsuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicvideofloatuiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicvideouiobject.h
    ${MUSIC_WIDGET_UI_DIR}/musicwidgetheaders.h
  )

set_property(GLOBAL PROPERTY MUSIC_UIS
    ${MUSIC_WIDGET_UI_DIR}/musicApplication/musicapplication.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrcartphotouploadwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrcmakerwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrcsearchwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrclocallinkwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrccolorwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrcposterwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicLrcKits/musiclrcdownloadbatchwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicfileinformationwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musiclocalsongsmanagerwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musictimerwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musictransformwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicnetworkconnectiontestwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicreplaygainwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicsongchecktoolswidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicsongringtonemakerwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicsoundkmicrowidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicspectrumwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsetsKits/musicsongdlnatransferwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musicwebfmradioplaywidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musicconnecttransferwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musiccloudfileinformationwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musictoolsetswidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musicttkfmradioplaywidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicToolsKits/musicttkfmradioinformationwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicbackgroundskindialog.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicbackgroundpalettewidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicdownloadwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicdownloadresetwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicequalizerdialog.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicmessagebox.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musiccolordialog.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsettingwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsongsharingwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsongslistiteminfowidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsourceupdatewidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsoundeffectswidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicmessageaboutdialog.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicdownloadbatchwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicsongitemselecteddialog.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicpluginwidget.ui
    ${MUSIC_WIDGET_UI_DIR}/musicWidgetKits/musicinputdialog.ui
)

#include "musiclrccontainer.h"
#include "musiclrcsearchwidget.h"
#include "musiclrcmakerwidget.h"

#include <QActionGroup>

MusicLrcContainer::MusicLrcContainer(QWidget *parent)
    : QWidget(parent)
{
    m_currentTime = 0;
    m_totalTime = 0;
    m_linkLocalLrc = true;
    m_lrcAnalysis = nullptr;
    m_lrcSearchWidget = nullptr;
}

MusicLrcContainer::~MusicLrcContainer()
{
    delete m_lrcSearchWidget;
}

void MusicLrcContainer::applySettingParameter()
{
    applySettingParameter(m_containerType == LRC_DESKTOP_TPYE ? LRC_DESKTOP_PREFIX : QString());
}

void MusicLrcContainer::setLinearGradientColor(MusicLrcColor::LrcColorType lrcColorType)
{
    const MusicLrcColor &cl = MusicLrcColor::mapIndexToColor(lrcColorType);
    setLinearGradientColor(cl);
}

void MusicLrcContainer::setLinearGradientColor(const MusicLrcColor &color)
{
    for(MusicLrcManager *manager : qAsConst(m_lrcManagers))
    {
        manager->setLinearGradientColor(color);
    }

    G_SETTING_PTR->setValue((m_containerType == LRC_DESKTOP_TPYE) ? MusicSettingManager::DLrcColor : MusicSettingManager::LrcColor, color.m_index);
    Q_EMIT linearGradientColorChanged();
}

void MusicLrcContainer::setCurrentTime(qint64 time, qint64 total)
{
    m_currentTime = time;
    m_totalTime = total;
}

qint64 MusicLrcContainer::totalTime() const
{
    return m_totalTime;
}

void MusicLrcContainer::currentLrcCustom()
{
    Q_EMIT changeCurrentLrcColorCustom();
    Q_EMIT changeCurrentLrcColorSetting();
}

void MusicLrcContainer::changeCurrentLrcColor(QAction *action)
{
    switch(action->data().toInt())
    {
        case 0: setLinearGradientColor(MusicLrcColor::IYellow); break;
        case 1: setLinearGradientColor(MusicLrcColor::IBlue); break;
        case 2: setLinearGradientColor(MusicLrcColor::IGray); break;
        case 3: setLinearGradientColor(MusicLrcColor::IPink); break;
        case 4: setLinearGradientColor(MusicLrcColor::IGreen); break;
        case 5: setLinearGradientColor(MusicLrcColor::IRed); break;
        case 6: setLinearGradientColor(MusicLrcColor::IPurple); break;
        case 7: setLinearGradientColor(MusicLrcColor::IOrange); break;
        case 8: setLinearGradientColor(MusicLrcColor::IIndigo); break;
        case 9: setLinearGradientColor(MusicLrcColor::DWhite); break;
        case 10: setLinearGradientColor(MusicLrcColor::DBlue); break;
        case 11: setLinearGradientColor(MusicLrcColor::DRed); break;
        case 12: setLinearGradientColor(MusicLrcColor::DBlack); break;
        case 13: setLinearGradientColor(MusicLrcColor::DYellow); break;
        case 14: setLinearGradientColor(MusicLrcColor::DPurple); break;
        case 15: setLinearGradientColor(MusicLrcColor::DGreen); break;
        default: break;
    }
}

void MusicLrcContainer::changeCurrentLrcColor(int index)
{
    setLinearGradientColor(TTKStatic_cast(MusicLrcColor::LrcColorType, index));
}

void MusicLrcContainer::searchMusicLrcs()
{
    delete m_lrcSearchWidget;
    m_lrcSearchWidget = new MusicLrcSearchWidget(this);
    m_lrcSearchWidget->setCurrentSongName(m_currentSongName);
    m_lrcSearchWidget->exec();
}

void MusicLrcContainer::showLrcMakedWidget()
{
    MusicLrcMakerWidget *w = GENERATE_SINGLE_WIDGET_PARENT(MusicLrcMakerWidget, this);
    w->setCurrentSongName(m_currentSongName);
    w->durationChanged(m_totalTime);
}

void MusicLrcContainer::linkLrcStateChanged()
{
    m_linkLocalLrc = !m_linkLocalLrc;
    for(MusicLrcManager *w : qAsConst(m_lrcManagers))
    {
        w->setVisible(m_linkLocalLrc);
    }
}

void MusicLrcContainer::clearAllMusicLRCManager()
{
    qDeleteAll(m_lrcManagers);
    m_lrcManagers.clear();
}

void MusicLrcContainer::applySettingParameter(const QString &t)
{
    for(MusicLrcManager *manager : qAsConst(m_lrcManagers))
    {
        manager->setFontFamily(G_SETTING_PTR->value(t + "LrcFamily").toInt());
        manager->setFontType(G_SETTING_PTR->value(t + "LrcType").toInt());
        manager->setFontTransparent(G_SETTING_PTR->value(t + "LrcColorTransparent").toInt());
        manager->setLrcFontSize(G_SETTING_PTR->value(t + "LrcSize").toInt());
    }
    if(G_SETTING_PTR->value(t + "LrcColor").toInt() != -1)
    {
        const MusicLrcColor::LrcColorType index = TTKStatic_cast(MusicLrcColor::LrcColorType, G_SETTING_PTR->value(t + "LrcColor").toInt());
        setLinearGradientColor(index);
    }
    else
    {
        const MusicLrcColor cl(MusicLrcColor::readColorConfig(G_SETTING_PTR->value(t + "LrcFrontgroundColor").toString()),
                               MusicLrcColor::readColorConfig(G_SETTING_PTR->value(t + "LrcBackgroundColor").toString()));
        setLinearGradientColor(cl);
    }
}

#include "musiclocalsongstablewidget.h"
#include "musicnumberutils.h"

#define MUSIC_INFO_ROLE     Qt::UserRole + 1000

Q_DECLARE_METATYPE(QFileInfoList)

MusicLocalSongsTableWidget::MusicLocalSongsTableWidget(QWidget *parent)
    : MusicAbstractSongsListTableWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    MusicUtils::Widget::setTransparent(this, 150);

    setColumnCount(5);

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 405);
    headerview->resizeSection(1, 65);
    headerview->resizeSection(2, 105);
    headerview->resizeSection(3, 26);
    headerview->resizeSection(4, 26);

    m_songs = new MusicSongList;
}

MusicLocalSongsTableWidget::~MusicLocalSongsTableWidget()
{
    clear();
    delete m_songs;
}

void MusicLocalSongsTableWidget::clear()
{
    MusicAbstractTableWidget::clear();
    m_songs->clear();
}

void MusicLocalSongsTableWidget::addItems(const QFileInfoList &path)
{
    QHeaderView *headerview = horizontalHeader();
    for(int i = 0; i < path.count(); ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setToolTip(path[i].fileName());
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(0) - 20));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 0, item);

                         item = new QTableWidgetItem;
        item->setToolTip(MusicUtils::Number::sizeByte2Label(path[i].size()));
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(1) - 15));
        QtItemSetTextAlignment(item, Qt::AlignRight | Qt::AlignVCenter);
        setItem(i, 1, item);

                         item = new QTableWidgetItem(path[i].lastModified().date().toString(Qt::ISODate));
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 2, item);

                         item = new QTableWidgetItem;
        item->setIcon(QIcon(":/contextMenu/btn_audition"));
        setItem(i, 3, item);

                         item = new QTableWidgetItem;
        item->setIcon(QIcon(":/contextMenu/btn_add"));
        setItem(i, 4, item);

        m_songs->append(MusicSong(path[i].absoluteFilePath()));
    }
}

void MusicLocalSongsTableWidget::itemCellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void MusicLocalSongsTableWidget::contextMenuEvent(QContextMenuEvent *event)
{
    MusicAbstractSongsListTableWidget::contextMenuEvent(event);

    QMenu menu(this);
    menu.setStyleSheet(MusicUIObject::MQSSMenuStyle02);
    menu.addAction(QIcon(":/contextMenu/btn_play"), tr("Play"), this, SLOT(musicPlayClicked()));
    menu.addAction(tr("Download More..."), this, SLOT(musicSongDownload()));
    menu.addSeparator();

    createMoreMenu(&menu);

    const bool status = !m_songs->isEmpty();
    menu.addAction(tr("Song Info..."), this, SLOT(musicFileInformation()))->setEnabled(status);
    menu.addAction(QIcon(":/contextMenu/btn_localFile"), tr("Open File Dir"), this, SLOT(musicOpenFileDir()))->setEnabled(status);
    menu.addAction(QIcon(":/contextMenu/btn_ablum"), tr("Ablum"), this, SLOT(musicAlbumQueryWidget()));
    menu.addSeparator();

    menu.exec(QCursor::pos());
}



MusicLocalSongsInfoTableWidget::MusicLocalSongsInfoTableWidget(QWidget *parent)
    : MusicAbstractTableWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    MusicUtils::Widget::setTransparent(this, 150);

    setColumnCount(3);
    setIconSize(QSize(50, 50));

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 60);
    headerview->resizeSection(1, 447);
    headerview->resizeSection(2, 120);

    connect(this, SIGNAL(cellDoubleClicked(int,int)), SLOT(itemDoubleClicked(int,int)));
}

void MusicLocalSongsInfoTableWidget::clear()
{
    MusicAbstractTableWidget::clear();
}

void MusicLocalSongsInfoTableWidget::addItems(const MusicInfoData &data)
{
    QHeaderView *headerview = horizontalHeader();
    MusicInfoDataIterator it(data);
    int i=0;
    while(it.hasNext())
    {
        it.next();
        setRowHeight(i, ITEM_ROW_HEIGHT_XL);

        QTableWidgetItem *item = new QTableWidgetItem;
        QPixmap pix(ART_DIR_FULL + it.key() + SKN_FILE);
        if(pix.isNull())
        {
            pix.load(":/image/lb_default_art");
        }

        item->setIcon(QIcon(pix));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 0, item);

                         item = new QTableWidgetItem;
        item->setToolTip(it.key());
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(1) - 20));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 1, item);

                         item = new QTableWidgetItem;
        item->setText(tr("All number %1").arg(it.value().count()));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        QVariant v;
        v.setValue(it.value());
        item->setData(MUSIC_INFO_ROLE, v);
        setItem(i, 2, item);

        ++i;
    }
}

void MusicLocalSongsInfoTableWidget::itemCellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void MusicLocalSongsInfoTableWidget::itemDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QTableWidgetItem *it = item(row, 2);
    if(it)
    {
        QFileInfoList list = it->data(MUSIC_INFO_ROLE).value<QFileInfoList>();
        Q_EMIT updateFileList(list);
    }
}

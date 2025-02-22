#include "musickwqueryplaylistrequest.h"

MusicKWQueryPlaylistRequest::MusicKWQueryPlaylistRequest(QObject *parent)
    : MusicQueryPlaylistRequest(parent)
{
    m_pageSize = 30;
    m_queryServer = QUERY_KW_INTERFACE;
}

void MusicKWQueryPlaylistRequest::startToSearch(QueryType type, const QString &value)
{
    if(type == MusicQuery)
    {
        startToSearch(value);
    }
    else
    {
        m_queryValue = value.isEmpty() ? "167" : value;
        startToPage(0);
    }
}

void MusicKWQueryPlaylistRequest::startToPage(int offset)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className()).arg(offset));

    deleteAll();
    m_totalSize = 0;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_URL, false).arg(m_queryValue).arg(offset).arg(m_pageSize));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicKWQueryPlaylistRequest::startToSearch(const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className(), value));

    deleteAll();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(value));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, SIGNAL(finished()), SLOT(downloadDetailsFinished()));
    QtNetworkErrorConnect(reply, this, replyError);
}

void MusicKWQueryPlaylistRequest::queryPlaylistInfo(MusicResultsItem &item)
{
    TTK_LOGGER_INFO(QString("%1 queryPlaylistInfo %2").arg(className(), item.m_id));

    MusicPageQueryRequest::downLoadFinished();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(item.m_id));
    MusicKWInterface::makeRequestRawHeader(&request);

    const QByteArray &bytes = MusicObject::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return;
    }

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        if(!value.isEmpty())
        {
            item.m_coverUrl = value["pic"].toString();
            item.m_name = value["title"].toString();
            item.m_playCount = QString::number(value["playnum"].toULongLong());
            item.m_description = value["info"].toString();
            item.m_updateTime = QDateTime::fromMSecsSinceEpoch(value["ctime"].toULongLong() * 1000).toString(MUSIC_YEAR_FORMAT);
            item.m_nickName = value["uname"].toString();
        }
    }
}

void MusicKWQueryPlaylistRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(className()));

    MusicQueryPlaylistRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            m_totalSize = value["total"].toString().toLongLong();
            if(value.contains("child"))
            {
                m_tags = value["ninfo"].toMap()["name"].toString();
                const QVariantList &datas = value["child"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();
                    morePlaylistDetails(value["sourceid"].toString());
                    TTK_NETWORK_QUERY_CHECK();
                }
            }
        }
    }

//    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicKWQueryPlaylistRequest::downloadDetailsFinished()
{
    TTK_LOGGER_INFO(QString("%1 downloadDetailsFinished").arg(className()));

    MusicQueryPlaylistRequest::downLoadFinished();
    QNetworkReply *reply = TTKObject_cast(QNetworkReply*, QObject::sender());
    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(!value.isEmpty() && value.contains("musiclist"))
            {
                const QVariantList &datas = value["musiclist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation info;
                    info.m_singerName = MusicUtils::String::charactersReplaced(value["artist"].toString());
                    info.m_songName = MusicUtils::String::charactersReplaced(value["name"].toString());
                    info.m_duration = MusicTime::msecTime2LabelJustified(value["duration"].toInt() * 1000);

                    info.m_songId = value["id"].toString();
                    info.m_artistId = value["artistid"].toString();
                    info.m_albumId = value["albumid"].toString();
                    info.m_albumName = MusicUtils::String::charactersReplaced(value["album"].toString());

                    info.m_year = QString();
                    info.m_discNumber = "1";
                    info.m_trackNumber = "0";

                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicSongPicture(&info);
                    TTK_NETWORK_QUERY_CHECK();
                    info.m_lrcUrl = MusicUtils::Algorithm::mdII(KW_SONG_LRC_URL, false).arg(info.m_songId);
                    readFromMusicSongProperty(&info, value["formats"].toString(), m_queryQuality, m_queryAllRecords);
                    TTK_NETWORK_QUERY_CHECK();

                    if(info.m_songProps.isEmpty())
                    {
                        continue;
                    }

                    if(!findUrlFileSize(&info.m_songProps))
                    {
                        return;
                    }

                    MusicSearchedItem item;
                    item.m_songName = info.m_songName;
                    item.m_singerName = info.m_singerName;
                    item.m_albumName = info.m_albumName;
                    item.m_duration = info.m_duration;
                    item.m_type = mapQueryServerString();
                    Q_EMIT createSearchedItem(item);
                    m_songInfos << info;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
}

void MusicKWQueryPlaylistRequest::downloadMoreDetailsFinished()
{
    TTK_LOGGER_INFO(QString("%1 downloadMoreDetailsFinished").arg(className()));

    QNetworkReply *reply = TTKObject_cast(QNetworkReply*, QObject::sender());
    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(reply->readAll(), &ok);
        if(ok)
        {
            const QVariantMap &value = data.toMap();
            if(value["result"].toString() == "ok")
            {
                MusicResultsItem result;
                result.m_tags = m_tags;
                result.m_coverUrl = value["pic"].toString();
                result.m_id = value["id"].toString();
                result.m_name = value["title"].toString();
                result.m_playCount = QString::number(value["playnum"].toULongLong());
                result.m_description = value["info"].toString();
                result.m_updateTime = QDateTime::fromMSecsSinceEpoch(value["ctime"].toULongLong() * 1000).toString(MUSIC_YEAR_FORMAT);
                result.m_nickName = value["uname"].toString();
                Q_EMIT createPlaylistItem(result);
            }
        }
    }
}

void MusicKWQueryPlaylistRequest::morePlaylistDetails(const QString &pid)
{
    TTK_LOGGER_INFO(QString("%1 morePlaylistDetails %2").arg(className(), pid));

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(pid));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, SIGNAL(finished()), SLOT(downloadMoreDetailsFinished()));
    QtNetworkErrorConnect(reply, this, replyError);
}

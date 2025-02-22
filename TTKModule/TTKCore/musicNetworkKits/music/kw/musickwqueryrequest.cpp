#include "musickwqueryrequest.h"

MusicKWMusicInfoConfigManager::MusicKWMusicInfoConfigManager(QObject *parent)
    : MusicAbstractXml(parent)
{

}

void MusicKWMusicInfoConfigManager::readBuffer(MusicObject::MusicSongInformation *info)
{
    info->m_singerName = readXmlTextByTagName("artist");
    info->m_songName = readXmlTextByTagName("name");
    info->m_songId = readXmlTextByTagName("music_id");
    info->m_artistId = readXmlTextByTagName("albid");
    info->m_albumId = readXmlTextByTagName("artid");
    info->m_albumName = readXmlTextByTagName("special");

    const QString &mp3Url = readXmlTextByTagName("mp3dl");
    if(!mp3Url.isEmpty())
    {
        QString v = readXmlTextByTagName("mp3path");
        if(!v.isEmpty())
        {
            MusicObject::MusicSongProperty prop;
            prop.m_bitrate = MB_128;
            prop.m_format = MP3_FILE_PREFIX;
            prop.m_size = TTK_DEFAULT_STR;
            prop.m_url = QString("%1%2/resource/%3").arg(HTTP_PREFIX, mp3Url, v);
            info->m_songProps.append(prop);
        }

        v = readXmlTextByTagName("path");
        if(!v.isEmpty())
        {
            MusicObject::MusicSongProperty prop;
            prop.m_bitrate = MB_96;
            prop.m_format = WMA_FILE_PREFIX;
            prop.m_size = TTK_DEFAULT_STR;
            prop.m_url = QString("%1%2/resource/%3").arg(HTTP_PREFIX, mp3Url, v);
            info->m_songProps.append(prop);
        }
    }

    const QString &aacUrl = readXmlTextByTagName("aacdl");
    if(!aacUrl.isEmpty())
    {
        const QString &v = readXmlTextByTagName("aacpath");
        if(!v.isEmpty())
        {
            MusicObject::MusicSongProperty prop;
            prop.m_bitrate = MB_32;
            prop.m_format = AAC_FILE_PREFIX;
            prop.m_size = TTK_DEFAULT_STR;
            prop.m_url = QString("%1%2/resource/%3").arg(HTTP_PREFIX, aacUrl, v);
            info->m_songProps.append(prop);
        }
    }
}



MusicKWQueryRequest::MusicKWQueryRequest(QObject *parent)
    : MusicAbstractQueryRequest(parent)
{
    m_pageSize = 40;
    m_queryServer = QUERY_KW_INTERFACE;
}

void MusicKWQueryRequest::startToSearch(QueryType type, const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className(), value));

    m_queryType = type;
    m_queryValue = value.trimmed();
    MusicAbstractQueryRequest::downLoadFinished();

    startToPage(0);
}

void MusicKWQueryRequest::startToPage(int offset)
{
    TTK_LOGGER_INFO(QString("%1 startToPage %2").arg(className()).arg(offset));

    deleteAll();
    m_totalSize = 0;
    m_pageIndex = offset;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_SONG_SEARCH_URL, false).arg(m_queryValue).arg(offset).arg(m_pageSize));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicKWQueryRequest::startToSingleSearch(const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSingleSearch %2").arg(className(), value));

    deleteAll();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_SONG_INFO_URL, false).arg(value));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkReply *reply = m_manager.get(request);
    connect(reply, SIGNAL(finished()), SLOT(downLoadSingleFinished()));
    QtNetworkErrorConnect(reply, this, replyError);
}

void MusicKWQueryRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(className()));

    MusicPageQueryRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll().replace("'", "\""), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value.contains("abslist"))
            {
                m_totalSize = value["TOTAL"].toInt();
                const QVariantList &datas = value["abslist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation info;
                    info.m_singerName = MusicUtils::String::charactersReplaced(value["ARTIST"].toString());
                    info.m_songName = MusicUtils::String::charactersReplaced(value["SONGNAME"].toString());
                    info.m_duration = MusicTime::msecTime2LabelJustified(value["DURATION"].toInt() * 1000);

                    info.m_songId = value["MUSICRID"].toString().replace("MUSIC_", "");
                    info.m_artistId = value["ARTISTID"].toString();
                    info.m_albumId = value["ALBUMID"].toString();

                    info.m_year = value["RELEASEDATE"].toString();
                    info.m_discNumber = "1";
                    info.m_trackNumber = "0";

                    if(!m_queryLite)
                    {
                        TTK_NETWORK_QUERY_CHECK();
                        readFromMusicSongPicture(&info);
                        TTK_NETWORK_QUERY_CHECK();
                        info.m_lrcUrl = MusicUtils::Algorithm::mdII(KW_SONG_LRC_URL, false).arg(info.m_songId);
                        info.m_albumName = MusicUtils::String::charactersReplaced(value["ALBUM"].toString());
                        readFromMusicSongProperty(&info, value["FORMATS"].toString(), m_queryQuality, m_queryAllRecords);
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
                    }
                    m_songInfos << info;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicKWQueryRequest::downLoadSingleFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadSingleFinished").arg(className()));

    MusicAbstractQueryRequest::downLoadFinished();
    QNetworkReply *reply = TTKObject_cast(QNetworkReply*, QObject::sender());
    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        data.insert(0, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
        data.replace("&", "%26");

        MusicKWMusicInfoConfigManager xml;
        if(xml.fromByteArray(data))
        {
            MusicObject::MusicSongInformation info;
            xml.readBuffer(&info);

            info.m_year = QString();
            info.m_discNumber = "1";
            info.m_trackNumber = "0";

            TTK_NETWORK_QUERY_CHECK();
            readFromMusicSongPicture(&info);
            TTK_NETWORK_QUERY_CHECK();
            info.m_lrcUrl = MusicUtils::Algorithm::mdII(KW_SONG_LRC_URL, false).arg(info.m_songId);

            if(!findUrlFileSize(&info.m_songProps))
            {
                return;
            }

            if(!info.m_songProps.isEmpty())
            {
                MusicSearchedItem item;
                item.m_songName = info.m_songName;
                item.m_singerName = info.m_singerName;
                item.m_duration = info.m_duration;
                item.m_type = mapQueryServerString();
                Q_EMIT createSearchedItem(item);
                m_songInfos << info;
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

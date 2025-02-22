#include "musicqqquerymovierequest.h"

MusicQQQueryMovieRequest::MusicQQQueryMovieRequest(QObject *parent)
    : MusicQueryMovieRequest(parent)
{
    m_pageSize = 30;
    m_queryServer = QUERY_QQ_INTERFACE;
}

void MusicQQQueryMovieRequest::startToSearch(QueryType type, const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className(), value));

    deleteAll();
    m_queryType = type;
    m_queryValue = value.trimmed();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_SONG_SEARCH_URL, false).arg(value).arg(0).arg(m_pageSize));
    MusicQQInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished())); 
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicQQQueryMovieRequest::startToPage(int offset)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className()).arg(offset));

    deleteAll();
    m_totalSize = 0;
    m_pageSize = 20;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_ARTIST_MOVIE_URL, false).arg(m_queryValue).arg(m_pageSize * offset).arg(m_pageSize));
    MusicQQInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadPageFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicQQQueryMovieRequest::startToSingleSearch(const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSingleSearch %2").arg(className(), value));

    deleteAll();
    m_queryValue = value.trimmed();

    QTimer::singleShot(MT_MS, this, SLOT(downLoadSingleFinished()));
}

void MusicQQQueryMovieRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(className()));

    MusicQueryMovieRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value.contains("data"))
            {
                value = value["data"].toMap();
                value = value["song"].toMap();
                const QVariantList &datas = value["list"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation info;
                    for(const QVariant &var : value["singer"].toList())
                    {
                        if(var.isNull())
                        {
                            continue;
                        }

                        const QVariantMap &name = var.toMap();
                        info.m_singerName = MusicUtils::String::charactersReplaced(name["name"].toString());
                        info.m_artistId = name["mid"].toString();
                        break; //just find first singer
                    }
                    info.m_songName = MusicUtils::String::charactersReplaced(value["songname"].toString());
                    info.m_duration = MusicTime::msecTime2LabelJustified(value["interval"].toInt() * 1000);

                    info.m_songId = value["vid"].toString();
                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicMVProperty(&info, false);
                    TTK_NETWORK_QUERY_CHECK();

                    if(info.m_songProps.isEmpty())
                    {
                        continue;
                    }

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
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::downLoadPageFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadPageFinished").arg(className()));

    MusicPageQueryRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value["code"].toInt() == 0 && value.contains("data"))
            {
                value = value["data"].toMap();
                m_totalSize = value["total"].toInt();
                const QVariantList &datas = value["list"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicResultsItem result;
                    result.m_id = value["vid"].toString();
                    result.m_coverUrl = value["pic"].toString();
                    result.m_name = value["title"].toString();
                    result.m_updateTime.clear();
                    Q_EMIT createMovieInfoItem(result);
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::downLoadSingleFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadSingleFinished").arg(className()));

    MusicQueryMovieRequest::downLoadFinished();

    MusicObject::MusicSongInformation info;
    info.m_songId = m_queryValue;
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVProperty(&info, true);
    TTK_NETWORK_QUERY_CHECK();

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

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::readFromMusicMVProperty(MusicObject::MusicSongInformation *info, bool more) const
{
    if(info->m_songId.isEmpty())
    {
        return;
    }

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_MOVIE_INFO_URL, false).arg(info->m_songId));
    MusicQQInterface::makeRequestRawHeader(&request);

    QByteArray bytes = MusicObject::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return;
    }

    bytes.replace("QZOutputJson=", "");
    bytes.chop(1);

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        QVariantMap value = data.toMap();
        if(value.contains("fl"))
        {
            QString urlPrefix;
            QVariantMap vlValue = value["vl"].toMap();
            QVariantList viList = vlValue["vi"].toList();
            if(!viList.isEmpty())
            {
                vlValue = viList.front().toMap();

                if(more)
                {
                    info->m_singerName = "Default";
                    info->m_songName = vlValue["ti"].toString();
                    info->m_duration = MusicTime::msecTime2LabelJustified(TTKStatic_cast(int, vlValue["td"].toString().toFloat()) * 1000);
                }

                vlValue = vlValue["ul"].toMap();
                viList = vlValue["ui"].toList();
                vlValue = viList.front().toMap();
                urlPrefix = vlValue["url"].toString();
            }

            QVariantMap flValue = value["fl"].toMap();
            const QVariantList &datas = flValue["fi"].toList();
            for(const QVariant &var : qAsConst(datas))
            {
                if(var.isNull())
                {
                    continue;
                }

                flValue = var.toMap();
                TTK_NETWORK_QUERY_CHECK();

                MusicObject::MusicSongProperty prop;
                prop.m_size = MusicUtils::Number::sizeByte2Label(flValue["fs"].toInt());
                prop.m_format = "mp4";

                int bitrate = flValue["br"].toInt() * 10;
                if(bitrate <= 375)
                    prop.m_bitrate = MB_250;
                else if(bitrate > 375 && bitrate <= 625)
                    prop.m_bitrate = MB_500;
                else if(bitrate > 625 && bitrate <= 875)
                    prop.m_bitrate = MB_750;
                else if(bitrate > 875)
                    prop.m_bitrate = MB_1000;

                bitrate = flValue["id"].toULongLong();
                TTK_NETWORK_QUERY_CHECK();
                const QString &key = generateMovieKey(bitrate, info->m_songId);
                TTK_NETWORK_QUERY_CHECK();

                if(!key.isEmpty())
                {
                    const QString &fn = QString("%1.p%2.1.mp4").arg(info->m_songId).arg(bitrate - 10000);
                    prop.m_url = QString("%1%2?vkey=%3").arg(urlPrefix, fn, key);
                    info->m_songProps.append(prop);
                }
            }
        }
    }
}

QString MusicQQQueryMovieRequest::generateMovieKey(int id, const QString &videoId) const
{
    if(videoId.isEmpty())
    {
        return QString();
    }

    const QString &fn = QString("%1.p%2.1.mp4").arg(videoId).arg(id - 10000);

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_MOVIE_KEY_URL, false).arg(id).arg(videoId, fn));
    MusicQQInterface::makeRequestRawHeader(&request);

    QByteArray bytes = MusicObject::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return QString();
    }

    bytes.replace("QZOutputJson=", "");
    bytes.chop(1);

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        if(value.contains("key"))
        {
            return value["key"].toString();
        }
    }

    return QString();
}

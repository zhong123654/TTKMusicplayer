#include "musickwquerymovierequest.h"

#include "qalgorithm/deswrapper.h"

MusicKWQueryMovieRequest::MusicKWQueryMovieRequest(QObject *parent)
    : MusicQueryMovieRequest(parent)
{
    m_pageSize = 40;
    m_queryServer = QUERY_KW_INTERFACE;
}

void MusicKWQueryMovieRequest::startToSearch(QueryType type, const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className(), value));

    deleteAll();
    m_queryType = type;
    m_queryValue = value.trimmed();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_SONG_SEARCH_URL, false).arg(value).arg(0).arg(m_pageSize));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicKWQueryMovieRequest::startToPage(int offset)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className()).arg(offset));

    deleteAll();
    m_totalSize = 0;
    m_pageSize = 20;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_ARTIST_MOVIE_URL, false).arg(m_queryValue).arg(m_pageSize).arg(offset));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadPageFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicKWQueryMovieRequest::startToSingleSearch(const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSingleSearch %2").arg(className(), value));

    deleteAll();
    m_queryValue = value.trimmed();

    QTimer::singleShot(MT_MS, this, SLOT(downLoadSingleFinished()));
}

void MusicKWQueryMovieRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(className()));

    MusicQueryMovieRequest::downLoadFinished();
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

                    info.m_songId = value["MUSICRID"].toString().remove("MUSIC_");
                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicMVProperty(&info, value["FORMATS"].toString());
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

void MusicKWQueryMovieRequest::downLoadPageFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadPageFinished").arg(className()));

    MusicPageQueryRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll().replace("'", "\""), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            m_totalSize = value["total"].toString().toLongLong();
            if(value.contains("mvlist"))
            {
                const QVariantList &datas = value["mvlist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicResultsItem result;
                    result.m_id = value["musicid"].toString();
                    result.m_coverUrl = value["pic"].toString();
                    if(!result.m_coverUrl.contains(HTTP_PREFIX) && !result.m_coverUrl.contains(TTK_NULL_STR))
                    {
                        result.m_coverUrl = MusicUtils::Algorithm::mdII(KW_MOVIE_COVER_URL, false) + result.m_coverUrl;
                    }
                    result.m_name = value["name"].toString();
                    result.m_updateTime.clear();
                    Q_EMIT createMovieInfoItem(result);
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicKWQueryMovieRequest::downLoadSingleFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadSingleFinished").arg(className()));

    MusicQueryMovieRequest::downLoadFinished();

    MusicObject::MusicSongInformation info;
    info.m_songId = m_queryValue;
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVInfo(&info);
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVProperty(&info, QString("MP4UL|MP4L|MP4HV|MP4"));
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

void MusicKWQueryMovieRequest::readFromMusicMVProperty(MusicObject::MusicSongInformation *info, const QString &format) const
{
    if(info->m_songId.isEmpty())
    {
        return;
    }

    for(const QString &v : format.split("|"))
    {
        if(v.contains("MP4L"))
        {
            readFromMusicMVProperty(info, "MP4L", MB_250);
        }
        else if(v.contains("MP4HV"))
        {
            readFromMusicMVProperty(info, "MP4HV", MB_750);
        }
        else if(v.contains("MP4UL"))
        {
            readFromMusicMVProperty(info, "MP4UL", MB_1000);
        }
        else if(v.contains("MP4"))
        {
            readFromMusicMVProperty(info, "MP4", MB_500);
        }
    }
}

void MusicKWQueryMovieRequest::readFromMusicMVProperty(MusicObject::MusicSongInformation *info, const QString &format, int bitrate) const
{
    if(info->m_songId.isEmpty())
    {
        return;
    }

    QAlgorithm::Des des;
    const QByteArray &parameter = des.encrypt(MusicUtils::Algorithm::mdII(KW_MOVIE_ATTR_URL, false).arg(info->m_songId, format).toUtf8(),
                                              MusicUtils::Algorithm::mdII(_SIGN, ALG_UNIMP_KEY, false).toUtf8());

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_MOVIE_URL, false).arg(QString(parameter)));
    MusicKWInterface::makeRequestRawHeader(&request);

    const QByteArray &bytes = MusicObject::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return;
    }

    if(!bytes.isEmpty() && !bytes.contains("res not found"))
    {
        const QString text(bytes);
        QRegExp regx(".*url=(.*)\r\nsig=");

        if(text.indexOf(regx) != -1)
        {
            MusicObject::MusicSongProperty prop;
            prop.m_url = regx.cap(1);
            prop.m_bitrate = bitrate;
            prop.m_format = "mp4";
            if(prop.m_url.isEmpty() || info->m_songProps.contains(prop))
            {
                return;
            }

            if(!findUrlFileSize(&prop))
            {
                return;
            }

            info->m_songProps.append(prop);
        }
    }
}

void MusicKWQueryMovieRequest::readFromMusicMVInfo(MusicObject::MusicSongInformation *info) const
{
    if(info->m_songId.isEmpty())
    {
        return;
    }

    info->m_songName = "Not Found";
    info->m_singerName = "Anonymous";

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_MOVIE_HOME_URL, false).arg(info->m_songId));
    MusicKWInterface::makeRequestRawHeader(&request);

    const QByteArray &bytes = MusicObject::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return;
    }

    const QString text(bytes);
    QRegExp regx("<h1 title=\"([^<]+)\">[^>]+>([^<]+)</span></h1>");

    if(text.indexOf(regx) != -1)
    {
        info->m_songName = regx.cap(1);
        info->m_singerName = regx.cap(2);
    }
}

#include "musickgartistsimilarrequest.h"
#include "musickgqueryinterface.h"

MusicKGArtistSimilarRequest::MusicKGArtistSimilarRequest(QObject *parent)
    : MusicSimilarRequest(parent)
{

}

void MusicKGArtistSimilarRequest::startToSearch(const QString &value)
{
    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(className(), value));

    deleteAll();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KG_ARTIST_SIMILAR_URL, false).arg(value));
    MusicKGInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicKGArtistSimilarRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(className()));

    MusicSimilarRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        const QString html(m_reply->readAll());
        QRegExp regx("class=\"pic\" onclick=.*_src=\"([^\"]+).*href=\"(.*)\">(.*)</a></strong>");
        regx.setMinimal(true);

        int pos = html.indexOf(regx);
        while(pos != -1)
        {
            TTK_NETWORK_QUERY_CHECK();

            MusicResultsItem result;
            QRegExp idrx("/(\\d+)");
            if(regx.cap(2).indexOf(idrx) != -1)
            {
                result.m_id = idrx.cap(1);
            }

            result.m_coverUrl = regx.cap(1);
            result.m_name = regx.cap(3);
            result.m_updateTime.clear();
            Q_EMIT createSimilarItem(result);

            pos += regx.matchedLength();
            pos = regx.indexIn(html, pos);
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

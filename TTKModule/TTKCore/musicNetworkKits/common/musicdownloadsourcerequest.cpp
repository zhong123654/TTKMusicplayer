#include "musicdownloadsourcerequest.h"

MusicDownloadSourceRequest::MusicDownloadSourceRequest(QObject *parent)
    : MusicAbstractNetwork(parent)
{

}

void MusicDownloadSourceRequest::deleteAll()
{
    MusicAbstractNetwork::deleteAll();
    deleteLater();
}

void MusicDownloadSourceRequest::startToDownload(const QString &url)
{
    QNetworkRequest request;
    request.setUrl(url);
    MusicObject::setSslConfiguration(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError);
}

void MusicDownloadSourceRequest::downLoadFinished()
{
    MusicAbstractNetwork::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        const QVariant &redirection = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirection.isValid())
        {
            const QString &url = redirection.toString();
            MusicAbstractNetwork::deleteAll();
            startToDownload(url);
        }
        else
        {
            Q_EMIT downLoadRawDataChanged(m_reply->readAll());
            deleteAll();
        }
    }
    else
    {
        TTK_LOGGER_ERROR("Download source data error");
        Q_EMIT downLoadRawDataChanged(QByteArray());
        deleteAll();
    }
}

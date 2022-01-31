#include "decoderv2mfactory.h"
#include "v2mhelper.h"
#include "decoder_v2m.h"

bool DecoderV2MFactory::canDecode(QIODevice *) const
{
    return false;
}

DecoderProperties DecoderV2MFactory::properties() const
{
    DecoderProperties properties;
    properties.name = "V2M Plugin";
    properties.shortName = "v2m";
    properties.filters << "*.v2m";
    properties.description = "V2 Module Player File";
    properties.protocols << "file";
    properties.noInput = true;
    return properties;
}

Decoder *DecoderV2MFactory::create(const QString &path, QIODevice *input)
{
    Q_UNUSED(input);
    return new DecoderV2M(path);
}

QList<TrackInfo*> DecoderV2MFactory::createPlayList(const QString &path, TrackInfo::Parts parts, QStringList *)
{
    TrackInfo *info = new TrackInfo(path);

    if(parts == TrackInfo::Parts())
    {
        return QList<TrackInfo*>() << info;
    }

    V2MHelper helper(path);
    if(!helper.initialize())
    {
        delete info;
        return QList<TrackInfo*>();
    }

    if(parts & TrackInfo::Properties)
    {
        info->setValue(Qmmp::BITRATE, helper.bitrate());
        info->setValue(Qmmp::SAMPLERATE, helper.sampleRate());
        info->setValue(Qmmp::CHANNELS, helper.channels());
        info->setValue(Qmmp::BITS_PER_SAMPLE, helper.depth());
        info->setValue(Qmmp::FORMAT_NAME, "V2M");
        info->setDuration(helper.totalTime());
    }

    return QList<TrackInfo*>() << info;
}

MetaDataModel* DecoderV2MFactory::createMetaDataModel(const QString &path, bool readOnly)
{
    Q_UNUSED(path);
    Q_UNUSED(readOnly);
    return nullptr;
}

void DecoderV2MFactory::showSettings(QWidget *parent)
{
    Q_UNUSED(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#include <QtPlugin>
Q_EXPORT_PLUGIN2(v2m, DecoderV2MFactory)
#endif

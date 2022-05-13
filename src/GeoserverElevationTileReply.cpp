#include "GeoserverElevationTileReply.h"
#include <QJsonDocument>
#include <QJsonObject>

GeoserverElevationTileReply::GeoserverElevationTileReply(QNetworkAccessManager* networkManager, QNetworkRequest request, double latitude, double longitude, QObject *parent)
    : QObject(parent)
    , _latitude(latitude)
    , _longitude(longitude)
    , _networkManager(networkManager)
{
//    double lat = _latitude;
//    double lon = _longitude;

//    double siny = std::sin((lat * M_PI) / 180);
//    siny = std::min(std::max(siny, -0.9999), 0.9999);

//    unsigned long long x,y;

//    x = 256 * (0.5 + lon / 360);
//    y = 256 * (0.5 - std::log((1+siny) / (1-siny)) / (4 * M_PI));

//    x = x * 2;
//    y = y * 2;

//    x = int(x) % 256;
//    y = int(y) % 256;
    QString url = request.url().toString();

    url += "&BBOX=" +
            QString::number(longitude,'g', 14) +
            "%2C" +
            QString::number(latitude,'g', 14) +
            "%2C" +
            QString::number(longitude + 0.941344916,'g', 14) +
            "%2C" +
            QString::number(latitude + 0.941344916,'g', 14);
    request.setUrl(url);
    _reply = _networkManager->get(request);
    connect(_reply, &QNetworkReply::readyRead, this, &GeoserverElevationTileReply::readyRead);
}

void GeoserverElevationTileReply::readyRead()
{
    if (_reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "GeoserverElevationTileReply error";
        terrainDone(_reply->error());
        return;
    }

    QJsonDocument document = QJsonDocument::fromJson(_reply->readAll());
    QJsonObject object = document.object();
    if (object.value("features")[0].isNull())
        terrainDone(QNetworkReply::ContentNotFoundError);

    _height = object.value("features")[0].toObject().value("properties").toObject().value("GRAY_INDEX").toDouble();

    terrainDone(QNetworkReply::NoError);
}

double GeoserverElevationTileReply::height() const
{
    return _height;
}

double GeoserverElevationTileReply::longitude() const
{
    return _longitude;
}

double GeoserverElevationTileReply::latitude() const
{
    return _latitude;
}

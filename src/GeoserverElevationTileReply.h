#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "QGeoTileFetcherQGC.h"

class GeoserverElevationTileReply : public QObject
{
    Q_OBJECT
public:
    GeoserverElevationTileReply(QNetworkAccessManager* networkManager, QNetworkRequest request, double latitude, double longitude, QObject *parent = nullptr);

    double latitude() const;

    double longitude() const;

    double height() const;

signals:
    void terrainDone(QNetworkReply::NetworkError error = QNetworkReply::NoError);
private slots:
    void readyRead();
private:
    double _latitude;
    double _longitude;
    double _height;
    QNetworkAccessManager*  _networkManager;
    QNetworkReply*          _reply;
};

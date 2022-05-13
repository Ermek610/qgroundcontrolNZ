#pragma once

#include "MapProvider.h"
#include <cmath>

#include <QByteArray>
#include <QMutex>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QPoint>
#include <QString>

static const quint32 AVERAGE_AIRMAP_ELEV_SIZE = 2786;

class ElevationProvider : public MapProvider {
    Q_OBJECT
  public:
    ElevationProvider(const QString& imageFormat, quint32 averageSize,
                      QGeoMapType::MapStyle mapType, QObject* parent = nullptr);

    virtual bool _isElevationProvider() const override { return true; }
};

// -----------------------------------------------------------
// Airmap Elevation

class AirmapElevationProvider : public ElevationProvider {
    Q_OBJECT
  public:
    AirmapElevationProvider(QObject* parent = nullptr)
        : ElevationProvider(QStringLiteral("bin"), AVERAGE_AIRMAP_ELEV_SIZE,
                            QGeoMapType::StreetMap, parent) {}

    int long2tileX(const double lon, const int z) const override;

    int lat2tileY(const double lat, const int z) const override;

    QGCTileSet getTileCount(const int zoom, const double topleftLon,
                            const double topleftLat, const double bottomRightLon,
                            const double bottomRightLat) const override;

  protected:
    QString _getURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) override;
};
// Geoserver Elevation

class GeoserverElevationProvider : public ElevationProvider {
    Q_OBJECT
  public:
    GeoserverElevationProvider(QString geoserverURL, QObject* parent = nullptr);

    enum WMSVersion{
        V1_0_0,
        V1_1_0,
        V1_1_1,
        V1_3_0
    };

    QString wmtsVersionString();
    QString formatURL(QString url);
    QString geoserverURL() const;
    void setGeoserverURL(const QString &geoserverURL);
    QString getWorkspace() const;
    void setWorkspace(const QString &workspace);
    QString getMapName() const;
    void setMapName(const QString &mapName);
    WMSVersion wmtsVersion() const;
    void setWmtsVersion(const WMSVersion &wmsVersion);
    unsigned long long getEPSGNumber() const;
    void setEPSGNumber(unsigned long long EPSGNumber);

    QGCTileSet getTileCount(const int zoom, const double topleftLon,
                            const double topleftLat, const double bottomRightLon,
                            const double bottomRightLat) const override;
    virtual QNetworkRequest getTileURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) override;
  protected:
    QString _getURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) override;
protected:
    static const QString m_serviceType;
    static const QString m_requestType;
    QString m_geoserverURL;
    QString m_workspace;
    QString m_mapName;
    WMSVersion m_wmtsVersion;
    unsigned long long m_EPSGNumber;
    QString m_imageFormat;
};



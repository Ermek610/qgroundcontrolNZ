#include "TerrainTile.h"
#include "ElevationMapProvider.h"
#if defined(DEBUG_GOOGLE_MAPS)
#include <QFile>
#include <QStandardPaths>
#endif
#include "QGCMapEngine.h"
#include "SettingsManager.h"
#include "GeoserverSettings.h"
#include "QGCApplication.h"

#include <QUrlQuery>

ElevationProvider::ElevationProvider(const QString& imageFormat, quint32 averageSize, QGeoMapType::MapStyle mapType, QObject* parent)
    : MapProvider(QStringLiteral("https://api.airmap.com/"), imageFormat, averageSize, mapType, parent) {}

//-----------------------------------------------------------------------------
int AirmapElevationProvider::long2tileX(const double lon, const int z) const {
    Q_UNUSED(z)
    return static_cast<int>(floor((lon + 180.0) / TerrainTile::tileSizeDegrees));
}

//-----------------------------------------------------------------------------
int AirmapElevationProvider::lat2tileY(const double lat, const int z) const {
    Q_UNUSED(z)
    return static_cast<int>(floor((lat + 90.0) / TerrainTile::tileSizeDegrees));
}

QString AirmapElevationProvider::_getURL(const int x, const int y, const int zoom, QNetworkAccessManager* networkManager) {
    Q_UNUSED(networkManager)
    Q_UNUSED(zoom)
    return QString("https://api.airmap.com/elevation/v1/ele/carpet?points=%1,%2,%3,%4")
        .arg(static_cast<double>(y) * TerrainTile::tileSizeDegrees - 90.0)
        .arg(static_cast<double>(x) * TerrainTile::tileSizeDegrees - 180.0)
        .arg(static_cast<double>(y + 1) * TerrainTile::tileSizeDegrees - 90.0)
        .arg(static_cast<double>(x + 1) * TerrainTile::tileSizeDegrees - 180.0);
}

QGCTileSet AirmapElevationProvider::getTileCount(const int zoom, const double topleftLon,
                                                 const double topleftLat, const double bottomRightLon,
                                                 const double bottomRightLat) const {
    QGCTileSet set;
    set.tileX0 = long2tileX(topleftLon, zoom);
    set.tileY0 = lat2tileY(bottomRightLat, zoom);
    set.tileX1 = long2tileX(bottomRightLon, zoom);
    set.tileY1 = lat2tileY(topleftLat, zoom);

    set.tileCount = (static_cast<quint64>(set.tileX1) -
                     static_cast<quint64>(set.tileX0) + 1) *
                    (static_cast<quint64>(set.tileY1) -
                     static_cast<quint64>(set.tileY0) + 1);

    set.tileSize = getAverageSize() * set.tileCount;

    return set;
}
const QString GeoserverElevationProvider::m_serviceType = "WMS";
const QString GeoserverElevationProvider::m_requestType = "GetFeatureInfo";

GeoserverElevationProvider::GeoserverElevationProvider(QString geoserverURL, QObject *parent)
    : ElevationProvider(QStringLiteral("bin"), 512, QGeoMapType::CustomMap, parent)
    , m_geoserverURL(geoserverURL)
    , m_wmtsVersion(V1_0_0)
    , m_EPSGNumber(900913)
    , m_imageFormat("png")
{

}

QString GeoserverElevationProvider::_getURL(const int x, const int y, const int zoom, QNetworkAccessManager *networkManager)
{
    Q_UNUSED(networkManager)
    Q_UNUSED(zoom)

    Q_UNUSED(networkManager)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(zoom)

    GeoserverSettings *geoserverSettings = qgcApp()->toolbox()->settingsManager()->geoserverSettings();

    setWmtsVersion(GeoserverElevationProvider::WMSVersion(geoserverSettings->elevationWMTSVersion()->rawValue().toInt()));
    setWorkspace(geoserverSettings->elevationMapWorkspace()->rawValue().toString());
    setMapName(geoserverSettings->elevationMapName()->rawValue().toString());
    setEPSGNumber(geoserverSettings->elevationEPSGNumber()->rawValue().toString().toInt());

    QString urlString = m_geoserverURL;

    urlString += QString(m_geoserverURL.at(geoserverURL().length()-1) == '/' ? "" : "/")
              + m_workspace + "/wms";

    return urlString;
}

QGCTileSet GeoserverElevationProvider::getTileCount(const int zoom, const double topleftLon, const double topleftLat, const double bottomRightLon, const double bottomRightLat) const
{
    QGCTileSet set;
    set.tileX0 = long2tileX(topleftLon, zoom);
    set.tileY0 = lat2tileY(bottomRightLat, zoom);
    set.tileX1 = long2tileX(bottomRightLon, zoom);
    set.tileY1 = lat2tileY(topleftLat, zoom);

    qDebug() << "Tiles: ";
    qDebug() << set.tileX0;
    qDebug() << set.tileY0;
    qDebug() << set.tileX1;
    qDebug() << set.tileY1;
    qDebug() << "";
    set.tileCount = (static_cast<quint64>(set.tileX1) -
                     static_cast<quint64>(set.tileX0) + 1) *
                    (static_cast<quint64>(set.tileY1) -
                     static_cast<quint64>(set.tileY0) + 1);

    set.tileSize = getAverageSize() * set.tileCount;

    return set;
}

QNetworkRequest GeoserverElevationProvider::getTileURL(const int x, const int y, const int zoom, QNetworkAccessManager *networkManager)
{

    //-- Build URL
    QNetworkRequest request;

    QUrl url = _getURL(x, y, zoom, networkManager);
    QUrlQuery query;

    if (url.isEmpty()) {
        return request;
    }

    //NEED REFACTORING
    query.addQueryItem("SERVICE",       m_serviceType);
    query.addQueryItem("VERSION",       wmtsVersionString());
    query.addQueryItem("REQUEST",       m_requestType);
    query.addQueryItem("INFO_FORMAT",   "application%2Fjson");
    query.addQueryItem("QUERY_LAYERS",  m_workspace+"%3A"+m_mapName);
    query.addQueryItem("LAYERS",        m_workspace+"%3A"+m_mapName);
    query.addQueryItem("FEATURE_COUNT", "50");
    query.addQueryItem("X",             "50");
    query.addQueryItem("Y",             "50");
    query.addQueryItem("WIDTH",         "100");
    query.addQueryItem("HEIGHT",        "100");
    query.addQueryItem("SRS",           "EPSG%3A" + QString::number(m_EPSGNumber));

    url.setQuery(query.query());
    request.setUrl(QUrl(url));

    return request;
}

QString GeoserverElevationProvider::wmtsVersionString()
{
    switch (m_wmtsVersion) {
    case V1_0_0: return "1.0.0";
    case V1_1_0: return "1.1.0";
    case V1_1_1: return "1.1.1";
    case V1_3_0: return "1.3.0";
    }
    return "1.0.0";
}

QString GeoserverElevationProvider::getMapName() const
{
    return m_mapName;
}

void GeoserverElevationProvider::setMapName(const QString &mapName)
{
    m_mapName = mapName;
}

QString GeoserverElevationProvider::getWorkspace() const
{
    return m_workspace;
}

void GeoserverElevationProvider::setWorkspace(const QString &workspace)
{
    m_workspace = workspace;
}

QString GeoserverElevationProvider::geoserverURL() const
{
    return m_geoserverURL;
}

void GeoserverElevationProvider::setGeoserverURL(const QString &geoserverURL)
{
    m_geoserverURL = geoserverURL;
}

GeoserverElevationProvider::WMSVersion GeoserverElevationProvider::wmtsVersion() const
{
    return m_wmtsVersion;
}

void GeoserverElevationProvider::setWmtsVersion(const WMSVersion &wmsVersion)
{
    m_wmtsVersion = wmsVersion;
}

unsigned long long GeoserverElevationProvider::getEPSGNumber() const
{
    return m_EPSGNumber;
}

void GeoserverElevationProvider::setEPSGNumber(unsigned long long EPSGNumber)
{
    m_EPSGNumber = EPSGNumber;
}


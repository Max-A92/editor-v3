#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QDateTime>

struct SessionRecord {
    int id = 0;
    QDateTime startTime;
    double durationSeconds = 0.0;
    int totalKeystrokes = 0;
    double grossWPM = 0.0;
    double netWPM = 0.0;
    double accuracy = 0.0;
    double kspc = 0.0;
    int backspaceCount = 0;
    double motoricRate = 0.0;
    double automatizationRate = 0.0;
    double contentRate = 0.0;
};

struct TrendInfo {
    double avgWPM = 0.0;
    double avgAccuracy = 0.0;
    double avgKSPC = 0.0;
    double wpmTrend = 0.0;      // positiv = Verbesserung
    double accuracyTrend = 0.0;
    double kspcTrend = 0.0;     // negativ = Verbesserung (weniger Korrekturen)
    int sessionCount = 0;
    QString diagnosis;
    QString recommendations;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initialize();
    bool saveSession(const SessionRecord &record);
    QList<SessionRecord> getAllSessions() const;
    QList<SessionRecord> getRecentSessions(int count = 10) const;
    TrendInfo analyzeTrends() const;

    static QString databasePath();

private:
    void createTables();
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H

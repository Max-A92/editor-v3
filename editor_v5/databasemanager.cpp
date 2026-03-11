#include "databasemanager.h"

#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDebug>
#include <QtMath>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

QString DatabaseManager::databasePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/sessions.db";
}

bool DatabaseManager::initialize()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(databasePath());

    if (!m_db.open()) {
        qWarning() << "Database open failed:" << m_db.lastError().text();
        return false;
    }

    createTables();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlQuery q(m_db);
    q.exec(
        "CREATE TABLE IF NOT EXISTS sessions ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   start_time TEXT NOT NULL,"
        "   duration_seconds REAL,"
        "   total_keystrokes INTEGER,"
        "   gross_wpm REAL,"
        "   net_wpm REAL,"
        "   accuracy REAL,"
        "   kspc REAL,"
        "   backspace_count INTEGER,"
        "   motoric_rate REAL,"
        "   automatization_rate REAL,"
        "   content_rate REAL"
        ")"
    );
}

bool DatabaseManager::saveSession(const SessionRecord &rec)
{
    QSqlQuery q(m_db);
    q.prepare(
        "INSERT INTO sessions "
        "(start_time, duration_seconds, total_keystrokes, gross_wpm, net_wpm, "
        " accuracy, kspc, backspace_count, motoric_rate, automatization_rate, content_rate) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    q.addBindValue(rec.startTime.toString(Qt::ISODate));
    q.addBindValue(rec.durationSeconds);
    q.addBindValue(rec.totalKeystrokes);
    q.addBindValue(rec.grossWPM);
    q.addBindValue(rec.netWPM);
    q.addBindValue(rec.accuracy);
    q.addBindValue(rec.kspc);
    q.addBindValue(rec.backspaceCount);
    q.addBindValue(rec.motoricRate);
    q.addBindValue(rec.automatizationRate);
    q.addBindValue(rec.contentRate);

    if (!q.exec()) {
        qWarning() << "Insert failed:" << q.lastError().text();
        return false;
    }
    return true;
}

QList<SessionRecord> DatabaseManager::getAllSessions() const
{
    QList<SessionRecord> list;
    QSqlQuery q("SELECT * FROM sessions ORDER BY start_time ASC", m_db);
    while (q.next()) {
        SessionRecord r;
        r.id = q.value("id").toInt();
        r.startTime = QDateTime::fromString(q.value("start_time").toString(), Qt::ISODate);
        r.durationSeconds = q.value("duration_seconds").toDouble();
        r.totalKeystrokes = q.value("total_keystrokes").toInt();
        r.grossWPM = q.value("gross_wpm").toDouble();
        r.netWPM = q.value("net_wpm").toDouble();
        r.accuracy = q.value("accuracy").toDouble();
        r.kspc = q.value("kspc").toDouble();
        r.backspaceCount = q.value("backspace_count").toInt();
        r.motoricRate = q.value("motoric_rate").toDouble();
        r.automatizationRate = q.value("automatization_rate").toDouble();
        r.contentRate = q.value("content_rate").toDouble();
        list.append(r);
    }
    return list;
}

QList<SessionRecord> DatabaseManager::getRecentSessions(int count) const
{
    QList<SessionRecord> list;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM sessions ORDER BY start_time DESC LIMIT ?");
    q.addBindValue(count);
    q.exec();
    while (q.next()) {
        SessionRecord r;
        r.id = q.value("id").toInt();
        r.startTime = QDateTime::fromString(q.value("start_time").toString(), Qt::ISODate);
        r.durationSeconds = q.value("duration_seconds").toDouble();
        r.totalKeystrokes = q.value("total_keystrokes").toInt();
        r.grossWPM = q.value("gross_wpm").toDouble();
        r.netWPM = q.value("net_wpm").toDouble();
        r.accuracy = q.value("accuracy").toDouble();
        r.kspc = q.value("kspc").toDouble();
        r.backspaceCount = q.value("backspace_count").toInt();
        r.motoricRate = q.value("motoric_rate").toDouble();
        r.automatizationRate = q.value("automatization_rate").toDouble();
        r.contentRate = q.value("content_rate").toDouble();
        list.append(r);
    }
    // Reverse to chronological order
    std::reverse(list.begin(), list.end());
    return list;
}

TrendInfo DatabaseManager::analyzeTrends() const
{
    TrendInfo info;
    auto sessions = getRecentSessions(20);
    info.sessionCount = sessions.size();

    if (sessions.isEmpty()) {
        info.diagnosis = "Noch keine Sessions gespeichert.";
        info.recommendations = "Starte eine Tipp-Session und speichere sie, um Trends zu sehen.";
        return info;
    }

    // Durchschnitte berechnen
    double sumWPM = 0, sumAcc = 0, sumKSPC = 0;
    for (const auto &s : sessions) {
        sumWPM += s.grossWPM;
        sumAcc += s.accuracy;
        sumKSPC += s.kspc;
    }
    info.avgWPM = sumWPM / sessions.size();
    info.avgAccuracy = sumAcc / sessions.size();
    info.avgKSPC = sumKSPC / sessions.size();

    // Trend berechnen (letzte 5 vs. erste 5, oder Hälfte vs. Hälfte)
    if (sessions.size() >= 4) {
        int half = sessions.size() / 2;

        double earlyWPM = 0, lateWPM = 0;
        double earlyAcc = 0, lateAcc = 0;
        double earlyKSPC = 0, lateKSPC = 0;

        for (int i = 0; i < half; ++i) {
            earlyWPM += sessions[i].grossWPM;
            earlyAcc += sessions[i].accuracy;
            earlyKSPC += sessions[i].kspc;
        }
        earlyWPM /= half; earlyAcc /= half; earlyKSPC /= half;

        for (int i = half; i < sessions.size(); ++i) {
            lateWPM += sessions[i].grossWPM;
            lateAcc += sessions[i].accuracy;
            lateKSPC += sessions[i].kspc;
        }
        int lateCount = sessions.size() - half;
        lateWPM /= lateCount; lateAcc /= lateCount; lateKSPC /= lateCount;

        info.wpmTrend = lateWPM - earlyWPM;
        info.accuracyTrend = lateAcc - earlyAcc;
        info.kspcTrend = lateKSPC - earlyKSPC;
    }

    // ═══ Regelbasiertes Diagnosesystem ═══

    QString diag;
    QString recs;

    // 1. Geschwindigkeits-Diagnose
    if (info.avgWPM >= 60) {
        diag += "Geschwindigkeit: SEHR GUT (" + QString::number(info.avgWPM, 'f', 1) + " WPM)\n";
    } else if (info.avgWPM >= 40) {
        diag += "Geschwindigkeit: GUT (" + QString::number(info.avgWPM, 'f', 1) + " WPM)\n";
    } else if (info.avgWPM >= 25) {
        diag += "Geschwindigkeit: AUSBAUFÄHIG (" + QString::number(info.avgWPM, 'f', 1) + " WPM)\n";
        recs += "- Tägliches Schnelligkeitstraining (15 Min): Häufige Wörter repetitiv tippen\n";
    } else {
        diag += "Geschwindigkeit: ANFÄNGER (" + QString::number(info.avgWPM, 'f', 1) + " WPM)\n";
        recs += "- Grundlagentraining: 10-Finger-System mit TIPP10 üben (30 Min/Tag)\n";
    }

    // 2. Genauigkeits-Diagnose
    double accPct = info.avgAccuracy * 100.0;
    if (accPct >= 95) {
        diag += "Genauigkeit: EXZELLENT (" + QString::number(accPct, 'f', 1) + "%)\n";
    } else if (accPct >= 90) {
        diag += "Genauigkeit: GUT (" + QString::number(accPct, 'f', 1) + "%)\n";
    } else if (accPct >= 80) {
        diag += "Genauigkeit: MANGELHAFT (" + QString::number(accPct, 'f', 1) + "%)\n";
        recs += "- Tempo drosseln und Genauigkeit priorisieren\n";
        recs += "- Lieber 35 WPM mit 95% als 50 WPM mit 85%\n";
    } else {
        diag += "Genauigkeit: KRITISCH (" + QString::number(accPct, 'f', 1) + "%)\n";
        recs += "- SOFORT Tempo reduzieren: Jeder Fehler kostet mehr Zeit als langsames Tippen\n";
    }

    // 3. Effizienz-Diagnose
    if (info.avgKSPC <= 1.1) {
        diag += "Effizienz: SEHR GUT (KSPC " + QString::number(info.avgKSPC, 'f', 2) + ")\n";
    } else if (info.avgKSPC <= 1.3) {
        diag += "Effizienz: NORMAL (KSPC " + QString::number(info.avgKSPC, 'f', 2) + ")\n";
    } else {
        diag += "Effizienz: SCHLECHT (KSPC " + QString::number(info.avgKSPC, 'f', 2) + ")\n";
        recs += "- Zu viele Korrekturen: Konzentriere dich auf sauberes Tippen statt Nachkorrigieren\n";
    }

    // 4. Trend-Diagnose (nur wenn genug Sessions)
    if (sessions.size() >= 4) {
        diag += "\n--- TREND (letzte " + QString::number(sessions.size()) + " Sessions) ---\n";

        if (info.wpmTrend > 2.0)
            diag += "WPM-Trend: STEIGEND (+" + QString::number(info.wpmTrend, 'f', 1) + " WPM) - Gut!\n";
        else if (info.wpmTrend < -2.0) {
            diag += "WPM-Trend: FALLEND (" + QString::number(info.wpmTrend, 'f', 1) + " WPM) - Achtung!\n";
            recs += "- Deine Geschwindigkeit sinkt: Mache Pausen, Übermüdung vermeiden\n";
        } else
            diag += "WPM-Trend: STABIL\n";

        if (info.accuracyTrend > 0.02)
            diag += "Accuracy-Trend: STEIGEND - Gut!\n";
        else if (info.accuracyTrend < -0.02) {
            diag += "Accuracy-Trend: FALLEND - Achtung!\n";
            recs += "- Deine Genauigkeit verschlechtert sich: Tempo zurücknehmen\n";
        } else
            diag += "Accuracy-Trend: STABIL\n";
    }

    // 5. Chronische Probleme (gleicher Fehlertypus über viele Sessions)
    if (sessions.size() >= 3) {
        int motoricDominant = 0, autoDominant = 0, contentDominant = 0;
        for (const auto &s : sessions) {
            if (s.motoricRate > s.automatizationRate && s.motoricRate > s.contentRate) motoricDominant++;
            else if (s.automatizationRate > s.motoricRate && s.automatizationRate > s.contentRate) autoDominant++;
            else contentDominant++;
        }

        if (autoDominant > sessions.size() * 0.6) {
            diag += "\nCHRONISCH: Tastsuche-Probleme in " + QString::number(autoDominant)
                    + " von " + QString::number(sessions.size()) + " Sessions\n";
            recs += "- PRIORITÄT: 10-Finger-Grundkurs, tägliche Grundreihen-Übungen\n";
        }
        if (motoricDominant > sessions.size() * 0.6) {
            diag += "\nCHRONISCH: Motorische Fehler in " + QString::number(motoricDominant)
                    + " von " + QString::number(sessions.size()) + " Sessions\n";
            recs += "- PRIORITÄT: Langsamer tippen, Finger-Koordination verbessern\n";
        }
    }

    if (recs.isEmpty())
        recs = "Keine spezifischen Empfehlungen - weiter so!";

    info.diagnosis = diag;
    info.recommendations = recs;
    return info;
}

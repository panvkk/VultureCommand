#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <QObject>
#include <QMap>
#include "winnerReport.h"

class StatisticsManager : public QObject
{
    Q_OBJECT
public:
    static StatisticsManager& instance();

    void updateStatistics(const WinnerReport& report);
    void saveAllStatistics() const;

private:
    StatisticsManager(QObject *parent = nullptr);
    QMap<QString, int> m_participantWins; // ФИО -> количество побед
    QMap<QString, int> m_rhymeUsage; // Считалка -> количество использований
    QList<QPair<QString, qint64>> m_executionTimes; // ФИО -> время выполнения (в секундах)

    void loadStatistics();
    void saveParticipantsRating() const;
    void saveRhymesRating() const;
    void saveExecutionTimes() const;
};

#endif // STATISTICSMANAGER_H

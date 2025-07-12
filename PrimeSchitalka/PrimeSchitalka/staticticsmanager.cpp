#include "statisticsmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>

StatisticsManager& StatisticsManager::instance()
{
    static StatisticsManager instance;
    return instance;
}

// Конструктор
StatisticsManager::StatisticsManager(QObject *parent)
    : QObject(parent)
{
    loadStatistics();
}

// Сохранение рейтинга участников
void StatisticsManager::saveParticipantsRating() const
{
    QFile file("participants_rating.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Рейтинг участников:\n";
        out << "ФИО\tКоличество побед\n";

        // Сортировка по количеству побед (по убыванию)
        QList<QPair<QString, int>> sortedList;
        for (auto it = m_participantWins.begin(); it != m_participantWins.end(); ++it) {
            sortedList.append(qMakePair(it.key(), it.value()));
        }

        std::sort(sortedList.begin(), sortedList.end(),
                  [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                      return a.second > b.second;
                  });

        // Запись в файл
        for (const auto& item : sortedList) {
            out << item.first << "\t" << item.second << "\n";
        }
    }
}

void StatisticsManager::loadStatistics()
{
    QFile file("statistics.dat");
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_0);

        while (!in.atEnd()) {
            WinnerReport report;
            in.readRawData(reinterpret_cast<char*>(&report), sizeof(WinnerReport));

            QString winnerName = QString::fromLocal8Bit(report.winnerName, 20).trimmed();
            QString rhymeText = QString::fromLocal8Bit(report.rhymeText, 240).trimmed();
            qint64 execTime = report.hours * 3600 + report.minutes * 60 + report.seconds;

            m_participantWins[winnerName]++;
            m_rhymeUsage[rhymeText]++;
            m_executionTimes.append(qMakePair(winnerName, execTime));
        }
    }
}

void StatisticsManager::updateStatistics(const WinnerReport& report)
{
    QString winnerName = QString::fromLocal8Bit(report.winnerName, 20).trimmed();
    QString rhymeText = QString::fromLocal8Bit(report.rhymeText, 240).trimmed();
    qint64 execTime = report.hours * 3600 + report.minutes * 60 + report.seconds;

    m_participantWins[winnerName]++;
    m_rhymeUsage[rhymeText]++;
    m_executionTimes.append(qMakePair(winnerName, execTime));

    saveAllStatistics();
}

void StatisticsManager::saveAllStatistics() const
{
    saveParticipantsRating();
    saveRhymesRating();
    saveExecutionTimes();
}

void StatisticsManager::saveRhymesRating() const
{
    QFile file("rhymes_rating.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Рейтинг считалок:\n";
        out << "Считалка\tКоличество использований\n";

        QList<QPair<QString, int>> sorted;
        for (auto it = m_rhymeUsage.begin(); it != m_rhymeUsage.end(); ++it) {
            sorted.append(qMakePair(it.key(), it.value()));
        }

        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        for (const auto& item : sorted) {
            out << item.first << "\t" << item.second << "\n";
        }
    }
}

void StatisticsManager::saveExecutionTimes() const
{
    QFile file("execution_times.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Время выполнения:\n";
        out << "ФИО\tКол-во участников\tСчиталка\tВремя (сек)\n";

        // Создаем копию для сортировки
        auto sortedTimes = m_executionTimes;

        // Сортируем копию
        std::sort(sortedTimes.begin(), sortedTimes.end(),
                  [](const auto& a, const auto& b) {
                      return a.second > b.second; // Сортировка по убыванию времени
                  });

        // Выводим отсортированные данные
        for (const auto& item : sortedTimes) {
            out << item.first << "\t"
                << /* количество участников */ "\t"
                << /* текст считалки */ "\t"
                << item.second << "\n";
        }
    }
}

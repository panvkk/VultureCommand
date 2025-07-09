#ifndef WINNERREPORT_H
#define WINNERREPORT_H

#include <QString>
#include <QFile>

class WinnerReport {
private:
    // Дата: день - 1 байт, месяц - 1 байт, год - 2 байта
    char day;       // 1 байт
    char month;     // 1 байт
    short year;     // 2 байта (short)

    // Время выполнения: часы - 1 байт, минуты - 1 байт, секунды - 1 байт
    char hours;     // 1 байт
    char minutes;   // 1 байт
    char seconds;   // 1 байт

    // Количество участников - 2 байта
    short participantCount; // 2 байта (short)

    // Случайное число (начальный участник) - 1 байт
    char startingParticipant; // 1 байт

    // Номер победителя - 1 байт
    char winnerNumber; // 1 байт

    // Текст считалки - 240 байтов
    char rhymeText[240]; // 240 байт (char[])

    // ФИО победителя - 20 байтов
    char winnerName[20]; // 20 байт (char[])

public:
    // === Геттеры ===
    int getDay() const;
    int getMonth() const;
    int getYear() const;

    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;

    int getParticipantCount() const;
    int getStartingParticipant() const;
    int getWinnerNumber() const;

    QString getRhymeText() const;
    QString getWinnerName() const;

    // === Метод записи в бинарный файл (Qt) ===
    void saveToBinaryFile(const QString& filename) const;
    // === Метод чтения из бинарного файла (опционально) ===
    void loadFromBinaryFile(const QString& filename);
};

#endif // WINNERREPORT_H

#include "report.h"

int Report::getDay() const { return static_cast<unsigned char>(day); }
int Report::getMonth() const { return static_cast<unsigned char>(month); }
int Report::getYear() const { return static_cast<unsigned short>(year); }
int Report::getCommand() const { return static_cast<unsigned char>(command); }
int Report::getParticipantCount() const { return static_cast<unsigned short>(participantCount); }
int Report::getStartingParticipant() const { return static_cast<unsigned char>(startingParticipant); }
int Report::getWinnerNumber() const { return static_cast<unsigned char>(winnerNumber); }
QString Report::getRhymeText() const { return QString::fromLocal8Bit(rhymeText, 240).trimmed(); }
QString Report::getWinnerName() const { return QString::fromLocal8Bit(winnerName, 20).trimmed(); }

// === Метод записи в бинарный файл (Qt) ===
void Report::saveToBinaryFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0); // Указываем версию для совместимости
    out.writeRawData(reinterpret_cast<const char*>(this), sizeof(Report));

    file.close();
}

// === Метод чтения из бинарного файла (опционально) ===
void Report::loadFromBinaryFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);
    in.readRawData(reinterpret_cast<char*>(this), sizeof(Report));

    file.close();
}

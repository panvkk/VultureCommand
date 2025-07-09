#include "winnerReport.h"

int WinnerReport::getDay() const { return static_cast<unsigned char>(day); }
int WinnerReport::getMonth() const { return static_cast<unsigned char>(month); }
int WinnerReport::getYear() const { return static_cast<unsigned short>(year); }

int WinnerReport::getHours() const { return static_cast<unsigned char>(hours); }
int WinnerReport::getMinutes() const { return static_cast<unsigned char>(minutes); }
int WinnerReport::getSeconds() const { return static_cast<unsigned char>(seconds); }

int WinnerReport::getStartingParticipant() const { return static_cast<unsigned char>(startingParticipant); }
int WinnerReport::getWinnerNumber() const { return static_cast<unsigned char>(winnerNumber); }

QString WinnerReport::getRhymeText() const { return QString::fromLocal8Bit(rhymeText, 240).trimmed(); }
QString WinnerReport::getWinnerName() const { return QString::fromLocal8Bit(winnerName, 20).trimmed(); }

// === Метод записи в бинарный файл (Qt) ===
void WinnerReport::saveToBinaryFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0); // Указываем версию для совместимости
    out.writeRawData(reinterpret_cast<const char*>(this), sizeof(WinnerReport));

    file.close();
}

// === Метод чтения из бинарного файла (опционально) ===
void WinnerReport::loadFromBinaryFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);
    in.readRawData(reinterpret_cast<char*>(this), sizeof(WinnerReport));

    file.close();
}

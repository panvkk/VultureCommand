#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QVector>
#include <QPushButton>
#include <QFile>
#include <QTextStream>

struct SessionStats {
    QTime startTime;
    QTime endTime;
    int durationSeconds;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateClock();
    void openClock();
    void showStatistics();
    void showHelp();
    void showNumericTimeDialog();
    void showVerbalTimeDialog();
    void setCustomTime(const QTime &time);

private:
    QString formatDuration(int seconds);
    void createMenus();
    QTime parseVerbalTime(const QString &verbalTime);
    void saveSessionStats(const SessionStats &stats);
    QVector<SessionStats> loadAllSessionStats();

    QPixmap background;
    QTimer *timer;
    bool showSplash;
    QPushButton *openButton;
    QWidget *splashWidget;
    QVector<QPixmap> hourHands;
    QVector<QPixmap> minuteHands;
    QTime customTime;
    bool useCustomTime = false;
    QTime sessionStartTime;
    void paintEvent(QPaintEvent *event);
    const QString STATS_FILE_NAME = "app_stats.txt";
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QSoundEffect>
#include <QPixmap>
#include <QMessageBox>
#include <QMediaPlayer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showClock(); // Function to show the clock after splash screen

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateClock();
    void playHourChime(int hour);
    void openClock();
    void showStatistics();
    void showHelp();

    // Background themes
    void setColdTheme();
    void setPeachGloomTheme();
    void setBelarusTheme();
    void setCloudyTheme();
    void setPeachGrapeTheme();
    void setSunnyGladeTheme();
    void setFatePredictionTheme();
    void setSeasonMixTheme();

    // Hand styles
    void setClassicHands();
    void setModernHands();
    void setFantasyHands();

    // Music
    void playMusicTrack1();
    void playMusicTrack2();
    void playMusicTrack3();
    void handleMusicStateChanged(QMediaPlayer::PlaybackState state);

    // Time format
    void setNumericFormat();
    void setTimerInput();
    void setManualInput();
    void setVerbalFormat();

    //Tests
    void saveTestSessions();

    void showNumericTimeDialog();
    void showTimerInputDialog();
    void setCustomTime(const QTime &time);

private:
    int totalSessions = 0;
    int totalDuration = 0;
    QTime startTime;
    QVector<QPair<QTime, QTime>> testSessions; // Пары (время запуска, время завершения)
    void createMenus();
    void drawClockFace(QPainter &painter);
    void drawHand(QPainter &painter, double angle, int length, int width, QColor color);
    void drawCuckoo(QPainter &painter, int secondsVisible);
    void drawSplashScreen(QPainter &painter);

    QPixmap background;
    QTimer *timer;
    int lastSecond;
    bool isTick;
    bool showSplash;

    // Sound effects
    QSoundEffect tickSound;
    QSoundEffect tockSound;
    QSoundEffect boomSound;
    QSoundEffect cuckooSound;
    QMediaPlayer *backgroundMusic;

    QMediaPlayer *musicPlayer;
    QAudioOutput *audioOutput;
    QString currentTrack;

    int lastChimedHour;
    bool cuckooPlayed;
    bool hourChimePlayed;

    // Current style settings
    QString currentHandStyle;
    QString currentTimeFormat;

    QTime customTime;
    QTime timeOffset;
    bool useCustomTime = false;
};
#endif // MAINWINDOW_H

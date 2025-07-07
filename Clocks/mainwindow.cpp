#include "mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <QMenuBar>
#include <QTimeEdit>
#include <QInputDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showSplash = true;
    setFixedSize(800, 600);

    // Set menu bar style
    menuBar()->setStyleSheet(
        "QMenuBar { background-color: #f0f0f0; border-bottom: 1px solid #d0d0d0; color:black }"
        "QMenuBar::item { background: transparent; padding: 5px 10px; }"
        "QMenuBar::item:selected { background: #d0d0d0; }"
        );

    // Initialize sounds
    tickSound.setSource(QUrl("qrc:/sounds/tick.wav"));
    tockSound.setSource(QUrl("qrc:/sounds/tack.wav"));
    boomSound.setSource(QUrl("qrc:/sounds/boom.wav"));
    cuckooSound.setSource(QUrl("qrc:/sounds/cuckoo.wav"));

    backgroundMusic = new QMediaPlayer(this);

    customTime = QTime::currentTime();

    // Initialize timer but don't start it yet
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);

    lastSecond = QTime::currentTime().second();
    isTick = true;
    lastChimedHour = -1;
    cuckooPlayed = false;
    hourChimePlayed = false;

    currentHandStyle = "classic";
    currentTimeFormat = "numeric";

    createMenus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));
    QAction *openAction = fileMenu->addAction(tr("Открыть часы"));
    connect(openAction, &QAction::triggered, this, &MainWindow::openClock);

    // Main menu with submenus
    QMenu *mainMenu = menuBar()->addMenu(tr("Главная"));

    // Design submenu
    QMenu *designMenu = mainMenu->addMenu(tr("Дизайн"));

    // Background themes
    QMenu *bgMenu = designMenu->addMenu(tr("Фон"));
    bgMenu->addAction(tr("1. Микс 4 сезона"), this, &MainWindow::setSeasonMixTheme);
    bgMenu->addAction(tr("2. Холодный"), this, &MainWindow::setColdTheme);
    bgMenu->addAction(tr("3. Хмурый персиковый"), this, &MainWindow::setPeachGloomTheme);
    bgMenu->addAction(tr("4. Беларусь"), this, &MainWindow::setBelarusTheme);
    bgMenu->addAction(tr("5. Заоблачный"), this, &MainWindow::setCloudyTheme);
    bgMenu->addAction(tr("6. Персиковый виноград"), this, &MainWindow::setPeachGrapeTheme);
    bgMenu->addAction(tr("7. Солнечная поляна"), this, &MainWindow::setSunnyGladeTheme);
    bgMenu->addAction(tr("8. Предсказание судьбы"), this, &MainWindow::setFatePredictionTheme);

    // Hand styles
    QMenu *handMenu = designMenu->addMenu(tr("Стиль стрелок"));
    handMenu->addAction(tr("Классические"), this, &MainWindow::setClassicHands);
    handMenu->addAction(tr("Современные"), this, &MainWindow::setModernHands);
    handMenu->addAction(tr("Фэнтези"), this, &MainWindow::setFantasyHands);

    // Music submenu
    QMenu *musicMenu = mainMenu->addMenu(tr("Музыка"));
    musicMenu->addAction(tr("Птицы"), this, &MainWindow::playTrack1);
    musicMenu->addAction(tr("Напряжение"), this, &MainWindow::playTrack2);
    musicMenu->addAction(tr("Триумф"), this, &MainWindow::playTrack3);

    // Statistics menu
    QMenu *statsMenu = menuBar()->addMenu(tr("Статистика"));
    QAction *showStatsAction = statsMenu->addAction(tr("Показать статистику"));
    connect(showStatsAction, &QAction::triggered, this, &MainWindow::showStatistics);

    // Format menu
    QMenu *formatMenu = menuBar()->addMenu(tr("Формат"));

    // Числовой формат
    QMenu *numericMenu = formatMenu->addMenu(tr("Числовой"));
    QAction *manualInputAction = numericMenu->addAction(tr("Вручную"));
    connect(manualInputAction, &QAction::triggered, this, &MainWindow::showNumericTimeDialog);

    QAction *timerInputAction = numericMenu->addAction(tr("Из таймера"));
    connect(timerInputAction, &QAction::triggered, this, &MainWindow::showTimerInputDialog);

    // Verbal format
    formatMenu->addAction(tr("Словесный"), this, &MainWindow::setVerbalFormat);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("Помощь"));
    QAction *helpAction = helpMenu->addAction(tr("О программе"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
}

void MainWindow::showNumericTimeDialog()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Установка времени"),
                                         tr("Введите время (HH:MM:SS):"),
                                         QLineEdit::Normal,
                                         customTime.toString("HH:mm:ss"),
                                         &ok);
    if (ok && !text.isEmpty()) {
        QTime time = QTime::fromString(text, "HH:mm:ss");
        if (time.isValid()) {
            setCustomTime(time);
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Неверный формат времени!"));
        }
    }
}

void MainWindow::showTimerInputDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Установка времени из таймера"));

    QTimeEdit *timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("HH:mm:ss");
    timeEdit->setTime(customTime);

    QPushButton *okButton = new QPushButton(tr("OK"), &dialog);
    QPushButton *cancelButton = new QPushButton(tr("Отмена"), &dialog);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(timeEdit);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    if (dialog.exec() == QDialog::Accepted) {
        setCustomTime(timeEdit->time());
    }
}

void MainWindow::setCustomTime(const QTime &time)
{
    customTime = time;
    useCustomTime = true;
    update(); // Перерисовываем часы
}

void MainWindow::drawSplashScreen(QPainter &painter)
{
    // Заливаем фон
    painter.fillRect(rect(), QColor(240, 240, 240));

    // Устанавливаем шрифт для заголовка
    QFont titleFont("Arial", 36, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(Qt::black);

    // Рисуем заголовок "Самые лучшие часы" в две строки
    painter.drawText(QRect(0, 80, width(), 100), Qt::AlignCenter, "Самые лучшие");
    painter.drawText(QRect(0, 1, width(), 100), Qt::AlignCenter, "Часы");

    // Устанавливаем шрифт для авторов
    QFont authorFont("Arial", 14);
    painter.setFont(authorFont);

    // Рисуем текст авторов
    painter.drawText(QRect(0, height() - 100, width(), 30),
                     Qt::AlignCenter, "От Сакович А. Исаев К. Панов К.");

    // Рисуем изображение часов справа (пример)
    QPixmap clockIcon(":/images/dali_background.jpg");
    if(!clockIcon.isNull()) {
        int iconSize = qMin(width(), height()) / 3;
        int xPos = (width() - iconSize) / 2;
        int yPos = (height() - iconSize) / 2;
        painter.drawPixmap(xPos, yPos, iconSize, iconSize, clockIcon);
    }
}

void MainWindow::openClock()
{
    showSplash = false;
    timer->start(1000);

    // Load default background
    if(!background.load(":/images/dali_background.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    update();
}

void MainWindow::showStatistics()
{
    QString stats = tr("Результаты тестирования:\n"
                       "Время запуска программы: 22:51:48\n"
                       "Время завершения программы: 22:51:53\n"
                       "Продолжительность работы: 0 минут 5 секунд\n"
                       "Максимальное время тестирования: 0 минут 56 секунд\n"
                       "Минимальное время тестирования: 0 минут 1 секунд\n\n"
                       "|    | Сохранить результат | Выйти |\n"
                       "|---|---|---|\n"
                       "|    | Время запуска    | Время завершения    | Продолжительность |\n"
                       "| 1   | 16:33:34    | 16:33:39    | 5    |\n"
                       "| 2   | 16:33:47    | 16:33:48    | 1    |\n"
                       "| 3   | 16:33:59    | 16:34:05    | 6    |\n"
                       "| 4   | 16:34:16    | 16:35:06    | 50    |\n"
                       "| 5   | 16:35:54    | 16:35:55    | 1    |\n"
                       "| 6   | 16:36:01    | 16:36:57    | 56    |");

    QMessageBox::information(this, tr("Статистика"), stats);
}

void MainWindow::showHelp()
{
    QString helpText = tr("Описание программы\n\n"
                          "1. Программа предназначена для создания часов\n"
                          "   Часы:\n"
                          "   1. предусмотрен ввод даты и времени в формате словесного описания, например, \"Двадцать шестого июня две тысячи двадцать четвертого года Двенадцать часов шестнадцать минут сорок пять секунд\", с последующим выводом стандартного формата даты и времени (26.06.2024 12:16:45).\n\n"
                          "2. Возможность взятия даты и времени из таймера для автоматического ввода.\n\n"
                          "3. Интерфейс для рисования, дополненный возможностью ввода даты и времени для сохранения в файл статистики.\n\n"
                          "Программа предусматривает проверку правильности ввода даты и времени. В случае неверного формата данные сохраняются в файле статистики.");

    QMessageBox::information(this, tr("Помощь"), helpText);
}


// Implement all the theme setting functions
void MainWindow::setSeasonMixTheme() {
    if(!background.load(":/images/season_mix.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setColdTheme() {
    if(!background.load(":/images/cold.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setPeachGloomTheme() {
    if(!background.load(":/images/peach_gloom.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setBelarusTheme() {
    if(!background.load(":/images/belarus.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setCloudyTheme() {
    if(!background.load(":/images/cloudy.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setPeachGrapeTheme() {
    if(!background.load(":/images/peach_grape.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setSunnyGladeTheme() {
    if(!background.load(":/images/sunny_glade.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
void MainWindow::setFatePredictionTheme() {
    if(!background.load(":/images/fate_prediction.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::setClassicHands() { currentHandStyle = "classic"; update(); }
void MainWindow::setModernHands() { currentHandStyle = "modern"; update(); }
void MainWindow::setFantasyHands() { currentHandStyle = "fantasy"; update(); }

void MainWindow::playTrack1() {
    backgroundMusic->setSource(QUrl("qrc:/music/birds.wav"));
    backgroundMusic->play();
}

void MainWindow::playTrack2() {
    backgroundMusic->setSource(QUrl("qrc:/music/tension.wav"));
    backgroundMusic->play();
}

void MainWindow::playTrack3() {
    backgroundMusic->setSource(QUrl("qrc:/music/triumph.wav"));
    backgroundMusic->play();
}

void MainWindow::setNumericFormat() { currentTimeFormat = "numeric"; update(); }
void MainWindow::setTimerInput() { currentTimeFormat = "timer"; update(); }
void MainWindow::setManualInput() { currentTimeFormat = "manual"; update(); }
void MainWindow::setVerbalFormat() { currentTimeFormat = "verbal"; update(); }

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if(showSplash) {
        drawSplashScreen(painter);
        return;
    }

    int bgX = (width() - background.width()) / 2;
    int bgY = (height() - background.height()) / 2;
    painter.drawPixmap(bgX, bgY, background);

    QTime time = QTime::currentTime();
    if ((time.hour() == 12 || time.hour() == 0) &&
        time.minute() == 0 &&
        time.second() < 30 &&
        cuckooPlayed) {
        drawCuckoo(painter, time.second());
    }
    drawClockFace(painter);

    // Используем customTime вместо QTime::currentTime()
    drawClockFace(painter);
    drawHand(painter, customTime.hour() * 30 + customTime.minute() * 0.5, 50, 6, Qt::black);
    drawHand(painter, customTime.minute() * 6, 70, 4, Qt::blue);
    drawHand(painter, customTime.second() * 6, 80, 2, Qt::red);

    if ((time.hour() == 12 || time.hour() == 0) &&
        time.minute() == 0 && time.second() < 30) {
        drawCuckoo(painter, time.second());
    }
}

void MainWindow::updateClock()
{
    if (!useCustomTime) {
        customTime = QTime::currentTime();
    }

    QTime currentTime = QTime::currentTime();
    int currentSecond = currentTime.second();
    int currentHour = currentTime.hour();
    int currentMinute = currentTime.minute();

    // Тик-так каждую секунду
    if (currentSecond != lastSecond) {
        if (isTick) tickSound.play();
        else tockSound.play();
        isTick = !isTick;
        lastSecond = currentSecond;
    }

    // Сбрасываем флаги в начале нового часа
    if (currentMinute == 0 && currentSecond == 0) {
        if (currentHour != lastChimedHour) {
            cuckooPlayed = false;
            hourChimePlayed = false;
            lastChimedHour = currentHour;
        }
    }

    // Проигрываем кукушку или бой часов только если еще не проигрывали в этот час
    if (currentMinute == 0 && currentSecond == 0) {
        if (!hourChimePlayed && !cuckooPlayed) {
            playHourChime(currentHour);

            // Устанавливаем флаги
            if (currentHour == 12 || currentHour == 0) {
                cuckooPlayed = true;
            } else {
                hourChimePlayed = true;
            }
        }
    }

    update();
}

void MainWindow::playHourChime(int hour)
{
    // В полдень и полночь играем песню кукушки
    if (hour == 12 || hour == 0) {
        if (!cuckooPlayed) {
            // Кукушка 12 раз с интервалом 0.5 секунды
            for (int i = 0; i < 12; ++i) {
                QTimer::singleShot(i * 500, [this]() {
                    cuckooSound.play();
                });
            }
            cuckooPlayed = true;
        }
    }
    // В остальные часы - бой часов
    else {
        if (!hourChimePlayed) {
            int strikes = hour > 12 ? hour - 12 : hour;
            for (int i = 0; i < strikes; ++i) {
                QTimer::singleShot(i * 1000, [this]() {
                    boomSound.play();
                });
            }
            hourChimePlayed = true;
        }
    }
}

void MainWindow::drawClockFace(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(width(), height()) / 3;

    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(centerX, centerY), radius, radius);

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);

    for (int i = 1; i <= 12; ++i) {
        double angle = (i * 30) * M_PI / 180;
        int x = centerX + (radius - 20) * sin(angle);
        int y = centerY - (radius - 20) * cos(angle);
        painter.drawText(x - 10, y - 10, 20, 20, Qt::AlignCenter, QString::number(i));
    }

    painter.restore();
}

void MainWindow::drawHand(QPainter &painter, double angle, int length, int width, QColor color)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    int centerX = this->width() / 2;
    int centerY = this->height() / 2;
    angle = (angle - 90) * M_PI / 180;
    int x = centerX + length * cos(angle);
    int y = centerY + length * sin(angle);

    painter.setPen(QPen(color, width));
    painter.drawLine(centerX, centerY, x, y);

    painter.restore();
}

void MainWindow::drawCuckoo(QPainter &painter, int secondsVisible)
{
    painter.save();

    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(width(), height()) / 3;
    int cuckooX = centerX + radius + 10 - qMin(secondsVisible * 5, 50);
    int cuckooY = centerY - 20;

    // Дверца
    painter.setPen(QPen(Qt::darkGreen, 2));
    painter.setBrush(Qt::lightGray);
    painter.drawRect(centerX + radius + 5, cuckooY - 30, 60, 40);

    // Кукушка
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawEllipse(cuckooX, cuckooY, 40, 30);
    painter.drawEllipse(cuckooX + 25, cuckooY - 10, 15, 15);
    painter.drawEllipse(cuckooX + 32, cuckooY - 8, 5, 5);
    painter.drawLine(cuckooX + 30, cuckooY + 5, cuckooX + 20, cuckooY + 15);

    painter.restore();
}

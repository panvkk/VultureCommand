#include "mainwindow.h"
#include <QPainter>
#include <QMenuBar>
#include <QInputDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QMessageBox>
#include <QLabel>
#include <QFont>
#include <QDebug>
#include <QCloseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTextBrowser>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    showSplash = true;
    setFixedSize(1024, 825);

    splashWidget = new QWidget(this);
    splashWidget->setGeometry(0, 0, width(), height());

    QVBoxLayout *layout = new QVBoxLayout(splashWidget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    QLabel *titleLabel1 = new QLabel("Мягкие часы", splashWidget);
    QLabel *titleLabel2 = new QLabel("Сальвадора Дали", splashWidget);

    QFont titleFont("Arial", 36, QFont::Bold);
    titleLabel1->setFont(titleFont);
    titleLabel2->setFont(titleFont);
    titleLabel1->setAlignment(Qt::AlignCenter);
    titleLabel2->setAlignment(Qt::AlignCenter);
    titleLabel1->setStyleSheet("color: #333;");
    titleLabel2->setStyleSheet("color: #333;");

    QLabel *imageLabel = new QLabel(splashWidget);
    QPixmap clockIcon(":/images/dali_background.jpg");
    if (!clockIcon.isNull()) {
        clockIcon = clockIcon.scaled(400 * 0.8, 300 * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(clockIcon);
    }
    imageLabel->setAlignment(Qt::AlignCenter);

    openButton = new QPushButton("Открыть часы", splashWidget);
    openButton->setFixedSize(200 * 0.8, 50 * 0.8);
    openButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4a86e8;"
        "   color: white;"
        "   border-radius: 10px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3a76d8;"
        "}"
        );
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openClock);

    QLabel *authorLabel = new QLabel("От Сакович А. Исаев К. Панов К.", splashWidget);
    QFont authorFont("Arial", 14 * 0.8);
    authorLabel->setFont(authorFont);
    authorLabel->setAlignment(Qt::AlignCenter);
    authorLabel->setStyleSheet("color: #555;");

    layout->addWidget(titleLabel1);
    layout->addWidget(titleLabel2);
    layout->addWidget(imageLabel);
    layout->addWidget(openButton, 0, Qt::AlignCenter);
    layout->addWidget(authorLabel);

    splashWidget->setLayout(layout);
    splashWidget->show();

    // Загрузка 12 часовых стрелок
    for (int i = 1; i <= 12; ++i) {
        QString hourPath = QString(":/images/hour_%1.png").arg(i);
        QPixmap hourPix(hourPath);
        if (!hourPix.isNull()) {
            hourPix = hourPix.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            qWarning() << "Failed to load hour image:" << hourPath;
        }
        hourHands.append(hourPix);
    }

    // Загрузка 60 минутных стрелок
    for (int i = 0; i < 60; ++i) {
        QString minutePath = QString(":/images/minute %1.png").arg(i);
        QPixmap minutePix(minutePath);
        if (!minutePix.isNull()) {
            minutePix = minutePix.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        } else {
            qWarning() << "Failed to load minute image:" << minutePath;
        }
        minuteHands.append(minutePix);
    }

    customTime = QTime::currentTime();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));
    QAction *exitAction = fileMenu->addAction(tr("Выход"));
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu *statsMenu = menuBar()->addMenu(tr("Статистика"));
    QAction *showStatsAction = statsMenu->addAction(tr("Показать статистику"));
    connect(showStatsAction, &QAction::triggered, this, &MainWindow::showStatistics);

    QMenu *formatMenu = menuBar()->addMenu(tr("Формат"));
    QAction *manualInputAction = formatMenu->addAction(tr("Установить время классически"));
    connect(manualInputAction, &QAction::triggered, this, &MainWindow::showNumericTimeDialog);
    QAction *verbalInputAction = formatMenu->addAction(tr("Установить время словесно"));
    connect(verbalInputAction, &QAction::triggered, this, &MainWindow::showVerbalTimeDialog);

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

void MainWindow::showVerbalTimeDialog()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Установка времени голосом"),
                                         tr("Введите время словами (например, \"три часа пятнадцать минут\"):"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);
    if (ok && !text.isEmpty()) {
        QTime time = parseVerbalTime(text);
        if (time.isValid()) {
            setCustomTime(time);
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось распознать время. Пожалуйста, попробуйте еще раз."));
        }
    }
}

QTime MainWindow::parseVerbalTime(const QString &verbalTime)
{
    QString lowerText = verbalTime.toLower();
    int hours = -1;
    int minutes = -1;
    int seconds = 0;

    QMap<QString, int> numWords = {
        {"ноль", 0}, {"один", 1}, {"одна", 1}, {"два", 2}, {"две", 2}, {"три", 3}, {"четыре", 4},
        {"пять", 5}, {"шесть", 6}, {"семь", 7}, {"восемь", 8}, {"девять", 9}, {"десять", 10},
        {"одиннадцать", 11}, {"двенадцать", 12}, {"тринадцать", 13}, {"четырнадцать", 14},
        {"пятнадцать", 15}, {"шестнадцать", 16}, {"семнадцать", 17}, {"восемнадцать", 18},
        {"девятнадцать", 19}, {"двадцать", 20}, {"тридцать", 30}, {"сорок", 40}, {"пятьдесят", 50}
    };
    QMap<QString, int> tensWords = {
        {"двадцать", 20}, {"тридцать", 30}, {"сорок", 40}, {"пятьдесят", 50}
    };

    QStringList words = lowerText.split(' ', Qt::SkipEmptyParts);

    // Определение часов
    for (int i = 0; i < words.size(); ++i) {
        if (words[i].contains("час") || words[i].contains("часов") || words[i].contains("часа")) {
            if (i > 0 && numWords.contains(words[i-1])) {
                hours = numWords[words[i-1]];
                if (hours == 12 && lowerText.contains("ночи")) hours = 0;
                if (hours == 12 && lowerText.contains("дня")) hours = 12;
                else if (hours < 12 && (lowerText.contains("пополудни") || lowerText.contains("вечера") || lowerText.contains("дня"))) {
                    hours += 12;
                }
            }
            break;
        }
    }

    bool inMinutesSection = false;
    bool inSecondsSection = false;

    // Определение минут и секунд
    for (int i = 0; i < words.size(); ++i) {
        if (words[i].contains("минут") || words[i].contains("минута") || words[i].contains("минуты")) {
            inMinutesSection = true;
            inSecondsSection = false;

            // Обработка составных числительных (десятки + единицы)
            if (i >= 2 && tensWords.contains(words[i-2]) && numWords.contains(words[i-1])) {
                minutes = tensWords[words[i-2]] + numWords[words[i-1]];
            }
            // Обработка простых числительных (десятки или единицы)
            else if (i >= 1 && numWords.contains(words[i-1])) {
                minutes = numWords[words[i-1]];
            }
        }
        else if (words[i].contains("секунд") || words[i].contains("секунда") || words[i].contains("секунды")) {
            inSecondsSection = true;
            inMinutesSection = false;

            // Аналогичная обработка для секунд
            if (i >= 2 && tensWords.contains(words[i-2]) && numWords.contains(words[i-1])) {
                seconds = tensWords[words[i-2]] + numWords[words[i-1]];
            }
            else if (i >= 1 && numWords.contains(words[i-1])) {
                seconds = numWords[words[i-1]];
            }
        }
        // Дополнительная обработка для минут
        else if (inMinutesSection && minutes == -1) {
            if (numWords.contains(words[i])) {
                minutes = numWords[words[i]];
            } else if (i < words.size() - 1 && tensWords.contains(words[i]) && numWords.contains(words[i+1])) {
                minutes = tensWords[words[i]] + numWords[words[i+1]];
                i++;
            }
        }
        // Дополнительная обработка для секунд
        else if (inSecondsSection && seconds == 0) {
            if (numWords.contains(words[i])) {
                seconds = numWords[words[i]];
            } else if (i < words.size() - 1 && tensWords.contains(words[i]) && numWords.contains(words[i+1])) {
                seconds = tensWords[words[i]] + numWords[words[i+1]];
                i++;
            }
        }
    }

    // Резервный поиск часов
    if (hours == -1 && minutes != -1) {
        int minutesIndex = lowerText.indexOf("минут");
        if (minutesIndex == -1) minutesIndex = lowerText.indexOf("минута");
        if (minutesIndex == -1) minutesIndex = lowerText.indexOf("минуты");

        if (minutesIndex != -1) {
            QString subString = lowerText.left(minutesIndex).trimmed();
            QStringList subWords = subString.split(' ', Qt::SkipEmptyParts);
            if (!subWords.isEmpty()) {
                QString lastWord = subWords.last();
                if (numWords.contains(lastWord)) {
                    hours = numWords[lastWord];
                    if (hours < 12 && (lowerText.contains("пополудни") || lowerText.contains("вечера") || lowerText.contains("дня"))) {
                        hours += 12;
                    }
                }
            }
        }
    }

    // Обработка специальных случаев
    if (lowerText.contains("полдень")) {
        hours = 12; minutes = 0; seconds = 0;
    } else if (lowerText.contains("полночь")) {
        hours = 0; minutes = 0; seconds = 0;
    }

    // Проверка валидности результата
    if (hours == -1 || minutes == -1) {
        return QTime();
    }

    // Коррекция часов (если указано 12 в 24-часовом формате)
    if (hours == 12 && !lowerText.contains("дня") && !lowerText.contains("пополудни")) {
        hours = 0;
    }

    return QTime(hours, minutes, seconds);
}

void MainWindow::setCustomTime(const QTime &time)
{
    customTime = time;
    useCustomTime = true;
    update();
}

void MainWindow::openClock()
{
    showSplash = false;
    splashWidget->hide();

    // Создаем меню сразу
    createMenus();

    // Вычисляем размер области часов (без меню)
    int menuHeight = menuBar()->height();
    QSize clockSize = QSize(width(), height() - menuHeight);

    // Загрузка фона
    if(!background.load(":/images/dali_background.jpg")) {
        background = QPixmap(clockSize);
        background.fill(Qt::black);
        qWarning() << "Failed to load background image!";
    } else {
        background = background.scaled(clockSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Масштабируем стрелки под новый размер
    for (int i = 0; i < hourHands.size(); ++i) {
        if (!hourHands[i].isNull()) {
            hourHands[i] = hourHands[i].scaled(clockSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }
    for (int i = 0; i < minuteHands.size(); ++i) {
        if (!minuteHands[i].isNull()) {
            minuteHands[i] = minuteHands[i].scaled(clockSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }

    sessionStartTime = QTime::currentTime();
    timer->start(1000);
    update();
}

QString MainWindow::formatDuration(int seconds)
{
    if (seconds > 120) {
        int m = seconds / 60;
        int s = seconds % 60;
        return tr("%1 мин. %2 сек.").arg(m).arg(s);
    } else {
        return tr("%1 сек.").arg(seconds);
    }
}


void MainWindow::showStatistics()
{
    QVector<SessionStats> allStats = loadAllSessionStats();

    // Создаём модальный диалог фиксированного размера
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Статистика"));
    dlg.setModal(true);
    dlg.setFixedSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 1) Блок общих показателей (всегда виден)
    QWidget *summaryWidget = new QWidget(&dlg);
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryWidget);
    summaryLayout->setSpacing(5);

    if (allStats.isEmpty()) {
        summaryLayout->addWidget(
            new QLabel(tr("Данные о предыдущих сессиях отсутствуют."), summaryWidget));
    }
    else {
        int totalSec = 0, minSec = INT_MAX, maxSec = 0;
        for (auto &s : allStats) {
            totalSec += s.durationSeconds;
            minSec = qMin(minSec, s.durationSeconds);
            maxSec = qMax(maxSec, s.durationSeconds);
        }
        int avgSec = qRound(double(totalSec) / allStats.size());

        summaryLayout->addWidget(
            new QLabel(tr("Всего сеансов: %1")
                           .arg(allStats.size()), summaryWidget));
        summaryLayout->addWidget(
            new QLabel(tr("Общее время работы: %1")
                           .arg(formatDuration(totalSec)), summaryWidget));
        summaryLayout->addWidget(
            new QLabel(tr("Средняя продолжительность: %1")
                           .arg(formatDuration(avgSec)), summaryWidget));
        summaryLayout->addWidget(
            new QLabel(tr("Минимальная / максимальная: %1 / %2")
                           .arg(formatDuration(minSec))
                           .arg(formatDuration(maxSec)), summaryWidget));

    }

    mainLayout->addWidget(summaryWidget);

    // 2) Прокручиваемая область со списком сеансов
    QScrollArea *scroll = new QScrollArea(&dlg);
    scroll->setWidgetResizable(true);

    QWidget *container = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(container);
    vbox->setAlignment(Qt::AlignTop);
    vbox->setSpacing(5);

    if (allStats.isEmpty()) {
        vbox->addWidget(
            new QLabel(tr("Нет данных для отображения."), container));
    }
    else {
        for (int i = 0; i < allStats.size(); ++i) {
            const SessionStats &s = allStats.at(i);
            int dur = s.durationSeconds;

            // Формируем строку длительности: если > 120 сек, переводим в мин. и сек.
            QString durStr;
            if (dur > 120) {
                int m = dur / 60;
                int sec = dur % 60;
                durStr = tr("%1 мин. %2 сек.").arg(m).arg(sec);
            } else {
                durStr = tr("%1 сек.").arg(dur);
            }

            QString line = tr("%1. %2 – %3   Длительность: %4")
                               .arg(i + 1)
                               .arg(s.startTime.toString("HH:mm:ss"))
                               .arg(s.endTime.toString("HH:mm:ss"))
                               .arg(durStr);

            QLabel *lbl = new QLabel(line, container);
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            vbox->addWidget(lbl);
        }

    }

    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    dlg.exec();
}



void MainWindow::showHelp()
{
    QString helpText = tr("Программа 'Мягкие часы Дали'\n\n"
                          "Эта программа отображает знаменитые часы Сальвадора Дали "
                          "из картины 'Постоянство памяти' (1931).\n\n"
                          "Особенности реализации:\n"
                          "- Точное воспроизведение оригинальных стрелок Дали\n"
                          "- Используются готовые изображения для каждой позиции стрелок\n"
                          "- Реалистичная текстура фона картины\n\n"
                          "Для установки времени используйте меню 'Формат' -> 'Установить время классически/словесно'.\n"
                          "Для просмотра статистики используйте меню 'Статистика' -> 'Показать статистику'.");

    QMessageBox::information(this, tr("Помощь"), helpText);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (showSplash) {
        return;
    }

    QPainter painter(this);
    int menuHeight = menuBar()->height();

    // Рисуем фон часов под меню
    painter.drawPixmap(0, menuHeight, background);

    // Рисуем стрелки
    int hour = customTime.hour();
    int minute = customTime.minute();

    // Выбор часового изображения
    int hourImageIndex = hour % 12;

    // Выбор минутного изображения
    int minuteImageIndex = minute;

    // Отрисовка часовой стрелки
    if (hourImageIndex < hourHands.size() && !hourHands[hourImageIndex].isNull()) {
        painter.drawPixmap(0, menuHeight, hourHands[hourImageIndex]);
    } else {
        qWarning() << "Hour hand image missing for index" << hourImageIndex;
    }

    // Отрисовка минутной стрелки
    if (minuteImageIndex < minuteHands.size() && !minuteHands[minuteImageIndex].isNull()) {
        painter.drawPixmap(0, menuHeight, minuteHands[minuteImageIndex]);
    } else {
        qWarning() << "Minute hand image missing for index" << minuteImageIndex;
    }
}

void MainWindow::updateClock()
{
    if (!useCustomTime) {
        customTime = QTime::currentTime();
    }
    update();
}

void MainWindow::saveSessionStats(const SessionStats &stats)
{
    QFile file(STATS_FILE_NAME);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << stats.startTime.toString("HH:mm:ss") << ","
            << stats.endTime.toString("HH:mm:ss") << ","
            << stats.durationSeconds << "\n";
        file.close();
    } else {
        qWarning() << "Failed to open stats file for writing:" << STATS_FILE_NAME;
    }
}

QVector<SessionStats> MainWindow::loadAllSessionStats()
{
    QVector<SessionStats> allStats;
    QFile file(STATS_FILE_NAME);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(',');
            if (parts.size() == 3) {
                SessionStats stats;
                stats.startTime = QTime::fromString(parts.at(0), "HH:mm:ss");
                stats.endTime = QTime::fromString(parts.at(1), "HH:mm:ss");
                stats.durationSeconds = parts.at(2).toInt();
                if (stats.startTime.isValid() && stats.endTime.isValid()) {
                    allStats.append(stats);
                }
            }
        }
        file.close();
    } else {
        qWarning() << "Failed to open stats file for reading (or file does not exist):" << STATS_FILE_NAME;
    }
    return allStats;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!showSplash) {
        QTime sessionEndTime = QTime::currentTime();
        int duration = sessionStartTime.secsTo(sessionEndTime);
        if (duration < 0) {
            duration += 24 * 3600;
        }

        SessionStats currentSession;
        currentSession.startTime = sessionStartTime;
        currentSession.endTime = sessionEndTime;
        currentSession.durationSeconds = duration;

        saveSessionStats(currentSession);
    }
    QMainWindow::closeEvent(event);
}

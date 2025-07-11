#include "mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <cmath>
#include <QMenuBar>
#include <QTimeEdit>
#include <QInputDialog>
#include <QPushButton>
#include <QAudioOutput>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextBrowser>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    startTime = QTime::currentTime();
    showSplash = true;
    setFixedSize(800, 600);

    // Set menu bar style
    menuBar()->setStyleSheet(
        "QMenuBar { background-color: #f0f0f0; border-bottom: 1px solid #d0d0d0; color:black }"
        "QMenuBar::item { background: transparent; padding: 5px 10px; }"
        "QMenuBar::item:selected { background: #d0d0d0; }"
        );

    currentHandStyle = "classic";

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

    // Загрузка сохраненной статистики
    QFile file("statistics.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" | ");
            if (parts.size() == 2) {
                QStringList times = parts[0].split(" - ");
                if (times.size() == 2) {
                    QTime start = QTime::fromString(times[0], "HH:mm:ss");
                    QTime end = QTime::fromString(times[1], "HH:mm:ss");
                    if (start.isValid() && end.isValid()) {
                        testSessions.append(qMakePair(start, end));
                        int duration = start.secsTo(end);
                        totalDuration += duration;
                    }
                }
            }
        }
        file.close();
    }
    totalSessions = testSessions.size();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveTestSessions();
    QMainWindow::closeEvent(event);
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
    // Инициализация аудио
    musicPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    musicPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5); // 50% громкости по умолчанию

    connect(musicPlayer, &QMediaPlayer::playbackStateChanged, this, &MainWindow::handleMusicStateChanged);

    QMenu *musicMenu = mainMenu->addMenu(tr("Музыка"));
    musicMenu->addAction(tr("Птицы"), this, &MainWindow::playMusicTrack1);
    musicMenu->addAction(tr("Напряжение"), this, &MainWindow::playMusicTrack2);
    musicMenu->addAction(tr("Триумф"), this, &MainWindow::playMusicTrack3);

    // Statistics menu
    QMenu *statsMenu = menuBar()->addMenu(tr("Статистика"));
    QAction *showStatsAction = statsMenu->addAction(tr("Показать статистику"));
    connect(showStatsAction, &QAction::triggered, this, &MainWindow::showStatistics);

    // Format menu
    QMenu *formatMenu = menuBar()->addMenu(tr("Формат"));
    QAction *numericAction = formatMenu->addAction(tr("Числовой ввод"));
    QAction *verbalAction = formatMenu->addAction(tr("Словесный ввод"));
    verbalAction->setVisible(false);

    connect(numericAction, &QAction::triggered, this, &MainWindow::showNumericTimeDialog);
    connect(verbalAction, &QAction::triggered, this, &MainWindow::showVerbalTimeDialog);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("Помощь"));
    QAction *helpAction = helpMenu->addAction(tr("О программе"));
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
}

void MainWindow::saveTestSessions()
{
    QTime endTime = QTime::currentTime();
    int duration = startTime.secsTo(endTime);

    testSessions.append(qMakePair(startTime, endTime));
    totalSessions++;
    totalDuration += duration;

    // Сохраняем в файл при каждом закрытии
    QFile file("statistics.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << startTime.toString("HH:mm:ss") << " - "
               << endTime.toString("HH:mm:ss") << " | "
               << duration << " сек.\n";
        file.close();
    }

    startTime = QTime::currentTime(); // Сбрасываем для следующей сессии
}

void MainWindow::showStatistics()
{
    // Загружаем историю из файла, если она есть
    QVector<QPair<QString, QString>> history;
    QFile file("statistics.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(" | ");
            if (parts.size() == 2) {
                history.append(qMakePair(parts[0], parts[1]));
            }
        }
        file.close();
    }

    // Формируем текст статистики
    QString stats = tr("<h2>Статистика работы часов</h2>");

    // Общая статистика
    stats += tr("<h3>Общие показатели:</h3>");
    stats += tr("<p>Всего сеансов: <b>%1</b></p>").arg(totalSessions);
    stats += tr("<p>Общее время работы: <b>%1 мин. %2 сек.</b></p>")
                 .arg(totalDuration / 60).arg(totalDuration % 60);
    stats += tr("<p>Средняя продолжительность: <b>%1 сек.</b></p>")
                 .arg(totalSessions > 0 ? totalDuration / totalSessions : 0);

    // Последние 5 сеансов
    stats += tr("<h3>Последние сеансы:</h3>");
    stats += "<table border='1' cellpadding='5' style='border-collapse: collapse;'>";
    stats += "<tr><th>№</th><th>Время запуска</th><th>Время завершения</th><th>Длительность</th></tr>";

    int startIdx = qMax(0, testSessions.size() - 5);
    for (int i = startIdx; i < testSessions.size(); ++i) {
        auto session = testSessions[i];
        int duration = session.first.secsTo(session.second);
        stats += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4 сек.</td></tr>")
                     .arg(i+1)
                     .arg(session.first.toString("HH:mm:ss"))
                     .arg(session.second.toString("HH:mm:ss"))
                     .arg(duration);
    }
    stats += "</table>";

    // Полная история (если есть)
    if (!history.isEmpty()) {
        stats += tr("<h3>Полная история:</h3>");
        stats += "<table border='1' cellpadding='5' style='border-collapse: collapse;'>";
        stats += "<tr><th>№</th><th>Период работы</th><th>Длительность</th></tr>";

        for (int i = 0; i < history.size(); ++i) {
            stats += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
            .arg(i+1)
                .arg(history[i].first)
                .arg(history[i].second);
        }
        stats += "</table>";
    }

    // Создаем диалог
    QDialog statsDialog(this);
    statsDialog.setWindowTitle(tr("Статистика работы"));
    statsDialog.resize(600, 500);

    QTextBrowser *textBrowser = new QTextBrowser(&statsDialog);
    textBrowser->setHtml(stats);

    QPushButton *saveButton = new QPushButton(tr("Сохранить в файл"), &statsDialog);
    QPushButton *closeButton = new QPushButton(tr("Закрыть"), &statsDialog);

    connect(saveButton, &QPushButton::clicked, [this, stats]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить статистику"),
                                                        "statistics_report.html",
                                                        tr("HTML файлы (*.html);;Текстовые файлы (*.txt)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << stats;
                file.close();
                QMessageBox::information(this, tr("Сохранено"), tr("Статистика сохранена в файл."));
            }
        }
    });

    connect(closeButton, &QPushButton::clicked, &statsDialog, &QDialog::close);

    QVBoxLayout *layout = new QVBoxLayout(&statsDialog);
    layout->addWidget(textBrowser);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    statsDialog.exec();
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

void MainWindow::showVerbalTimeDialog()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Словесный ввод"),
                                         tr("Введите время словами (например, \"два часа пятнадцать минут\"):"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);

    if (ok && !text.isEmpty()) {
        QTime time = parseVerbalTime(text);
        if (time.isValid()) {
            setCustomTime(time);
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Неверный формат времени!"));
        }
    }
}

QTime MainWindow::parseVerbalTime(const QString &verbalTime)
{
    QString input = verbalTime.toLower().simplified();
    int hours = -1;
    int minutes = 0;
    int seconds = 0;

    // Удаляем лишние слова
    input.remove("ровно");
    input.remove("часов");
    input.remove("минут");
    input.remove("секунд");

    // Словарь числительных
    QMap<QString, int> numbers = {
        {"ноль", 0}, {"один", 1}, {"два", 2}, {"три", 3}, {"четыре", 4},
        {"пять", 5}, {"шесть", 6}, {"семь", 7}, {"восемь", 8}, {"девять", 9},
        {"десять", 10}, {"одиннадцать", 11}, {"двенадцать", 12},
        {"тринадцать", 13}, {"четырнадцать", 14}, {"пятнадцать", 15},
        {"шестнадцать", 16}, {"семнадцать", 17}, {"восемнадцать", 18},
        {"девятнадцать", 19}, {"двадцать", 20}, {"тридцать", 30}, {"сорок", 40}, {"пятьдесят", 50}
    };

    // Специальные случаи
    if (input.contains("полдень")) return QTime(12, 0);
    if (input.contains("полночь")) return QTime(0, 0);

    // Разбиваем на слова
    QStringList words = input.split(' ', Qt::SkipEmptyParts);

    // Парсим часы
    for (int i = 0; i < words.size(); ++i) {
        if (words[i].startsWith("час") || words[i] == "часа") {
            if (i > 0 && numbers.contains(words[i-1])) {
                hours = numbers[words[i-1]];
            }
            break;
        }
    }

    // Если часы не найдены, берем первое число
    if (hours == -1) {
        for (const QString &word : words) {
            if (numbers.contains(word)) {
                hours = numbers[word];
                break;
            }
        }
    }

    // Парсим минуты
    bool minutesNext = false;
    for (int i = 0; i < words.size(); ++i) {
        if (words[i] == "и") {
            minutesNext = true;
            continue;
        }

        if (minutesNext || (i > 0 && words[i-1] == "и")) {
            if (numbers.contains(words[i])) {
                minutes = numbers[words[i]];
            }
            // Обработка составных чисел (двадцать пять)
            else if (i < words.size()-1 && numbers.contains(words[i+1])) {
                minutes = numbers[words[i]] + numbers[words[i+1]];
                i++;
            }
            break;
        }
    }

    // Коррекция формата (12/24 часа)
    if (input.contains("вечер") || input.contains("пополудни") ||
        input.contains("ночи") || input.contains("дня")) {
        if (hours < 12) hours += 12;
        if (hours == 24) hours = 0;
    }

    // Проверка валидности
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        return QTime(); // Некорректное время
    }

    return QTime(hours, minutes, seconds);
}

void MainWindow::setCustomTime(const QTime &time)
{
    QTime current = QTime::currentTime();
    // Вычисляем разницу между текущим временем и устанавливаемым
    timeOffset = QTime(0, 0, 0).addSecs(current.secsTo(time));
    useCustomTime = true;
    update();
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

void MainWindow::setClassicHands()
{
    currentHandStyle = "classic";
    update();
    QMessageBox::information(this, tr("Стиль стрелок"), tr("Установлен классический стиль стрелок"));
}

void MainWindow::setModernHands()
{
    currentHandStyle = "modern";
    update();
    QMessageBox::information(this, tr("Стиль стрелок"), tr("Установлен современный стиль стрелок"));
}

void MainWindow::setFantasyHands()
{
    currentHandStyle = "fantasy";
    update();
    QMessageBox::information(this, tr("Стиль стрелок"), tr("Установлен фэнтезийный стиль стрелок"));
}

void MainWindow::playMusicTrack1()
{
    currentTrack = "qrc:/sounds/birds.wav";
    musicPlayer->setSource(QUrl(currentTrack));
    musicPlayer->play();
}

void MainWindow::playMusicTrack2()
{
    currentTrack = "qrc:/sounds/tension.wav";
    musicPlayer->setSource(QUrl(currentTrack));
    musicPlayer->play();
}

void MainWindow::playMusicTrack3()
{
    currentTrack = "qrc:/sounds/triumph.wav";
    musicPlayer->setSource(QUrl(currentTrack));
    musicPlayer->play();
}

void MainWindow::handleMusicStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState && !currentTrack.isEmpty()) {
        // Если трек закончился - воспроизводим его снова
        musicPlayer->setSource(QUrl(currentTrack));
        musicPlayer->play();
    }
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

    QTime time = useCustomTime ? customTime : QTime::currentTime();
    if ((time.hour() == 12 || time.hour() == 0) &&
        time.minute() == 0 &&
        time.second() < 30 &&
        cuckooPlayed) {
        drawCuckoo(painter, time.second());
    }
    drawClockFace(painter);

    // Отрисовка стрелок
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
    QTime currentRealTime = QTime::currentTime();

    // Инициализация при первом запуске
    static bool firstUpdate = true;
    if (firstUpdate) {
        lastSecond = currentRealTime.second();
        isTick = true;
        firstUpdate = false;
    }

    if (useCustomTime) {
        // Добавляем смещение к текущему времени
        QTime current = QTime::currentTime();
        int totalSecs = current.hour() * 3600 + current.minute() * 60 + current.second();
        totalSecs += timeOffset.hour() * 3600 + timeOffset.minute() * 60 + timeOffset.second();

        // Нормализуем время (если вышли за 24 часа)
        totalSecs = totalSecs % (24 * 3600);
        customTime = QTime(0, 0, 0).addSecs(totalSecs);
    } else {
        customTime = QTime::currentTime();
    }

    // Тик-так каждую секунду
    int currentSecond = currentRealTime.second();
    if (currentSecond != lastSecond) {
        if (isTick) tickSound.play();
        else tockSound.play();
        isTick = !isTick;
        lastSecond = currentSecond;
    }

    // Остальная логика обновления часов
    int currentHour = currentRealTime.hour();
    int currentMinute = currentRealTime.minute();

    // Сбрасываем флаги в начале нового часа
    if (currentMinute == 0 && currentSecond == 0) {
        if (currentHour != lastChimedHour) {
            cuckooPlayed = false;
            hourChimePlayed = false;
            lastChimedHour = currentHour;
        }
    }

    // Проигрываем кукушку или бой часов
    if (currentMinute == 0 && currentSecond == 0) {
        if (!hourChimePlayed && !cuckooPlayed) {
            playHourChime(currentHour);
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
    angle = (angle - 90) * M_PI / 180; // Конвертируем в радианы и смещаем на 90°

    if (currentHandStyle == "classic") {
        // Улучшенные классические стрелки
        QLinearGradient gradient(centerX, centerY,
                                centerX + length*cos(angle),
                                centerY + length*sin(angle));
        gradient.setColorAt(0, color.darker(120));
        gradient.setColorAt(1, color.lighter(120));

        painter.setPen(QPen(QBrush(gradient), width));
        painter.setBrush(Qt::NoBrush);

        QPointF endPoint(centerX + length*cos(angle),
                        centerY + length*sin(angle));
        painter.drawLine(QPointF(centerX, centerY), endPoint);

        // Декоративный круг в основании
        painter.setBrush(gradient);
        painter.drawEllipse(QPointF(centerX, centerY), width*1.5, width*1.5);
    }
    else if (currentHandStyle == "modern") {
        // Стильные современные стрелки с градиентом и тенью
        QPainterPath path;
        QPointF tip(centerX + length*cos(angle), centerY + length*sin(angle));

        // Создаем форму стрелки
        path.moveTo(tip);
        path.lineTo(centerX + width*2*cos(angle + M_PI/2),
                   centerY + width*2*sin(angle + M_PI/2));
        path.lineTo(centerX + width*0.3*cos(angle + M_PI),
                   centerY + width*0.3*sin(angle + M_PI));
        path.lineTo(centerX + width*2*cos(angle - M_PI/2),
                   centerY + width*2*sin(angle - M_PI/2));
        path.closeSubpath();

        // Градиентная заливка
        QLinearGradient gradient(centerX, centerY, tip.x(), tip.y());
        gradient.setColorAt(0, color.darker(150));
        gradient.setColorAt(0.7, color);
        gradient.setColorAt(1, color.lighter(150));

        // Тень
        painter.setPen(QPen(QColor(0, 0, 0, 50), 1));
        painter.setBrush(QColor(0, 0, 0, 30));
        painter.drawPath(path.translated(2, 2));

        // Основная стрелка
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawPath(path);
    }
    else if (currentHandStyle == "fantasy") {
        // Фэнтезийные стрелки с орнаментом
        QPainterPath path;
        QPointF tip(centerX + length*cos(angle), centerY + length*sin(angle));

        // Основная линия с волнообразными краями
        for (float t = 0; t <= 1.0; t += 0.05) {
            float currentLength = t * length;
            float waveWidth = width * (1 + 0.3*sin(t * M_PI * 5));

            QPointF point(
                centerX + currentLength*cos(angle) + waveWidth*cos(angle + M_PI/2),
                centerY + currentLength*sin(angle) + waveWidth*sin(angle + M_PI/2)
            );

            if (t == 0) path.moveTo(point);
            else path.lineTo(point);
        }

        for (float t = 1.0; t >= 0; t -= 0.05) {
            float currentLength = t * length;
            float waveWidth = width * (1 + 0.3*sin(t * M_PI * 5));

            QPointF point(
                centerX + currentLength*cos(angle) - waveWidth*cos(angle + M_PI/2),
                centerY + currentLength*sin(angle) - waveWidth*sin(angle + M_PI/2)
            );

            path.lineTo(point);
        }

        path.closeSubpath();

        // Декоративные элементы
        QPainterPath decorPath;
        for (int i = 1; i <= 3; i++) {
            float r = length * 0.2 * i;
            decorPath.addEllipse(QPointF(centerX + r * cos(angle + M_PI / 2), centerY + r * sin(angle + M_PI / 2)), width * 0.3, width * 0.3);
            decorPath.addEllipse(QPointF(centerX + r * cos(angle + M_PI / 2), centerY + r * sin(angle + M_PI / 2)), width * 0.3, width * 0.3);
        }

        // Рисуем с градиентом
        QRadialGradient gradient(centerX, centerY, length*1.2);
        gradient.setColorAt(0, QColor(200, 160, 0));
        gradient.setColorAt(1, QColor(150, 50, 0));

        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(gradient);
        painter.drawPath(path);
        painter.setBrush(Qt::white);
        painter.drawPath(decorPath);
    }

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

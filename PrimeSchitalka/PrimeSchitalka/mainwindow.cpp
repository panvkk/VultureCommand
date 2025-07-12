#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <cmath>
#include <QMessageBox>
#include <QTimer>
#include <ctime>
#include <QRandomGenerator>
#include <QSettings>
#include <QStyle>
#include <QApplication>
#include <QPalette>
#include <QStyleHints>
#include <QMenuBar>
#include <QFileDialog>

using namespace std;

bool isSystemDarkTheme() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        return true;
    }
#endif

    QSettings registry(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
        );
    return registry.value("AppsUseLightTheme", 1).toInt() == 0;
}

/*MainWindow::MainWindow(int participants, const QString& rhyme, const QString& clientName, QWidget *parent)
    : QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_participants(participants),
    m_clientName(clientName)
{
    m_ui->setupUi(this);
    m_rhymeWords = rhyme.split(' ', Qt::SkipEmptyParts);

    // Остальная инициализация
    m_isDarkTheme = isSystemDarkTheme();
    applyThemeStyles();
    srand(time(0));
    CreateMenuBar();
    InitializeComponents();
    SetupCurrentRhymeWordLabel();
    SetupRhymeTimer();

    // Инициализация игры с заданными параметрами
    InitializeGame();
}*/

MainWindow::MainWindow(int participants, const QString& rhyme, const QString& clientName, QWidget *parent)
    : QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_participants(participants),
    m_clientName(clientName)
{
    m_ui->setupUi(this);
    m_rhymeWords = rhyme.split(' ', Qt::SkipEmptyParts);

    // Остальная инициализация
    m_isDarkTheme = isSystemDarkTheme();
    applyThemeStyles();
    srand(time(0));
    CreateMenuBar();
    InitializeComponents();
    SetupCurrentRhymeWordLabel();
    SetupRhymeTimer();

    // Не начинаем игру сразу, ждем выбора фото
    m_ui->nextWordButton->setEnabled(false);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_longestWordLabel(nullptr)
    , m_currentRhymeWordLabel(nullptr)
    , m_nextWordButton(nullptr)
    , m_rhymeTimer(nullptr)
    , m_currentIndex(0)
    , m_currentWordIndex(0)
    , m_rhymeRunning(false)
    , m_firstTime(true)
    , m_longestWordShowed(false)
    , m_rhymePaused(false)
    , m_highlightedLabel(nullptr)
{
    m_ui->setupUi(this);

    m_isDarkTheme = isSystemDarkTheme();
    applyThemeStyles();
    srand(time(0));
    CreateMenuBar();
    InitializeComponents();
    SetupCurrentRhymeWordLabel();
    SetupRhymeTimer();
}


MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_rhymeTimer;
}

void MainWindow::applyThemeStyles()
{
    QPalette palette = QApplication::palette();
    if (m_isDarkTheme) {
        // Темная палитра
        palette.setColor(QPalette::Window, QColor(45, 45, 45));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(35, 35, 35));
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(58, 58, 58));
        palette.setColor(QPalette::ButtonText, Qt::white);
    } else {
        // Светлая палитра
        palette = QApplication::style()->standardPalette(); // Исправлено
    }
    QApplication::setPalette(palette);

    // Стиль кнопки
    QString buttonStyle =
        "QPushButton {"
        "   background-color: %bg%;"
        "   color: %text%;"
        "   border: 1px solid %border%;"
        "   border-radius: 4px;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %hover%;"
        "}";

    if (m_isDarkTheme) {
        buttonStyle
            .replace("%bg%", "#3a3a3a")
            .replace("%text%", "white")
            .replace("%border%", "#555555")
            .replace("%hover%", "#4a4a4a");
    } else {
        buttonStyle
            .replace("%bg%", "#e0e0e0")
            .replace("%text%", "black")
            .replace("%border%", "#cccccc")
            .replace("%hover%", "#d0d0d0");
    }

    // Применяем стиль ко всем кнопкам
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        button->setStyleSheet(buttonStyle);
    }
}

void MainWindow::InitializeComponents()
{
    setWindowTitle("Vulture Command");
    QRandomGenerator::securelySeeded();

    m_nextWordButton = m_ui->nextWordButton;
    m_nextWordButton->setEnabled(false); // Делаем кнопку неактивной по умолчанию
    connect(m_ui->nextWordButton, &QPushButton::clicked, this, &MainWindow::OnNextWordButtonClicked);
}

bool MainWindow::InitializeGame()
{
    if (m_persons.isEmpty() || m_rhymeWords.isEmpty()) {
        return false;
    }
    random_device rd;
    mt19937 g(rd());
    shuffle(m_persons.begin(), m_persons.end(), g);

    m_currentIndex = rand() % m_persons.size();
    DisplayPhotosInCircle(m_persons, m_firstTime);
    m_firstTime = false;

    return true;
}


void MainWindow::CreateMenuBar()
{
    menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu("Файл");

    selectPhotosAction = new QAction("Выбрать папку с фото", this);
    connect(selectPhotosAction, &QAction::triggered, this, &MainWindow::SelectPhotosFolder);
    fileMenu->addAction(selectPhotosAction);

    selectRhymesAction = new QAction("Выбрать файл со считалочками", this);
    connect(selectRhymesAction, &QAction::triggered, this, &MainWindow::SelectRhymesFile);
    fileMenu->addAction(selectRhymesAction);

    setMenuBar(menuBar);
}


void MainWindow::SelectPhotosFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку с фото",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!folderPath.isEmpty()) {
        m_persons = LoadPhotos(folderPath);
        if (m_persons.isEmpty()) {
            QMessageBox::warning(this, "Ошибка",
                                 "В папке нет поддерживаемых фото (png, jpg, jpeg, HEIC)!");
        }
        else {
            QMessageBox::information(this, "Успешно",
                                     QString("Загружено %1 фотографий!").arg(m_persons.size()));
            DisplayPhotosInCircle(m_persons, true);
            InitializeLongestWordLabel(m_rhymeWords.join(" "));

            selectPhotosAction->setEnabled(false);
            selectPhotosAction->setText("Фото загружены");

            // Активируем кнопку только после выбора фото
            m_ui->nextWordButton->setEnabled(true);
        }
    }
}
void MainWindow::SelectRhymesFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Выберите файл со считалочками",
        QDir::homePath(),
        "Текстовые файлы (*.txt);;Все файлы (*)"
        );

    if (!filePath.isEmpty()) {
        m_allRhymes = LoadRhymes(filePath); // Сохраняем все считалочки
        if (m_allRhymes.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Файл пуст или не читается!");
        }
        else {
            QMessageBox::information(this, "Успешно", "Считалочки загружены!");
            selectRhymesAction->setEnabled(false);
            selectRhymesAction->setText("Считалки загружены");
        }
    }
}

void MainWindow::ResetGame()
{
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label != m_longestWordLabel && label != m_currentRhymeWordLabel) {
            label->deleteLater();
        }
    }

    m_firstTime = true;
    m_longestWordShowed = false;
    m_rhymeRunning = false;
    m_currentIndex = 0;
    m_currentWordIndex = 0;

    if (!m_persons.isEmpty() && !m_rhymeWords.isEmpty()) {
        DisplayPhotosInCircle(m_persons, true);
        InitializeLongestWordLabel(m_rhymeWords.join(" "));
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не хватает данных для запуска игры!");
    }
}

void MainWindow::SetupCurrentRhymeWordLabel()
{
    m_currentRhymeWordLabel = new QLabel(this);
    m_currentRhymeWordLabel->setAlignment(Qt::AlignCenter);
    m_currentRhymeWordLabel->setFont(QFont("TimesNewRoman", 14));

    //Установка цвета текста в зависимости от темы
    QString textColor = m_isDarkTheme ? "white" : "black";
    m_currentRhymeWordLabel->setStyleSheet("color: " + textColor + "; font-weight: bold;");

    m_currentRhymeWordLabel->move(-25, height() / 2);
    m_currentRhymeWordLabel->setFixedSize(width(), 30);
    m_currentRhymeWordLabel->hide();
}

bool MainWindow::InitializeGame(const QVector<QString>& rhymes)
{
    if (m_persons.isEmpty() || m_rhymeWords.isEmpty()) {
        return false;
    }

    random_device rd;
    mt19937 g(rd());
    shuffle(m_persons.begin(), m_persons.end(), g);
    m_currentIndex = rand() % m_persons.size();
    DisplayPhotosInCircle(m_persons, m_firstTime);
    m_firstTime = false;
    QString rhyme = SelectRandomRhyme(rhymes);
    m_rhymeWords = rhyme.split(' ');
    InitializeLongestWordLabel(rhyme);
    return true;
}

void MainWindow::InitializeLongestWordLabel(const QString& rhyme)
{
    m_longestWordLabel = new QLabel(this);
    m_longestWordLabel->setObjectName("longestWordLabel");
    m_longestWordLabel->setFont(QFont("TimesNewRoman", 8));
    m_longestWordLabel->setText("Самое длинное слово: " + FindLongestWord(rhyme));
    m_longestWordLabel->setAlignment(Qt::AlignLeft);
    m_longestWordLabel->setGeometry(10, menuBar->height() + 5, width() - 20, 20);

    // Установка цвета текста в зависимости от темы
    QString textColor = m_isDarkTheme ? "white" : "black";
    m_longestWordLabel->setStyleSheet("color: " + textColor + ";");

    m_longestWordLabel->hide();
}

void MainWindow::SetupRhymeTimer()
{
    m_rhymeTimer = new QTimer(this);
    connect(m_rhymeTimer, &QTimer::timeout, this, &MainWindow::UpdateRhymeWord);
}

QVector<QString> MainWindow::LoadRhymes(const QString& filePath)
{
    QVector<QString> rhymes;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine().remove(QRegularExpression("[,.-_;:!?]")).trimmed();
            if (!line.isEmpty()) rhymes.append(line);
        }
        file.close();
    }
    return rhymes;
}

QList<Person> MainWindow::LoadPhotos(const QString& folderPath)
{
    QList<Person> persons;
    QDir dir(folderPath);
    QStringList filters = { "*.png", "*.jpg", "*.jpeg", "*.HEIC" };
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo& fileInfo : fileList)
    {
        QPixmap pixmap(fileInfo.absoluteFilePath());
        if (!pixmap.isNull()) persons.append(Person(fileInfo.baseName(), pixmap));
    }
    return persons;
}

void MainWindow::DisplayPhotosInCircle(const QList<Person>& persons, bool firstTime)
{
    QList<QLabel*> photoLabels = findChildren<QLabel*>(QRegularExpression(".+_photo$"));
    QList<QLabel*> nameLabels = findChildren<QLabel*>(QRegularExpression(".+_name$"));
    QList<QLabel*> rhymeLabels = findChildren<QLabel*>(QRegularExpression(".+_rhyme$"));
    int centerX = width() / 2 - 75;
    int centerY = height() / 2 - 50;
    int radius = sqrt(pow(centerX, 2) + pow(centerY, 2)) / 2 + 30;
    int count = persons.size();
    if (firstTime)  for (int i = 0; i < count; ++i) CreatePersonWidget(persons[i], i, count, centerX, centerY, radius);
    else AnimateExistingWidgets(persons, centerX, centerY, radius, count);
}

void MainWindow::AnimateExistingWidgets(const QList<Person>& persons, int centerX, int centerY, int radius, int count)
{
    for (int i = 0; i < count; ++i)
    {
        double angle = (2 * M_PI / count) * i;
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);
        AnimatePersonWidgets(persons[i], x, y);
    }
}

void MainWindow::AnimatePersonWidgets(const Person& person, int x, int y)
{
    QLabel* photoLabel = findChild<QLabel*>(person.name + "_photo");
    QLabel* nameLabel = findChild<QLabel*>(person.name + "_name");
    QLabel* rhymeLabel = findChild<QLabel*>(person.name + "_rhyme");
    AnimateWidget(photoLabel, QPoint(x+25, y-25));
    AnimateWidget(nameLabel, QPoint(x+25, y+75));
    AnimateWidget(rhymeLabel, QPoint(x+25, y+90));
}

void MainWindow::AnimateWidget(QLabel* widget, const QPoint& endPos)
{
    if (widget)
    {
        QPropertyAnimation* anim = new QPropertyAnimation(widget, "pos");
        anim->setDuration(800);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        anim->setEndValue(endPos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::CreatePersonWidget(const Person& person, int index, int count, int centerX, int centerY, int radius)
{
    double angle = (2 * M_PI / count) * index;
    int x = centerX + radius * cos(angle);
    int y = centerY + radius * sin(angle);
    QLabel* photoLabel = CreatePhotoLabel(person);
    QLabel* nameLabel = CreateNameLabel(person);
    QLabel* rhymeLabel = CreateRhymeLabel(person);
    SetupPersonAnimations(photoLabel, nameLabel, rhymeLabel, x, y);
}

QLabel* MainWindow::CreatePhotoLabel(const Person& person)
{
    QLabel* photoLabel = new QLabel(this);
    photoLabel->setPixmap(person.photo.scaledToWidth(100, Qt::SmoothTransformation));
    photoLabel->setObjectName(person.name + "_photo");
    photoLabel->setGeometry(width()/2-50, height()/2-50, 100, 100);
    photoLabel->show();
    return photoLabel;
}

QLabel* MainWindow::CreateNameLabel(const Person& person)
{
    QLabel* nameLabel = new QLabel(person.name, this);
    nameLabel->setFont(QFont("TimesNewRoman", 10));
    QString textColor = m_isDarkTheme ? "white" : "black";
    nameLabel->setStyleSheet("color: " + textColor + "; font-weight: bold;");

    nameLabel->setAlignment(Qt::AlignLeft);
    nameLabel->setObjectName(person.name + "_name");
    nameLabel->setGeometry(width()/2-50, height()/2+50, 200, 20);
    nameLabel->show();
    return nameLabel;
}

QLabel* MainWindow::CreateRhymeLabel(const Person& person)
{
    QLabel* rhymeLabel = new QLabel(this);
    rhymeLabel->setAlignment(Qt::AlignLeft);
    rhymeLabel->setFont(QFont("TimesNewRoman", 10));
    QString textColor = m_isDarkTheme ? "white" : "black";
    rhymeLabel->setStyleSheet("color: " + textColor + ";");

    rhymeLabel->setObjectName(person.name + "_rhyme");
    rhymeLabel->setGeometry(width()/2-50, height()/2+65, 200, 20);
    rhymeLabel->hide();
    return rhymeLabel;
}

void MainWindow::SetupPersonAnimations(QLabel* photoLabel, QLabel* nameLabel, QLabel* rhymeLabel, int x, int y)
{
    AnimateWithEffect(photoLabel, QPoint(x, y));
    AnimateWithEffect(nameLabel, QPoint(x, y + 100));
    AnimateWithEffect(rhymeLabel, QPoint(x, y + 115));
}

void MainWindow::AnimateWithEffect(QLabel* label, const QPoint& endPos)
{
    if (label)
    {
        QPropertyAnimation* anim = new QPropertyAnimation(label, "pos");
        anim->setDuration(1000);
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->setEndValue(endPos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::RemoveCurrentPerson()
{
    QString currentName = m_persons[m_currentIndex].name;
    QLabel* photoLabel = findChild<QLabel*>(currentName + "_photo");
    QLabel* nameLabel = findChild<QLabel*>(currentName + "_name");
    QLabel* rhymeLabel = findChild<QLabel*>(currentName + "_rhyme");

    if (photoLabel && nameLabel)
    {
        AnimateRemoval(photoLabel);
        AnimateRemoval(nameLabel);
        if (rhymeLabel) AnimateRemoval(rhymeLabel);
        m_persons.removeAt(m_currentIndex);
        if (m_persons.size() == 1) MoveLastPersonToCenter();
        else if (!m_persons.isEmpty())
        {
            m_currentIndex = m_currentIndex % m_persons.size();
            QTimer::singleShot(1200, this, [this]() {DisplayPhotosInCircle(this->m_persons, false);});
        }
    }
}

void MainWindow::MoveLastPersonToCenter()
{
    QString lastName = m_persons[0].name;
    QLabel* lastPhotoLabel = findChild<QLabel*>(lastName + "_photo");
    QLabel* lastNameLabel = findChild<QLabel*>(lastName + "_name");
    if (lastPhotoLabel && lastNameLabel)
    {
        AnimateToCenter(lastPhotoLabel, QRect(width()/2-50, height()/2-50, 100, 100));
        AnimateToCenter(lastNameLabel, QRect(width()/2-50, height()/2+50, 200, 20));
        DisplayWinnerText(QRect(width()/2-50, height()/2+75, 150, 150), "venok.png");
    }
}

void MainWindow::AnimateToCenter(QLabel* label, const QRect& geometry)
{
    QPropertyAnimation* anim = new QPropertyAnimation(label, "geometry");
    anim->setDuration(800);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setEndValue(geometry);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QString MainWindow::SelectRandomRhyme(const QVector<QString>& rhymes)
{
    if(rhymes.isEmpty()) return "";
    else return rhymes[rand() % rhymes.size()];
}

QString MainWindow::FindLongestWord(const QString& rhyme)
{
    QStringList words = rhyme.split(' ');
    QString longestWord;
    for (const QString& word : words) if (word.length() > longestWord.length()) longestWord = word;
    return longestWord;
}

void MainWindow::OnNextWordButtonClicked()
{
    if (m_persons.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбраны фотографии!");
        return;
    }

    if (m_allRhymes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбран файл со считалочками!");
        return;
    }

    if (m_rhymeRunning) return;

    if (m_rhymeWords.isEmpty()) {
        QString rhyme = SelectRandomRhyme(m_allRhymes);
        if (rhyme.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось выбрать считалочку!");
            return;
        }
        m_rhymeWords = rhyme.split(' ', Qt::SkipEmptyParts);
        if (m_rhymeWords.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Считалочка пуста!");
            return;
        }
        InitializeLongestWordLabel(rhyme);
    }

    disconnect(m_nextWordButton, &QPushButton::clicked, this, &MainWindow::OnNextWordButtonClicked);

    if (!m_longestWordShowed) {
        ShowLongestWordWithAnimation();
    }
    else {
        ShowLongestWordStatic();
    }

    if(m_firstTime){
        connect(m_ui->nextWordButton, &QPushButton::clicked, this, &MainWindow::OnPlayPauseButtonClicked);
        m_ui->nextWordButton->setText("Stop rhyme");
    }
    StartRhyme();
}

void MainWindow::OnPlayPauseButtonClicked()
{
    if (m_rhymeRunning){
        if (m_rhymePaused){
            m_rhymePaused = false;
            m_rhymeTimer->start(750);
            m_ui->nextWordButton->setText("Stop rhyme");
        }else{
            m_rhymePaused = true;
            m_rhymeTimer->stop();
            m_ui->nextWordButton->setText("Start rhyme");
        }
    }
    else{
        if (!m_longestWordShowed) ShowLongestWordWithAnimation();
        else ShowLongestWordStatic();
        StartRhyme();
        m_ui->nextWordButton->setText("Stop rhyme");
    }
}


void MainWindow::ShowLongestWordWithAnimation()
{
    m_longestWordLabel->setGeometry(10, -20, width() -20, 20);
    m_longestWordLabel->show();
    QPropertyAnimation* anim = new QPropertyAnimation(m_longestWordLabel, "pos");
    anim->setDuration(1000);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->setEndValue(QPoint(10, menuBar->height() + 5));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    m_longestWordShowed = true;
}

void MainWindow::ShowLongestWordStatic()
{
    m_longestWordLabel->setGeometry(10, 10, width(), 20);
    m_longestWordLabel->show();
}

void MainWindow::UpdateRhymeWord()
{
    if (m_highlightedLabel) m_highlightedLabel->setStyleSheet("");
    m_currentWordIndex = (m_currentWordIndex + 1) % m_rhymeWords.size();
    m_currentRhymeWordLabel->setText(m_rhymeWords[m_currentWordIndex]);
    UpdateCurrentPerson();
    UpdateRhymeLabel();
    if (m_currentWordIndex == m_rhymeWords.size() - 1)FinishRhyme();
}

void MainWindow::UpdateCurrentPerson()
{
    m_currentIndex = (m_currentIndex + 1) % m_persons.size();
    HighlightCurrentPerson();
}

void MainWindow::UpdateRhymeLabel()
{
    QLabel* rhymeLabel = findChild<QLabel*>(m_persons[m_currentIndex].name + "_rhyme");
    HideAllRhymeLabels();
    if (rhymeLabel)
    {
        rhymeLabel->setText(m_rhymeWords[m_currentWordIndex]);
        rhymeLabel->show();
    }
}

void MainWindow::HideAllRhymeLabels()
{
    for (const Person& person : m_persons)
    {
        QLabel* label = findChild<QLabel*>(person.name + "_rhyme");
        if (label) label->hide();
    }
}

void MainWindow::HighlightCurrentPerson()
{
    if (m_highlightedLabel) {
        m_highlightedLabel->setStyleSheet("");
    }

    m_highlightedLabel = findChild<QLabel*>(m_persons[m_currentIndex].name + "_photo");
    if (m_highlightedLabel) {
        m_highlightedLabel->setStyleSheet(
            "border: 3px solid red;"
            "border-radius: 5px;"
            "background-color: rgba(255, 200, 200, 50);"
            );
    }
}

void MainWindow::AnimateRemoval(QLabel* label)
{
    QPropertyAnimation* animation = new QPropertyAnimation(label, "geometry");
    animation->setDuration(1000);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(label->geometry());
    QPoint endPos = label->pos();
    endPos.setX(width()+200);
    endPos.setY(-label->height()-200);
    QSize size = label->size();
    size.setHeight(height() / 5);
    size.setWidth(width() / 5);
    animation->setEndValue(QRect(endPos, size));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    connect(animation, &QPropertyAnimation::finished,[label]() { label->deleteLater(); });
}

void MainWindow::DisplayWinnerText(QRect target, QString imagePath)
{
    ShowWinnerLabel(target);
    ShowVenokImage(imagePath, target);
    HideLongestWordLabel();
}

void MainWindow::ShowWinnerLabel(QRect target)
{
    QLabel* winnerText = new QLabel("Won!", this);
    winnerText->setFont(QFont("TimesNewRoman", 24, QFont::Bold));

    QString textColor = m_isDarkTheme ? "white" : "black";
    winnerText->setStyleSheet("color: " + textColor + ";");

    winnerText->setAlignment(Qt::AlignLeft);
    winnerText->setGeometry(-200, -200, 100, 100);
    winnerText->show();
    QPropertyAnimation* anim = new QPropertyAnimation(winnerText, "geometry");
    anim->setDuration(800);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->setEndValue(target);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::ShowVenokImage(const QString& imagePath, QRect target)
{
    QLabel* venok = new QLabel(this);
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull())
    {
        venok->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        venok->setGeometry(width()/2 - 50, -100, 150, 150);
        venok->setAlignment(Qt::AlignCenter);
        venok->show();
        QPropertyAnimation* anim = new QPropertyAnimation(venok, "geometry");
        anim->setDuration(800);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        target.setX(width()/2-100);
        target.setY(height()/2 - 350);
        anim->setEndValue(target);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::HideLongestWordLabel()
{
    QPropertyAnimation* anim = new QPropertyAnimation(m_longestWordLabel, "pos");
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCurve);
    anim->setEndValue(QPoint(10, -15));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::StartRhyme()
{
    m_rhymeRunning = true;
    m_rhymePaused = false;
    m_currentWordIndex = 0;
    m_currentRhymeWordLabel->setText(m_rhymeWords[0]);
    m_currentRhymeWordLabel->show();
    HighlightCurrentPerson();
    QLabel* rhymeLabel = findChild<QLabel*>(m_persons[m_currentIndex].name + "_rhyme");
    if (rhymeLabel)
    {
        rhymeLabel->setText(m_rhymeWords[0]);
        rhymeLabel->show();
    }
    m_rhymeTimer->start(750);
}

void MainWindow::FinishRhyme()
{
    m_ui->nextWordButton->setEnabled(false);
    m_rhymeTimer->stop();

    QTimer::singleShot(1000, this, [this]() {
        RemoveCurrentPerson();
        m_currentRhymeWordLabel->hide();
        m_rhymeRunning = false;
        m_rhymePaused = false;
        HideAllRhymeLabels();
        m_ui->nextWordButton->setText("Start rhyme");

        // Проверяем, остался ли только один участник (победитель)
        if (m_persons.size() == 1) {
            // Получаем текущие дату и время
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QTime currentTime = currentDateTime.time();

            // Создаем отчет о победителе
            WinnerReport report;

            // Заполняем дату выполнения считалки
            report.day = static_cast<char>(currentDateTime.date().day());
            report.month = static_cast<char>(currentDateTime.date().month());
            report.year = static_cast<short>(currentDateTime.date().year());

            // Заполняем время выполнения
            report.hours = static_cast<char>(currentTime.hour());
            report.minutes = static_cast<char>(currentTime.minute());
            report.seconds = static_cast<char>(currentTime.second());

            // Заполняем информацию об игре
            report.participantCount = static_cast<short>(m_participants); // Сохраняем исходное количество
            report.startingParticipant = static_cast<char>(startingParticipant); // Сохраняем начальный индекс

            // Заполняем данные победителя
            const Person& winner = m_persons.first();
            report.winnerNumber = static_cast<char>(winner.name.toInt()); // Номер победителя
            strncpy(report.winnerName, winner.name.toLocal8Bit().constData(), 20);

            // Сохраняем текст считалки
            QString fullRhyme = m_rhymeWords.join(" ");
            strncpy(report.rhymeText, fullRhyme.toLocal8Bit().constData(), 240);

            // Отправляем сигнал с отчетом
            emit countingFinished(report);

            // Показываем сообщение о победителе
            QMessageBox::information(this, "Победитель",
                                     QString("Победитель: %1").arg(winner.name));
        }

        // Если участников больше одного, продолжаем игру
        if (m_persons.size() > 1) {
            QTimer::singleShot(1200, this, [this]() {
                OnPlayPauseButtonClicked();
            });
        }
    });
}

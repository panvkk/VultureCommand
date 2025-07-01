#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QTimer>
#include <QPropertyAnimation>
#include <QVector>
#include "person.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void OnNextWordButtonClicked();
    void UpdateRhymeWord();

private:
    // Инициализация компонентов
    void InitializeComponents();
    void SetupCurrentRhymeWordLabel();
    bool InitializeGame(const QVector<QString>& rhymes);
    void ShowErrorMessage();
    void InitializeLongestWordLabel(const QString& rhyme);
    void SetupRhymeTimer();

    // Работа с данными
    QVector<QString> LoadRhymes(const QString& filePath);
    QList<Person> LoadPhotos(const QString& folderPath);
    QString SelectRandomRhyme(const QVector<QString>& rhymes);
    QString FindLongestWord(const QString& rhyme);

    // Отображение и анимация
    void DisplayPhotosInCircle(const QList<Person>& persons, bool firstTime);
    void AnimateExistingWidgets(const QList<Person>& persons,
                                int centerX, int centerY,
                                int radius, int count);
    void AnimatePersonWidgets(const Person& person, int x, int y);
    void AnimateWidget(QLabel* widget, const QPoint& endPos);
    void CreatePersonWidget(const Person& person, int index,
                            int count, int centerX,
                            int centerY, int radius);
    QLabel* CreatePhotoLabel(const Person& person);
    QLabel* CreateNameLabel(const Person& person);
    QLabel* CreateRhymeLabel(const Person& person);
    void SetupPersonAnimations(QLabel* photoLabel,
                               QLabel* nameLabel,
                               QLabel* rhymeLabel,
                               int x, int y);
    void AnimateWithEffect(QLabel* label, const QPoint& endPos);
    void RemoveCurrentPerson();
    void MoveLastPersonToCenter();
    void AnimateToCenter(QLabel* label, const QRect& geometry);
    void HighlightCurrentPerson();
    void AnimateRemoval(QLabel* label);
    void DisplayWinnerText(QRect target, QString imagePath);
    void ShowWinnerLabel(QRect target);
    void ShowVenokImage(const QString& imagePath, QRect target);
    void HideLongestWordLabel();

    // Темы
    bool m_isDarkTheme;
    void applyThemeStyles();
    void updateStylesForTheme(bool isDark);
    void setLabelColors();
    void setButtonStyle();
    bool isDarkTheme() const;

    // Логика игры
    void ShowLongestWordWithAnimation();
    void ShowLongestWordStatic();
    void StartRhyme();
    void UpdateCurrentPerson();
    void UpdateRhymeLabel();
    void HideAllRhymeLabels();
    void FinishRhyme();

    Ui::MainWindow *m_ui;
    QLabel *m_longestWordLabel;
    QLabel *m_currentRhymeWordLabel;
    QPushButton *m_nextWordButton;
    QList<Person> m_persons;
    QStringList m_rhymeWords;
    QTimer *m_rhymeTimer;
    int m_currentIndex;
    int m_currentWordIndex;
    bool m_rhymeRunning;
    bool m_firstTime;
    bool m_longestWordShowed;
    QLabel *m_highlightedLabel;
};

#endif // MAINWINDOW_H

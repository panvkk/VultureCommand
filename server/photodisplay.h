#ifndef PHOTODISPLAY_H
#define PHOTODISPLAY_H

#include <QLabel>

class PhotoDisplay : public QLabel {
    Q_OBJECT
public:
    explicit PhotoDisplay(QWidget* parent = nullptr);

public slots:
    void showPhoto(const QString& surname);
};

#endif // PHOTODISPLAY_H

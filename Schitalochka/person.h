#ifndef PERSON_H
#define PERSON_H

#include <QString>
#include <QPixmap>

class Person {
public:
    QString name;
    QPixmap photo;
    Person(const QString& name, const QPixmap& photo);
};

#endif // PERSON_H

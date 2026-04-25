#ifndef PARSER_H
#define PARSER_H

#include <QVector>
#include <QStringList>
#include <QByteArray>
struct Lesson
{
    int _number;
    QString _name;
    QString _cabinet;
};

class Parser {
public:
    // Загружает XLSX из памяти (QByteArray) и заполняет _rawSchedule
    void loadXLSXFromMemory(const QByteArray& data, int groupIndex);

    // Сохраняет _rawSchedule в XML файл
    void writeXML(const QString& directory, const QString& fileName);

    // Читает XML файл и возвращает расписание (статический метод)
    static QVector<QVector<Lesson*>> readXML(const QString& directory, const QString& fileName, int userSubgroup, int userWeek);

    // Возвращает список групп из XML (статический метод)
    static QStringList groups(const QString& directory, const QString& fileName);

private:
    QVector<QVector<Lesson*>> _rawSchedule;  // 6 дней, внутри — список занятий
};

#endif // PARSER_H
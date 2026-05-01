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
    int _subgroup = 0;
    int _weekParity = 0;
    QString _type;         // "лк", "пр"
};

class Parser {
public:
    void loadXLSXFromMemory(const QByteArray& data, int groupIndex);
    void writeXML(const QString& directory, const QString& fileName);

    static QVector<QVector<Lesson*>> readXML(const QString& directory, const QString& fileName,
                                              int userSubgroup, int userWeek, int groupIndex = 0);
    static QStringList groups(const QString& directory, const QString& fileName);

private:
    QVector<QVector<Lesson*>> _rawSchedule;  // 6 дней
    QVector<QVector<QVector<Lesson*>>> _allGroupsSchedule; // все группы
    QStringList _groups;
};

#endif
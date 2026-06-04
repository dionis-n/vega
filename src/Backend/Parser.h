#ifndef PARSER_H
#define PARSER_H

#include <QVector>
#include <QStringList>
#include <QByteArray>
#include <QSettings>

struct Lesson
{
    int _number;
    QString _name;
    QString _cabinet;
    int _subgroup = 0;
    int _weekParity = 0;
    QString _type;
    QList<int> _weeks;
};

class Parser {
public:
    void loadXLSXFromMemory(const QByteArray& data, int groupIndex);
    // Загрузка и парсинг расписания напрямую из файла на диске.
    // Используется в первую очередь тестами, чтобы прогнать парсер по
    // реальному .xlsx, переданному при запуске (см. src/Tests/test_parser.cpp).
    void loadXLSXFromFile(const QString& path, int groupIndex);
    void writeXML(const QString& directory, const QString& fileName);

    static QVector<QVector<Lesson*>> readXML(const QString& directory, const QString& fileName,
                                              int userSubgroup, int userWeek, int groupIndex = 0);
    static QStringList groups(const QString& directory, const QString& fileName);

    // Доступ к результатам последнего парсинга (для проверок в тестах).
    const QStringList& parsedGroups() const { return _groups; }
    const QVector<QVector<Lesson*>>& parsedSchedule() const { return _rawSchedule; }
    const QVector<QVector<QVector<Lesson*>>>& allGroupsSchedule() const { return _allGroupsSchedule; }

private:
    QVector<QVector<Lesson*>> _rawSchedule;
    QVector<QVector<QVector<Lesson*>>> _allGroupsSchedule;
    QStringList _groups;
};

#endif
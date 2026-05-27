#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#include <QString>
#include <QList>

namespace ParserDetail {

QString extractLessonType(const QString& raw);
QList<int> extractSpecificWeeks(const QString& raw);
QString cleanSubjectName(const QString& raw);

}

#endif

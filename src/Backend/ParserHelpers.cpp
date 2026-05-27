#include "ParserHelpers.h"

#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

namespace ParserDetail {

// Граница (^|\s) и lookahead (?=\s|$) защищают от ложного матча "лк" внутри слов вроде "Балканы".
static const QRegularExpression reLecture(QStringLiteral(R"((^|\s)лк(?=\s|$))"));
static const QRegularExpression reSpecificWeeks(QStringLiteral(R"((\d+(?:,\d+)*)\s*н\.?)"));

QString extractLessonType(const QString& raw) {
    if (raw.contains(reLecture)) return "лк";
    if (!raw.isEmpty()) return "пр";
    return QString();
}

QList<int> extractSpecificWeeks(const QString& raw) {
    QList<int> weeks;
    QRegularExpressionMatchIterator it = reSpecificWeeks.globalMatch(raw);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QStringList parts = match.captured(1).split(",");
        for (const QString& p : parts) {
            bool ok;
            int w = p.trimmed().toInt(&ok);
            if (ok && w >= 1 && w <= 20 && !weeks.contains(w))
                weeks.append(w);
        }
    }
    std::sort(weeks.begin(), weeks.end());
    return weeks;
}

QString cleanSubjectName(const QString& raw) {
    QString cleaned = raw;
    cleaned.remove(QRegularExpression(R"(\s*\([12]пг\)\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*\([12]\*пг\)\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*[III]+\s*н\s*)"));
    cleaned.remove(reLecture);
    cleaned.remove(reSpecificWeeks);
    return cleaned.trimmed();
}

}

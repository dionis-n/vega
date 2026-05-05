#include "Parser.h"
#include <OpenXLSX.hpp>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>
#include <algorithm>

using namespace OpenXLSX;

static const QRegularExpression reLecture(QStringLiteral(R"(\s*лк\s*)"));
static const QRegularExpression reSpecificWeeks(QStringLiteral(R"((\d+(?:,\d+)*)\s*н\.?)"));

static QString extractLessonType(const QString& raw) {
    if (raw.contains(reLecture)) return "лк";
    if (!raw.isEmpty()) return "пр";
    return QString();
}

static QList<int> extractSpecificWeeks(const QString& raw) {
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

static QString getCellString(const XLCell& cell) {
    auto type = cell.value().type();

    if (type == XLValueType::Empty) {
        return QString();
    } else if (type == XLValueType::String) {
        try {
            return QString::fromStdString(cell.value().get<std::string>());
        } catch (...) {
            return QString();
        }
    } else if (type == XLValueType::Integer) {
        try {
            return QString::number(cell.value().get<int64_t>());
        } catch (...) {
            return QString();
        }
    } else if (type == XLValueType::Float) {
        try {
            return QString::number(cell.value().get<double>());
        } catch (...) {
            return QString();
        }
    } else if (type == XLValueType::Boolean) {
        try {
            return cell.value().get<bool>() ? "1" : "0";
        } catch (...) {
            return QString();
        }
    }
    return QString();
}

static QString cleanSubjectName(const QString& raw) {
    QString cleaned = raw;
    cleaned.remove(QRegularExpression(R"(\s*\([12]пг\)\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*\([12]\*пг\)\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*[III]+\s*н\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*лк\s*)"));
    cleaned.remove(reSpecificWeeks);
    return cleaned.trimmed();
}

void Parser::loadXLSXFromMemory(const QByteArray& data, int groupIndex)
{
    _rawSchedule.clear();
    _rawSchedule.resize(6);
    _allGroupsSchedule.clear();
    _groups.clear();

    FILE* f = fopen("/temp_schedule.xlsx", "wb");
    if (!f) { qDebug() << "Ошибка: не удалось создать виртуальный файл"; return; }
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    XLDocument doc;
    doc.open("/temp_schedule.xlsx");
    auto wb = doc.workbook();
    auto ws = wb.worksheet("Занятия");

    static const QRegularExpression hasCyrillic(QStringLiteral("[А-Яа-я]"));
    for (int col = 3; col < 200; col += 3) {
        QString g = getCellString(ws.cell(2, col)).trimmed();
        if (g.isEmpty()) break;
        if (!g.contains(hasCyrillic)) break;
        _groups.append(g);
    }

    if (groupIndex < 0 || groupIndex >= _groups.size()) {
        groupIndex = 0;
    }

    _allGroupsSchedule.resize(_groups.size());
    for (auto& dayVec : _allGroupsSchedule) {
        dayVec.resize(6);
    }

    for (int gIdx = 0; gIdx < _groups.size(); ++gIdx) {
        int groupColumn = 3 + gIdx * 3;

        int currentDay = -1;
        int currentLessonNumber = 0;
        const QStringList dayNames = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};

        static const QRegularExpression reSubgroup1(QStringLiteral(R"(\(1\*?пг\))"));
        static const QRegularExpression reSubgroup2(QStringLiteral(R"(\(2\*?пг\))"));
        static const QRegularExpression reEvenWeek(QStringLiteral(R"((^|\s)IIн)"));
        static const QRegularExpression reOddWeek(QStringLiteral(R"((^|\s)Iн)"));

        for (int rowNum = 3; rowNum <= 100; ++rowNum) {
            QString firstVal = getCellString(ws.cell(rowNum, 1)).trimmed();

            if (!firstVal.isEmpty()) {
                if (firstVal.contains("Легенда")) {
                    break;
                }
                for (int i = 0; i < dayNames.size(); ++i) {
                    if (firstVal == dayNames[i]) {
                        currentDay = i;
                        currentLessonNumber = 0;
                        break;
                    }
                }
            }

            if (currentDay < 0) {
                continue;
            }

            QString numStr = getCellString(ws.cell(rowNum, 2)).trimmed();
            if (!numStr.isEmpty()) {
                int n = numStr.toInt();
                if (n > 0 && n <= 7) {
                    currentLessonNumber = n;
                } else {
                    continue;
                }
            }

            if (currentLessonNumber == 0) {
                continue;
            }

            QString rawSubject = getCellString(ws.cell(rowNum, groupColumn)).trimmed();
            if (rawSubject.isEmpty()) {
                continue;
            }

            if (currentDay == 5 && currentLessonNumber == 6)
                break;

            if (rawSubject.contains("Легенда") || rawSubject.contains("курс") ||
                rawSubject.contains("дистанционно") || rawSubject.contains("кампусе") ||
                rawSubject.contains("Полигон") || rawSubject.contains("ФОК ") ||
                rawSubject.contains("нечетные") || rawSubject.contains("четные") ||
                rawSubject.contains("лекция") || rawSubject.contains("подгруппа")) {
                continue;
            }

            int subgroup = 0;
            if (rawSubject.contains(reSubgroup1))   subgroup = 1;
            else if (rawSubject.contains(reSubgroup2)) subgroup = 2;

            int weekParity = 0;
            if (rawSubject.contains(reEvenWeek))    weekParity = 2;
            else if (rawSubject.contains(reOddWeek)) weekParity = 1;

            QList<int> specificWeeks = extractSpecificWeeks(rawSubject);

            QString lessonType = extractLessonType(rawSubject);

            QString cleanedSubject = cleanSubjectName(rawSubject);
            if (cleanedSubject.isEmpty() || cleanedSubject.length() < 2) {
                continue;
            }

            QString cabinet = getCellString(ws.cell(rowNum, groupColumn + 2)).trimmed();

            Lesson* lesson = new Lesson();
            lesson->_number = currentLessonNumber;
            lesson->_name = cleanedSubject;
            lesson->_cabinet = cabinet;
            lesson->_subgroup = subgroup;
            lesson->_weekParity = weekParity;
            lesson->_type = lessonType;
            lesson->_weeks = specificWeeks;

            _allGroupsSchedule[gIdx][currentDay].append(lesson);
        }
    }

    _rawSchedule = _allGroupsSchedule[groupIndex];

    doc.close();
}

void Parser::writeXML(const QString& directory, const QString& fileNameXML)
{
    QString xmlString;
    QXmlStreamWriter stream(&xmlString);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("schedule");

    stream.writeStartElement("groups");
    for (const QString& g : _groups) {
        stream.writeTextElement("group", g);
    }
    stream.writeEndElement();

    for (int gIdx = 0; gIdx < _allGroupsSchedule.size(); ++gIdx) {
        stream.writeStartElement("groupSchedule");
        stream.writeAttribute("index", QString::number(gIdx));

        for (int dayIndex = 0; dayIndex < _allGroupsSchedule[gIdx].size(); ++dayIndex) {
            stream.writeStartElement("day");
            for (Lesson* lesson : _allGroupsSchedule[gIdx][dayIndex]) {
                stream.writeStartElement("lesson");
                stream.writeAttribute("subgroup", QString::number(lesson->_subgroup));
                stream.writeAttribute("weekParity", QString::number(lesson->_weekParity));
                stream.writeAttribute("type", lesson->_type);
                if (!lesson->_weeks.isEmpty()) {
                    QStringList w;
                    for (int ww : lesson->_weeks) w << QString::number(ww);
                    stream.writeAttribute("weeks", w.join(","));
                }
                stream.writeTextElement("number", QString::number(lesson->_number));
                stream.writeTextElement("name", lesson->_name);
                stream.writeTextElement("cabinet", lesson->_cabinet);
                stream.writeEndElement();
            }
            stream.writeEndElement();
        }

        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndDocument();

    QSettings settings;
    settings.beginGroup("/Schedule");
    settings.setValue("xml", xmlString);
    settings.endGroup();
}

QVector<QVector<Lesson*>> Parser::readXML(const QString& directory, const QString& fileNameXML,
                                           int userSubgroup, int userWeek, int groupIndex)
{
    QVector<QVector<Lesson*>> schedule(6);

    QSettings settings;
    settings.beginGroup("/Schedule");
    QString xmlString = settings.value("xml").toString();
    settings.endGroup();

    if (xmlString.isEmpty()) {
        qDebug() << "ERROR: No schedule data in QSettings";
        return schedule;
    }

    const int userWeekParity = (userWeek % 2 == 1) ? 1 : 2;

    QXmlStreamReader stream(xmlString);
    QString number, name, cabinet, curType;
    int dayIndex = -1;
    int curSubgroup = 0;
    int curWeekParity = 0;
    bool inDay = false;
    int currentGroupIdx = -1;
    QList<int> curWeeks;

    while (!stream.atEnd() && !stream.hasError()) {
        stream.readNext();
        QString token = stream.name().toString();

        if (stream.isStartElement()) {
            if (token == "groupSchedule") {
                currentGroupIdx = stream.attributes().value("index").toInt();
            } else if (token == "day" && currentGroupIdx == groupIndex) {
                ++dayIndex;
                inDay = true;
            } else if (token == "lesson" && inDay && currentGroupIdx == groupIndex) {
                curSubgroup = stream.attributes().value("subgroup").toInt();
                curWeekParity = stream.attributes().value("weekParity").toInt();
                curType = stream.attributes().value("type").toString();
                QString weeksStr = stream.attributes().value("weeks").toString();
                curWeeks.clear();
                if (!weeksStr.isEmpty()) {
                    for (const QString& w : weeksStr.split(","))
                        curWeeks.append(w.toInt());
                }
                number.clear();
                name.clear();
                cabinet.clear();
            } else if (inDay && token == "number" && currentGroupIdx == groupIndex) {
                number = stream.readElementText();
            } else if (inDay && token == "name" && currentGroupIdx == groupIndex) {
                name = stream.readElementText();
            } else if (inDay && token == "cabinet" && currentGroupIdx == groupIndex) {
                cabinet = stream.readElementText();
            }
        } else if (stream.isEndElement()) {
            if (token == "day" && currentGroupIdx == groupIndex) {
                inDay = false;
            } else if (token == "lesson" && dayIndex >= 0 && dayIndex < 6 && currentGroupIdx == groupIndex) {
                bool keep = true;
                if (curSubgroup != 0 && curSubgroup != userSubgroup) keep = false;
                if (curWeekParity != 0 && curWeekParity != userWeekParity) keep = false;
                if (!curWeeks.isEmpty() && !curWeeks.contains(userWeek)) keep = false;

                if (keep) {
                    Lesson* lesson = new Lesson();
                    lesson->_number = number.toInt();
                    lesson->_name = name;
                    lesson->_cabinet = cabinet;
                    lesson->_subgroup = curSubgroup;
                    lesson->_weekParity = curWeekParity;
                    lesson->_type = curType;
                    lesson->_weeks = curWeeks;
                    schedule[dayIndex].append(lesson);
                }
            }
        }
    }
    return schedule;
}

QStringList Parser::groups(const QString& directory, const QString& fileNameXML)
{
    QStringList result;

    QSettings settings;
    settings.beginGroup("/Schedule");
    QString xmlString = settings.value("xml").toString();
    settings.endGroup();

    if (xmlString.isEmpty()) {
        return result;
    }

    QXmlStreamReader stream(xmlString);
    bool inGroups = false;

    while (!stream.atEnd() && !stream.hasError()) {
        stream.readNext();
        QString token = stream.name().toString();

        if (stream.isStartElement()) {
            if (token == "groups") {
                inGroups = true;
            } else if (token == "group" && inGroups) {
                result << stream.readElementText();
            } else if (token == "day") {
                break;
            }
        } else if (stream.isEndElement() && token == "groups") {
            break;
        }
    }
    return result;
}
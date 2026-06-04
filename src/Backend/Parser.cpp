#include "Parser.h"
#include "ParserHelpers.h"
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

void Parser::loadXLSXFromMemory(const QByteArray& data, int groupIndex)
{
    // В WebAssembly нет прямого доступа к файловой системе хоста, поэтому
    // полученные из браузера байты кладём во временный файл виртуальной ФС
    // (MEMFS), а затем переиспользуем общий парсер файла.
    FILE* f = fopen("/temp_schedule.xlsx", "wb");
    if (!f) { qDebug() << "Ошибка: не удалось создать виртуальный файл"; return; }
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    loadXLSXFromFile("/temp_schedule.xlsx", groupIndex);
}

void Parser::loadXLSXFromFile(const QString& path, int groupIndex)
{
    _rawSchedule.clear();
    _rawSchedule.resize(6);
    _allGroupsSchedule.clear();
    _groups.clear();

    XLDocument doc;
    doc.open(path.toStdString());
    auto wb = doc.workbook();
    auto ws = wb.worksheet("Занятия");

    // 1. Динамически ищем первую строку с расписанием (ищем 'ПН')
    int dayStartRow = -1;
    for (int row = 1; row <= 30; ++row) {
        QString val = getCellString(ws.cell(row, 1)).trimmed().toUpper();
        if (val == "ПН" || val == "ПОНЕДЕЛЬНИК") {
            dayStartRow = row;
            break;
        }
    }

    if (dayStartRow == -1) {
        qDebug() << "Ошибка: не найден день 'ПН'. Проверьте формат файла.";
        doc.close();
        return;
    }

    // 2. Ищем строку с группами (обычно она на 1-2 строки выше дней недели)
    int groupRow = -1;
    int groupColStart = -1;
    static const QRegularExpression hasCyrillic(QStringLiteral("[А-Яа-яa-zA-Z]"));

    for (int row = dayStartRow - 1; row >= 1; --row) {
        // Проверяем 2, 3 и 4 колонки, чтобы точно зацепить начало
        for (int col = 2; col <= 5; ++col) {
            QString val = getCellString(ws.cell(row, col)).trimmed();
            if (!val.isEmpty() && val.contains(hasCyrillic) && !val.contains("РАСПИСАНИЕ", Qt::CaseInsensitive)) {
                groupRow = row;
                groupColStart = col;
                break;
            }
        }
        if (groupRow != -1) break;
    }

    if (groupRow == -1 || groupColStart == -1) {
        qDebug() << "Ошибка: не найдена строка с группами.";
        doc.close();
        return;
    }

    // 3. Парсим группы
    for (int col = groupColStart; col < 200; col += 3) {
        QString g = getCellString(ws.cell(groupRow, col)).trimmed();
        if (g.isEmpty() || !g.contains(hasCyrillic)) break;
        _groups.append(g);
    }

    // КРИТИЧЕСКАЯ ЗАЩИТА: если групп нет, прерываем, иначе будет краш index out of range
    if (_groups.isEmpty()) {
        qDebug() << "Ошибка: список групп пуст. Парсинг прерван.";
        doc.close();
        return;
    }

    if (groupIndex < 0 || groupIndex >= _groups.size()) {
        groupIndex = 0;
    }

    _allGroupsSchedule.resize(_groups.size());
    for (auto& dayVec : _allGroupsSchedule) {
        dayVec.resize(6);
    }

    // 4. Основной цикл парсинга расписания
    for (int gIdx = 0; gIdx < _groups.size(); ++gIdx) {
        int groupColumn = groupColStart + gIdx * 3;

        int currentDay = -1;
        int currentLessonNumber = 0;
        const QStringList dayNames = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};

        static const QRegularExpression reSubgroup1(QStringLiteral(R"(\(1\*?пг\))"));
        static const QRegularExpression reSubgroup2(QStringLiteral(R"(\(2\*?пг\))"));
        static const QRegularExpression reEvenWeek(QStringLiteral(R"((^|\s)IIн)"));
        static const QRegularExpression reOddWeek(QStringLiteral(R"((^|\s)Iн)"));

        for (int rowNum = dayStartRow; rowNum <= 200; ++rowNum) {
            QString firstVal = getCellString(ws.cell(rowNum, 1)).trimmed();

            if (!firstVal.isEmpty()) {
                if (firstVal.contains("Легенда", Qt::CaseInsensitive)) {
                    break;
                }
                for (int i = 0; i < dayNames.size(); ++i) {
                    if (firstVal.toUpper() == dayNames[i]) {
                        currentDay = i;
                        currentLessonNumber = 0;
                        break;
                    }
                }
            }

            if (currentDay < 0 || currentDay > 5) {
                continue;
            }

            QString numStr = getCellString(ws.cell(rowNum, 2)).trimmed();
            if (!numStr.isEmpty()) {
                int n = numStr.toInt();
                if (n > 0 && n <= 8) {
                    currentLessonNumber = n;
                } else if (n == 0 && numStr != "0") {
                    continue; // Защита от мусора в колонке с номерами пар
                }
            }

            if (currentDay == 5 && currentLessonNumber == 6)
                break;

            if (currentLessonNumber == 0) {
                continue;
            }

            QString rawSubject = getCellString(ws.cell(rowNum, groupColumn)).trimmed();
            if (rawSubject.isEmpty()) {
                continue;
            }

            if (rawSubject.contains("Легенда", Qt::CaseInsensitive) || rawSubject.contains("курс") ||
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

            QList<int> specificWeeks = ParserDetail::extractSpecificWeeks(rawSubject);
            QString lessonType = ParserDetail::extractLessonType(rawSubject);
            QString cleanedSubject = ParserDetail::cleanSubjectName(rawSubject);

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
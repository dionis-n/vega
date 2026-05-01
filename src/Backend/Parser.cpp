#include "Parser.h"
#include <OpenXLSX.hpp>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QRegularExpression>

using namespace OpenXLSX;

// 🔧 Безопасное получение строки из ячейки
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

// Вспомогательная функция для очистки названия предмета
static QString cleanSubjectName(const QString& raw) {
    QString cleaned = raw;
    // Удаляем маркеры подгрупп
    cleaned.remove(QRegularExpression(R"(\s*\([12]пг\)\s*)"));
    cleaned.remove(QRegularExpression(R"(\s*\([12]\*пг\)\s*)"));
    // Удаляем маркеры недель
    cleaned.remove(QRegularExpression(R"(\s*[III]+\s*н\s*)"));
    // Удаляем маркеры типов занятий
    cleaned.remove(QRegularExpression(R"(\s*лк\s*)"));
    return cleaned.trimmed();
}

void Parser::loadXLSXFromMemory(const QByteArray& data, int groupIndex)
{
    qDebug() << "=== loadXLSXFromMemory START ===";
    qDebug() << "Data size:" << data.size() << "groupIndex:" << groupIndex;

    _rawSchedule.clear();
    _rawSchedule.resize(6);
    _allGroupsSchedule.clear(); // Новое поле: вектор для всех групп
    _groups.clear();

    FILE* f = fopen("/temp_schedule.xlsx", "wb");
    if (!f) { qDebug() << "Ошибка: не удалось создать виртуальный файл"; return; }
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    XLDocument doc;
    doc.open("/temp_schedule.xlsx");
    auto wb = doc.workbook();
    auto ws = wb.worksheet("Занятия");

    // Сбор списка групп из строки 2
    static const QRegularExpression hasCyrillic(QStringLiteral("[А-Яа-я]"));
    for (int col = 3; col < 200; col += 3) {
        QString g = getCellString(ws.cell(2, col)).trimmed();
        if (g.isEmpty()) break;
        if (!g.contains(hasCyrillic)) break;
        _groups.append(g);
    }
    qDebug() << "Найдено групп:" << _groups.size() << _groups;

    if (groupIndex < 0 || groupIndex >= _groups.size()) {
        qDebug() << "Внимание: groupIndex вне диапазона, использую 0";
        groupIndex = 0;
    }

    // Инициализируем хранилище для всех групп
    _allGroupsSchedule.resize(_groups.size());
    for (auto& dayVec : _allGroupsSchedule) {
        dayVec.resize(6);
    }

    // Парсим расписание для КАЖДОЙ группы
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
            if (rawSubject.contains(reSubgroup1))      subgroup = 1;
            else if (rawSubject.contains(reSubgroup2)) subgroup = 2;

            int weekParity = 0;
            if (rawSubject.contains(reEvenWeek))     weekParity = 2;
            else if (rawSubject.contains(reOddWeek)) weekParity = 1;

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

            _allGroupsSchedule[gIdx][currentDay].append(lesson);
        }
    }

    // Копируем расписание выбранной группы в _rawSchedule для обратной совместимости
    _rawSchedule = _allGroupsSchedule[groupIndex];

    doc.close();
    qDebug() << "=== loadXLSXFromMemory FINISH ===";
    for (int i = 0; i < _rawSchedule.size(); ++i) {
        qDebug() << "День" << i << "занятий:" << _rawSchedule[i].size();
    }
}

void Parser::writeXML(const QString& directory, const QString& fileNameXML)
{
    qDebug() << "=== writeXML START ===";
    QDir dir(directory);
    if (!dir.exists() && !dir.mkpath(".")) {
        qDebug() << "Ошибка создания директории:" << directory;
        return;
    }

    QFile file(directory + "/" + fileNameXML);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open XML for writing:" << fileNameXML;
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("schedule");

    stream.writeStartElement("groups");
    for (const QString& g : _groups) {
        stream.writeTextElement("group", g);
    }
    stream.writeEndElement();

    // Сохраняем расписание для каждой группы
    for (int gIdx = 0; gIdx < _allGroupsSchedule.size(); ++gIdx) {
        stream.writeStartElement("groupSchedule");
        stream.writeAttribute("index", QString::number(gIdx));

        for (int dayIndex = 0; dayIndex < _allGroupsSchedule[gIdx].size(); ++dayIndex) {
            stream.writeStartElement("day");
            for (Lesson* lesson : _allGroupsSchedule[gIdx][dayIndex]) {
                stream.writeStartElement("lesson");
                stream.writeAttribute("subgroup", QString::number(lesson->_subgroup));
                stream.writeAttribute("weekParity", QString::number(lesson->_weekParity));
                stream.writeTextElement("number", QString::number(lesson->_number));
                stream.writeTextElement("name", lesson->_name);
                stream.writeTextElement("cabinet", lesson->_cabinet);
                stream.writeEndElement();
            }
            stream.writeEndElement();
        }

        stream.writeEndElement(); // groupSchedule
    }

    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
    qDebug() << "=== writeXML FINISH ===";
}

QVector<QVector<Lesson*>> Parser::readXML(const QString& directory, const QString& fileNameXML,
                                           int userSubgroup, int userWeek, int groupIndex)
{
    QVector<QVector<Lesson*>> schedule(6);
    QFile file(directory + "/" + fileNameXML);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Cannot open XML file for reading";
        return schedule;
    }

    const int userWeekParity = (userWeek % 2 == 1) ? 1 : 2;

    QXmlStreamReader stream(&file);
    QString number, name, cabinet;
    int dayIndex = -1;
    int curSubgroup = 0;
    int curWeekParity = 0;
    bool inDay = false;
    int currentGroupIdx = -1;

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

                if (keep) {
                    Lesson* lesson = new Lesson();
                    lesson->_number = number.toInt();
                    lesson->_name = name;
                    lesson->_cabinet = cabinet;
                    lesson->_subgroup = curSubgroup;
                    lesson->_weekParity = curWeekParity;
                    schedule[dayIndex].append(lesson);
                }
            }
        }
    }
    file.close();
    return schedule;
}

QStringList Parser::groups(const QString& directory, const QString& fileNameXML)
{
    QStringList result;
    QFile file(directory + "/" + fileNameXML);
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }

    QXmlStreamReader stream(&file);
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
                break; // список групп уже считан, дальше — расписание
            }
        } else if (stream.isEndElement() && token == "groups") {
            break;
        }
    }
    file.close();
    return result;
}
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

    FILE* f = fopen("/temp_schedule.xlsx", "wb");
    if (!f) { qDebug() << "Ошибка: не удалось создать виртуальный файл"; return; }
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    XLDocument doc;
    doc.open("/temp_schedule.xlsx");
    auto wb = doc.workbook();
    auto ws = wb.worksheet("Занятия");

    int groupColumn = 3 + groupIndex * 2;
    qDebug() << "groupColumn:" << groupColumn;

    int currentDay = -1;
    int currentLessonNumber = 0;
    QStringList dayNames = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};

    for (int rowNum = 3; rowNum <= 100; ++rowNum) {
        QString firstVal = getCellString(ws.cell(rowNum, 1)).trimmed();

        // Определение дня недели
        if (!firstVal.isEmpty()) {
            for (int i = 0; i < dayNames.size(); ++i) {
                if (firstVal == dayNames[i]) {
                    currentDay = i;
                    currentLessonNumber = 0;
                    qDebug() << "День установлен:" << dayNames[i];
                    break;
                }
            }
        }

        if (currentDay < 0) {
            continue;
        }


        // Определение номера пары
        QString numStr = getCellString(ws.cell(rowNum, 2)).trimmed();
        if (!numStr.isEmpty()) {
            int n = numStr.toInt();
            if (n > 0 && n <= 7) {
                currentLessonNumber = n;
            } else {
                continue;
            }
        }

        if (currentDay == 5 && currentLessonNumber == 6)
        {
            qDebug() << "Конец субботы";
            break; // После субботы не парсим
        }

        if (currentLessonNumber == 0) {
            continue;
        }

        // Чтение предмета и аудитории
        QString subject = getCellString(ws.cell(rowNum, groupColumn)).trimmed();
        if (subject.isEmpty()) {
            continue;
        }

        // ✅ Пропускаем легенду и мусор
        if (subject.contains("Легенда") || subject.contains("курс") ||
            subject.contains("дистанционно") || subject.contains("кампусе") ||
            subject.contains("Полигон") || subject.contains("ФОК") ||
            subject.contains("нечетные") || subject.contains("четные") ||
            subject.contains("лекция") || subject.contains("подгруппа")) {
            continue;
        }

        QString cleanedSubject = cleanSubjectName(subject);
        if (cleanedSubject.isEmpty() || cleanedSubject.length() < 2) {
            continue;
        }

        QString cabinet = getCellString(ws.cell(rowNum, groupColumn + 2)).trimmed();

        Lesson* lesson = new Lesson();
        lesson->_number = currentLessonNumber;
        lesson->_name = cleanedSubject;
        lesson->_cabinet = cabinet;

        _rawSchedule[currentDay].append(lesson);
        qDebug() << "  Добавлено: день" << currentDay
                 << "пара" << lesson->_number
                 << "предмет:" << lesson->_name
                 << "кабинет:" << lesson->_cabinet;
    }

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

    for (int dayIndex = 0; dayIndex < _rawSchedule.size(); ++dayIndex) {
        stream.writeStartElement("day");
        for (Lesson* lesson : _rawSchedule[dayIndex]) {
            stream.writeStartElement("lesson");
            stream.writeTextElement("number", QString::number(lesson->_number));
            stream.writeTextElement("name", lesson->_name);
            stream.writeTextElement("cabinet", lesson->_cabinet);
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
    qDebug() << "=== writeXML FINISH ===";
}

QVector<QVector<Lesson*>> Parser::readXML(const QString& directory, const QString& fileNameXML, int userSubgroup, int userWeek)
{
    Q_UNUSED(userSubgroup);
    Q_UNUSED(userWeek);

    QVector<QVector<Lesson*>> schedule(6);
    QFile file(directory + "/" + fileNameXML);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Cannot open XML file for reading";
        return schedule;
    }

    QXmlStreamReader stream(&file);
    QString number, name, cabinet;
    int index = -1;

    while (!stream.atEnd() && !stream.hasError()) {
        stream.readNext();
        QString token = stream.name().toString();

        if (stream.isStartElement()) {
            if (token == "day") {
                ++index;
            } else if (token == "number") {
                number = stream.readElementText();
            } else if (token == "name") {
                name = stream.readElementText();
            } else if (token == "cabinet") {
                cabinet = stream.readElementText();
            }
        } else if (stream.isEndElement() && token == "lesson") {
            if (index >= 0 && index < 6) {
                Lesson* lesson = new Lesson();
                lesson->_number = number.toInt();
                lesson->_name = name;
                lesson->_cabinet = cabinet;
                schedule[index].append(lesson);
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
        result << "Группа 1" << "Группа 2" << "Группа 3";
        return result;
    }
    result << "Группа 1" << "Группа 2" << "Группа 3";
    file.close();
    return result;
}
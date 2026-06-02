#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QList>
#include <QFile>
#include <iostream>

#include "Backend/ParserHelpers.h"
#include "Backend/Parser.h"

using ParserDetail::extractLessonType;
using ParserDetail::extractSpecificWeeks;
using ParserDetail::cleanSubjectName;

// Путь к .xlsx-файлу расписания, переданному при запуске теста.
// Заполняется в main() из аргумента --schedule=<файл> или из переменной
// окружения VEGA_SCHEDULE_FILE. Если пуст — файловый тест пропускается.
static QString g_scheduleFile;

TEST(ParserHelpersTest, ExtractLessonType)
{
    EXPECT_EQ(extractLessonType("лк Математика"), "лк");
    EXPECT_EQ(extractLessonType("Физика"), "пр");
    EXPECT_EQ(extractLessonType(""), "");
    EXPECT_EQ(extractLessonType("   лк   История"), "лк");
    EXPECT_EQ(extractLessonType("Математика лк"), "лк");
    EXPECT_EQ(extractLessonType("Балканы в средние века"), "пр");
    EXPECT_EQ(extractLessonType("Вулканология"), "пр");
}

TEST(ParserHelpersTest, CleanSubjectName)
{
    EXPECT_EQ(cleanSubjectName("Программирование (1пг)"), "Программирование");
    EXPECT_EQ(cleanSubjectName("Программирование (2пг)"), "Программирование");
    EXPECT_EQ(cleanSubjectName("Iн Математика"), "Математика");
    EXPECT_EQ(cleanSubjectName("IIн Физика"), "Физика");
    EXPECT_EQ(cleanSubjectName("лк История"), "История");
    EXPECT_EQ(cleanSubjectName("IIн лк Информатика (1пг)"), "Информатика");
    EXPECT_EQ(cleanSubjectName("Iн Программирование (2*пг)"), "Программирование");
    EXPECT_EQ(cleanSubjectName("1,3,5н лк Физика"), "Физика");
    EXPECT_EQ(cleanSubjectName("2,6,10,14н Экономика"), "Экономика");
    EXPECT_EQ(cleanSubjectName("   Базы данных   (1пг)   "), "Базы данных");
    EXPECT_EQ(cleanSubjectName("лк Балканы в средние века"), "Балканы в средние века");
}

TEST(ParserHelpersTest, ExtractSpecificWeeks)
{
    EXPECT_EQ(extractSpecificWeeks("2,6,10,14н Экономика"), QList<int>({2, 6, 10, 14}));
    EXPECT_EQ(extractSpecificWeeks("10,2,8н"),              QList<int>({2, 8, 10}));
    EXPECT_EQ(extractSpecificWeeks("0,1,21,20н"),           QList<int>({1, 20}));
    EXPECT_EQ(extractSpecificWeeks("5,5,5н"),               QList<int>({5}));
    EXPECT_EQ(extractSpecificWeeks("Iн 2,4н лк Физика 6,8н"), QList<int>({2, 4, 6, 8}));
    EXPECT_TRUE(extractSpecificWeeks("Просто предмет без недель").isEmpty());
}

// Прогон парсера по реальному .xlsx-расписанию, заданному при запуске теста.
// Без файла (например, в обычном CI) тест помечается как пропущенный, не падает.
TEST(ScheduleFileTest, ParsesProvidedSchedule)
{
    if (g_scheduleFile.isEmpty())
    {
        GTEST_SKIP() << "Файл расписания не задан. Передайте --schedule=<файл.xlsx> "
                        "или переменную окружения VEGA_SCHEDULE_FILE, чтобы прогнать "
                        "тест по конкретному расписанию.";
    }

    ASSERT_TRUE(QFile::exists(g_scheduleFile))
        << "Файл не найден: " << g_scheduleFile.toStdString();

    Parser parser;
    parser.loadXLSXFromFile(g_scheduleFile, 0);

    const QStringList& groups = parser.parsedGroups();
    ASSERT_FALSE(groups.isEmpty())
        << "Не удалось разобрать ни одной группы. Проверьте, что в файле есть лист "
           "'Занятия' и колонка с днями недели ('ПН').";

    const QVector<QVector<QVector<Lesson*>>>& allGroups = parser.allGroupsSchedule();
    ASSERT_EQ(allGroups.size(), groups.size());

    const QStringList validTypes = {"лк", "пр", ""};

    int totalLessons = 0;
    for (int g = 0; g < allGroups.size(); ++g)
    {
        for (int day = 0; day < allGroups[g].size(); ++day)
        {
            for (const Lesson* lesson : allGroups[g][day])
            {
                ++totalLessons;

                EXPECT_TRUE(validTypes.contains(lesson->_type))
                    << "Недопустимый тип пары '" << lesson->_type.toStdString()
                    << "' у предмета '" << lesson->_name.toStdString() << "'";

                EXPECT_GE(lesson->_name.size(), 2)
                    << "Слишком короткое название пары: '"
                    << lesson->_name.toStdString() << "'";

                EXPECT_GE(lesson->_number, 1) << "Номер пары вне диапазона 1..8";
                EXPECT_LE(lesson->_number, 8) << "Номер пары вне диапазона 1..8";

                for (int week : lesson->_weeks)
                {
                    EXPECT_GE(week, 1)  << "Номер недели вне диапазона 1..20";
                    EXPECT_LE(week, 20) << "Номер недели вне диапазона 1..20";
                }
            }
        }
    }

    EXPECT_GT(totalLessons, 0)
        << "Группы найдены, но не разобрано ни одной пары.";

    // Краткая сводка по разобранному файлу — удобно глазами проверить результат.
    std::cout << "\n=== Разбор файла: " << g_scheduleFile.toStdString() << " ===\n";
    std::cout << "Групп найдено: " << groups.size() << "\n";
    for (const QString& gr : groups)
        std::cout << "  - " << gr.toStdString() << "\n";
    std::cout << "Всего пар (по всем группам): " << totalLessons << "\n";
    std::cout << std::flush;
}

// Разбор аргумента --schedule=<файл> / --schedule <файл> и переменной окружения.
static void parseScheduleArg(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg.startsWith("--schedule="))
        {
            g_scheduleFile = arg.mid(QStringLiteral("--schedule=").size());
        }
        else if (arg == "--schedule" && i + 1 < argc)
        {
            g_scheduleFile = QString::fromLocal8Bit(argv[++i]);
        }
    }

    if (g_scheduleFile.isEmpty())
    {
        g_scheduleFile = qEnvironmentVariable("VEGA_SCHEDULE_FILE");
    }
}

// Свой main: под Node.js вывод теряется без явного flush перед exit.
int main(int argc, char **argv) {
    std::cout << "[Vega] Инициализация тестов..." << std::endl;

    parseScheduleArg(argc, argv);
    if (!g_scheduleFile.isEmpty())
        std::cout << "[Vega] Файл расписания для теста: "
                  << g_scheduleFile.toStdString() << std::endl;

    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cout << std::flush;
    std::cerr << std::flush;
    fflush(stdout);
    fflush(stderr);
    return result;
}

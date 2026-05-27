#include <gtest/gtest.h>
#include <QString>
#include <QList>
#include <iostream>

#include "Backend/ParserHelpers.h"

using ParserDetail::extractLessonType;
using ParserDetail::extractSpecificWeeks;
using ParserDetail::cleanSubjectName;

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

// Свой main: под Node.js вывод теряется без явного flush перед exit.
int main(int argc, char **argv) {
    std::cout << "[WASM] Инициализация тестов..." << std::endl;
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cout << std::flush;
    std::cerr << std::flush;
    fflush(stdout);
    fflush(stderr);
    return result;
}

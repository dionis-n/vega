#ifndef WIDGET_H
#define WIDGET_H

#include "Backend/Parser.h"
#include "CustomElements.h"
#include "CommonFunctions.h"

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QResizeEvent>
#include <QSizePolicy>

#include <array>

enum
{
    NONE = -1, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY
};


class ScheduleTab : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleTab(const QVector<QVector<Lesson*>>& schedule, bool showEmptyLessons, QWidget* parent = nullptr);
    ~ScheduleTab();

private:
    QHBoxLayout* createDayBarLayout();
    QHBoxLayout* createWideLayout();
    QVBoxLayout* createNarrowLayout();
    QWidget* createLessonRow(Lesson* lesson);
    int getWeekdayNumber();

    QVBoxLayout* _scheduleTabLayout;

    bool _wideMode;
    bool _showEmptyLessons;
    int _currentWeekday = NONE;

    std::array<QString, 6> _dayName = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб"};
    std::array<QString, 7> _lessonTime = {" 9:00\n10:30", "10:40\n12:10", "12:40\n14:10", "14:20\n15:50",
                                          "16:20\n17:50", "18:00\n19:30", "19:40\n21:00"};
    QVector<QVector<Lesson*>> _schedule;

protected:
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void slotDayButtonClicked();
};


#endif
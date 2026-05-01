#include "ScheduleTab.h"

ScheduleTab::ScheduleTab(const QVector<QVector<Lesson*>>& schedule, bool showEmptyLessons, QWidget* parent)
    : QWidget(parent), _schedule(schedule), _showEmptyLessons(showEmptyLessons), _scheduleTabLayout( new QVBoxLayout() )
{
    _scheduleTabLayout->addLayout( createDayBarLayout() );

    if ( checkOrientation(parent) )
    {
        _wideMode = true;
        _scheduleTabLayout->addLayout( createWideLayout() );
    }
    else
    {
        _wideMode = false;
        _scheduleTabLayout->addLayout( createNarrowLayout() );
    }

    setLayout(_scheduleTabLayout);
}

ScheduleTab::~ScheduleTab()
{
    delete _scheduleTabLayout;

    for (auto&& day : _schedule)
    {
        for (auto&& lesson : day)
        {
            delete lesson;
        }
    }
}

QHBoxLayout* ScheduleTab::createDayBarLayout() {
    QHBoxLayout* dayBarLayout = new QHBoxLayout();

    for (int i = 0; i < _dayName.size(); ++i)
    {
        dayButton* button = new dayButton(_dayName[i], i);

        dayBarLayout->addWidget(button);
        dayBarLayout->setAlignment(button, Qt::AlignHCenter);

        connect(button, SIGNAL( clicked() ), SLOT( slotDayButtonClicked() ));
    }

    return dayBarLayout;
}

QWidget* ScheduleTab::createLessonRow(Lesson* lesson)
{
    scheduleLabel* row = new scheduleLabel(QString());
    row->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    row->setMinimumHeight(40);

    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(5, 2, 5, 2);
    rowLayout->setSpacing(0);
    rowLayout->setAlignment(Qt::AlignVCenter);

    // Время
    QString time;
    if (_wideMode)
    {
        time = QString::number(lesson->_number) + ". ";
    }
    else
    {
        time = _lessonTime[lesson->_number - 1];
    }
    QLabel* timeLabel = new QLabel(time.trimmed());
    timeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Название
    QLabel* nameLabel = new QLabel(lesson->_name);
    nameLabel->setWordWrap(true);

    // Кружочек
    lessonTypeBadge* badge = nullptr;
    if (!lesson->_type.isEmpty())
    {
        badge = new lessonTypeBadge(lesson->_type);
    }

    // Контейнер: кружочек + название + пружина
    QWidget* nameContainer = new QWidget();
    QHBoxLayout* nameLayout = new QHBoxLayout(nameContainer);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(0);
    nameLayout->setAlignment(Qt::AlignVCenter);
    if (badge)
    {
        nameLayout->addWidget(badge);
        nameLayout->addSpacing(10);
    }
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();

    // Кабинет
    QLabel* cabinetLabel = new QLabel(lesson->_cabinet);
    cabinetLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    rowLayout->addWidget(timeLabel);
    rowLayout->addSpacing(10);
    rowLayout->addWidget(nameContainer, 1);
    rowLayout->addSpacing(8);
    rowLayout->addWidget(cabinetLabel);

    return row;
}

QHBoxLayout* ScheduleTab::createWideLayout()
{
    int weekday = getWeekdayNumber();

    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setChecked(true);
    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setDisabled(true);

    _wideMode = true;

    QHBoxLayout* wideLayout = new QHBoxLayout();

    for (int i = 0; i < _dayName.size(); ++i)
    {
        int lastNumber = 1;

        QVBoxLayout* columnLayout = new QVBoxLayout();
        columnLayout->setAlignment(Qt::AlignTop);

        for (auto&& lesson : _schedule[i])
        {
            if (_showEmptyLessons)
            {
                while (lastNumber != lesson->_number)
                {
                    Lesson emptyLesson;
                    emptyLesson._number = lastNumber;
                    QWidget* row = createLessonRow(&emptyLesson);
                    columnLayout->addWidget(row);
                    ++lastNumber;
                }
            }

            QWidget* row = createLessonRow(lesson);
            columnLayout->addWidget(row);
            ++lastNumber;
        }

        // Растяжка для пустого места внизу колонки
        columnLayout->addStretch();

        QWidget* columnWidget = new QWidget();
        columnWidget->setLayout(columnLayout);

        wideLayout->addWidget(columnWidget, 1);  // stretch = 1 — все колонки поровну
    }

    return wideLayout;
}

QVBoxLayout* ScheduleTab::createNarrowLayout()
{
    int weekday = getWeekdayNumber();
    int lastNumber = 1;

    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setChecked(true);
    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setDisabled(true);

    _wideMode = false;

    QVBoxLayout* narrowLayout = new QVBoxLayout();
    narrowLayout->setAlignment(Qt::AlignTop);

    for (auto&& lesson : _schedule[weekday])
    {
        if (_showEmptyLessons)
        {
            while (lastNumber != lesson->_number)
            {
                Lesson emptyLesson;
                emptyLesson._number = lastNumber;
                QWidget* row = createLessonRow(&emptyLesson);
                narrowLayout->addWidget(row);
                ++lastNumber;
            }
        }

        QWidget* row = createLessonRow(lesson);
        narrowLayout->addWidget(row);
        ++lastNumber;
    }

    if (_schedule[weekday].length() == 0)
    {
        QLabel* label = new QLabel("Пар нет");
        narrowLayout->addWidget(label);
        narrowLayout->setAlignment(Qt::AlignCenter);
    }

    return narrowLayout;
}

int ScheduleTab::getWeekdayNumber()
{
    QDateTime time;
    int weekday = time.currentDateTime().date().dayOfWeek() - 1;

    if (weekday == SUNDAY || weekday == NONE)
    {
        weekday = MONDAY;
    }

    if (_currentWeekday != NONE)
    {
        weekday = _currentWeekday;
    }
    else
    {
        _currentWeekday = weekday;
    }

    return weekday;
}

void ScheduleTab::resizeEvent(QResizeEvent* event) {
    if ( checkOrientation(this) )
    {
        if (_wideMode) return;

        // Удаляем narrow layout
        QLayout* oldLayout = _scheduleTabLayout->itemAt(1)->layout();
        if (oldLayout)
        {
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }
            oldLayout->deleteLater();
        }

        _scheduleTabLayout->addLayout( createWideLayout() );
        _wideMode = true;
    }
    else
    {
        if (!_wideMode) return;

        // Удаляем wide layout
        QLayout* oldLayout = _scheduleTabLayout->itemAt(1)->layout();
        if (oldLayout)
        {
            // Удаляем все колонки (QWidget + внутренний QVBoxLayout)
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                {
                    QWidget* column = item->widget();
                    QLayout* innerLayout = column->layout();
                    if (innerLayout)
                    {
                        QLayoutItem* innerItem;
                        while ((innerItem = innerLayout->takeAt(0)) != nullptr)
                        {
                            if (innerItem->widget())
                                innerItem->widget()->deleteLater();
                            delete innerItem;
                        }
                    }
                    column->deleteLater();
                }
                delete item;
            }
            oldLayout->deleteLater();
        }

        _scheduleTabLayout->addLayout( createNarrowLayout() );
        _wideMode = false;
    }
}

void ScheduleTab::slotDayButtonClicked()
{
    int weekday = qobject_cast<dayButton*>( sender() )->_weekday;

    if (weekday == _currentWeekday)
    {
        return;
    }

    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(_currentWeekday)->widget() )
        ->setChecked(false);
    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(_currentWeekday)->widget() )
        ->setDisabled(false);

    _currentWeekday = weekday;

    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setChecked(true);
    qobject_cast<dayButton*>( _scheduleTabLayout->itemAt(0)->layout()->itemAt(weekday)->widget() )
        ->setDisabled(true);

    if (_wideMode)
    {
        return;
    }

    QVBoxLayout* narrowLayout = qobject_cast<QVBoxLayout*>( _scheduleTabLayout->itemAt(1)->layout() );
    narrowLayout->setAlignment(Qt::AlignTop);

    while ( !narrowLayout->isEmpty() )
    {
        narrowLayout->takeAt(0)->widget()->deleteLater();
    }

    int lastNumber = 1;

    for (auto&& lesson : _schedule[weekday])
    {
        if (_showEmptyLessons)
        {
            while (lastNumber != lesson->_number)
            {
                Lesson emptyLesson;
                emptyLesson._number = lastNumber;
                QWidget* row = createLessonRow(&emptyLesson);
                narrowLayout->addWidget(row);
                ++lastNumber;
            }
        }

        QWidget* row = createLessonRow(lesson);
        narrowLayout->addWidget(row);
        ++lastNumber;
    }

    if (_schedule[weekday].length() == 0)
    {
        QLabel* label = new QLabel("Пар нет");
        narrowLayout->addWidget(label);
        narrowLayout->setAlignment(Qt::AlignCenter);
    }
}
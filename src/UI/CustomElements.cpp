#include "CustomElements.h"

scheduleLabel::scheduleLabel(const QString& str, QWidget* parent) : QLabel(str, parent)
{}


dayButton::dayButton(const QString& str, int weekday, QWidget* parent) : QPushButton(str, parent),
    _weekday(weekday)
{
    this->setCheckable(true);
}


scheduleButton::scheduleButton(QWidget* parent) : QPushButton(parent)
{
    setCheckable(true);
}


settingsButton::settingsButton(QWidget* parent) : QPushButton(parent)
{
    setCheckable(true);
}

lessonTypeBadge::lessonTypeBadge(const QString& type, QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);

    if (type == "лк") {
        setText("лк");
        setStyleSheet("background-color:#2196F3;color:white;font-size:11px;font-weight:bold;");
    } else if (type == "пр") {
        setText("пр");
        setStyleSheet("background-color:#4CAF50;color:white;font-size:11px;font-weight:bold;");
    }
}
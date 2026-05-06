#include "CommonFunctions.h"

bool checkOrientation(const QWidget* widget)
{
    return widget->size().width() > 700;
}
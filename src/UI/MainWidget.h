#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ScheduleTab.h"
#include "SettingsTab.h"
#include "CustomElements.h"
#include "CommonFunctions.h"
#include "Backend/Parser.h"

#include <QStandardPaths>
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QPushButton>

enum
{
    SCHEDULE_TAB_INDEX = 0,
    SETTINGS_TAB_INDEX = 1,

    START_UP_THEME,
    LIGHT_THEME,
    DARK_THEME,

    THEME_SYSTEM = 0,
    THEME_LIGHT = 1,
    THEME_DARK = 2,

    MAX_WEEK_NUMBER = 20,

    EXTRA_SIZE = 120
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

    void setTheme(int themeIndex);
    int getCurrentTheme() const { return _currentTheme; }

    int getGroupIndex() const { return _groupIndex; }
    QString getStandardPath() const { return _standardPath; }
    void switchToSchedule();

private:
    QHBoxLayout* createTabBarLayout();
    void appConfig();
    void saveSettingsFromTab();
    void calulateCurrentWeekNumber();
    void applyTheme();
    void updateTabBarButtons();

    QVBoxLayout* _mainLayout;

    int _currentTabIndex;
    int _appTheme = START_UP_THEME;
    int _currentTheme = THEME_SYSTEM;
    QTimer* _timer;
    bool _showEmptyLessons;

    QSettings _settings;
    QString _standardPath;
    int _groupIndex;
    int _subgroup;

    QDate _date;
    int _week;
    int _currentWeekNumber;

    QString _fileNameXLSX = "Schedule.xlsx";
    QString _fileNameXML = "Data.xml";

    scheduleButton* _scheduleBtn = nullptr;
    settingsButton* _settingsBtn = nullptr;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void slotScheduleButtonClicked();
    void slotSettingsButtonClicked();
    void slotCheckSystemTheme();
};

#endif // MAINWIDGET_H
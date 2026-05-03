#include "MainWidget.h"
#include <QFileDialog>
#include <QDebug>
#include <QDir>

MainWidget::MainWidget(QWidget* parent) : QWidget(parent), _mainLayout(new QVBoxLayout())
{
    appConfig();

    /*if (_showEmptyLessons)
    {
        this->setFixedHeight(this->height() + EXTRA_SIZE);
    }
    else
    {
        this->setFixedHeight(this->height());
    }*/

    slotCheckSystemTheme();

    QStringList groups = Parser::groups(_standardPath, _fileNameXML);

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(slotCheckSystemTheme()));
    _timer->start(100);

    if (!_settings.contains("/Settings/groupIndex"))
    {
        QStringList groups = Parser::groups(_standardPath, _fileNameXML);
        SettingsTab* settingsTab = new SettingsTab(groups, _groupIndex, _subgroup,
                                                   _currentWeekNumber, MAX_WEEK_NUMBER, _showEmptyLessons, this);
        _mainLayout->addWidget(settingsTab);
        _currentTabIndex = SETTINGS_TAB_INDEX;
    }
    else
    {
        ScheduleTab* scheduleTab = new ScheduleTab(
            Parser::readXML(_standardPath, _fileNameXML, _subgroup, _currentWeekNumber, _groupIndex),
            _showEmptyLessons, this);
        _mainLayout->addWidget(scheduleTab);
        _currentTabIndex = SCHEDULE_TAB_INDEX;
    }

    if (_currentTheme != THEME_SYSTEM) {
        setTheme(_currentTheme);
    }

    qobject_cast<QVBoxLayout*>(_mainLayout)->addLayout(createTabBarLayout());
    setLayout(_mainLayout);
}

MainWidget::~MainWidget()
{
    delete _mainLayout;
    delete _timer;
}

QHBoxLayout* MainWidget::createTabBarLayout()
{
    QHBoxLayout* tabBarLayout = new QHBoxLayout();

    _scheduleBtn = new scheduleButton();
    connect(_scheduleBtn, SIGNAL(clicked()), SLOT(slotScheduleButtonClicked()));

    _settingsBtn = new settingsButton();
    connect(_settingsBtn, SIGNAL(clicked()), SLOT(slotSettingsButtonClicked()));

    tabBarLayout->addWidget(_scheduleBtn);
    tabBarLayout->addWidget(_settingsBtn);

    updateTabBarButtons();

    return tabBarLayout;
}

void MainWidget::updateTabBarButtons()
{
    if (!_scheduleBtn || !_settingsBtn)
        return;

    if (_currentTabIndex == SCHEDULE_TAB_INDEX)
    {
        _scheduleBtn->setChecked(true);
        _scheduleBtn->setDisabled(true);
        _settingsBtn->setChecked(false);
        _settingsBtn->setDisabled(false);
    }
    else if (_currentTabIndex == SETTINGS_TAB_INDEX)
    {
        _settingsBtn->setChecked(true);
        _settingsBtn->setDisabled(true);
        _scheduleBtn->setChecked(false);
        _scheduleBtn->setDisabled(false);
    }
}

void MainWidget::appConfig()
{
    _standardPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(_standardPath);

    if (!dir.exists())
    {
        if (!dir.mkpath(_standardPath))
        {
            return;
        }
    }

    _settings.beginGroup("/Settings");

    _groupIndex = _settings.value("groupIndex", 0).toInt();
    _subgroup = _settings.value("subgroup", 1).toInt();
    _date = _settings.value("date", QDateTime::currentDateTime().date()).toDate();
    _week = _settings.value("week", 1).toInt();
    _showEmptyLessons = _settings.value("showEmptyLessons", false).toBool();
    _currentTheme = _settings.value("theme", THEME_SYSTEM).toInt();


    this->move(_settings.value("position", this->pos()).toPoint());
    this->resize(_settings.value("geometry", this->size()).toSize());
    this->resize(this->size());

    _settings.endGroup();

    calulateCurrentWeekNumber();
}

void MainWidget::saveSettingsFromTab()
{
    SettingsTab* settings = qobject_cast<SettingsTab*>(_mainLayout->itemAt(0)->widget());

    _groupIndex = settings->getGroupIndex();
    _subgroup = settings->getSubgroup();
    _week = settings->getWeek();
    _showEmptyLessons = settings->getShowEmptyLessons();

    _settings.beginGroup("/Settings");
    _settings.setValue("groupIndex", _groupIndex);
    _settings.setValue("subgroup", _subgroup);
    _settings.setValue("week", _week);
    _settings.setValue("showEmptyLessons", _showEmptyLessons);
    _settings.setValue("date", _date);
    _settings.endGroup();

    calulateCurrentWeekNumber();
}

void MainWidget::calulateCurrentWeekNumber()
{
    QDate currentDate = QDateTime::currentDateTime().date();

    _currentWeekNumber = currentDate.weekNumber() - _date.weekNumber() + _week;

    if (_currentWeekNumber < 1 || _currentWeekNumber > MAX_WEEK_NUMBER)
    {
        _currentWeekNumber = MAX_WEEK_NUMBER;
    }
}

void MainWidget::closeEvent(QCloseEvent* event)
{
    if (_currentTabIndex == SETTINGS_TAB_INDEX)
    {
        this->saveSettingsFromTab();
    }

    _settings.beginGroup("/Settings");

    _settings.setValue("groupIndex", _groupIndex);
    _settings.setValue("subgroup", _subgroup);
    _settings.setValue("showEmptyLessons", _showEmptyLessons);
    _settings.setValue("date", _date);
    _settings.setValue("week", _week);
    _settings.setValue("theme", _currentTheme);
    _settings.setValue("position", this->pos());

    if (_showEmptyLessons)
    {
        //this->setFixedHeight(this->height() - EXTRA_SIZE);
        _settings.setValue("geometry", this->size());
    }
    else
    {
        _settings.setValue("geometry", this->size());
    }

    _settings.endGroup();
}

void MainWidget::slotScheduleButtonClicked()
{
    if (_currentTabIndex == SCHEDULE_TAB_INDEX) {
        return;
    }

    switchToSchedule();
}

void MainWidget::slotCheckSystemTheme()
{
    if (_currentTheme != THEME_SYSTEM) {
        return;
    }

    QColor backgroundColor = QWidget::palette().color(QWidget::backgroundRole());


    int brightness = (backgroundColor.red() + backgroundColor.green() + backgroundColor.blue()) / 3;


    bool force = (_appTheme == START_UP_THEME);

    if (brightness < 128 && (force || _appTheme == LIGHT_THEME))
    {
        QFile style(":/dark_theme.qss");
        if (style.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(style.readAll());
            _appTheme = DARK_THEME;
        }
    }
    else if (brightness >= 128 && (force || _appTheme == DARK_THEME))
    {
        QFile style(":/light_theme.qss");
        if (style.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(style.readAll());
            _appTheme = LIGHT_THEME;
        }
    }
}

void MainWidget::setTheme(int themeIndex)
{
    _currentTheme = themeIndex;

    switch (themeIndex) {
    case THEME_LIGHT: {
        QFile lightStyle(":/light_theme.qss");
        if (lightStyle.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(lightStyle.readAll());
            _appTheme = LIGHT_THEME;
        }
        break;
    }
    case THEME_DARK: {
        QFile darkStyle(":/dark_theme.qss");
        if (darkStyle.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(darkStyle.readAll());
            _appTheme = DARK_THEME;
        }
        break;
    }
    case THEME_SYSTEM:
    default: {
        qApp->setStyleSheet("");
        _appTheme = START_UP_THEME;
        slotCheckSystemTheme();
        break;
    }
    }
}

void MainWidget::switchToSchedule()
{
    if (_currentTabIndex == SCHEDULE_TAB_INDEX) return;
    saveSettingsFromTab();

    QWidget* oldWidget = _mainLayout->itemAt(0)->widget();
    if (oldWidget) {
        _mainLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }

    // Передаём groupIndex при чтении
    auto schedule = Parser::readXML(_standardPath, _fileNameXML, _subgroup, _currentWeekNumber, _groupIndex);
    ScheduleTab* scheduleTab = new ScheduleTab(schedule, _showEmptyLessons, this);
    _mainLayout->insertWidget(0, scheduleTab);

    _currentTabIndex = SCHEDULE_TAB_INDEX;
    updateTabBarButtons();
}

void MainWidget::slotSettingsButtonClicked()
{
    if (_currentTabIndex == SETTINGS_TAB_INDEX) return;

    QWidget* oldWidget = _mainLayout->itemAt(0)->widget();
    if (oldWidget) {
        _mainLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }

    QStringList groups = Parser::groups(_standardPath, _fileNameXML);
    bool showGroups = !groups.isEmpty();
    SettingsTab* settingsTab = new SettingsTab(groups, _groupIndex, _subgroup, _currentWeekNumber,
                                               MAX_WEEK_NUMBER, _showEmptyLessons, this, showGroups);

    _mainLayout->insertWidget(0, settingsTab);

    _currentTabIndex = SETTINGS_TAB_INDEX;
    updateTabBarButtons();
}

void MainWidget::refreshSettingsTab()
{
    saveSettingsFromTab();
    _currentTabIndex = -1;
    slotSettingsButtonClicked();
}

void MainWidget::saveSetting(const QString& key, int value)
{
    _settings.beginGroup("/Settings");
    _settings.setValue(key, value);
    _settings.endGroup();
}
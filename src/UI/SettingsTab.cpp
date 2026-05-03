#include "SettingsTab.h"
#include "MainWidget.h"
#include <QPushButton>
#include <QSettings>

SettingsTab::SettingsTab(const QStringList& groups, int groupIndex, int subgroup, int week, int maxWeekNumber,
                         bool showEmptyLessons, QWidget* parent, bool showGroups) : QWidget(parent), _showEmptyLessons(showEmptyLessons)
{
    QFormLayout* settingsLayout = new QFormLayout();

    QComboBox* groupComboBox = new QComboBox(this);
    QComboBox* subgroupComboBox = new QComboBox(this);
    QComboBox* weekComboBox = new QComboBox(this);
    QComboBox* showEmptyLessonsComboBox = new QComboBox(this);
    QComboBox* themeComboBox = new QComboBox(this);

    connect(groupComboBox, SIGNAL( currentIndexChanged(int) ), SLOT( slotGroup(int) ));
    connect(subgroupComboBox, SIGNAL( currentIndexChanged(int) ), SLOT( slotSubgroup(int) ));
    connect(weekComboBox, SIGNAL( currentIndexChanged(int) ), SLOT( slotWeek(int) ));
    connect(showEmptyLessonsComboBox, SIGNAL( currentIndexChanged(int) ), SLOT( slotShowEmptyLessons(int) ));
    connect(themeComboBox, SIGNAL( currentIndexChanged(int) ), SLOT( onThemeChanged(int) ));

    QPushButton* loadButton = new QPushButton("Загрузить расписание (.XLSX)");
    connect(loadButton, &QPushButton::clicked, this, &SettingsTab::onLoadFileClicked);
    settingsLayout->addRow(loadButton);



    _statusLabel = new QLabel("");
    _statusLabel->setVisible(false);
    _statusLabel->setAlignment(Qt::AlignCenter);
    settingsLayout->addRow(_statusLabel);

    QSettings settings;
    settings.beginGroup("/Settings");
    if (settings.contains("statusMessage")) {
        QString msg = settings.value("statusMessage").toString();
        bool success = settings.value("statusSuccess", false).toBool();
        _statusLabel->setText(msg);
        _statusLabel->setStyleSheet(success ? "color: green; font-weight: bold;" : "color: red; font-weight: bold;");
        _statusLabel->setVisible(true);
    } else {
        // Проверяем, есть ли расписание
        QStringList existingGroups = Parser::groups("", "");
        if (existingGroups.isEmpty()) {
            _statusLabel->setText("Файл не выбран");
            _statusLabel->setStyleSheet("color: red; font-weight: bold;");
            _statusLabel->setVisible(true);
        } else {
            _statusLabel->setText("Расписание загружено");
            _statusLabel->setStyleSheet("color: green; font-weight: bold;");
            _statusLabel->setVisible(true);
        }
    }
    settings.endGroup();

    groupComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    subgroupComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    weekComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    showEmptyLessonsComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    themeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    loadButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    groupComboBox->addItems(groups);

    groupComboBox->setEnabled(showGroups && !groups.isEmpty());

    subgroupComboBox->addItems( {"1", "2"} );
    showEmptyLessonsComboBox->addItems( {"Нет", "Да"} );
    themeComboBox->addItems( {"Системная", "Светлая", "Тёмная"} );

    for (int i = 1; i < maxWeekNumber + 1; ++i)
    {
        weekComboBox->addItem( QString::number(i) );
    }

    groupComboBox->setCurrentIndex(groupIndex);
    subgroupComboBox->setCurrentIndex(subgroup - 1);
    weekComboBox->setCurrentIndex(week - 1);
    showEmptyLessonsComboBox->setCurrentIndex( int(showEmptyLessons) );

    MainWidget* mainWindow = qobject_cast<MainWidget*>(parent);
    int savedTheme = mainWindow ? mainWindow->getCurrentTheme() : 0;
    themeComboBox->blockSignals(true);
    themeComboBox->setCurrentIndex(savedTheme);
    themeComboBox->blockSignals(false);

    groupComboBox->setMaxVisibleItems(COUNT_MAX_VISIBLE_ITEMS);
    weekComboBox->setMaxVisibleItems(COUNT_MAX_VISIBLE_ITEMS);

    settingsLayout->addRow("Группа:", groupComboBox);
    settingsLayout->addRow("Подгруппа:", subgroupComboBox);
    settingsLayout->addRow("Неделя:", weekComboBox);
    settingsLayout->addRow("Показывать пустые пары:", showEmptyLessonsComboBox);
    settingsLayout->addRow("Тема:", themeComboBox);

    setLayout(settingsLayout);
}

int SettingsTab::getGroupIndex() const
{
    return _groupIndex;
}

int SettingsTab::getSubgroup() const
{
    return _subgroup;
}

int SettingsTab::getWeek() const
{
    return _week;
}

bool SettingsTab::getShowEmptyLessons() const
{
    return _showEmptyLessons;
}

void SettingsTab::slotGroup(int index)
{
    _groupIndex = index;
    MainWidget* mw = qobject_cast<MainWidget*>(parentWidget());
    if (mw) mw->saveSetting("groupIndex", index);
}

void SettingsTab::slotSubgroup(int index)
{
    _subgroup = index + 1;
    MainWidget* mw = qobject_cast<MainWidget*>(parentWidget());
    if (mw) mw->saveSetting("subgroup", index + 1);
}

void SettingsTab::slotWeek(int index)
{
    _week = index + 1;
    MainWidget* mw = qobject_cast<MainWidget*>(parentWidget());
    if (mw) mw->saveSetting("week", index + 1);
}

void SettingsTab::slotShowEmptyLessons(int index)
{
    _showEmptyLessons = index;
    MainWidget* mw = qobject_cast<MainWidget*>(parentWidget());
    if (mw) mw->saveSetting("showEmptyLessons", index);
}

void SettingsTab::onThemeChanged(int index)
{
    MainWidget* mainWindow = qobject_cast<MainWidget*>(parentWidget());
    if (mainWindow) {
        mainWindow->setTheme(index);
    }
}

void SettingsTab::onLoadFileClicked()
{
    MainWidget* mainWindow = qobject_cast<MainWidget*>(parentWidget());
    if (!mainWindow) return;

    int currentGroupIndex = _groupIndex;
    _statusLabel->setVisible(false);

    QFileDialog::getOpenFileContent(
        "Excel files (*.xlsx)",
        [this, mainWindow, currentGroupIndex](const QString &fileName, const QByteArray &fileContent) {
            if (fileName.isEmpty()) {
                _statusLabel->setText("Файл не выбран");
                _statusLabel->setStyleSheet("color: red; font-weight: bold;");
                _statusLabel->setVisible(true);

                QSettings settings;
                settings.beginGroup("/Settings");
                settings.setValue("statusMessage", "Файл не выбран");
                settings.setValue("statusSuccess", false);
                settings.endGroup();
                return;
            }

            Parser parser;
            parser.loadXLSXFromMemory(fileContent, currentGroupIndex);
            parser.writeXML(mainWindow->getStandardPath(), "Data.xml");

            _statusLabel->setText("Расписание загружено");
            _statusLabel->setStyleSheet("color: green; font-weight: bold;");
            _statusLabel->setVisible(true);

            QSettings settings;
            settings.beginGroup("/Settings");
            settings.setValue("statusMessage", "Расписание загружено");
            settings.setValue("statusSuccess", true);
            settings.endGroup();

            settings.beginGroup("/Settings");
            settings.setValue("groupIndex", currentGroupIndex);
            settings.endGroup();

            mainWindow->refreshSettingsTab();
        }
        );
}
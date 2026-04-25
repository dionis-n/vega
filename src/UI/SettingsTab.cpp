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
    settingsLayout->addRow("", loadButton);

    groupComboBox->addItems(groups);
    groupComboBox->setDisabled(showGroups);

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

    // Загружаем сохранённую тему
    QSettings settings;
    settings.beginGroup("/Settings");
    int savedTheme = settings.value("theme", 0).toInt();
    settings.endGroup();
    themeComboBox->setCurrentIndex(savedTheme);

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
}

void SettingsTab::slotSubgroup(int index)
{
    _subgroup = index + 1;
}

void SettingsTab::slotWeek(int index)
{
    _week = index + 1;
}

void SettingsTab::slotShowEmptyLessons(int index)
{
    _showEmptyLessons = index;
}

void SettingsTab::onThemeChanged(int index)
{
    MainWidget* mainWindow = qobject_cast<MainWidget*>(parentWidget());
    if (mainWindow) {
        mainWindow->setTheme(index);
    }

    // Сохраняем настройку темы
    QSettings settings;
    settings.beginGroup("/Settings");
    settings.setValue("theme", index);
    settings.endGroup();
}

void SettingsTab::onLoadFileClicked()
{
    MainWidget* mainWindow = qobject_cast<MainWidget*>(parentWidget());
    if (!mainWindow) return;

    QFileDialog::getOpenFileContent(
        "Excel files (*.xlsx)",
        [mainWindow](const QString &fileName, const QByteArray &fileContent) {
            if (fileName.isEmpty()) {
                qDebug() << "Файл не выбран";
                return;
            }

            qDebug() << "Загружен файл:" << fileName << "Размер:" << fileContent.size() << "байт";

            Parser parser;
            parser.loadXLSXFromMemory(fileContent, mainWindow->getGroupIndex());
            parser.writeXML(mainWindow->getStandardPath(), "Data.xml");

            mainWindow->switchToSchedule();
        }
        );
}
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "mainform.h"

#include "defs.h"
#include "tcontext.h"
#include "solution.h"

#include "stringutils.h"

#include <iostream>
using namespace std;


#include "centerwidgetonscreen.h"

#include "timetablegenerateform.h"
//#include "timetablegeneratemultipleform.h"

#include "statisticsprintform.h"

#include "export.h"
#include "import.h"

#include "institutionnameform.h"
#include "commentsform.h"
#include "daysform.h"
#include "hoursform.h"
#include "subjectsform.h"
#include "subjectsstatisticsform.h"
#include "activitytagsform.h"
#include "teachersform.h"
#include "teachersstatisticsform.h"
#include "yearsform.h"
#include "splityearform.h"
#include "groupsform.h"
#include "subgroupsform.h"
#include "studentsstatisticsform.h"
#include "activitiesform.h"
#include "alltimeconstraintsform.h"
#include "helpaboutform.h"
#include "helpfaqform.h"
#include "helptipsform.h"
#include "helpinstructionsform.h"

#include "m-fet.h"

#include "main_functions.h"

#include "constraintactivityendsstudentsdayform.h"
#include "constraintactivitiesendstudentsdayform.h"
#include "constrainttwoactivitiesconsecutiveform.h"
#include "constrainttwoactivitiesgroupedform.h"
#include "constraintthreeactivitiesgroupedform.h"
#include "constrainttwoactivitiesorderedform.h"
#include "constraintactivitiespreferredtimeslotsform.h"
#include "constraintactivitiespreferredstartingtimesform.h"

#include "constraintsubactivitiespreferredtimeslotsform.h"
#include "constraintsubactivitiespreferredstartingtimesform.h"

#include "constraintactivitiessamestartingtimeform.h"
#include "constraintactivitiessamestartinghourform.h"
#include "constraintactivitiessamestartingdayform.h"

#include "constraintactivitiesoccupymaxtimeslotsfromselectionform.h"
#include "constraintactivitiesmaxsimultaneousinselectedtimeslotsform.h"

#include "constraintteachernotavailabletimesform.h"
#include "constraintstudentssetnotavailabletimesform.h"
#include "constraintbreaktimesform.h"
#include "constraintteachermaxdaysperweekform.h"
#include "constraintteachersmaxdaysperweekform.h"

#include "constraintteachermindaysperweekform.h"
#include "constraintteachersmindaysperweekform.h"

#include "constraintteacherintervalmaxdaysperweekform.h"
#include "constraintteachersintervalmaxdaysperweekform.h"
#include "constraintstudentssetintervalmaxdaysperweekform.h"
#include "constraintstudentsintervalmaxdaysperweekform.h"

#include "constraintstudentssetmaxdaysperweekform.h"
#include "constraintstudentsmaxdaysperweekform.h"

#include "constraintteachermaxhoursdailyform.h"
#include "constraintteachersmaxhoursdailyform.h"
#include "constraintteachermaxhourscontinuouslyform.h"
#include "constraintteachersmaxhourscontinuouslyform.h"

//#include "constraintteacheractivitytagmaxhourscontinuouslyform.h"
//#include "constraintteachersactivitytagmaxhourscontinuouslyform.h"

//#include "constraintteacheractivitytagmaxhoursdailyform.h"
//#include "constraintteachersactivitytagmaxhoursdailyform.h"

#include "constraintteachersmaxhoursinworkdailyform.h"
#include "constraintteachermaxhoursinworkdailyform.h"
#include "constraintmaxactivitiesforallteachersform.h"
#include "constraintmaxactivitiesforteacherform.h"
#include "constraintminactivitiesforallteachersform.h"
#include "constraintminactivitiesforteacherform.h"

#include "constraintteacherminhoursdailyform.h"
#include "constraintteachersminhoursdailyform.h"
#include "constraintactivitypreferredstartingtimeform.h"
#include "constraintstudentssetmaxgapsperweekform.h"
#include "constraintstudentsmaxgapsperweekform.h"

#include "constraintstudentssetmaxgapsperdayform.h"
#include "constraintstudentsmaxgapsperdayform.h"

#include "constraintteachersmaxgapsperweekform.h"
#include "constraintteachermaxgapsperweekform.h"
#include "constraintteachersmaxgapsperdayform.h"
#include "constraintteachermaxgapsperdayform.h"
#include "constraintstudentsearlymaxbeginningsatsecondhourform.h"
#include "constraintstudentssetearlymaxbeginningsatsecondhourform.h"
#include "constraintstudentssetmaxhoursdailyform.h"
#include "constraintstudentsmaxhoursdailyform.h"
#include "constraintstudentssetmaxhourscontinuouslyform.h"
#include "constraintstudentsmaxhourscontinuouslyform.h"

#include "constraintnoteacherinsubjectpreferenceform.h"

//#include "constraintstudentssetactivitytagmaxhourscontinuouslyform.h"
//#include "constraintstudentsactivitytagmaxhourscontinuouslyform.h"

//#include "constraintstudentssetactivitytagmaxhoursdailyform.h"
//#include "constraintstudentsactivitytagmaxhoursdailyform.h"

#include "constraintstudentssetminhoursdailyform.h"
#include "constraintstudentsminhoursdailyform.h"
#include "constraintactivitiesnotoverlappingform.h"
#include "constraintmindaysbetweenactivitiesform.h"
#include "constraintmaxdaysbetweenactivitiesform.h"
#include "constraintmingapsbetweenactivitiesform.h"
#include "constraintactivitypreferredtimeslotsform.h"
#include "constraintactivitypreferredstartingtimesform.h"

#include "constraintteachermaxspanperdayform.h"
#include "constraintteachersmaxspanperdayform.h"
#include "constraintstudentssetmaxspanperdayform.h"
#include "constraintstudentsmaxspanperdayform.h"

#include "constraintteacherminrestinghoursform.h"
#include "constraintteachersminrestinghoursform.h"
#include "constraintstudentssetminrestinghoursform.h"
#include "constraintstudentsminrestinghoursform.h"

#include "timetablegeneratesettingsform.h"

#include "settingstimetablehtmllevelform.h"

#include "timetableshowsolutionsform.h"

//#include "spreadconfirmationform.h"

#include "removeredundantconfirmationform.h"
#include "removeredundantform.h"

#include "savetimetableconfirmationform.h"

#include "timetablestowriteondiskform.h"

#include "lockunlock.h"
#include "advancedlockunlockform.h"

#include "textmessages.h"

#include <QMessageBox>

#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QTranslator>
#include <QCloseEvent>
#include <QStatusBar>
#include <QMap>
#include <QWidget>
#include <QList>
#include <QHash>

#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QMenu>
#include <QCursor>
#include <QSettings>

#include <QFileInfo>
#include <QFile>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QRegExp>

//for the icons of not perfect constraints
#include <QIcon>

#include <QRect>

QRect mainFormSettingsRect;
int MAIN_FORM_SHORTCUTS_TAB_POSITION;

//#include "spreadmindaysconstraintsfivedaysform.h"

#include "statisticsexport.h"

extern QApplication* pqapplication;

static int ORIGINAL_WIDTH;
static int ORIGINAL_HEIGHT;
//static int ORIGINAL_X;
//static int ORIGINAL_Y;

bool SHOW_SHORTCUTS_ON_MAIN_WINDOW=true;

const int STATUS_BAR_MILLISECONDS=2500;

MainForm::MainForm()
{
	setupUi(this);

    QApplication::setPalette(this->palette());

	QIcon appIcon(":/images/appicon.png");
	pqapplication->setWindowIcon(appIcon);

    QSettings settings(defs::COMPANY, defs::PROGRAM);
	int nRec=settings.value(QString("MainForm/number-of-recent-files"), 0).toInt();
	if(nRec>MAX_RECENT_FILES)
		nRec=MAX_RECENT_FILES;
	recentFiles.clear();
	for(int i=0; i<nRec; i++)
        if(settings.contains(QString("MainForm/recent-file/")+utils::strings::number(i+1)))
            recentFiles.append(settings.value(QString("MainForm/recent-file/")+utils::strings::number(i+1)).toString());

	recentSeparatorAction=fileOpenRecentMenu->insertSeparator(fileClearRecentFilesListAction);
	for(int i=0; i<MAX_RECENT_FILES; i++){
		recentFileActions[i]=new QAction(this);
		recentFileActions[i]->setVisible(false);
		connect(recentFileActions[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));

		fileOpenRecentMenu->insertAction(recentSeparatorAction, recentFileActions[i]);
	}

	updateRecentFileActions();

	//statusBar()->showMessage(tr("m-FET started", "This is a message written in the status bar, saying that m-FET was started"), STATUS_BAR_MILLISECONDS);
	statusBar()->showMessage("", STATUS_BAR_MILLISECONDS); //to get the correct centralWidget for the logo, so we need status bar existing.

	defs::INPUT_FILENAME_XML=QString("");
	setCurrentFile(defs::INPUT_FILENAME_XML);

	//toolBox->setCurrentIndex(0);
	tabWidget->setVisible(SHOW_SHORTCUTS_ON_MAIN_WINDOW);
	tabWidget->setCurrentIndex(MAIN_FORM_SHORTCUTS_TAB_POSITION);

	shortcutBasicMenu=new QMenu();
	shortcutBasicMenu->addMenu(menuInstitution_information);
	shortcutBasicMenu->addSeparator(); //added on 19 Dec. 2009
	shortcutBasicMenu->addMenu(menuDays_and_hours);

	shortcutAdvancedTimeMenu=new QMenu();
    //shortcutAdvancedTimeMenu->addAction(spreadActivitiesAction);
    //shortcutAdvancedTimeMenu->addSeparator();
	shortcutAdvancedTimeMenu->addAction(removeRedundantConstraintsAction);
	//shortcutAdvancedTimeMenu->addSeparator();
	//shortcutAdvancedTimeMenu->addAction(groupActivitiesInInitialOrderAction);

	shortcutDataSpaceMenu=new QMenu();
	shortcutDataSpaceMenu->addAction(dataBuildingsAction);
	shortcutDataSpaceMenu->addSeparator();
	shortcutDataSpaceMenu->addAction(dataRoomsAction);

	shortcutDataAdvancedMenu=new QMenu();
	shortcutDataAdvancedMenu->addAction(activityPlanningAction);
	shortcutDataAdvancedMenu->addSeparator();
	shortcutDataAdvancedMenu->addAction(dataTeachersStatisticsAction);
	shortcutDataAdvancedMenu->addAction(dataSubjectsStatisticsAction);
	shortcutDataAdvancedMenu->addAction(dataStudentsStatisticsAction);

	//2014-07-01
	shortcutTimetableLockingMenu=new QMenu();
	shortcutTimetableLockingMenu->addAction(timetableLockAllActivitiesAction);
	shortcutTimetableLockingMenu->addAction(timetableUnlockAllActivitiesAction);
	shortcutTimetableLockingMenu->addSeparator();
	shortcutTimetableLockingMenu->addAction(timetableLockActivitiesDayAction);
	shortcutTimetableLockingMenu->addAction(timetableUnlockActivitiesDayAction);
	shortcutTimetableLockingMenu->addSeparator();
	shortcutTimetableLockingMenu->addAction(timetableLockActivitiesEndStudentsDayAction);
	shortcutTimetableLockingMenu->addAction(timetableUnlockActivitiesEndStudentsDayAction);

	shortcutTimetableAdvancedMenu=new QMenu();
	shortcutTimetableAdvancedMenu->addAction(groupActivitiesInInitialOrderAction);

	ORIGINAL_WIDTH=width();
	ORIGINAL_HEIGHT=height();
	//ORIGINAL_X=x();
	//ORIGINAL_Y=y();

	QRect rect=mainFormSettingsRect;
	if(!rect.isValid()){
		forceCenterWidgetOnScreen(this);
	}
	else{
		//this->setWindowFlags(this->windowFlags() | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

		/*resize(rect.size());
		move(rect.topLeft());*/
		this->setGeometry(rect);
	}

	//new data
    if(TContext::get()->instance.initialized)
        TContext::get()->instance.kill();
    TContext::get()->instance.init();

    TContext::get()->instance.setModified(true); //to avoid flicker of the main form modified flag

    bool tmp=TContext::get()->instance.addTimeConstraint(new ConstraintBasicCompulsoryTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100));
	assert(tmp);

    TContext::get()->instance.setModified(true); //force update of the modified flag of the main window
    setRulesUnmodifiedAndOtherThings(&TContext::get()->instance);

    TContext::get()->resetSchedule();

	settingsShowShortcutsOnMainWindowAction->setCheckable(true);
	settingsShowShortcutsOnMainWindowAction->setChecked(SHOW_SHORTCUTS_ON_MAIN_WINDOW);

	settingsShowToolTipsForConstraintsWithTablesAction->setCheckable(true);
	settingsShowToolTipsForConstraintsWithTablesAction->setChecked(defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES);

	checkForUpdatesAction->setCheckable(true);
	checkForUpdatesAction->setChecked(defs::checkForUpdates);

	settingsUseColorsAction->setCheckable(true);
	settingsUseColorsAction->setChecked(defs::USE_GUI_COLORS);

	settingsShowSubgroupsInComboBoxesAction->setCheckable(true);
	settingsShowSubgroupsInComboBoxesAction->setChecked(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES);
	settingsShowSubgroupsInActivityPlanningAction->setCheckable(true);
	settingsShowSubgroupsInActivityPlanningAction->setChecked(defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING);

	////////confirmations
	settingsConfirmActivityPlanningAction->setCheckable(true);
	settingsConfirmActivityPlanningAction->setChecked(defs::CONFIRM_ACTIVITY_PLANNING);

	settingsConfirmSpreadActivitiesAction->setCheckable(true);
	settingsConfirmSpreadActivitiesAction->setChecked(defs::CONFIRM_SPREAD_ACTIVITIES);

	settingsConfirmRemoveRedundantAction->setCheckable(true);
	settingsConfirmRemoveRedundantAction->setChecked(defs::CONFIRM_REMOVE_REDUNDANT);

	settingsConfirmSaveTimetableAction->setCheckable(true);
	settingsConfirmSaveTimetableAction->setChecked(defs::CONFIRM_SAVE_TIMETABLE);
	///////

	settingsDivideTimetablesByDaysAction->setCheckable(true);
	settingsDivideTimetablesByDaysAction->setChecked(defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS);

	settingsDuplicateVerticalNamesAction->setCheckable(true);
	settingsDuplicateVerticalNamesAction->setChecked(defs::TIMETABLE_HTML_REPEAT_NAMES);

//	if(defs::checkForUpdates){
//		networkManager=new QNetworkAccessManager(this);
//		connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
//		QUrl url("http://lalescu.ro/liviu/fet/crtversion/crtversion.txt");
//		if(defs::VERBOSE){
//			cout<<"New version checking host: "<<qPrintable(url.host())<<endl;
//			cout<<"New version checking path: "<<qPrintable(url.path())<<endl;
//		}
//		networkManager->get(QNetworkRequest(url));
//	}

	settingsPrintActivityTagsAction->setCheckable(true);
	settingsPrintActivityTagsAction->setChecked(defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS);

	settingsPrintDetailedTimetablesAction->setCheckable(true);
	settingsPrintDetailedTimetablesAction->setChecked(defs::PRINT_DETAILED_HTML_TIMETABLES);
	settingsPrintDetailedTeachersFreePeriodsTimetablesAction->setCheckable(true);
	settingsPrintDetailedTeachersFreePeriodsTimetablesAction->setChecked(defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS);

	settingsPrintNotAvailableSlotsAction->setCheckable(true);
	settingsPrintNotAvailableSlotsAction->setChecked(defs::PRINT_NOT_AVAILABLE_TIME_SLOTS);

	settingsPrintBreakSlotsAction->setCheckable(true);
	settingsPrintBreakSlotsAction->setChecked(defs::PRINT_BREAK_TIME_SLOTS);

	settingsPrintActivitiesWithSameStartingTimeAction->setCheckable(true);
	settingsPrintActivitiesWithSameStartingTimeAction->setChecked(defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME);

	//needed to sync the view table forms
	LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
	LockUnlock::increaseCommunicationSpinBox();

	showWarningForSubgroupsWithTheSameActivitiesAction->setCheckable(true);

	enableActivityTagMaxHoursDailyAction->setCheckable(true);
	enableStudentsMaxGapsPerDayAction->setCheckable(true);
	showWarningForNotPerfectConstraintsAction->setCheckable(true);

	enableStudentsMinHoursDailyWithAllowEmptyDaysAction->setCheckable(true);
	showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction->setCheckable(true);

	enableGroupActivitiesInInitialOrderAction->setCheckable(true);
	showWarningForGroupActivitiesInInitialOrderAction->setCheckable(true);

	showWarningForSubgroupsWithTheSameActivitiesAction->setChecked(defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES);

	enableActivityTagMaxHoursDailyAction->setChecked(defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	enableStudentsMaxGapsPerDayAction->setChecked(defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
	showWarningForNotPerfectConstraintsAction->setChecked(defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS);

	enableStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);
	showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);

	connect(showWarningForSubgroupsWithTheSameActivitiesAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForSubgroupsWithTheSameActivitiesToggled(bool)));

	connect(settingsShowSubgroupsInComboBoxesAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInComboBoxesToggled(bool)));
	connect(settingsShowSubgroupsInActivityPlanningAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInActivityPlanningToggled(bool)));

	connect(enableActivityTagMaxHoursDailyAction, SIGNAL(toggled(bool)), this, SLOT(enableActivityTagMaxHoursDailyToggled(bool)));
	connect(enableStudentsMaxGapsPerDayAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMaxGapsPerDayToggled(bool)));
	connect(showWarningForNotPerfectConstraintsAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForNotPerfectConstraintsToggled(bool)));

	connect(enableStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));
	connect(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));

	dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction->setIconVisibleInMenu(true);
	dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction->setIconVisibleInMenu(true);
	dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction->setIconVisibleInMenu(true);
	dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction->setIconVisibleInMenu(true);

	dataTimeConstraintsStudentsSetMaxGapsPerDayAction->setIconVisibleInMenu(true);
	dataTimeConstraintsStudentsMaxGapsPerDayAction->setIconVisibleInMenu(true);

	groupActivitiesInInitialOrderAction->setIconVisibleInMenu(true);

	setEnabledIcon(dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);

	setEnabledIcon(dataTimeConstraintsStudentsSetMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
	setEnabledIcon(dataTimeConstraintsStudentsMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);

    connect(&TContext::get()->instance, SIGNAL(gotModified()), this, SLOT(updateDescription()));

    updateDescription();
}

void MainForm::setEnabledIcon(QAction* action, bool enabled)
{
	static QIcon locked(":/images/locked.png");
	static QIcon unlocked(":/images/unlocked.png");

	if(enabled)
		action->setIcon(unlocked);
	else
		action->setIcon(locked);
}

//this is another place (out of two) in which you need to add a new language. The other one is in the file m-fet.cpp
void MainForm::populateLanguagesMap(QMap<QString, QString>& languagesMap)
{
	languagesMap.clear();
	languagesMap.insert("en_US", tr("US English"));
	languagesMap.insert("ar", tr("Arabic"));
	languagesMap.insert("ca", tr("Catalan"));
	languagesMap.insert("de", tr("German"));
	languagesMap.insert("el", tr("Greek"));
	languagesMap.insert("es", tr("Spanish"));
	languagesMap.insert("fr", tr("French"));
	languagesMap.insert("hu", tr("Hungarian"));
	languagesMap.insert("id", tr("Indonesian"));
	languagesMap.insert("it", tr("Italian"));
	languagesMap.insert("lt", tr("Lithuanian"));
	languagesMap.insert("mk", tr("Macedonian"));
	languagesMap.insert("ms", tr("Malay"));
	languagesMap.insert("nl", tr("Dutch"));
	languagesMap.insert("pl", tr("Polish"));
	languagesMap.insert("ro", tr("Romanian"));
	languagesMap.insert("tr", tr("Turkish"));
	languagesMap.insert("ru", tr("Russian"));
	languagesMap.insert("fa", tr("Persian"));
	languagesMap.insert("uk", tr("Ukrainian"));
	languagesMap.insert("pt_BR", tr("Brazilian Portuguese"));
	languagesMap.insert("da", tr("Danish"));
	languagesMap.insert("si", tr("Sinhala"));
	languagesMap.insert("sk", tr("Slovak"));
	languagesMap.insert("he", tr("Hebrew"));
	languagesMap.insert("sr", tr("Serbian"));
	languagesMap.insert("gl", tr("Galician"));
	languagesMap.insert("vi", tr("Vietnamese"));
	languagesMap.insert("uz", tr("Uzbek"));
	languagesMap.insert("sq", tr("Albanian"));
	languagesMap.insert("zh_CN", tr("Chinese Simplified"));
	languagesMap.insert("zh_TW", tr("Chinese Traditional"));
	languagesMap.insert("eu", tr("Basque"));
	languagesMap.insert("cs", tr("Czech"));
}


void MainForm::enableNotPerfectMessage()
{
	QString s=tr("Constraint is not enabled. To use this type of constraint you must enable it from the Settings->Advanced menu.");
	s+="\n\n";
	s+=tr("Explanation:");
	s+=" ";
	s+=tr("Constraints of this type are good, working, but they are not perfectly optimized.");
	s+=" ";
	s+=tr("For some situations, the generation of the timetable may take too long or be impossible.");
	s+="\n\n";
	s+=tr("Use with caution.");

	QMessageBox::information(this, tr("m-FET information"), s);
}

void MainForm::on_checkForUpdatesAction_toggled()
{
	defs::checkForUpdates=checkForUpdatesAction->isChecked();
}

void MainForm::on_settingsUseColorsAction_toggled()
{
	defs::USE_GUI_COLORS=settingsUseColorsAction->isChecked();

	LockUnlock::increaseCommunicationSpinBox();
}

void MainForm::showSubgroupsInComboBoxesToggled(bool checked)
{
	if(checked==false){
		QString s=tr("Note: if you disable this option, you will not be able to add/modify constraints for subgroups,"
		 " and you will not be able to make filters using a particular subgroup, when selecting activities.");
		//s+="\n\n";
		//s+=tr("Continue only if you know what you are doing.");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		if(b!=QMessageBox::Ok){
			disconnect(settingsShowSubgroupsInComboBoxesAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInComboBoxesToggled(bool)));
			settingsShowSubgroupsInComboBoxesAction->setChecked(true);
			connect(settingsShowSubgroupsInComboBoxesAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInComboBoxesToggled(bool)));
			return;
		}
	}

	defs::SHOW_SUBGROUPS_IN_COMBO_BOXES=checked;
}

void MainForm::showSubgroupsInActivityPlanningToggled(bool checked)
{
	if(checked==false){
		QString s=tr("Note: if you disable this option, you will not be able to view activities for subgroups"
		 " in the activity planning dialog.");
		//s+="\n\n";
		//s+=tr("Continue only if you know what you are doing.");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		if(b!=QMessageBox::Ok){
			disconnect(settingsShowSubgroupsInActivityPlanningAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInActivityPlanningToggled(bool)));
			settingsShowSubgroupsInActivityPlanningAction->setChecked(true);
			connect(settingsShowSubgroupsInActivityPlanningAction, SIGNAL(toggled(bool)), this, SLOT(showSubgroupsInActivityPlanningToggled(bool)));
			return;
		}
	}

	defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING=checked;
}

/////////confirmations
void MainForm::on_settingsConfirmActivityPlanningAction_toggled()
{
	defs::CONFIRM_ACTIVITY_PLANNING=settingsConfirmActivityPlanningAction->isChecked();
}

void MainForm::on_settingsConfirmSpreadActivitiesAction_toggled()
{
	defs::CONFIRM_SPREAD_ACTIVITIES=settingsConfirmSpreadActivitiesAction->isChecked();
}

void MainForm::on_settingsConfirmRemoveRedundantAction_toggled()
{
	defs::CONFIRM_REMOVE_REDUNDANT=settingsConfirmRemoveRedundantAction->isChecked();
}

void MainForm::on_settingsConfirmSaveTimetableAction_toggled()
{
	defs::CONFIRM_SAVE_TIMETABLE=settingsConfirmSaveTimetableAction->isChecked();
}
/////////

void MainForm::on_settingsShowShortcutsOnMainWindowAction_toggled()
{
	SHOW_SHORTCUTS_ON_MAIN_WINDOW=settingsShowShortcutsOnMainWindowAction->isChecked();
	tabWidget->setVisible(SHOW_SHORTCUTS_ON_MAIN_WINDOW);
}

void MainForm::on_settingsShowToolTipsForConstraintsWithTablesAction_toggled()
{
	defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES=settingsShowToolTipsForConstraintsWithTablesAction->isChecked();
}

void MainForm::on_settingsDivideTimetablesByDaysAction_toggled()
{
	defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS=settingsDivideTimetablesByDaysAction->isChecked();
}

void MainForm::on_settingsDuplicateVerticalNamesAction_toggled()
{
	defs::TIMETABLE_HTML_REPEAT_NAMES=settingsDuplicateVerticalNamesAction->isChecked();
}

void MainForm::on_timetablesToWriteOnDiskAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	TimetablesToWriteOnDiskForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::replyFinished(QNetworkReply* networkReply)
{
	if(networkReply->error()!=QNetworkReply::NoError){
		QString s=QString("");
		s+=tr("Could not search for possible updates on the internet - error message is: %1.").arg(networkReply->errorString());
		s+=QString("\n\n");
        s+=tr("Searching for file %1.").arg("http://lalescu.ro/liviu/fet/crtversion/crtversion.txt");
		s+=QString("\n\n");
		s+=tr("Possible actions: check your network connection, try again later, try to visit m-FET homepage: %1, or"
         " try to search for the new m-FET page on the internet (maybe it has changed).").arg("http://lalescu.ro/liviu/fet/");

		QMessageBox::warning(this, tr("m-FET warning"), s);
	}
	else{
        QString internetVersion;
		QString additionalComments;

		QRegExp regExp("^\\s*(\\S+)(.*)$");
		int t=regExp.indexIn(QString(networkReply->readAll()));
		if(t!=0){
			QString s=QString("");
            s+=tr("The file %1 from the m-FET homepage, indicating the current m-FET version, is incorrect.").arg("http://lalescu.ro/liviu/fet/crtversion/crtversion.txt");
			s+=QString("\n\n");
			s+=tr("Maybe the m-FET homepage has some temporary problems, so try again later."
			 " Or maybe the current structure on m-FET homepage was changed. You may visit m-FET homepage: %1, and get latest version or,"
			 " if it does not work, try to search for the new m-FET page on the internet (maybe it has changed).")
              .arg("http://lalescu.ro/liviu/fet/");

			QMessageBox::warning(this, tr("m-FET warning"), s);
		}
		else{
			defs::internetVersion=regExp.cap(1);
			additionalComments=regExp.cap(2).trimmed();

			if(defs::VERBOSE){
				cout<<"Your current version: '";
				cout<<qPrintable(FET_VERSION)<<"'"<<endl;
				cout<<"Latest version: '";
				cout<<qPrintable(defs::internetVersion)<<"'"<<endl;
			}
			if(defs::internetVersion!=FET_VERSION){
                QString s=tr("Another version: %1, is available on the m-FET homepage: %2", "%1 is new version, %2 is m-FET homepage").arg(defs::internetVersion).arg("http://lalescu.ro/liviu/fet/");
				s+=QString("\n\n");
				s+=tr("You have to manually download and install.")+QString(" ")+tr("You may need to hit Refresh in your web browser.")+QString("\n\n")+tr("Would you like to open the m-FET homepage now?");
				if(!additionalComments.isEmpty()){
					s+=QString("\n\n");
					s+=tr("Additional comments: %1").arg(additionalComments);
				}

				QMessageBox::StandardButton button=QMessageBox::information(this, tr("m-FET information"), s, QMessageBox::Yes|QMessageBox::No);

				if(button==QMessageBox::Yes){
                    bool tds=QDesktopServices::openUrl(QUrl("http://lalescu.ro/liviu/fet/"));
					if(!tds){
						QMessageBox::warning(this, tr("m-FET warning"), tr("Could not start the default internet browser (trying to open the link %1)."
                            " Maybe you can try to manually start your browser and open this link.").arg("http://lalescu.ro/liviu/fet/"));
					}
				}
			}
		}
	}

	networkReply->deleteLater();
}

void MainForm::closeOtherWindows()
{
	QList<QWidget*> tlwl=qApp->topLevelWidgets();

	foreach(QWidget* wi, tlwl)
		if(wi->isVisible() && wi!=this)
			wi->close();
}

void MainForm::closeEvent(QCloseEvent* event)
{
	//QRect rect(x(), y(), width(), height());
	QRect rect=this->geometry();
	mainFormSettingsRect=rect;

	MAIN_FORM_SHORTCUTS_TAB_POSITION=tabWidget->currentIndex();

    if(TContext::get()->instance.isModified()){
		QMessageBox::StandardButton res=QMessageBox::question( this, tr("m-FET - exiting"),
		 tr("Your data file has been modified - do you want to save it?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if(res==QMessageBox::Yes){
			bool t=this->fileSave();
			if(!t){
				event->ignore();
			}
			else{
				closeOtherWindows();
				event->accept();
			}
		}
		else if(res==QMessageBox::No){
			closeOtherWindows();
			event->accept();
		}
		else{
			assert(res==QMessageBox::Cancel);
			event->ignore();
		}
	}
	else{
		closeOtherWindows();
		event->accept();
	}
}

MainForm::~MainForm()
{
    QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(QString("MainForm/number-of-recent-files"), recentFiles.count());
	settings.remove(QString("MainForm/recent-file"));
	for(int i=0; i<recentFiles.count(); i++)
        settings.setValue(QString("MainForm/recent-file/")+utils::strings::number(i+1), recentFiles.at(i));

	shortcutBasicMenu->clear();
	delete shortcutBasicMenu;

	shortcutAdvancedTimeMenu->clear();
	delete shortcutAdvancedTimeMenu;

	shortcutDataSpaceMenu->clear();
	delete shortcutDataSpaceMenu;

	shortcutDataAdvancedMenu->clear();
	delete shortcutDataAdvancedMenu;

	//2014-07-01
	shortcutTimetableLockingMenu->clear();
	delete shortcutTimetableLockingMenu;

	shortcutTimetableAdvancedMenu->clear();
	delete shortcutTimetableAdvancedMenu;
}

void MainForm::on_fileExitAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	close();
}

QString MainForm::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainForm::setCurrentFile(const QString& fileName)
{
	QString currentFile=fileName;
	QString shownName=QString("Untitled");

	if(!currentFile.isEmpty()){
		shownName=strippedName(currentFile);
		recentFiles.removeAll(currentFile);
		recentFiles.prepend(currentFile);
		if(recentFiles.count()>MAX_RECENT_FILES){
			assert(recentFiles.count()==MAX_RECENT_FILES+1);
			assert(!recentFiles.isEmpty());
			recentFiles.removeLast();
		}
		updateRecentFileActions();
	}

	setWindowTitle(tr("%1[*] - m-FET", "The title of the main window, %1 is the name of the current file. "
	 "Please keep the string [*] unmodified (three characters) - it is used to make the difference between modified files and unmodified files.")
	 .arg(shownName));
}

void MainForm::updateRecentFileActions()
{
	QMutableStringListIterator i(recentFiles);
	while(i.hasNext()){
		if(!QFile::exists(i.next()))
			i.remove();
	}

	for(int j=0; j<MAX_RECENT_FILES; j++){
		if(j<recentFiles.count()){
			QString text=strippedName(recentFiles[j]);
			recentFileActions[j]->setText(text);
			recentFileActions[j]->setData(recentFiles[j]);
			recentFileActions[j]->setVisible(true);
		}
		else{
			recentFileActions[j]->setVisible(false);
		}
	}

	recentSeparatorAction->setVisible(!recentFiles.isEmpty());
}

void MainForm::on_fileClearRecentFilesListAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	recentFiles.clear();
	updateRecentFileActions();
}

void MainForm::on_fileNewAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	int confirm=0;

    if(TContext::get()->instance.isModified()){
		switch( QMessageBox::question(
		 this,
		 tr("m-FET application"),
		 tr("Your current data file has been modified. Are you sure you want to reset to new empty data?"),
		 QMessageBox::Yes|QMessageBox::No
		 ) ){
		case QMessageBox::Yes: // Yes
			confirm=1;
			break;
		case QMessageBox::No: // No
			confirm=0;
			break;
		default:
			assert(0);
			break;
		}
	}
	else
		confirm=1;

	if(confirm){
		defs::INPUT_FILENAME_XML=QString("");
		setCurrentFile(defs::INPUT_FILENAME_XML);

        if(TContext::get()->instance.initialized)
            TContext::get()->instance.kill();
        TContext::get()->instance.init();

        TContext::get()->instance.setModified(true); //to avoid flicker of the main form modified flag

        bool tmp=TContext::get()->instance.addTimeConstraint(new ConstraintBasicCompulsoryTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100));
		assert(tmp);
//		tmp=Timetable::getInstance()->rules.addSpaceConstraint(new ConstraintBasicCompulsorySpace(100));
//		assert(tmp);

        TContext::get()->instance.setModified(true); //force update of the modified flag of the main window
        setRulesUnmodifiedAndOtherThings(&TContext::get()->instance);

        TContext::get()->resetSchedule();

		LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
		LockUnlock::increaseCommunicationSpinBox();

		statusBar()->showMessage(tr("New file generated"), STATUS_BAR_MILLISECONDS);

        TContext::get()->solutions.clear();
	}
}

void MainForm::openRecentFile()
{
	QAction* action=qobject_cast<QAction*>(sender());
	if(action)
		openFile(action->data().toString());
}

void MainForm::on_fileOpenAction_triggered()
{
	openFile(QString());
}

void MainForm::openFile(const QString& fileName)
{
    statusBar()->showMessage(tr("Loading...", "This is a message in the status bar, that we are loading the file"), 0);
    pqapplication->processEvents();

    Enums::Status status = TContext::get()->loadInstance(this, fileName);

    if (status == Enums::Success) {
        LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
        LockUnlock::increaseCommunicationSpinBox();

        statusBar()->showMessage(tr("File opened"), STATUS_BAR_MILLISECONDS);

        setRulesUnmodifiedAndOtherThings(&TContext::get()->instance);

        setCurrentFile(defs::INPUT_FILENAME_XML);
    }
    else if (status == Enums::Fail){
        on_fileNewAction_triggered();
    }
    else if (status == Enums::Cancel){
        statusBar()->showMessage("", 0);
    }
}

bool MainForm::fileSaveAs()
{
    Enums::Status status = TContext::get()->saveInstance(this, QString());

    if (status == Enums::Success) {
        setRulesUnmodifiedAndOtherThings(&TContext::get()->instance);

        setCurrentFile(defs::INPUT_FILENAME_XML);

        statusBar()->showMessage(tr("File saved"), STATUS_BAR_MILLISECONDS);

        return true;
    }
    else if (status == Enums::Fail){

    }
    else if (status == Enums::Cancel){

    }

    return false;
}

void MainForm::updateDescription()
{
    instanceDetailsTextEdit->setPlainText(TContext::get()->instance.getDetailedDescription());
}

void MainForm::on_fileSaveAsAction_triggered()
{
	fileSaveAs();
}

// Start of code contributed by Volker Dirr
void MainForm::on_fileImportCSVRoomsBuildingsAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVRoomsAndBuildings(this);
}

void MainForm::on_fileImportCSVSubjectsAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVSubjects(this);
}

void MainForm::on_fileImportCSVTeachersAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVTeachers(this);
}

void MainForm::on_fileImportCSVActivitiesAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVActivities(this);

	//TODO: if the import takes care of locked activities, then we need
	//to do:
	//LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
	//LockUnlock::increaseCommunicationSpinBox();
	//after the importing
}

void MainForm::on_fileImportCSVActivityTagsAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVActivityTags(this);
}

void MainForm::on_fileImportCSVYearsGroupsSubgroupsAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Import::importCSVStudents(this);
}

void MainForm::on_fileExportCSVAction_triggered(){
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}
	Export::exportCSV(this);
}
// End of code contributed by Volker Dirr

void MainForm::on_timetableSaveTimetableAsAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::warning(this, tr("m-FET - Warning"), tr("You have not yet generated a timetable - please generate firstly"));
		return;
	}

    Solution tc=TContext::get()->bestSolution();

    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
        //Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
        int time=tc.times[(unsigned)ai];
		if(time==UNALLOCATED_TIME){
			QMessageBox::warning(this, tr("m-FET - Warning"), tr("It seems that you have an incomplete timetable."
			 " Saving of timetable does not work for incomplete timetables. Please generate a complete timetable"));
			 //.arg(act->id));
			return;
		}

//		int ri=tc->rooms[ai];
//		if(ri==UNALLOCATED_SPACE){
//			QMessageBox::warning(this, tr("m-FET - Warning"), tr("It seems that you have an incomplete timetable."
//			 " Saving of timetable does not work for incomplete timetables. Please generate a complete timetable"));
//			 //.arg(act->id));
//			return;
//		}
	}

	bool ok_to_continue;
	SaveTimetableConfirmationForm* pc_form=nullptr;
	if(defs::CONFIRM_SAVE_TIMETABLE){
		int confirm;

		pc_form=new SaveTimetableConfirmationForm(this);
		setParentAndOtherThings(pc_form, this);
		confirm=pc_form->exec();

		if(confirm==QDialog::Accepted){
			if(pc_form->dontShowAgain)
				settingsConfirmSaveTimetableAction->setChecked(false);

			ok_to_continue=true;
		}
		else
			ok_to_continue=false;
	}
	else
		ok_to_continue=true;

	if(ok_to_continue){
		QWidget* parent=pc_form;
		if(parent==nullptr)
			parent=this;

		QString s;

		for(;;){
			s = QFileDialog::getSaveFileName(parent, tr("Choose a filename for data and timetable"),
				defs::INPUT_FILENAME_XML, tr("m-FET XML files", "Instructions for translators: m-FET XML is a type of file format (using text mode). "
				"So this field means files in the m-FET XML format")+" (*.m-fet)"+";;"+tr("All files")+" (*)",
                nullptr, QFileDialog::DontConfirmOverwrite);
			if(s==QString())
				return;

			int tmp2=s.lastIndexOf(defs::FILE_SEP);
			QString s2=s.right(s.length()-tmp2-1);

			if(s2.length()>=1){
				if(s2.at(0).isSpace()){
					QMessageBox::warning(this, tr("m-FET information"),
					 tr("Please do not use a filename starting with white space(s), the html css code does not work.")
					 +"\n\n"+tr("File was not saved."));
					return;
				}
			}
			QString s3;
			bool ewf;
			if(s2.endsWith(".m-fet")){
				s3=s2.left(s2.length()-4);
				ewf=true;
			}
			else{
				s3=s2;
				ewf=false;
			}
			if(s3.length()>=1){
				if(s3.at(s3.length()-1).isSpace()){
					if(ewf)
						QMessageBox::warning(this, tr("m-FET information"),
						 tr("Please do not use a filename ending with white space(s) before the '.m-fet' termination, problems might arise.")
						 +"\n\n"+tr("File was not saved."));
					else
						QMessageBox::warning(this, tr("m-FET information"),
						 tr("Please do not use a filename ending with white space(s), problems might arise.")
						 +"\n\n"+tr("File was not saved."));
					return;
				}
			}
			if(s2.indexOf("\"") >= 0){
				QMessageBox::warning(parent, tr("m-FET information"), tr("Please do not use quotation marks \" in filename, the html css code does not work")
				 +"\n\n"+tr("File was not saved."));
				return;
			}
			if(s2.indexOf(";") >= 0){
				QMessageBox::warning(parent, tr("m-FET information"), tr("Please do not use semicolon ; in filename, the html css code does not work")
				 +"\n\n"+tr("File was not saved."));
				return;
			}
			if(s2.indexOf("#") >= 0){
				QMessageBox::warning(parent, tr("m-FET information"), tr("Please do not use # in filename, the html css code does not work")
				 +"\n\n"+tr("File was not saved."));
				return;
			}

			if(s.right(4)!=".m-fet")
				s+=".m-fet";

			int tmp=s.lastIndexOf(defs::FILE_SEP);
			defs::WORKING_DIRECTORY=s.left(tmp);

			if(QFile::exists(s)){
				QString t=tr("File exists");
				t+="\n\n";
				t+=tr("For safety (so you don't lose work), it is not allowed to overwrite an existing file with"
					" locking and saving a current data+timetable");
				t+="\n\n";
				t+=tr("Please choose a non-existing name");

				QMessageBox::warning( parent, tr("m-FET warning"), t);
			}
			else
				break;
		}

		rules2.initialized=true;

        rules2.institutionName=TContext::get()->instance.institutionName;
        rules2.comments=TContext::get()->instance.comments;

        rules2.nHoursPerDay=TContext::get()->instance.nHoursPerDay;
        for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
            rules2.hoursOfTheDay[i]=TContext::get()->instance.hoursOfTheDay[i];

        rules2.nDaysPerWeek=TContext::get()->instance.nDaysPerWeek;
        for(int i=0; i<TContext::get()->instance.nDaysPerWeek; i++)
            rules2.daysOfTheWeek[i]=TContext::get()->instance.daysOfTheWeek[i];

        rules2.yearsList=TContext::get()->instance.yearsList;

        rules2.teachersList=TContext::get()->instance.teachersList;

        rules2.subjectsList=TContext::get()->instance.subjectsList;

        rules2.activityTagsList=TContext::get()->instance.activityTagsList;

        rules2.activitiesList=TContext::get()->instance.activitiesList;

//		rules2.buildingsList=Timetable::getInstance()->rules.buildingsList;

//		rules2.roomsList=Timetable::getInstance()->rules.roomsList;

        rules2.timeConstraintsList=TContext::get()->instance.timeConstraintsList;

//		rules2.spaceConstraintsList=Timetable::getInstance()->rules.spaceConstraintsList;

        rules2.apstHash=TContext::get()->instance.apstHash;
//		rules2.aprHash=Timetable::getInstance()->rules.aprHash;

//		rules2.groupActivitiesInInitialOrderList=Timetable::getInstance()->rules.groupActivitiesInInitialOrderList;

		//add locking constraints
		TimeConstraintsList lockTimeConstraintsList;
//		SpaceConstraintsList lockSpaceConstraintsList;

		//bool report=true;

		int addedTime=0, duplicatesTime=0;
		int addedSpace=0, duplicatesSpace=0;

		QString constraintsString=QString("");

		//lock selected activities
        for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
            int time=tc.times[(unsigned)ai];
            if(time>=0 && time<TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay){
                int hour=tc.getHourForActivityIndex(ai);
                int day=tc.getDayForActivityIndex(ai);

                ConstraintActivityPreferredStartingTime* ctr=new ConstraintActivityPreferredStartingTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100.0, act->id, day, hour, false); //permanently locked is false
				bool t=rules2.addTimeConstraint(ctr);

				if(t){
					addedTime++;
					lockTimeConstraintsList.append(ctr);
				}
				else
					duplicatesTime++;

				QString s;

				if(t)
                    s=tr("Added to the saved file:", "It refers to a constraint")+QString("\n")+ctr->getDetailedDescription();
				else{
                    s=tr("NOT added to the saved file (already existing):", "It refers to a constraint")+QString("\n")+ctr->getDetailedDescription();
					delete ctr;
				}

				constraintsString+=QString("\n");
				constraintsString+=s;
			}

//			int ri=tc->rooms[ai];
//			if(ri!=UNALLOCATED_SPACE && ri!=UNSPECIFIED_ROOM && ri>=0 && ri<Timetable::getInstance()->rules.nInternalRooms){
//				ConstraintActivityPreferredRoom* ctr=new ConstraintActivityPreferredRoom(100, act->id, (Timetable::getInstance()->rules.internalRoomsList[ri])->name, false); //false means not permanently locked
//				bool t=rules2.addSpaceConstraint(ctr);

//				QString s;

//				if(t){
//					addedSpace++;
//					lockSpaceConstraintsList.append(ctr);
//				}
//				else
//					duplicatesSpace++;

//				if(t)
//					s=tr("Added to the saved file:", "It refers to a constraint")+QString("\n")+ctr->getDetailedDescription(Timetable::getInstance()->rules);
//				else{
//					s=tr("NOT added to the saved file (already existing):", "It refers to a constraint")+QString("\n")+ctr->getDetailedDescription(Timetable::getInstance()->rules);
//					delete ctr;
//				}

//				constraintsString+=QString("\n");
//				constraintsString+=s;
//			}
		}

		MessagesManager::information(parent, tr("m-FET information"), tr("Added %1 locking time constraints and %2 locking space constraints to saved file,"
		" ignored %3 activities which were already fixed in time and %4 activities which were already fixed in space.").arg(addedTime).arg(addedSpace).arg(duplicatesTime).arg(duplicatesSpace)
		+QString("\n\n")+tr("Detailed information about each locking constraint which was added or not (if already existing) to the saved file:")+QString("\n")+constraintsString
		+QString("\n")+tr("Your current data file remained untouched (no locking constraints were added), so you can save it also, or generate different timetables."));

		bool result=rules2.write(parent, s);

		Q_UNUSED(result);

		while(!lockTimeConstraintsList.isEmpty())
			delete lockTimeConstraintsList.takeFirst();
//		while(!lockSpaceConstraintsList.isEmpty())
//			delete lockSpaceConstraintsList.takeFirst();

		rules2.nHoursPerDay=0;
		rules2.nDaysPerWeek=0;

		rules2.yearsList.clear();

		rules2.teachersList.clear();

		rules2.subjectsList.clear();

        rules2.activityTagsList.clear();

		rules2.activitiesList.clear();

//		rules2.buildingsList.clear();

//		rules2.roomsList.clear();

		rules2.timeConstraintsList.clear();

//		rules2.spaceConstraintsList.clear();

		rules2.apstHash.clear();
//		rules2.aprHash.clear();

//		rules2.groupActivitiesInInitialOrderList.clear();
	}

	if(pc_form!=nullptr)
		delete pc_form;
}

bool MainForm::fileSave()
{
    Enums::Status status = TContext::get()->saveInstance(this, defs::INPUT_FILENAME_XML);

    if (status == Enums::Success) {
        setRulesUnmodifiedAndOtherThings(&TContext::get()->instance);

        setCurrentFile(defs::INPUT_FILENAME_XML);

        statusBar()->showMessage(tr("File saved"), STATUS_BAR_MILLISECONDS);

        return true;
    }
    else if (status == Enums::Fail){

    }
    else if (status == Enums::Cancel){

    }

    return false;
}

void MainForm::on_fileSaveAction_triggered()
{
	fileSave();
}

void MainForm::on_dataInstitutionNameAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	InstitutionNameForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataCommentsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	CommentsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataDaysAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	DaysForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataHoursAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	HoursForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTeachersAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	TeachersForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTeachersStatisticsAction_triggered()
{
	TeachersStatisticsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataSubjectsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	SubjectsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataSubjectsStatisticsAction_triggered()
{
	SubjectsStatisticsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataActivityTagsAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ActivityTagsForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_dataYearsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	YearsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataGroupsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	GroupsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataSubgroupsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	SubgroupsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataStudentsStatisticsAction_triggered()
{
	StudentsStatisticsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

//void MainForm::on_dataActivitiesRoomsStatisticsAction_triggered()
//{
//	ActivitiesRoomsStatisticsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

void MainForm::on_dataTeachersSubjectsQualificationsStatisticsAction_triggered()
{
	QHash<QString, Teacher*> teachersHash;

    foreach(Teacher* tch, TContext::get()->instance.teachersList)
		teachersHash.insert(tch->name, tch);

	bool unqualifiedExist=false;

	QString s=tr("The teachers who are not qualified to teach a certain activity (in activities order):");
	s+="\n\n";
    //TODO:
//	bool begin=true;

//    foreach(Activity* act, Timetable::getInstance()->instance.activitiesList){
//		bool alreadyAdded=false;
//		QString subject=act->subjectName;
//		foreach(QString teacher, act->teacherName){
//			Teacher* tch=teachersHash.value(teacher, nullptr);
//			assert(tch!=nullptr);
//			if(!tch->qualifiedSubjectsHash.contains(subject)){
//				unqualifiedExist=true;
//				if(!alreadyAdded){
//					if(!begin)
//						s+="\n";
//					else
//						begin=false;
//                    s+=tr("For activity: %1").arg(act->getDescription());
//					s+="\n";
//					alreadyAdded=true;
//				}
//				s+=QString(4, ' ');
//				s+=tr("Teacher %1 is not qualified to teach subject %2.").arg(teacher).arg(subject);
//				s+="\n";
//			}
//		}
//	}

	if(!unqualifiedExist)
		s=tr("All the teachers are qualified to teach their activities.");
	s+="\n";

	MessagesManager::information(this, tr("m-FET information"), s);
}

void MainForm::on_helpSettingsAction_triggered()
{
	QString s;

	s+=tr("Probably some settings which are more difficult to understand are these ones:");
	s+="\n\n";
	s+=tr("Option 'Divide html timetables with time-axis by days':"
	" This means simply that the html timetables of type 'time horizontal' or 'time vertical' (see the generated html timetables)"
	" should be or not divided according to the days.");
	s+=" ";
	s+=tr("If the 'time horizontal' or 'time vertical' html timetables are too large for you, then you might need to select this option");

	s+="\n\n";
	s+=tr("Option 'Print activities with same starting time in timetables': selecting it means that the html timetables will contain for"
	 " each slot all the activities which have the same starting time (fact specified by your constraints) as the activity(ies) which are normally shown in this slot."
	 " If you don't use constraints activities same starting time, this option has no effect for you.");

	s+="\n\n";
	s+=tr("Seed of random number generator: please read the help in the dialog of this option");

	s+="\n\n";
	s+=tr("Interface - use colors: the places with colors in m-FET interface are in:");
	s+="\n";
	s+=" -";
	s+=tr("add/modify constraints break, not available, preferred starting times or time slots, occupy max time slots from selection or"
		" max simultaneous in selected time slots (the table cells will have green or red colors).");
	s+="\n";
	s+=" -";
	s+=tr("activities and subactivities dialogs, the inactive activities will have a distinctive background color");
	s+="\n";
	s+=" -";
	s+=tr("all time constraints and all space constraints dialogs, the inactive constraints will have a distinctive background color");
	s+="\n";
	s+=" -";
	s+=tr("group activities in initial order items, the inactive items will have a distinctive background color");
	s+="\n";
	s+=" -";
	s+=tr("the three timetable view dialogs");

	s+="\n\n";
	s+=tr("Enable activity tag max hours daily:");
	s+="\n";
	s+=tr("This will enable the menu for 4 constraints: teacher(s) or students (set) activity tag max hours daily. These 4 constraints are good, but not perfect and"
		" may bring slow down of generation or impossible timetables if used unproperly. Select only if you know what you're doing.");
	s+="\n\n";
	s+=tr("Enable students max gaps per day:");
	s+="\n";
	s+=tr("This will enable the menu for 2 constraints: students (set) max gaps per day. These 2 constraints are good, but not perfect and"
		" may bring slow down of generation or impossible timetables if used unproperly. Select only if you know what you're doing.");

	s+="\n\n";
	s+=tr("Warn if using not perfect constraints:", "this is a warning if user uses not perfect constraints");
	s+="\n";
	s+=tr("If you use a not perfect constraint (activity tag max hours daily or students max gaps per day), you'll get a warning before generating"
		". Uncheck this option to get rid of that warning (it is recommended to keep the warning).");

	s+="\n\n";
	s+=tr("Enable students min hours daily with empty days:");
	s+="\n";
	s+=tr("This will enable you to modify the students min hours daily constraints, to allow empty days. It is IMPERATIVE (for performance reasons) to allow empty days for students min hours daily only"
		" if your institution allows empty days for students and if a solution with empty days is possible. Select only if you know what you're doing.");

	s+="\n\n";
	s+=tr("Warn if using students min hours daily with empty days:", "this is a warning if user has constraints min hours daily for students with allowed empty days");
	s+="\n";
	s+=tr("If you use constraints students min hours daily with allowed empty days, you'll get a warning before generating"
		". Uncheck this option to get rid of that warning (it is recommended to keep the warning).");

	s+="\n\n";

	s+=tr("Confirmations: unselect the corresponding check boxes if you want to skip introduction and confirmation to various advanced dialogs.");

	s+="\n\n";

	s+=tr("Duplicate vertical headers to the right (in timetable settings) - select this if you want the timetables to duplicate the table left vertical headers to the right"
		" part of the tables");

	s+="\n\n";

	s+=tr("If you have many subgroups and you don't explicitly use them, it is recommended to use the three global settings: hide subgroups"
		" in combo boxes, hide subgroups in activity planning, and do not write subgroups timetables on hard disk.");
	s+="\n";
	s+=tr("Note that using the global menu setting to hide subgroups in activity planning is a different thing from the check box in the activity"
		" planning dialog, and the global setting works better and faster.");
	s+="\n";
	s+=tr("If you hide subgroups in combo boxes, the affected dialogs (like the activities dialog or the add constraint students set not available times dialog)"
		" will load much faster if you have many subgroups.");

	s+="\n\n";

	s+=tr("If you are only working on a timetable, and you do not need to publish it, you may want to disable writing some categories of timetables"
		" on the hard disk, for operativity (the generation speed is not affected, only the overhead to write the partial/complete timetables"
		" when stopping/finishing the simulation). The timetables taking the longest time are the subgroups, groups AND years ones.");
	s+=" ";
	s+=tr("(Also the conflicts timetable might take long to write, if the file is large.)");
	s+=" ";
	s+=tr("After that, you can re-enable writing of the timetables and re-generate.");

	s+="\n\n";
	s+=tr("Show tool tips for constraints with tables: in the add/modify constraint dialogs which use tables, like the 'not available times' ones,"
		" each table cell will have a tool tip to show the current day/hour (column/row name).");

	MessagesManager::information(this, tr("m-FET information"), s);
}

void MainForm::on_dataHelpOnStatisticsAction_triggered()
{
	QString s;

	s+=tr("You will find in the statistics only active activities count. The inactive ones are not counted.");

	s+="\n\n";

	s+=tr("Statistics for students might be the most difficult to understand."
	 " If you are using divisions of years: probably the most relevant statistics"
	 " are the ones for each subgroup (so you may check only subgroups check box)."
	 " You may see more hours for the years or groups, but these are not significant, please ignore them,"
	 " because each year or group will count also activities of all contained subgroups."
	 "\n\n"
	 "Each subgroup should have a number of hours per week close to the average of"
	 " all subgroups and close to the normal number of working hours of each students set."
	 " If a subgroup has a much lower value, maybe you used incorrectly"
	 " the years/groups/subgroups for activities."
	 "\n\n"
	 "Please read FAQ for detailed description"
	 " on how divisions work. The key is that the subgroups are independent and represent the smallest unit of students."
	 " Each subgroup receives the activities of the parent year and parent group and of itself."
	 "\n\n"
	 "Having a subgroup with too little working hours per week means that you inputted activities in a wrong manner,"
	 " and also that some constraints like no gaps, early or min hours daily for this subgroup"
	 " are interpreted in a wrong manner (if subgroup has only 2 activities, then these must"
	 " be placed in the first hours, which is too hard and wrong)."
	 );

	s+="\n\n";
	s+=tr("Students' statistics form contains a check box named '%1'"
	 ". This has effect only if you have overlapping groups/years, and means that m-FET will show the complete tree structure"
	 ", even if that means that some subgroups/groups will appear twice or more in the table, with the same information."
	 " For instance, if you have year Y1, groups G1 and G2, subgroups S1, S2, S3, with structure: Y1 (G1 (S1, S2), G2 (S1, S3)),"
	 " S1 will appear twice in the table").arg(tr("Show duplicates"));

	s+="\n\n";
	s+=tr("Activities rooms statistics: this menu will show the activities which may be scheduled in an unspecified room,"
	 " if they are referred to by space constraints with weight under 100.0%, and the activities which will certainly be scheduled in an"
	 " unspecified room, if they are not referred to by any space constraints. Remember that home rooms constraints are effective only"
	 " on activities which have only the exact specified teacher or students set (activities with more teachers or students"
	 " sets are not affected by home rooms constraints, you need to have preferred rooms constraints for such activities to ensure"
	 " they will not end up in an unspecified room).");

	MessagesManager::information(this, tr("m-FET - information about statistics"), s);
}

void MainForm::on_dataActivitiesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ActivitiesForm form(this, "", "", "", "");
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataAllTimeConstraintsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	AllTimeConstraintsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTwoActivitiesConsecutiveAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTwoActivitiesConsecutiveForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTwoActivitiesGroupedAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTwoActivitiesGroupedForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsThreeActivitiesGroupedAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintThreeActivitiesGroupedForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTwoActivitiesOrderedAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTwoActivitiesOrderedForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesPreferredTimeSlotsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesPreferredTimeSlotsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesPreferredStartingTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesPreferredStartingTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsSubactivitiesPreferredTimeSlotsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintSubactivitiesPreferredTimeSlotsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsSubactivitiesPreferredStartingTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintSubactivitiesPreferredStartingTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivityEndsStudentsDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivityEndsStudentsDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesEndStudentsDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesEndStudentsDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesSameStartingTimeAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesSameStartingTimeForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesSameStartingHourAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesSameStartingHourForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesSameStartingDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesSameStartingDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesOccupyMaxTimeSlotsFromSelectionAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesOccupyMaxTimeSlotsFromSelectionForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesMaxSimultaneousInSelectedTimeSlotsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesMaxSimultaneousInSelectedTimeSlotsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherNotAvailableTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherNotAvailableTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

//void MainForm::on_dataSpaceConstraintsRoomNotAvailableTimesAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintRoomNotAvailableTimesForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsActivityPreferredRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintActivityPreferredRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsActivityPreferredRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintActivityPreferredRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsSubjectPreferredRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintSubjectPreferredRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsSubjectPreferredRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintSubjectPreferredRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsSubjectActivityTagPreferredRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintSubjectActivityTagPreferredRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsSubjectActivityTagPreferredRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintSubjectActivityTagPreferredRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

/////added 6 apr 2009
//void MainForm::on_dataSpaceConstraintsActivityTagPreferredRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintActivityTagPreferredRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsActivityTagPreferredRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintActivityTagPreferredRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}
/////

//void MainForm::on_dataSpaceConstraintsStudentsSetHomeRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsSetHomeRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsSetHomeRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsSetHomeRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}


//void MainForm::on_dataSpaceConstraintsStudentsSetMaxBuildingChangesPerDayAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsSetMaxBuildingChangesPerDayForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsMaxBuildingChangesPerDayAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsMaxBuildingChangesPerDayForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsSetMaxBuildingChangesPerWeekAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsSetMaxBuildingChangesPerWeekForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsMaxBuildingChangesPerWeekAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsMaxBuildingChangesPerWeekForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsSetMinGapsBetweenBuildingChangesAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsSetMinGapsBetweenBuildingChangesForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsStudentsMinGapsBetweenBuildingChangesAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintStudentsMinGapsBetweenBuildingChangesForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeacherMaxBuildingChangesPerDayAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeacherMaxBuildingChangesPerDayForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeachersMaxBuildingChangesPerDayAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeachersMaxBuildingChangesPerDayForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeacherMaxBuildingChangesPerWeekAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeacherMaxBuildingChangesPerWeekForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeachersMaxBuildingChangesPerWeekAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeachersMaxBuildingChangesPerWeekForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeacherMinGapsBetweenBuildingChangesAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeacherMinGapsBetweenBuildingChangesForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeachersMinGapsBetweenBuildingChangesAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeachersMinGapsBetweenBuildingChangesForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeacherHomeRoomAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeacherHomeRoomForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

//void MainForm::on_dataSpaceConstraintsTeacherHomeRoomsAction_triggered()
//{
//	if(Timetable::getInstance()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//	ConstraintTeacherHomeRoomsForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();
//}

void MainForm::on_dataTimeConstraintsStudentsSetNotAvailableTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetNotAvailableTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsBreakTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintBreakTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMinDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMinDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMinDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMinDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherIntervalMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherIntervalMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersIntervalMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersIntervalMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetIntervalMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetIntervalMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsIntervalMaxDaysPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsIntervalMaxDaysPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxHoursContinuouslyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxHoursContinuouslyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersActivityTagMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

//    ConstraintTeachersActivityTagMaxHoursContinuouslyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherActivityTagMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

//    ConstraintTeacherActivityTagMaxHoursContinuouslyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    if(!defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY){
        enableNotPerfectMessage();
        return;
    }

//    ConstraintTeachersActivityTagMaxHoursDailyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    if(!defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY){
        enableNotPerfectMessage();
        return;
    }

//    ConstraintTeacherActivityTagMaxHoursDailyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMinHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMinHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMinHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMinHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivityPreferredStartingTimeAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivityPreferredStartingTimeForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxGapsPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMaxGapsPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxGapsPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMaxGapsPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxGapsPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	if(!defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY){
		enableNotPerfectMessage();
		return;
	}

	ConstraintStudentsSetMaxGapsPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxGapsPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	if(!defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY){
		enableNotPerfectMessage();
		return;
	}

	ConstraintStudentsMaxGapsPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxGapsPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxGapsPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxGapsPerWeekAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxGapsPerWeekForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxGapsPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxGapsPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxGapsPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxGapsPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsEarlyMaxBeginningsAtSecondHourAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsEarlyMaxBeginningsAtSecondHourForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetEarlyMaxBeginningsAtSecondHourAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetEarlyMaxBeginningsAtSecondHourForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMaxHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMaxHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMaxHoursContinuouslyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMaxHoursContinuouslyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetActivityTagMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

//    ConstraintStudentsSetActivityTagMaxHoursContinuouslyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsActivityTagMaxHoursContinuouslyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

//    ConstraintStudentsActivityTagMaxHoursContinuouslyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    if(!defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY){
        enableNotPerfectMessage();
        return;
    }

//    ConstraintStudentsSetActivityTagMaxHoursDailyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    if(!defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY){
        enableNotPerfectMessage();
        return;
    }

//    ConstraintStudentsActivityTagMaxHoursDailyForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMinHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMinHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMinHoursDailyAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMinHoursDailyForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivitiesNotOverlappingAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivitiesNotOverlappingForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsMinDaysBetweenActivitiesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintMinDaysBetweenActivitiesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsMaxDaysBetweenActivitiesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintMaxDaysBetweenActivitiesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsMinGapsBetweenActivitiesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintMinGapsBetweenActivitiesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivityPreferredTimeSlotsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivityPreferredTimeSlotsForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsActivityPreferredStartingTimesAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintActivityPreferredStartingTimesForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMaxSpanPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMaxSpanPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMaxSpanPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMaxSpanPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMaxSpanPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMaxSpanPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMaxSpanPerDayAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMaxSpanPerDayForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeacherMinRestingHoursAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeacherMinRestingHoursForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsTeachersMinRestingHoursAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintTeachersMinRestingHoursForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsSetMinRestingHoursAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsSetMinRestingHoursForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_dataTimeConstraintsStudentsMinRestingHoursAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	ConstraintStudentsMinRestingHoursForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_helpAboutAction_triggered()
{
	HelpAboutForm* form=new HelpAboutForm(this);
	form->setWindowFlags(Qt::Window);
	form->setAttribute(Qt::WA_DeleteOnClose);
	forceCenterWidgetOnScreen(form);
	restoreFETDialogGeometry(form);
	form->show();
}

//void MainForm::on_helpHomepageAction_triggered()
//{
//    bool tds=QDesktopServices::openUrl(QUrl("http://lalescu.ro/liviu/fet/"));

//	if(!tds){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Could not start the default internet browser (trying to open the link %1)."
//        " Maybe you can try to manually start your browser and open this link.").arg("http://lalescu.ro/liviu/fet/"));
//	}
//}

//void MainForm::on_helpContentsAction_triggered()
//{
//    bool tds=QDesktopServices::openUrl(QUrl("http://lalescu.ro/liviu/fet/doc/"));

//	if(!tds){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Could not start the default internet browser (trying to open the link %1)."
//        " Maybe you can try to manually start your browser and open this link.").arg("http://lalescu.ro/liviu/fet/doc/"));
//	}
//}

//void MainForm::on_helpForumAction_triggered()
//{
//    bool tds=QDesktopServices::openUrl(QUrl("http://lalescu.ro/liviu/fet/forum/"));

//	if(!tds){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Could not start the default internet browser (trying to open the link %1)."
//        " Maybe you can try to manually start your browser and open this link.").arg("http://lalescu.ro/liviu/fet/forum/"));
//	}
//}

//void MainForm::on_helpAddressesAction_triggered()
//{
//	QString s="";
//	s+=tr("In case the Help/Online menus do not function, please write down these addresses and open them in an internet browser:");
//	s+="\n\n";
//    s+=tr("m-FET homepage: %1", "%1 is m-FET homepage, begins with http://...").arg("http://lalescu.ro/liviu/fet/");
//	s+="\n";
//    s+=tr("Documentation (online help contents): %1", "%1 is web page of m-FET Doc(umentation), which is the contents of the online help, it begins with http://...").arg("http://lalescu.ro/liviu/fet/doc/");
//	s+="\n";
//    s+=tr("Forum: %1", "%1 is web page of m-FET forum, begins with http://...").arg("http://lalescu.ro/liviu/fet/forum/");
//	s+="\n\n";
//	s+=tr("Additionally, you may find on the m-FET homepage other contact information.");
//	s+="\n\n";
//	s+=tr("In case these addresses do not function, maybe the m-FET webpage has temporary problems, so try again later. Or maybe the m-FET webpage has changed, so search for the new page on the internet.");

//	MessagesManager::information(this, tr("m-FET web addresses"), s);
//}

void MainForm::on_helpFAQAction_triggered()
{
	HelpFaqForm* form=new HelpFaqForm(this);
	form->setWindowFlags(Qt::Window);
	form->setAttribute(Qt::WA_DeleteOnClose);
	forceCenterWidgetOnScreen(form);
	restoreFETDialogGeometry(form);
	form->show();
}

void MainForm::on_helpTipsAction_triggered()
{
	HelpTipsForm* form=new HelpTipsForm(this);
	form->setWindowFlags(Qt::Window);
	form->setAttribute(Qt::WA_DeleteOnClose);
	forceCenterWidgetOnScreen(form);
	restoreFETDialogGeometry(form);
	form->show();
}

void MainForm::on_helpInstructionsAction_triggered()
{
	HelpInstructionsForm* form=new HelpInstructionsForm(this);
	form->setWindowFlags(Qt::Window);
	form->setAttribute(Qt::WA_DeleteOnClose);
	forceCenterWidgetOnScreen(form);
	restoreFETDialogGeometry(form);
	form->show();
}

void MainForm::on_timetableGenerateAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	int count=0;
    for(int i=0; i<TContext::get()->instance.activitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activitiesList[i];
		if(act->active)
			count++;
	}
	if(count<1){
		QMessageBox::information(this, tr("m-FET information"), tr("Please input at least one active activity before generating"));
		return;
	}
    TimetableGenerateForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();

	LockUnlock::increaseCommunicationSpinBox();
}

void MainForm::on_timetableGenerateMultipleAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	if(defs::INPUT_FILENAME_XML.isEmpty()){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Your current data has no name. Please save it as a file with a certain name before proceeding."));
		return;
	}

	int count=0;
    for(int i=0; i<TContext::get()->instance.activitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activitiesList[i];
		if(act->active)
			count++;
	}
	if(count<1){
		QMessageBox::information(this, tr("m-FET information"), tr("Please input at least one active activity before generating multiple"));
		return;
	}
//	TimetableGenerateMultipleForm form(this);
//	setParentAndOtherThings(&form, this);
//	form.exec();

	LockUnlock::increaseCommunicationSpinBox();
}

void MainForm::on_statisticsPrintAction_triggered()
{
	StartStatisticsPrint::startStatisticsPrint(this);
}

void MainForm::on_timetableLockAllActivitiesAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		return;
	}

	AdvancedLockUnlockForm::lockAll(this);
}

void MainForm::on_timetableUnlockAllActivitiesAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		//QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		QMessageBox::information(this, tr("m-FET information"), tr("The timetable is not generated, but anyway m-FET will proceed now"));

		AdvancedLockUnlockForm::unlockAllWithoutTimetable(this);

		return;
	}

	AdvancedLockUnlockForm::unlockAll(this);
}

void MainForm::on_timetableLockActivitiesDayAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		return;
	}

	AdvancedLockUnlockForm::lockDay(this);
}

void MainForm::on_timetableUnlockActivitiesDayAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		//QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		QMessageBox::information(this, tr("m-FET information"), tr("The timetable is not generated, but anyway m-FET will proceed now"));

		AdvancedLockUnlockForm::unlockDayWithoutTimetable(this);

		return;
	}

	AdvancedLockUnlockForm::unlockDay(this);
}

void MainForm::on_timetableLockActivitiesEndStudentsDayAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		return;
	}

	AdvancedLockUnlockForm::lockEndStudentsDay(this);
}

void MainForm::on_timetableUnlockActivitiesEndStudentsDayAction_triggered()
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please generate, firstly"));
		return;
	}

	AdvancedLockUnlockForm::unlockEndStudentsDay(this);
}

void MainForm::on_languageAction_triggered()
{
	QDialog dialog(this);
	dialog.setWindowTitle(tr("Please select m-FET language"));

	QVBoxLayout* taMainLayout=new QVBoxLayout(&dialog);

	QPushButton* tapb1=new QPushButton(tr("Cancel"));
	QPushButton* tapb2=new QPushButton(tr("OK"));

	QHBoxLayout* buttons=new QHBoxLayout();
	buttons->addStretch();
	buttons->addWidget(tapb1);
	buttons->addWidget(tapb2);

	QComboBox* languagesComboBox=new QComboBox();

	QSize tmp=languagesComboBox->minimumSizeHint();
	Q_UNUSED(tmp);

	QMap<QString, QString> languagesMap;
	populateLanguagesMap(languagesMap);

	//assert(languagesMap.count()==N_LANGUAGES);

	QMapIterator<QString, QString> it(languagesMap);
	int i=0;
	int j=-1;
	int eng=-1;
	while(it.hasNext()){
		it.next();
		languagesComboBox->addItem( it.key() + " (" + it.value() + ")" );
		if(it.key()==defs::FET_LANGUAGE)
			j=i;
		if(it.key()=="en_US")
			eng=i;
		i++;
	}
	assert(eng>=0);
	if(j==-1){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid current language - making it en_US (US English)"));
		defs::FET_LANGUAGE="en_US";
		j=eng;
	}
	languagesComboBox->setCurrentIndex(j);

	QLabel* label=new QLabel(tr("Please select m-FET language"));

	QHBoxLayout* languagesLayout=new QHBoxLayout();
	languagesLayout->addWidget(languagesComboBox);
	//languagesLayout->addStretch();

	taMainLayout->addStretch();
	taMainLayout->addWidget(label);
	//taMainLayout->addWidget(languagesComboBox);
	taMainLayout->addLayout(languagesLayout);
	taMainLayout->addStretch();
	taMainLayout->addLayout(buttons);

	QObject::connect(tapb2, SIGNAL(clicked()), &dialog, SLOT(accept()));
	QObject::connect(tapb1, SIGNAL(clicked()), &dialog, SLOT(reject()));

	tapb2->setDefault(true);
	tapb2->setFocus();

	const QString settingsName=QString("LanguageSelectionForm");

	int w=dialog.sizeHint().width();
	if(w<350)
		w=350;
	int h=dialog.sizeHint().height();
	if(h<180)
		h=180;
	dialog.resize(w,h);
	centerWidgetOnScreen(&dialog);
	restoreFETDialogGeometry(&dialog, settingsName);

	setParentAndOtherThings(&dialog, this);
	bool ok=dialog.exec();
	saveFETDialogGeometry(&dialog, settingsName);
	if(!ok)
		return;

	//QString newLang=languagesComboBox->currentText();
	int k=languagesComboBox->currentIndex();
	i=0;
	bool found=false;
	QMapIterator<QString, QString> it2(languagesMap);
	while(it2.hasNext()){
		it2.next();
		if(i==k){
			defs::FET_LANGUAGE=it2.key();
			found=true;
		}
		i++;
	}
	if(!found){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid language selected - making it en_US (US English)"));
		defs::FET_LANGUAGE="en_US";
	}

	setLanguage(*pqapplication, this);
	setCurrentFile(defs::INPUT_FILENAME_XML);

	//QMessageBox::information(this, tr("m-FET information"), tr("Language %1 selected").arg( defs::FET_LANGUAGE+" ("+languagesMap.value(defs::FET_LANGUAGE)+")" )+"\n\n"+
	// tr("Please exit and restart m-FET to activate language change"));
}

void MainForm::on_settingsRestoreDefaultsAction_triggered()
{
	QString default_working_directory="examples";
	QDir d2(default_working_directory);
	if(!d2.exists())
		default_working_directory=QDir::homePath();
	else
		default_working_directory=d2.absolutePath();

	QString s=tr("Are you sure you want to reset all settings to defaults?");
	s+="\n\n";

	s+=tr("That means:");
	s+="\n";

	s+=tr("1")+QString(". ")+tr("The geometry and any other saved settings of all the windows and dialogs will be reset to default");
	s+="\n";

	s+=tr("2")+QString(". ")+tr("Show shortcut buttons in main window will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";

	s+=tr("3")+QString(". ")+tr("In the shortcuts tab from the main window, the first section will be selected/shown", "Option refers to the main window tab widget for shortcuts, which currently contains 5 tabs: File, Data, "
		"Time, Space, Timetable (so it will select/show File tab).");
	s+="\n";

	s+=tr("4")+QString(". ")+tr("Check for updates at startup will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("5")+QString(". ")+tr("Use colors in m-FET graphical user interface will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

#ifndef USE_SYSTEM_LOCALE
	s+=tr("6")+QString(". ")+tr("Language will be %1", "%1 is the default language").arg(QString("en_US")+QString(" (")+tr("US English")+QString(")"));
#else
	QMap<QString, QString> languagesMap;
	populateLanguagesMap(languagesMap);

	QString NEW_FET_LANGUAGE=QLocale::system().name();

	bool ok=false;
	QMapIterator<QString, QString> i(languagesMap);
	while(i.hasNext()){
		i.next();
		if(NEW_FET_LANGUAGE.left(i.key().length())==i.key()){
			NEW_FET_LANGUAGE=i.key();
			ok=true;
			break;
		}
	}
	if(!ok)
		NEW_FET_LANGUAGE="en_US";

	assert(languagesMap.contains(NEW_FET_LANGUAGE));
	s+=tr("6")+QString(". ")+tr("Language will be %1", "%1 is the default language").arg(NEW_FET_LANGUAGE+QString(" (")+languagesMap.value(NEW_FET_LANGUAGE)+QString(")"));
#endif
	s+="\n";

	s+=tr("7")+QString(". ")+tr("The list of recently used files will be cleared");
	s+="\n";

	s+=tr("8")+QString(". ")+tr("Working directory will be %1", "%1 is the directory").arg(QDir::toNativeSeparators(default_working_directory));
	s+="\n";

	s+=tr("9")+QString(". ")+tr("Output directory will be %1", "%1 is the directory").arg(QDir::toNativeSeparators(QDir::homePath()+defs::FILE_SEP+"m-fet-results"));
	s+="\n";

	s+=tr("10")+QString(". ")+tr("Import directory will be %1", "%1 is the directory").arg(QDir::toNativeSeparators(QDir::homePath()+defs::FILE_SEP+"m-fet-results"));
	s+="\n";

	s+=tr("11")+QString(". ")+tr("Html level of the timetables will be %1", "%1 is default html level").arg(2);
	s+="\n";

	s+=tr("12")+QString(". ")+tr("Mark not available slots with -x- in timetables will be %1", "%1 is true or false. Lowercase -x-").arg(tr("true"));
	s+="\n";

	s+=tr("13")+QString(". ")+tr("Mark break slots with -X- in timetables will be %1", "%1 is true or false. Uppercase -X-").arg(tr("true"));
	s+="\n";

	s+=tr("14")+QString(". ")+tr("Divide html timetables with time-axis by days will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("15")+QString(". ")+tr("Duplicate vertical headers to the right will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("16")+QString(". ")+tr("Print activities with same starting time will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("17")+QString(". ")+tr("Print activities tags will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";

	s+=tr("18")+QString(". ")+tr("Enable activity tag max hours daily will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("19")+QString(". ")+tr("Enable students max gaps per day will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("20")+QString(". ")+tr("Warn if using not perfect constraints will be %1", "%1 is true or false. This is a warning if user uses not perfect constraints").arg(tr("true"));
	s+="\n";

	s+=tr("21")+QString(". ")+tr("Enable constraints students min hours daily with empty days will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";

	s+=tr("22")+QString(". ")+tr("Warn if using constraints students min hours daily with empty days will be %1", "%1 is true or false. This is a warning if user uses a nonstandard constraint"
		" students min hours daily with allowed empty days").arg(tr("true"));
	s+="\n";

	///////////////confirmations
	s+=tr("23")+QString(". ")+tr("Confirm activity planning will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("24")+QString(". ")+tr("Confirm spread activities over the week will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("25")+QString(". ")+tr("Confirm remove redundant constraints will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("26")+QString(". ")+tr("Confirm save data and timetable as will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	///////////////

	s+=tr("27")+QString(". ")+tr("Enable group activities in the initial order of generation will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";
	s+=tr("28")+QString(". ")+tr("Warn if using group activities in the initial order of generation will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	///////////////

	s+=tr("29")+QString(". ")+tr("Show subgroups in combo boxes will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("30")+QString(". ")+tr("Show subgroups in activity planning will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	///////////////

	s+=tr("31")+QString(". ")+tr("Write on disk the %1 timetable will be %2", "%1 is a category of timetables, like conflicts, %2 is true or false")
	 .arg(tr("conflicts")).arg(tr("true"));
	s+="\n";

	s+=tr("32")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("statistics")).arg(tr("true"));
	s+="\n";
	s+=tr("33")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("XML")).arg(tr("true"));
	s+="\n";
	s+=tr("34")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("days horizontal")).arg(tr("true"));
	s+="\n";
	s+=tr("35")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("days vertical")).arg(tr("true"));
	s+="\n";
	s+=tr("36")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("time horizontal")).arg(tr("true"));
	s+="\n";
	s+=tr("37")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("time vertical")).arg(tr("true"));
	s+="\n";

	s+=tr("38")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("subgroups")).arg(tr("true"));
	s+="\n";
	s+=tr("39")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("groups")).arg(tr("true"));
	s+="\n";
	s+=tr("40")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("years")).arg(tr("true"));
	s+="\n";
	s+=tr("41")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("teachers")).arg(tr("true"));
	s+="\n";
	s+=tr("42")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("teachers free periods")).arg(tr("true"));
	s+="\n";
	s+=tr("43")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("rooms")).arg(tr("true"));
	s+="\n";
	s+=tr("44")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("subjects")).arg(tr("true"));
	s+="\n";
	s+=tr("45")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("activity tags")).arg(tr("true"));
	s+="\n";
	s+=tr("46")+QString(". ")+tr("Write on disk the %1 timetables will be %2", "%1 is a category of timetables, like XML or subgroups timetables, %2 is true or false")
	 .arg(tr("activities")).arg(tr("true"));
	s+="\n";
	s+=tr("47")+QString(". ")+tr("Show tool tips for constraints with tables will be %1", "%1 is true or false").arg(tr("false"));
	s+="\n";
	s+=tr("48")+QString(". ")+tr("Show warning for subgroups with the same activities will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("49")+QString(". ")+tr("Print detailed timetables will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";
	s+=tr("50")+QString(". ")+tr("Print detailed teachers' free periods timetables will be %1", "%1 is true or false").arg(tr("true"));
	s+="\n";

    switch( MessagesManager::confirmation( this, tr("m-FET confirmation"), s,
	 tr("&Yes"), tr("&No"), QString(), 0 , 1 ) ) {
	case 0: // Yes
		break;
	case 1: // No
		return;
	}

    QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.clear();

	recentFiles.clear();
	updateRecentFileActions();

	resize(ORIGINAL_WIDTH, ORIGINAL_HEIGHT);
	//move(ORIGINAL_X, ORIGINAL_Y);
	forceCenterWidgetOnScreen(this);

#ifndef USE_SYSTEM_LOCALE
	defs::FET_LANGUAGE=QString("en_US");
#else
	defs::FET_LANGUAGE=NEW_FET_LANGUAGE;
#endif

	checkForUpdatesAction->setChecked(false);
	defs::checkForUpdates=0;

	SHOW_SHORTCUTS_ON_MAIN_WINDOW=true;
	settingsShowShortcutsOnMainWindowAction->setChecked(SHOW_SHORTCUTS_ON_MAIN_WINDOW);
	tabWidget->setVisible(SHOW_SHORTCUTS_ON_MAIN_WINDOW);

	tabWidget->setCurrentIndex(0);

	defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES=false;
	settingsShowToolTipsForConstraintsWithTablesAction->setChecked(defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES);

	defs::USE_GUI_COLORS=false;
	settingsUseColorsAction->setChecked(defs::USE_GUI_COLORS);

	defs::SHOW_SUBGROUPS_IN_COMBO_BOXES=true;
	settingsShowSubgroupsInComboBoxesAction->setChecked(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES);

	defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING=true;
	settingsShowSubgroupsInActivityPlanningAction->setChecked(defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING);

	///
	defs::WRITE_TIMETABLE_CONFLICTS=true;

	defs::WRITE_TIMETABLES_STATISTICS=true;
	defs::WRITE_TIMETABLES_XML=true;
	defs::WRITE_TIMETABLES_DAYS_HORIZONTAL=true;
	defs::WRITE_TIMETABLES_DAYS_VERTICAL=true;
	defs::WRITE_TIMETABLES_TIME_HORIZONTAL=true;
	defs::WRITE_TIMETABLES_TIME_VERTICAL=true;

	defs::WRITE_TIMETABLES_SUBGROUPS=true;
	defs::WRITE_TIMETABLES_GROUPS=true;
	defs::WRITE_TIMETABLES_YEARS=true;
	defs::WRITE_TIMETABLES_TEACHERS=true;
	defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS=true;
	defs::WRITE_TIMETABLES_ROOMS=true;
	defs::WRITE_TIMETABLES_SUBJECTS=true;
	defs::WRITE_TIMETABLES_ACTIVITY_TAGS=true;
	defs::defs::WRITE_TIMETABLES_ACTIVITIES=true;
	//

	////////confirmations
	defs::CONFIRM_ACTIVITY_PLANNING=true;
	settingsConfirmActivityPlanningAction->setChecked(defs::CONFIRM_ACTIVITY_PLANNING);

	defs::CONFIRM_SPREAD_ACTIVITIES=true;
	settingsConfirmSpreadActivitiesAction->setChecked(defs::CONFIRM_SPREAD_ACTIVITIES);

	defs::CONFIRM_REMOVE_REDUNDANT=true;
	settingsConfirmRemoveRedundantAction->setChecked(defs::CONFIRM_REMOVE_REDUNDANT);

	defs::CONFIRM_SAVE_TIMETABLE=true;
	settingsConfirmSaveTimetableAction->setChecked(defs::CONFIRM_SAVE_TIMETABLE);
	///////

	///////////
	defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES=true;
	showWarningForSubgroupsWithTheSameActivitiesAction->setChecked(defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES);

	defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY=false;
	enableActivityTagMaxHoursDailyAction->setChecked(defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);

	defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY=false;
	enableStudentsMaxGapsPerDayAction->setChecked(defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);

	defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS=true;
	showWarningForNotPerfectConstraintsAction->setChecked(defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS);

	setEnabledIcon(dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);

	setEnabledIcon(dataTimeConstraintsStudentsSetMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
	setEnabledIcon(dataTimeConstraintsStudentsMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);

	defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=false;
	enableStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);

	defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=true;
	showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);

	///////////

	settingsDivideTimetablesByDaysAction->setChecked(false);
	defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS=false;

	settingsDuplicateVerticalNamesAction->setChecked(false);
	defs::TIMETABLE_HTML_REPEAT_NAMES=false;

	defs::WORKING_DIRECTORY=default_working_directory;

	defs::OUTPUT_DIR=QDir::homePath()+defs::FILE_SEP+"m-fet-results";
	QDir dir;
	if(!dir.exists(defs::OUTPUT_DIR))
		dir.mkpath(defs::OUTPUT_DIR);
	defs::IMPORT_DIRECTORY=defs::OUTPUT_DIR;

	defs::TIMETABLE_HTML_LEVEL=2;

	settingsPrintActivityTagsAction->setChecked(true);
	defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS=true;

	settingsPrintDetailedTimetablesAction->setChecked(true);
	defs::PRINT_DETAILED_HTML_TIMETABLES=true;
	settingsPrintDetailedTeachersFreePeriodsTimetablesAction->setChecked(true);
	defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS=true;

	settingsPrintNotAvailableSlotsAction->setChecked(true);
	defs::PRINT_NOT_AVAILABLE_TIME_SLOTS=true;

	settingsPrintBreakSlotsAction->setChecked(true);
	defs::PRINT_BREAK_TIME_SLOTS=true;

	settingsPrintActivitiesWithSameStartingTimeAction->setChecked(false);
	defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME=false;

	setLanguage(*pqapplication, this);
	setCurrentFile(defs::INPUT_FILENAME_XML);

	LockUnlock::increaseCommunicationSpinBox(); //for GUI colors in timetables
}

void MainForm::on_settingsTimetableHtmlLevelAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	SettingsTimetableHtmlLevelForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void MainForm::on_settingsPrintActivityTagsAction_toggled()
{
	defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS=settingsPrintActivityTagsAction->isChecked();
}

void MainForm::on_settingsPrintDetailedTimetablesAction_toggled()
{
	defs::PRINT_DETAILED_HTML_TIMETABLES=settingsPrintDetailedTimetablesAction->isChecked();
}

void MainForm::on_settingsPrintDetailedTeachersFreePeriodsTimetablesAction_toggled()
{
	defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS=settingsPrintDetailedTeachersFreePeriodsTimetablesAction->isChecked();
}

void MainForm::on_settingsPrintNotAvailableSlotsAction_toggled()
{
	defs::PRINT_NOT_AVAILABLE_TIME_SLOTS=settingsPrintNotAvailableSlotsAction->isChecked();
}

void MainForm::on_settingsPrintBreakSlotsAction_toggled()
{
	defs::PRINT_BREAK_TIME_SLOTS=settingsPrintBreakSlotsAction->isChecked();
}

void MainForm::on_settingsPrintActivitiesWithSameStartingTimeAction_toggled()
{
	defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME=settingsPrintActivitiesWithSameStartingTimeAction->isChecked();
}

void MainForm::on_activityPlanningAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

//	if(defs::CONFIRM_ACTIVITY_PLANNING){
//		int confirm;

//		ActivityPlanningConfirmationForm c_form(this);
//		setParentAndOtherThings(&c_form, this);
//		confirm=c_form.exec();

//		if(confirm==QDialog::Accepted){
//			if(c_form.dontShowAgain)
//				settingsConfirmActivityPlanningAction->setChecked(false);

//			StartActivityPlanning::startActivityPlanning(&c_form);
//		}
//	}
//	else{
//		StartActivityPlanning::startActivityPlanning(this);
//	}
}

//void MainForm::on_spreadActivitiesAction_triggered()
//{
//    if(TContext::get()->simulation_running){
//		QMessageBox::information(this, tr("m-FET information"),
//			tr("Allocation in course.\nPlease stop simulation before this."));
//		return;
//	}

//    if(TContext::get()->instance.nDaysPerWeek>=7){
//		QString s;
//		s=tr("You have more than 6 days per week, so probably you won't need this feature. Do you still want to continue?");

//		int cfrm=0;
//		switch( QMessageBox::question( this, tr("m-FET question"),
//		 s,
//         tr("&Continue"), tr("&Cancel"), nullptr, 1 ) ) {
//		case 0: // Yes - continue
//			cfrm=1;
//			break;
//		case 1: // No - cancel
//			cfrm=0;
//			break;
//		}

//		if(!cfrm){
//			return;
//		}
//	}

//    if(TContext::get()->instance.nDaysPerWeek<=4){
//		QString s;
//		s=tr("You have less than 5 days per week, so probably you won't need this feature. Do you still want to continue?");

//		int cfrm=0;
//		switch( QMessageBox::question( this, tr("m-FET question"),
//		 s,
//         tr("&Continue"), tr("&Cancel"), nullptr, 1 ) ) {
//		case 0: // Yes - continue
//			cfrm=1;
//			break;
//		case 1: // No - cancel
//			cfrm=0;
//			break;
//		}

//		if(!cfrm){
//			return;
//		}
//	}

//	if(defs::CONFIRM_SPREAD_ACTIVITIES){
//		int confirm;

//		SpreadConfirmationForm c_form(this);
//		setParentAndOtherThings(&c_form, this);
//		confirm=c_form.exec();

//		if(confirm==QDialog::Accepted){
//			if(c_form.dontShowAgain)
//				settingsConfirmSpreadActivitiesAction->setChecked(false);

//			SpreadMinDaysConstraintsFiveDaysForm form(&c_form);
//			setParentAndOtherThings(&form, &c_form);
//			form.exec();
//		}
//	}
//	else{
//		SpreadMinDaysConstraintsFiveDaysForm form(this);
//		setParentAndOtherThings(&form, this);
//		form.exec();
//	}
//}

void MainForm::on_statisticsExportToDiskAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	StatisticsExport::exportStatistics(this);
}

void MainForm::on_removeRedundantConstraintsAction_triggered()
{
    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	if(defs::CONFIRM_REMOVE_REDUNDANT){
		int confirm;

		RemoveRedundantConfirmationForm c_form(this);
		setParentAndOtherThings(&c_form, this);
		confirm=c_form.exec();

		if(confirm==QDialog::Accepted){
			if(c_form.dontShowAgain)
				settingsConfirmRemoveRedundantAction->setChecked(false);

			RemoveRedundantForm form(&c_form);
			setParentAndOtherThings(&form, &c_form);
			form.exec();
		}
	}
	else{
		RemoveRedundantForm form(this);
		setParentAndOtherThings(&form, this);
		form.exec();
	}
}

void MainForm::on_selectOutputDirAction_triggered()
{
	QString od;

	od = QFileDialog::getExistingDirectory(this, tr("Choose results (output) directory"), defs::OUTPUT_DIR);

	if(!od.isNull()){
		QFile test(od+defs::FILE_SEP+"test_write_permissions_3.tmp");
		bool existedBefore=test.exists();
		bool t=test.open(QIODevice::ReadWrite);
		//if(!test.exists())
		//	t=false;
		if(!t){
			QMessageBox::warning(this, tr("m-FET warning"), tr("You don't have write permissions in this directory"));
			return;
		}
		test.close();
		if(!existedBefore)
			test.remove();

		defs::OUTPUT_DIR=od;
	}
}

void MainForm::enableActivityTagMaxHoursDailyToggled(bool checked)
{
	if(checked==true){
		QString s=tr("These kinds of constraints are good, but not perfectly optimized. Adding such constraints may make your"
		 " timetable solve too slow or even impossible.");
		s+=" ";
		s+=tr("It is recommended to use such constraints only at the end, after you generated successfully with all the other constraints.");
		s+="\n\n";
		s+=tr("If your timetable is too difficult, it may be from these kinds of constraints, so you may need to remove them and retry.");
		s+="\n\n";
		s+=tr("Continue only if you know what you are doing.");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		if(b!=QMessageBox::Ok){
			disconnect(enableActivityTagMaxHoursDailyAction, SIGNAL(toggled(bool)), this, SLOT(enableActivityTagMaxHoursDailyToggled(bool)));
			enableActivityTagMaxHoursDailyAction->setChecked(false);
			connect(enableActivityTagMaxHoursDailyAction, SIGNAL(toggled(bool)), this, SLOT(enableActivityTagMaxHoursDailyToggled(bool)));
			return;
		}
	}

	defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY=checked;

	setEnabledIcon(dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
	setEnabledIcon(dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction, defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
}

void MainForm::enableStudentsMaxGapsPerDayToggled(bool checked)
{
	if(checked==true){
		QString s=tr("These kinds of constraints are good, but not perfectly optimized. Adding such constraints may make your"
		 " timetable solve too slow or even impossible.");
		s+=" ";
		s+=tr("It is recommended to use such constraints only at the end, after you generated successfully with all the other constraints.");
		s+="\n\n";
		s+=tr("If your timetable is too difficult, it may be from these kinds of constraints, so you may need to remove them and retry.");
		s+="\n\n";
		s+=tr("Continue only if you know what you are doing.");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		if(b!=QMessageBox::Ok){
			disconnect(enableStudentsMaxGapsPerDayAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMaxGapsPerDayToggled(bool)));
			enableStudentsMaxGapsPerDayAction->setChecked(false);
			connect(enableStudentsMaxGapsPerDayAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMaxGapsPerDayToggled(bool)));
			return;
		}
	}

	defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY=checked;

	setEnabledIcon(dataTimeConstraintsStudentsSetMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
	setEnabledIcon(dataTimeConstraintsStudentsMaxGapsPerDayAction, defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
}

void MainForm::showWarningForSubgroupsWithTheSameActivitiesToggled(bool checked)
{
	if(checked==false){
		QString s=tr("It is recommended to keep this warning active, but if you really want, you can disable it.");
		s+="\n\n";
		s+=tr("Disable it only if you know what you are doing.");
		s+="\n\n";
		s+=tr("Are you sure you want to disable it?");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

		if(b!=QMessageBox::Yes){
			disconnect(showWarningForSubgroupsWithTheSameActivitiesAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForSubgroupsWithTheSameActivitiesToggled(bool)));
			showWarningForSubgroupsWithTheSameActivitiesAction->setChecked(true);
			connect(showWarningForSubgroupsWithTheSameActivitiesAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForSubgroupsWithTheSameActivitiesToggled(bool)));
			return;
		}
	}

	defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES=checked;
}

void MainForm::showWarningForNotPerfectConstraintsToggled(bool checked)
{
	if(checked==false){
		QString s=tr("It is recommended to keep this warning active, but if you really want, you can disable it.");
		s+="\n\n";
		s+=tr("Disable it only if you know what you are doing.");
		s+="\n\n";
		s+=tr("Are you sure you want to disable it?");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

		if(b!=QMessageBox::Yes){
			disconnect(showWarningForNotPerfectConstraintsAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForNotPerfectConstraintsToggled(bool)));
			showWarningForNotPerfectConstraintsAction->setChecked(true);
			connect(showWarningForNotPerfectConstraintsAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForNotPerfectConstraintsToggled(bool)));
			return;
		}
	}

	defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS=checked;
}


void MainForm::enableStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool checked)
{
	if(checked==true){
		QString s=tr("This kind of constraint is good, but only in the right case. Adding such constraints in the wrong circumstance may make your"
		 " timetable solve too slow or even impossible.");
		s+="\n\n";
		s+=tr("Please use such constraints only if you are sure that there exists a timetable with empty days for students. If your instution requires"
			" each day for the students to be not empty, or if there does not exist a solution with empty days for students, "
			"it is IMPERATIVE (for performance reasons) that you use the standard constraint which does not allow empty days."
			" Otherwise, the timetable may be impossible to find.");
		s+="\n\n";
		s+=tr("Continue only if you know what you are doing.");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);

		if(b!=QMessageBox::Ok){
			disconnect(enableStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));
			enableStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(false);
			connect(enableStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(enableStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));
			return;
		}
	}

	defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=checked;
}

void MainForm::showWarningForStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool checked)
{
	if(checked==false){
		QString s=tr("It is recommended to keep this warning active, but if you really want, you can disable it.");
		s+="\n\n";
		s+=tr("Disable it only if you know what you are doing.");
		s+="\n\n";
		s+=tr("Are you sure you want to disable it?");

		QMessageBox::StandardButton b=QMessageBox::warning(this, tr("m-FET warning"), s, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

		if(b!=QMessageBox::Yes){
			disconnect(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));
			showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction->setChecked(true);
			connect(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysAction, SIGNAL(toggled(bool)), this, SLOT(showWarningForStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool)));
			return;
		}
	}

	defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=checked;
}

//time constraints
void MainForm::on_shortcutAllTimeConstraintsPushButton_clicked()
{
	on_dataAllTimeConstraintsAction_triggered();
}

void MainForm::on_shortcutAdvancedTimeConstraintsPushButton_clicked()
{
	shortcutAdvancedTimeMenu->popup(QCursor::pos());
}

void MainForm::on_shortcutBreakTimeConstraintsPushButton_clicked()
{
	on_dataTimeConstraintsBreakTimesAction_triggered();
}

void MainForm::on_shortcutTeachersTimeConstraintsPushButton_clicked()
{
	menuTeachers_time_constraints->popup(QCursor::pos());
}

void MainForm::on_shortcutStudentsTimeConstraintsPushButton_clicked()
{
	menuStudents_time_constraints->popup(QCursor::pos());
}

void MainForm::on_shortcutActivitiesTimeConstraintsPushButton_clicked()
{
	menuActivities_time_constraints->popup(QCursor::pos());
}

//timetable
void MainForm::on_shortcutGeneratePushButton_clicked()
{
	on_timetableGenerateAction_triggered();
}

void MainForm::on_shortcutGenerateMultiplePushButton_clicked()
{
	on_timetableGenerateMultipleAction_triggered();
}

void MainForm::on_shortcutsTimetableLockingPushButton_clicked()
{
	shortcutTimetableLockingMenu->popup(QCursor::pos());
}

//data shortcut
void MainForm::on_shortcutBasicPushButton_clicked()
{
	shortcutBasicMenu->popup(QCursor::pos());
}

void MainForm::on_shortcutSubjectsPushButton_clicked()
{
	on_dataSubjectsAction_triggered();
}

void MainForm::on_shortcutActivityTagsPushButton_clicked()
{
    on_dataActivityTagsAction_triggered();
}

void MainForm::on_shortcutTeachersPushButton_clicked()
{
	on_dataTeachersAction_triggered();
}

void MainForm::on_shortcutStudentsPushButton_clicked()
{
	menuStudents->popup(QCursor::pos());
}

void MainForm::on_shortcutActivitiesPushButton_clicked()
{
	on_dataActivitiesAction_triggered();
}

void MainForm::on_shortcutDataAdvancedPushButton_clicked()
{
	shortcutDataAdvancedMenu->popup(QCursor::pos());
}

//file shortcut
void MainForm::on_shortcutNewPushButton_clicked()
{
	on_fileNewAction_triggered();
}

void MainForm::on_shortcutOpenPushButton_clicked()
{
	on_fileOpenAction_triggered();
}

void MainForm::on_shortcutOpenRecentPushButton_clicked()
{
	fileOpenRecentMenu->popup(QCursor::pos());
}

void MainForm::on_shortcutSavePushButton_clicked()
{
	on_fileSaveAction_triggered();
}

void MainForm::on_shortcutSaveAsPushButton_clicked()
{
	on_fileSaveAsAction_triggered();
}

void MainForm::on_generationSettingsPushButton_clicked()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    TimetableGenerateSettingsForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMax_number_of_activities_for_a_teacher_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintMaxActivitiesForTeacherForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMax_number_of_activities_for_all_teacher_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintMaxActivitiesForAllTeachersForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMin_number_of_activities_for_all_teachers_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintMinActivitiesForAllTeachersForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMin_number_of_activities_for_a_teacher_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintMinActivitiesForTeacherForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMax_hours_in_work_daily_for_a_teacher_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintTeacherMaxHoursInWorkDailyForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionMax_hours_in_work_daily_for_all_teacher_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintTeachersMaxHoursInWorkDailyForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_actionSubject_preference_to_avoid_for_all_teachers_triggered()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    ConstraintNoTeacherInSubjectPreferenceForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void MainForm::on_shortcutShowSoftConflictsPushButton_clicked()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    TimetableShowSolutionsForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

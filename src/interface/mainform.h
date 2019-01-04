
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef FETMAINFORM_H
#define FETMAINFORM_H

#include <QtGlobal>

#include "ui_mainform_template.h"

#include <QMutex>
#include <QThread>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QResizeEvent>
#include <QCloseEvent>

#include <QString>
#include <QStringList>

#include <QAction>

#include <QMap>

class QNetworkAccessManager;
class QNetworkReply;


const int MAX_RECENT_FILES=10;

class MainForm: public QMainWindow, public Ui::MainForm_template
{
	Q_OBJECT

private:
	QSpinBox communicationSpinBox;

	QMenu* shortcutBasicMenu;
	QMenu* shortcutDataSpaceMenu;
	QMenu* shortcutDataAdvancedMenu;
	QMenu* shortcutAdvancedTimeMenu;
	//2014-07-01
	QMenu* shortcutTimetableLockingMenu;
	QMenu* shortcutTimetableAdvancedMenu;

	QNetworkAccessManager* networkManager;

	QStringList recentFiles;

	QAction* recentFileActions[MAX_RECENT_FILES];
	QAction* recentSeparatorAction;

	void setEnabledIcon(QAction* action, bool enabled);

	void setCurrentFile(const QString& fileName);
	QString strippedName(const QString& fullFileName);
	void updateRecentFileActions();

	void populateLanguagesMap(QMap<QString, QString>& languagesMap);

public:
    MainForm();
    ~MainForm();

	void closeOtherWindows();

	void openFile(const QString& fileName);
	bool fileSave();
	bool fileSaveAs();

public slots:
    void updateDescription();

	void enableNotPerfectMessage();

	void on_fileNewAction_triggered();
	void on_fileSaveAction_triggered();
	void on_fileSaveAsAction_triggered();
	void on_fileExitAction_triggered();
	void on_fileOpenAction_triggered();
	void on_fileClearRecentFilesListAction_triggered();
	void openRecentFile();

	void on_fileImportCSVActivityTagsAction_triggered();
	void on_fileImportCSVActivitiesAction_triggered();
	void on_fileImportCSVRoomsBuildingsAction_triggered();
	void on_fileImportCSVSubjectsAction_triggered();
	void on_fileImportCSVTeachersAction_triggered();
	void on_fileImportCSVYearsGroupsSubgroupsAction_triggered();
	void on_fileExportCSVAction_triggered();

	void on_dataInstitutionNameAction_triggered();
	void on_dataCommentsAction_triggered();
	void on_dataDaysAction_triggered();
	void on_dataHoursAction_triggered();
	void on_dataTeachersAction_triggered();
	void on_dataTeachersStatisticsAction_triggered();
	void on_dataSubjectsAction_triggered();
	void on_dataSubjectsStatisticsAction_triggered();
    void on_dataActivityTagsAction_triggered();
	void on_dataYearsAction_triggered();
	void on_dataGroupsAction_triggered();
	void on_dataSubgroupsAction_triggered();
	void on_dataStudentsStatisticsAction_triggered();
	void on_dataTeachersSubjectsQualificationsStatisticsAction_triggered();
	void on_dataHelpOnStatisticsAction_triggered();

	void on_helpSettingsAction_triggered();
	void on_settingsUseColorsAction_toggled();
	void showSubgroupsInComboBoxesToggled(bool checked);
	void showSubgroupsInActivityPlanningToggled(bool checked);
	void on_settingsShowShortcutsOnMainWindowAction_toggled();
	void on_settingsShowToolTipsForConstraintsWithTablesAction_toggled();

	void on_timetablesToWriteOnDiskAction_triggered();
	///

	//////confirmations
	void on_settingsConfirmActivityPlanningAction_toggled();
	void on_settingsConfirmSpreadActivitiesAction_toggled();
	void on_settingsConfirmRemoveRedundantAction_toggled();
	void on_settingsConfirmSaveTimetableAction_toggled();
	//////

	void showWarningForSubgroupsWithTheSameActivitiesToggled(bool checked);

	void enableActivityTagMaxHoursDailyToggled(bool checked);
	void enableStudentsMaxGapsPerDayToggled(bool checked);
	void showWarningForNotPerfectConstraintsToggled(bool checked);

	void enableStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool checked);
	void showWarningForStudentsMinHoursDailyWithAllowEmptyDaysToggled(bool checked);

	void on_dataActivitiesAction_triggered();
	void on_dataAllTimeConstraintsAction_triggered();

	void on_dataTimeConstraintsBreakTimesAction_triggered();

	void on_dataTimeConstraintsTwoActivitiesConsecutiveAction_triggered();
	void on_dataTimeConstraintsTwoActivitiesGroupedAction_triggered();
	void on_dataTimeConstraintsThreeActivitiesGroupedAction_triggered();
	void on_dataTimeConstraintsTwoActivitiesOrderedAction_triggered();
	void on_dataTimeConstraintsActivityPreferredStartingTimeAction_triggered();
	void on_dataTimeConstraintsActivityPreferredTimeSlotsAction_triggered();
	void on_dataTimeConstraintsActivitiesPreferredTimeSlotsAction_triggered();
	void on_dataTimeConstraintsSubactivitiesPreferredTimeSlotsAction_triggered();
	void on_dataTimeConstraintsActivityPreferredStartingTimesAction_triggered();
	void on_dataTimeConstraintsActivitiesPreferredStartingTimesAction_triggered();
	void on_dataTimeConstraintsSubactivitiesPreferredStartingTimesAction_triggered();
	void on_dataTimeConstraintsActivitiesSameStartingTimeAction_triggered();
	void on_dataTimeConstraintsActivitiesSameStartingHourAction_triggered();
	void on_dataTimeConstraintsActivitiesSameStartingDayAction_triggered();
	void on_dataTimeConstraintsActivitiesOccupyMaxTimeSlotsFromSelectionAction_triggered();
	void on_dataTimeConstraintsActivitiesMaxSimultaneousInSelectedTimeSlotsAction_triggered();
	void on_dataTimeConstraintsActivitiesNotOverlappingAction_triggered();
	void on_dataTimeConstraintsMinDaysBetweenActivitiesAction_triggered();
	void on_dataTimeConstraintsMaxDaysBetweenActivitiesAction_triggered();
	void on_dataTimeConstraintsMinGapsBetweenActivitiesAction_triggered();
	void on_dataTimeConstraintsActivityEndsStudentsDayAction_triggered();
	void on_dataTimeConstraintsActivitiesEndStudentsDayAction_triggered();

	void on_dataTimeConstraintsTeacherNotAvailableTimesAction_triggered();
	void on_dataTimeConstraintsTeacherMaxDaysPerWeekAction_triggered();
	void on_dataTimeConstraintsTeachersMaxDaysPerWeekAction_triggered();

	void on_dataTimeConstraintsTeacherMinDaysPerWeekAction_triggered();
	void on_dataTimeConstraintsTeachersMinDaysPerWeekAction_triggered();

	void on_dataTimeConstraintsTeachersMaxHoursDailyAction_triggered();
	void on_dataTimeConstraintsTeacherMaxHoursDailyAction_triggered();
	void on_dataTimeConstraintsTeachersMaxHoursContinuouslyAction_triggered();
	void on_dataTimeConstraintsTeacherMaxHoursContinuouslyAction_triggered();

    void on_dataTimeConstraintsTeachersActivityTagMaxHoursContinuouslyAction_triggered();
    void on_dataTimeConstraintsTeacherActivityTagMaxHoursContinuouslyAction_triggered();

    void on_dataTimeConstraintsTeachersActivityTagMaxHoursDailyAction_triggered();
    void on_dataTimeConstraintsTeacherActivityTagMaxHoursDailyAction_triggered();

	void on_dataTimeConstraintsTeachersMinHoursDailyAction_triggered();
	void on_dataTimeConstraintsTeacherMinHoursDailyAction_triggered();
	void on_dataTimeConstraintsTeachersMaxGapsPerWeekAction_triggered();
	void on_dataTimeConstraintsTeacherMaxGapsPerWeekAction_triggered();
	void on_dataTimeConstraintsTeachersMaxGapsPerDayAction_triggered();
    void on_dataTimeConstraintsTeacherMaxGapsPerDayAction_triggered();

	void on_dataTimeConstraintsTeacherIntervalMaxDaysPerWeekAction_triggered();
	void on_dataTimeConstraintsTeachersIntervalMaxDaysPerWeekAction_triggered();
	void on_dataTimeConstraintsStudentsSetIntervalMaxDaysPerWeekAction_triggered();
    void on_dataTimeConstraintsStudentsIntervalMaxDaysPerWeekAction_triggered();

	void on_dataTimeConstraintsStudentsSetMaxDaysPerWeekAction_triggered();
	void on_dataTimeConstraintsStudentsMaxDaysPerWeekAction_triggered();

	void on_dataTimeConstraintsStudentsSetNotAvailableTimesAction_triggered();
	void on_dataTimeConstraintsStudentsSetMaxGapsPerWeekAction_triggered();
	void on_dataTimeConstraintsStudentsMaxGapsPerWeekAction_triggered();

	void on_dataTimeConstraintsStudentsSetMaxGapsPerDayAction_triggered();
	void on_dataTimeConstraintsStudentsMaxGapsPerDayAction_triggered();

	void on_dataTimeConstraintsStudentsEarlyMaxBeginningsAtSecondHourAction_triggered();
	void on_dataTimeConstraintsStudentsSetEarlyMaxBeginningsAtSecondHourAction_triggered();
	void on_dataTimeConstraintsStudentsMaxHoursDailyAction_triggered();
	void on_dataTimeConstraintsStudentsSetMaxHoursDailyAction_triggered();
	void on_dataTimeConstraintsStudentsMaxHoursContinuouslyAction_triggered();
	void on_dataTimeConstraintsStudentsSetMaxHoursContinuouslyAction_triggered();

    void on_dataTimeConstraintsStudentsActivityTagMaxHoursContinuouslyAction_triggered();
    void on_dataTimeConstraintsStudentsSetActivityTagMaxHoursContinuouslyAction_triggered();

    void on_dataTimeConstraintsStudentsActivityTagMaxHoursDailyAction_triggered();
    void on_dataTimeConstraintsStudentsSetActivityTagMaxHoursDailyAction_triggered();

	void on_dataTimeConstraintsStudentsMinHoursDailyAction_triggered();
	void on_dataTimeConstraintsStudentsSetMinHoursDailyAction_triggered();

	void on_dataTimeConstraintsTeacherMaxSpanPerDayAction_triggered();
	void on_dataTimeConstraintsTeachersMaxSpanPerDayAction_triggered();
	void on_dataTimeConstraintsStudentsSetMaxSpanPerDayAction_triggered();
	void on_dataTimeConstraintsStudentsMaxSpanPerDayAction_triggered();

	void on_dataTimeConstraintsTeacherMinRestingHoursAction_triggered();
	void on_dataTimeConstraintsTeachersMinRestingHoursAction_triggered();
	void on_dataTimeConstraintsStudentsSetMinRestingHoursAction_triggered();
	void on_dataTimeConstraintsStudentsMinRestingHoursAction_triggered();

	void on_activityPlanningAction_triggered();
//	void on_spreadActivitiesAction_triggered();
	void on_removeRedundantConstraintsAction_triggered();

	//about
	void on_helpAboutAction_triggered();
	//offline
	void on_helpFAQAction_triggered();
	void on_helpTipsAction_triggered();
	void on_helpInstructionsAction_triggered();
	//online
//	void on_helpHomepageAction_triggered();
//	void on_helpContentsAction_triggered();
//	void on_helpForumAction_triggered();
//	void on_helpAddressesAction_triggered();

	void on_timetableGenerateAction_triggered();
	void on_timetableGenerateMultipleAction_triggered();

	void on_timetableLockAllActivitiesAction_triggered();
	void on_timetableUnlockAllActivitiesAction_triggered();
	void on_timetableLockActivitiesDayAction_triggered();
	void on_timetableUnlockActivitiesDayAction_triggered();
	void on_timetableLockActivitiesEndStudentsDayAction_triggered();
	void on_timetableUnlockActivitiesEndStudentsDayAction_triggered();

	void on_timetableSaveTimetableAsAction_triggered();

	void on_languageAction_triggered();

	void on_checkForUpdatesAction_toggled();

	void on_settingsDivideTimetablesByDaysAction_toggled();
	void on_settingsDuplicateVerticalNamesAction_toggled();

	void on_settingsRestoreDefaultsAction_triggered();

	void on_settingsTimetableHtmlLevelAction_triggered();
	void on_settingsPrintActivityTagsAction_toggled();
	void on_settingsPrintDetailedTimetablesAction_toggled();
	void on_settingsPrintDetailedTeachersFreePeriodsTimetablesAction_toggled();
	void on_settingsPrintNotAvailableSlotsAction_toggled();
	void on_settingsPrintBreakSlotsAction_toggled();

	void on_settingsPrintActivitiesWithSameStartingTimeAction_toggled();

	void on_selectOutputDirAction_triggered();

	void on_statisticsExportToDiskAction_triggered();
	void on_statisticsPrintAction_triggered();

	void on_shortcutAllTimeConstraintsPushButton_clicked();
	void on_shortcutBreakTimeConstraintsPushButton_clicked();
	void on_shortcutTeachersTimeConstraintsPushButton_clicked();
	void on_shortcutStudentsTimeConstraintsPushButton_clicked();
	void on_shortcutActivitiesTimeConstraintsPushButton_clicked();
	void on_shortcutAdvancedTimeConstraintsPushButton_clicked();

	void on_shortcutGeneratePushButton_clicked();
	void on_shortcutGenerateMultiplePushButton_clicked();
	//2014-07-01
    void on_shortcutsTimetableLockingPushButton_clicked();

	void on_shortcutBasicPushButton_clicked();
	void on_shortcutSubjectsPushButton_clicked();
    void on_shortcutActivityTagsPushButton_clicked();
	void on_shortcutTeachersPushButton_clicked();
	void on_shortcutStudentsPushButton_clicked();
	void on_shortcutActivitiesPushButton_clicked();
	void on_shortcutDataAdvancedPushButton_clicked();

	void on_shortcutOpenPushButton_clicked();
	void on_shortcutOpenRecentPushButton_clicked();
	void on_shortcutNewPushButton_clicked();
	void on_shortcutSavePushButton_clicked();
	void on_shortcutSaveAsPushButton_clicked();

	void replyFinished(QNetworkReply* networkReply);

protected:
	void closeEvent(QCloseEvent* event);
private slots:
    void on_generationSettingsPushButton_clicked();
    void on_actionMax_number_of_activities_for_a_teacher_triggered();
    void on_actionMax_number_of_activities_for_all_teacher_triggered();
    void on_actionMax_hours_in_work_daily_for_a_teacher_triggered();
    void on_actionMax_hours_in_work_daily_for_all_teacher_triggered();
    void on_actionSubject_preference_to_avoid_for_all_teachers_triggered();
    void on_actionMin_number_of_activities_for_a_teacher_triggered();
    void on_actionMin_number_of_activities_for_all_teachers_triggered();
    void on_shortcutShowSoftConflictsPushButton_clicked();
};

#endif

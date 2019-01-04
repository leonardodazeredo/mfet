//
//
// Description: This file is part of m-FET
//
//
// Author: Lalescu Liviu <Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)>
// Copyright (C) 2016 Liviu Lalescu <http://lalescu.ro/liviu/>
//
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"

#include "timetablestowriteondiskform.h"

#include "centerwidgetonscreen.h"


TimetablesToWriteOnDiskForm::TimetablesToWriteOnDiskForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	okPushButton->setDefault(true);
	
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(wasAccepted()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(wasCanceled()));
	
	softConflictsCheckBox->setChecked(defs::WRITE_TIMETABLE_CONFLICTS);

	statisticsCheckBox->setChecked(defs::WRITE_TIMETABLES_STATISTICS);
	xmlCheckBox->setChecked(defs::WRITE_TIMETABLES_XML);
	daysHorizontalCheckBox->setChecked(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL);
	daysVerticalCheckBox->setChecked(defs::WRITE_TIMETABLES_DAYS_VERTICAL);
	timeHorizontalCheckBox->setChecked(defs::WRITE_TIMETABLES_TIME_HORIZONTAL);
	timeVerticalCheckBox->setChecked(defs::WRITE_TIMETABLES_TIME_VERTICAL);

	subgroupsCheckBox->setChecked(defs::WRITE_TIMETABLES_SUBGROUPS);
	groupsCheckBox->setChecked(defs::WRITE_TIMETABLES_GROUPS);
	yearsCheckBox->setChecked(defs::WRITE_TIMETABLES_YEARS);
	teachersCheckBox->setChecked(defs::WRITE_TIMETABLES_TEACHERS);
	teachersFreePeriodsCheckBox->setChecked(defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS);
	roomsCheckBox->setChecked(defs::WRITE_TIMETABLES_ROOMS);
	subjectsCheckBox->setChecked(defs::WRITE_TIMETABLES_SUBJECTS);
	activityTagsCheckBox->setChecked(defs::WRITE_TIMETABLES_ACTIVITY_TAGS);
	activitiesCheckBox->setChecked(defs::defs::WRITE_TIMETABLES_ACTIVITIES);
}

TimetablesToWriteOnDiskForm::~TimetablesToWriteOnDiskForm()
{
	saveFETDialogGeometry(this);
}

void TimetablesToWriteOnDiskForm::wasAccepted()
{
	defs::WRITE_TIMETABLE_CONFLICTS=softConflictsCheckBox->isChecked();

	defs::WRITE_TIMETABLES_STATISTICS=statisticsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_XML=xmlCheckBox->isChecked();
	defs::WRITE_TIMETABLES_DAYS_HORIZONTAL=daysHorizontalCheckBox->isChecked();
	defs::WRITE_TIMETABLES_DAYS_VERTICAL=daysVerticalCheckBox->isChecked();
	defs::WRITE_TIMETABLES_TIME_HORIZONTAL=timeHorizontalCheckBox->isChecked();
	defs::WRITE_TIMETABLES_TIME_VERTICAL=timeVerticalCheckBox->isChecked();

	defs::WRITE_TIMETABLES_SUBGROUPS=subgroupsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_GROUPS=groupsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_YEARS=yearsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_TEACHERS=teachersCheckBox->isChecked();
	defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS=teachersFreePeriodsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_ROOMS=roomsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_SUBJECTS=subjectsCheckBox->isChecked();
	defs::WRITE_TIMETABLES_ACTIVITY_TAGS=activityTagsCheckBox->isChecked();
	defs::defs::WRITE_TIMETABLES_ACTIVITIES=activitiesCheckBox->isChecked();

	this->accept();
}

void TimetablesToWriteOnDiskForm::wasCanceled()
{
	this->reject();
}

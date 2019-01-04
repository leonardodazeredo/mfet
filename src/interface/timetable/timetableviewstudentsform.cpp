/***************************************************************************
                          timetableviewstudentsform.cpp  -  description
                             -------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003 by Lalescu Liviu
    email                : Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include <QtGlobal>

#include "tablewidgetupdatebug.h"

#include "textmessages.h"

#include "mainform.h"
#include "timetableviewstudentsform.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"
#include "solution.h"

#include "matrix.h"

#include "lockunlock.h"

#include <QSplitter>
#include <QList>

#include <QMessageBox>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

#include <QAbstractItemView>

#include <QCoreApplication>
#include <QApplication>

#include <QString>
#include <QStringList>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>

#include "interfaceutils.h"

#include "addallocationform.h"

//begin by Marco Vassura
#include <QBrush>
#include <QColor>
//end by Marco Vassura




extern QSet <int> idsOfLockedTime;		//care about locked activities in view forms
extern QSet <int> idsOfLockedSpace;		//care about locked activities in view forms
extern QSet <int> idsOfPermanentlyLockedTime;	//care about locked activities in view forms
extern QSet <int> idsOfPermanentlyLockedSpace;	//care about locked activities in view forms

extern CommunicationSpinBox communicationSpinBox;	//small hint to sync the forms

TimetableViewStudentsForm::TimetableViewStudentsForm(QWidget* parent, Solution &solution): QDialog(parent), solution(solution)
{
	setupUi(this);

    connect(this, SIGNAL(manualSolutionAdded()), parent, SLOT(solutionsChanged()));

    bool auxb = !this->solution.complete();
    addPushButton->setEnabled(auxb);
    savePushButton->setEnabled(auxb);
//    lockGroupBox->setEnabled(!auxb);

	closePushButton->setDefault(true);
	
	detailsTextEdit->setReadOnly(true);

	verticalStudentsTableDetailsSplitter->setStretchFactor(0, 4);
	verticalStudentsTableDetailsSplitter->setStretchFactor(1, 1);
	horizontalSplitter->setStretchFactor(0, 3);
	horizontalSplitter->setStretchFactor(1, 10);
	
	studentsTimetableTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	yearsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	groupsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	subgroupsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	groupsListWidget->clear();
	subgroupsListWidget->clear();
	
	connect(yearsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(yearChanged(const QString&)));
	connect(groupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(groupChanged(const QString&)));
	connect(subgroupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(subgroupChanged(const QString&)));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(studentsTimetableTable, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));
	connect(lockTimePushButton, SIGNAL(clicked()), this, SLOT(lockTime()));
	connect(lockSpacePushButton, SIGNAL(clicked()), this, SLOT(lockSpace()));
	connect(lockTimeSpacePushButton, SIGNAL(clicked()), this, SLOT(lockTimeSpace()));
	connect(helpPushButton, SIGNAL(clicked()), this, SLOT(help()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	//restore vertical students list splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/vertical-students-list-splitter-state")))
		verticalStudentsListSplitter->restoreState(settings.value(this->metaObject()->className()+QString("/vertical-students-list-splitter-state")).toByteArray());

	//restore vertical students table details splitter state
	//QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/vertical-students-table-details-splitter-state")))
		verticalStudentsTableDetailsSplitter->restoreState(settings.value(this->metaObject()->className()+QString("/vertical-students-table-details-splitter-state")).toByteArray());

	//restore horizontal splitter state
	//QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/horizontal-splitter-state")))
		horizontalSplitter->restoreState(settings.value(this->metaObject()->className()+QString("/horizontal-splitter-state")).toByteArray());

//////////just for testing
	QSet<int> backupLockedTime;
	QSet<int> backupPermanentlyLockedTime;
	QSet<int> backupLockedSpace;
	QSet<int> backupPermanentlyLockedSpace;

	backupLockedTime=idsOfLockedTime;
	backupPermanentlyLockedTime=idsOfPermanentlyLockedTime;
	backupLockedSpace=idsOfLockedSpace;
	backupPermanentlyLockedSpace=idsOfPermanentlyLockedSpace;

	//added by Volker Dirr
	//these lines are not really needed - just to be safer
	LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
	
	assert(backupLockedTime==idsOfLockedTime);
	assert(backupPermanentlyLockedTime==idsOfPermanentlyLockedTime);
	assert(backupLockedSpace==idsOfLockedSpace);
	assert(backupPermanentlyLockedSpace==idsOfPermanentlyLockedSpace);
//////////

	LockUnlock::increaseCommunicationSpinBox();

    studentsTimetableTable->setRowCount(TContext::get()->instance.nHoursPerDay);
    studentsTimetableTable->setColumnCount(TContext::get()->instance.nDaysPerWeek);
    for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
        QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.daysOfTheWeek[j]);
		studentsTimetableTable->setHorizontalHeaderItem(j, item);
	}
    for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
        QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.hoursOfTheDay[i]);
		studentsTimetableTable->setVerticalHeaderItem(i, item);
	}
    for(int j=0; j<TContext::get()->instance.nHoursPerDay; j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek; k++){
			QTableWidgetItem* item= new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

			studentsTimetableTable->setItem(j, k, item);

			//if(j==0 && k==0)
			//	teachersTimetableTable->setCurrentItem(item);
		}
	}
	
	//resize columns
	//if(!columnResizeModeInitialized){
	studentsTimetableTable->horizontalHeader()->setMinimumSectionSize(studentsTimetableTable->horizontalHeader()->defaultSectionSize());
	//	columnResizeModeInitialized=true;
#if QT_VERSION >= 0x050000
	studentsTimetableTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
	studentsTimetableTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
	//}
	///////////////
	
	yearsListWidget->clear();
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
		yearsListWidget->addItem(sty->name);
	}
	if(yearsListWidget->count()>0)
		yearsListWidget->setCurrentRow(0);

	//added by Volker Dirr
	connect(&communicationSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateStudentsTimetableTable()));

    updateStudentsTimetableTable();
}

TimetableViewStudentsForm::~TimetableViewStudentsForm()
{
	saveFETDialogGeometry(this);

	//save vertical students list splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/vertical-students-list-splitter-state"), verticalStudentsListSplitter->saveState());

	//save vertical students table details splitter state
	//QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/vertical-students-table-details-splitter-state"), verticalStudentsTableDetailsSplitter->saveState());

	//save horizontal splitter state
	//QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/horizontal-splitter-state"), horizontalSplitter->saveState());
}

void TimetableViewStudentsForm::resizeRowsAfterShow()
{
	studentsTimetableTable->resizeRowsToContents();
//	tableWidgetUpdateBug(studentsTimetableTable);
}

void TimetableViewStudentsForm::yearChanged(const QString &yearName)
{
	if(yearName==QString())
		return;
    int yearIndex=TContext::get()->instance.searchAugmentedYear(yearName);
	if(yearIndex<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid year - please close this dialog and open a new students view timetable dialog"));
		return;
	}

	disconnect(groupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(groupChanged(const QString&)));

	groupsListWidget->clear();
    StudentsYear* sty=TContext::get()->instance.augmentedYearsList.at(yearIndex);
	for(int i=0; i<sty->groupsList.size(); i++){
		StudentsGroup* stg=sty->groupsList[i];
		groupsListWidget->addItem(stg->name);
	}

	connect(groupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(groupChanged(const QString&)));

	if(groupsListWidget->count()>0)
		groupsListWidget->setCurrentRow(0);
}

void TimetableViewStudentsForm::groupChanged(const QString &groupName)
{
	if(groupName==QString())
		return;

	QString yearName=yearsListWidget->currentItem()->text();
    int yearIndex=TContext::get()->instance.searchAugmentedYear(yearName);
	if(yearIndex<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid year - please close this dialog and open a new students view timetable dialog"));
		return;
	}

    StudentsYear* sty=TContext::get()->instance.augmentedYearsList.at(yearIndex);
    int groupIndex=TContext::get()->instance.searchAugmentedGroup(yearName, groupName);
	if(groupIndex<0){
		QMessageBox::warning(this, tr("m-FET warning"),
		 tr("Invalid group in the selected year, or the groups of the current year are not updated")+
		 "\n\n"+
		 tr("Solution: please try to select a different year and after that select the current year again, "
		 "to refresh the groups list, or close this dialog and open again the students view timetable dialog"));
		return;
	}
	
	disconnect(subgroupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(subgroupChanged(const QString&)));

	subgroupsListWidget->clear();
	
	StudentsGroup* stg=sty->groupsList.at(groupIndex);
	for(int i=0; i<stg->subgroupsList.size(); i++){
		StudentsSubgroup* sts=stg->subgroupsList[i];
		subgroupsListWidget->addItem(sts->name);
	}

	connect(subgroupsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(subgroupChanged(const QString&)));

	if(subgroupsListWidget->count()>0)
		subgroupsListWidget->setCurrentRow(0);
}

void TimetableViewStudentsForm::subgroupChanged(const QString &subgroupName)
{
	Q_UNUSED(subgroupName);
	
	updateStudentsTimetableTable();
}

void TimetableViewStudentsForm::updateStudentsTimetableTable(){

    this->solution.computeSubgroupsTimetable();

	QString s;
	QString yearname;
	QString groupname;
	QString subgroupname;

	if(yearsListWidget->currentRow()<0 || yearsListWidget->currentRow()>=yearsListWidget->count())
		return;
	if(groupsListWidget->currentRow()<0 || groupsListWidget->currentRow()>=groupsListWidget->count())
		return;
	if(subgroupsListWidget->currentRow()<0 || subgroupsListWidget->currentRow()>=subgroupsListWidget->count())
		return;

	yearname = yearsListWidget->currentItem()->text();
	groupname = groupsListWidget->currentItem()->text();
	subgroupname = subgroupsListWidget->currentItem()->text();

    StudentsSubgroup* sts=(StudentsSubgroup*)TContext::get()->instance.searchAugmentedStudentsSet(subgroupname);
	if(sts==nullptr){
		QMessageBox::information(this, tr("m-FET warning"), tr("You have an old timetable view students dialog opened - please close it"));
		return;
	}

	s="";
	s += subgroupname;

	classNameTextLabel->setText(s);

    assert(TContext::get()->instance.initialized);

	assert(sts);
	int i;
    for(i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        if(TContext::get()->instance.directSubgroupsList[i]==sts)
			break;

    assert(i<TContext::get()->instance.directSubgroupsList.size());
    for(int j=0; j<TContext::get()->instance.nHoursPerDay && j<studentsTimetableTable->rowCount(); j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek && k<studentsTimetableTable->columnCount(); k++){
			//begin by Marco Vassura
			// add colors (start)
			//if(defs::USE_GUI_COLORS) {
				studentsTimetableTable->item(j, k)->setBackground(studentsTimetableTable->palette().color(QPalette::Base));
				studentsTimetableTable->item(j, k)->setForeground(studentsTimetableTable->palette().color(QPalette::Text));
			//}
			// add colors (end)
			//end by Marco Vassura
			s="";
            int ai=solution.students_timetable_weekly[i][k][j]; //activity index
			if(ai!=UNALLOCATED_ACTIVITY){
                Activity* act=TContext::get()->instance.activeActivitiesList[ai];
				assert(act!=nullptr);
				
                assert(act->studentSetsNames.size()>=1);
                if((act->studentSetsNames.count()==1 && act->studentSetsNames.at(0)!=subgroupname) || act->studentSetsNames.count()>=2){
                    s+=act->studentSetsNames.join(", ");
                    s+="\n";
                }
				
				if(defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS){
					QString ats=act->activityTagsNames.join(", ");
					s+=act->subjectName +" "+ ats;
				}
				else{
					s+=act->subjectName;
				}

                if(solution.teachers[ai]!=UNALLOCATED_TEACHER){
                    Teacher* tchAct = TContext::get()->instance.teachersList[solution.teachers[ai]];

                    s+="\n";
                    s+=tchAct->name;
                }
                else{
                    s+="\nERROR";
                }

				//added by Volker Dirr (start)
				QString descr="";
				QString t="";
				if(idsOfPermanentlyLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "PLT", "Abbreviation for permanently locked time. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
					t=", ";
				}
				else if(idsOfLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "LT", "Abbreviation for locked time. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
					t=", ";
				}
				if(idsOfPermanentlyLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "PLS", "Abbreviation for permanently locked space. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
				}
				else if(idsOfLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "LS", "Abbreviation for locked space. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
				}
				if(descr!=""){
					descr.prepend("\n(");
					descr.append(")");
				}
				s+=descr;
				//added by Volker Dirr (end)
				
				//begin by Marco Vassura
				// add colors (start)
				if(defs::USE_GUI_COLORS) {
					QBrush bg(stringToColor(act->subjectName));
					studentsTimetableTable->item(j, k)->setBackground(bg);
					double brightness = bg.color().redF()*0.299 + bg.color().greenF()*0.587 + bg.color().blueF()*0.114;
					if (brightness<0.5)
						studentsTimetableTable->item(j, k)->setForeground(QBrush(Qt::white));
					else
						studentsTimetableTable->item(j, k)->setForeground(QBrush(Qt::black));
				}
				// add colors (end)
				//end by Marco Vassura
			}
			else{
                if(TContext::get()->instance.subgroupNotAvailableDayHour[i][k][j] && defs::PRINT_NOT_AVAILABLE_TIME_SLOTS)
					s+="-x-";
                else if(TContext::get()->instance.breakDayHour[k][j] && defs::PRINT_BREAK_TIME_SLOTS)
					s+="-X-";
			}
			studentsTimetableTable->item(j, k)->setText(s);
		}
	}

	studentsTimetableTable->resizeRowsToContents();
	
	tableWidgetUpdateBug(studentsTimetableTable);
	
	detailActivity(studentsTimetableTable->currentItem());
}

//begin by Marco Vassura
QColor TimetableViewStudentsForm::stringToColor(QString s)
{
	// CRC-24 Based on RFC 2440 Section 6.1
	unsigned long crc = 0xB704CEL;
	int i;
	QChar *data = s.data();
	while (!data->isNull()) {
		crc ^= (data->unicode() & 0xFF) << 16;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x1000000)
				crc ^= 0x1864CFBL;
		}
		data++;
	}
	return QColor::fromRgb((int)(crc>>16), (int)((crc>>8) & 0xFF), (int)(crc & 0xFF));
}
//end by Marco Vassura

void TimetableViewStudentsForm::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);

	studentsTimetableTable->resizeRowsToContents();
}

void TimetableViewStudentsForm::currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	Q_UNUSED(previous);
	
	detailActivity(current);
}

void TimetableViewStudentsForm::detailActivity(QTableWidgetItem* item)
{
	if(item==nullptr){
		detailsTextEdit->setPlainText(QString(""));
		return;
	}
	
    if(item->row()>=TContext::get()->instance.nHoursPerDay || item->column()>=TContext::get()->instance.nDaysPerWeek){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view students timetable dialog - please generate a new timetable "
		"or close the timetable view students dialog"));
		return;
	}

//	if(Timetable::getInstance()->rules.nInternalRooms!=Timetable::getInstance()->rules.roomsList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some rooms. Please regenerate the timetable and then view it"));
//		return;
//	}

	QString s;
	QString yearname;
	QString groupname;
	QString subgroupname;

	if(yearsListWidget->currentRow()<0 || yearsListWidget->currentRow()>=yearsListWidget->count())
		return;
	if(groupsListWidget->currentRow()<0 || groupsListWidget->currentRow()>=groupsListWidget->count())
		return;
	if(subgroupsListWidget->currentRow()<0 || subgroupsListWidget->currentRow()>=subgroupsListWidget->count())
		return;

	yearname = yearsListWidget->currentItem()->text();
	groupname = groupsListWidget->currentItem()->text();
	subgroupname = subgroupsListWidget->currentItem()->text();

    StudentsSubgroup* sts=(StudentsSubgroup*)TContext::get()->instance.searchAugmentedStudentsSet(subgroupname);
	if(!sts){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid students set - please close this dialog and open a new view students timetable dialog"));
		return;
	}
	assert(sts);
	int i;
    for(i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        if(TContext::get()->instance.directSubgroupsList[i]==sts)
			break;
/*	if(!(i<Timetable::getInstance()->rules.directSubgroupsList.size())){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid students set - please close this dialog and open a new view students dialog"));
		return;
	}*/
    assert(i<TContext::get()->instance.directSubgroupsList.size());
	int j=item->row();
	int k=item->column();
	s="";
	if(j>=0 && k>=0){
        int ai=solution.students_timetable_weekly[i][k][j]; //activity index
        //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			assert(act!=nullptr);
            //s+=act->getDetailedDescriptionWithConstraints(Timetable::getInstance()->rules);
            s+=act->getDetailedDescription();

//			//int r=rooms_timetable_weekly[i][k][j];
//			int r=solution.rooms[ai];
//			if(r!=UNALLOCATED_SPACE && r!=UNSPECIFIED_ROOM){
//				s+="\n";
//				s+=tr("Room: %1").arg(Timetable::getInstance()->rules.internalRoomsList[r]->name);
//			}
			//added by Volker Dirr (start)
			QString descr="";
			QString t="";
			if(idsOfPermanentlyLockedTime.contains(act->id)){
				descr+=QCoreApplication::translate("TimetableViewForm", "permanently locked time", "refers to activity");
				t=", ";
			}
			else if(idsOfLockedTime.contains(act->id)){
				descr+=QCoreApplication::translate("TimetableViewForm", "locked time", "refers to activity");
				t=", ";
			}
			if(idsOfPermanentlyLockedSpace.contains(act->id)){
				descr+=t+QCoreApplication::translate("TimetableViewForm", "permanently locked space", "refers to activity");
			}
			else if(idsOfLockedSpace.contains(act->id)){
				descr+=t+QCoreApplication::translate("TimetableViewForm", "locked space", "refers to activity");
			}
			if(descr!=""){
				descr.prepend("\n(");
				descr.append(")");
			}
			s+=descr;
			//added by Volker Dirr (end)
		}
		else{
            if(TContext::get()->instance.subgroupNotAvailableDayHour[i][k][j]){
				s+=tr("Students subgroup is not available 100% in this slot");
				s+="\n";
			}
            if(TContext::get()->instance.breakDayHour[k][j]){
				s+=tr("Break with weight 100% in this slot");
				s+="\n";
			}
		}
	}
	detailsTextEdit->setPlainText(s);
}

void TimetableViewStudentsForm::lockTime()
{
	lock(true, false);
}

void TimetableViewStudentsForm::lockSpace()
{
	lock(false, true);
}

void TimetableViewStudentsForm::lockTimeSpace()
{
	lock(true, true);
}

void TimetableViewStudentsForm::lock(bool lockTime, bool lockSpace)
{
    Q_UNUSED(lockSpace);

    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

	//find subgroup index

	QString yearname;
	QString groupname;
	QString subgroupname;
	
	if(yearsListWidget->currentRow()<0 || yearsListWidget->currentRow()>=yearsListWidget->count()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please select a year"));
		return;
	}
	if(groupsListWidget->currentRow()<0 || groupsListWidget->currentRow()>=groupsListWidget->count()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please select a group"));
		return;
	}
	if(subgroupsListWidget->currentRow()<0 || subgroupsListWidget->currentRow()>=subgroupsListWidget->count()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please select a subgroup"));
		return;
	}

	yearname = yearsListWidget->currentItem()->text();
	groupname = groupsListWidget->currentItem()->text();
	subgroupname = subgroupsListWidget->currentItem()->text();

    Solution &tc=solution;

    StudentsSubgroup* sts=(StudentsSubgroup*)TContext::get()->instance.searchAugmentedStudentsSet(subgroupname);
	if(!sts){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid students set - please close this dialog and open a new view students timetable dialog"));
		return;
	}
	assert(sts);
	int i;
    for(i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        if(TContext::get()->instance.directSubgroupsList[i]==sts)
			break;
    assert(i<TContext::get()->instance.directSubgroupsList.size());

	bool report=false; //the messages are annoying
	
	int addedT=0, unlockedT=0;
	int addedS=0, unlockedS=0;

	//lock selected activities
	QSet <int> careAboutIndex;		//added by Volker Dirr. Needed, because of activities with duration > 1
	careAboutIndex.clear();
    for(int j=0; j<TContext::get()->instance.nHoursPerDay && j<studentsTimetableTable->rowCount(); j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek && k<studentsTimetableTable->columnCount(); k++){
			if(studentsTimetableTable->item(j, k)->isSelected()){
                int ai=solution.students_timetable_weekly[i][k][j];
				if(ai!=UNALLOCATED_ACTIVITY && !careAboutIndex.contains(ai)){	//modified, because of activities with duration > 1
					careAboutIndex.insert(ai);					//Needed, because of activities with duration > 1

                    int hour=tc.getHourForActivityIndex(ai);
                    int day=tc.getDayForActivityIndex(ai);
                    //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];
					
					if(lockTime){
                        ConstraintActivityPreferredStartingTime* ctr=new ConstraintActivityPreferredStartingTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100.0, act->id, day, hour, false);
                        bool t=TContext::get()->instance.addTimeConstraint(ctr);
						QString s;
						if(t){ //modified by Volker Dirr, so you can also unlock (start)
							addedT++;
							idsOfLockedTime.insert(act->id);
							s+=tr("Added the following constraint:")+"\n"+ctr->getDetailedDescription();
						}
						else{
							delete ctr;
						
							QList<TimeConstraint*> tmptc;
							tmptc.clear();
							int count=0;

                            foreach(ConstraintActivityPreferredStartingTime* c, TContext::get()->instance.apstHash.value(act->id, QSet<ConstraintActivityPreferredStartingTime*>())){
								assert(c->activityId==act->id);
								if(c->activityId==act->id && c->weightPercentage==100.0 && c->active && c->day>=0 && c->hour>=0){
									count++;
									if(c->permanentlyLocked){
										if(idsOfLockedTime.contains(c->activityId) || !idsOfPermanentlyLockedTime.contains(c->activityId)){
											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
											  +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred starting time with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
											  +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
											  +"\n\n"+tr("Please report possible bug")
											);
										}
										else{
                                            s+=tr("Constraint %1 will not be removed, because it is permanently locked. If you want to unlock it you must go to the constraints menu.").arg("\n"+c->getDetailedDescription()+"\n");
										}
									}
									else{
										if(!idsOfLockedTime.contains(c->activityId) || idsOfPermanentlyLockedTime.contains(c->activityId)){
											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
											  +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred starting time with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
											  +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
											  +"\n\n"+tr("Please report possible bug")
											);
										}
										else{
											tmptc.append((TimeConstraint*)c);
										}
									}
								}
							}
							if(count!=1)
								QMessageBox::warning(this, tr("m-FET warning"), tr("You may have a problem, because m-FET expected to delete 1 constraint, but will delete %1 constraints").arg(tmptc.size()));

							foreach(TimeConstraint* deltc, tmptc){
                                s+=tr("The following constraint will be deleted:")+"\n"+deltc->getDetailedDescription()+"\n";
                                TContext::get()->instance.removeTimeConstraint(deltc);
								//delete deltc; - this is done by rules.remove...
								idsOfLockedTime.remove(act->id);
								unlockedT++;
							}
							tmptc.clear();
                            //Timetable::getInstance()->rules.internalStructureComputed=false;
						}  //modified by Volker Dirr, so you can also unlock (end)
						
						if(report){
							int k;
							k=QMessageBox::information(this, tr("m-FET information"), s,
							 tr("Skip information"), tr("See next"), QString(), 1, 0 );

		 					if(k==0)
								report=false;
						}
					}

//					int ri=tc.rooms[ai];
//					if(ri!=UNALLOCATED_SPACE && ri!=UNSPECIFIED_ROOM && lockSpace){
//						ConstraintActivityPreferredRoom* ctr=new ConstraintActivityPreferredRoom(100, act->id, (Timetable::getInstance()->rules.internalRoomsList[ri])->name, false);
//						bool t=Timetable::getInstance()->rules.addSpaceConstraint(ctr);

//						QString s;
						
//						if(t){ //modified by Volker Dirr, so you can also unlock (start)
//							addedS++;
//							idsOfLockedSpace.insert(act->id);
//							s+=tr("Added the following constraint:")+"\n"+ctr->getDetailedDescription(Timetable::getInstance()->rules);
//						}
//						else{
//							delete ctr;
						
//							QList<SpaceConstraint*> tmpsc;
//							tmpsc.clear();
//							int count=0;

//							foreach(ConstraintActivityPreferredRoom* c, Timetable::getInstance()->rules.aprHash.value(act->id, QSet<ConstraintActivityPreferredRoom*>())){
//								assert(c->activityId==act->id);
//								if(c->activityId==act->id && c->weightPercentage==100.0 && c->active){
//									count++;
//									if(c->permanentlyLocked){
//										if(idsOfLockedSpace.contains(c->activityId) || !idsOfPermanentlyLockedSpace.contains(c->activityId)){
//											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
//											  +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred room with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
//											  +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
//											  +"\n\n"+tr("Please report possible bug")
//											);
//										}
//										else{
//											s+=tr("Constraint %1 will not be removed, because it is permanently locked. If you want to unlock it you must go to the constraints menu.").arg("\n"+c->getDetailedDescription(Timetable::getInstance()->rules)+"\n");
//										}
//									}
//									else{
//										if(!idsOfLockedSpace.contains(c->activityId) || idsOfPermanentlyLockedSpace.contains(c->activityId)){
//											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
//											  +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred room with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
//											  +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
//											  +"\n\n"+tr("Please report possible bug")
//											);
//										}
//										else{
//											tmpsc.append((SpaceConstraint*)c);
//										}
//									}
//								}
//							}
//							if(count!=1)
//								QMessageBox::warning(this, tr("m-FET warning"), tr("You may have a problem, because m-FET expected to delete 1 constraint, but will delete %1 constraints").arg(tmpsc.size()));

//							foreach(SpaceConstraint* delsc, tmpsc){
//								s+=tr("The following constraint will be deleted:")+"\n"+delsc->getDetailedDescription(Timetable::getInstance()->rules)+"\n";
//								Timetable::getInstance()->rules.removeSpaceConstraint(delsc);
//								idsOfLockedSpace.remove(act->id);
//								unlockedS++;
//								//delete delsc; done by rules.remove...
//							}
//							tmpsc.clear();
//							//Timetable::getInstance()->rules.internalStructureComputed=false;
//						}  //modified by Volker Dirr, so you can also unlock (end)
						
//						if(report){
//							int k;
//							k=QMessageBox::information(this, tr("m-FET information"), s,
//							 tr("Skip information"), tr("See next"), QString(), 1, 0 );
							
//		 					if(k==0)
//								report=false;
//						}
//					}
				}
			}
		}
	}
	
	QStringList added;
	QStringList removed;
	if(addedT>0){
		if(addedT==1)
			added << QCoreApplication::translate("TimetableViewForm", "Added 1 locking time constraint.", "constraint is singular (only 1 constraint)");
		else
			added << QCoreApplication::translate("TimetableViewForm", "Added %1 locking time constraints.", "%1 is >= 2, so constraints is plural").arg(addedT);
	}
	if(addedS>0){
		if(addedS==1)
			added << QCoreApplication::translate("TimetableViewForm", "Added 1 locking space constraint.", "constraint is singular (only 1 constraint)");
		else
			added << QCoreApplication::translate("TimetableViewForm", "Added %1 locking space constraints.", "%1 is >= 2, so constraints is plural").arg(addedS);
	}
	if(unlockedT>0){
		if(unlockedT==1)
			removed << QCoreApplication::translate("TimetableViewForm", "Removed 1 locking time constraint.", "constraint is singular (only 1 constraint)");
		else
			removed << QCoreApplication::translate("TimetableViewForm", "Removed %1 locking time constraints.", "%1 is >= 2, so constraints is plural").arg(unlockedT);
	}
	if(unlockedS>0){
		if(unlockedS==1)
			removed << QCoreApplication::translate("TimetableViewForm", "Removed 1 locking space constraint.", "constraint is singular (only 1 constraint)");
		else
			removed << QCoreApplication::translate("TimetableViewForm", "Removed %1 locking space constraints.", "%1 is >= 2, so constraints is plural").arg(unlockedS);
	}
	QString ad=added.join("\n");
	QString re=removed.join("\n");
	QStringList all;
	if(!ad.isEmpty())
		all<<ad;
	if(!re.isEmpty())
		all<<re;
	QString s=all.join("\n\n");
	if(s.isEmpty())
		s=QCoreApplication::translate("TimetableViewForm", "No locking constraints added or removed.");
	QMessageBox::information(this, tr("m-FET information"), s);

///////////just for testing
	QSet<int> backupLockedTime;
	QSet<int> backupPermanentlyLockedTime;
	QSet<int> backupLockedSpace;
	QSet<int> backupPermanentlyLockedSpace;
	
	backupLockedTime=idsOfLockedTime;
	backupPermanentlyLockedTime=idsOfPermanentlyLockedTime;
	backupLockedSpace=idsOfLockedSpace;
	backupPermanentlyLockedSpace=idsOfPermanentlyLockedSpace;
	
	LockUnlock::computeLockedUnlockedActivitiesTimeSpace(); //just to make sure, not really needed, but to test better
	
	assert(backupLockedTime==idsOfLockedTime);
	assert(backupPermanentlyLockedTime==idsOfPermanentlyLockedTime);
	assert(backupLockedSpace==idsOfLockedSpace);
	assert(backupPermanentlyLockedSpace==idsOfPermanentlyLockedSpace);
///////////

	LockUnlock::increaseCommunicationSpinBox(); //this is needed
	
    //cout<<"students end, isc="<<Timetable::getInstance()->rules.internalStructureComputed<<endl;
	//cout<<endl;
}

void TimetableViewStudentsForm::help()
{
	QString s="";
	//s+=QCoreApplication::translate("TimetableViewForm", "You can drag sections to increase/decrease them.");
	//s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "Lock/unlock: you can select one or more activities in the table and toggle lock/unlock in time, space or both.");
	s+=" ";
	s+=QCoreApplication::translate("TimetableViewForm", "There will be added or removed locking constraints for the selected activities (they can be unlocked only if they are not permanently locked).");
	s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "Locking time constraints are constraints of type activity preferred starting time. Locking space constraints are constraints of type"
		" activity preferred room. You can see these constraints in the corresponding constraints dialogs. New locking constraints are added at the end of the list of constraints.");
	s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "If a cell is (permanently) locked in time or space, it contains abbreviations to show that: PLT (permanently locked time), LT (locked time), "
		"PLS (permanently locked space) or LS (locked space).", "Translate the abbreviations also. Make sure the abbreviations in your language are different between themselves "
		"and the user can differentiate easily between them. These abbreviations may appear also in other places, please use the same abbreviations.");

	MessagesManager::information(this, tr("m-FET help"), s);
}

void TimetableViewStudentsForm::on_nextPushButton_clicked()
{
    int iy = yearsListWidget->currentRow();
    int ig = groupsListWidget->currentRow();
    int is = subgroupsListWidget->currentRow();

    if (is == subgroupsListWidget->count() - 1) {
        if (ig == groupsListWidget->count() - 1) {
            if (iy == yearsListWidget->count() - 1) {
            }
            else{
                yearsListWidget->setCurrentRow(iy+1);
            }
        }
        else{
            groupsListWidget->setCurrentRow(ig+1);
        }
    }
    else{
        subgroupsListWidget->setCurrentRow(is+1);
    }
}

void TimetableViewStudentsForm::on_previousPushButton_clicked()
{
    int iy = yearsListWidget->currentRow();
    int ig = groupsListWidget->currentRow();
    int is = subgroupsListWidget->currentRow();

    if (is == 0) {
        if (ig == 0) {
            if (iy == 0) {
            }
            else{
                yearsListWidget->setCurrentRow(iy-1);
                groupsListWidget->setCurrentRow(groupsListWidget->count() - 1);
                subgroupsListWidget->setCurrentRow(subgroupsListWidget->count() - 1);
            }
        }
        else{
            groupsListWidget->setCurrentRow(ig-1);
            subgroupsListWidget->setCurrentRow(subgroupsListWidget->count() - 1);
        }
    }
    else{
        subgroupsListWidget->setCurrentRow(is-1);
    }
}

void TimetableViewStudentsForm::on_addPushButton_clicked()
{
    AddAllocationForm *form=new AddAllocationForm(this, this->solution);

    InterfaceUtils::openFormWindowModal(form);
}

void TimetableViewStudentsForm::on_savePushButton_clicked()
{
    this->solution.cost(new QString());

    TContext::get()->solutions.insert(this->solution);

    emit(manualSolutionAdded());

    close();
}

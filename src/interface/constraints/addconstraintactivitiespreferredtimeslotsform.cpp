/***************************************************************************
                          addconstraintactivitiespreferredtimeslotsform.cpp  -  description
                             -------------------
    begin                : 15 May 2004
    copyright            : (C) 2004 by Lalescu Liviu
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

#include <QMessageBox>

#include "tablewidgetupdatebug.h"

#include "textmessages.h"

#include "addconstraintactivitiespreferredtimeslotsform.h"
#include "timeconstraint.h"

#include "interfaceutils.h"

#include "centerwidgetonscreen.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QBrush>
#include <QColor>

#include "stringutils.h"

#define YES		(QString(" "))
#define NO		(QString("X"))

AddConstraintActivitiesPreferredTimeSlotsForm::AddConstraintActivitiesPreferredTimeSlotsForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	durationCheckBox->setChecked(false);
	durationSpinBox->setEnabled(false);
	durationSpinBox->setMinimum(1);
	durationSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	durationSpinBox->setValue(1);

	addConstraintPushButton->setDefault(true);

	connect(preferredTimesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(setAllAllowedPushButton, SIGNAL(clicked()), this, SLOT(setAllSlotsAllowed()));
	connect(setAllNotAllowedPushButton, SIGNAL(clicked()), this, SLOT(setAllSlotsNotAllowed()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	QSize tmp1=teachersComboBox->minimumSizeHint();
	Q_UNUSED(tmp1);
	QSize tmp2=studentsComboBox->minimumSizeHint();
	Q_UNUSED(tmp2);
	QSize tmp3=subjectsComboBox->minimumSizeHint();
	Q_UNUSED(tmp3);
	QSize tmp4=activityTagsComboBox->minimumSizeHint();
	Q_UNUSED(tmp4);
	
	updateTeachersComboBox();
	updateStudentsComboBox();
	updateSubjectsComboBox();
//	updateActivityTagsComboBox();

	preferredTimesTable->setRowCount(TContext::get()->instance.nHoursPerDay);
	preferredTimesTable->setColumnCount(TContext::get()->instance.nDaysPerWeek);

	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
		QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.daysOfTheWeek[j]);
		preferredTimesTable->setHorizontalHeaderItem(j, item);
	}
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
		QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.hoursOfTheDay[i]);
		preferredTimesTable->setVerticalHeaderItem(i, item);
	}

	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			QTableWidgetItem* item=new QTableWidgetItem(YES);
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			colorItem(item);
			if(defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES)
				item->setToolTip(TContext::get()->instance.daysOfTheWeek[j]+QString("\n")+TContext::get()->instance.hoursOfTheDay[i]);
			preferredTimesTable->setItem(i, j, item);
		}
		
	preferredTimesTable->resizeRowsToContents();

	connect(preferredTimesTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(horizontalHeaderClicked(int)));
	connect(preferredTimesTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(verticalHeaderClicked(int)));

	preferredTimesTable->setSelectionMode(QAbstractItemView::NoSelection);

	tableWidgetUpdateBug(preferredTimesTable);
	
	setStretchAvailabilityTableNicely(preferredTimesTable);
}

AddConstraintActivitiesPreferredTimeSlotsForm::~AddConstraintActivitiesPreferredTimeSlotsForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintActivitiesPreferredTimeSlotsForm::colorItem(QTableWidgetItem* item)
{
	if(defs::USE_GUI_COLORS){
		if(item->text()==YES)
			item->setBackground(QBrush(Qt::darkGreen));
		else
			item->setBackground(QBrush(Qt::darkRed));
		item->setForeground(QBrush(Qt::lightGray));
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::horizontalHeaderClicked(int col)
{
	if(col>=0 && col<TContext::get()->instance.nDaysPerWeek){
		QString s=preferredTimesTable->item(0, col)->text();
		if(s==YES)
			s=NO;
		else{
			assert(s==NO);
			s=YES;
		}

		for(int row=0; row<TContext::get()->instance.nHoursPerDay; row++){
			/*QString s=notAllowedTimesTable->text(row, col);
			if(s==YES)
				s=NO;
			else{
				assert(s==NO);
				s=YES;
			}*/
			preferredTimesTable->item(row, col)->setText(s);
			colorItem(preferredTimesTable->item(row, col));
		}
		tableWidgetUpdateBug(preferredTimesTable);
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::verticalHeaderClicked(int row)
{
	if(row>=0 && row<TContext::get()->instance.nHoursPerDay){
		QString s=preferredTimesTable->item(row, 0)->text();
		if(s==YES)
			s=NO;
		else{
			assert(s==NO);
			s=YES;
		}
	
		for(int col=0; col<TContext::get()->instance.nDaysPerWeek; col++){
			/*QString s=notAllowedTimesTable->text(row, col);
			if(s==YES)
				s=NO;
			else{
				assert(s==NO);
				s=YES;
			}*/
			preferredTimesTable->item(row, col)->setText(s);
			colorItem(preferredTimesTable->item(row, col));
		}
		tableWidgetUpdateBug(preferredTimesTable);
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::setAllSlotsAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(YES);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

void AddConstraintActivitiesPreferredTimeSlotsForm::setAllSlotsNotAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(NO);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

void AddConstraintActivitiesPreferredTimeSlotsForm::itemClicked(QTableWidgetItem* item)
{
	QString s=item->text();
	
	if(s==YES)
		s=NO;
	else{
		assert(s==NO);
		s=YES;
	}
	
	item->setText(s);
	colorItem(item);

	tableWidgetUpdateBug(preferredTimesTable);
}

void AddConstraintActivitiesPreferredTimeSlotsForm::updateTeachersComboBox(){
	teachersComboBox->clear();
	teachersComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
		Teacher* t=TContext::get()->instance.teachersList[i];
		teachersComboBox->addItem(t->name);
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::updateStudentsComboBox(){
	studentsComboBox->clear();
	studentsComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* sty=TContext::get()->instance.yearsList[i];
		studentsComboBox->addItem(sty->name);
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
			studentsComboBox->addItem(stg->name);
			if(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES) for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
				studentsComboBox->addItem(sts->name);
			}
		}
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::updateSubjectsComboBox(){
	subjectsComboBox->clear();
	subjectsComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
		Subject* s=TContext::get()->instance.subjectsList[i];
		subjectsComboBox->addItem(s->name);
	}
}

//void AddConstraintActivitiesPreferredTimeSlotsForm::updateActivityTagsComboBox(){
//	activityTagsComboBox->clear();
//	activityTagsComboBox->addItem("");
//	for(int i=0; i<Timetable::getInstance()->rules.activityTagsList.size(); i++){
//		ActivityTag* s=Timetable::getInstance()->rules.activityTagsList[i];
//		activityTagsComboBox->addItem(s->name);
//	}
//}

void AddConstraintActivitiesPreferredTimeSlotsForm::addConstraint()
{
	TimeConstraint *ctr=nullptr;

	int duration=-1;
	if(durationCheckBox->isChecked()){
		assert(durationSpinBox->isEnabled());
		duration=durationSpinBox->value();
	}

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	/*bool compulsory=false;
	if(compulsoryCheckBox->isChecked())
		compulsory=true;*/

	QString teacher=teachersComboBox->currentText();
	if(teacher!="")
		assert(TContext::get()->instance.searchTeacher(teacher)>=0);

	QString students=studentsComboBox->currentText();
	if(students!="")
		assert(TContext::get()->instance.searchStudentsSet(students)!=nullptr);

	QString subject=subjectsComboBox->currentText();
	if(subject!="")
		assert(TContext::get()->instance.searchSubject(subject)>=0);
		
    QString activityTag=activityTagsComboBox->currentText();
//	if(activityTag!="")
//		assert(Timetable::getInstance()->rules.searchActivityTag(activityTag)>=0);
		
	if(duration==-1 && teacher=="" && students=="" && subject=="" && activityTag==""){
		int t=QMessageBox::question(this, tr("m-FET question"),
		 tr("You specified all the activities. This might be a small problem: if you specify"
		  " a not allowed slot between two allowed slots, this not allowed slot will"
		  " be counted as a gap in the teachers' and students' timetable.\n\n"
		  " The best practice would be to use constraint break times.\n\n"
		  " If you need weight under 100%, then you can use this constraint, but be careful"
		  " not to obtain an impossible timetable (if your teachers/students are constrained on gaps"
		  " or early gaps and if you leave a not allowed slot between 2 allowed slots or"
		  " a not allowed slot early in the day and more allowed slots after it,"
		  " this possible gap might be counted in teachers' and students' timetable)")
		  +"\n\n"+tr("Do you want to add current constraint?"),
		 QMessageBox::Yes, QMessageBox::Cancel);
						 
		if(t==QMessageBox::Cancel)
				return;
	}

	if(duration==-1 && teacher!="" && students=="" && subject=="" && activityTag==""){
		int t=QMessageBox::question(this, tr("m-FET question"),
		 tr("You specified only the teacher. This might be a small problem: if you specify"
		  " a not allowed slot between two allowed slots, this not allowed slot will"
		  " be counted as a gap in the teacher's timetable.\n\n"
		  " The best practice would be to use constraint teacher not available times.\n\n"
		  " If you need weight under 100%, then you can use this constraint, but be careful"
		  " not to obtain an impossible timetable (if your teacher is constrained on gaps"
		  " and if you leave a not allowed slot between 2 allowed slots, this possible"
		  " gap might be counted in teacher's timetable)")
		  +"\n\n"+tr("Do you want to add current constraint?"),
		 QMessageBox::Yes, QMessageBox::Cancel);
						 
		if(t==QMessageBox::Cancel)
				return;
	}
	if(duration==-1 && teacher=="" && students!="" && subject=="" && activityTag==""){
		int t=QMessageBox::question(this, tr("m-FET question"),
		 tr("You specified only the students set. This might be a small problem: if you specify"
		  " a not allowed slot between two allowed slots (or a not allowed slot before allowed slots),"
		  " this not allowed slot will"
		  " be counted as a gap (or early gap) in the students' timetable.\n\n"
		  " The best practice would be to use constraint students set not available times.\n\n"
		  " If you need weight under 100%, then you can use this constraint, but be careful"
		  " not to obtain an impossible timetable (if your students set is constrained on gaps or early gaps"
		  " and if you leave a not allowed slot between 2 allowed slots (or a not allowed slot before allowed slots), this possible"
		  " gap might be counted in students' timetable)")
		  +"\n\n"+tr("Do you want to add current constraint?"),
		 QMessageBox::Yes, QMessageBox::Cancel);
						 
		if(t==QMessageBox::Cancel)
				return;
	}

	QList<int> days_L;
	QList<int> hours_L;
	/*int days[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];
	int hours[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];*/
	int n=0;
	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
		for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
			if(preferredTimesTable->item(i, j)->text()==YES){
				/*if(n>=MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
					QString s=tr("Not enough slots (too many \"Yes\" values).");
					s+="\n";
					s+=tr("Please increase the variable MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS");
					s+="\n";
					s+=tr("Currently, it is %1").arg(MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS);
					QMessageBox::warning(this, tr("m-FET information"), s);
					
					return;
				}*/
				
				days_L.append(j);
				hours_L.append(i);
				n++;
			}

	if(n<=0){
		int t=QMessageBox::question(this, tr("m-FET question"),
		 tr("Warning: 0 slots selected. Are you sure?"),
		 QMessageBox::Yes, QMessageBox::Cancel);
						 
		if(t==QMessageBox::Cancel)
				return;
	}

    ctr=new ConstraintActivitiesPreferredTimeSlots(TContext::get()->instance, group, weight, /*compulsory,*/ teacher, students, subject, activityTag, duration, n, days_L, hours_L);

	bool tmp2=TContext::get()->instance.addTimeConstraint(ctr);
	if(tmp2){
		QString s=tr("Constraint added:");
		s+="\n\n";
		s+=ctr->getDetailedDescription();
		MessagesManager::information(this, tr("m-FET information"), s);
	}
	else{
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Constraint NOT added - duplicate?"));
		delete ctr;
	}
}

void AddConstraintActivitiesPreferredTimeSlotsForm::on_durationCheckBox_toggled()
{
	durationSpinBox->setEnabled(durationCheckBox->isChecked());
}

#undef YES
#undef NO

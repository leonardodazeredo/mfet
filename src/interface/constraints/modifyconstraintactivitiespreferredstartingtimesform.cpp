/***************************************************************************
                          modifyconstraintactivitiespreferredstartingtimesform.cpp  -  description
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

#include "modifyconstraintactivitiespreferredstartingtimesform.h"
#include "timeconstraint.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QBrush>
#include <QColor>
#include "stringutils.h"

#define YES		(QString(" "))
#define NO		(QString("X"))

ModifyConstraintActivitiesPreferredStartingTimesForm::ModifyConstraintActivitiesPreferredStartingTimesForm(QWidget* parent, ConstraintActivitiesPreferredStartingTimes* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	int duration=ctr->duration;
	durationCheckBox->setChecked(duration>=1);
	durationSpinBox->setEnabled(duration>=1);
	durationSpinBox->setMinimum(1);
	durationSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	if(duration>=1)
		durationSpinBox->setValue(duration);
	else
		durationSpinBox->setValue(1);

	okPushButton->setDefault(true);

	connect(preferredTimesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
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
	
	this->_ctr=ctr;

	updateTeachersComboBox();
	updateStudentsComboBox(parent);
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

	//bool currentMatrix[MAX_HOURS_PER_DAY][MAX_DAYS_PER_WEEK];
	Matrix2D<bool> currentMatrix;
	currentMatrix.resize(TContext::get()->instance.nHoursPerDay, TContext::get()->instance.nDaysPerWeek);
	
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
			currentMatrix[i][j]=false;
	for(int k=0; k<ctr->nPreferredStartingTimes_L; k++){
		if(ctr->hours_L[k]==-1 || ctr->days_L[k]==-1)
			assert(0);
		int i=ctr->hours_L[k];
		int j=ctr->days_L[k];
		if(i>=0 && i<TContext::get()->instance.nHoursPerDay && j>=0 && j<TContext::get()->instance.nDaysPerWeek)
			currentMatrix[i][j]=true;
	}

	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			QTableWidgetItem* item= new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			if(defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES)
				item->setToolTip(TContext::get()->instance.daysOfTheWeek[j]+QString("\n")+TContext::get()->instance.hoursOfTheDay[i]);
			preferredTimesTable->setItem(i, j, item);
			
			if(!currentMatrix[i][j])
				item->setText(NO);
			else
				item->setText(YES);
				
			colorItem(item);
		}
		
	preferredTimesTable->resizeRowsToContents();
				
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);

	connect(preferredTimesTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(horizontalHeaderClicked(int)));
	connect(preferredTimesTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(verticalHeaderClicked(int)));

	preferredTimesTable->setSelectionMode(QAbstractItemView::NoSelection);

	tableWidgetUpdateBug(preferredTimesTable);
	
	setStretchAvailabilityTableNicely(preferredTimesTable);
}

ModifyConstraintActivitiesPreferredStartingTimesForm::~ModifyConstraintActivitiesPreferredStartingTimesForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::colorItem(QTableWidgetItem* item)
{
	if(defs::USE_GUI_COLORS){
		if(item->text()==YES)
			item->setBackground(QBrush(Qt::darkGreen));
		else
			item->setBackground(QBrush(Qt::darkRed));
		item->setForeground(QBrush(Qt::lightGray));
	}
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::horizontalHeaderClicked(int col)
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
			preferredTimesTable->item(row, col)->setText(s);
			colorItem(preferredTimesTable->item(row,col));
		}
		tableWidgetUpdateBug(preferredTimesTable);
	}
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::verticalHeaderClicked(int row)
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
			preferredTimesTable->item(row, col)->setText(s);
			colorItem(preferredTimesTable->item(row,col));
		}
		tableWidgetUpdateBug(preferredTimesTable);
	}
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::setAllSlotsAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(YES);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::setAllSlotsNotAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(NO);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::itemClicked(QTableWidgetItem* item)
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

void ModifyConstraintActivitiesPreferredStartingTimesForm::updateTeachersComboBox(){
	int i=0, j=-1;
	teachersComboBox->clear();
	teachersComboBox->addItem("");
	if(this->_ctr->teacherName=="")
		j=i;
	i++;
	for(int k=0; k<TContext::get()->instance.teachersList.size(); k++){
		Teacher* t=TContext::get()->instance.teachersList[k];
		teachersComboBox->addItem(t->name);
		if(t->name==this->_ctr->teacherName)
			j=i;
		i++;
	}
	assert(j>=0);
	teachersComboBox->setCurrentIndex(j);
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::updateStudentsComboBox(QWidget* parent){
	int i=0, j=-1;
	studentsComboBox->clear();
	studentsComboBox->addItem("");
	if(this->_ctr->studentsName=="")
		j=i;
	i++;
	for(int m=0; m<TContext::get()->instance.yearsList.size(); m++){
		StudentsYear* sty=TContext::get()->instance.yearsList[m];
		studentsComboBox->addItem(sty->name);
		if(sty->name==this->_ctr->studentsName)
			j=i;
		i++;
		for(int n=0; n<sty->groupsList.size(); n++){
			StudentsGroup* stg=sty->groupsList[n];
			studentsComboBox->addItem(stg->name);
			if(stg->name==this->_ctr->studentsName)
				j=i;
			i++;
			if(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES) for(int p=0; p<stg->subgroupsList.size(); p++){
				StudentsSubgroup* sts=stg->subgroupsList[p];
				studentsComboBox->addItem(sts->name);
				if(sts->name==this->_ctr->studentsName)
					j=i;
				i++;
			}
		}
	}
	if(j<0)
		showWarningForInvisibleSubgroupConstraint(parent, this->_ctr->studentsName);
	else
		assert(j>=0);
	studentsComboBox->setCurrentIndex(j);
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::updateSubjectsComboBox(){
	int i=0, j=-1;
	subjectsComboBox->clear();
	subjectsComboBox->addItem("");
	if(this->_ctr->subjectName=="")
		j=i;
	i++;
	for(int k=0; k<TContext::get()->instance.subjectsList.size(); k++){
		Subject* s=TContext::get()->instance.subjectsList[k];
		subjectsComboBox->addItem(s->name);
		if(s->name==this->_ctr->subjectName)
			j=i;
		i++;
	}
	assert(j>=0);
	subjectsComboBox->setCurrentIndex(j);
}

//void ModifyConstraintActivitiesPreferredStartingTimesForm::updateActivityTagsComboBox(){
//	int i=0, j=-1;
//	activityTagsComboBox->clear();
//	activityTagsComboBox->addItem("");
//	if(this->_ctr->activityTagName=="")
//		j=i;
//	i++;
//	for(int k=0; k<Timetable::getInstance()->rules.activityTagsList.size(); k++){
//		ActivityTag* s=Timetable::getInstance()->rules.activityTagsList[k];
//		activityTagsComboBox->addItem(s->name);
//		if(s->name==this->_ctr->activityTagName)
//			j=i;
//		i++;
//	}
//	assert(j>=0);
//	activityTagsComboBox->setCurrentIndex(j);
//}

void ModifyConstraintActivitiesPreferredStartingTimesForm::ok()
{
	int duration=-1;
	if(durationCheckBox->isChecked()){
		assert(durationSpinBox->isEnabled());
		duration=durationSpinBox->value();
	}

	if(studentsComboBox->currentIndex()<0){
		showWarningCannotModifyConstraintInvisibleSubgroupConstraint(this, this->_ctr->studentsName);
		return;
	}

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}

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
	//int days[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES];
	//int hours[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES];
	int n=0;
	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
		for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
			if(preferredTimesTable->item(i, j)->text()==YES){
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


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	this->_ctr->teacherName=teacher;
	this->_ctr->studentsName=students;
	this->_ctr->subjectName=subject;
	this->_ctr->activityTagName=activityTag;
	this->_ctr->nPreferredStartingTimes_L=n;
	this->_ctr->days_L=days_L;
	this->_ctr->hours_L=hours_L;

	this->_ctr->duration=duration;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::cancel()
{
	this->close();
}

void ModifyConstraintActivitiesPreferredStartingTimesForm::on_durationCheckBox_toggled()
{
	durationSpinBox->setEnabled(durationCheckBox->isChecked());
}
#undef YES
#undef NO

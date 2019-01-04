/***************************************************************************
                          addconstraintactivitiesendstudentsdayform.cpp  -  description
                             -------------------
    begin                : 2008
    copyright            : (C) 2008 by Lalescu Liviu
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

#include "interfaceutils.h"
#include "stringutils.h"


#include "textmessages.h"

#include "addconstraintactivitiesendstudentsdayform.h"

AddConstraintActivitiesEndStudentsDayForm::AddConstraintActivitiesEndStudentsDayForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

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
}

AddConstraintActivitiesEndStudentsDayForm::~AddConstraintActivitiesEndStudentsDayForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintActivitiesEndStudentsDayForm::updateTeachersComboBox(){
	teachersComboBox->clear();
	teachersComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
		Teacher* t=TContext::get()->instance.teachersList[i];
		teachersComboBox->addItem(t->name);
	}
}

void AddConstraintActivitiesEndStudentsDayForm::updateStudentsComboBox(){
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

void AddConstraintActivitiesEndStudentsDayForm::updateSubjectsComboBox(){
	subjectsComboBox->clear();
	subjectsComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
		Subject* s=TContext::get()->instance.subjectsList[i];
		subjectsComboBox->addItem(s->name);
	}
}

//void AddConstraintActivitiesEndStudentsDayForm::updateActivityTagsComboBox(){
//	activityTagsComboBox->clear();
//	activityTagsComboBox->addItem("");
//	for(int i=0; i<Timetable::getInstance()->rules.activityTagsList.size(); i++){
//		ActivityTag* s=Timetable::getInstance()->rules.activityTagsList[i];
//		activityTagsComboBox->addItem(s->name);
//	}
//}

void AddConstraintActivitiesEndStudentsDayForm::addConstraint()
{
	TimeConstraint *ctr=nullptr;

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}
	if(weight!=100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage) - must be 100%"));
		return;
	}
    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

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
		
    ctr=new ConstraintActivitiesEndStudentsDay(TContext::get()->instance, group, weight, teacher, students, subject, activityTag);

	bool tmp2=TContext::get()->instance.addTimeConstraint(ctr);
	if(tmp2){
		QString s=tr("Constraint added:");
		s+="\n\n";
		s+=ctr->getDetailedDescription();
		MessagesManager::information(this, tr("m-FET information"), s);
	}
	else{
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Constraint NOT added - please report error"));
		delete ctr;
	}
}

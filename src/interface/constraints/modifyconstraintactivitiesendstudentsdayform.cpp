/***************************************************************************
                          modifyconstraintactivitiesendstudentsdayform.cpp  -  description
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

#include "modifyconstraintactivitiesendstudentsdayform.h"
#include "timeconstraint.h"

ModifyConstraintActivitiesEndStudentsDayForm::ModifyConstraintActivitiesEndStudentsDayForm(QWidget* parent, ConstraintActivitiesEndStudentsDay* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));

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

	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
}

ModifyConstraintActivitiesEndStudentsDayForm::~ModifyConstraintActivitiesEndStudentsDayForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintActivitiesEndStudentsDayForm::updateTeachersComboBox(){
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

void ModifyConstraintActivitiesEndStudentsDayForm::updateStudentsComboBox(QWidget* parent){
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

void ModifyConstraintActivitiesEndStudentsDayForm::updateSubjectsComboBox(){
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

//void ModifyConstraintActivitiesEndStudentsDayForm::updateActivityTagsComboBox(){
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

void ModifyConstraintActivitiesEndStudentsDayForm::ok()
{
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
	if(weight!=100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage) - it must be 100%"));
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

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	this->_ctr->teacherName=teacher;
	this->_ctr->studentsName=students;
	this->_ctr->subjectName=subject;
	this->_ctr->activityTagName=activityTag;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintActivitiesEndStudentsDayForm::cancel()
{
	this->close();
}

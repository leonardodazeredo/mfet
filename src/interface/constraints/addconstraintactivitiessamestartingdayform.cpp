/***************************************************************************
                          addconstraintactivitiessamestartingdayform.cpp  -  description
                             -------------------
    begin                : Wed June 23 2004
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

#include "textmessages.h"

#include "addconstraintactivitiessamestartingdayform.h"
//#include "spaceconstraint.h"
#include "interfaceutils.h"

#include "stringutils.h"

#include <QListWidget>
#include <QAbstractItemView>
#include <QScrollBar>

AddConstraintActivitiesSameStartingDayForm::AddConstraintActivitiesSameStartingDayForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	activitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	simultaneousActivitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addConstraint()));
	connect(teachersComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(studentsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(subjectsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(activityTagsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(clearPushButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(activitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addActivity()));
	connect(addAllActivitiesPushButton, SIGNAL(clicked()), this, SLOT(addAllActivities()));
	connect(simultaneousActivitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeActivity()));

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
	
	teachersComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
		Teacher* tch=TContext::get()->instance.teachersList[i];
		teachersComboBox->addItem(tch->name);
	}
	teachersComboBox->setCurrentIndex(0);

	subjectsComboBox->addItem("");
	for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
		Subject* sb=TContext::get()->instance.subjectsList[i];
		subjectsComboBox->addItem(sb->name);
	}
	subjectsComboBox->setCurrentIndex(0);

//	activityTagsComboBox->addItem("");
//	for(int i=0; i<Timetable::getInstance()->rules.activityTagsList.size(); i++){
//		ActivityTag* st=Timetable::getInstance()->rules.activityTagsList[i];
//		activityTagsComboBox->addItem(st->name);
//	}
//	activityTagsComboBox->setCurrentIndex(0);

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
	studentsComboBox->setCurrentIndex(0);
	
	simultaneousActivitiesListWidget->clear();
	this->simultaneousActivitiesList.clear();

	updateActivitiesListWidget();
}

AddConstraintActivitiesSameStartingDayForm::~AddConstraintActivitiesSameStartingDayForm()
{
	saveFETDialogGeometry(this);
}

bool AddConstraintActivitiesSameStartingDayForm::filterOk(Activity* act)
{
	QString tn=teachersComboBox->currentText();
	QString stn=studentsComboBox->currentText();
	QString sbn=subjectsComboBox->currentText();
	QString sbtn=activityTagsComboBox->currentText();
	int ok=true;

	//subject
	if(sbn!="" && sbn!=act->subjectName)
		ok=false;
		
	//activity tag
	if(sbtn!="" && !act->activityTagsNames.contains(sbtn))
		ok=false;
		
	//students
	if(stn!=""){
		bool ok2=false;
        for(QStringList::Iterator it=act->studentSetsNames.begin(); it!=act->studentSetsNames.end(); it++)
            if(*it == stn){
                ok2=true;
                break;
            }
		if(!ok2)
			ok=false;
	}
	
	return ok;
}

void AddConstraintActivitiesSameStartingDayForm::filterChanged()
{
	this->updateActivitiesListWidget();
}

void AddConstraintActivitiesSameStartingDayForm::updateActivitiesListWidget()
{
	activitiesListWidget->clear();
	this->activitiesList.clear();

	for(int i=0; i<TContext::get()->instance.activitiesList.size(); i++){
		Activity* ac=TContext::get()->instance.activitiesList[i];
		if(filterOk(ac)){
			activitiesListWidget->addItem(ac->getDescription());
			this->activitiesList.append(ac->id);
		}
	}
	
	int q=activitiesListWidget->verticalScrollBar()->minimum();
	activitiesListWidget->verticalScrollBar()->setValue(q);
}

void AddConstraintActivitiesSameStartingDayForm::addConstraint()
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

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	if(this->simultaneousActivitiesList.count()==0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Empty list of simultaneous activities"));
		return;
	}
	if(this->simultaneousActivitiesList.count()==1){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Only one selected activity - impossible"));
		return;
	}

	QList<int> ids;
	QList<int>::iterator it;
	int i;
	ids.clear();
	for(i=0, it=this->simultaneousActivitiesList.begin(); it!=this->simultaneousActivitiesList.end(); i++,it++){
		ids.append(*it);
	}
    ctr=new ConstraintActivitiesSameStartingDay(TContext::get()->instance, group, weight, this->simultaneousActivitiesList.count(), ids);

	bool tmp2=TContext::get()->instance.addTimeConstraint(ctr);
		
	if(tmp2){
		QString s;
		
		s+=tr("Constraint added:");
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

void AddConstraintActivitiesSameStartingDayForm::addActivity()
{
	if(activitiesListWidget->currentRow()<0)
		return;
	int tmp=activitiesListWidget->currentRow();
	int _id=this->activitiesList.at(tmp);
	
	QString actName=activitiesListWidget->currentItem()->text();
	assert(actName!="");
	int i;
	//duplicate?
	for(i=0; i<simultaneousActivitiesListWidget->count(); i++)
		if(actName==simultaneousActivitiesListWidget->item(i)->text())
			break;
	if(i<simultaneousActivitiesListWidget->count())
		return;
	
	simultaneousActivitiesListWidget->addItem(actName);
	simultaneousActivitiesListWidget->setCurrentRow(simultaneousActivitiesListWidget->count()-1);
	
	this->simultaneousActivitiesList.append(_id);
}

void AddConstraintActivitiesSameStartingDayForm::addAllActivities()
{
	for(int tmp=0; tmp<activitiesListWidget->count(); tmp++){
		int _id=this->activitiesList.at(tmp);
	
		QString actName=activitiesListWidget->item(tmp)->text();
		assert(actName!="");
		int i;
		//duplicate?
		for(i=0; i<simultaneousActivitiesList.count(); i++)
			if(simultaneousActivitiesList.at(i)==_id)
				break;
		if(i<simultaneousActivitiesList.count())
			continue;
			
		simultaneousActivitiesListWidget->addItem(actName);
		this->simultaneousActivitiesList.append(_id);
	}
	
	simultaneousActivitiesListWidget->setCurrentRow(simultaneousActivitiesListWidget->count()-1);
}

void AddConstraintActivitiesSameStartingDayForm::removeActivity()
{
	if(simultaneousActivitiesListWidget->currentRow()<0 || simultaneousActivitiesListWidget->count()<=0)
		return;
	int tmp=simultaneousActivitiesListWidget->currentRow();

	this->simultaneousActivitiesList.removeAt(tmp);
	
	simultaneousActivitiesListWidget->setCurrentRow(-1);
	QListWidgetItem* item=simultaneousActivitiesListWidget->takeItem(tmp);
	delete item;
	if(tmp<simultaneousActivitiesListWidget->count())
		simultaneousActivitiesListWidget->setCurrentRow(tmp);
	else
		simultaneousActivitiesListWidget->setCurrentRow(simultaneousActivitiesListWidget->count()-1);
}

void AddConstraintActivitiesSameStartingDayForm::clear()
{
	simultaneousActivitiesListWidget->clear();
	simultaneousActivitiesList.clear();
}

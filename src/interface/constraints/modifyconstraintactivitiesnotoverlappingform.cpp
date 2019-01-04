/***************************************************************************
                          modifyconstraintactivitiesnotoverlappingform.cpp  -  description
                             -------------------
    begin                : Feb 11, 2005
    copyright            : (C) 2005 by Lalescu Liviu
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

#include "modifyconstraintactivitiesnotoverlappingform.h"
#include "m-fet.h"

#include <QListWidget>
#include <QAbstractItemView>
#include <QScrollBar>

ModifyConstraintActivitiesNotOverlappingForm::ModifyConstraintActivitiesNotOverlappingForm(QWidget* parent, ConstraintActivitiesNotOverlapping* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	activitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	notOverlappingActivitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(activitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addActivity()));
	connect(addAllActivitiesPushButton, SIGNAL(clicked()), this, SLOT(addAllActivities()));
	connect(notOverlappingActivitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeActivity()));
	connect(teachersComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(studentsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(subjectsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(activityTagsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(clearPushButton, SIGNAL(clicked()), this, SLOT(clear()));

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
	
	notOverlappingActivitiesList.clear();
	notOverlappingActivitiesListWidget->clear();
	for(int i=0; i<ctr->n_activities; i++){
		int actId=ctr->activitiesId[i];
		this->notOverlappingActivitiesList.append(actId);
		Activity* act=nullptr;
		for(int k=0; k<TContext::get()->instance.activitiesList.size(); k++){
			act=TContext::get()->instance.activitiesList[k];
			if(act->id==actId)
				break;
		}
		assert(act);
		this->notOverlappingActivitiesListWidget->addItem(act->getDescription());
	}
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	////////////////
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

	filterChanged();
}

ModifyConstraintActivitiesNotOverlappingForm::~ModifyConstraintActivitiesNotOverlappingForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintActivitiesNotOverlappingForm::filterChanged()
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

bool ModifyConstraintActivitiesNotOverlappingForm::filterOk(Activity* act)
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

void ModifyConstraintActivitiesNotOverlappingForm::ok()
{
	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}

	if(this->notOverlappingActivitiesList.count()==0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Empty list of not overlapping activities"));
		return;
	}
	if(this->notOverlappingActivitiesList.count()==1){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Only one selected activity"));
		return;
	}
	
	int i;
	QList<int>::iterator it;
	this->_ctr->activitiesId.clear();
	for(i=0, it=this->notOverlappingActivitiesList.begin(); it!=this->notOverlappingActivitiesList.end(); it++, i++){
		this->_ctr->activitiesId.append(*it);
	}
	this->_ctr->n_activities=i;

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintActivitiesNotOverlappingForm::cancel()
{
	this->close();
}

void ModifyConstraintActivitiesNotOverlappingForm::addActivity()
{
	if(activitiesListWidget->currentRow()<0)
		return;
	int tmp=activitiesListWidget->currentRow();
	int _id=this->activitiesList.at(tmp);
	
	QString actName=activitiesListWidget->currentItem()->text();
	assert(actName!="");
	int i;
	//duplicate?
	for(i=0; i<notOverlappingActivitiesListWidget->count(); i++)
		if(actName==notOverlappingActivitiesListWidget->item(i)->text())
			break;
	if(i<notOverlappingActivitiesListWidget->count())
		return;
	notOverlappingActivitiesListWidget->addItem(actName);
	notOverlappingActivitiesListWidget->setCurrentRow(notOverlappingActivitiesListWidget->count()-1);
	
	this->notOverlappingActivitiesList.append(_id);
}

void ModifyConstraintActivitiesNotOverlappingForm::addAllActivities()
{
	for(int tmp=0; tmp<activitiesListWidget->count(); tmp++){
		int _id=this->activitiesList.at(tmp);
	
		QString actName=activitiesListWidget->item(tmp)->text();
		assert(actName!="");
		int i;
		//duplicate?
		for(i=0; i<notOverlappingActivitiesList.count(); i++)
			if(notOverlappingActivitiesList.at(i)==_id)
				break;
		if(i<notOverlappingActivitiesList.count())
			continue;
		
		notOverlappingActivitiesListWidget->addItem(actName);
		this->notOverlappingActivitiesList.append(_id);
	}
	
	notOverlappingActivitiesListWidget->setCurrentRow(notOverlappingActivitiesListWidget->count()-1);
}

void ModifyConstraintActivitiesNotOverlappingForm::removeActivity()
{
	if(notOverlappingActivitiesListWidget->currentRow()<0 || notOverlappingActivitiesListWidget->count()<=0)
		return;
	int tmp=notOverlappingActivitiesListWidget->currentRow();
	
	notOverlappingActivitiesList.removeAt(tmp);

	notOverlappingActivitiesListWidget->setCurrentRow(-1);
	QListWidgetItem* item=notOverlappingActivitiesListWidget->takeItem(tmp);
	delete item;
	if(tmp<notOverlappingActivitiesListWidget->count())
		notOverlappingActivitiesListWidget->setCurrentRow(tmp);
	else
		notOverlappingActivitiesListWidget->setCurrentRow(notOverlappingActivitiesListWidget->count()-1);
}

void ModifyConstraintActivitiesNotOverlappingForm::clear()
{
	notOverlappingActivitiesListWidget->clear();
	notOverlappingActivitiesList.clear();
}


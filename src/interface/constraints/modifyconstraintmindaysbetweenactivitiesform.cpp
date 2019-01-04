/***************************************************************************
                          modifyconstraintmindaysbetweenactivitiesform.cpp  -  description
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

#include "modifyconstraintmindaysbetweenactivitiesform.h"
//#include "spaceconstraint.h"

#include <QList>

#include <QListWidget>
#include <QAbstractItemView>
#include <QScrollBar>

ModifyConstraintMinDaysBetweenActivitiesForm::ModifyConstraintMinDaysBetweenActivitiesForm(QWidget* parent, ConstraintMinDaysBetweenActivities* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);
	
	activitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	selectedActivitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(activitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(addActivity()));
	connect(addAllActivitiesPushButton, SIGNAL(clicked()), this, SLOT(addAllActivities()));
	connect(selectedActivitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeActivity()));
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

	selectedActivitiesList.clear();
	selectedActivitiesListWidget->clear();
	for(int i=0; i<ctr->n_activities; i++){
		int actId=ctr->activitiesId[i];
		this->selectedActivitiesList.append(actId);
		Activity* act=nullptr;
		for(int k=0; k<TContext::get()->instance.activitiesList.size(); k++){
			act=TContext::get()->instance.activitiesList[k];
			if(act->id==actId)
				break;
		}
		assert(act);
		this->selectedActivitiesListWidget->addItem(act->getDescription());
	}
	
	minDaysSpinBox->setMinimum(1);
	minDaysSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek-1);
	minDaysSpinBox->setValue(ctr->minDays);

	consecutiveIfSameDayCheckBox->setChecked(ctr->consecutiveIfSameDay);
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

ModifyConstraintMinDaysBetweenActivitiesForm::~ModifyConstraintMinDaysBetweenActivitiesForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintMinDaysBetweenActivitiesForm::filterChanged()
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

bool ModifyConstraintMinDaysBetweenActivitiesForm::filterOk(Activity* act)
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

void ModifyConstraintMinDaysBetweenActivitiesForm::ok()
{
	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}

	if(this->selectedActivitiesList.size()==0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Empty list of selected activities"));
		return;
	}
	if(this->selectedActivitiesList.size()==1){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Only one selected activity"));
		return;
	}

#if 0
	if(0 && this->selectedActivitiesList.size()>Timetable::getInstance()->rules.nDaysPerWeek){
		QString s=tr("You want to add a constraint min days between activities for more activities than the number of days per week."
		  " This is a very bad practice from the way the algorithm of generation works (it slows down the generation and makes it harder to find a solution).")+
		 "\n\n"+
		 tr("The best way to add the activities would be:")+
		 "\n\n"+

		 tr("1. If you add 'force consecutive if same day', then couple extra activities in pairs to obtain a number of activities equal to the number of days per week"
		  ". Example: 7 activities with duration 1 in a 5 days week, then transform into 5 activities with durations: 2,2,1,1,1 and add a single container activity with these 5 components"
		  " (possibly raising the weight of added constraint min days between activities up to 100%)")+

		  "\n\n"+

		 tr("2. If you don't add 'force consecutive if same day', then add a larger activity split into a number of"
		  " activities equal with the number of days per week and the remaining components into other larger split activity."
		  " For example, suppose you need to add 7 activities with duration 1 in a 5 days week. Add 2 larger container activities,"
		  " first one split into 5 activities with duration 1 and second one split into 2 activities with duration 1"
		  " (possibly raising the weight of added constraints min days between activities for each of the 2 containers up to 100%)")+

	  	 "\n\n"+
		 tr("Do you want to add current constraint as it is now (not recommended) or cancel and edit as instructed?");
	
		int t=QMessageBox::warning(this, tr("m-FET warning"),	s,
		 QMessageBox::Yes, QMessageBox::Cancel);
		if(t==QMessageBox::Cancel)
			return;
	}
#endif

	if(1){
        ConstraintMinDaysBetweenActivities adc(TContext::get()->instance);

		int i;
		QList<int>::iterator it;
		adc.activitiesId.clear();
		for(i=0, it=this->selectedActivitiesList.begin(); it!=this->selectedActivitiesList.end(); it++, i++){
			adc.activitiesId.append(*it);
		}
		adc.n_activities=i;
		
		adc.weightPercentage=weight;
		adc.consecutiveIfSameDay=consecutiveIfSameDayCheckBox->isChecked();
		adc.minDays=minDaysSpinBox->value();
		
		bool duplicate=false;
		
		foreach(TimeConstraint* tc, TContext::get()->instance.timeConstraintsList)
			if(tc!=this->_ctr && tc->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES)
				if( ( *((ConstraintMinDaysBetweenActivities*)tc) ) == adc){
					duplicate=true;
					break;
				}
				
		if(duplicate){
			QMessageBox::warning(this, tr("m-FET information"), tr("Cannot proceed, current constraint is equal to another one (it is duplicated)"));
			return;
		}
	}
	
	if(_ctr->activitiesId!=selectedActivitiesList){
		foreach(int oldId, _ctr->activitiesId){
			QSet<ConstraintMinDaysBetweenActivities*> cs=TContext::get()->instance.mdbaHash.value(oldId, QSet<ConstraintMinDaysBetweenActivities*>());
			assert(cs.contains(_ctr));
			cs.remove(_ctr);
			TContext::get()->instance.mdbaHash.insert(oldId, cs);
		}
		
		foreach(int newId, selectedActivitiesList){
			QSet<ConstraintMinDaysBetweenActivities*> cs=TContext::get()->instance.mdbaHash.value(newId, QSet<ConstraintMinDaysBetweenActivities*>());
			assert(!cs.contains(_ctr));
			cs.insert(_ctr);
			TContext::get()->instance.mdbaHash.insert(newId, cs);
		}
		
		_ctr->activitiesId=selectedActivitiesList;
		_ctr->n_activities=_ctr->activitiesId.count();
	}

	/*int i;
	QList<int>::iterator it;
	this->_ctr->activitiesId.clear();
	for(i=0, it=this->selectedActivitiesList.begin(); it!=this->selectedActivitiesList.end(); it++, i++){
		this->_ctr->activitiesId.append(*it);
	}
	this->_ctr->n_activities=i;*/


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	this->_ctr->consecutiveIfSameDay=consecutiveIfSameDayCheckBox->isChecked();
	this->_ctr->minDays=minDaysSpinBox->value();
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintMinDaysBetweenActivitiesForm::cancel()
{
	this->close();
}

void ModifyConstraintMinDaysBetweenActivitiesForm::addActivity()
{
	if(activitiesListWidget->currentRow()<0)
		return;
	int tmp=activitiesListWidget->currentRow();
	int _id=this->activitiesList.at(tmp);
	
	QString actName=activitiesListWidget->currentItem()->text();
	assert(actName!="");
	int i;
	//duplicate?
	for(i=0; i<selectedActivitiesListWidget->count(); i++)
		if(actName==selectedActivitiesListWidget->item(i)->text())
			break;
	if(i<selectedActivitiesListWidget->count())
		return;
	selectedActivitiesListWidget->addItem(actName);
	selectedActivitiesListWidget->setCurrentRow(selectedActivitiesListWidget->count()-1);
	
	this->selectedActivitiesList.append(_id);
}

void ModifyConstraintMinDaysBetweenActivitiesForm::addAllActivities()
{
	for(int tmp=0; tmp<activitiesListWidget->count(); tmp++){
		int _id=this->activitiesList.at(tmp);
	
		QString actName=activitiesListWidget->item(tmp)->text();
		assert(actName!="");
		int i;
		//duplicate?
		for(i=0; i<selectedActivitiesList.count(); i++)
			if(selectedActivitiesList.at(i)==_id)
				break;
		if(i<selectedActivitiesList.count())
			continue;
			
		selectedActivitiesListWidget->addItem(actName);
		this->selectedActivitiesList.append(_id);
	}
	
	selectedActivitiesListWidget->setCurrentRow(selectedActivitiesListWidget->count()-1);
}

void ModifyConstraintMinDaysBetweenActivitiesForm::removeActivity()
{
	if(selectedActivitiesListWidget->currentRow()<0 || selectedActivitiesListWidget->count()<=0)
		return;
	int tmp=selectedActivitiesListWidget->currentRow();
	
	this->selectedActivitiesList.removeAt(tmp);

	selectedActivitiesListWidget->setCurrentRow(-1);
	QListWidgetItem* item=selectedActivitiesListWidget->takeItem(tmp);
	delete item;
	if(tmp<selectedActivitiesListWidget->count())
		selectedActivitiesListWidget->setCurrentRow(tmp);
	else
		selectedActivitiesListWidget->setCurrentRow(selectedActivitiesListWidget->count()-1);
}

void ModifyConstraintMinDaysBetweenActivitiesForm::clear()
{
	selectedActivitiesListWidget->clear();
	selectedActivitiesList.clear();
}

/***************************************************************************
                          modifyconstraintactivitypreferredstartingtimesform.cpp  -  description
                             ------------------
    begin                : Feb 15, 2005
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

#include "tablewidgetupdatebug.h"

#include "modifyconstraintactivitypreferredstartingtimesform.h"
#include "timeconstraint.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QBrush>
#include <QColor>

#include "stringutils.h"

#define YES		(QString(" "))
#define NO		(QString("X"))

ModifyConstraintActivityPreferredStartingTimesForm::ModifyConstraintActivityPreferredStartingTimesForm(QWidget* parent, ConstraintActivityPreferredStartingTimes* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(preferredTimesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(teachersComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(studentsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(subjectsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(activityTagsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
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

	QSize tmp5=activitiesComboBox->minimumSizeHint();
	Q_UNUSED(tmp5);
	
	activitiesComboBox->setMaximumWidth(maxRecommendedWidth(this));
	
	this->_ctr=ctr;
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);

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
	
	updateActivitiesComboBox();

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
		
	Matrix2D<bool> currentMatrix;
	currentMatrix.resize(TContext::get()->instance.nHoursPerDay, TContext::get()->instance.nDaysPerWeek);
	//bool currentMatrix[MAX_HOURS_PER_DAY][MAX_DAYS_PER_WEEK];
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
			currentMatrix[i][j]=false;
	for(int k=0; k<ctr->nPreferredStartingTimes_L; k++){
		if(ctr->days_L[k]==-1 || ctr->hours_L[k]==-1)
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

	connect(preferredTimesTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(horizontalHeaderClicked(int)));
	connect(preferredTimesTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(verticalHeaderClicked(int)));

	preferredTimesTable->setSelectionMode(QAbstractItemView::NoSelection);

	tableWidgetUpdateBug(preferredTimesTable);
	
	setStretchAvailabilityTableNicely(preferredTimesTable);
}

ModifyConstraintActivityPreferredStartingTimesForm::~ModifyConstraintActivityPreferredStartingTimesForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintActivityPreferredStartingTimesForm::colorItem(QTableWidgetItem* item)
{
	if(defs::USE_GUI_COLORS){
		if(item->text()==YES)
			item->setBackground(QBrush(Qt::darkGreen));
		else
			item->setBackground(QBrush(Qt::darkRed));
		item->setForeground(QBrush(Qt::lightGray));
	}
}

void ModifyConstraintActivityPreferredStartingTimesForm::horizontalHeaderClicked(int col)
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

void ModifyConstraintActivityPreferredStartingTimesForm::verticalHeaderClicked(int row)
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

void ModifyConstraintActivityPreferredStartingTimesForm::setAllSlotsAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(YES);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

void ModifyConstraintActivityPreferredStartingTimesForm::setAllSlotsNotAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			preferredTimesTable->item(i, j)->setText(NO);
			colorItem(preferredTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(preferredTimesTable);
}

bool ModifyConstraintActivityPreferredStartingTimesForm::filterOk(Activity* act)
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

void ModifyConstraintActivityPreferredStartingTimesForm::filterChanged()
{
	this->updateActivitiesComboBox();
}

void ModifyConstraintActivityPreferredStartingTimesForm::itemClicked(QTableWidgetItem* item)
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

void ModifyConstraintActivityPreferredStartingTimesForm::updateActivitiesComboBox(){
	int i=0, j=-1;
	activitiesComboBox->clear();
	activitiesList.clear();
	for(int k=0; k<TContext::get()->instance.activitiesList.size(); k++){
		Activity* act=TContext::get()->instance.activitiesList[k];
		if(filterOk(act)){
			activitiesComboBox->addItem(act->getDescription());
			this->activitiesList.append(act->id);
			if(act->id==this->_ctr->activityId)
				j=i;
		
			i++;
		}
	}
	//assert(j>=0); only first time
	activitiesComboBox->setCurrentIndex(j);
}

void ModifyConstraintActivityPreferredStartingTimesForm::cancel()
{
	this->close();
}

void ModifyConstraintActivityPreferredStartingTimesForm::ok()
{
	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}

	int i=activitiesComboBox->currentIndex();
	assert(i<activitiesList.size());
	if(i<0 || activitiesComboBox->count()<=0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid activity"));
		return;
	}
	int id=activitiesList.at(i);
	
	QList<int> days_L;
	QList<int> hours_L;
	//int days[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES];
	//int hours[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES];
	int n=0;
	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
		for(i=0; i<TContext::get()->instance.nHoursPerDay; i++)
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
	this->_ctr->activityId=id;
	this->_ctr->nPreferredStartingTimes_L=n;
	this->_ctr->days_L=days_L;
	this->_ctr->hours_L=hours_L;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

#undef YES
#undef NO

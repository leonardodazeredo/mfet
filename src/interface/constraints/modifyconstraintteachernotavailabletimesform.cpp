/***************************************************************************
                          modifyconstraintteachernotavailabletimesform.cpp  -  description
                             -------------------
    begin                : Feb 10, 2005
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

#include "modifyconstraintteachernotavailabletimesform.h"
#include "timeconstraint.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QBrush>
#include <QColor>
#include "stringutils.h"

#define YES		(QString("X"))
#define NO		(QString(" "))

ModifyConstraintTeacherNotAvailableTimesForm::ModifyConstraintTeacherNotAvailableTimesForm(QWidget* parent, ConstraintTeacherNotAvailableTimes* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(notAllowedTimesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(setAllAvailablePushButton, SIGNAL(clicked()), this, SLOT(setAllAvailable()));
	connect(setAllNotAvailablePushButton, SIGNAL(clicked()), this, SLOT(setAllNotAvailable()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	QSize tmp1=teachersComboBox->minimumSizeHint();
	Q_UNUSED(tmp1);
	
	this->_ctr=ctr;
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	updateTeachersComboBox();

	notAllowedTimesTable->setRowCount(TContext::get()->instance.nHoursPerDay);
	notAllowedTimesTable->setColumnCount(TContext::get()->instance.nDaysPerWeek);

	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
		QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.daysOfTheWeek[j]);
		notAllowedTimesTable->setHorizontalHeaderItem(j, item);
	}
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
		QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.hoursOfTheDay[i]);
		notAllowedTimesTable->setVerticalHeaderItem(i, item);
	}

	//bool currentMatrix[MAX_HOURS_PER_DAY][MAX_DAYS_PER_WEEK];
	Matrix2D<bool> currentMatrix;
	currentMatrix.resize(TContext::get()->instance.nHoursPerDay, TContext::get()->instance.nDaysPerWeek);

	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
			currentMatrix[i][j]=false;			
	assert(ctr->days.count()==ctr->hours.count());	
	for(int k=0; k<ctr->days.count(); k++){
		if(ctr->hours.at(k)==-1 || ctr->days.at(k)==-1)
			assert(0);
		int i=ctr->hours.at(k);
		int j=ctr->days.at(k);
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
			notAllowedTimesTable->setItem(i, j, item);

			if(!currentMatrix[i][j])
				item->setText(NO);
			else
				item->setText(YES);
				
			colorItem(item);
		}
		
	notAllowedTimesTable->resizeRowsToContents();

	connect(notAllowedTimesTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(horizontalHeaderClicked(int)));
	connect(notAllowedTimesTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(verticalHeaderClicked(int)));
	
	notAllowedTimesTable->setSelectionMode(QAbstractItemView::NoSelection);
	
	tableWidgetUpdateBug(notAllowedTimesTable);
	
	setStretchAvailabilityTableNicely(notAllowedTimesTable);
}

ModifyConstraintTeacherNotAvailableTimesForm::~ModifyConstraintTeacherNotAvailableTimesForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintTeacherNotAvailableTimesForm::colorItem(QTableWidgetItem* item)
{
	if(defs::USE_GUI_COLORS){
		if(item->text()==NO)
			item->setBackground(QBrush(Qt::darkGreen));
		else
			item->setBackground(QBrush(Qt::darkRed));
		item->setForeground(QBrush(Qt::lightGray));
	}
}

void ModifyConstraintTeacherNotAvailableTimesForm::horizontalHeaderClicked(int col)
{
	if(col>=0 && col<TContext::get()->instance.nDaysPerWeek){
		QString s=notAllowedTimesTable->item(0, col)->text();
		if(s==YES)
			s=NO;
		else{
			assert(s==NO);
			s=YES;
		}

		for(int row=0; row<TContext::get()->instance.nHoursPerDay; row++){
			notAllowedTimesTable->item(row, col)->setText(s);
			colorItem(notAllowedTimesTable->item(row,col));
		}
		tableWidgetUpdateBug(notAllowedTimesTable);
	}
}

void ModifyConstraintTeacherNotAvailableTimesForm::verticalHeaderClicked(int row)
{
	if(row>=0 && row<TContext::get()->instance.nHoursPerDay){
		QString s=notAllowedTimesTable->item(row, 0)->text();
		if(s==YES)
			s=NO;
		else{
			assert(s==NO);
			s=YES;
		}
	
		for(int col=0; col<TContext::get()->instance.nDaysPerWeek; col++){
			notAllowedTimesTable->item(row, col)->setText(s);
			colorItem(notAllowedTimesTable->item(row,col));
		}
		tableWidgetUpdateBug(notAllowedTimesTable);
	}
}

void ModifyConstraintTeacherNotAvailableTimesForm::setAllAvailable()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			notAllowedTimesTable->item(i, j)->setText(NO);
			colorItem(notAllowedTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(notAllowedTimesTable);
}

void ModifyConstraintTeacherNotAvailableTimesForm::setAllNotAvailable()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			notAllowedTimesTable->item(i, j)->setText(YES);
			colorItem(notAllowedTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(notAllowedTimesTable);
}

void ModifyConstraintTeacherNotAvailableTimesForm::updateTeachersComboBox(){
	teachersComboBox->clear();
	int i=0, j=-1;
	for(int k=0; k<TContext::get()->instance.teachersList.size(); k++, i++){
		Teacher* tch=TContext::get()->instance.teachersList[k];
		teachersComboBox->addItem(tch->name);
		if(tch->name==this->_ctr->teacher)
			j=i;
	}
	assert(j>=0);
	teachersComboBox->setCurrentIndex(j);
}

void ModifyConstraintTeacherNotAvailableTimesForm::itemClicked(QTableWidgetItem* item)
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

	tableWidgetUpdateBug(notAllowedTimesTable);
}

void ModifyConstraintTeacherNotAvailableTimesForm::ok()
{
	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<100.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage). It has to be 100"));
		return;
	}

	QString teacher_name=teachersComboBox->currentText();
	int teacher_ID=TContext::get()->instance.searchTeacher(teacher_name);
	if(teacher_ID<0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid teacher"));
		return;
	}

	foreach(TimeConstraint* c, TContext::get()->instance.timeConstraintsList)
		if(c!=this->_ctr && c->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
			ConstraintTeacherNotAvailableTimes* cc=(ConstraintTeacherNotAvailableTimes*)c;
			if(cc->teacher==teacher_name){
				QMessageBox::warning(this, tr("m-FET information"),
					tr("A constraint of this type exists for the same teacher - cannot proceed"));
				return;
			}
		}


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;

	if(_ctr->teacher!=teacher_name){
		QString oldName=_ctr->teacher;
		QString newName=teacher_name;

		QSet<ConstraintTeacherNotAvailableTimes*> cs=TContext::get()->instance.tnatHash.value(oldName, QSet<ConstraintTeacherNotAvailableTimes*>());
		assert(cs.contains(_ctr));
		cs.remove(_ctr);
		TContext::get()->instance.tnatHash.insert(oldName, cs);

		cs=TContext::get()->instance.tnatHash.value(newName, QSet<ConstraintTeacherNotAvailableTimes*>());
		assert(!cs.contains(_ctr));
		cs.insert(_ctr);
		TContext::get()->instance.tnatHash.insert(newName, cs);
		
		this->_ctr->teacher=teacher_name;
	}

	QList<int> days;
	QList<int> hours;
	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
		for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
			if(notAllowedTimesTable->item(i, j)->text()==YES){
				days.append(j);
				hours.append(i);
			}

	this->_ctr->days=days;
	this->_ctr->hours=hours;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintTeacherNotAvailableTimesForm::cancel()
{
	this->close();
}

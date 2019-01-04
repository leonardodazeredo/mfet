/***************************************************************************
                          constraintactivitiessamestartinghourform.cpp  -  description
                             -------------------
    begin                : 15 Feb 2005
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

#include "textmessages.h"

#include "constraintactivitiessamestartinghourform.h"
#include "addconstraintactivitiessamestartinghourform.h"
#include "modifyconstraintactivitiessamestartinghourform.h"

#include <QListWidget>
#include <QScrollBar>
#include <QAbstractItemView>

ConstraintActivitiesSameStartingHourForm::ConstraintActivitiesSameStartingHourForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	currentConstraintTextEdit->setReadOnly(true);
	
	modifyConstraintPushButton->setDefault(true);
	
	constraintsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addConstraint()));
	connect(removeConstraintPushButton, SIGNAL(clicked()), this, SLOT(removeConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(constraintsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(constraintChanged(int)));
	connect(modifyConstraintPushButton, SIGNAL(clicked()), this, SLOT(modifyConstraint()));
	connect(constraintsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyConstraint()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	this->refreshConstraintsListWidget();
}

ConstraintActivitiesSameStartingHourForm::~ConstraintActivitiesSameStartingHourForm()
{
	saveFETDialogGeometry(this);
}

void ConstraintActivitiesSameStartingHourForm::refreshConstraintsListWidget()
{
	this->visibleConstraintsList.clear();
	constraintsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.timeConstraintsList.size(); i++){
		TimeConstraint* ctr=TContext::get()->instance.timeConstraintsList[i];
		if(filterOk(ctr)){
			QString s;
			s=ctr->getDescription();
			visibleConstraintsList.append(ctr);
			constraintsListWidget->addItem(s);
		}
	}
	if(constraintsListWidget->count()>0)
		constraintsListWidget->setCurrentRow(0);
	else
		this->constraintChanged(-1);
}

bool ConstraintActivitiesSameStartingHourForm::filterOk(TimeConstraint* ctr)
{
	if(ctr->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR)
		return true;
	else
		return false;
}

void ConstraintActivitiesSameStartingHourForm::constraintChanged(int index)
{
	if(index<0){
		currentConstraintTextEdit->setPlainText("");
		return;
	}
	
	QString s;
	assert(index<this->visibleConstraintsList.size());
	TimeConstraint* ctr=this->visibleConstraintsList.at(index);
	assert(ctr!=nullptr);
	s=ctr->getDetailedDescription();
	currentConstraintTextEdit->setPlainText(s);
}

void ConstraintActivitiesSameStartingHourForm::addConstraint()
{
	AddConstraintActivitiesSameStartingHourForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();

	this->refreshConstraintsListWidget();
	
	constraintsListWidget->setCurrentRow(constraintsListWidget->count()-1);
}

void ConstraintActivitiesSameStartingHourForm::modifyConstraint()
{
	int valv=constraintsListWidget->verticalScrollBar()->value();
	int valh=constraintsListWidget->horizontalScrollBar()->value();

	int i=constraintsListWidget->currentRow();
	if(i<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected constraint"));
		return;
	}
	TimeConstraint* ctr=this->visibleConstraintsList.at(i);

	ModifyConstraintActivitiesSameStartingHourForm form(this, (ConstraintActivitiesSameStartingHour*)ctr);
	setParentAndOtherThings(&form, this);
	form.exec();

	this->refreshConstraintsListWidget();

	constraintsListWidget->verticalScrollBar()->setValue(valv);
	constraintsListWidget->horizontalScrollBar()->setValue(valh);
	
	if(i>=constraintsListWidget->count())
		i=constraintsListWidget->count()-1;

	if(i>=0)
		constraintsListWidget->setCurrentRow(i);
	else
		this->constraintChanged(-1);
}

void ConstraintActivitiesSameStartingHourForm::removeConstraint()
{
	int i=constraintsListWidget->currentRow();
	if(i<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected constraint"));
		return;
	}
	TimeConstraint* ctr=this->visibleConstraintsList.at(i);
	QString s;
	s=tr("Remove constraint?");
	s+="\n\n";
	s+=ctr->getDetailedDescription();
	
	QListWidgetItem* item;

	switch( MessagesManager::confirmation( this, tr("m-FET confirmation"),
		s, tr("Yes"), tr("No"), 0, 0, 1 ) ){
	case 0: // The user clicked the OK button or pressed Enter
		TContext::get()->instance.removeTimeConstraint(ctr);
		
		visibleConstraintsList.removeAt(i);
		constraintsListWidget->setCurrentRow(-1);
		item=constraintsListWidget->takeItem(i);
		delete item;
		
		break;
	case 1: // The user clicked the Cancel button or pressed Escape
		break;
	}

	if(i>=constraintsListWidget->count())
		i=constraintsListWidget->count()-1;
	if(i>=0)
		constraintsListWidget->setCurrentRow(i);
	else
		this->constraintChanged(-1);
}

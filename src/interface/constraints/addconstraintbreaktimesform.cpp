/***************************************************************************
                          addconstraintbreaktimesform.cpp  -  description
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

#include "textmessages.h"

#include "addconstraintbreaktimesform.h"
#include "timeconstraint.h"
#include "interfaceutils.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QBrush>
#include <QColor>
#include "stringutils.h"

#define YES	(QString("X"))
#define NO	(QString(" "))

AddConstraintBreakTimesForm::AddConstraintBreakTimesForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(notAllowedTimesTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(setAllAllowedPushButton, SIGNAL(clicked()), this, SLOT(setAllAllowed()));
	connect(setAllBreakPushButton, SIGNAL(clicked()), this, SLOT(setAllBreak()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
							
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

	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			QTableWidgetItem* item=new QTableWidgetItem(NO);
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			colorItem(item);
			if(defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES)
				item->setToolTip(TContext::get()->instance.daysOfTheWeek[j]+QString("\n")+TContext::get()->instance.hoursOfTheDay[i]);
			notAllowedTimesTable->setItem(i, j, item);
		}
		
	notAllowedTimesTable->resizeRowsToContents();
	//notAllowedTimesTable->resizeColumnsToContents();

	connect(notAllowedTimesTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(horizontalHeaderClicked(int)));
	connect(notAllowedTimesTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(verticalHeaderClicked(int)));

	notAllowedTimesTable->setSelectionMode(QAbstractItemView::NoSelection);
	
	tableWidgetUpdateBug(notAllowedTimesTable);
	
	setStretchAvailabilityTableNicely(notAllowedTimesTable);
}

AddConstraintBreakTimesForm::~AddConstraintBreakTimesForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintBreakTimesForm::colorItem(QTableWidgetItem* item)
{
	if(defs::USE_GUI_COLORS){
		if(item->text()==NO)
			item->setBackground(QBrush(Qt::darkGreen));
		else
			item->setBackground(QBrush(Qt::darkRed));
		item->setForeground(QBrush(Qt::lightGray));
	}
}

void AddConstraintBreakTimesForm::horizontalHeaderClicked(int col)
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

void AddConstraintBreakTimesForm::verticalHeaderClicked(int row)
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

void AddConstraintBreakTimesForm::setAllAllowed()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			notAllowedTimesTable->item(i, j)->setText(NO);
			colorItem(notAllowedTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(notAllowedTimesTable);
}

void AddConstraintBreakTimesForm::setAllBreak()
{
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
			notAllowedTimesTable->item(i, j)->setText(YES);
			colorItem(notAllowedTimesTable->item(i,j));
		}
	tableWidgetUpdateBug(notAllowedTimesTable);
}

void AddConstraintBreakTimesForm::itemClicked(QTableWidgetItem* item)
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

void AddConstraintBreakTimesForm::addCurrentConstraint()
{
	TimeConstraint *ctr=nullptr;

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<100.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage). It has to be 100"));
		return;
	}

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	QList<int> days;
	QList<int> hours;
	for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
		for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
			if(notAllowedTimesTable->item(i, j)->text()==YES){
				days.append(j);
				hours.append(i);
			}

    ctr=new ConstraintBreakTimes(TContext::get()->instance, group, weight, days, hours);

	bool tmp2=TContext::get()->instance.addTimeConstraint(ctr);
	if(tmp2)
		MessagesManager::information(this, tr("m-FET information"),
			tr("Constraint added:")+"\n\n"+ctr->getDetailedDescription());
	else{
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Constraint NOT added - there must be another constraint of this "
			  "type. Please edit that one"));
		delete ctr;
	}
}

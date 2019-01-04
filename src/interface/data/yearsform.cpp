
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "addstudentsyearform.h"
#include "modifystudentsyearform.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"
#include "yearsform.h"
#include "studentsset.h"

#include "splityearform.h"

#include "centerwidgetonscreen.h"


#include <QMessageBox>

#include <QListWidget>
#include <QScrollBar>
#include <QAbstractItemView>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>


YearsForm::YearsForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	
	detailsTextEdit->setReadOnly(true);
	
	modifyYearPushButton->setDefault(true);

	yearsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(addYearPushButton, SIGNAL(clicked()), this, SLOT(addYear()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(removeYearPushButton, SIGNAL(clicked()), this, SLOT(removeYear()));
	connect(yearsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(yearChanged()));
	connect(modifyYearPushButton, SIGNAL(clicked()), this, SLOT(modifyYear()));

	connect(moveYearUpPushButton, SIGNAL(clicked()), this, SLOT(moveYearUp()));
	connect(moveYearDownPushButton, SIGNAL(clicked()), this, SLOT(moveYearDown()));

	connect(sortYearsPushButton, SIGNAL(clicked()), this, SLOT(sortYears()));
	connect(activateStudentsPushButton, SIGNAL(clicked()), this, SLOT(activateStudents()));
	connect(deactivateStudentsPushButton, SIGNAL(clicked()), this, SLOT(deactivateStudents()));
	connect(divisionsPushButton, SIGNAL(clicked()), this, SLOT(divideYear()));
	connect(yearsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyYear()));

	connect(commentsPushButton, SIGNAL(clicked()), this, SLOT(comments()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	//restore splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/splitter-state")))
		splitter->restoreState(settings.value(this->metaObject()->className()+QString("/splitter-state")).toByteArray());
	
	yearsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* year=TContext::get()->instance.yearsList[i];
		yearsListWidget->addItem(year->name);
	}
	
	if(yearsListWidget->count()>0)
		yearsListWidget->setCurrentRow(0);
}

YearsForm::~YearsForm()
{
	saveFETDialogGeometry(this);
	//save splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/splitter-state"), splitter->saveState());
}

void YearsForm::addYear()
{
	AddStudentsYearForm form(this);
	setParentAndOtherThings(&form, this);
	form.exec();

	yearsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* year=TContext::get()->instance.yearsList[i];
		yearsListWidget->addItem(year->name);
	}
	
	int i=yearsListWidget->count()-1;
	if(i>=0)
		yearsListWidget->setCurrentRow(i);
}

void YearsForm::removeYear()
{
	if(yearsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	QString yearName=yearsListWidget->currentItem()->text();
	int yearIndex=TContext::get()->instance.searchYear(yearName);
	assert(yearIndex>=0);

	if(QMessageBox::warning( this, tr("m-FET"),
		tr("Are you sure you want to delete year %1 and all related groups, subgroups, activities and constraints?").arg(yearName),
		tr("Yes"), tr("No"), 0, 0, 1 ) == 1)
		return;

	bool tmp=TContext::get()->instance.removeYear(yearName);
	assert(tmp);
	if(tmp){
		int q=yearsListWidget->currentRow();

		yearsListWidget->setCurrentRow(-1);
		QListWidgetItem* item;
		item=yearsListWidget->takeItem(q);
		delete item;

		if(q>=yearsListWidget->count())
			q=yearsListWidget->count()-1;
		if(q>=0)
			yearsListWidget->setCurrentRow(q);
		else
			detailsTextEdit->setPlainText(QString(""));
	}
}

void YearsForm::yearChanged()
{
	if(yearsListWidget->currentRow()<0){
		detailsTextEdit->setPlainText("");
		return;
	}
	StudentsYear* sty=TContext::get()->instance.yearsList.at(yearsListWidget->currentRow());
	detailsTextEdit->setPlainText(sty->getDetailedDescriptionWithConstraints(TContext::get()->instance));
}

void YearsForm::moveYearUp()
{
	if(yearsListWidget->count()<=1)
		return;
	int i=yearsListWidget->currentRow();
	if(i<0 || i>=yearsListWidget->count())
		return;
	if(i==0)
		return;
		
	QString s1=yearsListWidget->item(i)->text();
	QString s2=yearsListWidget->item(i-1)->text();
	
	StudentsYear* sy1=TContext::get()->instance.yearsList.at(i);
	StudentsYear* sy2=TContext::get()->instance.yearsList.at(i-1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	yearsListWidget->item(i)->setText(s2);
	yearsListWidget->item(i-1)->setText(s1);
	
	TContext::get()->instance.yearsList[i]=sy2;
	TContext::get()->instance.yearsList[i-1]=sy1;
	
	yearsListWidget->setCurrentRow(i-1);
	yearChanged(/*i-1*/);
}

void YearsForm::moveYearDown()
{
	if(yearsListWidget->count()<=1)
		return;
	int i=yearsListWidget->currentRow();
	if(i<0 || i>=yearsListWidget->count())
		return;
	if(i==yearsListWidget->count()-1)
		return;
		
	QString s1=yearsListWidget->item(i)->text();
	QString s2=yearsListWidget->item(i+1)->text();
	
	StudentsYear* sy1=TContext::get()->instance.yearsList.at(i);
	StudentsYear* sy2=TContext::get()->instance.yearsList.at(i+1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	yearsListWidget->item(i)->setText(s2);
	yearsListWidget->item(i+1)->setText(s1);
	
	TContext::get()->instance.yearsList[i]=sy2;
	TContext::get()->instance.yearsList[i+1]=sy1;
	
	yearsListWidget->setCurrentRow(i+1);
	yearChanged(/*i+1*/);
}

void YearsForm::sortYears()
{
	TContext::get()->instance.sortYearsAlphabetically();

	yearsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* year=TContext::get()->instance.yearsList[i];
		yearsListWidget->addItem(year->name);
	}

	if(yearsListWidget->count()>0)
		yearsListWidget->setCurrentRow(0);
}

void YearsForm::modifyYear()
{
	int q=yearsListWidget->currentRow();
	int valv=yearsListWidget->verticalScrollBar()->value();
	int valh=yearsListWidget->horizontalScrollBar()->value();

	if(yearsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	QString yearName=yearsListWidget->currentItem()->text();
	
	StudentsSet* sset=TContext::get()->instance.searchStudentsSet(yearName);
	assert(sset!=nullptr);
	int numberOfStudents=sset->numberOfStudents;

	ModifyStudentsYearForm form(this, yearName, numberOfStudents);
	setParentAndOtherThings(&form, this);
	form.exec();

	yearsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* year=TContext::get()->instance.yearsList[i];
		yearsListWidget->addItem(year->name);
	}
	
	yearsListWidget->verticalScrollBar()->setValue(valv);
	yearsListWidget->horizontalScrollBar()->setValue(valh);

	if(q>=yearsListWidget->count())
		q=yearsListWidget->count()-1;
	if(q>=0)
		yearsListWidget->setCurrentRow(q);
	else
		detailsTextEdit->setPlainText(QString(""));
}

void YearsForm::activateStudents()
{
	if(yearsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	
	QString yearName=yearsListWidget->currentItem()->text();
	int count=TContext::get()->instance.activateStudents(yearName);
	QMessageBox::information(this, tr("m-FET information"), tr("Activated a number of %1 activities").arg(count));
}

void YearsForm::deactivateStudents()
{
	if(yearsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	
	QString yearName=yearsListWidget->currentItem()->text();
	int count=TContext::get()->instance.deactivateStudents(yearName);
	QMessageBox::information(this, tr("m-FET information"), tr("De-activated a number of %1 activities").arg(count));
}

void YearsForm::divideYear()
{
	if(yearsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	
	QString yearName=yearsListWidget->currentItem()->text();
	
	SplitYearForm form(this, yearName);
	setParentAndOtherThings(&form, this);
	form.exec();
}

void YearsForm::comments()
{
	int ind=yearsListWidget->currentRow();
	if(ind<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected year"));
		return;
	}
	
	QString yearName=yearsListWidget->currentItem()->text();
	
	StudentsSet* sset=TContext::get()->instance.searchStudentsSet(yearName);
	assert(sset!=nullptr);

	QDialog getCommentsDialog(this);
	
	getCommentsDialog.setWindowTitle(tr("Students year comments"));
	
	QPushButton* okPB=new QPushButton(tr("OK"));
	okPB->setDefault(true);
	QPushButton* cancelPB=new QPushButton(tr("Cancel"));
	
	connect(okPB, SIGNAL(clicked()), &getCommentsDialog, SLOT(accept()));
	connect(cancelPB, SIGNAL(clicked()), &getCommentsDialog, SLOT(reject()));

	QHBoxLayout* hl=new QHBoxLayout();
	hl->addStretch();
	hl->addWidget(okPB);
	hl->addWidget(cancelPB);
	
	QVBoxLayout* vl=new QVBoxLayout();
	
	QPlainTextEdit* commentsPT=new QPlainTextEdit();
	commentsPT->setPlainText(sset->comments);
	commentsPT->selectAll();
	commentsPT->setFocus();
	
	vl->addWidget(commentsPT);
	vl->addLayout(hl);
	
	getCommentsDialog.setLayout(vl);
	
	const QString settingsName=QString("StudentsYearCommentsDialog");
	
	getCommentsDialog.resize(500, 320);
	centerWidgetOnScreen(&getCommentsDialog);
	restoreFETDialogGeometry(&getCommentsDialog, settingsName);
	
	int t=getCommentsDialog.exec();
	saveFETDialogGeometry(&getCommentsDialog, settingsName);
	
	if(t==QDialog::Accepted){
		sset->comments=commentsPT->toPlainText();
	
		TContext::get()->instance.internalStructureComputed=false;
		setRulesModifiedAndOtherThings(&TContext::get()->instance);

		yearChanged();
	}
}

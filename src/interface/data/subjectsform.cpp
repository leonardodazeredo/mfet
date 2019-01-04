
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"
#include "subjectsform.h"
#include "studentsset.h"
#include "teacher.h"
#include "subject.h"

#include "centerwidgetonscreen.h"


#include <QInputDialog>

#include <QMessageBox>

#include <QListWidget>
#include <QAbstractItemView>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>


SubjectsForm::SubjectsForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	
	currentSubjectTextEdit->setReadOnly(true);
	
	renameSubjectPushButton->setDefault(true);

	subjectsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(addSubjectPushButton, SIGNAL(clicked()), this, SLOT(addSubject()));
	connect(removeSubjectPushButton, SIGNAL(clicked()), this, SLOT(removeSubject()));
	connect(renameSubjectPushButton, SIGNAL(clicked()), this, SLOT(renameSubject()));

	connect(moveSubjectUpPushButton, SIGNAL(clicked()), this, SLOT(moveSubjectUp()));
	connect(moveSubjectDownPushButton, SIGNAL(clicked()), this, SLOT(moveSubjectDown()));

	connect(sortSubjectsPushButton, SIGNAL(clicked()), this, SLOT(sortSubjects()));
	connect(subjectsListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(subjectChanged(int)));
	connect(activateSubjectPushButton, SIGNAL(clicked()), this, SLOT(activateSubject()));
	connect(deactivateSubjectPushButton, SIGNAL(clicked()), this, SLOT(deactivateSubject()));
	connect(subjectsListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(renameSubject()));

	connect(commentsPushButton, SIGNAL(clicked()), this, SLOT(comments()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	//restore splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/splitter-state")))
		splitter->restoreState(settings.value(this->metaObject()->className()+QString("/splitter-state")).toByteArray());
	
	subjectsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
		Subject* sbj=TContext::get()->instance.subjectsList[i];
		subjectsListWidget->addItem(sbj->name);
	}
	
	if(subjectsListWidget->count()>0)
		subjectsListWidget->setCurrentRow(0);
}


SubjectsForm::~SubjectsForm()
{
	saveFETDialogGeometry(this);
	//save splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/splitter-state"), splitter->saveState());
}

void SubjectsForm::addSubject()
{
	bool ok = false;
	Subject* sbj=new Subject();
	sbj->name = QInputDialog::getText( this, tr("Add subject"), tr("Please enter subject's name") ,
	 QLineEdit::Normal, QString(), &ok );

	if ( ok && !((sbj->name).isEmpty()) ){
		// user entered something and pressed OK
		if(!TContext::get()->instance.addSubject(sbj)){
			QMessageBox::information( this, tr("Subject insertion dialog"),
				tr("Could not insert item. Must be a duplicate"));
			delete sbj;
		}
		else{
			subjectsListWidget->addItem(sbj->name);
			subjectsListWidget->setCurrentRow(subjectsListWidget->count()-1);
		}
	}
	else{
		if(ok){ //the user entered nothing
			QMessageBox::information(this, tr("m-FET information"), tr("Incorrect name"));
		}
		delete sbj;// user entered nothing or pressed Cancel
	}
}

void SubjectsForm::removeSubject()
{
	int i=subjectsListWidget->currentRow();
	if(subjectsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}

	QString text=subjectsListWidget->currentItem()->text();
	int subject_ID=TContext::get()->instance.searchSubject(text);
	if(subject_ID<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}

	if(QMessageBox::warning( this, tr("m-FET"),
		tr("Are you sure you want to delete this subject and all related activities and constraints?"),
		tr("Yes"), tr("No"), 0, 0, 1 ) == 1)
		return;

	int tmp=TContext::get()->instance.removeSubject(text);
	if(tmp){
		subjectsListWidget->setCurrentRow(-1);
		QListWidgetItem* item;
		item=subjectsListWidget->takeItem(i);
		delete item;

		if(i>=subjectsListWidget->count())
			i=subjectsListWidget->count()-1;
		if(i>=0)
			subjectsListWidget->setCurrentRow(i);
		else
			currentSubjectTextEdit->setPlainText(QString(""));
	}
}

void SubjectsForm::renameSubject()
{
	int i=subjectsListWidget->currentRow();
	if(subjectsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}
	
	QString initialSubjectName=subjectsListWidget->currentItem()->text();

	int subject_ID=TContext::get()->instance.searchSubject(initialSubjectName);
	if(subject_ID<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}

	bool ok = false;
	QString finalSubjectName;
	finalSubjectName = QInputDialog::getText( this, tr("Modify subject"), tr("Please enter new subject's name") ,
	 QLineEdit::Normal, initialSubjectName, &ok);

	if ( ok && !(finalSubjectName.isEmpty()) ){
		// user entered something and pressed OK
		if(TContext::get()->instance.searchSubject(finalSubjectName)>=0){
			QMessageBox::information( this, tr("Subject insertion dialog"),
				tr("Could not modify item. New name must be a duplicate"));
		}
		else{
			TContext::get()->instance.modifySubject(initialSubjectName, finalSubjectName);
			subjectsListWidget->item(i)->setText(finalSubjectName);
			subjectChanged(subjectsListWidget->currentRow());
		}
	}
}

void SubjectsForm::moveSubjectUp()
{
	if(subjectsListWidget->count()<=1)
		return;
	int i=subjectsListWidget->currentRow();
	if(i<0 || i>=subjectsListWidget->count())
		return;
	if(i==0)
		return;
		
	QString s1=subjectsListWidget->item(i)->text();
	QString s2=subjectsListWidget->item(i-1)->text();
	
	Subject* sbj1=TContext::get()->instance.subjectsList.at(i);
	Subject* sbj2=TContext::get()->instance.subjectsList.at(i-1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	subjectsListWidget->item(i)->setText(s2);
	subjectsListWidget->item(i-1)->setText(s1);
	
	TContext::get()->instance.subjectsList[i]=sbj2;
	TContext::get()->instance.subjectsList[i-1]=sbj1;
	
	subjectsListWidget->setCurrentRow(i-1);
	subjectChanged(i-1);
}

void SubjectsForm::moveSubjectDown()
{
	if(subjectsListWidget->count()<=1)
		return;
	int i=subjectsListWidget->currentRow();
	if(i<0 || i>=subjectsListWidget->count())
		return;
	if(i==subjectsListWidget->count()-1)
		return;
		
	QString s1=subjectsListWidget->item(i)->text();
	QString s2=subjectsListWidget->item(i+1)->text();
	
	Subject* sbj1=TContext::get()->instance.subjectsList.at(i);
	Subject* sbj2=TContext::get()->instance.subjectsList.at(i+1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	subjectsListWidget->item(i)->setText(s2);
	subjectsListWidget->item(i+1)->setText(s1);
	
	TContext::get()->instance.subjectsList[i]=sbj2;
	TContext::get()->instance.subjectsList[i+1]=sbj1;
	
	subjectsListWidget->setCurrentRow(i+1);
	subjectChanged(i+1);
}

void SubjectsForm::sortSubjects()
{
	TContext::get()->instance.sortSubjectsAlphabetically();
	
	subjectsListWidget->clear();
	for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
		Subject* sbj=TContext::get()->instance.subjectsList[i];
		subjectsListWidget->addItem(sbj->name);
	}
	
	if(subjectsListWidget->count()>0)
		subjectsListWidget->setCurrentRow(0);
}

void SubjectsForm::subjectChanged(int index)
{
	if(index<0){
		currentSubjectTextEdit->setPlainText(QString(""));
		return;
	}
	
	Subject* sb=TContext::get()->instance.subjectsList.at(index);
	assert(sb);
	QString s=sb->getDetailedDescriptionWithConstraints(TContext::get()->instance);
	currentSubjectTextEdit->setPlainText(s);
}

void SubjectsForm::activateSubject()
{
	if(subjectsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}
	
	QString subjectName=subjectsListWidget->currentItem()->text();
	
	int count=TContext::get()->instance.activateSubject(subjectName);
	QMessageBox::information(this, tr("m-FET information"), tr("Activated a number of %1 activities").arg(count));
}

void SubjectsForm::deactivateSubject()
{
	if(subjectsListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}
	
	QString subjectName=subjectsListWidget->currentItem()->text();
	
	int count=TContext::get()->instance.deactivateSubject(subjectName);
	QMessageBox::information(this, tr("m-FET information"), tr("De-activated a number of %1 activities").arg(count));
}

void SubjectsForm::comments()
{
	int ind=subjectsListWidget->currentRow();
	if(ind<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected subject"));
		return;
	}
	
	Subject* sbj=TContext::get()->instance.subjectsList[ind];
	assert(sbj!=nullptr);

	QDialog getCommentsDialog(this);
	
	getCommentsDialog.setWindowTitle(tr("Subject comments"));
	
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
	commentsPT->setPlainText(sbj->comments);
	commentsPT->selectAll();
	commentsPT->setFocus();
	
	vl->addWidget(commentsPT);
	vl->addLayout(hl);
	
	getCommentsDialog.setLayout(vl);
	
	const QString settingsName=QString("SubjectCommentsDialog");
	
	getCommentsDialog.resize(500, 320);
	centerWidgetOnScreen(&getCommentsDialog);
	restoreFETDialogGeometry(&getCommentsDialog, settingsName);
	
	int t=getCommentsDialog.exec();
	saveFETDialogGeometry(&getCommentsDialog, settingsName);
	
	if(t==QDialog::Accepted){
		sbj->comments=commentsPT->toPlainText();
	
		TContext::get()->instance.internalStructureComputed=false;
		setRulesModifiedAndOtherThings(&TContext::get()->instance);

		subjectChanged(ind);
	}
}

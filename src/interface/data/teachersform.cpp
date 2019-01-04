//
//
// Description: This file is part of m-FET
//
//
// Author: Lalescu Liviu <Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)>
// Copyright (C) 2003 Liviu Lalescu <http://lalescu.ro/liviu/>
//
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
#include "teachersform.h"
#include "teacher.h"
#include "teachersubjectsqualificationsform.h"
#include "centerwidgetonscreen.h"


#include <QInputDialog>

#include <QMessageBox>

#include <QListWidget>
#include <QAbstractItemView>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>


TeachersForm::TeachersForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	
	currentTeacherTextEdit->setReadOnly(true);

	renameTeacherPushButton->setDefault(true);

	teachersListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(renameTeacherPushButton, SIGNAL(clicked()), this, SLOT(renameTeacher()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(addTeacherPushButton, SIGNAL(clicked()), this, SLOT(addTeacher()));

	connect(targetNumberOfHoursPushButton, SIGNAL(clicked()), this, SLOT(targetNumberOfHours()));
	connect(qualifiedSubjectsPushButton, SIGNAL(clicked()), this, SLOT(qualifiedSubjects()));

	connect(moveTeacherUpPushButton, SIGNAL(clicked()), this, SLOT(moveTeacherUp()));
	connect(moveTeacherDownPushButton, SIGNAL(clicked()), this, SLOT(moveTeacherDown()));

	connect(sortTeachersPushButton, SIGNAL(clicked()), this, SLOT(sortTeachers()));
	connect(removeTeacherPushButton, SIGNAL(clicked()), this, SLOT(removeTeacher()));
	connect(teachersListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(teacherChanged(int)));
    connect(teachersListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(qualifiedSubjects()));

	connect(commentsPushButton, SIGNAL(clicked()), this, SLOT(comments()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	//restore splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/splitter-state")))
		splitter->restoreState(settings.value(this->metaObject()->className()+QString("/splitter-state")).toByteArray());
	
    teachersChanged();
}

TeachersForm::~TeachersForm()
{
	saveFETDialogGeometry(this);
	//save splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/splitter-state"), splitter->saveState());
}

void TeachersForm::addTeacher()
{
	bool ok = false;
	Teacher* tch=new Teacher();
	tch->name = QInputDialog::getText( this, tr("Add teacher"), tr("Please enter teacher's name") ,
	 QLineEdit::Normal, QString(), &ok );

	if ( ok && !((tch->name).isEmpty()) ){
		// user entered something and pressed OK
		if(!TContext::get()->instance.addTeacher(tch)){
			QMessageBox::information( this, tr("Teacher insertion dialog"),
				tr("Could not insert item. Must be a duplicate"));
			delete tch;
		}
		else{
			teachersListWidget->addItem(tch->name);
			teachersListWidget->setCurrentRow(teachersListWidget->count()-1);
		}
	}
	else{
		if(ok){ //the user entered nothing
			QMessageBox::information(this, tr("m-FET information"), tr("Incorrect name"));
		}
		delete tch;// user entered nothing or pressed Cancel
	}
}

void TeachersForm::removeTeacher()
{
	int i=teachersListWidget->currentRow();
	if(teachersListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}

    int teacher_ID=teachersListWidget->currentRow();

	if(teacher_ID<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}

	if(QMessageBox::warning( this, tr("m-FET"),
		tr("Are you sure you want to delete this teacher and all related activities and constraints?"),
		tr("Yes"), tr("No"), 0, 0, 1 ) == 1)
		return;

    QString text=TContext::get()->instance.teachersList.at(teacher_ID)->name;

	int tmp=TContext::get()->instance.removeTeacher(text);
	if(tmp){
		teachersListWidget->setCurrentRow(-1);
		QListWidgetItem* item;
		item=teachersListWidget->takeItem(i);
		delete item;

		if(i>=teachersListWidget->count())
			i=teachersListWidget->count()-1;
		if(i>=0)
			teachersListWidget->setCurrentRow(i);
		else
			currentTeacherTextEdit->setPlainText(QString(""));
	}
}

void TeachersForm::renameTeacher()
{
//	int i=teachersListWidget->currentRow();
	if(teachersListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}

    int teacher_ID=teachersListWidget->currentRow();
    QString initialTeacherName=TContext::get()->instance.teachersList.at(teacher_ID)->name;

	bool ok = false;
	QString finalTeacherName;
	finalTeacherName = QInputDialog::getText( this, tr("Modify teacher"), tr("Please enter new teacher's name") ,
	 QLineEdit::Normal, initialTeacherName, &ok);

	if ( ok && !(finalTeacherName.isEmpty())){
		// user entered something and pressed OK
		if(TContext::get()->instance.searchTeacher(finalTeacherName)>=0){
			QMessageBox::information( this, tr("Teacher modification dialog"),
				tr("Could not modify item. New name must be a duplicate"));
		}
		else{
			TContext::get()->instance.modifyTeacher(initialTeacherName, finalTeacherName);

            teachersChanged();
		}
	}
}

void TeachersForm::targetNumberOfHours()
{
	if(teachersListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}

    int teacher_ID=teachersListWidget->currentRow();
	
	Teacher* tch=TContext::get()->instance.teachersList.at(teacher_ID);
    assert(tch);

    QString teacherName=TContext::get()->instance.teachersList.at(teacher_ID)->name;

	bool ok = false;
	int newTargetNumberOfHours = QInputDialog::getInt(this, tr("Target number of hours"), tr("Please enter the target number of hours for teacher %1").arg(teacherName),
	 tch->targetNumberOfHours, 0, TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay, 1, &ok);

	if(ok){
		// user entered something and pressed OK
		tch->targetNumberOfHours=newTargetNumberOfHours;

		TContext::get()->instance.internalStructureComputed=false;
		setRulesModifiedAndOtherThings(&TContext::get()->instance);

		teacherChanged(teachersListWidget->currentRow());
	}
}

void TeachersForm::qualifiedSubjects()
{
	if(teachersListWidget->currentRow()<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}

    int teacher_ID=teachersListWidget->currentRow();
	
	Teacher* tch=TContext::get()->instance.teachersList.at(teacher_ID);
    assert(tch);

	TeacherSubjectsQualificationsForm form(this, tch);
	setParentAndOtherThings(&form, this);
	form.exec();

    teachersChanged();
}

void TeachersForm::moveTeacherUp()
{
	if(teachersListWidget->count()<=1)
		return;
	int i=teachersListWidget->currentRow();
	if(i<0 || i>=teachersListWidget->count())
		return;
	if(i==0)
		return;
		
	QString s1=teachersListWidget->item(i)->text();
	QString s2=teachersListWidget->item(i-1)->text();
	
	Teacher* at1=TContext::get()->instance.teachersList.at(i);
	Teacher* at2=TContext::get()->instance.teachersList.at(i-1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	teachersListWidget->item(i)->setText(s2);
	teachersListWidget->item(i-1)->setText(s1);
	
	TContext::get()->instance.teachersList[i]=at2;
	TContext::get()->instance.teachersList[i-1]=at1;
	
	teachersListWidget->setCurrentRow(i-1);

    teachersChanged();
}

void TeachersForm::moveTeacherDown()
{
	if(teachersListWidget->count()<=1)
		return;
	int i=teachersListWidget->currentRow();
	if(i<0 || i>=teachersListWidget->count())
		return;
	if(i==teachersListWidget->count()-1)
		return;
		
	QString s1=teachersListWidget->item(i)->text();
	QString s2=teachersListWidget->item(i+1)->text();
	
	Teacher* at1=TContext::get()->instance.teachersList.at(i);
	Teacher* at2=TContext::get()->instance.teachersList.at(i+1);
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	teachersListWidget->item(i)->setText(s2);
	teachersListWidget->item(i+1)->setText(s1);
	
	TContext::get()->instance.teachersList[i]=at2;
	TContext::get()->instance.teachersList[i+1]=at1;
	
	teachersListWidget->setCurrentRow(i+1);

    teachersChanged();
}

void TeachersForm::sortTeachers()
{
	TContext::get()->instance.sortTeachersAlphabetically();

    teachersChanged();
}

void TeachersForm::teacherChanged(int index)
{
	if(index<0){
		currentTeacherTextEdit->setPlainText("");
		return;
	}
	
	Teacher* t=TContext::get()->instance.teachersList.at(index);
	assert(t);
	QString s=t->getDetailedDescriptionWithConstraints(TContext::get()->instance);
    currentTeacherTextEdit->setPlainText(s);
}

void TeachersForm::teachersChanged()
{
    int i = teachersListWidget->currentRow();

    teachersListWidget->clear();
    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        Teacher* tch=TContext::get()->instance.teachersList[i];
        teachersListWidget->addItem(tch->active ? tch->name : QString(" X - ") + tch->name);
    }

    if(teachersListWidget->count()>0){
        if (i >= 0) {
            teachersListWidget->setCurrentRow(i);
        }
        else{
            teachersListWidget->setCurrentRow(0);
        }
    }

    teacherChanged(teachersListWidget->currentRow());
}

void TeachersForm::comments()
{
	int ind=teachersListWidget->currentRow();
	if(ind<0){
		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
		return;
	}
	
	Teacher* tch=TContext::get()->instance.teachersList[ind];
	assert(tch!=nullptr);

	QDialog getCommentsDialog(this);
	
	getCommentsDialog.setWindowTitle(tr("Teacher comments"));
	
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
	commentsPT->setPlainText(tch->comments);
	commentsPT->selectAll();
	commentsPT->setFocus();
	
	vl->addWidget(commentsPT);
	vl->addLayout(hl);
	
	getCommentsDialog.setLayout(vl);
	
	const QString settingsName=QString("TeacherCommentsDialog");
	
	getCommentsDialog.resize(500, 320);
	centerWidgetOnScreen(&getCommentsDialog);
	restoreFETDialogGeometry(&getCommentsDialog, settingsName);
	
	int t=getCommentsDialog.exec();
	saveFETDialogGeometry(&getCommentsDialog, settingsName);
	
	if(t==QDialog::Accepted){
		tch->comments=commentsPT->toPlainText();
	
		TContext::get()->instance.internalStructureComputed=false;
		setRulesModifiedAndOtherThings(&TContext::get()->instance);

		teacherChanged(ind);
	}
}

void TeachersForm::on_activateTeacherPushButton_clicked()
{
    if(teachersListWidget->currentRow()<0){
        QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
        return;
    }

    int teacher_ID=teachersListWidget->currentRow();

    TContext::get()->instance.teachersList[teacher_ID]->active = true;

    TContext::get()->instance.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&TContext::get()->instance);

    teachersChanged();
}

void TeachersForm::on_deactivateTeacherPushButton_clicked()
{
    if(teachersListWidget->currentRow()<0){
        QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected teacher"));
        return;
    }

    int teacher_ID=teachersListWidget->currentRow();

    TContext::get()->instance.teachersList[teacher_ID]->active = false;

    TContext::get()->instance.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&TContext::get()->instance);

    teachersChanged();
}

void TeachersForm::on_activateAllPushButton_clicked()
{
    for (auto teacher: TContext::get()->instance.teachersList){
        teacher->active = true;
    }

    TContext::get()->instance.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&TContext::get()->instance);

    teachersChanged();
}

void TeachersForm::on_deactivateAllPushButton_clicked()
{
    for (auto teacher: TContext::get()->instance.teachersList){
        teacher->active = false;
    }

    TContext::get()->instance.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&TContext::get()->instance);

    teachersChanged();
}

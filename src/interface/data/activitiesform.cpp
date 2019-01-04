
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"
#include "m-fet.h"
#include "tcontext.h"

#include "studentsset.h"

#include "activitiesform.h"
#include "addactivityform.h"
#include "modifyactivityform.h"

#include "centerwidgetonscreen.h"

#include <QString>
#include <QMessageBox>

#include <QListWidget>
#include <QScrollBar>
#include <QAbstractItemView>

#include "textmessages.h"

#include "interfaceutils.h"

#include <QBrush>
#include <QPalette>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>




ActivitiesForm::ActivitiesForm(QWidget* parent, const QString& teacherName, const QString& studentsSetName, const QString& subjectName, const QString& activityTagName): QDialog(parent)
{
    Q_UNUSED(activityTagName);
    Q_UNUSED(teacherName);

	setupUi(this);
	
	activityTextEdit->setReadOnly(true);

	modifyActivityPushButton->setDefault(true);

	activitiesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(activitiesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(activityChanged()));
	connect(addActivityPushButton, SIGNAL(clicked()), this, SLOT(addActivity()));
	connect(removeActivityPushButton, SIGNAL(clicked()), this, SLOT(removeActivity()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(studentsComboBox, SIGNAL(activated(QString)), this, SLOT(studentsFilterChanged()));
	connect(subjectsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(modifyActivityPushButton, SIGNAL(clicked()), this, SLOT(modifyActivity()));
	connect(activityTagsComboBox, SIGNAL(activated(QString)), this, SLOT(filterChanged()));
	connect(activitiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyActivity()));
	connect(recursiveCheckBox, SIGNAL(toggled(bool)), this, SLOT(studentsFilterChanged()));
	connect(helpPushButton, SIGNAL(clicked()), this, SLOT(help()));
	connect(commentsPushButton, SIGNAL(clicked()), this, SLOT(activityComments()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	//restore splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/splitter-state")))
		splitter->restoreState(settings.value(this->metaObject()->className()+QString("/splitter-state")).toByteArray());
	
	QSize tmp2=studentsComboBox->minimumSizeHint();
	Q_UNUSED(tmp2);
	QSize tmp3=subjectsComboBox->minimumSizeHint();
	Q_UNUSED(tmp3);
	QSize tmp4=activityTagsComboBox->minimumSizeHint();
	Q_UNUSED(tmp4);

	subjectsComboBox->addItem("");
	int cisu=0;
    for(int i=0; i<TContext::get()->instance.subjectsList.size(); i++){
        Subject* sb=TContext::get()->instance.subjectsList[i];
		subjectsComboBox->addItem(sb->name);
		if(sb->name==subjectName)
			cisu=i+1;
	}
	subjectsComboBox->setCurrentIndex(cisu);

	activityTagsComboBox->addItem("");
	int ciat=0;
//	for(int i=0; i<Timetable::getInstance()->rules.activityTagsList.size(); i++){
//		ActivityTag* st=Timetable::getInstance()->rules.activityTagsList[i];
//		activityTagsComboBox->addItem(st->name);
//		if(st->name==activityTagName)
//			ciat=i+1;
//	}
	activityTagsComboBox->setCurrentIndex(ciat);

	studentsComboBox->addItem("");
	int cist=0;
	int currentID=0;
    for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.yearsList[i];
		studentsComboBox->addItem(sty->name);
		currentID++;
		if(sty->name==studentsSetName)
			cist=currentID;
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
			studentsComboBox->addItem(stg->name);
			currentID++;
			if(stg->name==studentsSetName)
				cist=currentID;
			if(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES) for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
				studentsComboBox->addItem(sts->name);
				currentID++;
				if(sts->name==studentsSetName)
					cist=currentID;
			}
		}
	}
	studentsComboBox->setCurrentIndex(cist);
	
	if(studentsSetName!=""){
		if(cist==0){
			showWarningForInvisibleSubgroupActivity(parent, studentsSetName);

			showedStudents.clear();
			showedStudents.insert("");
	
			filterChanged();
		}
		else{
			this->studentsFilterChanged();
		}
	}
	else{
		showedStudents.clear();
		showedStudents.insert("");
	
		filterChanged();
	}
}

ActivitiesForm::~ActivitiesForm()
{
	saveFETDialogGeometry(this);
	//save splitter state
	QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/splitter-state"), splitter->saveState());
}

bool ActivitiesForm::filterOk(Activity* act)
{
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
            //if(*it == stn){
            if(showedStudents.contains(*it)){
                ok2=true;
                break;
            }
        if(!ok2)
            ok=false;
    }
    else{
        assert(showedStudents.count()==1);
        assert(showedStudents.contains(""));
    }
	
	return ok;
}

void ActivitiesForm::studentsFilterChanged()
{
	bool showContained=recursiveCheckBox->isChecked();
	
	showedStudents.clear();
	
	if(!showContained){
		showedStudents.insert(studentsComboBox->currentText());
	}
	else{
		if(studentsComboBox->currentText()=="")
			showedStudents.insert("");
		else{
			//down
            StudentsSet* set=TContext::get()->instance.searchStudentsSet(studentsComboBox->currentText());
			assert(set!=nullptr);
			if(set->type==STUDENTS_YEAR){
				StudentsYear* year=(StudentsYear*)set;
				showedStudents.insert(year->name);
				foreach(StudentsGroup* group, year->groupsList){
					showedStudents.insert(group->name);
					foreach(StudentsSubgroup* subgroup, group->subgroupsList)
						showedStudents.insert(subgroup->name);
				}
			}
			else if(set->type==STUDENTS_GROUP){
				StudentsGroup* group=(StudentsGroup*) set;
				showedStudents.insert(group->name);
				foreach(StudentsSubgroup* subgroup, group->subgroupsList)
					showedStudents.insert(subgroup->name);
			}
			else if(set->type==STUDENTS_SUBGROUP){
				StudentsSubgroup* subgroup=(StudentsSubgroup*) set;
				showedStudents.insert(subgroup->name);
			}
			else
				assert(0);
				
			//up
			QString crt=studentsComboBox->currentText();
            foreach(StudentsYear* year, TContext::get()->instance.yearsList){
				foreach(StudentsGroup* group, year->groupsList){
					if(group->name==crt){
						showedStudents.insert(year->name);
					}
					foreach(StudentsSubgroup* subgroup, group->subgroupsList){
						if(subgroup->name==crt){
							showedStudents.insert(year->name);
							showedStudents.insert(group->name);
						}
					}
				}
			}
		}
	}
	
	filterChanged();
}

void ActivitiesForm::filterChanged()
{
    disconnect(activitiesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(activityChanged()));

	int nacts=0, nsubacts=0, nh=0;
	int ninact=0, ninacth=0;

	QString s;
    activitiesListWidget->clear();
	visibleActivitiesList.clear();
    listItemToActId_map.clear();
	
	int k=0;
    int il=0;
    int iv=0;
    int maxL=0;
    for(int i=0; i<TContext::get()->instance.activitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activitiesList[i];

		if(this->filterOk(act)){
            s="";

            s+=act->getDescription();

			visibleActivitiesList.append(act);

            if (s.length() > maxL) {
                maxL = s.length();
            }

			activitiesListWidget->addItem(s);
            listItemToActId_map.insert(il++, iv++);

			k++;
			if(defs::USE_GUI_COLORS && !act->active)
				activitiesListWidget->item(k-1)->setBackground(activitiesListWidget->palette().alternateBase());
			
			if(act->id==act->activityGroupId || act->activityGroupId==0)
				nacts++;
			nsubacts++;
			
			nh+=act->duration;
			
			if(!act->active){
				ninact++;
				ninacth+=act->duration;
			}

            if(act->activityGroupId==0 || (i<TContext::get()->instance.activitiesList.size()-1 && act->activityGroupId!=TContext::get()->instance.activitiesList[i + 1]->activityGroupId)){
                activitiesListWidget->addItem("");
                listItemToActId_map.insert(il++, -1);
            }
		}
	}

    for (auto itemIndex: listItemToActId_map.keys()){
        if (listItemToActId_map.value(itemIndex)!=-1) {
            continue;
        }

        QString auxs;
        activitiesListWidget->item(itemIndex)->setText(auxs.fill('-', maxL));
    }
	
	assert(nsubacts-ninact>=0);
	assert(nh-ninacth>=0);

	activeTextLabel->setText(tr("No: %1 / %2", "No means number, %1 is the number of active activities, %2 is the number of total activities."
		" Please leave spaces between fields, so that they are better visible").arg(nsubacts-ninact).arg(nsubacts));
	totalTextLabel->setText(tr("Dur: %1 / %2", "Dur means duration, %1 is the duration of active activities, %2 is the duration of total activities."
		" Please leave spaces between fields, so that they are better visible").arg(nh-ninacth).arg(nh));

    connect(activitiesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(activityChanged()));
	
	if(activitiesListWidget->count()>0)
		activitiesListWidget->setCurrentRow(0);
	else
		activityTextEdit->setPlainText(QString(""));
}

void ActivitiesForm::addActivity()
{
    int nInitialActs=TContext::get()->instance.activitiesList.count();

	QString stn=studentsComboBox->currentText();
	QString sn=subjectsComboBox->currentText();
	QString atn=activityTagsComboBox->currentText();
	
    AddActivityForm addActivityForm(this, "", stn, sn, atn);
	setParentAndOtherThings(&addActivityForm, this);
	addActivityForm.exec();

    if(TContext::get()->instance.activitiesList.count()!=nInitialActs){
        assert(TContext::get()->instance.activitiesList.count()>nInitialActs);

		//rebuild the activities list box
		filterChanged();
	
		int ind=activitiesListWidget->count()-1;
		if(ind>=0)
			activitiesListWidget->setCurrentRow(ind);
	}
}

void ActivitiesForm::modifyActivity()
{
    int ind=listItemToActId_map.value(activitiesListWidget->currentRow());

	if(ind<0){
        activityTextEdit->setPlainText("");
//		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected activity"));
		return;
	}
	
	assert(ind<visibleActivitiesList.count());
	
	int valv=activitiesListWidget->verticalScrollBar()->value();
	int valh=activitiesListWidget->horizontalScrollBar()->value();

	Activity* act=visibleActivitiesList[ind];
	assert(act!=nullptr);
	
    QString teachers;//TODO: =act->teacherName;
	bool diffTeachers=false;
	
	QString subject=act->subjectName;
	bool diffSubject=false;
	
	QStringList activityTags=act->activityTagsNames;
	bool diffActivityTags=false;
	
    QStringList students=act->studentSetsNames;
	bool diffStudents=false;
	
	int nTotalStudents=act->nTotalStudents;
	bool diffNTotalStudents=false;
	
	bool computeNTotalStudents=act->computeNTotalStudents;
	bool diffComputeNTotalStudents=false;
	
	if(act->isSplit()){
		int nSplit=0;
        for(int i=0; i<TContext::get()->instance.activitiesList.size(); i++){
            Activity* act2=TContext::get()->instance.activitiesList[i];
			if(act2->activityGroupId==act->activityGroupId){
				nSplit++;
				
				if(subject!=act2->subjectName)
					diffSubject=true;
				if(activityTags!=act2->activityTagsNames)
					diffActivityTags=true;
                if(students!=act2->studentSetsNames)
					diffStudents=true;
				if( /* !computeNTotalStudents && !act2->computeNTotalStudents && */ nTotalStudents!=act2->nTotalStudents )
					diffNTotalStudents=true;
				if(computeNTotalStudents!=act2->computeNTotalStudents)
					diffComputeNTotalStudents=true;
			}
			if(nSplit>MAX_SPLIT_OF_AN_ACTIVITY){
				QMessageBox::warning(this, tr("m-FET information"),
				 tr("Cannot modify this large activity, because it contains more than %1 activities.")
				 .arg(MAX_SPLIT_OF_AN_ACTIVITY));
				return;
			}
		}
		
		if(diffTeachers || diffSubject || diffActivityTags || diffStudents || diffNTotalStudents || diffComputeNTotalStudents){
			QStringList s;
			if(diffTeachers)
				s.append(tr("different teachers"));
			if(diffSubject)
				s.append(tr("different subject"));
			if(diffActivityTags)
				s.append(tr("different activity tags"));
			if(diffStudents)
				s.append(tr("different students"));
			if(diffComputeNTotalStudents)
				s.append(tr("different boolean variable 'must compute n total students'"));
			if(diffNTotalStudents)
				s.append(tr("different number of students"));
				
			QString s2;
			s2+=tr("The current split activity has subactivities which were individually modified. It is recommended to abort now"
			 " and modify individual subactivities from the corresponding menu. Otherwise you will modify the fields for all the subactivities"
			 " from this larger split activity.");
			s2+="\n\n";
			s2+=tr("The fields which are different are: %1").arg(s.join(", "));
				
			int t=QMessageBox::warning(this, tr("m-FET warning"), s2, tr("Abort"), tr("Continue"), QString(), 1, 0);
			
			if(t==0)
				return;
		}
	}
	
	ModifyActivityForm modifyActivityForm(this, act->id, act->activityGroupId);
	int t;
	setParentAndOtherThings(&modifyActivityForm, this);
	t=modifyActivityForm.exec();
	
	if(t==QDialog::Accepted){
		filterChanged();
	
		activitiesListWidget->verticalScrollBar()->setValue(valv);
		activitiesListWidget->horizontalScrollBar()->setValue(valh);

		if(ind >= activitiesListWidget->count())
			ind = activitiesListWidget->count()-1;
		if(ind>=0)
			activitiesListWidget->setCurrentRow(ind);
	}
	else{
		assert(t==QDialog::Rejected);
	}
}

void ActivitiesForm::removeActivity()
{
    int ind=listItemToActId_map.value(activitiesListWidget->currentRow());

	if(ind<0){
        activityTextEdit->setPlainText("");
		return;
	}
	
	assert(ind<visibleActivitiesList.count());

	int valv=activitiesListWidget->verticalScrollBar()->value();
	int valh=activitiesListWidget->horizontalScrollBar()->value();

	Activity* act=visibleActivitiesList[ind];
	assert(act!=nullptr);

    QString s = tr("Remove activity?") + "\n";
	if(act->isSplit())
        s+=tr("The related activities from the same larger split activity will also be removed") + "\n\n";
    s+=act->getDetailedDescription() + "\n";

    auto func = [this,&act](){
        TContext::get()->instance.removeActivity(act->id, act->activityGroupId);
//        PlanningChanged::increasePlanningCommunicationSpinBox();
        this->filterChanged();
    };

    InterfaceUtils::showConfirmationBox(this, func, s);

	activitiesListWidget->verticalScrollBar()->setValue(valv);
	activitiesListWidget->horizontalScrollBar()->setValue(valh);

	if(ind >= activitiesListWidget->count())
		ind = activitiesListWidget->count()-1;
	if(ind>=0)
		activitiesListWidget->setCurrentRow(ind);
}

void ActivitiesForm::activityChanged()
{
    int index=listItemToActId_map.value(activitiesListWidget->currentRow());

    if (index<0 || TContext::get()->instance.activitiesList.empty()) {
        activityTextEdit->setPlainText("");
        return;
    }
	
	if(index<0){
		activityTextEdit->setPlainText(QString(""));
		return;
	}
	if(index>=visibleActivitiesList.count()){
		activityTextEdit->setPlainText(tr("Invalid activity"));
		return;
	}

	QString s;
	Activity* act=visibleActivitiesList[index];

	assert(act!=nullptr);
    s=act->getDetailedDescriptionWithConstraints(TContext::get()->instance);
	activityTextEdit->setPlainText(s);
}

void ActivitiesForm::help()
{
	QString s;
	
	s+=tr("Useful instructions/tips:");
	s+="\n\n";
	
	s+=tr("Above the (sub)activities list, we have 2 labels, containing 4 numbers. The first label contains text: No: a / b. The first number a is the"
		" number of active (sub)activities (we number each individual subactivity as 1), while the second number b is the number of total (sub)activities."
		" The second label contains text: Dur: c / d. The third number c is the duration of active (sub)activities, in periods"
		" (or m-FET hours), while the fourth number d is the duration of total (sub)activities, in periods (or m-FET hours)."
		" So, No means number and Dur means duration.");
	s+="\n\n";
	s+=tr("Example: No: 100 / 102, Dur: 114 / 117. They represent: 100 - the number of active (sub)activities,"
		" then 102 - the number of total (sub)activities,"
		" 114 - the duration of active activities (in periods or m-FET hours) and 117 - the duration of total activities"
		" (in periods or m-FET hours). In this example we have 2 inactive activities with their combined duration being 3 periods.");
	
	s+="\n\n";
	s+=tr("Explanation of the short description of an activity: first comes the id."
		" If the activity is inactive, an X follows. Then the duration. Then, if the activity is split, a slash and the total duration."
		" Then teachers, subject, activity tag (if it is not void) and students. Then the number of students (if specified).");
	s+="\n\n";
	s+=tr("The activities which are inactive:", "This is the help for activities which are inactive, after this field there come explanations for how inactive activities are displayed.");
	s+="\n";
	s+=" -";
	s+=tr("have an X mark after the id.", "It refers to inactive activities, which have this mark after the id.");
	s+="\n";
	s+=" -";
	s+=tr("if you use colors in interface (see Settings/Interface menu), they will appear with different background color.", "It refers to inactive activities");
	s+="\n\n";
	s+=tr("To modify an activity, you can also double click it.");
	s+="\n\n";
	s+=tr("Show related: if you select this, there will be listed activities for groups and subgroups contained also in the current set (if the current set"
		" is a year or a group) and also higher ranked year or group (if the current set is a group or a subgroup).");
	
	MessagesManager::information(this, tr("m-FET Help"), s);
}

void ActivitiesForm::activityComments()
{
    int ind=listItemToActId_map.value(activitiesListWidget->currentRow());

	if(ind<0){
        activityTextEdit->setPlainText("");
//		QMessageBox::information(this, tr("m-FET information"), tr("Invalid selected activity"));
		return;
	}
	
	assert(ind<visibleActivitiesList.count());

	Activity* act=visibleActivitiesList[ind];
	assert(act!=nullptr);

	QDialog getCommentsDialog(this);
	
	getCommentsDialog.setWindowTitle(tr("Activity comments"));
	
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
	commentsPT->setPlainText(act->comments);
	commentsPT->selectAll();
	commentsPT->setFocus();
	
	vl->addWidget(commentsPT);
	vl->addLayout(hl);
	
	getCommentsDialog.setLayout(vl);
	
	const QString settingsName=QString("ActivityCommentsDialog");
	
	getCommentsDialog.resize(500, 320);
	centerWidgetOnScreen(&getCommentsDialog);
	restoreFETDialogGeometry(&getCommentsDialog, settingsName);
	
	int t=getCommentsDialog.exec();
	saveFETDialogGeometry(&getCommentsDialog, settingsName);
	
	if(t==QDialog::Accepted){
		act->comments=commentsPT->toPlainText();
	
        TContext::get()->instance.internalStructureComputed=false;
        setRulesModifiedAndOtherThings(&TContext::get()->instance);

        activitiesListWidget->currentItem()->setText(act->getDescription());
		activityChanged();
	}
}

void ActivitiesForm::on_clearPushButton_clicked()
{
    QString s = tr("Remove all activities?");

    auto func = [this](){
        TContext::get()->instance.removeActivities();
//        PlanningChanged::increasePlanningCommunicationSpinBox();
        this->filterChanged();
    };

    InterfaceUtils::showConfirmationBox(this, func, s);
}

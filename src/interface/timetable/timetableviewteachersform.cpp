
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include <QtGlobal>

#include "tablewidgetupdatebug.h"

#include "textmessages.h"

#include "centerwidgetonscreen.h"


#include "mainform.h"
#include "timetableviewteachersform.h"
#include "defs.h"
#include "tcontext.h"
#include "solution.h"

#include "m-fet.h"

#include "matrix.h"

#include "lockunlock.h"

#include <QMessageBox>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

#include <QAbstractItemView>

#include <QListWidget>

#include <QList>

#include <QCoreApplication>
#include <QApplication>

#include <QString>
#include <QStringList>

#include <QSplitter>
#include <QSettings>
#include <QObject>
#include <QMetaObject>

//begin by Marco Vassura
#include <QBrush>
#include <QColor>
//end by Marco Vassura




extern QSet <int> idsOfLockedTime;		//care about locked activities in view forms
extern QSet <int> idsOfLockedSpace;		//care about locked activities in view forms
extern QSet <int> idsOfPermanentlyLockedTime;	//care about locked activities in view forms
extern QSet <int> idsOfPermanentlyLockedSpace;	//care about locked activities in view forms

extern CommunicationSpinBox communicationSpinBox;	//small hint to sync the forms

TimetableViewTeachersForm::TimetableViewTeachersForm(QWidget* parent, Solution &solution): QDialog(parent), solution(solution)
{
	setupUi(this);

    this->solution.computeTeachersTimetable();
	
	closePushButton->setDefault(true);
	
	detailsTextEdit->setReadOnly(true);

    //columnResizeModeInitialized=false;

    //verticalSplitter->setStretchFactor(0, 1);	//unneeded, because both have the same value
    //verticalSplitter->setStretchFactor(1, 1);	//unneeded, because both have the same value
    horizontalSplitter->setStretchFactor(0, 3);
	horizontalSplitter->setStretchFactor(1, 10);

	teachersTimetableTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	
	teachersListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(teachersListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(teacherChanged(const QString&)));
	connect(teachersTimetableTable, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));
	connect(lockTimePushButton, SIGNAL(clicked()), this, SLOT(lockTime()));
	connect(lockSpacePushButton, SIGNAL(clicked()), this, SLOT(lockSpace()));
	connect(lockTimeSpacePushButton, SIGNAL(clicked()), this, SLOT(lockTimeSpace()));

	connect(helpPushButton, SIGNAL(clicked()), this, SLOT(help()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	//restore vertical splitter state
    QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/vertical-splitter-state")))
		verticalSplitter->restoreState(settings.value(this->metaObject()->className()+QString("/vertical-splitter-state")).toByteArray());

	//restore horizontal splitter state
    //QSettings settings(defs::COMPANY, defs::PROGRAM);
	if(settings.contains(this->metaObject()->className()+QString("/horizontal-splitter-state")))
		horizontalSplitter->restoreState(settings.value(this->metaObject()->className()+QString("/horizontal-splitter-state")).toByteArray());

///////////just for testing
	QSet<int> backupLockedTime;
	QSet<int> backupPermanentlyLockedTime;
	QSet<int> backupLockedSpace;
	QSet<int> backupPermanentlyLockedSpace;
	
	backupLockedTime=idsOfLockedTime;
	backupPermanentlyLockedTime=idsOfPermanentlyLockedTime;
	backupLockedSpace=idsOfLockedSpace;
	backupPermanentlyLockedSpace=idsOfPermanentlyLockedSpace;
	
	//added by Volker Dirr
	//these 2 lines are not really needed - just to be safer
	LockUnlock::computeLockedUnlockedActivitiesTimeSpace();
	
	assert(backupLockedTime==idsOfLockedTime);
	assert(backupPermanentlyLockedTime==idsOfPermanentlyLockedTime);
	assert(backupLockedSpace==idsOfLockedSpace);
	assert(backupPermanentlyLockedSpace==idsOfPermanentlyLockedSpace);
///////////

	LockUnlock::increaseCommunicationSpinBox();
	
    teachersTimetableTable->setRowCount(TContext::get()->instance.nHoursPerDay);
    teachersTimetableTable->setColumnCount(TContext::get()->instance.nDaysPerWeek);
    for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
        QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.daysOfTheWeek[j]);
		teachersTimetableTable->setHorizontalHeaderItem(j, item);
	}
    for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
        QTableWidgetItem* item=new QTableWidgetItem(TContext::get()->instance.hoursOfTheDay[i]);
		teachersTimetableTable->setVerticalHeaderItem(i, item);
	}

    for(int j=0; j<TContext::get()->instance.nHoursPerDay; j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek; k++){
			QTableWidgetItem* item= new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

			teachersTimetableTable->setItem(j, k, item);
			
			//if(j==0 && k==0)
				//teachersTimetableTable->setCurrentItem(item);
		}
	}
	
	//resize columns
	//if(!columnResizeModeInitialized){
	teachersTimetableTable->horizontalHeader()->setMinimumSectionSize(teachersTimetableTable->horizontalHeader()->defaultSectionSize());
	//	columnResizeModeInitialized=true;
#if QT_VERSION >= 0x050000
	teachersTimetableTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
	teachersTimetableTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
	//}
	////////////////
	
	teachersListWidget->clear();

//	if(Timetable::getInstance()->instance.nInternalTeachers!=Timetable::getInstance()->instance.teachersList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some teachers. Please regenerate the timetable and then view it"));
//	}
//	else{
        for(int i=0; i<TContext::get()->instance.teachersList.size(); i++)
            teachersListWidget->addItem(TContext::get()->instance.teachersList.at(i)->name);
//	}

	if(teachersListWidget->count()>0)
		teachersListWidget->setCurrentRow(0);

	//added by Volker Dirr
	connect(&communicationSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTeachersTimetableTable()));
}

TimetableViewTeachersForm::~TimetableViewTeachersForm()
{
	saveFETDialogGeometry(this);

	//save vertical splitter state
    QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/vertical-splitter-state"), verticalSplitter->saveState());

	//save horizontal splitter state
    //QSettings settings(defs::COMPANY, defs::PROGRAM);
	settings.setValue(this->metaObject()->className()+QString("/horizontal-splitter-state"), horizontalSplitter->saveState());
}

void TimetableViewTeachersForm::resizeRowsAfterShow()
{
	teachersTimetableTable->resizeRowsToContents();
//	tableWidgetUpdateBug(teachersTimetableTable);
}

void TimetableViewTeachersForm::teacherChanged(const QString &teacherName)
{
    if(TContext::get()->solutions.empty()){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view teachers timetable dialog - please generate a new timetable"));
		return;
	}

	if(teacherName==QString())
		return;

    int teacherId=TContext::get()->instance.searchTeacher(teacherName);
	if(teacherId<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid teacher - please close this dialog and open a new view teachers dialog"));
		return;
	}

	updateTeachersTimetableTable();
}

void TimetableViewTeachersForm::updateTeachersTimetableTable(){
    if(TContext::get()->solutions.empty()){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view teachers timetable dialog - please generate a new timetable "
		"or close the timetable view teachers dialog"));
		return;
	}

//	if(Timetable::getInstance()->rules.nInternalRooms!=Timetable::getInstance()->rules.roomsList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some rooms. Please regenerate the timetable and then view it"));
//		return;
//	}

	QString s;
	QString teachername;

	if(teachersListWidget->currentRow()<0 || teachersListWidget->currentRow()>=teachersListWidget->count())
		return;

	teachername = teachersListWidget->currentItem()->text();
	
    int teacher=TContext::get()->instance.searchTeacher(teachername);
	if(teacher<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("You have an old timetable view teachers dialog opened - please close it"));
		return;
	}

	s = teachername;
	teacherNameTextLabel->setText(s);

    assert(TContext::get()->instance.initialized);
	
    for(int j=0; j<TContext::get()->instance.nHoursPerDay && j<teachersTimetableTable->rowCount(); j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek && k<teachersTimetableTable->columnCount(); k++){
			//begin by Marco Vassura
			// add colors (start)
			//if(defs::USE_GUI_COLORS) {
				teachersTimetableTable->item(j, k)->setBackground(teachersTimetableTable->palette().color(QPalette::Base));
				teachersTimetableTable->item(j, k)->setForeground(teachersTimetableTable->palette().color(QPalette::Text));
			//}
			// add colors (end)
			//end by Marco Vassura
			s = "";
            int ai=solution.teachers_timetable_weekly[teacher][k][j]; //activity index
            //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
			if(ai!=UNALLOCATED_ACTIVITY){
                Activity* act=TContext::get()->instance.activeActivitiesList[ai];
				assert(act!=nullptr);
                //TODO:
//                if(act->teacherName.size()==1){
//					//Don't do the assert below, because it crashes if you change the teacher's name and view the teachers' timetable,
//					//without generating again (as reported by Yush Yuen).
//					//assert(act->teachersNames.at(0)==teachername);
//				}
//				else{
//                    assert(act->teacherName.size()>=2);
//					//Don't do the assert below, because it crashes if you change the teacher's name and view the teachers' timetable,
//					//without generating again (as reported by Yush Yuen).
//					//assert(act->teachersNames.contains(teachername));
//                    s+=act->teacherName;
//					s+="\n";
//				}
				
				if(defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS){
					QString ats=act->activityTagsNames.join(", ");
					s += act->subjectName+" "+ats;
				}
				else{
					s += act->subjectName;
				}
				
				//students
                if(act->studentSetsNames.size()>0){
					s+="\n";
                    s+=act->studentSetsNames.join(", ");
				}
				
				//added by Volker Dirr (start)
				QString descr="";
				QString t="";
				if(idsOfPermanentlyLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "PLT", "Abbreviation for permanently locked time. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
					t=", ";
				}
				else if(idsOfLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "LT", "Abbreviation for locked time. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
					t=", ";
				}
				if(idsOfPermanentlyLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "PLS", "Abbreviation for permanently locked space. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
				}
				else if(idsOfLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "LS", "Abbreviation for locked space. There are 4 string: permanently locked time, permanently locked space, "
						"locked time, locked space. Make sure their abbreviations contain different letters and are visually different, so user can easily differentiate between them."
						" These abbreviations may appear also in other places, please use the same abbreviations.");
				}
				if(descr!=""){
					descr.prepend("\n(");
					descr.append(")");
				}
				s+=descr;
				//added by Volker Dirr (end)
				
				//begin by Marco Vassura
				// add colors (start)
                if(defs::USE_GUI_COLORS /*&& act->studentsNames.size()>0*/){
                    QBrush bg(stringToColor(act->subjectName+" "+act->studentSetsNames.join(", ")));
					teachersTimetableTable->item(j, k)->setBackground(bg);
					double brightness = bg.color().redF()*0.299 + bg.color().greenF()*0.587 + bg.color().blueF()*0.114;
					if (brightness<0.5)
						teachersTimetableTable->item(j, k)->setForeground(QBrush(Qt::white));
					else
						teachersTimetableTable->item(j, k)->setForeground(QBrush(Qt::black));
				}
				// add colors (end)
				//end by Marco Vassura
			}
			else{
                if(TContext::get()->instance.teacherNotAvailableDayHour[teacher][k][j] && defs::PRINT_NOT_AVAILABLE_TIME_SLOTS)
					s+="-x-";
                else if(TContext::get()->instance.breakDayHour[k][j] && defs::PRINT_BREAK_TIME_SLOTS)
					s+="-X-";
			}
			teachersTimetableTable->item(j, k)->setText(s);
		}
	}
    //	for(int i=0; i<Timetable::getInstance()->rules.nHoursPerDay; i++) //added in version 3_9_16, on 16 Oct. 2004
	//		teachersTimetableTable->adjustRow(i);

	teachersTimetableTable->resizeRowsToContents();
	
	tableWidgetUpdateBug(teachersTimetableTable);
	
	detailActivity(teachersTimetableTable->currentItem());
}

void TimetableViewTeachersForm::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);

	teachersTimetableTable->resizeRowsToContents();
}

//begin by Marco Vassura
QColor TimetableViewTeachersForm::stringToColor(QString s)
{
	// CRC-24 Based on RFC 2440 Section 6.1
	unsigned long crc = 0xB704CEL;
	int i;
	QChar *data = s.data();
	while (!data->isNull()) {
		crc ^= (data->unicode() & 0xFF) << 16;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x1000000)
				crc ^= 0x1864CFBL;
		}
		data++;
	}
	return QColor::fromRgb((int)(crc>>16), (int)((crc>>8) & 0xFF), (int)(crc & 0xFF));
}
//end by Marco Vassura

void TimetableViewTeachersForm::currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	Q_UNUSED(previous);
	
	detailActivity(current);
}

void TimetableViewTeachersForm::detailActivity(QTableWidgetItem* item){
	if(item==nullptr){
		detailsTextEdit->setPlainText(QString(""));
		return;
	}

    if(item->row()>=TContext::get()->instance.nHoursPerDay || item->column()>=TContext::get()->instance.nDaysPerWeek){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view teachers timetable dialog - please generate a new timetable "
		"or close the timetable view teachers dialog"));
		return;
	}

    if(TContext::get()->solutions.empty()){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view teachers timetable dialog - please generate a new timetable"));
		return;
	}

//	if(Timetable::getInstance()->rules.nInternalRooms!=Timetable::getInstance()->rules.roomsList.count()){
//		QMessageBox::warning(this, tr("m-FET warning"), tr("Cannot display the timetable, because you added or removed some rooms. Please regenerate the timetable and then view it"));
//		return;
//	}

	QString s;
	QString teachername;

	if(teachersListWidget->currentRow()<0 || teachersListWidget->currentRow()>=teachersListWidget->count())
		return;

	teachername = teachersListWidget->currentItem()->text();

	s = teachername;

	teacherNameTextLabel->setText(s);

    int teacher=TContext::get()->instance.searchTeacher(teachername);
	if(teacher<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("The teacher is invalid - please close this dialog and open a new view teachers timetable"));
		return;
	}
	else{
		int j=item->row();
		int k=item->column();
		s = "";
		if(j>=0 && k>=0){
            int ai=solution.teachers_timetable_weekly[teacher][k][j]; //activity index
            //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
			if(ai!=UNALLOCATED_ACTIVITY){
                Activity* act=TContext::get()->instance.activeActivitiesList[ai];
				assert(act!=nullptr);
                //s += act->getDetailedDescriptionWithConstraints(Timetable::getInstance()->rules);
				s += act->getDetailedDescription();

//				//int r=rooms_timetable_weekly[teacher][k][j];
//				int r=solution.rooms[ai];
//				if(r!=UNALLOCATED_SPACE && r!=UNSPECIFIED_ROOM){
//					s+="\n";
//					s+=tr("Room: %1").arg(Timetable::getInstance()->rules.internalRoomsList[r]->name);
//				}
				//added by Volker Dirr (start)
				QString descr="";
				QString t="";
				if(idsOfPermanentlyLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "permanently locked time", "refers to activity");
					t=", ";
				}
				else if(idsOfLockedTime.contains(act->id)){
					descr+=QCoreApplication::translate("TimetableViewForm", "locked time", "refers to activity");
					t=", ";
				}
				if(idsOfPermanentlyLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "permanently locked space", "refers to activity");
				}
				else if(idsOfLockedSpace.contains(act->id)){
					descr+=t+QCoreApplication::translate("TimetableViewForm", "locked space", "refers to activity");
				}
				if(descr!=""){
					descr.prepend("\n(");
					descr.append(")");
				}
				s+=descr;
				//added by Volker Dirr (end)
			}
			else{
                if(TContext::get()->instance.teacherNotAvailableDayHour[teacher][k][j]){
					s+=tr("Teacher is not available 100% in this slot");
					s+="\n";
				}
                if(TContext::get()->instance.breakDayHour[k][j]){
					s+=tr("Break with weight 100% in this slot");
					s+="\n";
				}
			}
		}
		detailsTextEdit->setPlainText(s);
	}
}

void TimetableViewTeachersForm::lockTime()
{
	this->lock(true, false);
}
	
void TimetableViewTeachersForm::lockSpace()
{
	this->lock(false, true);
}

void TimetableViewTeachersForm::lockTimeSpace()
{
	this->lock(true, true);
}
			
void TimetableViewTeachersForm::lock(bool lockTime, bool lockSpace)
{
    Q_UNUSED(lockSpace);

    //cout<<"teachers begin, isc="<<Timetable::getInstance()->rules.internalStructureComputed<<endl;

    if(TContext::get()->simulation_running){
		QMessageBox::information(this, tr("m-FET information"),
			tr("Allocation in course.\nPlease stop simulation before this."));
		return;
	}

    if(TContext::get()->solutions.empty()){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Timetable not available in view teachers timetable dialog - please generate a new timetable"));
		return;
	}

	//find teacher index
	QString teachername;

	if(teachersListWidget->currentRow()<0 || teachersListWidget->currentRow()>=teachersListWidget->count()){
		QMessageBox::information(this, tr("m-FET information"), tr("Please select a teacher"));
		return;
	}

	teachername = teachersListWidget->currentItem()->text();
    int i=TContext::get()->instance.searchTeacher(teachername);
	
	if(i<0){
		QMessageBox::warning(this, tr("m-FET warning"), tr("Invalid teacher - please close this dialog and open a new view teachers dialog"));
		return;
	}

    Solution &tc=solution;
	
	bool report=false; //the messages are annoying
	
	int addedT=0, unlockedT=0;
	int addedS=0, unlockedS=0;

	//lock selected activities
	QSet <int> careAboutIndex;		//added by Volker Dirr. Needed, because of activities with duration > 1
	careAboutIndex.clear();
    for(int j=0; j<TContext::get()->instance.nHoursPerDay && j<teachersTimetableTable->rowCount(); j++){
        for(int k=0; k<TContext::get()->instance.nDaysPerWeek && k<teachersTimetableTable->columnCount(); k++){
			if(teachersTimetableTable->item(j, k)->isSelected()){
                int ai=solution.teachers_timetable_weekly[i][k][j];
				if(ai!=UNALLOCATED_ACTIVITY && !careAboutIndex.contains(ai)){	//modified, because of activities with duration > 1
					careAboutIndex.insert(ai);					//Needed, because of activities with duration > 1

                    int hour=tc.getHourForActivityIndex(ai);
                    int day=tc.getDayForActivityIndex(ai);
                    //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];

					if(lockTime){
                        ConstraintActivityPreferredStartingTime* ctr=new ConstraintActivityPreferredStartingTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100.0, act->id, day, hour, false);
                        bool t=TContext::get()->instance.addTimeConstraint(ctr);
						QString s;
						if(t){ //modified by Volker Dirr, so you can also unlock (start)
							addedT++;
							idsOfLockedTime.insert(act->id);
							s+=tr("Added the following constraint:")+"\n"+ctr->getDetailedDescription();
						}
						else{
							delete ctr;
						
							QList<TimeConstraint*> tmptc;
							tmptc.clear();
							int count=0;
                            foreach(ConstraintActivityPreferredStartingTime* c, TContext::get()->instance.apstHash.value(act->id, QSet<ConstraintActivityPreferredStartingTime*>())){
								assert(c->activityId==act->id);
								if(c->activityId==act->id && c->weightPercentage==100.0 && c->active && c->day>=0 && c->hour>=0){
									count++;
									if(c->permanentlyLocked){
										if(idsOfLockedTime.contains(c->activityId) || !idsOfPermanentlyLockedTime.contains(c->activityId)){
											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
											 +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred starting time with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
											 +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
											 +"\n\n"+tr("Please report possible bug")
											 );
										}
										else{
                                            s+=tr("Constraint %1 will not be removed, because it is permanently locked. If you want to unlock it you must go to the constraints menu.").arg("\n"+c->getDetailedDescription()+"\n");
										}
									}
									else{
										if(!idsOfLockedTime.contains(c->activityId) || idsOfPermanentlyLockedTime.contains(c->activityId)){
											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
											 +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred starting time with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
											 +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
											 +"\n\n"+tr("Please report possible bug")
											 );
										}
										else{
											tmptc.append((TimeConstraint*)c);
										}
									}
								}
							}
							if(count!=1)
								QMessageBox::warning(this, tr("m-FET warning"), tr("You may have a problem, because m-FET expected to delete 1 constraint, but will delete %1 constraints").arg(tmptc.size()));

							foreach(TimeConstraint* deltc, tmptc){
                                s+=tr("The following constraint will be deleted:")+"\n"+deltc->getDetailedDescription()+"\n";
                                TContext::get()->instance.removeTimeConstraint(deltc);
								idsOfLockedTime.remove(act->id);
								unlockedT++;
								//delete deltc; - done by rules.removeTim...
							}
							tmptc.clear();
                            //Timetable::getInstance()->rules.internalStructureComputed=false;
						}  //modified by Volker Dirr, so you can also unlock (end)
						
						if(report){
							int k;
							k=QMessageBox::information(this, tr("m-FET information"), s,
							 tr("Skip information"), tr("See next"), QString(), 1, 0 );

		 					if(k==0)
								report=false;
						}
					}
					
//					int ri=tc.rooms[ai];
//					if(ri!=UNALLOCATED_SPACE && ri!=UNSPECIFIED_ROOM && lockSpace){
//						ConstraintActivityPreferredRoom* ctr=new ConstraintActivityPreferredRoom(100, act->id, (Timetable::getInstance()->rules.internalRoomsList[ri])->name, false);
//						bool t=Timetable::getInstance()->rules.addSpaceConstraint(ctr);

//						QString s;
						
//						if(t){ //modified by Volker Dirr, so you can also unlock (start)
//							addedS++;
//							idsOfLockedSpace.insert(act->id);
//							s+=tr("Added the following constraint:")+"\n"+ctr->getDetailedDescription(Timetable::getInstance()->rules);
//						}
//						else{
//							delete ctr;
						
//							QList<SpaceConstraint*> tmpsc;
//							tmpsc.clear();
//							int count=0;

//							foreach(ConstraintActivityPreferredRoom* c, Timetable::getInstance()->rules.aprHash.value(act->id, QSet<ConstraintActivityPreferredRoom*>())){
//								assert(c->activityId==act->id);

//								if(c->activityId==act->id && c->weightPercentage==100.0 && c->active){
//									count++;
//									if(c->permanentlyLocked){
//										if(idsOfLockedSpace.contains(c->activityId) || !idsOfPermanentlyLockedSpace.contains(c->activityId)){
//											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
//											 +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred room with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
//											 +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
//											 +"\n\n"+tr("Please report possible bug")
//											 );
//										}
//										else{
//											s+=tr("Constraint %1 will not be removed, because it is permanently locked. If you want to unlock it you must go to the constraints menu.").arg("\n"+c->getDetailedDescription(Timetable::getInstance()->rules)+"\n");
//										}
//									}
//									else{
//										if(!idsOfLockedSpace.contains(c->activityId) || idsOfPermanentlyLockedSpace.contains(c->activityId)){
//											QMessageBox::warning(this, tr("m-FET warning"), tr("Small problem detected")
//											 +"\n\n"+tr("A possible problem might be that you have 2 or more constraints of type activity preferred room with weight 100% related to activity id %1, please leave only one of them").arg(act->id)
//											 +"\n\n"+tr("A possible problem might be synchronization - so maybe try to close the timetable view dialog and open it again")
//											 +"\n\n"+tr("Please report possible bug")
//											 );
//										}
//										else{
//											tmpsc.append((SpaceConstraint*)c);
//										}
//									}
//								}
//							}
//							if(count!=1)
//								QMessageBox::warning(this, tr("m-FET warning"), tr("You may have a problem, because m-FET expected to delete 1 constraint, but will delete %1 constraints").arg(tmpsc.size()));

//							foreach(SpaceConstraint* delsc, tmpsc){
//								s+=tr("The following constraint will be deleted:")+"\n"+delsc->getDetailedDescription(Timetable::getInstance()->rules)+"\n";
//								Timetable::getInstance()->rules.removeSpaceConstraint(delsc);
//								idsOfLockedSpace.remove(act->id);
//								unlockedS++;
//								//delete delsc; done by rules.removeSpa...
//							}
//							tmpsc.clear();
//							//Timetable::getInstance()->rules.internalStructureComputed=false;
//						}  //modified by Volker Dirr, so you can also unlock (end)
						
//						if(report){
//							int k;
//							k=QMessageBox::information(this, tr("m-FET information"), s,
//							 tr("Skip information"), tr("See next"), QString(), 1, 0 );
								
//							if(k==0)
//								report=false;
//						}
//					}
				}
			}
		}
	}

	QStringList added;
	QStringList removed;
	if(addedT>0){
		if(addedT==1)
			added << QCoreApplication::translate("TimetableViewForm", "Added 1 locking time constraint.", "constraint is singular (only 1 constraint)");
		else
			added << QCoreApplication::translate("TimetableViewForm", "Added %1 locking time constraints.", "%1 is >= 2, so constraints is plural").arg(addedT);
	}
	if(addedS>0){
		if(addedS==1)
			added << QCoreApplication::translate("TimetableViewForm", "Added 1 locking space constraint.", "constraint is singular (only 1 constraint)");
		else
			added << QCoreApplication::translate("TimetableViewForm", "Added %1 locking space constraints.", "%1 is >= 2, so constraints is plural").arg(addedS);
	}
	if(unlockedT>0){
		if(unlockedT==1)
			removed << QCoreApplication::translate("TimetableViewForm", "Removed 1 locking time constraint.", "constraint is singular (only 1 constraint)");
		else
			removed << QCoreApplication::translate("TimetableViewForm", "Removed %1 locking time constraints.", "%1 is >= 2, so constraints is plural").arg(unlockedT);
	}
	if(unlockedS>0){
		if(unlockedS==1)
			removed << QCoreApplication::translate("TimetableViewForm", "Removed 1 locking space constraint.", "constraint is singular (only 1 constraint)");
		else
			removed << QCoreApplication::translate("TimetableViewForm", "Removed %1 locking space constraints.", "%1 is >= 2, so constraints is plural").arg(unlockedS);
	}
	QString ad=added.join("\n");
	QString re=removed.join("\n");
	QStringList all;
	if(!ad.isEmpty())
		all<<ad;
	if(!re.isEmpty())
		all<<re;
	QString s=all.join("\n\n");
	if(s.isEmpty())
		s=QCoreApplication::translate("TimetableViewForm", "No locking constraints added or removed.");
	QMessageBox::information(this, tr("m-FET information"), s);

////////// just for testing
	QSet<int> backupLockedTime;
	QSet<int> backupPermanentlyLockedTime;
	QSet<int> backupLockedSpace;
	QSet<int> backupPermanentlyLockedSpace;
	
	backupLockedTime=idsOfLockedTime;
	backupPermanentlyLockedTime=idsOfPermanentlyLockedTime;
	backupLockedSpace=idsOfLockedSpace;
	backupPermanentlyLockedSpace=idsOfPermanentlyLockedSpace;
	
	LockUnlock::computeLockedUnlockedActivitiesTimeSpace(); //not needed, just for testing
	
	assert(backupLockedTime==idsOfLockedTime);
	assert(backupPermanentlyLockedTime==idsOfPermanentlyLockedTime);
	assert(backupLockedSpace==idsOfLockedSpace);
	assert(backupPermanentlyLockedSpace==idsOfPermanentlyLockedSpace);
///////////

	LockUnlock::increaseCommunicationSpinBox();
	
    //cout<<"teachers end, isc="<<Timetable::getInstance()->rules.internalStructureComputed<<endl;
	//cout<<endl;
}

void TimetableViewTeachersForm::help()
{
	QString s="";
	//s+=QCoreApplication::translate("TimetableViewForm", "You can drag sections to increase/decrease them.");
	//s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "Lock/unlock: you can select one or more activities in the table and toggle lock/unlock in time, space or both.");
	s+=" ";
	s+=QCoreApplication::translate("TimetableViewForm", "There will be added or removed locking constraints for the selected activities (they can be unlocked only if they are not permanently locked).");
	s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "Locking time constraints are constraints of type activity preferred starting time. Locking space constraints are constraints of type"
		" activity preferred room. You can see these constraints in the corresponding constraints dialogs. New locking constraints are added at the end of the list of constraints.");
	s+="\n\n";
	s+=QCoreApplication::translate("TimetableViewForm", "If a cell is (permanently) locked in time or space, it contains abbreviations to show that: PLT (permanently locked time), LT (locked time), "
		"PLS (permanently locked space) or LS (locked space).", "Translate the abbreviations also. Make sure the abbreviations in your language are different between themselves "
		"and the user can differentiate easily between them. These abbreviations may appear also in other places, please use the same abbreviations.");

	MessagesManager::information(this, tr("m-FET help"), s);
}

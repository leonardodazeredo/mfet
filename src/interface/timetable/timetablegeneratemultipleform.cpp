/***************************************************************************
                          timetablegeneratemultipleform.cpp  -  description
                             -------------------
    begin                : Aug 20, 2007
    copyright            : (C) 2007 by Lalescu Liviu
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

#include "textmessages.h"

#include "generate.h"

#include "timetablegeneratemultipleform.h"
#include "timetable_defs.h"
#include "timetable.h"
#include "m-fet.h"
#include "timetableexport.h"

#include <ctime>

#include <QMessageBox>

#include <QMutex>

#include <QScrollBar>

#include <QDir>

#include <QApplication>

#include "stringutils.h"

#include "centerwidgetonscreen.h"


static GenerateMultipleThread generateMultipleThread;

#include <QSemaphore>

static QSemaphore semaphoreTimetableFinished; 

static QSemaphore semaphoreTimetableStarted;

Generate genMulti;

static int nTimetables;
static int timeLimit;

extern int maxActivitiesPlaced;

static time_t start_time;

static time_t initial_time;

int savedPermutation[MAX_ACTIVITIES];

void GenerateMultipleThread::run()
{
//    gt.simulation_abort=false;
	
//	time(&initial_time); // leo

	for(int i=0; i<nTimetables; i++){
//		time(&start_time); // leo
	
		bool impossible;
		bool timeExceeded;
		
//        for(int qq=0; qq<gt.instance.activeActivitiesList.size(); qq++)
//			permutation[qq]=savedPermutation[qq];
			
		emit(timetableStarted(i+1));
		semaphoreTimetableStarted.acquire();

        genMulti._generate(timeLimit, impossible, timeExceeded); //true means threaded

		QString s;
		
        bool ok;

//        CRITICAL(gt.myMutex,
//            if(gt.simulation_abort){
//                gt.myMutex.unlock();
//                return;
//            }
//            else if(impossible){
//                s=tr("Timetable impossible to generate");
//                s+=QString(".");
//                ok=false;
//            }
//            else if(timeExceeded){
//                s=tr("Time exceeded for current timetable");

//                ok=false;
//            }
//            else{
//                ok=true;

//                time_t finish_time;
////                time(&finish_time); // leo
//                int seconds=int(finish_time-start_time);
//                int hours=seconds/3600;
//                seconds%=3600;
//                int minutes=seconds/60;
//                seconds%=60;

//                genMulti.solutionFound.cost(true);

//                s=tr("Timetable breaks %1 soft constraints, has %2 soft conflicts total, and was generated in %3 hours, %4 minutes and %5 seconds.")
//                 .arg(genMulti.solutionFound.constraintConflictData.conflictsCostList().count())
//                 .arg(utils::strings::number(genMulti.solutionFound.constraintConflictData.costTotal()))
//                 .arg(hours)
//                 .arg(minutes)
//                 .arg(seconds);
//            }
//        )
		
		emit(timetableGenerated(i+1, s, ok));
		semaphoreTimetableFinished.acquire();
	}
	
	emit(finished());
}

TimetableGenerateMultipleForm::TimetableGenerateMultipleForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	
	currentResultsTextEdit->setReadOnly(true);
	
	startPushButton->setDefault(true);

	connect(startPushButton, SIGNAL(clicked()), this, SLOT(start()));
	connect(stopPushButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(closePressed()));
	connect(pushButton4, SIGNAL(clicked()), this, SLOT(help()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
    gt.simulation_running_multi=false;

	startPushButton->setEnabled(true);
	stopPushButton->setDisabled(true);
	closePushButton->setEnabled(true);
	minutesGroupBox->setEnabled(true);
	timetablesGroupBox->setEnabled(true);

	connect(&generateMultipleThread, SIGNAL(timetableGenerated(int, const QString&, bool)),
		this, SLOT(timetableGenerated(int, const QString&, bool)));

	connect(&generateMultipleThread, SIGNAL(timetableStarted(int)),
		this, SLOT(timetableStarted(int)));

	connect(&generateMultipleThread, SIGNAL(finished()),
		this, SLOT(finished()));

//	connect(&genMulti, SIGNAL(activityPlaced(int)),
//		this, SLOT(activityPlaced(int)));
}

TimetableGenerateMultipleForm::~TimetableGenerateMultipleForm()
{
	saveFETDialogGeometry(this);
    if(gt.simulation_running_multi)
		this->stop();
}

void TimetableGenerateMultipleForm::help()
{
	QString s2=INPUT_FILENAME_XML.right(INPUT_FILENAME_XML.length()-INPUT_FILENAME_XML.lastIndexOf(FILE_SEP)-1);
	
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	
	QString destDir=OUTPUT_DIR+FILE_SEP+"timetables"+FILE_SEP+s2+"-multi";

	QString s=tr("You can only see generated timetables on the hard disk,"
	 " in HTML and XML formats and soft conflicts in text format, or latest timetable in the Timetable/View menu."
	 " It is needed that the directory"
	 " %1 to be emptied and deleted before proceeding.").arg(QDir::toNativeSeparators(destDir))
	 +"\n\n"
	 +tr("Note that, for large data, each timetable might occupy more"
	 " megabytes of hard disk space,"
	 " so make sure you have enough space (you can check the dimension of a single timetable as a precaution).")
	 +"\n\n"
	 +tr("There are also saved the timetables in .m-fet format (data + constraints to lock the timetable), so that you can open each of them later.")
	 +"\n\n"
	 +tr("If you get impossible timetable, please enter menu Generate (single) and see the initial order of evaluation of activities,"
	 " this might help.")
	 +"\n\n"
	 +tr("You can limit the search time, by specifying the maximum number of minutes allowed to spend for each timetable (option %1).").arg("'"+tr("Limit for each timetable")+"'")
	 +" "+tr("The maximum and also the predefined value is %1 minutes, which means %2 hours, so virtually unlimited.").arg(60000).arg(1000)
	 ;
	 
	 LongTextMessageBox::largeInformation(this, tr("m-FET information"), s);
}

void TimetableGenerateMultipleForm::start(){
	nTimetables=timetablesSpinBox->value();
	assert(nTimetables>0);
	timeLimit=60*minutesSpinBox->value(); //seconds
	assert(timeLimit>0);

	QDir dir;
	QString s2=INPUT_FILENAME_XML.right(INPUT_FILENAME_XML.length()-INPUT_FILENAME_XML.lastIndexOf(FILE_SEP)-1);

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	
	QString destDir=OUTPUT_DIR+FILE_SEP+"timetables"+FILE_SEP+s2+"-multi";
	if(dir.exists(destDir)){
		QMessageBox::warning(this, tr("m-FET information"), tr("Directory %1 exists and might not be empty,"
		 " (it might contain old files). You need to manually remove all contents of this directory AND the directory itself (or rename it)"
		 " and then you can generate multiple timetables")
		 .arg(QDir::toNativeSeparators(destDir)));
		 
		return;
	}

	if(!gt.instance.internalStructureComputed){
		if(!gt.instance.computeInternalStructure(this)){
			QMessageBox::warning(this, TimetableGenerateMultipleForm::tr("m-FET warning"), TimetableGenerateMultipleForm::tr("Data is wrong. Please correct and try again"));
			return;
		}
	}

	if(!gt.instance.initialized || gt.instance.activitiesList.isEmpty()){
		QMessageBox::critical(this, TimetableGenerateMultipleForm::tr("m-FET information"),
			TimetableGenerateMultipleForm::tr("You have entered simulation with uninitialized rules or 0 activities...aborting"));
		assert(0);
		exit(1);
		return;
	}

	currentResultsTextEdit->setPlainText("");

	bool ok=genMulti.precompute(this);
	if(!ok){
		currentResultsTextEdit->setPlainText(TimetableGenerateMultipleForm::tr("Cannot optimize - please modify your data"));
		currentResultsTextEdit->update();

		QMessageBox::information(this, TimetableGenerateMultipleForm::tr("m-FET information"),
		 TimetableGenerateMultipleForm::tr("Your data cannot be processed - please modify it as instructed."));

		return;
	}

	startPushButton->setDisabled(true);
	stopPushButton->setEnabled(true);
	minutesGroupBox->setDisabled(true);
	timetablesGroupBox->setDisabled(true);
	closePushButton->setDisabled(true);

    gt.simulation_running_multi=true;

//    for(int qq=0; qq<gt.instance.activeActivitiesList.size(); qq++)
//		savedPermutation[qq]=permutation[qq];
		
    genMulti.solutionFound.makeUnallocated();

	generateMultipleThread.start();
}

void TimetableGenerateMultipleForm::timetableStarted(int timetable)
{
	TimetableExport::writeRandomSeed(this, timetable, true); //true represents 'before' state
	
	semaphoreTimetableStarted.release();
}

void TimetableGenerateMultipleForm::timetableGenerated(int timetable, const QString& description, bool ok)
{
	TimetableExport::writeRandomSeed(this, timetable, false); //false represents 'before' state

	QString s=QString("");
	s+=tr("Timetable no: %1 => %2", "%1 is the number of this timetable when generating multiple timetables, %2 is its description").arg(timetable).arg(description);
	currentResultsTextEdit->appendPlainText(s);
	
	bool begin;
	if(timetable==1)
		begin=true;
	else
		begin=false;
	TimetableExport::writeReportForMultiple(this, s, begin);

	if(ok){
		//needed to get the conflicts string
        genMulti.solutionFound.cost(true);

        gt.addSolution(genMulti.solutionFound);

		TimetableExport::writeSimulationResults(this, timetable);
	}

	semaphoreTimetableFinished.release();
}

void TimetableGenerateMultipleForm::stop()
{
    if(!gt.simulation_running_multi){
		return;
	}

    gt.simulation_running_multi=false;

//    CRITICAL(gt.myMutex,
//        gt.simulation_abort=true;
//    )

	QString s=TimetableGenerateMultipleForm::tr("Simulation interrupted!");
	s+="\n\n";

	QString s2=INPUT_FILENAME_XML.right(INPUT_FILENAME_XML.length()-INPUT_FILENAME_XML.lastIndexOf(FILE_SEP)-1);

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	
	QString destDir=OUTPUT_DIR+FILE_SEP+"timetables"+FILE_SEP+s2+"-multi";

	time_t final_time;
//	time(&final_time); // leo
	int sec=int(final_time-initial_time);
	int h=sec/3600;
	sec%=3600;
	int m=sec/60;
	sec%=60;

	s+=TimetableGenerateMultipleForm::tr("The results were saved in the directory %1").arg(QDir::toNativeSeparators(destDir));
	s+="\n\n";
	s+=tr("Total searching time: %1h %2m %3s").arg(h).arg(m).arg(sec);
	
	TimetableExport::writeReportForMultiple(this, QString("\n")+s, false);

	QMessageBox::information(this, tr("m-FET information"), s);

	startPushButton->setEnabled(true);
	stopPushButton->setDisabled(true);
	minutesGroupBox->setEnabled(true);
	timetablesGroupBox->setEnabled(true);
	closePushButton->setEnabled(true);
}

void TimetableGenerateMultipleForm::finished()
{
	simulationFinished();
}

void TimetableGenerateMultipleForm::simulationFinished()
{
    if(!gt.simulation_running_multi){
		return;
	}

    gt.simulation_running_multi=false;

	QString s2=INPUT_FILENAME_XML.right(INPUT_FILENAME_XML.length()-INPUT_FILENAME_XML.lastIndexOf(FILE_SEP)-1);

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	
	QString destDir=OUTPUT_DIR+FILE_SEP+"timetables"+FILE_SEP+s2+"-multi";
	
	time_t final_time;
//	time(&final_time); // leo
	int s=int(final_time-initial_time);
	int h=s/3600;
	s%=3600;
	int m=s/60;
	s%=60;

	QString ms=QString("");
	ms+=TimetableGenerateMultipleForm::tr("Simulation finished!");
	ms+=QString("\n\n");
	ms+=TimetableGenerateMultipleForm::tr("The results were saved in the directory %1").arg(QDir::toNativeSeparators(destDir));
	ms+=QString("\n\n");
	ms+=TimetableGenerateMultipleForm::tr("Total searching time was %1h %2m %3s").arg(h).arg(m).arg(s);
	
	TimetableExport::writeReportForMultiple(this, QString("\n")+ms, false);

#ifndef Q_WS_QWS
	QApplication::beep();
#endif
	
	//Trick so that the message box will be silent (the only sound is thus the beep above).
	QMessageBox msgBox(this);
	msgBox.setWindowTitle(TimetableGenerateMultipleForm::tr("m-FET information"));
	msgBox.setText(ms);
	msgBox.exec();
	//QMessageBox::information(this, TimetableGenerateMultipleForm::tr("m-FET information"), ms);
	
	startPushButton->setEnabled(true);
	stopPushButton->setDisabled(true);
	minutesGroupBox->setEnabled(true);
	timetablesGroupBox->setEnabled(true);
	closePushButton->setEnabled(true);
}

void TimetableGenerateMultipleForm::activityPlaced(int na)
{
	time_t finish_time;
//	time(&finish_time); // leo
	int seconds=int(finish_time-start_time);
	int hours=seconds/3600;
	seconds%=3600;
	int minutes=seconds/60;
	seconds%=60;

	QString s;
	
	textLabel->setText(tr("Current timetable: %1 out of %2 activities placed, %3h %4m %5s")
	 .arg(na)
     .arg(gt.instance.activeActivitiesList.size())
	 .arg(hours)
	 .arg(minutes)
	 .arg(seconds)+s);
}

void TimetableGenerateMultipleForm::closePressed()
{
	if(!generateMultipleThread.isRunning())
		this->close();
}

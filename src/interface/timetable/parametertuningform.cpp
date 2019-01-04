/***************************************************************************
                          timetableexp.generateform.cpp  -  description
                             -------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003 by Lalescu Liviu
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

#include "parametertuningform.h"

#include "timetable_defs.h"
#include "timetable.h"
#include "m-fet.h"
#include "experiments.h"

#include <QString>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QLocale>
#include <QMessageBox>
#include <QMutex>
#include <QDir>
#include <QApplication>
#include <QTimer>

#include "textmessages.h"
#include "centerwidgetonscreen.h"

#include "timerutils.h"
#include "interfaceutils.h"

ParameterTuningExperiment experiment;

ParameterTuningForm::ParameterTuningForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	currentResultsTextEdit->setReadOnly(true);
	
	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	startPushButton->setDefault(true);

    connect(&experiment.gen, SIGNAL(simulationFinished()), this, SLOT(simulationFinished()));
    connect(&experiment.gen, SIGNAL(iterationFinished(int, int, int, QString)), this, SLOT(iterationFinished(int, int, int, QString)));

    updateButtonsState();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    resetTimer();
}

void ParameterTuningForm::updateTimer()
{
    QString s;

    s=ParameterTuningForm::tr("Elapsed time: ");
    s+=utils::time::secondsToStr(time++);

    timeLabel->setText(s);
}

void ParameterTuningForm::resetTimer()
{
    timer->stop();

    time = 0;
    timeAvg = 0;
    crtIter = 0;

    estTimeLabel->setText("");
}

void ParameterTuningForm::updateButtonsState()
{
    startPushButton->setEnabled(!gt.simulation_running);
//    generationSettingsPushButton->setEnabled(!gt.simulation_running);
    closePushButton->setEnabled(!gt.simulation_running);

    stopPushButton->setEnabled(gt.simulation_running);
    skipStepPushButton->setEnabled(gt.simulation_running);
}

ParameterTuningForm::~ParameterTuningForm()
{
	saveFETDialogGeometry(this);
    if(gt.simulation_running)
        this->on_stopPushButton_clicked();
}

void ParameterTuningForm::closeEvent(QCloseEvent * event)
{
    if(gt.simulation_running){
        QMessageBox::warning(this, ParameterTuningForm::tr("m-FET warning"), ParameterTuningForm::tr("Simulation running!"));
        event->ignore();
    }
    else{
        event->accept();
    }
}

void ParameterTuningForm::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape){
        QDialog::keyPressEvent(e);
    }
    else {
        this->close();
    }
}

void ParameterTuningForm::simulationFinished()
{
    if(!gt.simulation_running){
		return;
	}

    gt.simulation_running=false;

    Solution& c=experiment.gen.solutionFound;

	//needed to find the conflicts strings
    c.cost(true);

    gt.addSolution(c);

//    QString s="\n********************************************************************************************\n";
//    s+=ParameterTuningForm::tr("Generation successful!");
//    s+="\n\n";
//    s+=c.getCostDetailsString(Enums::Short);
//    s+="\n\n";
//    s+=tr(" -> Number of broken constraints: %1").arg(c.constraintConflictData.conflictsDescriptionList().count());
//    s+="\n\n";
//    s+=tr("Results were saved in the directory:\n %1").arg(QDir::toNativeSeparators(OUTPUT_DIR+FILE_SEP+"timetables"+TimetableExport::getOutPutDir_single()));
//    s+="\n********************************************************************************************\n";

//    currentResultsTextEdit->appendPlainText(s);

    updateButtonsState();

    resetTimer();

#ifndef Q_WS_QWS
    QApplication::beep();
#endif
}

void ParameterTuningForm::on_skipStepPushButton_clicked()
{
    if(!gt.simulation_running){
        return;
    }

    CRITICAL(gt.myMutex,
        experiment.gen.generationStrategy->simulation_skip_step = true;
    )
}

void ParameterTuningForm::iterationFinished(int searchTime, int it, int nIterations, QString best){
    assert(gt.instance.initialized && gt.instance.internalStructureComputed);

    //write to the Qt interface
    QString s;
    s+= experiment.gen.generationStrategy->description() + "\n\n";
    s+=ParameterTuningForm::tr("%1 out of %2 iterations done").arg(it).arg(nIterations)+"\n";
    s+= "\nBest so far:\n\n" + best;

    currentResultsTextEdit->setPlainText(s);

    int p = (it*100)/nIterations;
    generationProgressBar->setValue(p);

    if (it > 0) {
        timeAvg = (double)searchTime / (double)it;
        crtIter = it;

        s=ParameterTuningForm::tr("Estimated time until done: ");
        s+=utils::time::secondsToStr((int)(timeAvg*(nIterations - it)));

        estTimeLabel->setText(s);
    }
}

void ParameterTuningForm::on_generationSettingsPushButton_clicked()
{
    if(gt.simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }
//    TimetableGenerateSettingsForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void ParameterTuningForm::on_startPushButton_clicked()
{
    if(!gt.instance.internalStructureComputed){
        if(!gt.instance.computeInternalStructure(this)){
            QMessageBox::warning(this, ParameterTuningForm::tr("m-FET warning"), ParameterTuningForm::tr("Data is wrong. Please correct and try again"));
            return;
        }
    }

    if(!gt.instance.initialized || gt.instance.activitiesList.isEmpty()){
        QMessageBox::critical(this, ParameterTuningForm::tr("m-FET information"),
            ParameterTuningForm::tr("You have entered simulation with uninitialized rules or 0 activities...aborting"));
        assert(0);exit(1);return;
    }

    currentResultsTextEdit->setPlainText(ParameterTuningForm::tr("Entering simulation....precomputing, please be patient"));

    bool ok=experiment.gen.precompute(this);

    if(!ok){
        currentResultsTextEdit->setPlainText(ParameterTuningForm::tr("Cannot generate - please modify your data"));
        currentResultsTextEdit->update();

        QMessageBox::information(this, ParameterTuningForm::tr("m-FET information"),
         ParameterTuningForm::tr("Your data cannot be processed - please modify it as instructed"));

        return;
    }

    updateButtonsState();

    GenerationStrategy* ge = gt.instance.getCurrentStrategy();
    experiment.runExperiments(ge, true);

    timer->start(900);
    timeLabel->setText("");
}

void ParameterTuningForm::on_stopPushButton_clicked()
{
    if(!gt.simulation_running){
        return;
    }

    CRITICAL(gt.myMutex,
        experiment.gen.generationStrategy->simulation_abort = true;
    )

    QString s="\n\n";
    s+=ParameterTuningForm::tr(" > Stopping simulation, please be patient...");

    if (timeAvg > 0) {
        int t = ((int)((crtIter + 1) * timeAvg)) - time;

        if (t < 0) {
            t = 0;
        }

        s+=ParameterTuningForm::tr(" Total estimated time to stop successfully: %1").arg(utils::time::secondsToStr(t));
    }

    s+=QString("\n\n");

    currentResultsTextEdit->appendPlainText(s);

    updateButtonsState();
    stopPushButton->setEnabled(false);
}

void ParameterTuningForm::on_closePushButton_clicked()
{
    if(!gt.simulation_running)
        this->close();
}

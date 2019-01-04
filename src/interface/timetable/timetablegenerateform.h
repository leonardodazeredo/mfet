/***************************************************************************
                          timetablegenerateform.h  -  description
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

#ifndef TIMETABLEGENERATEFORM_H
#define TIMETABLEGENERATEFORM_H

#include "ui_timetablegenerateform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class Generate;

class TimetableGenerateForm : public QDialog, Ui::TimetableGenerateForm_template  {
	Q_OBJECT

public:
    Generate* gen;

	TimetableGenerateForm(QWidget* parent);

	~TimetableGenerateForm();

    void closeEvent(QCloseEvent * event);
    void keyPressEvent(QKeyEvent *e);

public slots:
    void start();
    void run_tests_generate();
    void run_tests_improve();
    void stop();
    void stopHighest();

    void startImproving(int index);

    void write();
	void writeHighestStage();

    void closePressed();
    void help();

private slots:
    void iterationFinished(int searchTime, int it, int nIterations, QString best);

	void simulationFinished();

	void impossibleToSolve();

    void updateTimer();

    void resetTimer();

    void on_generationSettingsPushButton_clicked();

    void on_skipStepPushButton_clicked();

private:
    QTimer *timer;

    QTimer *timerToAbort;

    size_t time;

    double timeAvg = 0.0;
    int crtIter = 0;

    void updateButtonsState();

    void printTestResults(QVector<QVector<double>> resultadosMatrix, QVector<int> precisoes);
};

#endif

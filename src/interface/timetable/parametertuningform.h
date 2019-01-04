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

#ifndef PARAMETERTUNINGFORM_H
#define PARAMETERTUNINGFORM_H

#include "ui_parametertuningform_template.h"

class ParameterTuningForm : public QDialog, Ui::ParameterTuningForm_template  {
	Q_OBJECT

public:

    ParameterTuningForm(QWidget* parent);

    ~ParameterTuningForm();

    void closeEvent(QCloseEvent * event);
    void keyPressEvent(QKeyEvent *e);

//public slots:

private slots:
    void iterationFinished(int searchTime, int it, int nIterations, QString best);

	void simulationFinished();

    void updateTimer();

    void resetTimer();

    void on_generationSettingsPushButton_clicked();

    void on_skipStepPushButton_clicked()__attribute__((optimize(0)));

    void on_startPushButton_clicked()__attribute__((optimize(0)));

    void on_stopPushButton_clicked()__attribute__((optimize(0)));

    void on_closePushButton_clicked();

private:
    QTimer *timer;

    size_t time;

    double timeAvg = 0.0;
    int crtIter = 0;

    void updateButtonsState();
};

#endif

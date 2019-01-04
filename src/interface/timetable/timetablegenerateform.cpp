/***************************************************************************
                          timetablegenerateform.cpp  -  description
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

#include "generate.h"

#include "timetablegenerateform.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"
#include "timetableexport.h"

#include "timetablegeneratesettingsform.h"

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

#include "timerutils.h"

#include "centerwidgetonscreen.h"

#include "interfaceutils.h"

#include <QGenericMatrix>

const QString settingsName=QString("TimetableGenerateUnsuccessfulForm");

TimetableGenerateForm::TimetableGenerateForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

    gen = new Generate(TContext::get()->instance);

	currentResultsTextEdit->setReadOnly(true);
	
	connect(startPushButton, SIGNAL(clicked()), this, SLOT(start()));
	connect(stopPushButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(writeResultsPushButton, SIGNAL(clicked()), this, SLOT(write()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(closePressed()));
	connect(helpPushButton, SIGNAL(clicked()), this, SLOT(help()));
	connect(writeHighestStagePushButton, SIGNAL(clicked()), this, SLOT(writeHighestStage()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	startPushButton->setDefault(true);

    connect(gen, SIGNAL(simulationFinished()), this, SLOT(simulationFinished()));
    connect(gen, SIGNAL(impossibleToSolve()), this, SLOT(impossibleToSolve()));
    connect(gen, SIGNAL(iterationFinished(int, int, int, QString)), this, SLOT(iterationFinished(int, int, int, QString)));

    updateButtonsState();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    timerToAbort = new QTimer(this);
    connect(timerToAbort, SIGNAL(timeout()), this, SLOT(stop()));

    resetTimer();
}

void TimetableGenerateForm::updateTimer()
{
    QString s;

    s=TimetableGenerateForm::tr("Elapsed time: ");
    s+=utils::time::secondsToStr(time++);

    timeLabel->setText(s);
}

void TimetableGenerateForm::resetTimer()
{
    timer->stop();
    timerToAbort->stop();

    time = 0;
    timeAvg = 0;
    crtIter = 0;

    estTimeLabel->setText("");
}

void TimetableGenerateForm::updateButtonsState()
{
    startPushButton->setEnabled(!TContext::get()->simulation_running);
    generationSettingsPushButton->setEnabled(!TContext::get()->simulation_running);
    closePushButton->setEnabled(!TContext::get()->simulation_running);

    stopPushButton->setEnabled(TContext::get()->simulation_running);
    skipStepPushButton->setEnabled(TContext::get()->simulation_running);
    stopHighestPushButton->setEnabled(TContext::get()->simulation_running);

//    writeResultsPushButton->setEnabled(Timetable::getInstance()->simulation_running);
    writeHighestStagePushButton->setEnabled(TContext::get()->simulation_running);
}

void TimetableGenerateForm::printTestResults(QVector<QVector<double> > resultadosMatrix, QVector<int> precisoes)
{
    console::coutnl("\n\n#################################################################################");

    QString r_out;

    r_out = QString();
    for (int i = 0; i < (int) TContext::get()->instance.strategyList.size(); ++i) {
        for (int j = 0; j < resultadosMatrix[0].size(); ++j) {
            r_out += strings::number(resultadosMatrix[i][j], precisoes[j]);
            r_out += "\t";
        }
    }
    console::coutnl("\n\n%1\n", C_STR(r_out));

    r_out = QString();
    for (int i = 0; i < (int) TContext::get()->instance.strategyList.size(); ++i) {
        for (int j = 0; j < resultadosMatrix[0].size(); ++j) {
            r_out += strings::number(resultadosMatrix[i][j], precisoes[j]);
            r_out += "\t";
        }
        r_out += "\t\n";
    }
    console::coutnl("\n\n%1\n", C_STR(r_out));

    r_out = QString();
    for (int j = 0; j < resultadosMatrix[0].size(); ++j) {
        for (int i = 0; i < (int) TContext::get()->instance.strategyList.size(); ++i) {
            r_out += strings::number(resultadosMatrix[i][j], precisoes[j]);
            r_out += "\t";
        }
    }
    console::coutnl("\n\n%1\n", C_STR(r_out));

    r_out = QString();
    for (int j = 0; j < resultadosMatrix[0].size(); ++j) {
        for (int i = 0; i < (int) TContext::get()->instance.strategyList.size(); ++i) {
            r_out += strings::number(resultadosMatrix[i][j], precisoes[j]);
            r_out += "\t";
        }
        r_out += "\t\n";
    }
    console::coutnl("\n\n%1\n", C_STR(r_out));
}

TimetableGenerateForm::~TimetableGenerateForm()
{
	saveFETDialogGeometry(this);
    if(TContext::get()->simulation_running)
		this->stop();
    delete gen;
}

void TimetableGenerateForm::closeEvent(QCloseEvent * event)
{
    if(TContext::get()->simulation_running){
        QMessageBox::warning(this, TimetableGenerateForm::tr("m-FET warning"), TimetableGenerateForm::tr("Simulation running!"));
        event->ignore();
    }
    else{
        event->accept();
    }
}

void TimetableGenerateForm::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape){
        QDialog::keyPressEvent(e);
    }
    else {
        this->close();
    }
}

void TimetableGenerateForm::closePressed()
{
    if(!TContext::get()->simulation_running)
        this->close();
}

void TimetableGenerateForm::start(){
	currentResultsTextEdit->setPlainText(TimetableGenerateForm::tr("Entering simulation....precomputing, please be patient"));
    currentResultsTextEdit->update();

    Enums::Status status = gen->runGenerate(true, this);

    if (status == Enums::Success) {
        timer->start(1000);
        timeLabel->setText("");
        timerToAbort->start(60000 * MAX_MINUTES);
    }
    else if (status == Enums::Fail || status == Enums::Cancel){
        currentResultsTextEdit->setPlainText(TimetableGenerateForm::tr("Cannot generate - please modify your data"));
        currentResultsTextEdit->update();
    }
    else if (status == Enums::Cancel){}

    updateButtonsState();
}

void TimetableGenerateForm::run_tests_generate()
{
    TContext::get()->solutions.clear();

    int N = 3;

    QVector<int> precisoes = {2, 1, 2, 2, 1, 2, 2};

    QString resultTemplate_7 = "C: %1  |  V: %2  |  R: %3  |  T: %4  |  I: %5  |  Tb: %6  |  Tv: %7";

    console::coutnl("\n\n>>>> %1\n\n", C_STR(TContext::get()->getInstanceName()));

    utils::time::ClockTimer ctGeral;
    utils::time::ClockTimer ctPorRodada;

    QVector<QVector<double>> resultadosMatrix;

    int strategyIndex = 0;

    for (GenerationStrategy* generationStrategy: TContext::get()->instance.strategyList) {

        gen->generationStrategy = generationStrategy;

        console::cout("%1", C_STR(generationStrategy->description()));

        ctGeral.init_clock();

        double costSum = 0;
        double restriSum = 0;
        double initSum = 0;
        double iterOfBestSum = 0;
        double timeOfBestSum = 0;
        double timeOfBestViableSum = 0;

        for (int r = 1; r < N + 1; ++r) {
            ctPorRodada.init_clock();

            gen->generate(false);

            Solution s_new = gen->solutionFound;

            double c = s_new.cost();
            costSum += c;

            double e = s_new.getViolationsFactorForGroupEssential();
            restriSum += e;

            double n_reinicios_medio = generationStrategy->n_reinicios_medio();
            initSum += n_reinicios_medio;

            unsigned int iterOfBest = generationStrategy->getInicialPhase()->getIterationOfNewBest();
            iterOfBestSum += iterOfBest;

            double timeOfBest = generationStrategy->timeOfBestCost;
            timeOfBestSum += timeOfBest;

            double timeOfBestViable = generationStrategy->timeOfBestViable;
            timeOfBestViableSum += timeOfBestViable;

            Solution s = *std::next(TContext::get()->solutions.begin(), r - 1);

            QString template_7_single = strings::number(r) + ": " + resultTemplate_7 + " ... \n";
            console::cout(C_STR(template_7_single),

                      C_STR(strings::number(c, 2)),
                      C_STR(strings::number(e, 1)),
                      C_STR(strings::number(n_reinicios_medio, 1)),
                      C_STR(strings::number(ctPorRodada.elapsedMinutes(), 1)),
                      C_STR(strings::number(iterOfBest, 1)),
                      C_STR(strings::number(timeOfBest, 2)),
                      C_STR(strings::number(timeOfBestViable, 2))
                      );
        }

        ctGeral.end_clock();

        double costMean = costSum/N;
        double restriMean = restriSum/N;
        double initMean = initSum/N;
        double timeMean = ctGeral.elapsedMinutes()/N;
        double iterOfBestMean = iterOfBestSum/N;
        double timeOfBestMean = timeOfBestSum/N;
        double timeOfBestViableMean = timeOfBestViableSum/N;

        QVector<double> v_temp;

        v_temp.push_back(costMean);                //0
        v_temp.push_back(restriMean);              //1
        v_temp.push_back(initMean);                //2
        v_temp.push_back(timeMean);                //3
        v_temp.push_back(iterOfBestMean);          //4
        v_temp.push_back(timeOfBestMean);          //5
        v_temp.push_back(timeOfBestViableMean);    //6


        resultadosMatrix.push_back(v_temp);

        QString resultadoStr(resultTemplate_7);
        resultadoStr = resultadoStr.arg(strings::number(costMean, 2),
                                        strings::number(restriMean, 1),
                                        strings::number(initMean, 2),
                                        strings::number(timeMean, 2),
                                        strings::number(iterOfBestMean, 1),
                                        strings::number(timeOfBestMean, 2),
                                        strings::number(timeOfBestViableMean, 2)
                                        );
        console::coutnl("\t\tMédias: ( %1 )\n\n\n",
                    C_STR(resultadoStr));

        TContext::get()->saveSolutions(QString("/home/leo/")
                         + TContext::get()->getInstanceName()
                         + " ["
                         + "("
                         + strings::number(strategyIndex)
                         + ")"
                         + generationStrategy->description()
                         + "]"
                         + ".solutions"
                         );

        TContext::get()->solutions.clear();

        strategyIndex++;
    }

    printTestResults(resultadosMatrix, precisoes);
}

void TimetableGenerateForm::run_tests_improve()
{
    assert(TContext::get()->solutions.size() > 0);

    console::coutnl("\n\n>>>>melhorando %1\n\n", C_STR(TContext::get()->getInstanceName()));
    console::coutnl("Arquivo das soluções iniciais:\n%1\n\n", C_STR(TContext::get()->nome_arquivo_solucoes));

    QVector<int> precisoes = {2, 1, 2, 2, 2};

    QString resultTemplate_5 = "C: %1  |  V: %2  |  T: %3  |  Tb: %4  |  Tv: %5";

    std::multiset<Solution> solutionsCopia = TContext::get()->solutions;

    TContext::get()->solutions.clear();

    int N = (int) solutionsCopia.size();

    utils::time::ClockTimer ctGeral;
    utils::time::ClockTimer ctPorRodada;

    QVector<QVector<double>> resultadosMatrix;

    int strategyIndex = 0;

    for (GenerationStrategy* generationStrategy: TContext::get()->instance.strategyList) {

        gen->generationStrategy = generationStrategy;

        console::cout("%1", C_STR(generationStrategy->description()));

        ctGeral.init_clock();

        double costSum = 0;
        double restriSum = 0;
        double timeOfBestSum = 0;
        double timeOfBestViableSum = 0;

        int ri = 1;

        for(Solution s: solutionsCopia) {
            ctPorRodada.init_clock();

            gen->improve(s);

            Solution s_new = gen->solutionFound;

            double c = s_new.cost();
            costSum += c;

            double e = s_new.getViolationsFactorForGroupEssential();
            restriSum += e;

            double timeOfBest = generationStrategy->timeOfBestCost;
            timeOfBestSum += timeOfBest;

            double timeOfBestViable = generationStrategy->timeOfBestViable;
            timeOfBestViableSum += timeOfBestViable;

            QString template_5_single = strings::number(ri) + ": " + resultTemplate_5 + " ... \n";
            console::cout(C_STR(template_5_single),

                      C_STR(strings::number(c, 2)),
                      C_STR(strings::number(e, 1)),
                      C_STR(strings::number(ctPorRodada.elapsedMinutes(), 1)),
                      C_STR(strings::number(timeOfBest, 2)),
                      C_STR(strings::number(timeOfBestViable, 2))
                      );
            ++ri;
        }

        ctGeral.end_clock();

        double costMean = costSum/N;
        double restriMean = restriSum/N;
        double timeMean = ctGeral.elapsedMinutes()/N;
        double timeOfBestMean = timeOfBestSum/N;
        double timeOfBestViableMean = timeOfBestViableSum/N;

        QVector<double> v_temp;

        v_temp.push_back(costMean);                //0
        v_temp.push_back(restriMean);              //1
        v_temp.push_back(timeMean);                //2
        v_temp.push_back(timeOfBestMean);          //3
        v_temp.push_back(timeOfBestViableMean);    //4

        resultadosMatrix.push_back(v_temp);

        QString resultadoStr(resultTemplate_5);
        resultadoStr = resultadoStr.arg(strings::number(costMean, 2),
                                        strings::number(restriMean, 1),
                                        strings::number(timeMean, 2),
                                        strings::number(timeOfBestMean, 2),
                                        strings::number(timeOfBestViableMean, 2)
                                        );
        console::coutnl("\t\tMédias: ( %1 )\n\n\n",  C_STR(resultadoStr));

        TContext::get()->saveSolutions(QString("/media/leo/")
                         + TContext::get()->getInstanceName()
                         + " IMPROVE"
                         + " ["
                         + "("
                         + strings::number(strategyIndex)
                         + ")"
                         + generationStrategy->improvementDescription()
                         + "]"
                         + ".solutions"
                         , TContext::get()->nome_arquivo_solucoes);

        TContext::get()->solutions.clear();

        strategyIndex++;
    }

    printTestResults(resultadosMatrix, precisoes);
}

void TimetableGenerateForm::startImproving(int index)
{
    if(!TContext::get()->instance.internalStructureComputed){
        if(!TContext::get()->instance.computeInternalStructure(this)){
            QMessageBox::warning(this, TimetableGenerateForm::tr("m-FET warning"), TimetableGenerateForm::tr("Data is wrong. Please correct and try again"));
            return;
        }
    }

    if(!TContext::get()->instance.initialized || TContext::get()->instance.activitiesList.isEmpty()){
        QMessageBox::critical(this, TimetableGenerateForm::tr("m-FET information"),
            TimetableGenerateForm::tr("You have entered simulation with uninitialized rules or 0 activities...aborting"));
        assert(0);exit(1);
    }

    currentResultsTextEdit->setPlainText(TimetableGenerateForm::tr("Entering simulation....precomputing, please be patient"));

    gen->generationStrategy = TContext::get()->instance.getCurrentStrategy();

    bool ok=gen->precompute(this);

    if(!ok){
        currentResultsTextEdit->setPlainText(TimetableGenerateForm::tr("Cannot generate - please modify your data"));
        currentResultsTextEdit->update();

        QMessageBox::information(this, TimetableGenerateForm::tr("m-FET information"),
         TimetableGenerateForm::tr("Your data cannot be processed - please modify it as instructed"));

        return;
    }

    Solution s = *std::next(TContext::get()->solutions.begin(), index);

    gen->improve(s);

    updateButtonsState();

    timer->start(1000);
    timeLabel->setText("");
    timerToAbort->start(60000 * MAX_MINUTES);
}

void TimetableGenerateForm::simulationFinished()
{
    if(!TContext::get()->simulation_running){
		return;
	}

    TContext::get()->simulation_running=false;

    Solution& c=gen->solutionFound;

	//needed to find the conflicts strings
    c.cost(true);

    TContext::get()->addSolution(c);

//	TimetableExport::writeSimulationResults(this);

    QString s="\n********************************************************************************************\n";
    s+=TimetableGenerateForm::tr("Generation successful!");
    s+="\n\n";
    s+=c.getCostDetailsString(Enums::Short);
    s+="\n\n";
    s+=tr(" -> Number of broken constraints: %1").arg(c.constraintConflictData.conflictsDescriptionList().count());
    s+="\n\n";
    s+=tr("Results were saved in the directory:\n %1").arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"timetables"+TimetableExport::getOutPutDir_single()));
    s+="\n********************************************************************************************\n";

    currentResultsTextEdit->appendPlainText(s);

    updateButtonsState();

    resetTimer();

#ifndef Q_WS_QWS
    QApplication::beep();
#endif
}

void TimetableGenerateForm::stop()
{
    if(!TContext::get()->simulation_running){
        return;
    }

    CRITICAL(TContext::get()->myMutex,
        gen->generationStrategy->abort();
    )

    QString s="\n\n";
    s+=TimetableGenerateForm::tr(" > Stopping simulation, please be patient...");

    if (timeAvg > 0) {
        int t = ((int)((crtIter + 1) * timeAvg)) - (unsigned)time;

        if (t < 0) {
            t = 0;
        }

        s+=TimetableGenerateForm::tr(" Total estimated time to stop successfully: %1").arg(utils::time::secondsToStr(t));
    }

    s+=QString("\n\n");

    currentResultsTextEdit->appendPlainText(s);

    updateButtonsState();
    stopPushButton->setEnabled(false);
}

void TimetableGenerateForm::on_skipStepPushButton_clicked()
{
    if(!TContext::get()->simulation_running){
        return;
    }

    CRITICAL(TContext::get()->myMutex,
        gen->generationStrategy->simulation_skip_step=true;
    )
}

void TimetableGenerateForm::iterationFinished(int searchTime, int it, int nIterations, QString best){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    Q_UNUSED(searchTime)Q_UNUSED(it)Q_UNUSED(nIterations)

    //write to the Qt interface
    QString s;
    s+= gen->generationStrategy->description() + "\n";
//    s+=TimetableGenerateForm::tr("%1 out of %2 iterations done").arg(it).arg(nIterations)+"\n";
    s+= "\nGenerating...\n\n";

    if (!best.isEmpty()) {
        s+= "\nBest so far:\n\n" + best;
    }

    currentResultsTextEdit->setPlainText(s);

//    int p = (it*100)/nIterations;
//    generationProgressBar->setValue(p);

//    if (it > 0) {
//        timeAvg = (double)searchTime / (double)it;
//        crtIter = it;

//        s=TimetableGenerateForm::tr("Estimated time until done: ");
//        s+=utils::time::secondsToStr((int)(timeAvg*(nIterations - it)));

//        estTimeLabel->setText(s);
//    }
}

void TimetableGenerateForm::write(){
//	Timetable::getInstance()->myMutex.lock();

//	Solution& c=gen->solutionFound;

//	//needed to find the conflicts strings
//	QString tmp;
//    c.cost(&tmp);

//    Timetable::getInstance()->setUpTimetable(c);

////	TimetableExport::writeSimulationResults(this);

//	Timetable::getInstance()->myMutex.unlock();

//	QMessageBox::information(this, TimetableGenerateForm::tr("m-FET information"),
//		TimetableGenerateForm::tr("Simulation results should now be written in the directory %1 in html and xml mode"
//        " and the conflicts in txt mode").arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"timetables"+TimetableExport::getOutPutDir_single())));
}

void TimetableGenerateForm::stopHighest()
{
//    if(!Timetable::getInstance()->simulation_running){
//        return;
//    }

//    Timetable::getInstance()->simulation_running=false;

//    Timetable::getInstance()->myMutex.lock();
//    gen->abortOptimization=true;
//    Timetable::getInstance()->myMutex.unlock();

//    Timetable::getInstance()->myMutex.lock();

//    Solution& c=highestStageSolution;

//    //needed to find the conflicts strings
//    QString tmp;
//    c.cost(&tmp);

//    Timetable::getInstance()->setUpTimetable(c);

////    TimetableExport::writeHighestStageResults(this);

//    QString s=TimetableGenerateForm::tr("Simulation interrupted! m-FET could not find a timetable."
//     " Maybe you can consider lowering the constraints.");

//    s+=" ";

//    s+=TimetableGenerateForm::tr("The partial highest-stage results were saved in the directory %1")
//     .arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"timetables"+TimetableExport::getOutPutDir_highest()));

//    s+="\n\n";

//    s+=TimetableGenerateForm::tr("Additional information relating impossible to schedule activities:");
//    s+="\n\n";

//    s+=tr("m-FET managed to schedule correctly the first %1 most difficult activities."
//     " You can see initial order of placing the activities in the generate dialog. The activity which might cause problems"
//     " might be the next activity in the initial order of evaluation. This activity is listed below:")
//     .arg(maxActivitiesPlaced);

//    s+="\n\n";

//    Timetable::getInstance()->myMutex.unlock();

//    //show the message in a dialog
//    QDialog dialog(this);

//    dialog.setWindowTitle(TimetableGenerateForm::tr("Generation stopped (highest stage)", "The title of a dialog, meaning that the generation of the timetable was stopped "
//        "and highest stage timetable written."));

//    QVBoxLayout* vl=new QVBoxLayout(&dialog);
//    QPlainTextEdit* te=new QPlainTextEdit();
//    te->setPlainText(s);
//    te->setReadOnly(true);
//    QPushButton* pb=new QPushButton(TimetableGenerateForm::tr("OK"));

//    QHBoxLayout* hl=new QHBoxLayout(0);
//    hl->addStretch(1);
//    hl->addWidget(pb);

//    vl->addWidget(te);
//    vl->addLayout(hl);
//    connect(pb, SIGNAL(clicked()), &dialog, SLOT(close()));

//    dialog.resize(700,500);
//    centerWidgetOnScreen(&dialog);
//    restoreFETDialogGeometry(&dialog, settingsName);

//    setParentAndOtherThings(&dialog, this);
//    dialog.exec();
//    saveFETDialogGeometry(&dialog, settingsName);

    //    updateButtonsState();
}

void TimetableGenerateForm::writeHighestStage(){
//	Timetable::getInstance()->myMutex.lock();

//	Solution& c=highestStageSolution;

//	//needed to find the conflicts strings
//	QString tmp;
//    c.cost(&tmp);

//    Timetable::getInstance()->setUpTimetable(c);

////	TimetableExport::writeHighestStageResults(this);

//	Timetable::getInstance()->myMutex.unlock();

//	QMessageBox::information(this, TimetableGenerateForm::tr("m-FET information"),
//		TimetableGenerateForm::tr("Highest stage results should now be written in the directory %1 in html and xml mode"
//        " and the conflicts in txt mode").arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"timetables"+TimetableExport::getOutPutDir_highest())));
}

void TimetableGenerateForm::impossibleToSolve()
{
//    if(!Timetable::getInstance()->simulation_running){
//		return;
//	}

//    Timetable::getInstance()->simulation_running=false;

//	Timetable::getInstance()->myMutex.lock();
//	gen->abortOptimization=true;
//	Timetable::getInstance()->myMutex.unlock();

//	Timetable::getInstance()->myMutex.lock();

//	Solution& c=gen->solutionFound;

//	//needed to find the conflicts strings
//	QString tmp;
//    c.cost(&tmp);

//    Timetable::getInstance()->setUpTimetable(c);

//	//update the string representing the conflicts
//    Timetable::getInstance()->conflictsStringTitle=TimetableGenerateForm::tr("Conflicts", "Title of dialog");
//    Timetable::getInstance()->conflictsString=c.getConflictsDescriptionString();

//	TimetableExport::writeSimulationResults(this);

//	QString s=TimetableGenerateForm::tr("Simulation impossible! Maybe you can consider lowering the constraints.");

//	s+=" ";

//	s+=TimetableGenerateForm::tr("The partial results were saved in the directory %1")
//	 .arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"timetables"+TimetableExport::getOutPutDir_single()));

//	s+="\n\n";

//	s+=TimetableGenerateForm::tr("Additional information relating impossible to schedule activities:");
//	s+="\n\n";
//	s+=TimetableGenerateForm::tr("Please check the constraints related to the "
//	 "activity below, which might be impossible to schedule:");
//	s+="\n\n";
//	for(int i=0; i<gen->nDifficultActivities; i++){
//		int ai=gen->difficultActivities[i];

//		s+=TimetableGenerateForm::tr("No: %1").arg(i+1);

//		s+=", ";

//		s+=TimetableGenerateForm::tr("Id: %1 (%2)", "%1 is id of activity, %2 is detailed description of activity")
//            .arg(Timetable::getInstance()->instance.activeActivitiesList[ai]->id)
//            .arg(Timetable::getInstance()->instance.getActivityDetailedDescription(Timetable::getInstance()->instance.activeActivitiesList[ai]->id));

//		s+="\n";
//	}

//	Timetable::getInstance()->myMutex.unlock();

//	//show the message in a dialog
//	QDialog dialog(this);

//	dialog.setWindowTitle(TimetableGenerateForm::tr("Generation impossible", "The title of a dialog, meaning that the generation of the timetable is impossible."));

//	QVBoxLayout* vl=new QVBoxLayout(&dialog);
//	QPlainTextEdit* te=new QPlainTextEdit();
//	te->setPlainText(s);
//	te->setReadOnly(true);
//	QPushButton* pb=new QPushButton(TimetableGenerateForm::tr("OK"));

//	QHBoxLayout* hl=new QHBoxLayout(0);
//	hl->addStretch(1);
//	hl->addWidget(pb);

//	vl->addWidget(te);
//	vl->addLayout(hl);
//	connect(pb, SIGNAL(clicked()), &dialog, SLOT(close()));

//	dialog.resize(700,500);
//	centerWidgetOnScreen(&dialog);
//	restoreFETDialogGeometry(&dialog, settingsName);

//	setParentAndOtherThings(&dialog, this);

//#ifndef Q_WS_QWS
//	QApplication::beep();
//#endif

//	dialog.exec();
//	saveFETDialogGeometry(&dialog, settingsName);

//	startPushButton->setEnabled(true);
//	stopPushButton->setDisabled(true);
//	closePushButton->setEnabled(true);
//	writeResultsPushButton->setDisabled(true);
    //	writeHighestStagePushButton->setDisabled(true);
}

void TimetableGenerateForm::help()
{
    QString s;

    s+=TimetableGenerateForm::tr("Please wait. It might take 5 to 20 minutes or even more for very difficult timetables");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("Activities are placed in order, most difficult ones first");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("The process of searching is semi-randomized, which means that "
     "you will get different timetables and running times each time. You can choose the best timetable from several runs");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("Usually, there is no need to stop and restart the search."
     " But for very difficult timetables this can help. Sometimes in such cases m-FET can become stuck and cycle forever,"
     " and restarting might produce a very fast solution.");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("It is recommended to strengthen the constraints step by step (for"
     " instance min days between activities weight or teacher(s) max gaps), as you obtain feasible timetables.");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("If your timetable gets stuck on a certain activity number k (and then"
     " begins going back), please check the initial evaluation order and see activity number k+1 in this list. You may find"
     " errors this way.");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("If the generation is successful, you cannot have hard conflicts. You can have only soft conflicts,"
     " corresponding to constraints with weight lower than 100.0%, which are reported in detail.");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("After the generation (successful or interrupted), you can view the current (complete or incomplete) timetable"
     " in the corresponding view timetable dialogs, and the list of conflicts in the view conflicts dialog.");
    s+="\n\n";
    s+=TimetableGenerateForm::tr("The results are saved in your selected results directory in HTML and XML mode and the soft conflicts"
     " in text mode, along with the current data and timetable, saved as a .m-fet data file (with activities locked by constraints), so"
     " that you can open, modify and regenerate the current timetable later");

    MessagesManager::information(this, tr("m-FET help"), s);
}

void TimetableGenerateForm::on_generationSettingsPushButton_clicked()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

    TimetableGenerateSettingsForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

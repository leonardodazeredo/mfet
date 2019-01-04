
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "tcontext.h"

#ifndef FET_COMMAND_LINE
#include <QFileDialog>
#include <QMessageBox>
#endif

#include <QDir>

#include "textmessages.h"

#include "generate_pre.h"
#include "strategies.h"

#include "stringutils.h"
#include "fileutils.h"

#define INSTANCE_EXT    QString(".m-fet.xml")
#define STRATEGIES_EXT  QString(".m-fet.strategies.json")
#define SOLUTIONS_EXT   QString(".m-fet.solutions.json")

#define DIALOG_FILE_FILTER_INSTANCE     QString("Instance XML files (*.m-fet.xml)") + ";;" + QString("All files (*)")

#define DIALOG_FILE_FILTER_STRATEGIES   QString("Strategies JSON files (*.m-fet.strategies.json)") + ";;" + QString("All files (*)")

#define DIALOG_FILE_FILTER_SOLUTIONS    QString("Solutions JSON files (*.m-fet.solutions.json)") + ";;" + QString("All files (*)")

TContext* TContext::_instance = nullptr;

int TContext::debugCount = 0;

TContext* TContext::get()
{
    debugCount = debugCount + 1;
    if (_instance==nullptr){
        _instance = new TContext();
        assert(debugCount==1);
    }
    return _instance;
}

TContext::TContext()
{
    simulation_running = false;
    assert(debugCount==1);
}
TContext::TContext(const TContext &){}
TContext &TContext::operator=(const TContext &){return *this;}

void TContext::loadDefaultStrategies()
{
    QString dirn = QDir::currentPath();
    QString fn = QString(dirn + "/strategies/default" + STRATEGIES_EXT);
    if (utils::file::exists(fn)){
        loadStrategies(nullptr, fn);
    }
}

void TContext::getNumberOfPlacedActivities(int& number1, int& number2)
{
    Solution best_solution = bestSolution();

    number1=0;
    for(int i=0; i<instance.activeActivitiesList.size(); i++)
        if(best_solution.times[i]!=UNALLOCATED_TIME)
            number1++;

    number2=0;
}

void TContext::addSolution(Solution &s)
{
    std::multiset<Solution> solutionsAux;

    for (const Solution& s: solutions) {
        Solution sa = s;
        sa.stateChanged();
        sa.cost();
        solutionsAux.insert(sa);
    }

    solutions = solutionsAux;

    Solution solution(s);
    solution.stateChanged();
    solution.cost();
    solution.hashCode();
    solutions.insert(solution);
}



Solution TContext::bestSolution()
{
    Solution s = *solutions.begin();
    s.computeTeachersTimetable();
    s.computeSubgroupsTimetable();
    return s;
}

void TContext::resetSchedule()
{
    for (auto s: solutions) {
        s.students_schedule_ready=false;
        s.teachers_schedule_ready=false;
    }
}


QString TContext::getInstanceName()
{
    QString instanceName;

    QString fileName = getInstanceFileName();

    int fsidx = fileName.indexOf(".");
    instanceName = fileName.left(fsidx);

    return instanceName;
}

QString TContext::getInstanceFileName()
{
    QString fileName;

    QString fileFullPath = getInstanceFileFullPath();

    int fsidx = fileFullPath.lastIndexOf(defs::FILE_SEP);
    fileName = fileFullPath.right(fileFullPath.length() - fsidx - 1);

    return fileName;
}

QString TContext::getInstanceFileFullPath()
{
    return defs::INPUT_FILENAME_XML;
}

QString TContext::getInstanceFileDirectoryFullPath()
{
    QString fileDirectoryFullPath;

    QString fileFullPath = getInstanceFileFullPath();

    int fsidx = fileFullPath.lastIndexOf(defs::FILE_SEP);
    fileDirectoryFullPath = fileFullPath.left(fsidx);

    return fileDirectoryFullPath;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Enums::Status TContext::saveInstance(QString fileNameAndPath)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for instance's output file is empty."));
        return Enums::Cancel;
    }

    return saveInstance(nullptr, fileNameAndPath, false);
}

Enums::Status TContext::saveInstance(QWidget *parent, QString fileNameAndPath, bool showMessages)
{
    Q_UNUSED(showMessages)

#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        QString predefFileName=defs::INPUT_FILENAME_XML;
        if(predefFileName.isEmpty())
            predefFileName=defs::WORKING_DIRECTORY+defs::FILE_SEP+QString("untitled")+INSTANCE_EXT;
        fileNameAndPath = QFileDialog::getSaveFileName(parent, QString("Choose a filename to save under"),
                                                       predefFileName, DIALOG_FILE_FILTER_INSTANCE, nullptr, QFileDialog::DontConfirmOverwrite);
    }
#endif

    if(fileNameAndPath==QString()) {
        return Enums::Cancel;
    }

    if(fileNameAndPath.right(INSTANCE_EXT.length())!=INSTANCE_EXT) {
        fileNameAndPath+=INSTANCE_EXT;
    }

    int tmp=fileNameAndPath.lastIndexOf(defs::FILE_SEP);
    defs::WORKING_DIRECTORY=fileNameAndPath.left(tmp);

    fileNameAndPath = utils::file::makeIndexedFilePath(fileNameAndPath);

    if(TContext::get()->instance.write(parent, fileNameAndPath)){
        defs::INPUT_FILENAME_XML = fileNameAndPath;

        TContext::get()->instance.setModified(true); //force update of the modified flag of the main window

        return Enums::Success;
    }
    else{
        return Enums::Fail;
    }
}

Enums::Status TContext::loadInstance(QString fileNameAndPath)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for instance's input file is empty."));
        return Enums::Cancel;
    }

    return loadInstance(nullptr, fileNameAndPath, false);
}

Enums::Status TContext::loadInstance(QWidget *parent, QString fileNameAndPath, bool showMessages, bool commandLine, QString commandLineDirectory)
{
    if(this->simulation_running){
        if (showMessages) MessagesManager::information(parent, QString(""), QString("Allocation in course.\nPlease stop simulation before this."));
        return Enums::Cancel;
    }

    if(this->instance.isModified()){
        int confirm = MessagesManager::confirmation(
                    QString(""),
                    QString("Your current data file has been modified. Are you sure you want to open another data file?"),
                    parent
                    );

        if (confirm) {
            return Enums::Cancel;
        }
    }

#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        fileNameAndPath = QFileDialog::getOpenFileName(parent, QString("Choose a file to open"), defs::WORKING_DIRECTORY, DIALOG_FILE_FILTER_INSTANCE);
    }
#endif

    if(fileNameAndPath==QString()) {
        return Enums::Cancel;
    }

    QFile inputFile(fileNameAndPath);
    if(!inputFile.open(QIODevice::ReadOnly)){
        if (showMessages) MessagesManager::information(parent, QString(), QString("Could not open file - not existing or in use"));
        return Enums::Cancel;
    }

    this->instance.setModified(true); //to avoid flicker of the main form modified flag

    if(this->instance.read(parent, fileNameAndPath, commandLine, commandLineDirectory)){
        this->resetSchedule();

        defs::INPUT_FILENAME_XML = fileNameAndPath;

        this->solutions.clear();

        this->instance.setModified(true); //force update of the modified flag of the main window

        //get the directory
        int tmp=fileNameAndPath.lastIndexOf(defs::FILE_SEP);
        defs::WORKING_DIRECTORY=fileNameAndPath.left(tmp);

        return Enums::Success;
    }
    else{
        assert(!this->simulation_running);
        this->instance.setModified(false);

        return Enums::Fail;
    }
}

bool TContext::saveSolutions(QString fileNameAndPath, QString headerContent)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for solutions's output file is empty."));
        return false;
    }

    return saveSolutions(nullptr, fileNameAndPath, headerContent, false);
}

bool TContext::saveSolutions(QWidget* parent, QString fileNameAndPath, QString headerContent, bool showMessages)
{
    if(this->solutions.empty()){
        if (showMessages) MessagesManager::warning(parent, QString(), QString("No solutions."));
        return false;
    }

#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        QString predefFileName=defs::INPUT_FILENAME_XML;
        if(predefFileName.isEmpty())
            predefFileName=defs::WORKING_DIRECTORY+defs::FILE_SEP+QString("untitled")+SOLUTIONS_EXT;
        fileNameAndPath = QFileDialog::getSaveFileName(parent, QString("Choose a filename to save under"), predefFileName, DIALOG_FILE_FILTER_SOLUTIONS);
    }
#endif

    if(fileNameAndPath==QString()) {
        return false;
    }

    if(fileNameAndPath.right(SOLUTIONS_EXT.length())!=SOLUTIONS_EXT) {
        fileNameAndPath+=SOLUTIONS_EXT;
    }

    int tmp=fileNameAndPath.lastIndexOf(defs::FILE_SEP);
    defs::WORKING_DIRECTORY=fileNameAndPath.left(tmp);

    /* Commenting out to never overwrite solutions
    if(QFile::exists(fileNameAndPath)) {
        auto choice = MessagesManager::confirmation(QString("Warning"),
                                                    QString("File %1 exists - are you sure you want to overwrite it?")
                                                    .arg(fileNameAndPath),
                                                    parent
                                                    );

        if(choice == MessagesManager::No)
            return false;
    }
    */

    fileNameAndPath = utils::file::makeIndexedFilePath(fileNameAndPath);

    assert(!QFile::exists(fileNameAndPath));

    QFile outputfile(fileNameAndPath);

    if(!outputfile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        if (showMessages) MessagesManager::critical(parent,
                                                    QString(),
                                                    QString("Cannot open %1 for writing ... please check write permissions of the selected directory or your disk free space. Saving of file aborted")
                                                    .arg(QFileInfo(fileNameAndPath).fileName())
                                                    );

        return false;
    }

    QTextStream tos(&outputfile);

    tos.setCodec("UTF-8");
    tos.setGenerateByteOrderMark(true);

    json js(solutions);

    json j;
    j["instanceFileName"] = TContext::get()->getInstanceFileName().toStdString();
    j["instanceHash"] = instance.hashCode().toStdString();
    j["solutions"] = js;

    std::string jdump = j.dump(4);
    tos << QString::fromStdString(jdump);

    if(outputfile.error()>0){
        if (showMessages) MessagesManager::critical(parent,
                                  QString(),
                                  QString("Saved file gave error code %1, which means saving is compromised. Please check your disk free space")
                                  .arg(outputfile.error())
                                  );
        outputfile.close();
        return false;
    }

    outputfile.close();

    ///////////////////////////////////////////////////////////////////////////////////

    if (headerContent!=QString()) {
        QString headerFilePath = fileNameAndPath + "-HEADER";
        QFile headerFile(headerFilePath);

        if(!headerFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            if (showMessages) MessagesManager::warning(parent, QString(), QString("Error when opening header file."));
            return false;
        }

        QTextStream tos2(&headerFile);

        tos2 << headerContent;

        if(headerFile.error()>0){
            headerFile.close();
            if (showMessages) MessagesManager::warning(parent, QString(), QString("Error when writing into header file."));
            return false;
        }

        headerFile.close();
    }

    ///////////////////////////////////////////////////////////////////////////////////

    return true;
}

bool TContext::loadSolutions(QString fileNameAndPath)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for solutions's input file is empty."));
        return false;
    }

    return loadSolutions(nullptr, fileNameAndPath, false);
}

bool TContext::loadSolutions(QWidget *parent, QString fileNameAndPath, bool showMessages)
{
#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        fileNameAndPath = QFileDialog::getOpenFileName(parent, QString("Choose a file to open"), defs::WORKING_DIRECTORY, DIALOG_FILE_FILTER_SOLUTIONS);
    }
#endif

    if(fileNameAndPath==QString()) {
        return false;
    }

    QFile inputFile(fileNameAndPath);
    if(!inputFile.open(QIODevice::ReadOnly)){
        if (showMessages) MessagesManager::information(parent, QString(), QString("Could not open file - not existing or in use"));
        return false;
    }

    QTextStream in(&inputFile);

    QString jsonString = in.readAll();

    if(inputFile.error()>0){
        if (showMessages) MessagesManager::information(parent, QString(), QString("Error opening input file."));
        inputFile.close();
        return false;
    }

    inputFile.close();

    instance.computeInternalStructure(parent);

    processConstraints(nullptr);

    json j = json::parse(jsonString.toStdString());

    std::multiset<Solution> solutionsAux;

    for (const Solution& s: solutions) {
        Solution sa = s;
        sa.stateChanged();
        sa.cost();
        solutionsAux.insert(sa);
    }

    solutions = solutionsAux;

    if (j.find("solutions") != j.end()) {
        if (j.find("instanceHash") != j.end()) {
            if (j["instanceHash"]!=TContext::get()->instance.hashCode().toStdString()) {
                if (showMessages) MessagesManager::critical(parent, QString(), QString("Invalid instance hash."));
                inputFile.close();
                return false;
            }
        }
    }
    else{
        if (j[0].find("instanceHash") != j[0].end()) {
            if (j[0]["instanceHash"]!=TContext::get()->instance.hashCode().toStdString()) {
                if (showMessages) MessagesManager::critical(parent, QString(), QString("Invalid instance hash."));
                inputFile.close();
                return false;
            }
        }
    }

    auto& _j = j.find("solutions") == j.end() ? j : j["solutions"];

    for (auto& sj : _j) {
        Solution s = sj;
        s.stateChanged();
        s.cost();
        solutions.insert(s);
    }

    int tmp=fileNameAndPath.lastIndexOf(defs::FILE_SEP);
    QString auxStr=fileNameAndPath.right(fileNameAndPath.length()-tmp-1);

    this->nome_arquivo_solucoes = auxStr;

    return true;
}

bool TContext::safeStrategies(QString fileNameAndPath)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for strategies's output file is empty."));
        return false;
    }

    return safeStrategies(nullptr, fileNameAndPath, false);
}

bool TContext::safeStrategies(QWidget *parent, QString fileNameAndPath, bool showMessages)
{
    if(this->instance.strategyList.empty()){
        if (showMessages) MessagesManager::warning(parent, QString(), QString("No strategies."));
        return false;
    }

#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        QString predefFileName=defs::INPUT_FILENAME_XML;
        if(predefFileName.isEmpty())
            predefFileName=defs::WORKING_DIRECTORY+defs::FILE_SEP+QString("untitled")+STRATEGIES_EXT;
        fileNameAndPath = QFileDialog::getSaveFileName(parent, QString("Choose a filename to save under"), predefFileName, DIALOG_FILE_FILTER_STRATEGIES);
    }
#endif

    if(fileNameAndPath==QString()) {
        return false;
    }

    if(fileNameAndPath.right(STRATEGIES_EXT.length())!=STRATEGIES_EXT) {
        fileNameAndPath+=STRATEGIES_EXT;
    }

    int tmp=fileNameAndPath.lastIndexOf(defs::FILE_SEP);
    defs::WORKING_DIRECTORY=fileNameAndPath.left(tmp);

    fileNameAndPath = utils::file::makeIndexedFilePath(fileNameAndPath);

    assert(!QFile::exists(fileNameAndPath));

    QFile outPutFile(fileNameAndPath);
    if(!outPutFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        if (showMessages) MessagesManager::critical(parent, QString(),
                                                QString("Cannot open %1 for writing ... please check write permissions of the selected directory or your disk free space. Saving of file aborted")
                                                .arg(QFileInfo(fileNameAndPath).fileName()));
        return false;
    }

    QTextStream tos(&outPutFile);

    tos.setCodec("UTF-8");
    tos.setGenerateByteOrderMark(true);

    json j;

    for (auto es: instance.strategyList){
        j.push_back(generationStrategyToJson(*es));
    }

    std::string jdump = j.dump(4);
    tos << QString::fromStdString(jdump);

    if(outPutFile.error()>0){
        if (showMessages) MessagesManager::critical(parent, QString(),
                                                QString("Saved file gave error code %1, which means saving is compromised. Please check your disk free space")
                                                .arg(outPutFile.error()));
        outPutFile.close();
        return false;
    }

    outPutFile.close();

    return true;
}

bool TContext::loadStrategies(QString fileNameAndPath)
{
    if(fileNameAndPath==QString()) {
        MessagesManager::critical(nullptr, QString(), QString("Path for strategies's input file is empty."));
        return false;
    }

    return loadStrategies(nullptr, fileNameAndPath, false);
}

bool TContext::loadStrategies(QWidget *parent, QString fileNameAndPath, bool showMessages)
{
#ifndef FET_COMMAND_LINE
    if(fileNameAndPath==QString()) {
        fileNameAndPath = QFileDialog::getOpenFileName(parent, QString("Choose a file to open"), defs::WORKING_DIRECTORY,  DIALOG_FILE_FILTER_STRATEGIES);
    }
#endif

    if(fileNameAndPath==QString()) {
        return false;
    }

    QFile inputFile(fileNameAndPath);
    if(!inputFile.open(QIODevice::ReadOnly)){
        if (showMessages) MessagesManager::information(parent, QString(), QString("Could not open file - not existing or in use"));
        return false;
    }

    QTextStream in(&inputFile);

    QString jsonString = in.readAll();

    if(inputFile.error()>0){
        if (showMessages) MessagesManager::critical(parent, QString(), QString("Error code %1.").arg(inputFile.error()));
        inputFile.close();
        return false;
    }

    inputFile.close();

    json j = json::parse(jsonString.toStdString());

    for (auto ej: j){
        instance.strategyList.push_back(generationStrategyFromJson(ej));
    }

    return true;
}

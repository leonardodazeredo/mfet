/*
File rules.cpp
*/

/***************************************************************************
                          rules.cpp  -  description
                             -------------------
    begin                : 2002
    copyright            : (C) 2002 by Lalescu Liviu
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

#include "instance.h"

#ifndef FET_COMMAND_LINE
#include <QProgressDialog>
#endif
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDate>
#include <QTime>
#include <QLocale>
#include <QString>
#include <QXmlStreamReader>
#include <QTranslator>
#include <QtAlgorithms>
#include <QtGlobal>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QMetaEnum>
#include <QRegExp>
#include <QCryptographicHash>

#include <algorithm>
#include <iostream>

#include "defs.h"
#include "tcontext.h"

#include "enumutils.h"
#include "stringutils.h"
#include "randutils.h"
#include "decimalutils.h"


#include "textmessages.h"
#include "centerwidgetonscreen.h"

using namespace std;



FakeString::FakeString()
{
}

void FakeString::operator=(const QString& other)
{
    Q_UNUSED(other);
}

void FakeString::operator=(const char* str)
{
    Q_UNUSED(str);
}

void FakeString::operator+=(const QString& other)
{
    Q_UNUSED(other);
}

void FakeString::operator+=(const char* str)
{
    Q_UNUSED(str);
}

GenerationStrategy *Instance::getCurrentStrategy()
{
    if (this->strategyList.size() == 0)
        return nullptr;

    assert(this->strategy_index_to_use < this->strategyList.size());

    GenerationStrategy * gs = this->strategyList[this->strategy_index_to_use];

    return gs;
}

int Instance::getNTimeSlots()
{
    return (nDaysPerWeek * nHoursPerDay);
}

void Instance::init() //initializes the rules (empty, but with default hours and days)
{
    this->institutionName=tr("Default institution");
    this->comments=tr("Default comments");

    this->nDaysPerWeek=5;
    this->daysOfTheWeek[0] = tr("Monday");
    this->daysOfTheWeek[1] = tr("Tuesday");
    this->daysOfTheWeek[2] = tr("Wednesday");
    this->daysOfTheWeek[3] = tr("Thursday");
    this->daysOfTheWeek[4] = tr("Friday");

    //defaults
    this->nHoursPerDay=12;
    this->hoursOfTheDay[0]=tr("08:00", "Hour name");
    this->hoursOfTheDay[1]=tr("09:00", "Hour name");
    this->hoursOfTheDay[2]=tr("10:00", "Hour name");
    this->hoursOfTheDay[3]=tr("11:00", "Hour name");
    this->hoursOfTheDay[4]=tr("12:00", "Hour name");
    this->hoursOfTheDay[5]=tr("13:00", "Hour name");
    this->hoursOfTheDay[6]=tr("14:00", "Hour name");
    this->hoursOfTheDay[7]=tr("15:00", "Hour name");
    this->hoursOfTheDay[8]=tr("16:00", "Hour name");
    this->hoursOfTheDay[9]=tr("17:00", "Hour name");
    this->hoursOfTheDay[10]=tr("18:00", "Hour name");
    this->hoursOfTheDay[11]=tr("19:00", "Hour name");

    permanentStudentsHash.clear();

    groupActivitiesPointerHash.clear();
    activitiesPointerHash.clear();
    bctSet.clear();
    btSet.clear();
    apstHash.clear();
    mdbaHash.clear();
    tnatHash.clear();
    ssnatHash.clear();

    this->initialized=true;
}

int Instance::getActiveTeacherIndex(const Teacher &teacher)
{
    for(int i=0; i<this->activeTeachersList.size(); i++)
        if(this->activeTeachersList.at(i)==&teacher)
            return i;

    return -1;
}

int Instance::getTeacherIndex(const QString& teacherName){
    return teachersHash.value(teacherName, -1);
}

int Instance::getSubjectIndex(const QString& subjectName){
    return subjectsHash.value(subjectName, -1);
}

int Instance::getActivityTagIndex(const QString& activityTagName){
    return activityTagsHash.value(activityTagName, -1);
}

StudentsSet* Instance::getStudentsSet(const QString& studentsSetName){
    return studentsHash.value(studentsSetName, nullptr);
}

int Instance::getActivityIndex(int activityId){
    return activeActivitiesHash.value(activityId, -1);
}

bool Instance::containsActivityInActiveList(int id){
    return activeActivitiesHash.contains(id);
}

QString Instance::getDetailedDescription()
{
    QString s= "";

    s.append(tr("Name= %1").arg(institutionName));
    s.append("\n");
    s.append(tr("# hours per day= %1").arg(utils::strings::number(nHoursPerDay)));
    s.append("\n");
    s.append(tr("# days per week= %1").arg(utils::strings::number(nDaysPerWeek)));
    s.append("\n");
    s.append(tr("# teachers= %1").arg(utils::strings::number(teachersList.size())));
    s.append("\n");
    s.append(tr("# subjects= %1").arg(utils::strings::number(subjectsList.size())));
    s.append("\n");
    s.append(tr("# students groups= %1").arg(utils::strings::number(yearsList.size())));
    s.append("\n");
    s.append(tr("# activities= %1").arg(utils::strings::number(groupActivitiesPointerHash.size())));
    s.append("\n");
    s.append(tr("# classes= %1").arg(utils::strings::number(activitiesList.size())));
    s.append("\n");

    if(!comments.isEmpty()){
        s.append(tr("Comments:\n%1").arg(comments));
        s.append("\n");
    }

    return s;
}

bool Instance::isModified() const
{
    return modified;
}

void Instance::setModified(bool value)
{
    modified = value;

    if (modified) {
        TContext::get()->resetSchedule();
        emit gotModified();
    }
}

QString Instance::getActivityDetailedDescription(int activityId)
{
    QString s;

    Activity* act=activitiesPointerHash.value(activityId, nullptr);
    if(act==nullptr){
        s+=QCoreApplication::translate("Activity", "Invalid (inexistent) id for activity");
        return s;
    }

    if(act->activityTagsNames.count()>0){
        s+=QCoreApplication::translate("Activity", "S:%2, AT:%3, St:%4", "This is an important translation for an activity's detailed description, please take care (it appears in many places in constraints)."
                                                                               "The abbreviations are: Teachers, Subject, Activity tags, Students. This variant includes activity tags").arg(act->subjectName).arg(act->activityTagsNames.join(",")).arg(act->studentSetsNames.join(","));
    }
    else{
        s+=QCoreApplication::translate("Activity", "S:%2, St:%3", "This is an important translation for an activity's detailed description, please take care (it appears in many places in constraints)."
                                                                        "The abbreviations are: Teachers, Subject, Students. There are no activity tags here").arg(act->subjectName).arg(act->studentSetsNames.join(","));
    }
    return s;
}

int Instance::getConstraintGroupWeight(Enums::ConstraintGroup g)
{
    switch (g) {
    case Enums::ConstraintGroup::Essential:
        return currentGenerationSettings.essential_constraint_weight;
    case Enums::ConstraintGroup::Important:
        return currentGenerationSettings.important_constraint_weight;
    case Enums::ConstraintGroup::Desirable:
        return currentGenerationSettings.desirable_constraint_weight;
    default:
        assert(0);
        break;
    }
}

int Instance::getSubjectPrefferenceWeight()
{
    return currentGenerationSettings.subjects_preferences_weight;
}

int Instance::getSubjectPrefferenceFactor(Enums::SubjectPreference pref)
{
    switch (pref) {
    case Enums::SubjectPreference::Very_High:
        return P_VH;
    case Enums::SubjectPreference::High:
        return P_H;
    case Enums::SubjectPreference::Indifferent:
        return P_I;
    case Enums::SubjectPreference::Low:
        return P_L;
    case Enums::SubjectPreference::Very_Low:
        return P_VL;
    default:
        assert(0);
        break;
    }
}

QByteArray Instance::hashCode()
{
    if (!internalStructureComputed) {
        computeInternalStructure(nullptr);
    }

    QString auxString;

    auxString+= QString::number(nHoursPerDay);
    auxString+= QString::number(nDaysPerWeek);
    auxString+= QString::number(activeTeachersList.size());
    auxString+= QString::number(subjectsList.size());
    auxString+= QString::number(activityTagsList.size());
    auxString+= QString::number(directSubgroupsList.size());
    auxString+= QString::number(activeActivitiesList.size());
    auxString+= QString::number(validTimeConstraintsList.size());

    return QCryptographicHash::hash(auxString.toUtf8(),QCryptographicHash::Sha256).toHex();
}

void Instance::renameAllActivitiesForSubject(const QString &inputedSubjectName)
{
    int i = 1;

    if (!groupActivitiesPointerHash.isEmpty()) {
        if (groupActivitiesPointerHash.contains(0)) {
            foreach (Activity* a, *groupActivitiesPointerHash.value(0)) {
                if (inputedSubjectName == a->subjectName) {
                    a->activityName = "T" + QString::number(i);
                    i++;
                }
            }
        }

        int maxGroupId = 0;

        foreach (int k, groupActivitiesPointerHash.keys()) {
            if (k > maxGroupId) {
                maxGroupId = k;
            }
        }

        if (maxGroupId > 0) {
            for (int groupId = 1; groupId <= maxGroupId; ++groupId) {

                assert(groupActivitiesPointerHash.size() > 0);

                ActivitiesList* al = groupActivitiesPointerHash.value(groupId, nullptr);

                if (al != nullptr) {
                    assert(al->size() > 0);

                    if (inputedSubjectName == al->at(0)->subjectName) {
                        foreach (Activity* a, *al) {
                            a->activityName = "T" + QString::number(i);
                        }

                        i++;
                    }
                }
            }
        }
    }
}

bool Instance::computeInternalStructure(QWidget* parent)
{
    //To fix a bug reported by Frans on forum, on 7 May 2010.
    //If user generates, then changes some activities (changes teachers of them), then tries to generate but m-FET cannot precompute in generate_pre.cpp,
    //then if user views the timetable, the timetable of a teacher contains activities of other teacher.
    //The bug appeared because it is possible to compute internal structure, so internal activities change the teacher, but the timetables remain the same,
    //with the same activities indexes.
    TContext::get()->resetSchedule();

    //The order is important - firstly the teachers, subjects, activity tags and students.
    //After that, the buildings.
    //After that, the rooms.
    //After that, the activities.
    //After that, the time constraints.
    //After that, the space constraints.

    if(this->teachersList.size()>MAX_TEACHERS){
        MessagesManager::warning(parent, tr("m-FET information"),
                                 tr("You have too many teachers. You need to increase the variable MAX_TEACHERS (which is currently %1).")
                                 .arg(MAX_TEACHERS));
        return false;
    }

    //kill augmented students sets
    QList<StudentsYear*> ayears;
    QList<StudentsGroup*> agroups;
    QList<StudentsSubgroup*> asubgroups;
    foreach(StudentsYear* year, augmentedYearsList){
        if(!ayears.contains(year))
            ayears.append(year);
        foreach(StudentsGroup* group, year->groupsList){
            if(!agroups.contains(group))
                agroups.append(group);
            foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                if(!asubgroups.contains(subgroup))
                    asubgroups.append(subgroup);
            }
        }
    }
    foreach(StudentsYear* year, ayears){
        assert(year!=nullptr);
        delete year;
    }
    foreach(StudentsGroup* group, agroups){
        assert(group!=nullptr);
        delete group;
    }
    foreach(StudentsSubgroup* subgroup, asubgroups){
        assert(subgroup!=nullptr);
        delete subgroup;
    }
    augmentedYearsList.clear();
    //////////////////

    //copy list of students sets into augmented list
    QHash<QString, StudentsSet*> augmentedHash;

    foreach(StudentsYear* y, yearsList){
        StudentsYear* ay=new StudentsYear();
        ay->name=y->name;
        ay->numberOfStudents=y->numberOfStudents;
        ay->groupsList.clear();
        augmentedYearsList << ay;

        assert(!augmentedHash.contains(ay->name));
        augmentedHash.insert(ay->name, ay);

        foreach(StudentsGroup* g, y->groupsList){
            if(augmentedHash.contains(g->name)){
                StudentsSet* tmpg=augmentedHash.value(g->name);
                assert(tmpg->type==STUDENTS_GROUP);
                ay->groupsList<<((StudentsGroup*)tmpg);
            }
            else{
                StudentsGroup* ag=new StudentsGroup();
                ag->name=g->name;
                ag->numberOfStudents=g->numberOfStudents;
                ag->subgroupsList.clear();
                ay->groupsList << ag;

                assert(!augmentedHash.contains(ag->name));
                augmentedHash.insert(ag->name, ag);

                foreach(StudentsSubgroup* s, g->subgroupsList){
                    if(augmentedHash.contains(s->name)){
                        StudentsSet* tmps=augmentedHash.value(s->name);
                        assert(tmps->type==STUDENTS_SUBGROUP);
                        ag->subgroupsList<<((StudentsSubgroup*)tmps);
                    }
                    else{
                        StudentsSubgroup* as=new StudentsSubgroup();
                        as->name=s->name;
                        as->numberOfStudents=s->numberOfStudents;
                        ag->subgroupsList << as;

                        assert(!augmentedHash.contains(as->name));
                        augmentedHash.insert(as->name, as);
                    }
                }
            }
        }
    }

    /////////
    for(int i=0; i<this->augmentedYearsList.size(); i++){
        StudentsYear* sty=this->augmentedYearsList[i];

        //if this year has no groups, insert something to simulate the whole year
        if(sty->groupsList.count()==0){
            StudentsGroup* tmpGroup = new StudentsGroup();
            tmpGroup->name = sty->name+" "+tr("Automatic Group", "Please keep the translation short. It is used when a year contains no groups and an automatic group "
                                                                 "is added in the year, in the timetable (when viewing the students timetable from m-FET and also in the html timetables for students groups or subgroups)"
                                                                 ". In the empty year there will be added a group with name = yearName+a space character+your translation of 'Automatic Group'.");
            tmpGroup->numberOfStudents = sty->numberOfStudents;
            sty->groupsList << tmpGroup;
        }

        for(int j=0; j<sty->groupsList.size(); j++){
            StudentsGroup* stg=sty->groupsList[j];

            //if this group has no subgroups, insert something to simulate the whole group
            if(stg->subgroupsList.size()==0){
                StudentsSubgroup* tmpSubgroup = new StudentsSubgroup();
                tmpSubgroup->name = stg->name+" "+tr("Automatic Subgroup", "Please keep the translation short. It is used when a group contains no subgroups and an automatic subgroup "
                                                                           "is added in the group, in the timetable (when viewing the students timetable from m-FET and also in the html timetables for students subgroups)"
                                                                           ". In the empty group there will be added a subgroup with name = groupName+a space character+your translation of 'Automatic Subgroup'.");
                tmpSubgroup->numberOfStudents=stg->numberOfStudents;
                stg->subgroupsList << tmpSubgroup;
            }
        }
    }
    //////////

    QSet<StudentsGroup*> allGroupsSet;
    QSet<StudentsSubgroup*> allSubgroupsSet;
    QList<StudentsGroup*> allGroupsList;
    QList<StudentsSubgroup*> allSubgroupsList;

    for(int i=0; i<this->augmentedYearsList.size(); i++){
        StudentsYear* sty=this->augmentedYearsList.at(i);
        sty->indexInAugmentedYearsList=i;

        for(int j=0; j<sty->groupsList.size(); j++){
            StudentsGroup* stg=sty->groupsList.at(j);
            if(!allGroupsSet.contains(stg)){
                allGroupsSet.insert(stg);
                allGroupsList.append(stg);
                stg->indexInInternalGroupsList=allGroupsSet.count()-1;
            }

            for(int k=0; k<stg->subgroupsList.size(); k++)
                if(!allSubgroupsSet.contains(stg->subgroupsList.at(k))){
                    allSubgroupsSet.insert(stg->subgroupsList.at(k));
                    allSubgroupsList.append(stg->subgroupsList.at(k));
                    stg->subgroupsList.at(k)->indexInInternalSubgroupsList=allSubgroupsSet.count()-1;
                }
        }
    }
    int tmpNSubgroups = allSubgroupsList.count();
    if(tmpNSubgroups > MAX_TOTAL_SUBGROUPS){
        MessagesManager::warning(parent, tr("m-FET information"),
                                 tr("You have too many total subgroups. You need to increase the variable MAX_TOTAL_SUBGROUPS (which is currently %1).")
                                 .arg(MAX_TOTAL_SUBGROUPS));
        return false;
    }

    int counter=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList.at(i);
        if(act->active)
            counter++;
    }
    if(counter>MAX_ACTIVITIES){
        MessagesManager::warning(parent, tr("m-FET information"),
                                 tr("You have too many active activities. You need to increase the variable MAX_ACTIVITIES (which is currently %1).")
                                 .arg(MAX_ACTIVITIES));
        return false;
    }

    assert(this->initialized);

    //days and hours
    assert(this->nHoursPerDay>0);
    assert(this->nDaysPerWeek>0);
    this->nHoursPerWeek=this->nHoursPerDay*this->nDaysPerWeek;

    //teachers
    assert(this->teachersList.size()<=MAX_TEACHERS);

    activeTeachersList.clear();

    for (auto t:  teachersList){
        if (t->active) {
            activeTeachersList.append(t);
        }
    }

    teachersHash.clear();
    for(int i=0; i<teachersList.size(); i++)
        teachersHash.insert(teachersList.at(i)->name, i);

    //subjects
    subjectsHash.clear();
    for(int i=0; i<subjectsList.size(); i++)
        subjectsHash.insert(subjectsList.at(i)->name, i);

    this->directSubgroupsList.clear();
    //students
    for(int i=0; i<allSubgroupsList.count(); i++){
        assert(allSubgroupsList.at(i)->indexInInternalSubgroupsList==i);
        this->directSubgroupsList << allSubgroupsList.at(i);
    }

    this->internalGroupsList.clear();
    for(int i=0; i<allGroupsList.count(); i++){
        assert(allGroupsList.at(i)->indexInInternalGroupsList==i);
        this->internalGroupsList.append(allGroupsList.at(i));
    }

    studentsHash.clear();
    for(StudentsYear* year: augmentedYearsList){
        studentsHash.insert(year->name, year);
        foreach(StudentsGroup* group, year->groupsList){
            studentsHash.insert(group->name, group);
            foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                studentsHash.insert(subgroup->name, subgroup);
        }
    }

    assert(this->directSubgroupsList.size()==tmpNSubgroups);

    //activities
    int range=0;
    foreach(Activity* act, this->activitiesList)
        if(act->active)
            range++;
    QProgressDialog progress(parent);
    progress.setWindowTitle(tr("Computing internal structure", "Title of a progress dialog"));
    progress.setLabelText(tr("Processing internally the activities ... please wait"));
    progress.setRange(0, qMax(range, 1));
    progress.setModal(true);
    int ttt=0;

    Activity* activity;
    counter=0;

    this->inactiveActivitiesIdsSet.clear();

    for(int i=0; i<this->activitiesList.size(); i++){
        activity=this->activitiesList[i];
        if(activity->active){
            progress.setValue(ttt);

            if(progress.wasCanceled()){
                progress.setValue(range);
                MessagesManager::warning(parent, tr("m-FET information"), tr("Canceled"));
                return false;
            }
            ttt++;

            counter++;
            activity->computeInternalStructure(*this);
        }
        else
            inactiveActivitiesIdsSet.insert(activity->id);
    }

    progress.setValue(qMax(range, 1));

    foreach (StudentsSubgroup* subgroup, directSubgroupsList) {
        subgroup->activitiesForSubgroup.clear();
    }

    assert(counter<=MAX_ACTIVITIES);
    this->activeActivitiesList.clear();
    int activei=0;
    for(int ai=0; ai<this->activitiesList.size(); ai++){
        activity=this->activitiesList[ai];
        if(activity->active){
            this->activeActivitiesList << activity;

            for(int j=0; j<activity->iSubgroupsList.count(); j++){
                int k=activity->iSubgroupsList.at(j);
                //The test below takes time
                assert(!directSubgroupsList[k]->activitiesForSubgroup.contains(activei));
                directSubgroupsList[k]->activitiesForSubgroup.append(activei);
            }

            activei++;
        }
    }

    activeActivitiesHash.clear();
    foreach (Activity* activity, activeActivitiesList) {
        assert(!activeActivitiesHash.contains(activity->id));
        activeActivitiesHash.insert(activity->id, activeActivitiesList.indexOf(activity));
    }

    //activities list for each subject - used for subjects timetable - in order for students and teachers
    activitiesForSubjectList.resize(subjectsList.size());
    activitiesForSubjectSet.resize(subjectsList.size());
    for(int sb=0; sb<subjectsList.size(); sb++){
        activitiesForSubjectList[sb].clear();
        activitiesForSubjectSet[sb].clear();
    }

    for(int i=0; i<this->augmentedYearsList.size(); i++){
        StudentsYear* sty=this->augmentedYearsList[i];

        for(int j=0; j<sty->groupsList.size(); j++){
            StudentsGroup* stg=sty->groupsList[j];

            for(int k=0; k<stg->subgroupsList.size(); k++){
                StudentsSubgroup* sts=stg->subgroupsList[k];

                foreach(int ai, directSubgroupsList[sts->indexInInternalSubgroupsList]->activitiesForSubgroup)
                    if(!activitiesForSubjectSet[activeActivitiesList[ai]->subjectIndex].contains(ai)){
                        activitiesForSubjectList[activeActivitiesList[ai]->subjectIndex].append(ai);
                        activitiesForSubjectSet[activeActivitiesList[ai]->subjectIndex].insert(ai);
                    }
            }
        }
    }
//TODO: !!!
//    for(int i=0; i<teachersList.size(); i++){
//        foreach(int ai, teachersList.at(i)->activitiesForTeacher)
//            if(!activitiesForSubjectSet[activeActivitiesList[ai]->subjectIndex].contains(ai)){
//                activitiesForSubjectList[activeActivitiesList[ai]->subjectIndex].append(ai);
//                activitiesForSubjectSet[activeActivitiesList[ai]->subjectIndex].insert(ai);
//            }
//    }

    //for activities without students or teachers
    for(int ai=0; ai<activeActivitiesList.size(); ai++){
        int si=activeActivitiesList[ai]->subjectIndex;
        if(!activitiesForSubjectSet[si].contains(ai)){
            activitiesForSubjectList[si].append(ai);
            activitiesForSubjectSet[si].insert(ai);
        }
    }
    /////////////////////////////////////////////////////////////////

    //activities list for each activity tag - used for activity tags timetable - in order for students and teachers
    activitiesForActivityTagList.resize(activityTagsList.size());
    activitiesForActivityTagSet.resize(activityTagsList.size());
    for(int a=0; a<activityTagsList.size(); a++){
        activitiesForActivityTagList[a].clear();
        activitiesForActivityTagSet[a].clear();
    }

    for(int i=0; i<this->augmentedYearsList.size(); i++){
        StudentsYear* sty=this->augmentedYearsList[i];

        for(int j=0; j<sty->groupsList.size(); j++){
            StudentsGroup* stg=sty->groupsList[j];

            for(int k=0; k<stg->subgroupsList.size(); k++){
                StudentsSubgroup* sts=stg->subgroupsList[k];

                foreach(int ai, directSubgroupsList[sts->indexInInternalSubgroupsList]->activitiesForSubgroup)
                    foreach(int activityTagInt, activeActivitiesList[ai]->iActivityTagsSet)
                        if(!activitiesForActivityTagSet[activityTagInt].contains(ai)){
                            activitiesForActivityTagList[activityTagInt].append(ai);
                            activitiesForActivityTagSet[activityTagInt].insert(ai);
                        }
            }
        }
    }
//TODO: !!!
//    for(int i=0; i<teachersList.size(); i++){
//        foreach(int ai, teachersList.at(i)->activitiesForTeacher)
//            foreach(int activityTagInt, activeActivitiesList[ai]->iActivityTagsSet)
//                if(!activitiesForActivityTagSet[activityTagInt].contains(ai)){
//                    activitiesForActivityTagList[activityTagInt].append(ai);
//                    activitiesForActivityTagSet[activityTagInt].insert(ai);
//                }
//    }

    //for activities without students or teachers
    for(int ai=0; ai<activeActivitiesList.size(); ai++){
        foreach(int ati, activeActivitiesList[ai]->iActivityTagsSet){
            if(!activitiesForActivityTagSet[ati].contains(ai)){
                activitiesForActivityTagList[ati].append(ai);
                activitiesForActivityTagSet[ati].insert(ai);
            }
        }
    }
    /////////////////////////////////////////////////////////////////

    bool ok=true;

    //time constraints
    //progress.reset();

    bool skipInactiveTimeConstraints=false;

    TimeConstraint* timeConstraint;

    QSet<int> toSkipTimeIndexesSet;

    int _c=0;

    for(int timeConstraintIndex=0; timeConstraintIndex<this->timeConstraintsList.size(); timeConstraintIndex++){
        timeConstraint=this->timeConstraintsList[timeConstraintIndex];

        if(!timeConstraint->active){
            toSkipTimeIndexesSet.insert(timeConstraintIndex);
        }
        else if(timeConstraint->hasInactiveActivities()){

            toSkipTimeIndexesSet.insert(timeConstraintIndex);

            if(!skipInactiveTimeConstraints){
                QString s=tr("The following time constraint is ignored, because it refers to inactive activities:");
                s+="\n";
                s+=timeConstraint->getDetailedDescription();

                int t=MessagesManager::confirmation(parent, tr("m-FET information"), s, tr("Skip rest"), tr("See next"), QString(), 1, 0 );

                if(t==0)
                    skipInactiveTimeConstraints=true;
            }
        }
        else{
            _c++;
        }
    }

    validTimeConstraintsList.clear();

    progress.setLabelText(tr("Processing internally the time constraints ... please wait"));
    progress.setRange(0, qMax(timeConstraintsList.size(), 1));
    ttt=0;

    for(int timeConstraintIndex=0; timeConstraintIndex<this->timeConstraintsList.size(); timeConstraintIndex++){
        progress.setValue(ttt);

        if(progress.wasCanceled()){
            progress.setValue(timeConstraintsList.size());
            MessagesManager::warning(parent, tr("m-FET information"), tr("Canceled"));
            return false;
        }

        ttt++;

        timeConstraint=this->timeConstraintsList[timeConstraintIndex];

        if(toSkipTimeIndexesSet.contains(timeConstraintIndex))
            continue;

        if(!timeConstraint->computeInternalStructure(parent)){
            ok=false;
            continue;
        }

        this->validTimeConstraintsList << timeConstraint;
    }

    progress.setValue(qMax(timeConstraintsList.size(), 1));

    if(defs::VERBOSE){
        cout<<_c<<" time constraints after first pass (after removing inactive ones)"<<endl;
        cout<<"  " << this->validTimeConstraintsList.size() << " time constraints after second pass (after removing wrong ones)\n"<<endl;
    }
    assert(_c>=this->validTimeConstraintsList.size()); //because some constraints may have toSkipTime false, but computeInternalStructure also false
    //assert(this->nInternalTimeConstraints<=MAX_TIME_CONSTRAINTS);


    ////////////COISAS TIRADAS DO GENERETE_PRE
    this->breakDayHour.resize(this->nDaysPerWeek, this->nHoursPerDay);
    //BREAK
    for(int j=0; j<this->nDaysPerWeek; j++)
        for(int k=0; k<this->nHoursPerDay; k++)
            this->breakDayHour[j][k]=false;

    this->subgroupNotAvailableDayHour.resize(this->directSubgroupsList.size(), this->nDaysPerWeek, this->nHoursPerDay);
    //STUDENTS SET NOT AVAILABLE
    for(int i=0; i<this->directSubgroupsList.size(); i++)
        for(int j=0; j<this->nDaysPerWeek; j++)
            for(int k=0; k<this->nHoursPerDay; k++)
                this->subgroupNotAvailableDayHour[i][j][k]=false;

    this->teacherNotAvailableDayHour.resize(this->teachersList.size(), this->nDaysPerWeek, this->nHoursPerDay);
    //TEACHER NOT AVAILABLE
    for(int i=0; i<this->teachersList.size(); i++)
        for(int j=0; j<this->nDaysPerWeek; j++)
            for(int k=0; k<this->nHoursPerDay; k++)
                this->teacherNotAvailableDayHour[i][j][k]=false;
    ////////////

    for (auto sb : subjectsList){
        sb->preferredTeachers.clear();
        sb->restrictedTeachers.clear();
        sb->indifferentTeachers.clear();
    }

    for (auto sg : directSubgroupsList)
        sg->notAvailableTimeSlots.clear();

    for (auto tch : teachersList){
        tch->notAvailableTimeSlots.clear();
        tch->preferredSubjects.clear();
        tch->restrictedSubjects.clear();
        tch->indifferentSubjects.clear();
    }

    for (auto tc : validTimeConstraintsList){
        if (tc->type == CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES) {
            ConstraintStudentsSetNotAvailableTimes* c = (ConstraintStudentsSetNotAvailableTimes*) tc;

            for (auto isg : c->iSubgroupsList){
                for (int i = 0; i < c->days.size(); ++i)
                    directSubgroupsList[isg]->notAvailableTimeSlots.insert(c->days[i]*nHoursPerDay + c->hours[i]);
            }
        }
        else if (tc->type == CONSTRAINT_BREAK_TIMES) {
            ConstraintBreakTimes* c = (ConstraintBreakTimes*) tc;

            for (int isg = 0; isg < this->directSubgroupsList.size(); ++isg)
                for (int i = 0; i < c->days.size(); ++i)
                    directSubgroupsList[isg]->notAvailableTimeSlots.insert(c->days[i]*nHoursPerDay + c->hours[i]);

            for (int tchi = 0; tchi < this->teachersList.size(); ++tchi)
                for (int i = 0; i < c->days.size(); ++i)
                    teachersList[tchi]->notAvailableTimeSlots.insert(c->days[i]*nHoursPerDay + c->hours[i]);
        }
        else if (tc->type == CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES) {
            ConstraintTeacherNotAvailableTimes* c = (ConstraintTeacherNotAvailableTimes*) tc;

            for (int i = 0; i < c->days.size(); ++i)
                teachersList[c->teacher_ID]->notAvailableTimeSlots.insert(c->days[i]*nHoursPerDay + c->hours[i]);
        }

        ////////////COISAS TIRADAS DO GENERETE_PRE
        //TEACHER not available
        if(tc->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
            ConstraintTeacherNotAvailableTimes* tn=(ConstraintTeacherNotAvailableTimes*)tc;

            assert(tn->days.count()==tn->hours.count());

            for(int kk=0; kk<tn->days.count(); kk++){
                int d=tn->days.at(kk);
                int h=tn->hours.at(kk);

                this->teacherNotAvailableDayHour[tn->teacher_ID][d][h]=true;
            }
        }
        //STUDENTS SET not available
        if(tc->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
            ConstraintStudentsSetNotAvailableTimes* sn=(ConstraintStudentsSetNotAvailableTimes*)tc;

            for(int q=0; q<sn->iSubgroupsList.count(); q++){
                int ss=sn->iSubgroupsList.at(q);
                assert(sn->days.count()==sn->hours.count());
                for(int kk=0; kk<sn->days.count(); kk++){
                    int d=sn->days.at(kk);
                    int h=sn->hours.at(kk);

                    this->subgroupNotAvailableDayHour[ss][d][h]=true;
                }
            }
        }
        //BREAK
        if(tc->type==CONSTRAINT_BREAK_TIMES){
            ConstraintBreakTimes* br=(ConstraintBreakTimes*)tc;

            assert(br->days.count()==br->hours.count());

            for(int kk=0; kk<br->days.count(); kk++){
                int d=br->days.at(kk);
                int h=br->hours.at(kk);

                this->breakDayHour[d][h]=true;
            }
        }
        ////////////
    }

    for (auto tch : activeTeachersList){
        for (auto sb : subjectsList){
            switch (teacher_subject_preference_map[tch][sb]) {
            case Enums::SubjectPreference::Very_High:
                tch->preferredSubjects.insert(sb);
                sb->preferredTeachers.insert(tch);
                break;
            case Enums::SubjectPreference::High:
                tch->preferredSubjects.insert(sb);
                sb->preferredTeachers.insert(tch);
                break;
            case Enums::SubjectPreference::Indifferent:
                tch->indifferentSubjects.insert(sb);
                sb->indifferentTeachers.insert(tch);
                break;
            case Enums::SubjectPreference::Low:
                tch->restrictedSubjects.insert(sb);
                sb->restrictedTeachers.insert(tch);
                break;
            case Enums::SubjectPreference::Very_Low:
                tch->restrictedSubjects.insert(sb);
                sb->restrictedTeachers.insert(tch);
                break;
            default:
                assert(0);
                break;
            }
        }
    }

    this->allMovesN1.clear();
    this->allMovesN2.clear();
    this->allMovesN3.clear();
    this->allMovesN4.clear();
    this->allMovesN5.clear();

    this->allMovesN1 = _allMovesN1();
    this->allMovesN2 = _allMovesN2();
    this->allMovesN3 = _allMovesN3();
    this->allMovesN4 = _allMovesN4();
    this->allMovesN5 = _allMovesN5();

    //done.
    this->internalStructureComputed=ok;

    return ok;
}

vector<Move *> Instance::_allMovesN1()
{
    vector<Move*> movimentos;
    for (int t = 0; t < this->teachersList.size(); ++t) {

        if (!this->teachersList.at(t)->active) {
            continue;
        }

        for (int d = 0; d < this->nDaysPerWeek; ++d) {

            for (int i = 0; i < this->nHoursPerDay; ++i) {
                for (int j = i + 1; j < this->nHoursPerDay; ++j) {
                    MoveN1andN2* movimento = new MoveN1andN2();

                    movimento->tchi = t;
                    movimento->ts1 = i + d*this->nHoursPerDay;
                    movimento->ts2 = j + d*this->nHoursPerDay;

                    movimentos.push_back(movimento);
                }
            }
        }
    }

    return movimentos;
}

vector<Move *> Instance::_allMovesN2()
{
    vector<Move*> movimentos;
    for (int t = 0; t < this->teachersList.size(); ++t) {

        if (!this->teachersList.at(t)->active) {
            continue;
        }

        for (int d1 = 0; d1 < this->nDaysPerWeek; ++d1) {

            for (int i = 0; i < this->nHoursPerDay; ++i) {

                for (int d2 = d1 + 1; d2 < this->nDaysPerWeek; ++d2) {

                    for (int j = 0 ; j < this->nHoursPerDay; ++j) {
                        MoveN1andN2* movimento = new MoveN1andN2();

                        movimento->tchi = t;
                        movimento->ts1 = i + d1*this->nHoursPerDay;
                        movimento->ts2 = j + d2*this->nHoursPerDay;

                        movimentos.push_back(movimento);
                    }
                }
            }
        }
    }

    return movimentos;
}

vector<Move *> Instance::_allMovesN3()
{
    vector<Move*> movimentos;

    for (ActivitiesList* actList: this->groupActivitiesPointerHash.values()){
        for (int ai1 = 0; ai1 < actList->size(); ++ai1) {

            if (actList->at(ai1)->allowMultipleTeachers) {

                for (int ai2 = ai1; ai2 < actList->size(); ++ai2) {

                    MoveN3* movimento = new MoveN3();

                    movimento->ai1 = ai1;
                    movimento->ai2 = ai2;

                    movimentos.push_back(movimento);
                }
            }
        }
    }

    return movimentos;
}

vector<Move *> Instance::_allMovesN4()
{
    vector<Move*> movimentos;

    for (int ai = 0; ai < this->activeActivitiesList.size(); ++ai) {

        for (int t = 0; t < this->teachersList.size(); ++t) {

            if (!this->teachersList.at(t)->active) {
                continue;
            }

            MoveN4* movimento = new MoveN4();

            movimento->ai = ai;
            movimento->newTchi = t;

            movimentos.push_back(movimento);
        }
    }

    return movimentos;
}

vector<Move *> Instance::_allMovesN5()
{
    vector<Move*> movimentos;

    for (int ai1 = 0; ai1 < this->activeActivitiesList.size(); ++ai1) {

        for (int ai2 = ai1; ai2 < this->activeActivitiesList.size(); ++ai2) {

            MoveN5* movimento = new MoveN5();

            movimento->ai1 = ai1;
            movimento->ai2 = ai2;

            movimentos.push_back(movimento);
        }
    }

    return movimentos;
}


int Instance::getDayFromUnifiedTime(int timeSlot)
{
    return timeSlot/nHoursPerDay;
}

int Instance::getHourFromUnifiedTime(int timeSlot)
{
    return timeSlot - getDayFromUnifiedTime(timeSlot)*nHoursPerDay;
}

void Instance::kill() //clears memory for the rules, destroys them
{
    teacher_subject_preference_map.clear();

    //Teachers
    while(!teachersList.isEmpty())
        delete teachersList.takeFirst();

    //Subjects
    while(!subjectsList.isEmpty())
        delete subjectsList.takeFirst();

    //Years
    /*while(!yearsList.isEmpty())
                delete yearsList.takeFirst();*/

    //students sets
    QSet<StudentsYear*> iyears;
    QSet<StudentsGroup*> igroups;
    QSet<StudentsSubgroup*> isubgroups;
    foreach(StudentsYear* year, yearsList){
        if(!iyears.contains(year))
            iyears.insert(year);
        foreach(StudentsGroup* group, year->groupsList){
            if(!igroups.contains(group))
                igroups.insert(group);
            foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                if(!isubgroups.contains(subgroup))
                    isubgroups.insert(subgroup);
            }
        }
    }
    foreach(StudentsYear* year, iyears){
        assert(year!=nullptr);
        delete year;
    }
    foreach(StudentsGroup* group, igroups){
        assert(group!=nullptr);
        delete group;
    }
    foreach(StudentsSubgroup* subgroup, isubgroups){
        assert(subgroup!=nullptr);
        delete subgroup;
    }
    yearsList.clear();

    permanentStudentsHash.clear();
    //////////////////

    //kill augmented students sets
    QList<StudentsYear*> ayears;
    QList<StudentsGroup*> agroups;
    QList<StudentsSubgroup*> asubgroups;
    foreach(StudentsYear* year, augmentedYearsList){
        if(!ayears.contains(year))
            ayears.append(year);
        foreach(StudentsGroup* group, year->groupsList){
            if(!agroups.contains(group))
                agroups.append(group);
            foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                if(!asubgroups.contains(subgroup))
                    asubgroups.append(subgroup);
            }
        }
    }
    foreach(StudentsYear* year, ayears){
        assert(year!=nullptr);
        delete year;
    }
    foreach(StudentsGroup* group, agroups){
        assert(group!=nullptr);
        delete group;
    }
    foreach(StudentsSubgroup* subgroup, asubgroups){
        assert(subgroup!=nullptr);
        delete subgroup;
    }
    augmentedYearsList.clear();
    //////////////////

    //Activities
    while(!activitiesList.isEmpty())
        delete activitiesList.takeFirst();

    //Time constraints
    while(!timeConstraintsList.isEmpty())
        delete timeConstraintsList.takeFirst();

    groupActivitiesPointerHash.clear();
    activitiesPointerHash.clear();
    bctSet.clear();
    btSet.clear();
    apstHash.clear();
    mdbaHash.clear();
    tnatHash.clear();
    ssnatHash.clear();

    teachersHash.clear();
    subjectsHash.clear();
    activityTagsHash.clear();
    studentsHash.clear();
    activeActivitiesHash.clear();

    //done
    this->internalStructureComputed=false;
    this->initialized=false;
}

Instance::Instance()
{
    this->initialized=false;
    this->setModified(false);
}

Instance::~Instance()
{
    if(this->initialized)
        this->kill();
}

void Instance::setInstitutionName(const QString& newInstitutionName)
{
    this->institutionName=newInstitutionName;
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

void Instance::setComments(const QString& newComments)
{
    this->comments=newComments;
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addTeacher(Teacher* teacher)
{
    for(int i=0; i<this->teachersList.size(); i++){
        Teacher* tch=this->teachersList[i];
        if(tch->name==teacher->name)
            return false;
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    QHash<Subject*, Enums::SubjectPreference> subject_preference_map;

    for (auto sbj: subjectsList)
        if (!subject_preference_map.contains(sbj))
            subject_preference_map.insert(sbj, Enums::SubjectPreference::Indifferent);

    this->teacher_subject_preference_map.insert(teacher, subject_preference_map);

    this->teachersList.append(teacher);
    return true;
}

bool Instance::addTeacherFast(Teacher* teacher)
{
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    this->teachersList.append(teacher);
    return true;
}

int Instance::searchTeacher(const QString& teacherName)
{
    for(int i=0; i<this->teachersList.size(); i++)
        if(this->teachersList.at(i)->name==teacherName)
            return i;

    return -1;
}

bool Instance::removeTeacher(const QString& teacherName)
{
    //	QList<int> idsToBeRemoved;
    //	foreach(Activity* act, activitiesList){
    //		bool t=act->removeTeacher(teacherName);
    //		if(t && act->teacherName.count()==0)
    //			idsToBeRemoved.append(act->id);
    //	}
    //	removeActivities(idsToBeRemoved, false);

    for(int i=0; i<this->teachersList.size(); i++)
        if(this->teachersList.at(i)->name==teacherName){
            Teacher* tch=this->teachersList[i];
            this->teachersList.removeAt(i);
            delete tch;
            break;
        }

    updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    return true;
}

bool Instance::modifyTeacher(const QString& initialTeacherName, const QString& finalTeacherName)
{
    assert(this->searchTeacher(finalTeacherName)==-1);
    assert(this->searchTeacher(initialTeacherName)>=0);

    foreach(TimeConstraint* ctr, timeConstraintsList){
        if(ctr->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
            ConstraintTeacherNotAvailableTimes* crt_constraint=(ConstraintTeacherNotAvailableTimes*)ctr;
            if(initialTeacherName == crt_constraint->teacher)
                crt_constraint->teacher=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK){
            ConstraintTeacherMaxGapsPerWeek* crt_constraint=(ConstraintTeacherMaxGapsPerWeek*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY){
            ConstraintTeacherMaxGapsPerDay* crt_constraint=(ConstraintTeacherMaxGapsPerDay*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_HOURS_DAILY){
            ConstraintTeacherMaxHoursDaily* crt_constraint=(ConstraintTeacherMaxHoursDaily*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherMaxHoursContinuously* crt_constraint=(ConstraintTeacherMaxHoursContinuously*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherActivityTagMaxHoursContinuously* crt_constraint=(ConstraintTeacherActivityTagMaxHoursContinuously*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintTeacherActivityTagMaxHoursDaily* crt_constraint=(ConstraintTeacherActivityTagMaxHoursDaily*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MIN_HOURS_DAILY){
            ConstraintTeacherMinHoursDaily* crt_constraint=(ConstraintTeacherMinHoursDaily*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK){
            ConstraintTeacherMaxDaysPerWeek* crt_constraint=(ConstraintTeacherMaxDaysPerWeek*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK){
            ConstraintTeacherMinDaysPerWeek* crt_constraint=(ConstraintTeacherMinDaysPerWeek*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintTeacherIntervalMaxDaysPerWeek* crt_constraint=(ConstraintTeacherIntervalMaxDaysPerWeek*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintActivitiesPreferredTimeSlots* crt_constraint=(ConstraintActivitiesPreferredTimeSlots*)ctr;
            if(initialTeacherName == crt_constraint->p_teacherName)
                crt_constraint->p_teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintActivitiesPreferredStartingTimes* crt_constraint=(ConstraintActivitiesPreferredStartingTimes*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            ConstraintActivitiesEndStudentsDay* crt_constraint=(ConstraintActivitiesEndStudentsDay*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintSubactivitiesPreferredTimeSlots* crt_constraint=(ConstraintSubactivitiesPreferredTimeSlots*)ctr;
            if(initialTeacherName == crt_constraint->p_teacherName)
                crt_constraint->p_teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintSubactivitiesPreferredStartingTimes* crt_constraint=(ConstraintSubactivitiesPreferredStartingTimes*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        //2017-02-07
        else if(ctr->type==CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY){
            ConstraintTeacherMaxSpanPerDay* crt_constraint=(ConstraintTeacherMaxSpanPerDay*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_MIN_RESTING_HOURS){
            ConstraintTeacherMinRestingHours* crt_constraint=(ConstraintTeacherMinRestingHours*)ctr;
            if(initialTeacherName == crt_constraint->teacherName)
                crt_constraint->teacherName=finalTeacherName;
        }
    }

    int t=0;
    for(int i=0; i<this->teachersList.size(); i++){
        Teacher* tch=this->teachersList[i];

        if(tch->name==initialTeacherName){
            tch->name=finalTeacherName;
            t++;
        }
    }
    assert(t==1);

    if(tnatHash.contains(initialTeacherName)){
        QSet<ConstraintTeacherNotAvailableTimes*> cs=tnatHash.value(initialTeacherName);
        tnatHash.remove(initialTeacherName);
        assert(!tnatHash.contains(finalTeacherName));
        tnatHash.insert(finalTeacherName, cs);
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

void Instance::sortTeachersAlphabetically()
{
    std::stable_sort(this->teachersList.begin(), this->teachersList.end(), teachersAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addSubject(Subject* subject)
{
    for(int i=0; i<this->subjectsList.size(); i++){
        Subject* sbj=this->subjectsList[i];
        if(sbj->name==subject->name)
            return false;
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    for (auto teacher: teachersList){
        teacher_subject_preference_map[teacher].insert(subject, Enums::SubjectPreference::Indifferent);
    }

    this->subjectsList << subject;
    return true;
}

bool Instance::addSubjectFast(Subject* subject)
{
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    this->subjectsList << subject;
    return true;
}

int Instance::searchSubject(const QString& subjectName)
{
    for(int i=0; i<this->subjectsList.size(); i++)
        if(this->subjectsList.at(i)->name == subjectName)
            return i;

    return -1;
}

bool Instance::removeSubject(const QString& subjectName)
{
    //check the qualified subjects for teachers
    for(Teacher* tch: teachersList){
        assert(teacher_subject_preference_map.contains(tch));
        for(Subject* sbj: subjectsList){
            assert(teacher_subject_preference_map[tch].contains(sbj));
            if (sbj->name == subjectName) {
                teacher_subject_preference_map[tch].remove(sbj);
            }
        }
    }

    QList<int> idsToBeRemoved;
    foreach(Activity* act, activitiesList){
        if(act->subjectName==subjectName)
            idsToBeRemoved.append(act->id);
    }
    removeActivities(idsToBeRemoved, false);

    //remove the subject from the list
    for(int i=0; i<this->subjectsList.size(); i++)
        if(this->subjectsList[i]->name==subjectName){
            Subject* sbj=this->subjectsList[i];
            this->subjectsList.removeAt(i);
            delete sbj;
            break;
        }

    updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    return true;
}

bool Instance::modifySubject(const QString& initialSubjectName, const QString& finalSubjectName)
{
    assert(this->searchSubject(finalSubjectName)==-1);
    assert(this->searchSubject(initialSubjectName)>=0);

//    //check the qualified subjects for teachers
//    foreach(Teacher* tch, teachersList){
//        if(tch->subject_preference_map.contains(initialSubjectName)){

//            Enums::SubjectPreference pref = tch->subject_preference_map[initialSubjectName];

//            tch->subject_preference_map.remove(initialSubjectName);
//            tch->subject_preference_map.insert(finalSubjectName, pref);
//        }
//    }

    //check the activities
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];

        if( act->subjectName == initialSubjectName)
            act->subjectName=finalSubjectName;
    }

    //modify the time constraints related to this subject
    foreach(TimeConstraint* ctr, timeConstraintsList){
        if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintActivitiesPreferredTimeSlots* crt_constraint=(ConstraintActivitiesPreferredTimeSlots*)ctr;
            if(initialSubjectName == crt_constraint->p_subjectName)
                crt_constraint->p_subjectName=finalSubjectName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintActivitiesPreferredStartingTimes* crt_constraint=(ConstraintActivitiesPreferredStartingTimes*)ctr;
            if(initialSubjectName == crt_constraint->subjectName)
                crt_constraint->subjectName=finalSubjectName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            ConstraintActivitiesEndStudentsDay* crt_constraint=(ConstraintActivitiesEndStudentsDay*)ctr;
            if(initialSubjectName == crt_constraint->subjectName)
                crt_constraint->subjectName=finalSubjectName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintSubactivitiesPreferredTimeSlots* crt_constraint=(ConstraintSubactivitiesPreferredTimeSlots*)ctr;
            if(initialSubjectName == crt_constraint->p_subjectName)
                crt_constraint->p_subjectName=finalSubjectName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintSubactivitiesPreferredStartingTimes* crt_constraint=(ConstraintSubactivitiesPreferredStartingTimes*)ctr;
            if(initialSubjectName == crt_constraint->subjectName)
                crt_constraint->subjectName=finalSubjectName;
        }
    }

    //rename the subject in the list
    int t=0;
    for(int i=0; i<this->subjectsList.size(); i++){
        Subject* sbj=this->subjectsList[i];

        if(sbj->name==initialSubjectName){
            t++;
            sbj->name=finalSubjectName;
        }
    }
    assert(t==1);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

void Instance::sortSubjectsAlphabetically()
{
    std::stable_sort(this->subjectsList.begin(), this->subjectsList.end(), subjectsAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addActivityTag(ActivityTag* activityTag)
{
    for(int i=0; i<this->activityTagsList.size(); i++){
        ActivityTag* sbt=this->activityTagsList[i];

        if(sbt->name==activityTag->name)
            return false;
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    this->activityTagsList << activityTag;
    return true;
}

bool Instance::addActivityTagFast(ActivityTag* activityTag)
{
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    this->activityTagsList << activityTag;
    return true;
}


int Instance::searchActivityTag(const QString& activityTagName)
{
    for(int i=0; i<this->activityTagsList.size(); i++)
        if(this->activityTagsList.at(i)->name==activityTagName)
            return i;

    return -1;
}

bool Instance::removeActivityTag(const QString& activityTagName)
{
    foreach(Activity* act, activitiesList)
        if(act->activityTagsNames.contains(activityTagName))
            act->activityTagsNames.removeAll(activityTagName);

    //remove the activity tag from the list
    for(int i=0; i<this->activityTagsList.size(); i++)
        if(this->activityTagsList[i]->name==activityTagName){
            ActivityTag* sbt=this->activityTagsList[i];
            this->activityTagsList.removeAt(i);
            delete sbt;
            break;
        }

    updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    TContext::get()->resetSchedule();

    return true;
}

bool Instance::modifyActivityTag(const QString& initialActivityTagName, const QString& finalActivityTagName)
{
    assert(this->searchActivityTag(finalActivityTagName)==-1);
    assert(this->searchActivityTag(initialActivityTagName)>=0);

    //check the activities first
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];

        for(int kk=0; kk<act->activityTagsNames.count(); kk++)
            if(act->activityTagsNames.at(kk)==initialActivityTagName)
                act->activityTagsNames[kk]=finalActivityTagName;
    }

    //modify the constraints related to this activity tag
    foreach(TimeConstraint* ctr, timeConstraintsList){
        if(ctr->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherActivityTagMaxHoursContinuously* crt_constraint=(ConstraintTeacherActivityTagMaxHoursContinuously*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintTeacherActivityTagMaxHoursDaily* crt_constraint=(ConstraintTeacherActivityTagMaxHoursDaily*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeachersActivityTagMaxHoursContinuously* crt_constraint=(ConstraintTeachersActivityTagMaxHoursContinuously*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintTeachersActivityTagMaxHoursDaily* crt_constraint=(ConstraintTeachersActivityTagMaxHoursDaily*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsActivityTagMaxHoursContinuously* crt_constraint=(ConstraintStudentsActivityTagMaxHoursContinuously*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintStudentsActivityTagMaxHoursDaily* crt_constraint=(ConstraintStudentsActivityTagMaxHoursDaily*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetActivityTagMaxHoursContinuously* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursContinuously*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintStudentsSetActivityTagMaxHoursDaily* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursDaily*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintActivitiesPreferredTimeSlots* crt_constraint=(ConstraintActivitiesPreferredTimeSlots*)ctr;
            if(initialActivityTagName == crt_constraint->p_activityTagName)
                crt_constraint->p_activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintActivitiesPreferredStartingTimes* crt_constraint=(ConstraintActivitiesPreferredStartingTimes*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            ConstraintActivitiesEndStudentsDay* crt_constraint=(ConstraintActivitiesEndStudentsDay*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintSubactivitiesPreferredTimeSlots* crt_constraint=(ConstraintSubactivitiesPreferredTimeSlots*)ctr;
            if(initialActivityTagName == crt_constraint->p_activityTagName)
                crt_constraint->p_activityTagName=finalActivityTagName;
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintSubactivitiesPreferredStartingTimes* crt_constraint=(ConstraintSubactivitiesPreferredStartingTimes*)ctr;
            if(initialActivityTagName == crt_constraint->activityTagName)
                crt_constraint->activityTagName=finalActivityTagName;
        }
    }

    //TODO: space
    //modify the space constraints related to this subject tag
//    foreach(SpaceConstraint* ctr, spaceConstraintsList){
//        if(ctr->type==CONSTRAINT_SUBJECT_ACTIVITY_TAG_PREFERRED_ROOM){
//            ConstraintSubjectActivityTagPreferredRoom* c=(ConstraintSubjectActivityTagPreferredRoom*)ctr;
//            if(c->activityTagName == initialActivityTagName)
//                c->activityTagName=finalActivityTagName;
//        }
//        else if(ctr->type==CONSTRAINT_SUBJECT_ACTIVITY_TAG_PREFERRED_ROOMS){
//            ConstraintSubjectActivityTagPreferredRooms* c=(ConstraintSubjectActivityTagPreferredRooms*)ctr;
//            if(c->activityTagName == initialActivityTagName)
//                c->activityTagName=finalActivityTagName;
//        }
//        else if(ctr->type==CONSTRAINT_ACTIVITY_TAG_PREFERRED_ROOM){
//            ConstraintActivityTagPreferredRoom* c=(ConstraintActivityTagPreferredRoom*)ctr;
//            if(c->activityTagName == initialActivityTagName)
//                c->activityTagName=finalActivityTagName;
//        }
//        else if(ctr->type==CONSTRAINT_ACTIVITY_TAG_PREFERRED_ROOMS){
//            ConstraintActivityTagPreferredRooms* c=(ConstraintActivityTagPreferredRooms*)ctr;
//            if(c->activityTagName == initialActivityTagName)
//                c->activityTagName=finalActivityTagName;
//        }
//    }

    //rename the activity tag in the list
    int t=0;

    for(int i=0; i<this->activityTagsList.size(); i++){
        ActivityTag* sbt=this->activityTagsList[i];

        if(sbt->name==initialActivityTagName){
            t++;
            sbt->name=finalActivityTagName;
        }
    }

    assert(t==1);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

void Instance::sortActivityTagsAlphabetically()
{
    std::stable_sort(this->activityTagsList.begin(), this->activityTagsList.end(), activityTagsAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::setsShareStudents(const QString& studentsSet1, const QString& studentsSet2)
{
    StudentsSet* s1=this->searchStudentsSet(studentsSet1);
    StudentsSet* s2=this->searchStudentsSet(studentsSet2);
    assert(s1!=nullptr);
    assert(s2!=nullptr);

    QSet<QString> downwardSets1;

    if(s1->type==STUDENTS_YEAR){
        StudentsYear* year1=(StudentsYear*)s1;
        downwardSets1.insert(year1->name);
        foreach(StudentsGroup* group1, year1->groupsList){
            downwardSets1.insert(group1->name);
            foreach(StudentsSubgroup* subgroup1, group1->subgroupsList)
                downwardSets1.insert(subgroup1->name);
        }
    }
    else if(s1->type==STUDENTS_GROUP){
        StudentsGroup* group1=(StudentsGroup*)s1;
        downwardSets1.insert(group1->name);
        foreach(StudentsSubgroup* subgroup1, group1->subgroupsList)
            downwardSets1.insert(subgroup1->name);
    }
    else if(s1->type==STUDENTS_SUBGROUP){
        StudentsSubgroup* subgroup1=(StudentsSubgroup*)s1;
        downwardSets1.insert(subgroup1->name);
    }
    else
        assert(0);

    if(s2->type==STUDENTS_YEAR){
        StudentsYear* year2=(StudentsYear*)s2;
        if(downwardSets1.contains(year2->name))
            return true;
        foreach(StudentsGroup* group2, year2->groupsList){
            if(downwardSets1.contains(group2->name))
                return true;
            foreach(StudentsSubgroup* subgroup2, group2->subgroupsList)
                if(downwardSets1.contains(subgroup2->name))
                    return true;
        }
    }
    else if(s2->type==STUDENTS_GROUP){
        StudentsGroup* group2=(StudentsGroup*)s2;
        if(downwardSets1.contains(group2->name))
            return true;
        foreach(StudentsSubgroup* subgroup2, group2->subgroupsList)
            if(downwardSets1.contains(subgroup2->name))
                return true;
    }
    else if(s2->type==STUDENTS_SUBGROUP){
        StudentsSubgroup* subgroup2=(StudentsSubgroup*)s2;
        if(downwardSets1.contains(subgroup2->name))
            return true;
    }
    else
        assert(0);

    return false;

}

void Instance::computePermanentStudentsHash()
{
    //The commented tests are good, but bring a somewhat slowdown.
    permanentStudentsHash.clear();

    foreach(StudentsYear* year, yearsList){
        assert(!permanentStudentsHash.contains(year->name));
        permanentStudentsHash.insert(year->name, year);

        //QSet<QString> groupsInYear;

        foreach(StudentsGroup* group, year->groupsList){
            //assert(!groupsInYear.contains(group->name));
            //groupsInYear.insert(group->name);

            if(!permanentStudentsHash.contains(group->name))
                permanentStudentsHash.insert(group->name, group);
            else
                assert(permanentStudentsHash.value(group->name)==group);

            //QSet<QString> subgroupsInGroup;

            foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                //assert(!subgroupsInGroup.contains(subgroup->name));
                //subgroupsInGroup.insert(subgroup->name);

                if(!permanentStudentsHash.contains(subgroup->name))
                    permanentStudentsHash.insert(subgroup->name, subgroup);
                else
                    assert(permanentStudentsHash.value(subgroup->name)==subgroup);
            }
        }
    }
}

StudentsSet* Instance::searchStudentsSet(const QString& setName)
{
    return permanentStudentsHash.value(setName, nullptr);

    /*for(int i=0; i<this->yearsList.size(); i++){
                StudentsYear* sty=this->yearsList[i];
                if(sty->name==setName)
                        return sty;
                for(int j=0; j<sty->groupsList.size(); j++){
                        StudentsGroup* stg=sty->groupsList[j];
                        if(stg->name==setName)
                                return stg;
                        for(int k=0; k<stg->subgroupsList.size(); k++){
                                StudentsSubgroup* sts=stg->subgroupsList[k];
                                if(sts->name==setName)
                                        return sts;
                        }
                }
        }
        return nullptr;*/
}

StudentsSet* Instance::searchAugmentedStudentsSet(const QString& setName)
{
    for(int i=0; i<this->augmentedYearsList.size(); i++){
        StudentsYear* sty=this->augmentedYearsList[i];
        if(sty->name==setName)
            return sty;
        for(int j=0; j<sty->groupsList.size(); j++){
            StudentsGroup* stg=sty->groupsList[j];
            if(stg->name==setName)
                return stg;
            for(int k=0; k<stg->subgroupsList.size(); k++){
                StudentsSubgroup* sts=stg->subgroupsList[k];
                if(sts->name==setName)
                    return sts;
            }
        }
    }
    return nullptr;
}

bool Instance::addYear(StudentsYear* year)
{
    //already existing?
    foreach(StudentsYear* ty, yearsList)
        if(ty->name==year->name)
            return false;
    //if(this->searchStudentsSet(year->name)!=nullptr)
    //	return false;
    this->yearsList << year;

    assert(!permanentStudentsHash.contains(year->name));
    permanentStudentsHash.insert(year->name, year);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::addYearFast(StudentsYear* year)
{
    this->yearsList << year;
    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

/*bool Instance::removeYear(const QString& yearName)
{
        return removeYear(yearName, true);
}

bool Instance::emptyYear(const QString& yearName)
{
        return removeYear(yearName, false);
}*/

bool Instance::removeYear(const QString& yearName/*, bool removeAlsoThisYear*/)
{
    const bool removeAlsoThisYear=true;

    StudentsYear* yearPointer=nullptr;
    foreach(StudentsYear* ty, this->yearsList){
        if(ty->name==yearName){
            yearPointer=ty;
            break;
        }
    }

    assert(yearPointer!=nullptr);

    //pointers
    QSet<StudentsSet*> tmpSet;
    foreach(StudentsYear* year, yearsList)
        if(year->name!=yearName){
            tmpSet.insert(year);
            foreach(StudentsGroup* group, year->groupsList){
                tmpSet.insert(group);
                foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                    tmpSet.insert(subgroup);
            }
        }

    QSet<StudentsSet*> toBeRemoved;
    if(removeAlsoThisYear)
        toBeRemoved.insert(yearPointer);
    foreach(StudentsGroup* group, yearPointer->groupsList){
        assert(!toBeRemoved.contains(group));
        if(!tmpSet.contains(group))
            toBeRemoved.insert(group);
        foreach(StudentsSubgroup* subgroup, group->subgroupsList){
            //assert(!toBeRemoved.contains(subgroup));
            if(!tmpSet.contains(subgroup) && !toBeRemoved.contains(subgroup))
                toBeRemoved.insert(subgroup);
        }
    }

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    if(removeAlsoThisYear){
        for(int i=0; i<yearsList.count(); i++)
            if(yearsList.at(i)==yearPointer){
                yearsList.removeAt(i);
                break;
            }
    }
    else{
        yearPointer->groupsList.clear();
    }

    foreach(StudentsSet* studentsSet, toBeRemoved){
        assert(permanentStudentsHash.contains(studentsSet->name));
        permanentStudentsHash.remove(studentsSet->name);

        delete studentsSet;
    }

    if(toBeRemoved.count()>0)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::removeYearPointerAfterSplit(StudentsYear* yearPointer)
{
    assert(yearPointer!=nullptr);

    //names
    /*QSet<QString> tmpSet;
        foreach(StudentsYear* year, yearsList){
                tmpSet.insert(year->name);
                foreach(StudentsGroup* group, year->groupsList){
                        tmpSet.insert(group->name);
                        foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                                tmpSet.insert(subgroup->name);
                }
        }*/

    QSet<StudentsSet*> toBeRemoved;
    //Not here, because there exists another pointer with the same name (to the new year),
    //and I don't want to remove the activities with this year name
    //toBeRemoved.insert(yearPointer);
    foreach(StudentsGroup* group, yearPointer->groupsList){
        assert(!toBeRemoved.contains(group));
        if(!permanentStudentsHash.contains(group->name))
            toBeRemoved.insert(group);
        foreach(StudentsSubgroup* subgroup, group->subgroupsList){
            //assert(!toBeRemoved.contains(subgroup));
            if(!permanentStudentsHash.contains(subgroup->name) && !toBeRemoved.contains(subgroup))
                toBeRemoved.insert(subgroup);
        }
    }

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    toBeRemoved.insert(yearPointer);
    foreach(StudentsSet* studentsSet, toBeRemoved)
        delete studentsSet;

    if(toBeRemoved.count()>1)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

int Instance::searchYear(const QString& yearName)
{
    for(int i=0; i<this->yearsList.size(); i++)
        if(this->yearsList[i]->name==yearName)
            return i;

    return -1;
}

int Instance::searchAugmentedYear(const QString& yearName)
{
    for(int i=0; i<this->augmentedYearsList.size(); i++)
        if(this->augmentedYearsList[i]->name==yearName)
            return i;

    return -1;
}

bool Instance::modifyStudentsSet(const QString& initialStudentsSetName, const QString& finalStudentsSetName, int finalNumberOfStudents)
{
    StudentsSet* studentsSet=searchStudentsSet(initialStudentsSetName);
    assert(studentsSet!=nullptr);
    if(initialStudentsSetName!=finalStudentsSetName)
        assert(searchStudentsSet(finalStudentsSetName)==nullptr);
    int initialNumberOfStudents=studentsSet->numberOfStudents;

    foreach(Activity* act, activitiesList)
        act->renameStudents(*this, initialStudentsSetName, finalStudentsSetName, initialNumberOfStudents, finalNumberOfStudents);

    if(initialStudentsSetName!=finalStudentsSetName){
        foreach(TimeConstraint* ctr, timeConstraintsList){
            if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
                ConstraintStudentsSetNotAvailableTimes* crt_constraint=(ConstraintStudentsSetNotAvailableTimes*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY){
                ConstraintStudentsSetMaxHoursDaily* crt_constraint=(ConstraintStudentsSetMaxHoursDaily*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK){
                ConstraintStudentsSetMaxDaysPerWeek* crt_constraint=(ConstraintStudentsSetMaxDaysPerWeek*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK){
                ConstraintStudentsSetIntervalMaxDaysPerWeek* crt_constraint=(ConstraintStudentsSetIntervalMaxDaysPerWeek*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY){
                ConstraintStudentsSetMaxHoursContinuously* crt_constraint=(ConstraintStudentsSetMaxHoursContinuously*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
                ConstraintStudentsSetActivityTagMaxHoursContinuously* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursContinuously*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY){
                ConstraintStudentsSetActivityTagMaxHoursDaily* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursDaily*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY){
                ConstraintStudentsSetMinHoursDaily* crt_constraint=(ConstraintStudentsSetMinHoursDaily*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR){
                ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* crt_constraint=(ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK){
                ConstraintStudentsSetMaxGapsPerWeek* crt_constraint=(ConstraintStudentsSetMaxGapsPerWeek*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY){
                ConstraintStudentsSetMaxGapsPerDay* crt_constraint=(ConstraintStudentsSetMaxGapsPerDay*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
                ConstraintActivitiesPreferredTimeSlots* crt_constraint=(ConstraintActivitiesPreferredTimeSlots*)ctr;
                if(initialStudentsSetName == crt_constraint->p_studentsName)
                    crt_constraint->p_studentsName=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
                ConstraintActivitiesPreferredStartingTimes* crt_constraint=(ConstraintActivitiesPreferredStartingTimes*)ctr;
                if(initialStudentsSetName == crt_constraint->studentsName)
                    crt_constraint->studentsName=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
                ConstraintActivitiesEndStudentsDay* crt_constraint=(ConstraintActivitiesEndStudentsDay*)ctr;
                if(initialStudentsSetName == crt_constraint->studentsName)
                    crt_constraint->studentsName=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
                ConstraintSubactivitiesPreferredTimeSlots* crt_constraint=(ConstraintSubactivitiesPreferredTimeSlots*)ctr;
                if(initialStudentsSetName == crt_constraint->p_studentsName)
                    crt_constraint->p_studentsName=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
                ConstraintSubactivitiesPreferredStartingTimes* crt_constraint=(ConstraintSubactivitiesPreferredStartingTimes*)ctr;
                if(initialStudentsSetName == crt_constraint->studentsName)
                    crt_constraint->studentsName=finalStudentsSetName;
            }
            //2017-02-07
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY){
                ConstraintStudentsSetMaxSpanPerDay* crt_constraint=(ConstraintStudentsSetMaxSpanPerDay*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
            else if(ctr->type==CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS){
                ConstraintStudentsSetMinRestingHours* crt_constraint=(ConstraintStudentsSetMinRestingHours*)ctr;
                if(initialStudentsSetName == crt_constraint->students)
                    crt_constraint->students=finalStudentsSetName;
            }
        }
    }

    assert(studentsSet->name==initialStudentsSetName);
    assert(studentsSet->numberOfStudents==initialNumberOfStudents);
    studentsSet->name=finalStudentsSetName;
    studentsSet->numberOfStudents=finalNumberOfStudents;

    assert(permanentStudentsHash.contains(initialStudentsSetName));
    if(initialStudentsSetName!=finalStudentsSetName){
        permanentStudentsHash.remove(initialStudentsSetName);
        permanentStudentsHash.insert(studentsSet->name, studentsSet);
    }

    if(initialStudentsSetName!=finalStudentsSetName){
        if(ssnatHash.contains(initialStudentsSetName)){
            QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(initialStudentsSetName);
            ssnatHash.remove(initialStudentsSetName);
            assert(!ssnatHash.contains(finalStudentsSetName));
            ssnatHash.insert(finalStudentsSetName, cs);
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

//by Volker Dirr (start) - very similar to Liviu's modifyStudentsSet
bool Instance::modifyStudentsSets(const QHash<QString, QString>& oldAndNewStudentsSetNames){
    if(oldAndNewStudentsSetNames.isEmpty())
        return true;

    foreach(Activity* act, activitiesList){
        for(int i=0; i<act->studentSetsNames.count(); i++)
            if(oldAndNewStudentsSetNames.contains(act->studentSetsNames.at(i)))
                act->studentSetsNames[i]=oldAndNewStudentsSetNames.value(act->studentSetsNames.at(i));
    }

    foreach(TimeConstraint* ctr, timeConstraintsList){
        if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
            ConstraintStudentsSetNotAvailableTimes* crt_constraint=(ConstraintStudentsSetNotAvailableTimes*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY){
            ConstraintStudentsSetMaxHoursDaily* crt_constraint=(ConstraintStudentsSetMaxHoursDaily*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetMaxDaysPerWeek* crt_constraint=(ConstraintStudentsSetMaxDaysPerWeek*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetIntervalMaxDaysPerWeek* crt_constraint=(ConstraintStudentsSetIntervalMaxDaysPerWeek*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetMaxHoursContinuously* crt_constraint=(ConstraintStudentsSetMaxHoursContinuously*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetActivityTagMaxHoursContinuously* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursContinuously*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintStudentsSetActivityTagMaxHoursDaily* crt_constraint=(ConstraintStudentsSetActivityTagMaxHoursDaily*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY){
            ConstraintStudentsSetMinHoursDaily* crt_constraint=(ConstraintStudentsSetMinHoursDaily*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR){
            ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* crt_constraint=(ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK){
            ConstraintStudentsSetMaxGapsPerWeek* crt_constraint=(ConstraintStudentsSetMaxGapsPerWeek*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY){
            ConstraintStudentsSetMaxGapsPerDay* crt_constraint=(ConstraintStudentsSetMaxGapsPerDay*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintActivitiesPreferredTimeSlots* crt_constraint=(ConstraintActivitiesPreferredTimeSlots*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->p_studentsName))
                crt_constraint->p_studentsName=oldAndNewStudentsSetNames.value(crt_constraint->p_studentsName);
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintActivitiesPreferredStartingTimes* crt_constraint=(ConstraintActivitiesPreferredStartingTimes*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->studentsName))
                crt_constraint->studentsName=oldAndNewStudentsSetNames.value(crt_constraint->studentsName);
        }
        else if(ctr->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            ConstraintActivitiesEndStudentsDay* crt_constraint=(ConstraintActivitiesEndStudentsDay*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->studentsName))
                crt_constraint->studentsName=oldAndNewStudentsSetNames.value(crt_constraint->studentsName);
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintSubactivitiesPreferredTimeSlots* crt_constraint=(ConstraintSubactivitiesPreferredTimeSlots*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->p_studentsName))
                crt_constraint->p_studentsName=oldAndNewStudentsSetNames.value(crt_constraint->p_studentsName);
        }
        else if(ctr->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintSubactivitiesPreferredStartingTimes* crt_constraint=(ConstraintSubactivitiesPreferredStartingTimes*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->studentsName))
                crt_constraint->studentsName=oldAndNewStudentsSetNames.value(crt_constraint->studentsName);
        }
        //2017-02-07
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY){
            ConstraintStudentsSetMaxSpanPerDay* crt_constraint=(ConstraintStudentsSetMaxSpanPerDay*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
        else if(ctr->type==CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS){
            ConstraintStudentsSetMinRestingHours* crt_constraint=(ConstraintStudentsSetMinRestingHours*)ctr;
            if(oldAndNewStudentsSetNames.contains(crt_constraint->students))
                crt_constraint->students=oldAndNewStudentsSetNames.value(crt_constraint->students);
        }
    }

    QHashIterator<QString, QString> i(oldAndNewStudentsSetNames);
    while (i.hasNext()) {
        i.next();
        StudentsSet* studentsSet=searchStudentsSet(i.key());
        assert(studentsSet!=nullptr);
        studentsSet->name=i.value();

        assert(permanentStudentsHash.contains(i.key()));
        permanentStudentsHash.remove(i.key());
        permanentStudentsHash.insert(studentsSet->name, studentsSet);

        assert(i.key()!=i.value());
        if(ssnatHash.contains(i.key())){
            QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(i.key());
            ssnatHash.remove(i.key());
            assert(!ssnatHash.contains(i.value()));
            ssnatHash.insert(i.value(), cs);
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}
//by Volker Dirr (end) - very similar to Liviu's modifyStudentsSet

void Instance::sortYearsAlphabetically()
{
    std::stable_sort(this->yearsList.begin(), this->yearsList.end(), yearsAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addGroup(const QString& yearName, StudentsGroup* group)
{
    StudentsYear* sty=nullptr;
    for(int i=0; i<this->yearsList.size(); i++){
        sty=yearsList[i];
        if(sty->name==yearName)
            break;
    }
    assert(sty);

    for(int i=0; i<sty->groupsList.size(); i++){
        StudentsGroup* stg=sty->groupsList[i];
        if(stg->name==group->name)
            return false;
    }

    sty->groupsList << group; //append

    if(!permanentStudentsHash.contains(group->name))
        permanentStudentsHash.insert(group->name, group);

    /*
        foreach(StudentsYear* y, yearsList)
                foreach(StudentsGroup* g, y->groupsList)
                        if(g->name==group->name)
                                g->numberOfStudents=group->numberOfStudents;*/

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::addGroupFast(StudentsYear* year, StudentsGroup* group)
{
    year->groupsList << group; //append

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::removeGroup(const QString& yearName, const QString& groupName)
{
    StudentsYear* yearPointer=nullptr;
    foreach(StudentsYear* ty, this->yearsList){
        if(ty->name==yearName){
            yearPointer=ty;
            break;
        }
    }

    assert(yearPointer!=nullptr);

    StudentsGroup* groupPointer=nullptr;
    foreach(StudentsGroup* tg, yearPointer->groupsList){
        if(tg->name==groupName){
            groupPointer=tg;
            break;
        }
    }

    assert(groupPointer!=nullptr);

    //pointers
    QSet<StudentsSet*> tmpSet;
    foreach(StudentsYear* year, yearsList){
        if(year->name!=yearName){
            //tmpSet.insert(year); useless
            foreach(StudentsGroup* group, year->groupsList){
                if(group->name==groupName) //we shall not purge groupName, because it still exists in the current year
                    tmpSet.insert(group);
                foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                    tmpSet.insert(subgroup);
            }
        }
        else{
            foreach(StudentsGroup* group, year->groupsList)
                if(group->name!=groupName){
                    //tmpSet.insert(group); //useless
                    foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                        tmpSet.insert(subgroup);
                }
        }
    }

    QSet<StudentsSet*> toBeRemoved;
    if(!tmpSet.contains(groupPointer))
        toBeRemoved.insert(groupPointer);
    foreach(StudentsSubgroup* subgroup, groupPointer->subgroupsList){
        assert(!toBeRemoved.contains(subgroup));
        if(!tmpSet.contains(subgroup))
            toBeRemoved.insert(subgroup);
    }

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    for(int i=0; i<yearPointer->groupsList.count(); i++)
        if(yearPointer->groupsList.at(i)==groupPointer){
            yearPointer->groupsList.removeAt(i);
            break;
        }

    foreach(StudentsSet* studentsSet, toBeRemoved){
        assert(permanentStudentsHash.contains(studentsSet->name));
        permanentStudentsHash.remove(studentsSet->name);

        delete studentsSet;
    }

    if(toBeRemoved.count()>0)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::purgeGroup(const QString& groupName)
{
    StudentsGroup* groupPointer=nullptr;
    foreach(StudentsYear* year, yearsList){
        int j=-1;
        for(int i=0; i<year->groupsList.count(); i++){
            if(year->groupsList.at(i)->name==groupName){
                j=i;
                if(groupPointer==nullptr)
                    groupPointer=year->groupsList.at(i);
                else
                    assert(groupPointer==year->groupsList.at(i));
                break;
            }
        }
        if(j>=0)
            year->groupsList.removeAt(j);
    }

    assert(groupPointer!=nullptr);

    //pointers
    QSet<StudentsSet*> tmpSet;
    foreach(StudentsYear* year, yearsList)
        foreach(StudentsGroup* group, year->groupsList)
            foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                tmpSet.insert(subgroup);

    QSet<StudentsSet*> toBeRemoved;
    if(!tmpSet.contains(groupPointer))
        toBeRemoved.insert(groupPointer);
    else
        assert(0);
    foreach(StudentsSubgroup* subgroup, groupPointer->subgroupsList){
        assert(!toBeRemoved.contains(subgroup));
        if(!tmpSet.contains(subgroup))
            toBeRemoved.insert(subgroup);
    }

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    foreach(StudentsSet* studentsSet, toBeRemoved){
        assert(permanentStudentsHash.contains(studentsSet->name));
        permanentStudentsHash.remove(studentsSet->name);

        delete studentsSet;
    }

    if(toBeRemoved.count()>0)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

int Instance::searchGroup(const QString& yearName, const QString& groupName)
{
    StudentsYear* sty=nullptr;
    foreach(StudentsYear* ty, yearsList)
        if(ty->name==yearName){
            sty=ty;
            break;
        }
    assert(sty!=nullptr);

    for(int i=0; i<sty->groupsList.size(); i++)
        if(sty->groupsList[i]->name==groupName)
            return i;

    return -1;
}

int Instance::searchAugmentedGroup(const QString& yearName, const QString& groupName)
{
    StudentsYear* sty=nullptr;
    foreach(StudentsYear* ty, augmentedYearsList)
        if(ty->name==yearName){
            sty=ty;
            break;
        }
    assert(sty!=nullptr);

    for(int i=0; i<sty->groupsList.size(); i++)
        if(sty->groupsList[i]->name==groupName)
            return i;

    return -1;
}

void Instance::sortGroupsAlphabetically(const QString& yearName)
{
    StudentsYear* sty=this->yearsList[this->searchYear(yearName)];
    assert(sty);

    std::stable_sort(sty->groupsList.begin(), sty->groupsList.end(), groupsAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addSubgroup(const QString& yearName, const QString& groupName, StudentsSubgroup* subgroup)
{
    StudentsYear* sty=this->yearsList.at(this->searchYear(yearName));
    assert(sty);
    StudentsGroup* stg=sty->groupsList.at(this->searchGroup(yearName, groupName));
    assert(stg);

    for(int i=0; i<stg->subgroupsList.size(); i++){
        StudentsSubgroup* sts=stg->subgroupsList[i];
        if(sts->name==subgroup->name)
            return false;
    }

    stg->subgroupsList << subgroup; //append

    if(!permanentStudentsHash.contains(subgroup->name))
        permanentStudentsHash.insert(subgroup->name, subgroup);

    /*
        foreach(StudentsYear* y, yearsList)
                foreach(StudentsGroup* g, y->groupsList)
                        foreach(StudentsSubgroup* s, g->subgroupsList)
                                if(s->name==subgroup->name)
                                        s->numberOfStudents=subgroup->numberOfStudents;*/

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::addSubgroupFast(StudentsYear* year, StudentsGroup* group, StudentsSubgroup* subgroup)
{
    Q_UNUSED(year);

    group->subgroupsList << subgroup; //append

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::removeSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName)
{
    StudentsYear* yearPointer=nullptr;
    foreach(StudentsYear* ty, this->yearsList){
        if(ty->name==yearName){
            yearPointer=ty;
            break;
        }
    }

    assert(yearPointer!=nullptr);

    StudentsGroup* groupPointer=nullptr;
    foreach(StudentsGroup* tg, yearPointer->groupsList){
        if(tg->name==groupName){
            groupPointer=tg;
            break;
        }
    }

    assert(groupPointer!=nullptr);

    StudentsSubgroup* subgroupPointer=nullptr;
    foreach(StudentsSubgroup* ts, groupPointer->subgroupsList){
        if(ts->name==subgroupName){
            subgroupPointer=ts;
            break;
        }
    }

    assert(subgroupPointer!=nullptr);

    //pointers
    QSet<StudentsSet*> toBeRemoved;
    toBeRemoved.insert(subgroupPointer);
    foreach(StudentsYear* year, yearsList)
        foreach(StudentsGroup* group, year->groupsList)
            foreach(StudentsSubgroup* subgroup, group->subgroupsList)
                if(subgroup->name==subgroupName && (year->name!=yearName || group->name!=groupName))
                    toBeRemoved.remove(subgroupPointer);

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    for(int i=0; i<groupPointer->subgroupsList.count(); i++)
        if(groupPointer->subgroupsList.at(i)==subgroupPointer){
            groupPointer->subgroupsList.removeAt(i);
            break;
        }

    foreach(StudentsSet* studentsSet, toBeRemoved){
        assert(permanentStudentsHash.contains(studentsSet->name));
        permanentStudentsHash.remove(studentsSet->name);

        delete studentsSet;
    }

    if(toBeRemoved.count()>0)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

bool Instance::purgeSubgroup(const QString& subgroupName)
{
    StudentsSubgroup* subgroupPointer=nullptr;
    foreach(StudentsYear* year, yearsList)
        foreach(StudentsGroup* group, year->groupsList){
            int j=-1;
            for(int i=0; i<group->subgroupsList.count(); i++){
                if(group->subgroupsList.at(i)->name==subgroupName){
                    j=i;
                    if(subgroupPointer==nullptr)
                        subgroupPointer=group->subgroupsList.at(i);
                    else
                        assert(subgroupPointer==group->subgroupsList.at(i));
                    break;
                }
            }
            if(j>=0)
                group->subgroupsList.removeAt(j);
        }

    assert(subgroupPointer!=nullptr);

    //pointers
    QSet<StudentsSet*> toBeRemoved;
    toBeRemoved.insert(subgroupPointer);

    updateActivitiesWhenRemovingStudents(toBeRemoved, false);

    foreach(StudentsSet* studentsSet, toBeRemoved){
        assert(permanentStudentsHash.contains(studentsSet->name));
        permanentStudentsHash.remove(studentsSet->name);

        delete studentsSet;
    }

    if(toBeRemoved.count()>0)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
    return true;
}

int Instance::searchSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName)
{
    StudentsYear* sty=nullptr;
    foreach(StudentsYear* ty, yearsList)
        if(ty->name==yearName){
            sty=ty;
            break;
        }
    assert(sty!=nullptr);

    StudentsGroup* stg=nullptr;
    foreach(StudentsGroup* tg, sty->groupsList)
        if(tg->name==groupName){
            stg=tg;
            break;
        }
    assert(stg!=nullptr);

    for(int i=0; i<stg->subgroupsList.size(); i++)
        if(stg->subgroupsList[i]->name==subgroupName)
            return i;

    return -1;
}

int Instance::searchAugmentedSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName)
{
    StudentsYear* sty=nullptr;
    foreach(StudentsYear* ty, augmentedYearsList)
        if(ty->name==yearName){
            sty=ty;
            break;
        }
    assert(sty!=nullptr);

    StudentsGroup* stg=nullptr;
    foreach(StudentsGroup* tg, sty->groupsList)
        if(tg->name==groupName){
            stg=tg;
            break;
        }
    assert(stg!=nullptr);

    for(int i=0; i<stg->subgroupsList.size(); i++)
        if(stg->subgroupsList[i]->name==subgroupName)
            return i;

    return -1;
}

void Instance::sortSubgroupsAlphabetically(const QString& yearName, const QString& groupName)
{
    StudentsYear* sty=this->yearsList.at(this->searchYear(yearName));
    assert(sty);
    StudentsGroup* stg=sty->groupsList.at(this->searchGroup(yearName, groupName));
    assert(stg);

    std::stable_sort(stg->subgroupsList.begin(), stg->subgroupsList.end(), subgroupsAscending);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addSimpleActivityFast(QWidget* parent,
                                     int _id,
                                     int _activityGroupId,
                                     const QString& _activityName,
                                     bool _allowMultipleTeachers,
                                     const QString& _subjectName,
                                     const QStringList& _activityTagsNames,
                                     const QStringList& _studentsName,
                                     int _duration,
                                     int _totalDuration,
                                     bool _active,
                                     bool _computeNTotalStudents,
                                     int _nTotalStudents,
                                     int _computedNumberOfStudents)
{
    //check for duplicates - idea and code by Volker Dirr
    int t=QStringList(_studentsName).removeDuplicates();
    if(t>0)
        MessagesManager::warning(parent, tr("m-FET warning"), tr("Activity with Id=%1 contains %2 duplicate students sets - please correct that")
                                          .arg(_id).arg(t));

    t=QStringList(_activityTagsNames).removeDuplicates();
    if(t>0)
        MessagesManager::warning(parent, tr("m-FET warning"), tr("Activity with Id=%1 contains %2 duplicate activity tags - please correct that")
                                          .arg(_id).arg(t));

    Activity *act=new Activity(*this, _id, _activityGroupId, _activityName, _allowMultipleTeachers, _subjectName, _activityTagsNames,
                               _studentsName, _duration, _totalDuration, _active, _computeNTotalStudents, _nTotalStudents, _computedNumberOfStudents);

    this->addActivity(act);

    if (_activityName == "") {
        renameAllActivitiesForSubject(_subjectName);
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

bool Instance::addSplitActivityFast(QWidget* parent,
                                    int _firstActivityId,
                                    int _activityGroupId,
                                    const QString& _activityName,
                                    bool _allowMultipleTeachers,
                                    const QString& _subjectName,
                                    const QStringList& _activityTagsNames,
                                    const QStringList& _studentsName,
                                    int _nSplits,
                                    int _totalDuration,
                                    int _durations[],
                                    bool _active[],
                                    int _minDayDistance,
                                    Enums::ConstraintGroup _group,
                                    double _weightPercentage,
                                    bool _consecutiveIfSameDay,
                                    bool _computeNTotalStudents,
                                    int _nTotalStudents,
                                    int _computedNumberOfStudents)
{
    //check for duplicates - idea and code by Volker Dirr
    int t=QStringList(_studentsName).removeDuplicates();
    if(t>0)
        MessagesManager::warning(parent, tr("m-FET warning"), tr("Activities with group_Id=%1 contain %2 duplicate students sets - please correct that")
                                          .arg(_activityGroupId).arg(t));

    t=QStringList(_activityTagsNames).removeDuplicates();
    if(t>0)
        MessagesManager::warning(parent, tr("m-FET warning"), tr("Activities with group_Id=%1 contain %2 duplicate activity tags - please correct that")
                                          .arg(_activityGroupId).arg(t));

    assert(_firstActivityId==_activityGroupId);

    QList<int> acts;

    acts.clear();
    for(int i=0; i<_nSplits; i++){
        Activity *act=new Activity(*this, _firstActivityId+i, _activityGroupId,
                                   _activityName, _allowMultipleTeachers, _subjectName, _activityTagsNames, _studentsName,
                                   _durations[i], _totalDuration, _active[i], _computeNTotalStudents, _nTotalStudents, _computedNumberOfStudents);

        this->addActivity(act);

        acts.append(_firstActivityId+i);
    }

    if (_activityName == "") {
        renameAllActivitiesForSubject(_subjectName);
    }

    if(_minDayDistance>0){
        TimeConstraint *constr=new ConstraintMinDaysBetweenActivities(*this, _group, _weightPercentage, _consecutiveIfSameDay, _nSplits, acts, _minDayDistance);
        bool tmp=this->addTimeConstraint(constr);
        assert(tmp);
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return true;
}

bool Instance::addActivity(Activity* act)
{
    assert(!activitiesPointerHash.contains(act->id));

    activitiesList << act;

    activitiesPointerHash.insert(act->id, act);

    if (!groupActivitiesPointerHash.contains(act->activityGroupId))
        groupActivitiesPointerHash.insert(act->activityGroupId, new ActivitiesList());

    groupActivitiesPointerHash.value(act->activityGroupId)->append(act);

    emit gotModified();

    return true;
}

void Instance::removeActivity(int _id, int _activityGroupId)
{
    QList<int> tmpList;
    foreach(Activity* act, activitiesList)
        if(_id==act->id || (_activityGroupId>0 && _activityGroupId==act->activityGroupId))
            tmpList.append(act->id);
    removeActivities(tmpList, true);
}

void Instance::removeActivities()
{
    QList<int> _idsList;

    for (auto act: activitiesList) {
        _idsList.append(act->id);
    }

    removeActivities(_idsList, true);
}

void Instance::removeActivities(const QList<int>& _idsList, bool updateConstraints)
{
    if(_idsList.count()==0)
        return;

    QSet<int> _removedIdsSet=_idsList.toSet();

    QSet<int> _groupIdsSet;
    foreach(Activity* act, activitiesList)
        if(act->activityGroupId>0 && _removedIdsSet.contains(act->id))
            _groupIdsSet.insert(act->activityGroupId);
    foreach(Activity* act, activitiesList)
        if(act->activityGroupId>0 && _groupIdsSet.contains(act->activityGroupId))
            _removedIdsSet.insert(act->id);

    ActivitiesList newActivitiesList;
    ActivitiesList toBeRemoved;

    for(int i=0; i<activitiesList.count(); i++){
        int id=activitiesList.at(i)->id;
        if(!_removedIdsSet.contains(id)){
            newActivitiesList.append(activitiesList[i]);
        }
        else{
            toBeRemoved.append(activitiesList[i]);

            assert(activitiesPointerHash.contains(id));

            Activity* act = activitiesPointerHash.value(id);

            if (act->activityGroupId==0) {
                ActivitiesList* al = groupActivitiesPointerHash.value(0);
                al->removeOne(act);
            }
            else{
                groupActivitiesPointerHash.remove(act->activityGroupId);
            }

            activitiesPointerHash.remove(id);

            if(mdbaHash.contains(id)){
                int t=mdbaHash.remove(id);
                assert(t==1);
            }
        }
    }

    while(!toBeRemoved.isEmpty())
        delete toBeRemoved.takeFirst();

    activitiesList=newActivitiesList;

    if(updateConstraints)
        updateConstraintsAfterRemoval();

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    emit gotModified();
}

void Instance::modifyActivity(int _id,
                              int _activityGroupId,
                              const QString& _activityName,
                              bool _allowMultipleTeachers,
                              const QString& _subjectName,
                              const QStringList& _activityTagsNames,
                              const QStringList& _studentsName,
                              int _nSplits,
                              int _totalDuration,
                              int _durations[],
                              bool _active[],
                              bool _computeNTotalStudents,
                              int _nTotalStudents)
{
    int i=0;
    for(int j=0; j<this->activitiesList.size(); j++){
        Activity* act=this->activitiesList[j];
        if((_activityGroupId==0 && act->id==_id) || (_activityGroupId!=0 && act->activityGroupId==_activityGroupId)){
            act->activityName=_activityName;
            act->allowMultipleTeachers=_allowMultipleTeachers;
            act->subjectName=_subjectName;
            act->activityTagsNames=_activityTagsNames;
            act->studentSetsNames=_studentsName;
            act->duration=_durations[i];
            act->active=_active[i];
            act->totalDuration=_totalDuration;
            act->computeNTotalStudents=_computeNTotalStudents;
            act->nTotalStudents=_nTotalStudents;
            i++;
        }
    }

    assert(i==_nSplits);

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    emit gotModified();
}

void Instance::modifySubactivity(int _id,
                                 int _activityGroupId,
                                 const QString& _activityName,
                                 bool _allowMultipleTeachers,
                                 const QString& _subjectName,
                                 const QStringList& _activityTagsNames,
                                 const QStringList& _studentsName,
                                 int _duration,
                                 bool _active,
                                 bool _computeNTotalStudents,
                                 int _nTotalStudents)
{
    QList<Activity*> actsList;
    Activity* crtAct=nullptr;

    foreach(Activity* act, this->activitiesList){
        if(act->id==_id && act->activityGroupId==_activityGroupId){
            crtAct=act;
            //actsList.append(act);
        }
        else if(act->activityGroupId!=0 && _activityGroupId!=0 && act->activityGroupId==_activityGroupId){
            actsList.append(act);
        }
    }

    assert(crtAct!=nullptr);

    int td=0;
    foreach(Activity* act, actsList)
        td+=act->duration;
    td+=_duration; //crtAct->duration;
    foreach(Activity* act, actsList)
        act->totalDuration=td;

    crtAct->activityName=_activityName;
    crtAct->allowMultipleTeachers=_allowMultipleTeachers;
    crtAct->subjectName=_subjectName;
    crtAct->activityTagsNames=_activityTagsNames;
    crtAct->studentSetsNames=_studentsName;
    crtAct->duration=_duration;
    crtAct->totalDuration=td;
    crtAct->active=_active;
    crtAct->computeNTotalStudents=_computeNTotalStudents;
    crtAct->nTotalStudents=_nTotalStudents;

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);
}

bool Instance::addTimeConstraint(TimeConstraint* ctr)
{
    bool ok=true;

    //TODO: improve this

    //check if this constraint is already added, for ConstraintActivityPreferredStartingTime
    if(ctr->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
        ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) ctr;
        QSet<ConstraintActivityPreferredStartingTime*> cs=apstHash.value(c->activityId, QSet<ConstraintActivityPreferredStartingTime*>());
        foreach(ConstraintActivityPreferredStartingTime* oldc, cs){
            if((*oldc)==(*c)){
                ok=false;
                break;
            }
        }
    }

    //check if this constraint is already added, for ConstraintMinDaysBetweenActivities
    else if(ctr->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
        ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*) ctr;
        foreach(int aid, c->activitiesId){
            QSet<ConstraintMinDaysBetweenActivities*> cs=mdbaHash.value(aid, QSet<ConstraintMinDaysBetweenActivities*>());
            foreach(ConstraintMinDaysBetweenActivities* oldc, cs){
                if((*oldc)==(*c)){
                    ok=false;
                    break;
                }
            }
            if(!ok)
                break;
        }
    }

    else if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
        ConstraintStudentsSetNotAvailableTimes* c=(ConstraintStudentsSetNotAvailableTimes*) ctr;
        QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(c->students, QSet<ConstraintStudentsSetNotAvailableTimes*>());
        foreach(ConstraintStudentsSetNotAvailableTimes* oldc, cs){
            if(oldc->students==c->students){
                ok=false;
                break;
            }
        }
    }

    else if(ctr->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
        ConstraintTeacherNotAvailableTimes* c=(ConstraintTeacherNotAvailableTimes*) ctr;
        QSet<ConstraintTeacherNotAvailableTimes*> cs=tnatHash.value(c->teacher, QSet<ConstraintTeacherNotAvailableTimes*>());
        foreach(ConstraintTeacherNotAvailableTimes* oldc, cs){
            if(oldc->teacher==c->teacher){
                ok=false;
                break;
            }
        }
    }

    else if(ctr->type==CONSTRAINT_BREAK_TIMES){
        //ConstraintBreakTimes* c=(ConstraintBreakTimes*) ctr;
        QSet<ConstraintBreakTimes*> cs=btSet;
        if(cs.count()>0)
            ok=false;
    }

    else if(ctr->type==CONSTRAINT_BASIC_COMPULSORY_TIME){
        //ConstraintBasicCompulsoryTime* c=(ConstraintBasicCompulsoryTime*) ctr;
        QSet<ConstraintBasicCompulsoryTime*> cs=bctSet;
        if(cs.count()>0)
            ok=false;
    }

    if(ok){
        this->timeConstraintsList << ctr; //append

        if(ctr->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
            ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) ctr;
            QSet<ConstraintActivityPreferredStartingTime*> cs=apstHash.value(c->activityId, QSet<ConstraintActivityPreferredStartingTime*>());
            assert(!cs.contains(c));
            cs.insert(c);
            apstHash.insert(c->activityId, cs);
        }

        else if(ctr->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
            ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*) ctr;
            foreach(int aid, c->activitiesId){
                QSet<ConstraintMinDaysBetweenActivities*> cs=mdbaHash.value(aid, QSet<ConstraintMinDaysBetweenActivities*>());
                assert(!cs.contains(c));
                cs.insert(c);
                mdbaHash.insert(aid, cs);
            }
        }

        else if(ctr->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
            ConstraintTeacherNotAvailableTimes* c=(ConstraintTeacherNotAvailableTimes*) ctr;
            QSet<ConstraintTeacherNotAvailableTimes*> cs=tnatHash.value(c->teacher, QSet<ConstraintTeacherNotAvailableTimes*>());
            assert(!cs.contains(c));
            cs.insert(c);
            tnatHash.insert(c->teacher, cs);
        }

        else if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
            ConstraintStudentsSetNotAvailableTimes* c=(ConstraintStudentsSetNotAvailableTimes*) ctr;
            QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(c->students, QSet<ConstraintStudentsSetNotAvailableTimes*>());
            assert(!cs.contains(c));
            cs.insert(c);
            ssnatHash.insert(c->students, cs);
        }
        else if(ctr->type==CONSTRAINT_BASIC_COMPULSORY_TIME){
            ConstraintBasicCompulsoryTime* c=(ConstraintBasicCompulsoryTime*) ctr;
            QSet<ConstraintBasicCompulsoryTime*> &cs=bctSet;
            assert(!cs.contains(c));
            cs.insert(c);
        }
        else if(ctr->type==CONSTRAINT_BREAK_TIMES){
            ConstraintBreakTimes* c=(ConstraintBreakTimes*) ctr;
            QSet<ConstraintBreakTimes*> &cs=btSet;
            assert(!cs.contains(c));
            cs.insert(c);
        }

        this->internalStructureComputed=false;
        setRulesModifiedAndOtherThings(this);
        return true;
    }
    else
        return false;
}

bool Instance::removeTimeConstraint(TimeConstraint* ctr)
{
    for(int i=0; i<this->timeConstraintsList.size(); i++){
        if(this->timeConstraintsList[i]==ctr){
            if(ctr->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
                ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) ctr;
                QSet<ConstraintActivityPreferredStartingTime*> cs=apstHash.value(c->activityId, QSet<ConstraintActivityPreferredStartingTime*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    apstHash.insert(c->activityId, cs);
                }
                else{
                    int t=apstHash.remove(c->activityId);
                    assert(t==1);
                }
            }

            else if(ctr->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
                ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*) ctr;
                foreach(int aid, c->activitiesId){
                    QSet<ConstraintMinDaysBetweenActivities*> cs=mdbaHash.value(aid, QSet<ConstraintMinDaysBetweenActivities*>());
                    assert(cs.contains(c));
                    cs.remove(c);
                    if(!cs.isEmpty()){
                        mdbaHash.insert(aid, cs);
                    }
                    else{
                        int t=mdbaHash.remove(aid);
                        assert(t==1);
                    }
                }
            }

            else if(ctr->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
                ConstraintTeacherNotAvailableTimes* c=(ConstraintTeacherNotAvailableTimes*) ctr;
                QSet<ConstraintTeacherNotAvailableTimes*> cs=tnatHash.value(c->teacher, QSet<ConstraintTeacherNotAvailableTimes*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    assert(0);
                    tnatHash.insert(c->teacher, cs);
                }
                else{
                    int t=tnatHash.remove(c->teacher);
                    assert(t==1);
                }
            }

            else if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
                ConstraintStudentsSetNotAvailableTimes* c=(ConstraintStudentsSetNotAvailableTimes*) ctr;
                QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(c->students, QSet<ConstraintStudentsSetNotAvailableTimes*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    assert(0);
                    ssnatHash.insert(c->students, cs);
                }
                else{
                    int t=ssnatHash.remove(c->students);
                    assert(t==1);
                }
            }
            else if(ctr->type==CONSTRAINT_BASIC_COMPULSORY_TIME){
                ConstraintBasicCompulsoryTime* c=(ConstraintBasicCompulsoryTime*) ctr;
                QSet<ConstraintBasicCompulsoryTime*> &cs=bctSet;
                assert(cs.contains(c));
                cs.remove(c);
            }
            else if(ctr->type==CONSTRAINT_BREAK_TIMES){
                ConstraintBreakTimes* c=(ConstraintBreakTimes*) ctr;
                QSet<ConstraintBreakTimes*> &cs=btSet;
                assert(cs.contains(c));
                cs.remove(c);
            }

            delete ctr;
            this->timeConstraintsList.removeAt(i);
            this->internalStructureComputed=false;
            setRulesModifiedAndOtherThings(this);

            return true;
        }
    }

    return false;
}

bool Instance::removeTimeConstraints(QList<TimeConstraint*> _tcl)
{
    QSet<TimeConstraint*> _tcs=_tcl.toSet();
    QList<TimeConstraint*> remaining;

    for(int i=0; i<this->timeConstraintsList.size(); i++){
        TimeConstraint* ctr=timeConstraintsList[i];
        if(_tcs.contains(ctr)){
            if(ctr->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
                ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) ctr;
                QSet<ConstraintActivityPreferredStartingTime*> cs=apstHash.value(c->activityId, QSet<ConstraintActivityPreferredStartingTime*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    apstHash.insert(c->activityId, cs);
                }
                else{
                    int t=apstHash.remove(c->activityId);
                    assert(t==1);
                }
            }

            else if(ctr->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
                ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*) ctr;
                foreach(int aid, c->activitiesId){
                    QSet<ConstraintMinDaysBetweenActivities*> cs=mdbaHash.value(aid, QSet<ConstraintMinDaysBetweenActivities*>());
                    assert(cs.contains(c));
                    cs.remove(c);
                    if(!cs.isEmpty()){
                        mdbaHash.insert(aid, cs);
                    }
                    else{
                        int t=mdbaHash.remove(aid);
                        assert(t==1);
                    }
                }
            }

            else if(ctr->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
                ConstraintTeacherNotAvailableTimes* c=(ConstraintTeacherNotAvailableTimes*) ctr;
                QSet<ConstraintTeacherNotAvailableTimes*> cs=tnatHash.value(c->teacher, QSet<ConstraintTeacherNotAvailableTimes*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    assert(0);
                    tnatHash.insert(c->teacher, cs);
                }
                else{
                    int t=tnatHash.remove(c->teacher);
                    assert(t==1);
                }
            }

            else if(ctr->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
                ConstraintStudentsSetNotAvailableTimes* c=(ConstraintStudentsSetNotAvailableTimes*) ctr;
                QSet<ConstraintStudentsSetNotAvailableTimes*> cs=ssnatHash.value(c->students, QSet<ConstraintStudentsSetNotAvailableTimes*>());
                assert(cs.contains(c));
                cs.remove(c);
                if(!cs.isEmpty()){
                    assert(0);
                    ssnatHash.insert(c->students, cs);
                }
                else{
                    int t=ssnatHash.remove(c->students);
                    assert(t==1);
                }
            }
            else if(ctr->type==CONSTRAINT_BASIC_COMPULSORY_TIME){
                ConstraintBasicCompulsoryTime* c=(ConstraintBasicCompulsoryTime*) ctr;
                QSet<ConstraintBasicCompulsoryTime*> &cs=bctSet;
                assert(cs.contains(c));
                cs.remove(c);
            }
            else if(ctr->type==CONSTRAINT_BREAK_TIMES){
                ConstraintBreakTimes* c=(ConstraintBreakTimes*) ctr;
                QSet<ConstraintBreakTimes*> &cs=btSet;
                assert(cs.contains(c));
                cs.remove(c);
            }

            //_tcs.remove(ctr);
            delete ctr;
        }
        else
            remaining.append(ctr);
    }

    bool ok;
    assert(timeConstraintsList.count()<=remaining.count()+_tcs.count());
    if(timeConstraintsList.count()==remaining.count()+_tcs.count())
        ok=true;
    else
        ok=false;

    if(remaining.count()!=timeConstraintsList.count()){
        timeConstraintsList=remaining;

        this->internalStructureComputed=false;
        setRulesModifiedAndOtherThings(this);
    }

    return ok;
}

//void Instance::updateGroupActivitiesInInitialOrderAfterRemoval()
//{
//	GroupActivitiesInInitialOrderList newList;
//	GroupActivitiesInInitialOrderList toBeRemovedList;

//	foreach(GroupActivitiesInInitialOrderItem* item, groupActivitiesInInitialOrderList){
//		item->removeUseless(*this);
//		if(item->ids.count()<2)
//			toBeRemovedList.append(item);
//		else
//			newList.append(item);
//	}

//	groupActivitiesInInitialOrderList=newList;
//	while(!toBeRemovedList.isEmpty())
//		delete toBeRemovedList.takeFirst();
//}

void Instance::updateActivitiesWhenRemovingStudents(const QSet<StudentsSet*>& studentsSets, bool updateConstraints)
{
    if(studentsSets.count()==0)
        return;

    QList<int> toBeRemovedIds;

    QHash<QString, int> numberOfStudentsPerSet;

    foreach(StudentsSet* studentsSet, studentsSets)
        numberOfStudentsPerSet.insert(studentsSet->name, studentsSet->numberOfStudents);

    foreach(Activity* act, activitiesList){
        QStringList newStudents;
        foreach(QString st, act->studentSetsNames){
            if(!numberOfStudentsPerSet.contains(st)){
                newStudents << st;
            }
            else{
                if(act->computeNTotalStudents){
                    act->nTotalStudents-=numberOfStudentsPerSet.value(st);
                    assert(act->nTotalStudents>=0);
                }
            }
        }
        if(act->studentSetsNames.size()>0 && newStudents.size()==0)
            toBeRemovedIds.append(act->id);
        act->studentSetsNames=newStudents;
    }

    removeActivities(toBeRemovedIds, updateConstraints);
}

void Instance::updateConstraintsAfterRemoval()
{
    bool recomputeTime=false;
    //    bool recomputeSpace=false;

    Q_UNUSED(recomputeTime);

    QSet<int> existingActivitiesIds;
    QSet<QString> existingTeachersNames;
    //QSet<QString> existingStudentsNames;
    QSet<QString> existingSubjectsNames;
    QSet<QString> existingActivityTagsNames;
    QSet<QString> existingRoomsNames;

    QList<TimeConstraint*> toBeRemovedTime;

    foreach(Activity* act, activitiesList)
        existingActivitiesIds.insert(act->id);

    foreach(Teacher* tch, teachersList)
        existingTeachersNames.insert(tch->name);

    /*foreach(StudentsYear* sty, yearsList){
                existingStudentsNames.insert(sty->name);
                foreach(StudentsGroup* stg, sty->groupsList){
                        existingStudentsNames.insert(stg->name);
                        foreach(StudentsSubgroup* sts, stg->subgroupsList)
                                existingStudentsNames.insert(sts->name);
                }
        }*/

    foreach(Subject* sbj, subjectsList)
        existingSubjectsNames.insert(sbj->name);

    foreach(TimeConstraint* tc, timeConstraintsList){
        if(tc->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
            ConstraintTeacherNotAvailableTimes* c=(ConstraintTeacherNotAvailableTimes*)tc;
            if(!existingTeachersNames.contains(c->teacher))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
            ConstraintStudentsSetNotAvailableTimes* c=(ConstraintStudentsSetNotAvailableTimes*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME){
            ConstraintActivitiesSameStartingTime* c=(ConstraintActivitiesSameStartingTime*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING){
            ConstraintActivitiesNotOverlapping* c=(ConstraintActivitiesNotOverlapping*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
            ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_MAX_DAYS_BETWEEN_ACTIVITIES){
            ConstraintMaxDaysBetweenActivities* c=(ConstraintMaxDaysBetweenActivities*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_MIN_GAPS_BETWEEN_ACTIVITIES){
            ConstraintMinGapsBetweenActivities* c=(ConstraintMinGapsBetweenActivities*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MAX_HOURS_DAILY){
            ConstraintTeacherMaxHoursDaily* c=(ConstraintTeacherMaxHoursDaily*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherMaxHoursContinuously* c=(ConstraintTeacherMaxHoursContinuously*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeachersActivityTagMaxHoursContinuously* c=(ConstraintTeachersActivityTagMaxHoursContinuously*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherActivityTagMaxHoursContinuously* c=(ConstraintTeacherActivityTagMaxHoursContinuously*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName) || !existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK){
            ConstraintTeacherMaxDaysPerWeek* c=(ConstraintTeacherMaxDaysPerWeek*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK){
            ConstraintTeacherMinDaysPerWeek* c=(ConstraintTeacherMinDaysPerWeek*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK){
            ConstraintStudentsSetMaxGapsPerWeek* c=(ConstraintStudentsSetMaxGapsPerWeek*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK){
            ConstraintTeacherMaxGapsPerWeek* c=(ConstraintTeacherMaxGapsPerWeek*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY){
            ConstraintTeacherMaxGapsPerDay* c=(ConstraintTeacherMaxGapsPerDay*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR){
            ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* c=(ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY){
            ConstraintStudentsSetMaxHoursDaily* c=(ConstraintStudentsSetMaxHoursDaily*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetMaxHoursContinuously* c=(ConstraintStudentsSetMaxHoursContinuously*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsActivityTagMaxHoursContinuously* c=(ConstraintStudentsActivityTagMaxHoursContinuously*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetActivityTagMaxHoursContinuously* c=(ConstraintStudentsSetActivityTagMaxHoursContinuously*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName) || !permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY){
            ConstraintStudentsSetMinHoursDaily* c=(ConstraintStudentsSetMinHoursDaily*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
            ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*)tc;
            if(!existingActivitiesIds.contains(c->activityId)){
                toBeRemovedTime.append(tc);
                recomputeTime=true;
            }
        }
        else if(tc->type==CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS){
            ConstraintActivityPreferredTimeSlots* c=(ConstraintActivityPreferredTimeSlots*)tc;
            if(!existingActivitiesIds.contains(c->p_activityId))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES){
            ConstraintActivityPreferredStartingTimes* c=(ConstraintActivityPreferredStartingTimes*)tc;
            if(!existingActivitiesIds.contains(c->activityId))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintActivitiesPreferredTimeSlots* c=(ConstraintActivitiesPreferredTimeSlots*)tc;
            if( (c->p_teacherName!="" && !existingTeachersNames.contains(c->p_teacherName)) ||
                    (c->p_studentsName!="" && !permanentStudentsHash.contains(c->p_studentsName)) ||
                    (c->p_subjectName!="" && !existingSubjectsNames.contains(c->p_subjectName)) ||
                    (c->p_activityTagName!="" && !existingActivityTagsNames.contains(c->p_activityTagName)) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
            ConstraintSubactivitiesPreferredTimeSlots* c=(ConstraintSubactivitiesPreferredTimeSlots*)tc;
            if( (c->p_teacherName!="" && !existingTeachersNames.contains(c->p_teacherName)) ||
                    (c->p_studentsName!="" && !permanentStudentsHash.contains(c->p_studentsName)) ||
                    (c->p_subjectName!="" && !existingSubjectsNames.contains(c->p_subjectName)) ||
                    (c->p_activityTagName!="" && !existingActivityTagsNames.contains(c->p_activityTagName)) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintActivitiesPreferredStartingTimes* c=(ConstraintActivitiesPreferredStartingTimes*)tc;
            if( (c->teacherName!="" && !existingTeachersNames.contains(c->teacherName)) ||
                    (c->studentsName!="" && !permanentStudentsHash.contains(c->studentsName)) ||
                    (c->subjectName!="" && !existingSubjectsNames.contains(c->subjectName)) ||
                    (c->activityTagName!="" && !existingActivityTagsNames.contains(c->activityTagName)) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
            ConstraintSubactivitiesPreferredStartingTimes* c=(ConstraintSubactivitiesPreferredStartingTimes*)tc;
            if( (c->teacherName!="" && !existingTeachersNames.contains(c->teacherName)) ||
                    (c->studentsName!="" && !permanentStudentsHash.contains(c->studentsName)) ||
                    (c->subjectName!="" && !existingSubjectsNames.contains(c->subjectName)) ||
                    (c->activityTagName!="" && !existingActivityTagsNames.contains(c->activityTagName)) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR){
            ConstraintActivitiesSameStartingHour* c=(ConstraintActivitiesSameStartingHour*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY){
            ConstraintActivitiesSameStartingDay* c=(ConstraintActivitiesSameStartingDay*)tc;
            c->removeUseless();
            if(c->n_activities<2)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TWO_ACTIVITIES_CONSECUTIVE){
            ConstraintTwoActivitiesConsecutive* c=(ConstraintTwoActivitiesConsecutive*)tc;
            if( !existingActivitiesIds.contains(c->firstActivityId) ||
                    !existingActivitiesIds.contains(c->secondActivityId) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TWO_ACTIVITIES_GROUPED){
            ConstraintTwoActivitiesGrouped* c=(ConstraintTwoActivitiesGrouped*)tc;
            if( !existingActivitiesIds.contains(c->firstActivityId) ||
                    !existingActivitiesIds.contains(c->secondActivityId) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_THREE_ACTIVITIES_GROUPED){
            ConstraintThreeActivitiesGrouped* c=(ConstraintThreeActivitiesGrouped*)tc;
            if( !existingActivitiesIds.contains(c->firstActivityId) ||
                    !existingActivitiesIds.contains(c->secondActivityId) ||
                    !existingActivitiesIds.contains(c->thirdActivityId) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TWO_ACTIVITIES_ORDERED){
            ConstraintTwoActivitiesOrdered* c=(ConstraintTwoActivitiesOrdered*)tc;
            if( !existingActivitiesIds.contains(c->firstActivityId) ||
                    !existingActivitiesIds.contains(c->secondActivityId) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITY_ENDS_STUDENTS_DAY){
            ConstraintActivityEndsStudentsDay* c=(ConstraintActivityEndsStudentsDay*)tc;
            if(!existingActivitiesIds.contains(c->activityId))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MIN_HOURS_DAILY){
            ConstraintTeacherMinHoursDaily* c=(ConstraintTeacherMinHoursDaily*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintTeacherIntervalMaxDaysPerWeek* c=(ConstraintTeacherIntervalMaxDaysPerWeek*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetIntervalMaxDaysPerWeek* c=(ConstraintStudentsSetIntervalMaxDaysPerWeek*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            ConstraintActivitiesEndStudentsDay* c=(ConstraintActivitiesEndStudentsDay*)tc;
            if( (c->teacherName!="" && !existingTeachersNames.contains(c->teacherName)) ||
                    (c->studentsName!="" && !permanentStudentsHash.contains(c->studentsName)) ||
                    (c->subjectName!="" && !existingSubjectsNames.contains(c->subjectName)) ||
                    (c->activityTagName!="" && !existingActivityTagsNames.contains(c->activityTagName)) )
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintTeachersActivityTagMaxHoursDaily* c=(ConstraintTeachersActivityTagMaxHoursDaily*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintTeacherActivityTagMaxHoursDaily* c=(ConstraintTeacherActivityTagMaxHoursDaily*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName) || !existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintStudentsActivityTagMaxHoursDaily* c=(ConstraintStudentsActivityTagMaxHoursDaily*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY){
            ConstraintStudentsSetActivityTagMaxHoursDaily* c=(ConstraintStudentsSetActivityTagMaxHoursDaily*)tc;
            if(!existingActivityTagsNames.contains(c->activityTagName) || !permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY){
            ConstraintStudentsSetMaxGapsPerDay* c=(ConstraintStudentsSetMaxGapsPerDay*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_OCCUPY_MAX_TIME_SLOTS_FROM_SELECTION){
            ConstraintActivitiesOccupyMaxTimeSlotsFromSelection* c=(ConstraintActivitiesOccupyMaxTimeSlotsFromSelection*)tc;
            c->removeUseless();
            if(c->activitiesIds.count()<1)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_ACTIVITIES_MAX_SIMULTANEOUS_IN_SELECTED_TIME_SLOTS){
            ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots* c=(ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots*)tc;
            c->removeUseless();
            if(c->activitiesIds.count()<1)
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetMaxDaysPerWeek* c=(ConstraintStudentsSetMaxDaysPerWeek*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        //2017-02-07
        else if(tc->type==CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY){
            ConstraintTeacherMaxSpanPerDay* c=(ConstraintTeacherMaxSpanPerDay*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_TEACHER_MIN_RESTING_HOURS){
            ConstraintTeacherMinRestingHours* c=(ConstraintTeacherMinRestingHours*)tc;
            if(!existingTeachersNames.contains(c->teacherName))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY){
            ConstraintStudentsSetMaxSpanPerDay* c=(ConstraintStudentsSetMaxSpanPerDay*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
        else if(tc->type==CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS){
            ConstraintStudentsSetMinRestingHours* c=(ConstraintStudentsSetMinRestingHours*)tc;
            if(!permanentStudentsHash.contains(c->students))
                toBeRemovedTime.append(tc);
        }
    }

    bool t;
    if(toBeRemovedTime.count()>0){
        t=removeTimeConstraints(toBeRemovedTime);
        assert(t);
    }
}

bool Instance::read(QWidget* parent, const QString& fileName, bool commandLine, QString commandLineDirectory) //commandLineDirectory has trailing defs::FILE_SEP
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        //cout<<"Could not open file - not existing or in use\n";
        MessagesManager::information(parent, tr("m-FET warning"), tr("Could not open file - not existing or in use"));
        return false;
    }

    xmlReaderNumberOfUnrecognizedFields=0;

    QXmlStreamReader xmlReader(&file);

    ////////////////////////////////////////

    if(!commandLine){
        //logging part
        QDir dir;
        bool t=true;
        if(!dir.exists(defs::OUTPUT_DIR+defs::FILE_SEP+"logs"))
            t=dir.mkpath(defs::OUTPUT_DIR+defs::FILE_SEP+"logs");
        if(!t){
            MessagesManager::information(parent, tr("m-FET warning"), tr("Cannot create or use directory %1 - cannot continue").arg(QDir::toNativeSeparators(defs::OUTPUT_DIR+defs::FILE_SEP+"logs")));
            return false;
        }
        assert(t);
    }
    else{
        QDir dir;
        bool t=true;
        if(!dir.exists(commandLineDirectory+"logs"))
            t=dir.mkpath(commandLineDirectory+"logs");
        if(!t){
            MessagesManager::information(parent, tr("m-FET warning"), tr("Cannot create or use directory %1 - cannot continue").arg(QDir::toNativeSeparators(commandLineDirectory+"logs")));
            return false;
        }
        assert(t);
    }

    FakeString xmlReadingLog;
    xmlReadingLog="";

    QDate dat=QDate::currentDate();
    QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
    QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);

    QString reducedXmlLog="";
    reducedXmlLog+="Log generated by m-FET "+FET_VERSION+" on "+sTime+"\n\n";
    QString shortFileName=fileName.right(fileName.length()-fileName.lastIndexOf(defs::FILE_SEP)-1);
    reducedXmlLog+="Reading file "+shortFileName+"\n";
    QFileInfo fileinfo(fileName);
    reducedXmlLog+="Complete file name, including path: "+QDir::toNativeSeparators(fileinfo.absoluteFilePath())+"\n";
    reducedXmlLog+="\n";

    QString tmp;
    if(commandLine)
        tmp=commandLineDirectory+"logs"+defs::FILE_SEP+defs::XML_PARSING_LOG_FILENAME;
    else
        tmp=defs::OUTPUT_DIR+defs::FILE_SEP+"logs"+defs::FILE_SEP+defs::XML_PARSING_LOG_FILENAME;

    QFile file2(tmp);
    bool canWriteLogFile=true;
    if(!file2.open(QIODevice::WriteOnly)){
        QString s=tr("m-FET cannot open the log file %1 for writing. This might mean that you don't"
                     " have write permissions in this location. You can continue operation, but you might not be able to save the generated timetables"
                     " as html files").arg(QDir::toNativeSeparators(tmp))+
                "\n\n"+tr("A solution is to remove that file (if it exists already) or set its permissions to allow writing")+
                "\n\n"+tr("Please report possible bug");
        MessagesManager::critical(parent, tr("m-FET critical"), s);
        canWriteLogFile=false;
    }
    QTextStream logStream;
    if(canWriteLogFile){
        logStream.setDevice(&file2);
        logStream.setCodec("UTF-8");
        logStream.setGenerateByteOrderMark(true);
    }

    bool okAbove3_12_17=true;
    bool version5AndAbove=false;
    bool warning=false;

    QString file_version;

    if(xmlReader.readNextStartElement()){
        xmlReadingLog+=" Found "+xmlReader.name().toString()+" tag\n";

        bool okfetTag;
        if(xmlReader.name()=="m-fet" || xmlReader.name()=="m-FET" || xmlReader.name()=="fet") //the new tag is m-fet, the old tag - m-FET
            okfetTag=true;
        else
            okfetTag=false;

        if(!okfetTag)
            okAbove3_12_17=false;
        else{
            assert(xmlReader.isStartElement());
            assert(okAbove3_12_17==true);

            int filev[3], fetv[3];

            assert(xmlReader.name()=="m-fet" || xmlReader.name()=="m-FET" || xmlReader.name()=="fet");
            QString version=xmlReader.attributes().value("version").toString();
            file_version=version;

            QRegExp fileVerReCap("^(\\d+)\\.(\\d+)\\.(\\d+)(.*)$");

            int tfile=fileVerReCap.indexIn(file_version);
            filev[0]=filev[1]=filev[2]=-1;
            if(tfile!=0){
                MessagesManager::warning(parent, tr("m-FET warning"), tr("File contains a version numbering scheme which"
                                                                                " is not matched by v.v.va (3 numbers separated by points, followed by any string a, which may be empty). File will be opened, but you are advised"
                                                                                " to check the version of the .m-fet file (in the beginning of the file). If this is a m-FET bug, please report it")+"\n\n"+
                                                  tr("If you are opening a file older than m-FET format version 5, it will be converted to latest m-FET data format"));
                if(defs::VERBOSE){
                    cout<<"Opened file version not matched by regexp"<<endl;
                }
            }
            else{
                bool ok;
                filev[0]=fileVerReCap.cap(1).toInt(&ok);
                assert(ok);
                filev[1]=fileVerReCap.cap(2).toInt(&ok);
                assert(ok);
                filev[2]=fileVerReCap.cap(3).toInt(&ok);
                assert(ok);
                if(defs::VERBOSE){
                    cout<<"Opened FET file version matched by regexp: major="<<filev[0]<<", minor="<<filev[1]<<", revision="<<filev[2];
                    cout<<", additional text="<<qPrintable(fileVerReCap.cap(4))<<"."<<endl;
                }
            }

            QRegExp fetVerReCap("^(\\d+)\\.(\\d+)\\.(\\d+)(.*)$");

            int tfet=fetVerReCap.indexIn(FET_VERSION);
            fetv[0]=fetv[1]=fetv[2]=-1;
            if(tfet!=0){
                MessagesManager::warning(parent, tr("m-FET warning"), tr("m-FET version does not respect the format v.v.va"
                                                                                " (3 numbers separated by points, followed by any string a, which may be empty). This is probably a bug in m-FET - please report it"));
                if(defs::VERBOSE){
                    cout<<"m-FET version not matched by regexp"<<endl;
                }
            }
            else{
                bool ok;
                fetv[0]=fetVerReCap.cap(1).toInt(&ok);
                assert(ok);
                fetv[1]=fetVerReCap.cap(2).toInt(&ok);
                assert(ok);
                fetv[2]=fetVerReCap.cap(3).toInt(&ok);
                assert(ok);
                if(defs::VERBOSE){
                    cout<<"FET file version matched by regexp: major="<<fetv[0]<<", minor="<<fetv[1]<<", revision="<<fetv[2];
                    cout<<", additional text="<<qPrintable(fetVerReCap.cap(4))<<"."<<endl;
                }
            }

            if(filev[0]>=0 && fetv[0]>=0 && filev[1]>=0 && fetv[1]>=0 && filev[2]>=0 && fetv[2]>=0){
                if(filev[0]>fetv[0] || (filev[0]==fetv[0] && filev[1]>fetv[1]) || (filev[0]==fetv[0]&&filev[1]==fetv[1]&&filev[2]>fetv[2])){
                    warning=true;
                }
            }

            if(filev[0]>=5 || (filev[0]==-1 && filev[1]==-1 && filev[2]==-1))
                //if major is >= 5 or major cannot be read
                version5AndAbove=true;
        }
    }

    if(!okAbove3_12_17){
        if(defs::VERBOSE){
            cout<<"Invalid m-fet 3.12.17 or above"<<endl;
        }
        file2.close();
        MessagesManager::information(parent, tr("m-FET warning"), tr("File does not have a corresponding beginning tag - it should be %1 or %2. File is incorrect..."
                                                                          "it cannot be opened").arg("m-fet").arg("m-FET"));
        return false;
    }

    if(!version5AndAbove){
        MessagesManager::warning(parent, tr("m-FET information"),
                                          tr("Opening older file - it will be converted to latest format, automatically "
                                             "assigning weight percentages to constraints and dropping parity for activities. "
                                             "You are advised to make a backup of your old file before saving in new format.\n\n"
                                             "Please note that the default weight percentage of constraints min days between activities "
                                             "will be 95% (mainly satisfied, not always) and 'force consecutive if same day' will be set to true "
                                             "(meaning that if the activities are in the same day, they will be placed continuously, in a bigger duration activity). "
                                             "If you want, you can modify this percent to be 100%, manually in the m-fet input file "
                                             "or from the interface"));
    }

    if(warning){
        MessagesManager::warning(parent, tr("m-FET information"),
                                          tr("Opening a file generated with a newer version than your current m-FET software ... file will be opened but it is recommended to update your m-FET software to the latest version")
                                          +"\n\n"+tr("Your m-FET version: %1, file version: %2").arg(FET_VERSION).arg(file_version));
    }

    //Clear old rules, initialize new rules
    if(this->initialized)
        this->kill();
    this->init();

    this->institutionName=tr("Default institution");
    this->comments=tr("Default comments");

    this->nDaysPerWeek=5;
    this->daysOfTheWeek[0] = tr("Monday");
    this->daysOfTheWeek[1] = tr("Tuesday");
    this->daysOfTheWeek[2] = tr("Wednesday");
    this->daysOfTheWeek[3] = tr("Thursday");
    this->daysOfTheWeek[4] = tr("Friday");

    this->nHoursPerDay=12;
    this->hoursOfTheDay[0]=tr("08:00", "Hour name");
    this->hoursOfTheDay[1]=tr("09:00", "Hour name");
    this->hoursOfTheDay[2]=tr("10:00", "Hour name");
    this->hoursOfTheDay[3]=tr("11:00", "Hour name");
    this->hoursOfTheDay[4]=tr("12:00", "Hour name");
    this->hoursOfTheDay[5]=tr("13:00", "Hour name");
    this->hoursOfTheDay[6]=tr("14:00", "Hour name");
    this->hoursOfTheDay[7]=tr("15:00", "Hour name");
    this->hoursOfTheDay[8]=tr("16:00", "Hour name");
    this->hoursOfTheDay[9]=tr("17:00", "Hour name");
    this->hoursOfTheDay[10]=tr("18:00", "Hour name");
    this->hoursOfTheDay[11]=tr("19:00", "Hour name");

    bool skipDeprecatedConstraints=false;

    bool skipDuplicatedStudentsSets=false;

    assert(xmlReader.isStartElement() && (xmlReader.name()=="m-fet" || xmlReader.name()=="m-FET" || xmlReader.name()=="fet"));
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="  Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Institution_Name"){
            QString text=xmlReader.readElementText();
            this->institutionName=text;
            xmlReadingLog+="  Found institution name="+this->institutionName+"\n";
            reducedXmlLog+="Read institution name="+this->institutionName+"\n";
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            this->comments=text;
            xmlReadingLog+="  Found comments="+this->comments+"\n";
            reducedXmlLog+="Read comments="+this->comments+"\n";
        }
        else if(xmlReader.name()=="Days_List"){
            int tmp=0;
            bool numberWasFound=false;
            assert(xmlReader.isStartElement());
            QString numberString="Number_of_Days", dayString="Name";
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Number" || xmlReader.name()=="Number_of_Days" ){
                    numberString=xmlReader.name().toString();
                    QString text=xmlReader.readElementText();
                    this->nDaysPerWeek=text.toInt();
                    numberWasFound=true;
                    xmlReadingLog+="   Found the number of days per week = "+
                            utils::strings::number(this->nDaysPerWeek)+"\n";
                    reducedXmlLog+="Added "+
                            utils::strings::number(this->nDaysPerWeek)+" days per week\n";
                    if(nDaysPerWeek<=0){
                        xmlReader.raiseError(tr("%1 is incorrect").arg(numberString));
                    }
                    else if(nDaysPerWeek>MAX_DAYS_PER_WEEK){
                        xmlReader.raiseError(tr("%1 is too large. Maximum allowed is %2.").arg(numberString).arg(MAX_DAYS_PER_WEEK));
                    }
                    else{
                        assert(this->nDaysPerWeek>0 && nDaysPerWeek<=MAX_DAYS_PER_WEEK);
                    }
                }
                //old .m-fet XML format
                else if(xmlReader.name()=="Name"){
                    dayString=xmlReader.name().toString();
                    if(tmp>=MAX_DAYS_PER_WEEK){
                        xmlReader.raiseError(tr("Too many %1 items. Maximum allowed is %2.").arg(dayString).arg(MAX_DAYS_PER_WEEK));
                        xmlReader.skipCurrentElement();
                    }
                    else{
                        QString text=xmlReader.readElementText();
                        this->daysOfTheWeek[tmp]=text;
                        xmlReadingLog+="   Found day "+this->daysOfTheWeek[tmp]+"\n";
                        tmp++;
                    }
                }
                //end old .m-fet XML format
                else if(xmlReader.name()=="Day"){
                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Name"){
                            dayString=xmlReader.name().toString();
                            if(tmp>=MAX_DAYS_PER_WEEK){
                                xmlReader.raiseError(tr("Too many %1 items. Maximum allowed is %2.").arg(dayString).arg(MAX_DAYS_PER_WEEK));
                                xmlReader.skipCurrentElement();
                            }
                            else{
                                QString text=xmlReader.readElementText();
                                this->daysOfTheWeek[tmp]=text;
                                xmlReadingLog+="   Found day "+this->daysOfTheWeek[tmp]+"\n";
                                tmp++;
                            }
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            if(!xmlReader.error()){
                if(!numberWasFound){
                    xmlReader.raiseError(tr("%1 not found").arg(numberString));
                }
                else{
                    if(!(tmp==nDaysPerWeek))
                        xmlReader.raiseError(tr("%1: %2 and the number of %3 read do not correspond").arg("Days_List").arg(numberString).arg(dayString));
                    else
                        assert(tmp==this->nDaysPerWeek);
                }
            }
        }
        else if(xmlReader.name()=="Hours_List"){
            int tmp=0;
            bool numberWasFound=false;
            assert(xmlReader.isStartElement());
            QString numberString="Number_of_Hours", hourString="Name";
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Number" || xmlReader.name()=="Number_of_Hours"){
                    numberString=xmlReader.name().toString();
                    QString text=xmlReader.readElementText();
                    this->nHoursPerDay=text.toInt();
                    numberWasFound=true;
                    xmlReadingLog+="   Found the number of hours per day = "+
                            utils::strings::number(this->nHoursPerDay)+"\n";
                    reducedXmlLog+="Added "+
                            utils::strings::number(this->nHoursPerDay)+" hours per day\n";
                    if(nHoursPerDay<=0){
                        xmlReader.raiseError(tr("%1 is incorrect").arg(numberString));
                    }
                    else if(nHoursPerDay>MAX_HOURS_PER_DAY){
                        xmlReader.raiseError(tr("%1 is too large. Maximum allowed is %2.").arg("Number").arg(MAX_HOURS_PER_DAY));
                    }
                    else{
                        assert(this->nHoursPerDay>0 && nHoursPerDay<=MAX_HOURS_PER_DAY);
                    }
                }
                //old .m-fet XML format
                else if(xmlReader.name()=="Name"){
                    hourString=xmlReader.name().toString();
                    if(tmp>=MAX_HOURS_PER_DAY){
                        xmlReader.raiseError(tr("Too many %1 items. Maximum allowed is %2.").arg(hourString).arg(MAX_HOURS_PER_DAY));
                        xmlReader.skipCurrentElement();
                    }
                    else{
                        QString text=xmlReader.readElementText();
                        this->hoursOfTheDay[tmp]=text;
                        xmlReadingLog+="   Found hour "+this->hoursOfTheDay[tmp]+"\n";
                        tmp++;
                    }
                }
                //end old .m-fet XML format
                else if(xmlReader.name()=="Hour"){
                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Name"){
                            hourString=xmlReader.name().toString();
                            if(tmp>=MAX_HOURS_PER_DAY){
                                xmlReader.raiseError(tr("Too many %1 items. Maximum allowed is %2.").arg(hourString).arg(MAX_HOURS_PER_DAY));
                                xmlReader.skipCurrentElement();
                            }
                            else{
                                QString text=xmlReader.readElementText();
                                this->hoursOfTheDay[tmp]=text;
                                xmlReadingLog+="    Found hour "+this->hoursOfTheDay[tmp]+"\n";
                                tmp++;
                            }
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            if(!xmlReader.error()){
                if(!numberWasFound){
                    xmlReader.raiseError(tr("%1 not found").arg(numberString));
                }
                else{
                    if(numberString=="Number"){
                        //some older files contain also the end of day hour, so tmp==nHoursPerDay+1 in this case
                        if(!(tmp==nHoursPerDay || tmp==nHoursPerDay+1))
                            xmlReader.raiseError(tr("%1: %2 and the number of %3 read do not correspond").arg("Hours_List").arg(numberString).arg(hourString));
                        else
                            assert(tmp==nHoursPerDay || tmp==nHoursPerDay+1);
                    }
                    else{
                        assert(numberString=="Number_of_Hours");
                        if(!(tmp==nHoursPerDay))
                            xmlReader.raiseError(tr("%1: %2 and the number of %3 read do not correspond").arg("Hours_List").arg(numberString).arg(hourString));
                        else
                            assert(tmp==nHoursPerDay);
                    }
                }
            }
        }
        else if(xmlReader.name()=="Teachers_List"){
            QSet<QString> teachersRead;

            QSet<QString> subjectsRead; //we assume that the reading of the subjects is done before the reading of the teachers
            foreach(Subject* sbj, subjectsList)
                subjectsRead.insert(sbj->name);

            int tmp=0;
            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Teacher"){
                    Teacher* teacher=new Teacher();

                    QHash<Subject*, Enums::SubjectPreference> subject_preference_map;

                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Name"){
                            QString text=xmlReader.readElementText();
                            teacher->name=text;
                            xmlReadingLog+="    Read teacher name: "+teacher->name+"\n";
                        }
                        else if(xmlReader.name()=="Active"){
                            QString text=xmlReader.readElementText();
                            if(text=="yes" || text=="true" || text=="1"){
                                teacher->active=true;
                                xmlReadingLog+="	Current teacher is active\n";
                            }
                            else{
                                if(!(text=="no" || text=="false" || text=="0")){
                                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                                      tr("Found teacher active tag which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                                         " The teacher will be considered not active",
                                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                                }
                                //assert(text=="no" || text=="false" || text=="0");
                                teacher->active=false;
                                xmlReadingLog+="	Current teacher is not active\n";
                            }
                        }
                        else if(xmlReader.name()=="Target_Number_of_Hours"){
                            QString text=xmlReader.readElementText();
                            teacher->targetNumberOfHours=text.toInt();
                            xmlReadingLog+="    Read teacher target number of hours: "+utils::strings::number(teacher->targetNumberOfHours)+"\n";
                        }
                        else if(xmlReader.name()=="Subjects_Preferences"){
                            assert(xmlReader.isStartElement());
                            while(xmlReader.readNextStartElement()){
                                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";

                                QString name;
                                Enums::SubjectPreference pref = Enums::SubjectPreference::Indifferent;

                                if(xmlReader.name()=="Subject"){
                                    QString text=xmlReader.readElementText();
                                    if(subject_preference_map.contains(subjectsList[searchSubject(text)])){
                                        xmlReader.raiseError(tr("The teacher %1 has duplicate qualified subject %2.").arg(teacher->name).arg(text));
                                    }
                                    else if(!subjectsRead.contains(text)){
                                        xmlReader.raiseError(tr("The teacher %1 has attached an inexistent qualified subject %2."
                                                                " Please note that starting with m-FET version %3 the list of subjects (tag <Subjects_List> in the .m-fet XML file)"
                                                                " must appear before the list of teachers (tag <Teachers_List> in the .m-fet XML file)", "Please keep <Subjects_List> and"
                                                                                                                                                       " <Teachers_List> untranslated, as in the original English string")
                                                             .arg(teacher->name).arg(text).arg("5.30.0"));
                                    }
                                    else{
                                        name = text;
                                    }
                                }
                                else{
                                    xmlReader.skipCurrentElement();
                                    xmlReaderNumberOfUnrecognizedFields++;
                                }

                                xmlReader.readNextStartElement();

                                if(xmlReader.name()=="Preference"){
                                    QString text=xmlReader.readElementText();
                                    pref = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::SubjectPreference::Indifferent);
                                }
                                else{
                                    xmlReader.skipCurrentElement();
                                    xmlReaderNumberOfUnrecognizedFields++;
                                }

                                if (name.size()>0) {
                                    subject_preference_map.insert(subjectsList[searchSubject(name)], pref);
                                }
                            }
                        }
                        else if(xmlReader.name()=="Comments"){
                            QString text=xmlReader.readElementText();
                            teacher->comments=text;
                            xmlReadingLog+="    Crt. teacher comments="+teacher->comments+"\n";
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                    bool tmp2=teachersRead.contains(teacher->name);
                    if(tmp2){
                        MessagesManager::warning(parent, tr("m-FET warning"),
                                                          tr("Duplicate teacher %1 found - ignoring").arg(teacher->name));
                        xmlReadingLog+="   Teacher not added - duplicate\n";

                        delete teacher;
                    }
                    else{
                        teachersRead.insert(teacher->name);
                        this->addTeacherFast(teacher);

                        for (auto sbj: subjectsList)
                            if (!subject_preference_map.contains(sbj))
                                subject_preference_map.insert(sbj, Enums::SubjectPreference::Indifferent);

                        teacher_subject_preference_map.insert(teacher, subject_preference_map);

                        tmp++;
                        xmlReadingLog+="   Teacher added\n";
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            if(!(tmp==this->teachersList.size()))
                xmlReader.raiseError(tr("%1 is incorrect").arg("Teachers_List"));
            else{
                assert(tmp==this->teachersList.size());
                xmlReadingLog+="  Added "+utils::strings::number(tmp)+" teachers\n";
                reducedXmlLog+="Added "+utils::strings::number(tmp)+" teachers\n";
            }
        }
        else if(xmlReader.name()=="Subjects_List"){
            QSet<QString> subjectsRead;

            int tmp=0;
            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Subject"){
                    Subject* subject=new Subject();
                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Name"){
                            QString text=xmlReader.readElementText();
                            subject->name=text;
                            xmlReadingLog+="    Read subject name: "+subject->name+"\n";
                        }
                        else if(xmlReader.name()=="Comments"){
                            QString text=xmlReader.readElementText();
                            subject->comments=text;
                            xmlReadingLog+="    Crt. subject comments="+subject->comments+"\n";
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                    bool tmp2=subjectsRead.contains(subject->name);
                    if(tmp2){
                        MessagesManager::warning(parent, tr("m-FET warning"),
                                                          tr("Duplicate subject %1 found - ignoring").arg(subject->name));
                        xmlReadingLog+="   Subject not added - duplicate\n";

                        delete subject;
                    }
                    else{
                        subjectsRead.insert(subject->name);
                        this->addSubjectFast(subject);
                        tmp++;
                        xmlReadingLog+="   Subject inserted\n";
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            if(!(tmp==this->subjectsList.size()))
                xmlReader.raiseError(tr("%1 is incorrect").arg("Subjects_List"));
            else{
                assert(tmp==this->subjectsList.size());
                xmlReadingLog+="  Added "+utils::strings::number(tmp)+" subjects\n";
                reducedXmlLog+="Added "+utils::strings::number(tmp)+" subjects\n";
            }
        }
        else if(xmlReader.name()=="Students_List"){
            QSet<StudentsSet*> allAllocatedStudentsSets;

            bool okStudents=true;

            //permanentStudentsHash.clear();

            QHash<QString, StudentsSet*> currentStudentsHash;

            int tsgr=0;
            int tgr=0;

            int ny=0;
            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Year"){
                    StudentsYear* sty=new StudentsYear();
                    allAllocatedStudentsSets.insert(sty);
                    int ng=0;

                    QSet<QString> groupsInYear;

                    bool tmp2=this->addYearFast(sty);
                    assert(tmp2==true);
                    ny++;

                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Name"){
                            QString text=xmlReader.readElementText();
                            if(!skipDuplicatedStudentsSets){
                                QString nn=text;
                                //StudentsSet* ss=this->searchStudentsSet(nn);
                                StudentsSet* ss=currentStudentsHash.value(nn, nullptr);
                                if(ss!=nullptr){
                                    QString str;

                                    if(ss->type==STUDENTS_YEAR)
                                        str=tr("Trying to add year %1, which is already added as another year.").arg(nn);
                                    else if(ss->type==STUDENTS_GROUP)
                                        str=tr("Trying to add year %1, which is already added as another group.").arg(nn);
                                    else if(ss->type==STUDENTS_SUBGROUP)
                                        str=tr("Trying to add year %1, which is already added as another subgroup.").arg(nn);

                                    xmlReader.raiseError(str);
                                    okStudents=false;
                                }
                            }

                            sty->name=text;
                            if(!currentStudentsHash.contains(sty->name))
                                currentStudentsHash.insert(sty->name, sty);
                            xmlReadingLog+="    Read year name: "+sty->name+"\n";
                        }
                        else if(xmlReader.name()=="Number_of_Students"){
                            QString text=xmlReader.readElementText();
                            sty->numberOfStudents=text.toInt();
                            xmlReadingLog+="    Read year number of students: "+utils::strings::number(sty->numberOfStudents)+"\n";
                        }
                        else if(xmlReader.name()=="Comments"){
                            QString text=xmlReader.readElementText();
                            sty->comments=text;
                            xmlReadingLog+="    Crt. year comments="+sty->comments+"\n";
                        }
                        else if(xmlReader.name()=="Group"){
                            QSet<StudentsSubgroup*> allocatedSubgroups;

                            StudentsGroup* stg=new StudentsGroup();
                            allAllocatedStudentsSets.insert(stg);
                            int ns=0;

                            QSet<QString> subgroupsInGroup;

                            assert(xmlReader.isStartElement());
                            while(xmlReader.readNextStartElement()){
                                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                                if(xmlReader.name()=="Name"){
                                    QString text=xmlReader.readElementText();
                                    if(!skipDuplicatedStudentsSets){
                                        QString nn=text;
                                        StudentsSet* ss=currentStudentsHash.value(nn, nullptr);
                                        if(ss!=nullptr){
                                            QString str;
                                            bool haveError=false;

                                            if(ss->type==STUDENTS_YEAR){
                                                str=tr("Trying to add group %1, which is already added as another year.").arg(nn);
                                                haveError=true;
                                            }
                                            else if(ss->type==STUDENTS_GROUP){
                                                if(groupsInYear.contains(nn)){
                                                    str=tr("Trying to add group %1 in year %2 but it is already added.").arg(nn).arg(sty->name);
                                                    haveError=true;
                                                }
                                                else{
                                                    str="";
                                                    assert(haveError==false);
                                                }
                                            }
                                            else if(ss->type==STUDENTS_SUBGROUP){
                                                str=tr("Trying to add group %1, which is already added as another subgroup.").arg(nn);
                                                haveError=true;
                                            }

                                            if(haveError){
                                                xmlReader.raiseError(str);
                                                okStudents=false;
                                            }
                                        }
                                    }

                                    groupsInYear.insert(text);

                                    if(currentStudentsHash.contains(text)){
                                        foreach(StudentsSubgroup* sts, allocatedSubgroups){
                                            assert(currentStudentsHash.contains(sts->name));
                                            currentStudentsHash.remove(sts->name);

                                            assert(allAllocatedStudentsSets.contains(sts));
                                            allAllocatedStudentsSets.remove(sts);
                                            delete sts;
                                        }
                                        allocatedSubgroups.clear(); //not really needed

                                        assert(allAllocatedStudentsSets.contains(stg));
                                        allAllocatedStudentsSets.remove(stg);
                                        delete stg;

                                        if(okStudents){
                                            assert(currentStudentsHash.value(text)->type==STUDENTS_GROUP);
                                            stg=(StudentsGroup*)(currentStudentsHash.value(text));

                                            bool tmp4=this->addGroupFast(sty, stg);
                                            assert(tmp4==true);
                                            //ng++;
                                        }

                                        xmlReader.skipCurrentElement();
                                        break;
                                    }

                                    bool tmp4=this->addGroupFast(sty, stg);
                                    assert(tmp4==true);
                                    ng++;

                                    stg->name=text;
                                    if(!currentStudentsHash.contains(stg->name))
                                        currentStudentsHash.insert(stg->name, stg);
                                    xmlReadingLog+="     Read group name: "+stg->name+"\n";
                                }
                                else if(xmlReader.name()=="Number_of_Students"){
                                    QString text=xmlReader.readElementText();
                                    stg->numberOfStudents=text.toInt();
                                    xmlReadingLog+="     Read group number of students: "+utils::strings::number(stg->numberOfStudents)+"\n";
                                }
                                else if(xmlReader.name()=="Comments"){
                                    QString text=xmlReader.readElementText();
                                    stg->comments=text;
                                    xmlReadingLog+="    Crt. group comments="+stg->comments+"\n";
                                }
                                else if(xmlReader.name()=="Subgroup"){
                                    StudentsSubgroup* sts=new StudentsSubgroup();
                                    allAllocatedStudentsSets.insert(sts);

                                    assert(xmlReader.isStartElement());
                                    while(xmlReader.readNextStartElement()){
                                        xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                                        if(xmlReader.name()=="Name"){
                                            QString text=xmlReader.readElementText();
                                            if(!skipDuplicatedStudentsSets){
                                                QString nn=text;
                                                StudentsSet* ss=currentStudentsHash.value(nn, nullptr);
                                                if(ss!=nullptr){
                                                    QString str;
                                                    bool haveError=false;

                                                    if(ss->type==STUDENTS_YEAR){
                                                        str=tr("Trying to add subgroup %1, which is already added as another year.").arg(nn);
                                                        haveError=true;
                                                    }
                                                    else if(ss->type==STUDENTS_GROUP){
                                                        str=tr("Trying to add subgroup %1, which is already added as another group.").arg(nn);
                                                        haveError=true;
                                                    }
                                                    else if(ss->type==STUDENTS_SUBGROUP){
                                                        if(subgroupsInGroup.contains(nn)){
                                                            str=tr("Trying to add subgroup %1 in year %2, group %3 but it is already added.").arg(nn).arg(sty->name).arg(stg->name);
                                                            haveError=true;
                                                        }
                                                        else{
                                                            str="";
                                                            assert(haveError==false);
                                                        }
                                                    }

                                                    if(haveError){
                                                        xmlReader.raiseError(str);
                                                        okStudents=false;
                                                    }
                                                }
                                            }

                                            subgroupsInGroup.insert(text);

                                            if(currentStudentsHash.contains(text)){
                                                assert(allAllocatedStudentsSets.contains(sts));
                                                allAllocatedStudentsSets.remove(sts);
                                                delete sts;

                                                if(okStudents){
                                                    assert(currentStudentsHash.value(text)->type==STUDENTS_SUBGROUP);
                                                    sts=(StudentsSubgroup*)(currentStudentsHash.value(text));

                                                    bool tmp6=this->addSubgroupFast(sty, stg, sts);
                                                    assert(tmp6==true);
                                                    //ns++;
                                                }

                                                xmlReader.skipCurrentElement();
                                                break;
                                            }
                                            else{
                                                allocatedSubgroups.insert(sts);
                                            }

                                            bool tmp6=this->addSubgroupFast(sty, stg, sts);
                                            assert(tmp6==true);
                                            ns++;

                                            sts->name=text;
                                            if(!currentStudentsHash.contains(sts->name))
                                                currentStudentsHash.insert(sts->name, sts);
                                            xmlReadingLog+="     Read subgroup name: "+sts->name+"\n";
                                        }
                                        else if(xmlReader.name()=="Number_of_Students"){
                                            QString text=xmlReader.readElementText();
                                            sts->numberOfStudents=text.toInt();
                                            xmlReadingLog+="     Read subgroup number of students: "+utils::strings::number(sts->numberOfStudents)+"\n";
                                        }
                                        else if(xmlReader.name()=="Comments"){
                                            QString text=xmlReader.readElementText();
                                            sts->comments=text;
                                            xmlReadingLog+="    Crt. subgroup comments="+sts->comments+"\n";
                                        }
                                        else{
                                            xmlReader.skipCurrentElement();
                                            xmlReaderNumberOfUnrecognizedFields++;
                                        }
                                    }
                                }
                                else{
                                    xmlReader.skipCurrentElement();
                                    xmlReaderNumberOfUnrecognizedFields++;
                                }
                            }
                            if(ns == stg->subgroupsList.size()){
                                xmlReadingLog+="    Added "+utils::strings::number(ns)+" subgroups\n";
                                tsgr+=ns;
                                //reducedXmlLog+="		Added "+utils::strings::number(ns)+" subgroups\n";
                            }
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                    if(ng == sty->groupsList.size()){
                        xmlReadingLog+="   Added "+utils::strings::number(ng)+" groups\n";
                        tgr+=ng;
                        //reducedXmlLog+="	Added "+utils::strings::number(ng)+" groups\n";
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            xmlReadingLog+="  Added "+utils::strings::number(ny)+" years\n";
            reducedXmlLog+="Added "+utils::strings::number(ny)+" students years\n";
            //reducedXmlLog+="Added "+utils::strings::number(tgr)+" students groups (see note below)\n";
            reducedXmlLog+="Added "+utils::strings::number(tgr)+" students groups\n";
            //reducedXmlLog+="Added "+utils::strings::number(tsgr)+" students subgroups (see note below)\n";
            reducedXmlLog+="Added "+utils::strings::number(tsgr)+" students subgroups\n";
            assert(this->yearsList.size()==ny);

            //BEGIN test for number of students is the same in all sets with the same name
            /*bool reportWrongNumberOfStudents=true;
                        foreach(StudentsYear* year, yearsList){
                                assert(studentsHash.contains(year->name));
                                StudentsSet* sy=studentsHash.value(year->name);
                                if(sy->numberOfStudents!=year->numberOfStudents){
                                        if(reportWrongNumberOfStudents){
                                                QString str=tr("Minor problem found and corrected: year %1 has different number of students in two places (%2 and %3)", "%2 and %3 are number of students")
                                                        .arg(year->name).arg(sy->numberOfStudents).arg(year->numberOfStudents)
                                                        +
                                                        "\n\n"+
                                                        tr("Explanation: this is a minor problem, which appears if using overlapping students set, due to a bug in m-FET previous to version %1."
                                                        " m-FET will now correct this problem by setting the number of students for this year, in all places where it appears,"
                                                        " to the number that was found in the first appearance (%2). It is advisable to check the number of students for this year.")
                                                        .arg("5.12.1").arg(sy->numberOfStudents);
                                                int t=MessagesManager::warning(parent, tr("m-FET warning"), str,
                                                         tr("Skip rest"), tr("See next"), QString(),
                                                         1, 0 );

                                                if(t==0)
                                                        reportWrongNumberOfStudents=false;
                                        }
                                        year->numberOfStudents=sy->numberOfStudents;
                                }

                                foreach(StudentsGroup* group, year->groupsList){
                                        assert(studentsHash.contains(group->name));
                                        StudentsSet* sg=studentsHash.value(group->name);
                                        if(sg->numberOfStudents!=group->numberOfStudents){
                                                if(reportWrongNumberOfStudents){
                                                        QString str=tr("Minor problem found and corrected: group %1 has different number of students in two places (%2 and %3)", "%2 and %3 are number of students")
                                                                .arg(group->name).arg(sg->numberOfStudents).arg(group->numberOfStudents)
                                                                +
                                                                "\n\n"+
                                                                tr("Explanation: this is a minor problem, which appears if using overlapping students set, due to a bug in m-FET previous to version %1."
                                                                " m-FET will now correct this problem by setting the number of students for this group, in all places where it appears,"
                                                                " to the number that was found in the first appearance (%2). It is advisable to check the number of students for this group.")
                                                                .arg("5.12.1").arg(sg->numberOfStudents);
                                                        int t=MessagesManager::warning(parent, tr("m-FET warning"), str,
                                                                 tr("Skip rest"), tr("See next"), QString(),
                                                                 1, 0 );

                                                        if(t==0)
                                                                reportWrongNumberOfStudents=false;
                                                }
                                                group->numberOfStudents=sg->numberOfStudents;
                                        }

                                        foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                                                assert(studentsHash.contains(subgroup->name));
                                                StudentsSet* ss=studentsHash.value(subgroup->name);
                                                if(ss->numberOfStudents!=subgroup->numberOfStudents){
                                                        if(reportWrongNumberOfStudents){
                                                                QString str=tr("Minor problem found and corrected: subgroup %1 has different number of students in two places (%2 and %3)", "%2 and %3 are number of students")
                                                                        .arg(subgroup->name).arg(ss->numberOfStudents).arg(subgroup->numberOfStudents)
                                                                        +
                                                                        "\n\n"+
                                                                        tr("Explanation: this is a minor problem, which appears if using overlapping students set, due to a bug in m-FET previous to version %1."
                                                                        " m-FET will now correct this problem by setting the number of students for this subgroup, in all places where it appears,"
                                                                        " to the number that was found in the first appearance (%2). It is advisable to check the number of students for this subgroup.")
                                                                        .arg("5.12.1").arg(ss->numberOfStudents);
                                                                int t=MessagesManager::warning(parent, tr("m-FET warning"), str,
                                                                         tr("Skip rest"), tr("See next"), QString(),
                                                                         1, 0 );

                                                                if(t==0)
                                                                        reportWrongNumberOfStudents=false;
                                                        }
                                                        subgroup->numberOfStudents=ss->numberOfStudents;
                                                }
                                        }
                                }
                        }*/
            //END test for number of students is the same in all sets with the same name

            if(okStudents){
                //This is redundant, but I make this an additional test, just in case anything was wrong.
                computePermanentStudentsHash();
                assert(permanentStudentsHash==currentStudentsHash);
            }
            else{
                foreach(StudentsSet* studentsSet, allAllocatedStudentsSets)
                    delete studentsSet;
                yearsList.clear();
            }
        }
        else if(xmlReader.name()=="Activities_List"){
            QSet<QString> allTeachers;
            QHash<QString, int> studentsSetsCount;
            QSet<QString> allSubjects;
            QSet<QString> allActivityTags;

            foreach(Teacher* tch, this->teachersList)
                allTeachers.insert(tch->name);

            foreach(Subject* sbj, this->subjectsList)
                allSubjects.insert(sbj->name);

            foreach(StudentsYear* year, this->yearsList){
                if(!studentsSetsCount.contains(year->name))
                    studentsSetsCount.insert(year->name, year->numberOfStudents);
                else if(studentsSetsCount.value(year->name)!=year->numberOfStudents){
                    //cout<<"Mistake: year "<<qPrintable(year->name)<<" appears in more places with different number of students"<<endl;
                }

                foreach(StudentsGroup* group, year->groupsList){
                    if(!studentsSetsCount.contains(group->name))
                        studentsSetsCount.insert(group->name, group->numberOfStudents);
                    else if(studentsSetsCount.value(group->name)!=group->numberOfStudents){
                        //cout<<"Mistake: group "<<qPrintable(group->name)<<" appears in more places with different number of students"<<endl;
                    }

                    foreach(StudentsSubgroup* subgroup, group->subgroupsList){
                        if(!studentsSetsCount.contains(subgroup->name))
                            studentsSetsCount.insert(subgroup->name, subgroup->numberOfStudents);
                        else if(studentsSetsCount.value(subgroup->name)!=subgroup->numberOfStudents){
                            //cout<<"Mistake: subgroup "<<qPrintable(subgroup->name)<<" appears in more places with different number of students"<<endl;
                        }
                    }
                }
            }

            //int nchildrennodes=elem2.childNodes().length();

            /*QProgressDialog progress(parent);
                        progress.setLabelText(tr("Loading activities ... please wait"));
                        progress.setRange(0, nchildrennodes);
                        progress.setModal(true);*/
            //progress.setCancelButton(parent);

            //int ttt=0;

            int na=0;
            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){

                /*progress.setValue(ttt);
                                pqapplication->processEvents();
                                if(progress.wasCanceled()){
                                        QMessageBox::information(parent, tr("m-FET information"), tr("Interrupted - only partial file was loaded"));
                                        return true;
                                }

                                ttt++;*/

                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Activity"){
                    bool correct=true;

                    QString cm=QString(""); //comments
                    QString tn="";
                    QStringList tl;
                    QString acn="";
                    QString sjn="";
                    QString atn="";
                    QStringList atl;
                    QString stn="";
                    QStringList stl;
                    bool nt=false;
                    int td=-1;
                    int d=-1;
                    int id=-1;
                    int gid=-1;
                    bool ac=true;
                    int nos=-1;
                    bool cnos=true;

                    QSet<QString> _teachersSet;
                    QSet<QString> _studentsSet;
                    QSet<QString> _activityTagsSet;
                    int _duplicateTeachersCount=0;
                    int _duplicateStudentsCount=0;
                    int _duplicateActivityTagsCount=0;

                    assert(xmlReader.isStartElement());
                    while(xmlReader.readNextStartElement()){
                        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                        if(xmlReader.name()=="Weekly"){
                            xmlReader.skipCurrentElement();
                            xmlReadingLog+="    Current activity is weekly - ignoring tag\n";
                            //assert(p==PARITY_NOT_INITIALIZED);
                            //p=PARITY_WEEKLY;
                        }
                        //old tag
                        else if(xmlReader.name()=="Biweekly"){
                            xmlReader.skipCurrentElement();
                            xmlReadingLog+="    Current activity is fortnightly - ignoring tag\n";
                            //assert(p==PARITY_NOT_INITIALIZED);
                            //p=PARITY_FORTNIGHTLY;
                        }
                        else if(xmlReader.name()=="Fortnightly"){
                            xmlReader.skipCurrentElement();
                            xmlReadingLog+="    Current activity is fortnightly - ignoring tag\n";
                            //assert(p==PARITY_NOT_INITIALIZED);
                            //p=PARITY_FORTNIGHTLY;
                        }
                        else if(xmlReader.name()=="Active"){
                            QString text=xmlReader.readElementText();
                            if(text=="yes" || text=="true" || text=="1"){
                                ac=true;
                                xmlReadingLog+="	Current activity is active\n";
                            }
                            else{
                                if(!(text=="no" || text=="false" || text=="0")){
                                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                                      tr("Found activity active tag which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                                         " The activity will be considered not active",
                                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                                }
                                //assert(text=="no" || text=="false" || text=="0");
                                ac=false;
                                xmlReadingLog+="	Current activity is not active\n";
                            }
                        }
                        else if(xmlReader.name()=="Comments"){
                            QString text=xmlReader.readElementText();
                            cm=text;
                            xmlReadingLog+="    Crt. activity comments="+cm+"\n";
                        }
                        //						else if(xmlReader.name()=="Teacher"){
                        //							QString text=xmlReader.readElementText();
                        //							tn=text;
                        //							xmlReadingLog+="    Crt. activity teacher="+tn+"\n";

                        //							if(_teachersSet.contains(tn))
                        //								_duplicateTeachersCount++;
                        //							else
                        //								_teachersSet.insert(tn);

                        //							tl.append(tn);
                        //							if(!allTeachers.contains(tn))
                        //								correct=false;
                        //						}
                        else if(xmlReader.name()=="Name"){
                            QString text=xmlReader.readElementText();
                            acn=text;
                            xmlReadingLog+="    Crt. activity name="+acn+"\n";
                        }
                        else if(xmlReader.name()=="Subject"){
                            QString text=xmlReader.readElementText();
                            sjn=text;
                            xmlReadingLog+="    Crt. activity subject="+sjn+"\n";
                            if(!allSubjects.contains(sjn))
                                //if(this->searchSubject(sjn)<0)
                                correct=false;
                        }
                        else if(xmlReader.name()=="Subject_Tag"){
                            QString text=xmlReader.readElementText();
                            atn=text;
                            xmlReadingLog+="    Crt. activity activity_tag="+atn+"\n";

                            if(_activityTagsSet.contains(atn))
                                _duplicateActivityTagsCount++;
                            else
                                _activityTagsSet.insert(atn);


                            if(atn!="")
                                atl.append(atn);
                            if(atn!="" && !allActivityTags.contains(atn))
                                //if(atn!="" && this->searchActivityTag(atn)<0)
                                correct=false;
                        }
                        else if(xmlReader.name()=="Activity_Tag"){
                            QString text=xmlReader.readElementText();
                            atn=text;
                            xmlReadingLog+="    Crt. activity activity_tag="+atn+"\n";

                            if(_activityTagsSet.contains(atn))
                                _duplicateActivityTagsCount++;
                            else
                                _activityTagsSet.insert(atn);

                            if(atn!="")
                                atl.append(atn);
                            if(atn!="" && !allActivityTags.contains(atn))
                                //if(atn!="" && this->searchActivityTag(atn)<0)
                                correct=false;
                        }
                        else if(xmlReader.name()=="Students"){
                            QString text=xmlReader.readElementText();
                            stn=text;
                            xmlReadingLog+="    Crt. activity students+="+stn+"\n";

                            if(_studentsSet.contains(stn))
                                _duplicateStudentsCount++;
                            else
                                _studentsSet.insert(stn);

                            stl.append(stn);
                            if(!studentsSetsCount.contains(stn))
                                correct=false;
                        }
                        else if(xmlReader.name()=="Allow_Multiple_Teachers"){
                            QString text=xmlReader.readElementText();
                            if(text=="yes" || text=="true" || text=="1"){
                                nt=true;
                                xmlReadingLog+="    Crt. activity does not allow multiple teachers\n";
                            }
                            else{
                                if(!(text=="no" || text=="false" || text=="0")){
                                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                                      tr("Found activity 'allows multiple teachers' tag which is not 'true', 'false', 'yes', 'no', '1' or '0'.",
                                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                                }
                                nt=false;
                                xmlReadingLog+="    Crt. activity allows multiple teachers\n";
                            }
                        }
                        else if(xmlReader.name()=="Duration"){
                            QString text=xmlReader.readElementText();
                            d=text.toInt();
                            xmlReadingLog+="    Crt. activity duration="+utils::strings::number(d)+"\n";
                        }
                        else if(xmlReader.name()=="Total_Duration"){
                            QString text=xmlReader.readElementText();
                            td=text.toInt();
                            xmlReadingLog+="    Crt. activity total duration="+utils::strings::number(td)+"\n";
                        }
                        else if(xmlReader.name()=="Id"){
                            QString text=xmlReader.readElementText();
                            id=text.toInt();
                            xmlReadingLog+="    Crt. activity id="+utils::strings::number(id)+"\n";
                        }
                        else if(xmlReader.name()=="Activity_Group_Id"){
                            QString text=xmlReader.readElementText();
                            gid=text.toInt();
                            xmlReadingLog+="    Crt. activity group id="+utils::strings::number(gid)+"\n";
                        }
                        else if(xmlReader.name()=="Number_Of_Students"){
                            QString text=xmlReader.readElementText();
                            cnos=false;
                            nos=text.toInt();
                            xmlReadingLog+="    Crt. activity number of students="+utils::strings::number(nos)+"\n";
                        }
                        else{
                            xmlReader.skipCurrentElement();
                            xmlReaderNumberOfUnrecognizedFields++;
                        }
                    }
                    if(id<0)
                        xmlReader.raiseError(tr("%1 is incorrect").arg("Id"));
                    else if(gid<0)
                        xmlReader.raiseError(tr("%1 is incorrect").arg("Activity_Group_Id"));
                    else if(d<=0)
                        xmlReader.raiseError(tr("%1 is incorrect").arg("Duration"));
                    else if(_duplicateTeachersCount>0)
                        xmlReader.raiseError(tr("Activity with Id=%1 contains %2 duplicate teachers - please correct that").arg(id).arg(_duplicateTeachersCount));
                    else if(_duplicateStudentsCount>0)
                        xmlReader.raiseError(tr("Activity with Id=%1 contains %2 duplicate students sets - please correct that").arg(id).arg(_duplicateStudentsCount));
                    else if(_duplicateActivityTagsCount>0)
                        xmlReader.raiseError(tr("Activity with Id=%1 contains %2 duplicate activity tags - please correct that").arg(id).arg(_duplicateActivityTagsCount));
                    else if(correct){
                        assert(id>=0 && gid>=0);
                        assert(d>0);
                        if(td<0)
                            td=d;

                        if(cnos==true){
                            assert(nos==-1);
                            int _ns=0;
                            foreach(QString _s, stl){
                                assert(studentsSetsCount.contains(_s));
                                _ns+=studentsSetsCount.value(_s);
                            }

                            this->addSimpleActivityFast(parent, id, gid, acn, nt, sjn, atl, stl,
                                                        d, td, ac, cnos, nos, _ns);
                        }
                        else{
                            this->addSimpleActivityFast(parent, id, gid, acn, nt, sjn, atl, stl,
                                                        d, td, ac, cnos, nos, nos);
                        }

                        this->activitiesList[this->activitiesList.count()-1]->comments=cm;

                        na++;
                        xmlReadingLog+="   Added the activity\n";
                    }
                    else{
                        xmlReader.raiseError(tr("The activity with id=%1 contains incorrect data").arg(id));
                    }
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            xmlReadingLog+="  Added "+utils::strings::number(na)+" activities\n";
            reducedXmlLog+="Added "+utils::strings::number(na)+" activities\n";
        }
        else if(xmlReader.name()=="Equipments_List"){
            MessagesManager::warning(parent, tr("m-FET warning"),
                                              tr("File contains deprecated equipments list - will be ignored"));
            xmlReader.skipCurrentElement();
            //NOT! xmlReaderNumberOfUnrecognizedFields++; because this entry was once allowed
        }
        else if(xmlReader.name()=="Time_Constraints_List"){
            bool reportMaxBeginningsAtSecondHourChange=true;
            bool reportMaxGapsChange=true;
            bool reportStudentsSetNotAvailableChange=true;
            bool reportTeacherNotAvailableChange=true;
            bool reportBreakChange=true;

            bool reportActivityPreferredTimeChange=true;

            bool reportActivityPreferredTimesChange=true;
            bool reportActivitiesPreferredTimesChange=true;

            bool reportUnspecifiedPermanentlyLockedTime=true;

            bool reportUnspecifiedDayOrHourPreferredStartingTime=true;

#if 0
            bool reportIncorrectMinDays=true;
#endif

            bool seeNextWarnNotAddedTimeConstraint=true;

            int nc=0;
            TimeConstraint *crt_constraint;
            assert(xmlReader.isStartElement());

            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";
                crt_constraint=nullptr;
                if(xmlReader.name()=="ConstraintBasicCompulsoryTime"){
                    crt_constraint=readBasicCompulsoryTime(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherNotAvailable"){
                    if(reportTeacherNotAvailableChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint teacher not available, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint teacher not available times (a matrix)."),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportTeacherNotAvailableChange=false;
                    }

                    crt_constraint=readTeacherNotAvailable(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherNotAvailableTimes"){
                    crt_constraint=readTeacherNotAvailableTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxDaysPerWeek"){
                    crt_constraint=readTeacherMaxDaysPerWeek(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxDaysPerWeek"){
                    crt_constraint=readTeachersMaxDaysPerWeek(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintTeacherMinDaysPerWeek"){
                    crt_constraint=readTeacherMinDaysPerWeek(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMinDaysPerWeek"){
                    crt_constraint=readTeachersMinDaysPerWeek(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintTeacherIntervalMaxDaysPerWeek"){
                    crt_constraint=readTeacherIntervalMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersIntervalMaxDaysPerWeek"){
                    crt_constraint=readTeachersIntervalMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxDaysPerWeek"){
                    crt_constraint=readStudentsSetMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMaxDaysPerWeek"){
                    crt_constraint=readStudentsMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetIntervalMaxDaysPerWeek"){
                    crt_constraint=readStudentsSetIntervalMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsIntervalMaxDaysPerWeek"){
                    crt_constraint=readStudentsIntervalMaxDaysPerWeek(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetNotAvailable"){
                    if(reportStudentsSetNotAvailableChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint students set not available, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint students set not available times (a matrix)."),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportStudentsSetNotAvailableChange=false;
                    }

                    crt_constraint=readStudentsSetNotAvailable(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetNotAvailableTimes"){
                    crt_constraint=readStudentsSetNotAvailableTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMinNDaysBetweenActivities"){
                    crt_constraint=readMinNDaysBetweenActivities(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMinDaysBetweenActivities"){
                    crt_constraint=readMinDaysBetweenActivities(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMaxDaysBetweenActivities"){
                    crt_constraint=readMaxDaysBetweenActivities(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMinGapsBetweenActivities"){
                    crt_constraint=readMinGapsBetweenActivities(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesNotOverlapping"){
                    crt_constraint=readActivitiesNotOverlapping(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesSameStartingTime"){
                    crt_constraint=readActivitiesSameStartingTime(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesSameStartingHour"){
                    crt_constraint=readActivitiesSameStartingHour(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesSameStartingDay"){
                    crt_constraint=readActivitiesSameStartingDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxHoursDaily"){
                    crt_constraint=readTeachersMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxHoursDaily"){
                    crt_constraint=readTeacherMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxHoursContinuously"){
                    crt_constraint=readTeachersMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxHoursContinuously"){
                    crt_constraint=readTeacherMaxHoursContinuously(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintNoTeacherInSubjectPreference"){
                    crt_constraint=readNoSubjectPreferenceForAllTeachers(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintMaxActivitiesForAllTeachers"){
                    crt_constraint=readMaxActivitiesForAllTeachers(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMaxActivitiesForTeacher"){
                    crt_constraint=readMaxActivitiesForATeacher(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintMinActivitiesForAllTeachers"){
                    crt_constraint=readMinActivitiesForAllTeachers(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintMinActivitiesForTeacher"){
                    crt_constraint=readMinActivitiesForATeacher(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintTeachersMaxHoursInWorkDaily"){
                    crt_constraint=readTeachersMaxHoursInWork(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxHoursInWorkDaily"){
                    crt_constraint=readTeacherMaxHoursInWork(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherActivityTagMaxHoursContinuously"){
                    crt_constraint=readTeacherActivityTagMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersActivityTagMaxHoursContinuously"){
                    crt_constraint=readTeachersActivityTagMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherActivityTagMaxHoursDaily"){
                    crt_constraint=readTeacherActivityTagMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersActivityTagMaxHoursDaily"){
                    crt_constraint=readTeachersActivityTagMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMinHoursDaily"){
                    crt_constraint=readTeachersMinHoursDaily(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMinHoursDaily"){
                    crt_constraint=readTeacherMinHoursDaily(parent, xmlReader, xmlReadingLog);
                }
                else if((xmlReader.name()=="ConstraintTeachersSubgroupsMaxHoursDaily"
                         //TODO: erase the line below. It is only kept for compatibility with older versions
                         || xmlReader.name()=="ConstraintTeachersSubgroupsNoMoreThanXHoursDaily") && !skipDeprecatedConstraints){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint teachers subgroups max hours daily - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                else if(xmlReader.name()=="ConstraintStudentsNHoursDaily" && !skipDeprecatedConstraints){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint students n hours daily - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                else if(xmlReader.name()=="ConstraintStudentsSetNHoursDaily" && !skipDeprecatedConstraints){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint students set n hours daily - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                else if(xmlReader.name()=="ConstraintStudentsMaxHoursDaily"){
                    crt_constraint=readStudentsMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxHoursDaily"){
                    crt_constraint=readStudentsSetMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMaxHoursContinuously"){
                    crt_constraint=readStudentsMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxHoursContinuously"){
                    crt_constraint=readStudentsSetMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetActivityTagMaxHoursContinuously"){
                    crt_constraint=readStudentsSetActivityTagMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsActivityTagMaxHoursContinuously"){
                    crt_constraint=readStudentsActivityTagMaxHoursContinuously(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetActivityTagMaxHoursDaily"){
                    crt_constraint=readStudentsSetActivityTagMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsActivityTagMaxHoursDaily"){
                    crt_constraint=readStudentsActivityTagMaxHoursDaily(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMinHoursDaily"){
                    crt_constraint=readStudentsMinHoursDaily(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMinHoursDaily"){
                    crt_constraint=readStudentsSetMinHoursDaily(parent, xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivityPreferredTime"){
                    if(reportActivityPreferredTimeChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains old constraint type activity preferred time, which will be converted"
                                                                       " to the newer similar constraint of this type, constraint activity preferred STARTING time."
                                                                       " This improvement is done in versions 5.5.9 and above"),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportActivityPreferredTimeChange=false;
                    }

                    crt_constraint=readActivityPreferredTime(parent, xmlReader, xmlReadingLog,
                                                             reportUnspecifiedPermanentlyLockedTime, reportUnspecifiedDayOrHourPreferredStartingTime);
                }

                else if(xmlReader.name()=="ConstraintActivityPreferredStartingTime"){
                    crt_constraint=readActivityPreferredStartingTime(parent, xmlReader, xmlReadingLog,
                                                                     reportUnspecifiedPermanentlyLockedTime, reportUnspecifiedDayOrHourPreferredStartingTime);
                }
                else if(xmlReader.name()=="ConstraintActivityEndsStudentsDay"){
                    crt_constraint=readActivityEndsStudentsDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesEndStudentsDay"){
                    crt_constraint=readActivitiesEndStudentsDay(xmlReader, xmlReadingLog);
                }
                //old, with 2 and 3
                else if(xmlReader.name()=="Constraint2ActivitiesConsecutive"){
                    crt_constraint=read2ActivitiesConsecutive(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="Constraint2ActivitiesGrouped"){
                    crt_constraint=read2ActivitiesGrouped(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="Constraint3ActivitiesGrouped"){
                    crt_constraint=read3ActivitiesGrouped(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="Constraint2ActivitiesOrdered"){
                    crt_constraint=read2ActivitiesOrdered(xmlReader, xmlReadingLog);
                }
                //end old
                else if(xmlReader.name()=="ConstraintTwoActivitiesConsecutive"){
                    crt_constraint=readTwoActivitiesConsecutive(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTwoActivitiesGrouped"){
                    crt_constraint=readTwoActivitiesGrouped(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintThreeActivitiesGrouped"){
                    crt_constraint=readThreeActivitiesGrouped(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTwoActivitiesOrdered"){
                    crt_constraint=readTwoActivitiesOrdered(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivityEndsDay" && !skipDeprecatedConstraints ){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint activity ends day - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                else if(xmlReader.name()=="ConstraintActivityPreferredTimes"){
                    if(reportActivityPreferredTimesChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("Your file contains old constraint activity preferred times, which will be converted to"
                                                                       " new equivalent constraint activity preferred starting times. Beginning with m-FET-5.5.9 it is possible"
                                                                       " to specify: 1. the starting times of an activity (constraint activity preferred starting times)"
                                                                       " or: 2. the accepted time slots (constraint activity preferred time slots)."
                                                                       " If what you need is type 2 of this constraint, you will have to add it by yourself from the interface."),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportActivityPreferredTimesChange=false;
                    }

                    crt_constraint=readActivityPreferredTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivityPreferredTimeSlots"){
                    crt_constraint=readActivityPreferredTimeSlots(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivityPreferredStartingTimes"){
                    crt_constraint=readActivityPreferredStartingTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintBreak"){
                    if(reportBreakChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint break, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint break times (a matrix)."),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportBreakChange=false;
                    }

                    crt_constraint=readBreak(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintBreakTimes"){
                    crt_constraint=readBreakTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersNoGaps"){
                    crt_constraint=readTeachersNoGaps(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxGapsPerWeek"){
                    crt_constraint=readTeachersMaxGapsPerWeek(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxGapsPerWeek"){
                    crt_constraint=readTeacherMaxGapsPerWeek(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxGapsPerDay"){
                    crt_constraint=readTeachersMaxGapsPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMaxGapsPerDay"){
                    crt_constraint=readTeacherMaxGapsPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsNoGaps"){
                    if(reportMaxGapsChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint students no gaps, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint students max gaps per week,"
                                                                       " with max gaps=0. If you like, you can modify this constraint to allow"
                                                                       " more gaps per week (normally not accepted in schools)"),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportMaxGapsChange=false;
                    }

                    crt_constraint=readStudentsNoGaps(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetNoGaps"){
                    if(reportMaxGapsChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint students set no gaps, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint students set max gaps per week,"
                                                                       " with max gaps=0. If you like, you can modify this constraint to allow"
                                                                       " more gaps per week (normally not accepted in schools)"),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportMaxGapsChange=false;
                    }

                    crt_constraint=readStudentsSetNoGaps(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMaxGapsPerWeek"){
                    crt_constraint=readStudentsMaxGapsPerWeek(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxGapsPerWeek"){
                    crt_constraint=readStudentsSetMaxGapsPerWeek(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintStudentsMaxGapsPerDay"){
                    crt_constraint=readStudentsMaxGapsPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxGapsPerDay"){
                    crt_constraint=readStudentsSetMaxGapsPerDay(xmlReader, xmlReadingLog);
                }

                else if(xmlReader.name()=="ConstraintStudentsEarly"){
                    if(reportMaxBeginningsAtSecondHourChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint students early, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint students early max beginnings at second hour,"
                                                                       " with max beginnings=0. If you like, you can modify this constraint to allow"
                                                                       " more beginnings at second available hour (above 0 - this will make the timetable easier)"),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportMaxBeginningsAtSecondHourChange=false;
                    }

                    crt_constraint=readStudentsEarly(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsEarlyMaxBeginningsAtSecondHour"){
                    crt_constraint=readStudentsEarlyMaxBeginningsAtSecondHour(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetEarly"){
                    if(reportMaxBeginningsAtSecondHourChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("File contains constraint students set early, which is old (it was improved in m-FET 5.5.0), and will be converted"
                                                                       " to the similar constraint of this type, constraint students set early max beginnings at second hour,"
                                                                       " with max beginnings=0. If you like, you can modify this constraint to allow"
                                                                       " more beginnings at second available hour (above 0 - this will make the timetable easier)"),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportMaxBeginningsAtSecondHourChange=false;
                    }

                    crt_constraint=readStudentsSetEarly(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour"){
                    crt_constraint=readStudentsSetEarlyMaxBeginningsAtSecondHour(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesPreferredTimes"){
                    if(reportActivitiesPreferredTimesChange){
                        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("Your file contains old constraint activities preferred times, which will be converted to"
                                                                       " new equivalent constraint activities preferred starting times. Beginning with m-FET-5.5.9 it is possible"
                                                                       " to specify: 1. the starting times of several activities (constraint activities preferred starting times)"
                                                                       " or: 2. the accepted time slots (constraint activities preferred time slots)."
                                                                       " If what you need is type 2 of this constraint, you will have to add it by yourself from the interface."),
                                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
                        if(t==0)
                            reportActivitiesPreferredTimesChange=false;
                    }

                    crt_constraint=readActivitiesPreferredTimes(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesPreferredTimeSlots"){
                    crt_constraint=readActivitiesPreferredTimeSlots(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintActivitiesPreferredStartingTimes"){
                    crt_constraint=readActivitiesPreferredStartingTimes(xmlReader, xmlReadingLog);
                }
                ////////////////
                else if(xmlReader.name()=="ConstraintSubactivitiesPreferredTimeSlots"){
                    crt_constraint=readSubactivitiesPreferredTimeSlots(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintSubactivitiesPreferredStartingTimes"){
                    crt_constraint=readSubactivitiesPreferredStartingTimes(xmlReader, xmlReadingLog);
                }
                ////////////////2011-09-25
                else if(xmlReader.name()=="ConstraintActivitiesOccupyMaxTimeSlotsFromSelection"){
                    crt_constraint=readActivitiesOccupyMaxTimeSlotsFromSelection(xmlReader, xmlReadingLog);
                }
                ////////////////
                ////////////////2011-09-30
                else if(xmlReader.name()=="ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots"){
                    crt_constraint=readActivitiesMaxSimultaneousInSelectedTimeSlots(xmlReader, xmlReadingLog);
                }
                ////////////////

                else if(xmlReader.name()=="ConstraintTeachersSubjectTagsMaxHoursContinuously" && !skipDeprecatedConstraints){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint teachers subject tags max hours continuously - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                else if(xmlReader.name()=="ConstraintTeachersSubjectTagMaxHoursContinuously" && !skipDeprecatedConstraints){
                    int t=MessagesManager::confirmation(parent, tr("m-FET warning"),
                                                            tr("File contains deprecated constraint teachers subject tag max hours continuously - will be ignored\n"),
                                                            tr("Skip rest"), tr("See next"), QString(),
                                                            1, 0 );

                    if(t==0)
                        skipDeprecatedConstraints=true;
                    crt_constraint=nullptr;
                    xmlReader.skipCurrentElement();
                }
                /////////begin 2017-02-07
                else if(xmlReader.name()=="ConstraintTeacherMaxSpanPerDay"){
                    crt_constraint=readTeacherMaxSpanPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMaxSpanPerDay"){
                    crt_constraint=readTeachersMaxSpanPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMaxSpanPerDay"){
                    crt_constraint=readStudentsSetMaxSpanPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMaxSpanPerDay"){
                    crt_constraint=readStudentsMaxSpanPerDay(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeacherMinRestingHours"){
                    crt_constraint=readTeacherMinRestingHours(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintTeachersMinRestingHours"){
                    crt_constraint=readTeachersMinRestingHours(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsSetMinRestingHours"){
                    crt_constraint=readStudentsSetMinRestingHours(xmlReader, xmlReadingLog);
                }
                else if(xmlReader.name()=="ConstraintStudentsMinRestingHours"){
                    crt_constraint=readStudentsMinRestingHours(xmlReader, xmlReadingLog);
                }
                /////////  end 2017-02-07
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }

                //corruptConstraintTime:
                //here we skip an invalid constraint or add a valid one
                if(crt_constraint!=nullptr){
                    assert(crt_constraint!=nullptr);
                    bool tmp=this->addTimeConstraint(crt_constraint);
                    if(!tmp){
                        if(seeNextWarnNotAddedTimeConstraint){
                            int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                                    tr("Constraint\n%1\nnot added - must be a duplicate").
                                                                    arg(crt_constraint->getDetailedDescription()), tr("Skip rest"), tr("See next"), QString(""), 1, 0);
                            if(t==0)
                                seeNextWarnNotAddedTimeConstraint=false;
                        }
                        delete crt_constraint;
                    }
                    else
                        nc++;
                }
            }
            xmlReadingLog+="  Added "+utils::strings::number(nc)+" time constraints\n";
            reducedXmlLog+="Added "+utils::strings::number(nc)+" time constraints\n";
        }
        //        else if(xmlReader.name()=="Timetable_Generation_Options_List"){
        //			int tgol=0;
        //			assert(xmlReader.isStartElement());
        //			while(xmlReader.readNextStartElement()){
        //				xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";

        //				if(xmlReader.name()=="GroupActivitiesInInitialOrder"){
        //					tgol++;
        //					GroupActivitiesInInitialOrderItem* item=new GroupActivitiesInInitialOrderItem();
        //					int nActs=-1;
        //					assert(xmlReader.isStartElement());
        //					while(xmlReader.readNextStartElement()){
        //						xmlReadingLog+="   Found "+xmlReader.name().toString()+" tag\n";

        //						if(xmlReader.name()=="Number_of_Activities"){
        //							QString text=xmlReader.readElementText();
        //							nActs=text.toInt();
        //							xmlReadingLog+="    Read n_activities="+utils::strings::number(nActs)+"\n";
        //						}
        //						else if(xmlReader.name()=="Activity_Id"){
        //							QString text=xmlReader.readElementText();
        //							int id=text.toInt();
        //							xmlReadingLog+="    Activity id="+utils::strings::number(id)+"\n";
        //							item->ids.append(id);
        //						}
        //						else if(xmlReader.name()=="Active"){
        //							QString text=xmlReader.readElementText();
        //							if(text=="false"){
        //								item->active=false;
        //							}
        //						}
        //						else if(xmlReader.name()=="Comments"){
        //							QString text=xmlReader.readElementText();
        //							item->comments=text;
        //						}
        //						else{
        //							xmlReader.skipCurrentElement();
        //							xmlReaderNumberOfUnrecognizedFields++;
        //						}
        //					}
        //					if(!(nActs==item->ids.count())){
        //						xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        //						delete item;
        //					}
        //					else{
        //						assert(nActs==item->ids.count());
        //						groupActivitiesInInitialOrderList.append(item);
        //					}
        //				}
        //				else{
        //					xmlReader.skipCurrentElement();
        //					xmlReaderNumberOfUnrecognizedFields++;
        //				}
        //			}
        //			xmlReadingLog+="  Added "+utils::strings::number(tgol)+" timetable generation options\n";
        //			reducedXmlLog+="Added "+utils::strings::number(tgol)+" timetable generation options\n";
        //		}
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    if(xmlReader.error()){
        MessagesManager::information(parent, tr("m-FET warning"),
                                            tr("Could not read file - XML parse error at line %1, column %2:\n%3", "The error description is %3")
                                            .arg(xmlReader.lineNumber())
                                            .arg(xmlReader.columnNumber())
                                            .arg(xmlReader.errorString()));

        file.close();
        return false;
    }
    file.close();

    this->internalStructureComputed=false;

    /*reducedXmlLog+="\n";
        reducedXmlLog+="Note: if you have overlapping students sets (years or groups), a group or a subgroup may be counted more than once. "
                "A unique group name is counted once for each year it belongs to and a unique subgroup name is counted once for each year+group it belongs to.\n";*/

    if(xmlReaderNumberOfUnrecognizedFields>0){
        xmlReadingLog+="  Warning: There were "+utils::strings::number(xmlReaderNumberOfUnrecognizedFields)+" unrecognized fields in the input file\n";

        reducedXmlLog+="\n";
        reducedXmlLog+="Warning: There were "+utils::strings::number(xmlReaderNumberOfUnrecognizedFields)+" unrecognized fields in the input file\n";
    }

    if(canWriteLogFile){
        //logStream<<xmlReadingLog;
        logStream<<reducedXmlLog;
    }

    if(file2.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
                                                tr("Saving of logging gave error code %1, which means you cannot see the log of reading the file. Please check your disk free space")
                                                .arg(file2.error()));
    }

    if(canWriteLogFile)
        file2.close();

    ////////////////////////////////////////

    return true;
}

bool Instance::write(QWidget* parent, const QString& filename)
{
    assert(this->initialized);

    //QString s;

    bool exists=false;
    QString filenameTmp=filename;
    if(QFile::exists(filenameTmp)){
        exists=true;

        filenameTmp.append(QString(".tmp"));

        if(QFile::exists(filenameTmp)){
            int i=1;
            for(;;){
                QString t2=filenameTmp+utils::strings::number(i);
                if(!QFile::exists(t2)){
                    filenameTmp=t2;
                    break;
                }
                i++;
            }
        }
    }

    assert(!QFile::exists(filenameTmp));

    QFile file(filenameTmp);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        MessagesManager::critical(parent, tr("m-FET critical"),
                                                tr("Cannot open %1 for writing ... please check write permissions of the selected directory or your disk free space. Saving of file aborted").arg(QFileInfo(filenameTmp).fileName()));

        return false;
    }

    QTextStream tos(&file);

    tos.setCodec("UTF-8");
    tos.setGenerateByteOrderMark(true);
    //tos.setEncoding(QTextStream::UnicodeUTF8);

    tos<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";

    //	tos<<"<!DOCTYPE m-FET><m-FET version=\""+FET_VERSION+"\">\n\n";
    tos<<"<m-fet version=\""+FET_VERSION+"\">\n\n";

    //the institution name and the comments
    tos<<"<Institution_Name>"+utils::strings::parseStrForXml(this->institutionName)+"</Institution_Name>\n\n";
    tos<<"<Comments>"+utils::strings::parseStrForXml(this->comments)+"</Comments>\n\n";

    //the days and the hours
    tos<<"<Days_List>\n<Number_of_Days>"+utils::strings::number(this->nDaysPerWeek)+"</Number_of_Days>\n";
    for(int i=0; i<this->nDaysPerWeek; i++){
        tos<<"<Day>\n";
        tos<<"	<Name>"+utils::strings::parseStrForXml(this->daysOfTheWeek[i])+"</Name>\n";
        tos<<"</Day>\n";
    }
    tos<<"</Days_List>\n\n";

    tos<<"<Hours_List>\n<Number_of_Hours>"+utils::strings::number(this->nHoursPerDay)+"</Number_of_Hours>\n";
    for(int i=0; i<this->nHoursPerDay; i++){
        tos<<"<Hour>\n";
        tos<<"	<Name>"+utils::strings::parseStrForXml(this->hoursOfTheDay[i])+"</Name>\n";
        tos<<"</Hour>\n";
    }
    tos<<"</Hours_List>\n\n";

    //subjects list - should be before teachers list, because each teacher has a list of associated qualified subjects
    tos << "<Subjects_List>\n";
    for(int i=0; i<this->subjectsList.size(); i++){
        Subject* sbj=this->subjectsList[i];
        tos << sbj->getXmlDescription();
    }
    tos << "</Subjects_List>\n\n";

    //teachers list
    tos << "<Teachers_List>\n";
    for(int i=0; i<this->teachersList.size(); i++){
        Teacher* tch=this->teachersList[i];
        tos << tch->getXmlDescription();
    }
    tos << "</Teachers_List>\n\n";

    //students list
    tos<<"<Students_List>\n";
    for(int i=0; i<this->yearsList.size(); i++){
        StudentsYear* sty=this->yearsList[i];
        tos << sty->getXmlDescription();
    }
    tos<<"</Students_List>\n\n";

    //activities list
    tos << "<Activities_List>\n";
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        tos << act->getXmlDescription();
        tos << "\n";
    }
    tos << "</Activities_List>\n\n";

    //time constraints list
    tos << "<Time_Constraints_List>\n";
    for(int i=0; i<this->timeConstraintsList.size(); i++){
        TimeConstraint* ctr=this->timeConstraintsList[i];
        tos << ctr->getXmlDescription();
    }
    tos << "</Time_Constraints_List>\n\n";

    tos << this->currentGenerationSettings.getXmlDescription();

    //	tos<<"</m-FET>\n";
    tos<<"</m-fet>\n";

    //tos<<s;

    if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
                                                tr("Saved file gave error code %1, which means saving is compromised. Please check your disk free space")
                                                .arg(file.error()));

        file.close();
        return false;
    }

    file.close();

    if(exists){
        bool tf=QFile::remove(filename);
        assert(tf);
        tf=QFile::rename(filenameTmp, filename);
        assert(tf);
    }

    return true;
}

int Instance::activateStudents(const QString& studentsName)
{
    QSet<QString> allSets;

    StudentsSet* studentsSet=this->searchStudentsSet(studentsName);
    assert(studentsSet!=nullptr);
    if(studentsSet->type==STUDENTS_SUBGROUP)
        allSets.insert(studentsName);
    else if(studentsSet->type==STUDENTS_GROUP){
        allSets.insert(studentsName);
        StudentsGroup* g=(StudentsGroup*)studentsSet;
        foreach(StudentsSubgroup* s, g->subgroupsList)
            allSets.insert(s->name);
    }
    else if(studentsSet->type==STUDENTS_YEAR){
        allSets.insert(studentsName);
        StudentsYear* y=(StudentsYear*)studentsSet;
        foreach(StudentsGroup* g, y->groupsList){
            allSets.insert(g->name);
            foreach(StudentsSubgroup* s, g->subgroupsList)
                allSets.insert(s->name);
        }
    }

    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(!act->active){
            foreach(QString studentsSetName, act->studentSetsNames){
                if(allSets.contains(studentsSetName)){
                    count++;
                    act->active=true;
                    break;
                }
            }
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

int Instance::activateSubject(const QString& subjectName)
{
    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(act->subjectName==subjectName){
            if(!act->active)
                count++;
            act->active=true;
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

int Instance::activateActivityTag(const QString& activityTagName)
{
    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(act->activityTagsNames.contains(activityTagName)){
            if(!act->active)
                count++;
            act->active=true;
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

int Instance::deactivateStudents(const QString& studentsName)
{
    QSet<QString> allSets;

    StudentsSet* studentsSet=this->searchStudentsSet(studentsName);
    assert(studentsSet!=nullptr);
    if(studentsSet->type==STUDENTS_SUBGROUP)
        allSets.insert(studentsName);
    else if(studentsSet->type==STUDENTS_GROUP){
        allSets.insert(studentsName);
        StudentsGroup* g=(StudentsGroup*)studentsSet;
        foreach(StudentsSubgroup* s, g->subgroupsList)
            allSets.insert(s->name);
    }
    else if(studentsSet->type==STUDENTS_YEAR){
        allSets.insert(studentsName);
        StudentsYear* y=(StudentsYear*)studentsSet;
        foreach(StudentsGroup* g, y->groupsList){
            allSets.insert(g->name);
            foreach(StudentsSubgroup* s, g->subgroupsList)
                allSets.insert(s->name);
        }
    }

    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(act->active){
            foreach(QString studentsSetName, act->studentSetsNames){
                if(allSets.contains(studentsSetName)){
                    count++;
                    act->active=false;
                    break;
                }
            }
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

int Instance::deactivateSubject(const QString& subjectName)
{
    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(act->subjectName==subjectName){
            if(act->active)
                count++;
            act->active=false;
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

int Instance::deactivateActivityTag(const QString& activityTagName)
{
    int count=0;
    for(int i=0; i<this->activitiesList.size(); i++){
        Activity* act=this->activitiesList[i];
        if(act->activityTagsNames.contains(activityTagName)){
            if(act->active)
                count++;
            act->active=false;
        }
    }

    this->internalStructureComputed=false;
    setRulesModifiedAndOtherThings(this);

    return count;
}

void Instance::makeActivityTagPrintable(const QString& activityTagName)
{
    int i=searchActivityTag(activityTagName);
    assert(i>=0 && i<activityTagsList.count());

    ActivityTag* at=activityTagsList[i];

    if(at->printable==false){
        at->printable=true;

        this->internalStructureComputed=false;
        setRulesModifiedAndOtherThings(this);
    }
}

void Instance::makeActivityTagNotPrintable(const QString& activityTagName)
{
    int i=searchActivityTag(activityTagName);
    assert(i>=0 && i<activityTagsList.count());

    ActivityTag* at=activityTagsList[i];

    if(at->printable==true){
        at->printable=false;

        this->internalStructureComputed=false;
        setRulesModifiedAndOtherThings(this);
    }
}

TimeConstraint* Instance::readBasicCompulsoryTime(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintBasicCompulsoryTime");
    ConstraintBasicCompulsoryTime* cn=new ConstraintBasicCompulsoryTime(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating automatic 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherNotAvailable(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherNotAvailable");

    QList<int> days;
    QList<int> hours;
    QString teacher;
    Enums::ConstraintGroup group(Enums::ConstraintGroup::Essential);
    double weightPercentage=100;
    int d=-1, h1=-1, h2=-1;
    bool active=true;
    QString comments=QString("");
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            weightPercentage=utils::strings::customFETStrToDouble(text);
            if(weightPercentage<0){
                xmlReader.raiseError(tr("Weight percentage incorrect"));
                return nullptr;
            }
            assert(weightPercentage>=0);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            comments=text;
        }
        else if(xmlReader.name()=="Day"){
            QString text=xmlReader.readElementText();
            for(d=0; d<this->nDaysPerWeek; d++)
                if(this->daysOfTheWeek[d]==text)
                    break;
            if(d>=this->nDaysPerWeek){
                xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeacherNotAvailable day corrupt for teacher %1, day %2 is inexistent ... ignoring constraint")
                                        .arg(teacher)
                                        .arg(text));*/
                //cn=nullptr;

                return nullptr;
            }
            assert(d<this->nDaysPerWeek);
            xmlReadingLog+="    Crt. day="+this->daysOfTheWeek[d]+"\n";
        }
        else if(xmlReader.name()=="Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1==this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is the last hour - impossible").arg(text));
                return nullptr;
            }
            else if(h1>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeacherNotAvailable start hour corrupt for teacher %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(teacher)
                                        .arg(text));*/
                //cn=nullptr;

                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Start hour="+this->hoursOfTheDay[h1]+"\n";
        }
        else if(xmlReader.name()=="End_Hour"){
            QString text=xmlReader.readElementText();
            for(h2=0; h2 < this->nHoursPerDay; h2++)
                if(this->hoursOfTheDay[h2]==text)
                    break;
            if(h2==0){
                xmlReader.raiseError(tr("Hour %1 is the first hour - impossible").arg(text));
                return nullptr;
            }
            else if(h2>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                return nullptr;
            }
            /*if(h2<=0 || h2>this->nHoursPerDay){
                                MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeacherNotAvailable end hour corrupt for teacher %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(teacher)
                                        .arg(text));

                                return nullptr;
                        }*/
            assert(h2>0 && h2 <= this->nHoursPerDay);
            xmlReadingLog+="    End hour="+this->hoursOfTheDay[h2]+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            teacher=text;
            xmlReadingLog+="    Read teacher name="+teacher+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    assert(weightPercentage>=0);
    if(d<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Day"));
        return nullptr;
    }
    else if(h1<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Start_Hour"));
        return nullptr;
    }
    else if(h2<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("End_Hour"));
        return nullptr;
    }
    assert(d>=0 && h1>=0 && h2>=0);

    ConstraintTeacherNotAvailableTimes* cn = nullptr;

    bool found=false;
    foreach(TimeConstraint* c, this->timeConstraintsList)
        if(c->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
            ConstraintTeacherNotAvailableTimes* tna=(ConstraintTeacherNotAvailableTimes*) c;
            if(tna->teacher==teacher){
                found=true;

                for(int hh=h1; hh<h2; hh++){
                    int k;
                    for(k=0; k<tna->days.count(); k++)
                        if(tna->days.at(k)==d && tna->hours.at(k)==hh)
                            break;
                    if(k==tna->days.count()){
                        tna->days.append(d);
                        tna->hours.append(hh);
                    }
                }

                assert(tna->days.count()==tna->hours.count());
            }
        }
    if(!found){
        days.clear();
        hours.clear();
        for(int hh=h1; hh<h2; hh++){
            days.append(d);
            hours.append(hh);
        }

        cn=new ConstraintTeacherNotAvailableTimes(*this, group, weightPercentage, teacher, days, hours);
        cn->active=active;
        cn->comments=comments;

        return cn;
    }
    else
        return nullptr;
}

TimeConstraint* Instance::readTeacherNotAvailableTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherNotAvailableTimes");

    ConstraintTeacherNotAvailableTimes* cn=new ConstraintTeacherNotAvailableTimes(*this);
    int nNotAvailableSlots=-1;
    int i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }

        else if(xmlReader.name()=="Number_of_Not_Available_Times"){
            QString text=xmlReader.readElementText();
            nNotAvailableSlots=text.toInt();
            xmlReadingLog+="    Read number of not available times="+utils::strings::number(nNotAvailableSlots)+"\n";
        }

        else if(xmlReader.name()=="Not_Available_Time"){
            xmlReadingLog+="    Read: not available time\n";

            int d=-1;
            int h=-1;

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Day"){
                    QString text=xmlReader.readElementText();
                    for(d=0; d<this->nDaysPerWeek; d++)
                        if(this->daysOfTheWeek[d]==text)
                            break;

                    if(d>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint TeacherNotAvailableTimes day corrupt for teacher %1, day %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacher)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(d<this->nDaysPerWeek);
                    xmlReadingLog+="    Day="+this->daysOfTheWeek[d]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Hour"){
                    QString text=xmlReader.readElementText();
                    for(h=0; h < this->nHoursPerDay; h++)
                        if(this->hoursOfTheDay[h]==text)
                            break;

                    if(h>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint TeacherNotAvailableTimes hour corrupt for teacher %1, hour %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacher)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(h>=0 && h < this->nHoursPerDay);
                    xmlReadingLog+="    Hour="+this->hoursOfTheDay[h]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            i++;

            cn->days.append(d);
            cn->hours.append(h);

            if(d==-1 || h==-1){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Not_Available_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
        }
        else if(xmlReader.name()=="Teacher"){
            QString text=xmlReader.readElementText();
            cn->teacher=text;
            xmlReadingLog+="    Read teacher name="+cn->teacher+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    assert(i==cn->days.count() && i==cn->hours.count());
    if(!(i==nNotAvailableSlots)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Not_Available_Times").arg("Not_Available_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==nNotAvailableSlots);

    return cn;
}

TimeConstraint* Instance::readTeacherMaxDaysPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxDaysPerWeek");

    ConstraintTeacherMaxDaysPerWeek* cn=new ConstraintTeacherMaxDaysPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek<=0 || cn->maxDaysPerWeek>this->nDaysPerWeek){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Max_Days_Per_Week"));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeacherMaxDaysPerWeek max days corrupt for teacher %1, max days %2 <= 0 or >nDaysPerWeek, ignoring constraint")
                                        .arg(cn->teacherName)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMaxDaysPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxDaysPerWeek");

    ConstraintTeachersMaxDaysPerWeek* cn=new ConstraintTeachersMaxDaysPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek<=0 || cn->maxDaysPerWeek>this->nDaysPerWeek){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Max_Days_Per_Week"));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeachersMaxDaysPerWeek max days corrupt, max days %1 <= 0 or >nDaysPerWeek, ignoring constraint")
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMinDaysPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMinDaysPerWeek");

    ConstraintTeacherMinDaysPerWeek* cn=new ConstraintTeacherMinDaysPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Minimum_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->minDaysPerWeek=text.toInt();
            if(cn->minDaysPerWeek<=0 || cn->minDaysPerWeek>this->nDaysPerWeek){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Minimum_Days_Per_Week"));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeacherMinDaysPerWeek min days corrupt for teacher %1, min days %2 <= 0 or >nDaysPerWeek, ignoring constraint")
                                        .arg(cn->teacherName)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->minDaysPerWeek>0 && cn->minDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Min. days per week="+utils::strings::number(cn->minDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMinDaysPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMinDaysPerWeek");

    ConstraintTeachersMinDaysPerWeek* cn=new ConstraintTeachersMinDaysPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Minimum_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->minDaysPerWeek=text.toInt();
            if(cn->minDaysPerWeek<=0 || cn->minDaysPerWeek>this->nDaysPerWeek){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Minimum_Days_Per_Week"));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint TeachersMinDaysPerWeek min days corrupt, min days %1 <= 0 or >nDaysPerWeek, ignoring constraint")
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->minDaysPerWeek>0 && cn->minDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Min. days per week="+utils::strings::number(cn->minDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherIntervalMaxDaysPerWeek");
    ConstraintTeacherIntervalMaxDaysPerWeek* cn=new ConstraintTeacherIntervalMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    cn->startHour=this->nHoursPerDay;
    cn->endHour=this->nHoursPerDay;
    int h1, h2;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint TeacherIntervalMaxDaysPerWeek max days corrupt for teacher %1, max days %2 >nDaysPerWeek, constraint added, please correct constraint")
                                                      .arg(cn->teacherName)
                                                      .arg(text));
            }
            //assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else if(xmlReader.name()=="Interval_Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint Teacher interval max days per week start hour corrupt for teacher %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(cn->teacherName)
                                        .arg(text));*/
                delete cn;
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Interval start hour="+this->hoursOfTheDay[h1]+"\n";
            cn->startHour=h1;
        }
        else if(xmlReader.name()=="Interval_End_Hour"){
            QString text=xmlReader.readElementText();
            if(text==""){
                xmlReadingLog+="    Interval end hour void, meaning end of day\n";
                cn->endHour=this->nHoursPerDay;
            }
            else{
                for(h2=0; h2 < this->nHoursPerDay; h2++)
                    if(this->hoursOfTheDay[h2]==text)
                        break;
                if(h2>=this->nHoursPerDay){
                    xmlReader.raiseError(tr("Hour %1 is inexistent (it is also not void, to specify end of the day)").arg(text));
                    /*MessagesManager::information(parent, tr("m-FET information"),
                                                tr("Constraint Teacher interval max days per week end hour corrupt for teacher %1, hour %2 is inexistent (it is also not void, to specify end of the day) ... ignoring constraint")
                                                .arg(cn->teacherName)
                                                .arg(text));*/
                    delete cn;
                    //cn=nullptr;
                    return nullptr;
                }
                assert(h2>=0 && h2 < this->nHoursPerDay);
                xmlReadingLog+="    Interval end hour="+this->hoursOfTheDay[h2]+"\n";
                cn->endHour=h2;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersIntervalMaxDaysPerWeek");
    ConstraintTeachersIntervalMaxDaysPerWeek* cn=new ConstraintTeachersIntervalMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    cn->startHour=this->nHoursPerDay;
    cn->endHour=this->nHoursPerDay;
    int h1, h2;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        /*else if(xmlReader.name()=="Teacher_Name"){
                        cn->teacherName=text;
                        xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
                }*/
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint TeachersIntervalMaxDaysPerWeek max days corrupt, max days %1 >nDaysPerWeek, constraint added, please correct constraint")
                                                      //.arg(cn->teacherName)
                                                      .arg(text));
            }
            //assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else if(xmlReader.name()=="Interval_Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint Teachers interval max days per week start hour corrupt because hour %1 is inexistent ... ignoring constraint")
                                        //.arg(cn->teacherName)
                                        .arg(text));*/
                delete cn;
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Interval start hour="+this->hoursOfTheDay[h1]+"\n";
            cn->startHour=h1;
        }
        else if(xmlReader.name()=="Interval_End_Hour"){
            QString text=xmlReader.readElementText();
            if(text==""){
                xmlReadingLog+="    Interval end hour void, meaning end of day\n";
                cn->endHour=this->nHoursPerDay;
            }
            else{
                for(h2=0; h2 < this->nHoursPerDay; h2++)
                    if(this->hoursOfTheDay[h2]==text)
                        break;
                if(h2>=this->nHoursPerDay){
                    xmlReader.raiseError(tr("Hour %1 is inexistent (it is also not void, to specify end of the day)").arg(text));
                    /*MessagesManager::information(parent, tr("m-FET information"),
                                                tr("Constraint Teachers interval max days per week end hour corrupt because hour %1 is inexistent (it is also not void, to specify end of the day) ... ignoring constraint")
                                                //.arg(cn->teacherName)
                                                .arg(text));*/
                    delete cn;
                    //cn=nullptr;
                    return nullptr;
                }
                assert(h2>=0 && h2 < this->nHoursPerDay);
                xmlReadingLog+="    Interval end hour="+this->hoursOfTheDay[h2]+"\n";
                cn->endHour=h2;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxDaysPerWeek");
    ConstraintStudentsSetMaxDaysPerWeek* cn=new ConstraintStudentsSetMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students set name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint StudentsSetMaxDaysPerWeek max days corrupt for students set %1, max days %2 >nDaysPerWeek, constraint added, please correct constraint")
                                                      .arg(cn->students)
                                                      .arg(text));
            }
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxDaysPerWeek");
    ConstraintStudentsMaxDaysPerWeek* cn=new ConstraintStudentsMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint StudentsMaxDaysPerWeek max days corrupt, max days %1 >nDaysPerWeek, constraint added, please correct constraint")
                                                      .arg(text));
            }
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetIntervalMaxDaysPerWeek");
    ConstraintStudentsSetIntervalMaxDaysPerWeek* cn=new ConstraintStudentsSetIntervalMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    cn->startHour=this->nHoursPerDay;
    cn->endHour=this->nHoursPerDay;
    int h1, h2;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students set name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint StudentsSetIntervalMaxDaysPerWeek max days corrupt for students set %1, max days %2 >nDaysPerWeek, constraint added, please correct constraint")
                                                      .arg(cn->students)
                                                      .arg(text));
            }
            //assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else if(xmlReader.name()=="Interval_Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint Students set interval max days per week start hour corrupt for students %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(cn->students)
                                        .arg(text));*/
                delete cn;
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Interval start hour="+this->hoursOfTheDay[h1]+"\n";
            cn->startHour=h1;
        }
        else if(xmlReader.name()=="Interval_End_Hour"){
            QString text=xmlReader.readElementText();
            if(text==""){
                xmlReadingLog+="    Interval end hour void, meaning end of day\n";
                cn->endHour=this->nHoursPerDay;
            }
            else{
                for(h2=0; h2 < this->nHoursPerDay; h2++)
                    if(this->hoursOfTheDay[h2]==text)
                        break;
                if(h2>=this->nHoursPerDay){
                    xmlReader.raiseError(tr("Hour %1 is inexistent (it is also not void, to specify end of the day)").arg(text));
                    /*MessagesManager::information(parent, tr("m-FET information"),
                                                tr("Constraint Students set interval max days per week end hour corrupt for students %1, hour %2 is inexistent (it is also not void, to specify end of the day) ... ignoring constraint")
                                                .arg(cn->students)
                                                .arg(text));*/
                    delete cn;
                    //cn=nullptr;
                    return nullptr;
                }
                assert(h2>=0 && h2 < this->nHoursPerDay);
                xmlReadingLog+="    Interval end hour="+this->hoursOfTheDay[h2]+"\n";
                cn->endHour=h2;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsIntervalMaxDaysPerWeek");
    ConstraintStudentsIntervalMaxDaysPerWeek* cn=new ConstraintStudentsIntervalMaxDaysPerWeek(*this);
    cn->maxDaysPerWeek=this->nDaysPerWeek;
    cn->startHour=this->nHoursPerDay;
    cn->endHour=this->nHoursPerDay;
    int h1, h2;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating 100% weight percentage\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        /*else if(xmlReader.name()=="Students"){
                        cn->students=text;
                        xmlReadingLog+="    Read students set name="+cn->students+"\n";
                }*/
        else if(xmlReader.name()=="Max_Days_Per_Week"){
            QString text=xmlReader.readElementText();
            cn->maxDaysPerWeek=text.toInt();
            if(cn->maxDaysPerWeek>this->nDaysPerWeek){
                MessagesManager::information(parent, tr("m-FET information"),
                                                      tr("Constraint StudentsIntervalMaxDaysPerWeek max days corrupt: max days %1 >nDaysPerWeek, constraint added, please correct constraint")
                                                      .arg(text));
            }
            //assert(cn->maxDaysPerWeek>0 && cn->maxDaysPerWeek <= this->nDaysPerWeek);
            xmlReadingLog+="    Max. days per week="+utils::strings::number(cn->maxDaysPerWeek)+"\n";
        }
        else if(xmlReader.name()=="Interval_Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint Students interval max days per week start hour corrupt: hour %1 is inexistent ... ignoring constraint")
                                        //.arg(cn->students)
                                        .arg(text));*/
                delete cn;
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Interval start hour="+this->hoursOfTheDay[h1]+"\n";
            cn->startHour=h1;
        }
        else if(xmlReader.name()=="Interval_End_Hour"){
            QString text=xmlReader.readElementText();
            if(text==""){
                xmlReadingLog+="    Interval end hour void, meaning end of day\n";
                cn->endHour=this->nHoursPerDay;
            }
            else{
                for(h2=0; h2 < this->nHoursPerDay; h2++)
                    if(this->hoursOfTheDay[h2]==text)
                        break;
                if(h2>=this->nHoursPerDay){
                    xmlReader.raiseError(tr("Hour %1 is inexistent (it is also not void, to specify end of the day)").arg(text));
                    /*MessagesManager::information(parent, tr("m-FET information"),
                                                tr("Constraint Students interval max days per week end hour corrupt: hour %1 is inexistent (it is also not void, to specify end of the day) ... ignoring constraint")
                                                //.arg(cn->students)
                                                .arg(text));*/
                    delete cn;
                    //cn=nullptr;
                    return nullptr;
                }
                assert(h2>=0 && h2 < this->nHoursPerDay);
                xmlReadingLog+="    Interval end hour="+this->hoursOfTheDay[h2]+"\n";
                cn->endHour=h2;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetNotAvailable(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetNotAvailable");

    //ConstraintStudentsSetNotAvailableTimes* cn=new ConstraintStudentsSetNotAvailableTimes();
    QList<int> days;
    QList<int> hours;
    QString students;
    Enums::ConstraintGroup group(Enums::ConstraintGroup::Essential);
    double weightPercentage=100;
    int d=-1, h1=-1, h2=-1;
    bool active=true;
    QString comments=QString("");
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            weightPercentage=utils::strings::customFETStrToDouble(text);
            if(weightPercentage<0){
                xmlReader.raiseError(tr("Weight percentage incorrect"));
                return nullptr;
            }
            assert(weightPercentage>=0);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            comments=text;
        }
        else if(xmlReader.name()=="Day"){
            QString text=xmlReader.readElementText();
            for(d=0; d<this->nDaysPerWeek; d++)
                if(this->daysOfTheWeek[d]==text)
                    break;
            if(d>=this->nDaysPerWeek){
                xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint StudentsSetNotAvailable day corrupt for students %1, day %2 is inexistent ... ignoring constraint")
                                        .arg(students)
                                        .arg(text));*/
                //cn=nullptr;
                return nullptr;
            }
            assert(d<this->nDaysPerWeek);
            xmlReadingLog+="    Crt. day="+this->daysOfTheWeek[d]+"\n";
        }
        else if(xmlReader.name()=="Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1==this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is the last hour - impossible").arg(text));
                return nullptr;
            }
            else if(h1>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint StudentsSetNotAvailable start hour corrupt for students set %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(students)
                                        .arg(text));*/
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Start hour="+this->hoursOfTheDay[h1]+"\n";
        }
        else if(xmlReader.name()=="End_Hour"){
            QString text=xmlReader.readElementText();
            for(h2=0; h2 < this->nHoursPerDay; h2++)
                if(this->hoursOfTheDay[h2]==text)
                    break;
            if(h2==0){
                xmlReader.raiseError(tr("Hour %1 is the first hour - impossible").arg(text));
                return nullptr;
            }
            else if(h2>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                return nullptr;
            }
            /*if(h2<=0 || h2>this->nHoursPerDay){
                                MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint StudentsSetNotAvailable end hour corrupt for students %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(students)
                                        .arg(text));
                                return nullptr;
                        }*/
            assert(h2>0 && h2 <= this->nHoursPerDay);
            xmlReadingLog+="    End hour="+this->hoursOfTheDay[h2]+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            students=text;
            xmlReadingLog+="    Read students name="+students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    assert(weightPercentage>=0);
    if(d<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Day"));
        return nullptr;
    }
    else if(h1<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Start_Hour"));
        return nullptr;
    }
    else if(h2<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("End_Hour"));
        return nullptr;
    }
    assert(d>=0 && h1>=0 && h2>=0);

    ConstraintStudentsSetNotAvailableTimes* cn = nullptr;

    bool found=false;
    foreach(TimeConstraint* c, this->timeConstraintsList)
        if(c->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
            ConstraintStudentsSetNotAvailableTimes* ssna=(ConstraintStudentsSetNotAvailableTimes*) c;
            if(ssna->students==students){
                found=true;

                for(int hh=h1; hh<h2; hh++){
                    int k;
                    for(k=0; k<ssna->days.count(); k++)
                        if(ssna->days.at(k)==d && ssna->hours.at(k)==hh)
                            break;
                    if(k==ssna->days.count()){
                        ssna->days.append(d);
                        ssna->hours.append(hh);
                    }
                }

                assert(ssna->days.count()==ssna->hours.count());
            }
        }
    if(!found){
        days.clear();
        hours.clear();
        for(int hh=h1; hh<h2; hh++){
            days.append(d);
            hours.append(hh);
        }

        cn=new ConstraintStudentsSetNotAvailableTimes(*this, group, weightPercentage, students, days, hours);
        cn->active=active;
        cn->comments=comments;

        //crt_constraint=cn;
        return cn;
    }
    else
        return nullptr;
}

TimeConstraint* Instance::readStudentsSetNotAvailableTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetNotAvailableTimes");

    ConstraintStudentsSetNotAvailableTimes* cn=new ConstraintStudentsSetNotAvailableTimes(*this);
    int nNotAvailableSlots=0;
    int i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }

        else if(xmlReader.name()=="Number_of_Not_Available_Times"){
            QString text=xmlReader.readElementText();
            nNotAvailableSlots=text.toInt();
            xmlReadingLog+="    Read number of not available times="+utils::strings::number(nNotAvailableSlots)+"\n";
        }

        else if(xmlReader.name()=="Not_Available_Time"){
            xmlReadingLog+="    Read: not available time\n";

            int d=-1;
            int h=-1;

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Day"){
                    QString text=xmlReader.readElementText();
                    for(d=0; d<this->nDaysPerWeek; d++)
                        if(this->daysOfTheWeek[d]==text)
                            break;

                    if(d>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::information(parent, tr("m-FET information"),
                                                        tr("Constraint StudentsSetNotAvailableTimes day corrupt for students %1, day %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->students)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(d<this->nDaysPerWeek);
                    xmlReadingLog+="    Day="+this->daysOfTheWeek[d]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Hour"){
                    QString text=xmlReader.readElementText();
                    for(h=0; h < this->nHoursPerDay; h++)
                        if(this->hoursOfTheDay[h]==text)
                            break;

                    if(h>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::information(parent, tr("m-FET information"),
                                                        tr("Constraint StudentsSetNotAvailableTimes hour corrupt for students %1, hour %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->students)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(h>=0 && h < this->nHoursPerDay);
                    xmlReadingLog+="    Hour="+this->hoursOfTheDay[h]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            i++;

            cn->days.append(d);
            cn->hours.append(h);

            if(d==-1 || h==-1){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Not_Available_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    assert(i==cn->days.count() && i==cn->hours.count());
    if(!(i==nNotAvailableSlots)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Not_Available_Times").arg("Not_Available_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==nNotAvailableSlots);

    return cn;
}

TimeConstraint* Instance::readMinNDaysBetweenActivities(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMinNDaysBetweenActivities");

    ConstraintMinDaysBetweenActivities* cn=new ConstraintMinDaysBetweenActivities(*this);
    cn->n_activities=0;
    bool foundCISD=false;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating weightPercentage=95%\n";
            cn->weightPercentage=95;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weightPercentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Consecutive_If_Same_Day" || xmlReader.name()=="Adjacent_If_Broken"){
            QString text=xmlReader.readElementText();
            if(text=="yes" || text=="true" || text=="1"){
                cn->consecutiveIfSameDay=true;
                foundCISD=true;
                xmlReadingLog+="    Current constraint has consecutive if same day=true\n";
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint min days between activities with tag consecutive if same day"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="no" || text=="false" || text=="0");
                cn->consecutiveIfSameDay=false;
                foundCISD=true;
                xmlReadingLog+="    Current constraint has consecutive if same day=false\n";
            }
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=95;
                cn->consecutiveIfSameDay=true;
                foundCISD=true;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
                cn->consecutiveIfSameDay=false;
                foundCISD=true;
            }
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else if(xmlReader.name()=="MinDays"){
            QString text=xmlReader.readElementText();
            cn->minDays=text.toInt();
            xmlReadingLog+="    Read MinDays="+utils::strings::number(cn->minDays)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!foundCISD){
        xmlReadingLog+="    Could not find consecutive if same day information - making it true\n";
        cn->consecutiveIfSameDay=true;
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
    /*
#if 0
        if(0 && reportIncorrectMinDays && cn->n_activities > this->nDaysPerWeek){
                QString s=tr("You have a constraint min days between activities with more activities than the number of days per week.");
                s+=" ";
                s+=tr("Constraint is:");
                s+="\n";
                s+=crt_constraint->getDescription(*this);
                s+="\n";
                s+=tr("This is a very bad practice from the way the algorithm of generation works (it slows down the generation and makes it harder to find a solution).");
                s+="\n\n";
                s+=tr("To improve your file, you are advised to remove the corresponding activities and constraint and add activities again, respecting the following rules:");
                s+="\n\n";
                s+=tr("1. If you add 'force consecutive if same day', then couple extra activities in pairs to obtain a number of activities equal to the number of days per week"
                        ". Example: 7 activities with duration 1 in a 5 days week, then transform into 5 activities with durations: 2,2,1,1,1 and add a single container activity with these 5 components"
                        " (possibly raising the weight of added constraint min days between activities up to 100%)");
                s+="\n\n";

                s+=tr("2. If you don't add 'force consecutive if same day', then add a larger activity split into a number of"
                        " activities equal with the number of days per week and the remaining components into other larger split activity."
                        " For example, suppose you need to add 7 activities with duration 1 in a 5 days week. Add 2 larger container activities,"
                        " first one split into 5 activities with duration 1 and second one split into 2 activities with duration 1"
                        " (possibly raising the weight of added constraints min days between activities for each of the 2 containers up to 100%)");

                int t=QMessageBox::warning(parent, tr("m-FET warning"), s,
                        tr("Skip rest"), tr("See next"), QString(),
                        1, 0 );

                if(t==0)
                        reportIncorrectMinDays=false;
        }
#endif
*/
}

TimeConstraint* Instance::readMinDaysBetweenActivities(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMinDaysBetweenActivities");

    ConstraintMinDaysBetweenActivities* cn=new ConstraintMinDaysBetweenActivities(*this);
    cn->n_activities=0;
    bool foundCISD=false;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - generating weightPercentage=95%\n";
            cn->weightPercentage=95;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weightPercentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Consecutive_If_Same_Day" || xmlReader.name()=="Adjacent_If_Broken"){
            QString text=xmlReader.readElementText();
            if(text=="yes" || text=="true" || text=="1"){
                cn->consecutiveIfSameDay=true;
                foundCISD=true;
                xmlReadingLog+="    Current constraint has consecutive if same day=true\n";
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint min days between activities with tag consecutive if same day"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="no" || text=="false" || text=="0");
                cn->consecutiveIfSameDay=false;
                foundCISD=true;
                xmlReadingLog+="    Current constraint has consecutive if same day=false\n";
            }
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=95;
                cn->consecutiveIfSameDay=true;
                foundCISD=true;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
                cn->consecutiveIfSameDay=false;
                foundCISD=true;
            }
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else if(xmlReader.name()=="MinDays"){
            QString text=xmlReader.readElementText();
            cn->minDays=text.toInt();
            xmlReadingLog+="    Read MinDays="+utils::strings::number(cn->minDays)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!foundCISD){
        xmlReadingLog+="    Could not find consecutive if same day information - making it true\n";
        cn->consecutiveIfSameDay=true;
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
    /*
#if 0
        if(0 && reportIncorrectMinDays && cn->n_activities > this->nDaysPerWeek){
                QString s=tr("You have a constraint min days between activities with more activities than the number of days per week.");
                s+=" ";
                s+=tr("Constraint is:");
                s+="\n";
                s+=crt_constraint->getDescription(*this);
                s+="\n";
                s+=tr("This is a very bad practice from the way the algorithm of generation works (it slows down the generation and makes it harder to find a solution).");
                s+="\n\n";
                s+=tr("To improve your file, you are advised to remove the corresponding activities and constraint and add activities again, respecting the following rules:");
                s+="\n\n";
                s+=tr("1. If you add 'force consecutive if same day', then couple extra activities in pairs to obtain a number of activities equal to the number of days per week"
                        ". Example: 7 activities with duration 1 in a 5 days week, then transform into 5 activities with durations: 2,2,1,1,1 and add a single container activity with these 5 components"
                        " (possibly raising the weight of added constraint min days between activities up to 100%)");
                s+="\n\n";

                s+=tr("2. If you don't add 'force consecutive if same day', then add a larger activity split into a number of"
                        " activities equal with the number of days per week and the remaining components into other larger split activity."
                        " For example, suppose you need to add 7 activities with duration 1 in a 5 days week. Add 2 larger container activities,"
                        " first one split into 5 activities with duration 1 and second one split into 2 activities with duration 1"
                        " (possibly raising the weight of added constraints min days between activities for each of the 2 containers up to 100%)");

                int t=QMessageBox::warning(parent, tr("m-FET warning"), s,
                        tr("Skip rest"), tr("See next"), QString(),
                        1, 0 );

                if(t==0)
                        reportIncorrectMinDays=false;
        }
#endif
*/
}

TimeConstraint* Instance::readMaxDaysBetweenActivities(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMaxDaysBetweenActivities");

    ConstraintMaxDaysBetweenActivities* cn=new ConstraintMaxDaysBetweenActivities(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weightPercentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            //cn->activitiesId[n_act]=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else if(xmlReader.name()=="MaxDays"){
            QString text=xmlReader.readElementText();
            cn->maxDays=text.toInt();
            xmlReadingLog+="    Read MaxDays="+utils::strings::number(cn->maxDays)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readMinGapsBetweenActivities(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMinGapsBetweenActivities");
    ConstraintMinGapsBetweenActivities* cn=new ConstraintMinGapsBetweenActivities(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weightPercentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else if(xmlReader.name()=="MinGaps"){
            QString text=xmlReader.readElementText();
            cn->minGaps=text.toInt();
            xmlReadingLog+="    Read MinGaps="+utils::strings::number(cn->minGaps)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readActivitiesNotOverlapping(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesNotOverlapping");
    ConstraintActivitiesNotOverlapping* cn=new ConstraintActivitiesNotOverlapping(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readActivitiesSameStartingTime(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesSameStartingTime");
    ConstraintActivitiesSameStartingTime* cn=new ConstraintActivitiesSameStartingTime(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readActivitiesSameStartingHour(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesSameStartingHour");
    ConstraintActivitiesSameStartingHour* cn=new ConstraintActivitiesSameStartingHour(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readActivitiesSameStartingDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesSameStartingDay");
    ConstraintActivitiesSameStartingDay* cn=new ConstraintActivitiesSameStartingDay(*this);
    cn->n_activities=0;
    int n_act=0;
    cn->activitiesId.clear();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            cn->n_activities=text.toInt();
            xmlReadingLog+="    Read n_activities="+utils::strings::number(cn->n_activities)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            //cn->activitiesId[n_act]=text.toInt();
            cn->activitiesId.append(text.toInt());
            assert(n_act==cn->activitiesId.count()-1);
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesId[n_act])+"\n";
            n_act++;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(n_act==cn->n_activities)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(n_act==cn->n_activities);
    return cn;
}

TimeConstraint* Instance::readTeachersMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxHoursDaily");
    ConstraintTeachersMaxHoursDaily* cn=new ConstraintTeachersMaxHoursDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxHoursDaily");
    ConstraintTeacherMaxHoursDaily* cn=new ConstraintTeacherMaxHoursDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxHoursContinuously");
    ConstraintTeachersMaxHoursContinuously* cn=new ConstraintTeachersMaxHoursContinuously(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxHoursContinuously");
    ConstraintTeacherMaxHoursContinuously* cn=new ConstraintTeacherMaxHoursContinuously(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readNoSubjectPreferenceForAllTeachers(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintNoTeacherInSubjectPreference");
    ConstraintNoTeacherInSubjectPreference* cn=new ConstraintNoTeacherInSubjectPreference(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Preference"){
            QString text=xmlReader.readElementText();

            Enums::SubjectPreference pref = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::SubjectPreference::Very_Low);

            xmlReadingLog+="    Read preference="+utils::strings::number(pref)+"\n";
            cn->pref = pref;
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readTeachersMaxHoursInWork(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxHoursInWorkDaily");
    ConstraintTeachersMaxHoursInWorkDaily* cn=new ConstraintTeachersMaxHoursInWorkDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_InWork"){
            QString text=xmlReader.readElementText();
            cn->maxHoursInWork=text.toInt();
            xmlReadingLog+="    Read Maximum_Hours_InWork="+utils::strings::number(cn->maxHoursInWork)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readTeacherMaxHoursInWork(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxHoursInWorkDaily");
    ConstraintTeacherMaxHoursInWorkDaily* cn=new ConstraintTeacherMaxHoursInWorkDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_InWork"){
            QString text=xmlReader.readElementText();
            cn->maxHoursInWork=text.toInt();
            xmlReadingLog+="    Read Maximum_Hours_InWork="+utils::strings::number(cn->maxHoursInWork)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readMaxActivitiesForAllTeachers(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMaxActivitiesForAllTeachers");
    ConstraintMaxActivitiesForAllTeachers* cn=new ConstraintMaxActivitiesForAllTeachers(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Activities"){
            QString text=xmlReader.readElementText();
            cn->maxActivities=text.toInt();
            xmlReadingLog+="    Read Maximum_Activities="+utils::strings::number(cn->maxActivities)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readMaxActivitiesForATeacher(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMaxActivitiesForTeacher");
    ConstraintMaxActivitiesForTeacher* cn=new ConstraintMaxActivitiesForTeacher(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Activities"){
            QString text=xmlReader.readElementText();
            cn->maxActivities=text.toInt();
            xmlReadingLog+="    Read Maximum_Activities="+utils::strings::number(cn->maxActivities)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readMinActivitiesForAllTeachers(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMinActivitiesForAllTeachers");
    ConstraintMinActivitiesForAllTeachers* cn=new ConstraintMinActivitiesForAllTeachers(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Activities"){
            QString text=xmlReader.readElementText();
            cn->minActivities=text.toInt();
            xmlReadingLog+="    Read Minimum_Activities="+utils::strings::number(cn->minActivities)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint *Instance::readMinActivitiesForATeacher(QXmlStreamReader &xmlReader, FakeString &xmlReadingLog)
{
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintMinActivitiesForTeacher");
    ConstraintMinActivitiesForTeacher* cn=new ConstraintMinActivitiesForTeacher(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Activities"){
            QString text=xmlReader.readElementText();
            cn->minActivities=text.toInt();
            xmlReadingLog+="    Read Minimum_Activities="+utils::strings::number(cn->minActivities)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherActivityTagMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherActivityTagMaxHoursContinuously");
    ConstraintTeacherActivityTagMaxHoursContinuously* cn=new ConstraintTeacherActivityTagMaxHoursContinuously(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersActivityTagMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersActivityTagMaxHoursContinuously");
    ConstraintTeachersActivityTagMaxHoursContinuously* cn=new ConstraintTeachersActivityTagMaxHoursContinuously(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherActivityTagMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherActivityTagMaxHoursDaily");
    ConstraintTeacherActivityTagMaxHoursDaily* cn=new ConstraintTeacherActivityTagMaxHoursDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersActivityTagMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersActivityTagMaxHoursDaily");
    ConstraintTeachersActivityTagMaxHoursDaily* cn=new ConstraintTeachersActivityTagMaxHoursDaily(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMinHoursDaily(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMinHoursDaily");
    ConstraintTeachersMinHoursDaily* cn=new ConstraintTeachersMinHoursDaily(*this);
    cn->allowEmptyDays=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->minHoursDaily=text.toInt();
            xmlReadingLog+="    Read minHoursDaily="+utils::strings::number(cn->minHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Allow_Empty_Days"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Read allow empty days=true\n";
                cn->allowEmptyDays=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint teachers min hours daily with tag allow empty days"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Read allow empty days=false\n";
                cn->allowEmptyDays=false;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMinHoursDaily(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMinHoursDaily");
    ConstraintTeacherMinHoursDaily* cn=new ConstraintTeacherMinHoursDaily(*this);
    cn->allowEmptyDays=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->minHoursDaily=text.toInt();
            xmlReadingLog+="    Read minHoursDaily="+utils::strings::number(cn->minHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Allow_Empty_Days"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Read allow empty days=true\n";
                cn->allowEmptyDays=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint teacher min hours daily with tag allow empty days"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Read allow empty days=false\n";
                cn->allowEmptyDays=false;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxHoursDaily");
    ConstraintStudentsMaxHoursDaily* cn=new ConstraintStudentsMaxHoursDaily(*this);
    cn->maxHoursDaily=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            if(cn->maxHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxHoursDaily");
    ConstraintStudentsSetMaxHoursDaily* cn=new ConstraintStudentsSetMaxHoursDaily(*this);
    cn->maxHoursDaily=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            if(cn->maxHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxHoursContinuously");
    ConstraintStudentsMaxHoursContinuously* cn=new ConstraintStudentsMaxHoursContinuously(*this);
    cn->maxHoursContinuously=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            if(cn->maxHoursContinuously<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Continuously"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursContinuously<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Continuously"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursContinuously>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxHoursContinuously");
    ConstraintStudentsSetMaxHoursContinuously* cn=new ConstraintStudentsSetMaxHoursContinuously(*this);
    cn->maxHoursContinuously=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            if(cn->maxHoursContinuously<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Continuously"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursContinuously<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Continuously"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursContinuously>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetActivityTagMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetActivityTagMaxHoursContinuously");
    ConstraintStudentsSetActivityTagMaxHoursContinuously* cn=new ConstraintStudentsSetActivityTagMaxHoursContinuously(*this);
    cn->maxHoursContinuously=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            if(cn->maxHoursContinuously<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Continuously"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursContinuously<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Continuously"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursContinuously>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsActivityTagMaxHoursContinuously(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsActivityTagMaxHoursContinuously");
    ConstraintStudentsActivityTagMaxHoursContinuously* cn=new ConstraintStudentsActivityTagMaxHoursContinuously(*this);
    cn->maxHoursContinuously=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Continuously"){
            QString text=xmlReader.readElementText();
            cn->maxHoursContinuously=text.toInt();
            if(cn->maxHoursContinuously<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Continuously"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursContinuously="+utils::strings::number(cn->maxHoursContinuously)+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursContinuously<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Continuously"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursContinuously>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetActivityTagMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetActivityTagMaxHoursDaily");
    ConstraintStudentsSetActivityTagMaxHoursDaily* cn=new ConstraintStudentsSetActivityTagMaxHoursDaily(*this);
    cn->maxHoursDaily=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            if(cn->maxHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsActivityTagMaxHoursDaily(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsActivityTagMaxHoursDaily");
    ConstraintStudentsActivityTagMaxHoursDaily* cn=new ConstraintStudentsActivityTagMaxHoursDaily(*this);
    cn->maxHoursDaily=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
        if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Maximum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->maxHoursDaily=text.toInt();
            if(cn->maxHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Maximum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read maxHoursDaily="+utils::strings::number(cn->maxHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->maxHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Maximum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsMinHoursDaily(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMinHoursDaily");
    ConstraintStudentsMinHoursDaily* cn=new ConstraintStudentsMinHoursDaily(*this);
    cn->minHoursDaily=-1;
    cn->allowEmptyDays=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->minHoursDaily=text.toInt();
            if(cn->minHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Minimum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read minHoursDaily="+utils::strings::number(cn->minHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Allow_Empty_Days"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Read allow empty days=true\n";
                cn->allowEmptyDays=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint students min hours daily with tag allow empty days"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Read allow empty days=false\n";
                cn->allowEmptyDays=false;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->minHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Minimum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->minHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetMinHoursDaily(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMinHoursDaily");
    ConstraintStudentsSetMinHoursDaily* cn=new ConstraintStudentsSetMinHoursDaily(*this);
    cn->minHoursDaily=-1;
    cn->allowEmptyDays=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Minimum_Hours_Daily"){
            QString text=xmlReader.readElementText();
            cn->minHoursDaily=text.toInt();
            if(cn->minHoursDaily<0){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Minimum_Hours_Daily"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Read minHoursDaily="+utils::strings::number(cn->minHoursDaily)+"\n";
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Allow_Empty_Days"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Read allow empty days=true\n";
                cn->allowEmptyDays=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint students set min hours daily with tag allow empty days"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Read allow empty days=false\n";
                cn->allowEmptyDays=false;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(cn->minHoursDaily<0){
        xmlReader.raiseError(tr("%1 not found").arg("Minimum_Hours_Daily"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->minHoursDaily>=0);
    return cn;
}

TimeConstraint* Instance::readActivityPreferredTime(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog,
                                                    bool& reportUnspecifiedPermanentlyLockedTime, bool& reportUnspecifiedDayOrHourPreferredStartingTime){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityPreferredTime");

    ConstraintActivityPreferredStartingTime* cn=new ConstraintActivityPreferredStartingTime(*this);
    cn->day = cn->hour = -1;
    cn->permanentlyLocked=false; //default not locked
    bool foundLocked=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Permanently_Locked"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Permanently locked\n";
                cn->permanentlyLocked=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint activity preferred starting time with tag permanently locked"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Not permanently locked\n";
                cn->permanentlyLocked=false;
            }
            foundLocked=true;
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activityId)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Day"){
            QString text=xmlReader.readElementText();
            for(cn->day=0; cn->day<this->nDaysPerWeek; cn->day++)
                if(this->daysOfTheWeek[cn->day]==text)
                    break;
            if(cn->day>=this->nDaysPerWeek){
                xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint ActivityPreferredTime day corrupt for activity with id %1, day %2 is inexistent ... ignoring constraint")
                                        .arg(cn->activityId)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->day<this->nDaysPerWeek);
            xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->day]+"\n";
        }
        else if(xmlReader.name()=="Preferred_Hour"){
            QString text=xmlReader.readElementText();
            for(cn->hour=0; cn->hour < this->nHoursPerDay; cn->hour++)
                if(this->hoursOfTheDay[cn->hour]==text)
                    break;
            if(cn->hour>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::information(parent, tr("m-FET information"),
                                        tr("Constraint ActivityPreferredTime hour corrupt for activity with id %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(cn->activityId)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->hour>=0 && cn->hour < this->nHoursPerDay);
            xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->hour]+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    //crt_constraint=cn;

    if(cn->hour>=0 && cn->day>=0 && !foundLocked && reportUnspecifiedPermanentlyLockedTime){
        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                    tr("Found constraint activity preferred starting time, with unspecified tag"
                                                       " 'permanently locked' - this tag will be set to 'false' by default. You can always modify it"
                                                       " by editing the constraint in the 'Data' menu")+"\n\n"
                                                    +tr("Explanation: starting with version 5.8.0 (January 2009), the constraint"
                                                        " activity preferred starting time has"
                                                        " a new tag, 'permanently locked' (true or false)."
                                                        " It is recommended to make the tag 'permanently locked' true for the constraints you"
                                                        " need to be not modifiable from the 'Timetable' menu"
                                                        " and leave this tag false for the constraints you need to be modifiable from the 'Timetable' menu"
                                                        " (the 'permanently locked' tag can be modified by editing the constraint from the 'Data' menu)."
                                                        " This way, when viewing the timetable"
                                                        " and locking/unlocking some activities, you will not unlock the constraints which"
                                                        " need to be locked all the time."
                                                        ),
                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
        if(t==0)
            reportUnspecifiedPermanentlyLockedTime=false;
    }

    if(cn->hour==-1 || cn->day==-1){
        if(reportUnspecifiedDayOrHourPreferredStartingTime){
            int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Found constraint activity preferred starting time, with unspecified day or hour."
                                                           " This constraint will be transformed into constraint activity preferred starting times (a set of times, not only one)."
                                                           " This change is done in m-FET versions 5.8.1 and higher."
                                                           ),
                                                        tr("Skip rest"), tr("See next"), QString(), 1, 0 );
            if(t==0)
                reportUnspecifiedDayOrHourPreferredStartingTime=false;
        }

        ConstraintActivityPreferredStartingTimes* cgood=new ConstraintActivityPreferredStartingTimes(*this);
        if(cn->day==-1){
            cgood->activityId=cn->activityId;
            cgood->weightPercentage=cn->weightPercentage;
            cgood->nPreferredStartingTimes_L=this->nDaysPerWeek;
            for(int i=0; i<cgood->nPreferredStartingTimes_L; i++){
                /*cgood->days[i]=i;
                                cgood->hours[i]=cn->hour;*/
                cgood->days_L.append(i);
                cgood->hours_L.append(cn->hour);
            }
        }
        else{
            assert(cn->hour==-1);
            cgood->activityId=cn->activityId;
            cgood->weightPercentage=cn->weightPercentage;
            cgood->nPreferredStartingTimes_L=this->nHoursPerDay;
            for(int i=0; i<cgood->nPreferredStartingTimes_L; i++){
                /*cgood->days[i]=cn->day;
                                cgood->hours[i]=i;*/
                cgood->days_L.append(cn->day);
                cgood->hours_L.append(i);
            }
        }

        delete cn;
        return cgood;
    }

    return cn;
}

TimeConstraint* Instance::readActivityPreferredStartingTime(QWidget* parent, QXmlStreamReader& xmlReader, FakeString& xmlReadingLog,
                                                            bool& reportUnspecifiedPermanentlyLockedTime, bool& reportUnspecifiedDayOrHourPreferredStartingTime){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityPreferredStartingTime");
    ConstraintActivityPreferredStartingTime* cn=new ConstraintActivityPreferredStartingTime(*this);
    cn->day = cn->hour = -1;
    cn->permanentlyLocked=false; //default false
    bool foundLocked=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Permanently_Locked"){
            QString text=xmlReader.readElementText();
            if(text=="true" || text=="1" || text=="yes"){
                xmlReadingLog+="    Permanently locked\n";
                cn->permanentlyLocked=true;
            }
            else{
                if(!(text=="no" || text=="false" || text=="0")){
                    MessagesManager::warning(parent, tr("m-FET warning"),
                                                      tr("Found constraint activity preferred starting time with tag permanently locked"
                                                         " which is not 'true', 'false', 'yes', 'no', '1' or '0'."
                                                         " The tag will be considered false",
                                                         "Instructions for translators: please leave the 'true', 'false', 'yes' and 'no' fields untranslated, as they are in English"));
                }
                //assert(text=="false" || text=="0" || text=="no");
                xmlReadingLog+="    Not permanently locked\n";
                cn->permanentlyLocked=false;
            }
            foundLocked=true;
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activityId)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Day"){
            QString text=xmlReader.readElementText();
            for(cn->day=0; cn->day<this->nDaysPerWeek; cn->day++)
                if(this->daysOfTheWeek[cn->day]==text)
                    break;
            if(cn->day>=this->nDaysPerWeek){
                xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint ActivityPreferredStartingTime day corrupt for activity with id %1, day %2 is inexistent ... ignoring constraint")
                                        .arg(cn->activityId)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->day<this->nDaysPerWeek);
            xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->day]+"\n";
        }
        else if(xmlReader.name()=="Preferred_Hour"){
            QString text=xmlReader.readElementText();
            for(cn->hour=0; cn->hour < this->nHoursPerDay; cn->hour++)
                if(this->hoursOfTheDay[cn->hour]==text)
                    break;
            if(cn->hour>=this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint ActivityPreferredStartingTime hour corrupt for activity with id %1, hour %2 is inexistent ... ignoring constraint")
                                        .arg(cn->activityId)
                                        .arg(text));*/
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(cn->hour>=0 && cn->hour < this->nHoursPerDay);
            xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->hour]+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    //crt_constraint=cn;

    if(cn->hour>=0 && cn->day>=0 && !foundLocked && reportUnspecifiedPermanentlyLockedTime){
        int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                    tr("Found constraint activity preferred starting time, with unspecified tag"
                                                       " 'permanently locked' - this tag will be set to 'false' by default. You can always modify it"
                                                       " by editing the constraint in the 'Data' menu")+"\n\n"
                                                    +tr("Explanation: starting with version 5.8.0 (January 2009), the constraint"
                                                        " activity preferred starting time has"
                                                        " a new tag, 'permanently locked' (true or false)."
                                                        " It is recommended to make the tag 'permanently locked' true for the constraints you"
                                                        " need to be not modifiable from the 'Timetable' menu"
                                                        " and leave this tag false for the constraints you need to be modifiable from the 'Timetable' menu"
                                                        " (the 'permanently locked' tag can be modified by editing the constraint from the 'Data' menu)."
                                                        " This way, when viewing the timetable"
                                                        " and locking/unlocking some activities, you will not unlock the constraints which"
                                                        " need to be locked all the time."
                                                        ),
                                                    tr("Skip rest"), tr("See next"), QString(), 1, 0 );
        if(t==0)
            reportUnspecifiedPermanentlyLockedTime=false;
    }

    if(cn->hour==-1 || cn->day==-1){
        if(reportUnspecifiedDayOrHourPreferredStartingTime){
            int t=MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Found constraint activity preferred starting time, with unspecified day or hour."
                                                           " This constraint will be transformed into constraint activity preferred starting times (a set of times, not only one)."
                                                           " This change is done in m-FET versions 5.8.1 and higher."
                                                           ),
                                                        tr("Skip rest"), tr("See next"), QString(), 1, 0 );
            if(t==0)
                reportUnspecifiedDayOrHourPreferredStartingTime=false;
        }

        ConstraintActivityPreferredStartingTimes* cgood=new ConstraintActivityPreferredStartingTimes(*this);
        if(cn->day==-1){
            cgood->activityId=cn->activityId;
            cgood->weightPercentage=cn->weightPercentage;
            cgood->nPreferredStartingTimes_L=this->nDaysPerWeek;
            for(int i=0; i<cgood->nPreferredStartingTimes_L; i++){
                /*cgood->days[i]=i;
                                cgood->hours[i]=cn->hour;*/
                cgood->days_L.append(i);
                cgood->hours_L.append(cn->hour);
            }
        }
        else{
            assert(cn->hour==-1);
            cgood->activityId=cn->activityId;
            cgood->weightPercentage=cn->weightPercentage;
            cgood->nPreferredStartingTimes_L=this->nHoursPerDay;
            for(int i=0; i<cgood->nPreferredStartingTimes_L; i++){
                /*cgood->days[i]=cn->day;
                                cgood->hours[i]=i;*/
                cgood->days_L.append(cn->day);
                cgood->hours_L.append(i);
            }
        }

        delete cn;
        return cgood;
    }

    return cn;
}

TimeConstraint* Instance::readActivityEndsStudentsDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityEndsStudentsDay");
    ConstraintActivityEndsStudentsDay* cn=new ConstraintActivityEndsStudentsDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readActivitiesEndStudentsDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesEndStudentsDay");
    ConstraintActivitiesEndStudentsDay* cn=new ConstraintActivitiesEndStudentsDay(*this);
    cn->teacherName="";
    cn->studentsName="";
    cn->subjectName="";
    cn->activityTagName="";

    //i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->studentsName=text;
            xmlReadingLog+="    Read students name="+cn->studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::read2ActivitiesConsecutive(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="Constraint2ActivitiesConsecutive");
    ConstraintTwoActivitiesConsecutive* cn=new ConstraintTwoActivitiesConsecutive(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::read2ActivitiesGrouped(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="Constraint2ActivitiesGrouped");
    ConstraintTwoActivitiesGrouped* cn=new ConstraintTwoActivitiesGrouped(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::read3ActivitiesGrouped(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="Constraint3ActivitiesGrouped");
    ConstraintThreeActivitiesGrouped* cn=new ConstraintThreeActivitiesGrouped(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else if(xmlReader.name()=="Third_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->thirdActivityId=text.toInt();
            xmlReadingLog+="    Read third activity id="+utils::strings::number(cn->thirdActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::read2ActivitiesOrdered(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="Constraint2ActivitiesOrdered");
    ConstraintTwoActivitiesOrdered* cn=new ConstraintTwoActivitiesOrdered(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTwoActivitiesConsecutive(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTwoActivitiesConsecutive");
    ConstraintTwoActivitiesConsecutive* cn=new ConstraintTwoActivitiesConsecutive(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTwoActivitiesGrouped(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTwoActivitiesGrouped");
    ConstraintTwoActivitiesGrouped* cn=new ConstraintTwoActivitiesGrouped(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readThreeActivitiesGrouped(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintThreeActivitiesGrouped");
    ConstraintThreeActivitiesGrouped* cn=new ConstraintThreeActivitiesGrouped(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else if(xmlReader.name()=="Third_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->thirdActivityId=text.toInt();
            xmlReadingLog+="    Read third activity id="+utils::strings::number(cn->thirdActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTwoActivitiesOrdered(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTwoActivitiesOrdered");
    ConstraintTwoActivitiesOrdered* cn=new ConstraintTwoActivitiesOrdered(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="First_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->firstActivityId=text.toInt();
            xmlReadingLog+="    Read first activity id="+utils::strings::number(cn->firstActivityId)+"\n";
        }
        else if(xmlReader.name()=="Second_Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->secondActivityId=text.toInt();
            xmlReadingLog+="    Read second activity id="+utils::strings::number(cn->secondActivityId)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readActivityPreferredTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityPreferredTimes");

    ConstraintActivityPreferredStartingTimes* cn=new ConstraintActivityPreferredStartingTimes(*this);
    cn->nPreferredStartingTimes_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES; i++){
                cn->days[i] = cn->hours[i] = -1;
        }*/
    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activityId)+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Times"){
            QString text=xmlReader.readElementText();
            cn->nPreferredStartingTimes_L=text.toInt();
            xmlReadingLog+="    Read number of preferred times="+utils::strings::number(cn->nPreferredStartingTimes_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Time"){
            xmlReadingLog+="    Read: preferred time\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Day"){
                    QString text=xmlReader.readElementText();
                    cn->days_L.append(0);
                    assert(cn->days_L.count()-1==i);
                    for(cn->days_L[i]=0; cn->days_L[i]<this->nDaysPerWeek; cn->days_L[i]++)
                        if(this->daysOfTheWeek[cn->days_L[i]]==text)
                            break;

                    if(cn->days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredTimes day corrupt for activity with id %1, day %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->hours_L.append(0);
                    assert(cn->hours_L.count()-1==i);
                    for(cn->hours_L[i]=0; cn->hours_L[i] < this->nHoursPerDay; cn->hours_L[i]++)
                        if(this->hoursOfTheDay[cn->hours_L[i]]==text)
                            break;

                    if(cn->hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredTimes hour corrupt for activity with id %1, hour %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->hours_L[i]>=0 && cn->hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            i++;

            if(!(i==cn->days_L.count()) || !(i==cn->hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->days_L.count());
            assert(i==cn->hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->nPreferredStartingTimes_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Times").arg("Preferred_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->nPreferredStartingTimes_L);
    return cn;
}

TimeConstraint* Instance::readActivityPreferredTimeSlots(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityPreferredTimeSlots");
    ConstraintActivityPreferredTimeSlots* cn=new ConstraintActivityPreferredTimeSlots(*this);
    cn->p_nPreferredTimeSlots_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS; i++){
                cn->p_days[i] = cn->p_hours[i] = -1;
        }*/
    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->p_activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->p_activityId)+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Time_Slots"){
            QString text=xmlReader.readElementText();
            cn->p_nPreferredTimeSlots_L=text.toInt();
            xmlReadingLog+="    Read number of preferred times="+utils::strings::number(cn->p_nPreferredTimeSlots_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Time_Slot"){
            xmlReadingLog+="    Read: preferred time slot\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Day"){
                    QString text=xmlReader.readElementText();
                    cn->p_days_L.append(0);
                    assert(cn->p_days_L.count()-1==i);
                    for(cn->p_days_L[i]=0; cn->p_days_L[i]<this->nDaysPerWeek; cn->p_days_L[i]++)
                        if(this->daysOfTheWeek[cn->p_days_L[i]]==text)
                            break;

                    if(cn->p_days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredTimeSlots day corrupt for activity with id %1, day %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->p_days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->p_hours_L.append(0);
                    assert(cn->p_hours_L.count()-1==i);
                    for(cn->p_hours_L[i]=0; cn->p_hours_L[i] < this->nHoursPerDay; cn->p_hours_L[i]++)
                        if(this->hoursOfTheDay[cn->p_hours_L[i]]==text)
                            break;

                    if(cn->p_hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredTimeSlots hour corrupt for activity with id %1, hour %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_hours_L[i]>=0 && cn->p_hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->p_hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->p_days_L.count()) || !(i==cn->p_hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Time_Slot"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->p_days_L.count());
            assert(i==cn->p_hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->p_nPreferredTimeSlots_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Time_Slots").arg("Preferred_Time_Slot"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->p_nPreferredTimeSlots_L);
    return cn;
}

TimeConstraint* Instance::readActivityPreferredStartingTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivityPreferredStartingTimes");
    ConstraintActivityPreferredStartingTimes* cn=new ConstraintActivityPreferredStartingTimes(*this);
    cn->nPreferredStartingTimes_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES; i++){
                cn->days[i] = cn->hours[i] = -1;
        }*/
    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activityId=text.toInt();
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activityId)+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Starting_Times"){
            QString text=xmlReader.readElementText();
            cn->nPreferredStartingTimes_L=text.toInt();
            xmlReadingLog+="    Read number of preferred starting times="+utils::strings::number(cn->nPreferredStartingTimes_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Starting_Time"){
            xmlReadingLog+="    Read: preferred starting time\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Starting_Day"){
                    QString text=xmlReader.readElementText();
                    cn->days_L.append(0);
                    assert(cn->days_L.count()-1==i);
                    for(cn->days_L[i]=0; cn->days_L[i]<this->nDaysPerWeek; cn->days_L[i]++)
                        if(this->daysOfTheWeek[cn->days_L[i]]==text)
                            break;

                    if(cn->days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredStartingTimes day corrupt for activity with id %1, day %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred starting day="+this->daysOfTheWeek[cn->days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Starting_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->hours_L.append(0);
                    assert(cn->hours_L.count()-1==i);
                    for(cn->hours_L[i]=0; cn->hours_L[i] < this->nHoursPerDay; cn->hours_L[i]++)
                        if(this->hoursOfTheDay[cn->hours_L[i]]==text)
                            break;

                    if(cn->hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivityPreferredStartingTimes hour corrupt for activity with id %1, hour %2 is inexistent ... ignoring constraint")
                                                        .arg(cn->activityId)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->hours_L[i]>=0 && cn->hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred starting hour="+this->hoursOfTheDay[cn->hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->days_L.count()) || !(i==cn->hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Starting_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->days_L.count());
            assert(i==cn->hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->nPreferredStartingTimes_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Starting_Times").arg("Preferred_Starting_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->nPreferredStartingTimes_L);
    return cn;
}

TimeConstraint* Instance::readBreak(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintBreak");

    QList<int> days;
    QList<int> hours;
    Enums::ConstraintGroup group(Enums::ConstraintGroup::Essential);
    double weightPercentage=100;
    int d=-1, h1=-1, h2=-1;
    bool active=true;
    QString comments=QString("");
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            weightPercentage=utils::strings::customFETStrToDouble(text);
            if(weightPercentage<0){
                xmlReader.raiseError(tr("Weight percentage incorrect"));
                return nullptr;
            }
            assert(weightPercentage>=0);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            comments=text;
        }
        else if(xmlReader.name()=="Day"){
            QString text=xmlReader.readElementText();
            for(d=0; d<this->nDaysPerWeek; d++)
                if(this->daysOfTheWeek[d]==text)
                    break;
            if(d>=this->nDaysPerWeek){
                xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint Break day corrupt for day %1 is inexistent ... ignoring constraint")
                                        .arg(text));*/
                //cn=nullptr;
                return nullptr;
            }
            assert(d<this->nDaysPerWeek);
            xmlReadingLog+="    Crt. day="+this->daysOfTheWeek[d]+"\n";
        }
        else if(xmlReader.name()=="Start_Hour"){
            QString text=xmlReader.readElementText();
            for(h1=0; h1 < this->nHoursPerDay; h1++)
                if(this->hoursOfTheDay[h1]==text)
                    break;
            if(h1==this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is the last hour - impossible").arg(text));
                return nullptr;
            }
            else if(h1>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint Break start hour corrupt for hour %1 is inexistent ... ignoring constraint")
                                        .arg(text));*/
                //cn=nullptr;
                return nullptr;
            }
            assert(h1>=0 && h1 < this->nHoursPerDay);
            xmlReadingLog+="    Start hour="+this->hoursOfTheDay[h1]+"\n";
        }
        else if(xmlReader.name()=="End_Hour"){
            QString text=xmlReader.readElementText();
            for(h2=0; h2 < this->nHoursPerDay; h2++)
                if(this->hoursOfTheDay[h2]==text)
                    break;
            if(h2==0){
                xmlReader.raiseError(tr("Hour %1 is the first hour - impossible").arg(text));
                return nullptr;
            }
            else if(h2>this->nHoursPerDay){
                xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                return nullptr;
            }
            /*if(h2<=0 || h2>this->nHoursPerDay){
                                MessagesManager::confirmation(parent, tr("m-FET information"),
                                        tr("Constraint Break end hour corrupt for hour %1 is inexistent ... ignoring constraint")
                                        .arg(text));
                                return nullptr;
                        }*/
            assert(h2>0 && h2 <= this->nHoursPerDay);
            xmlReadingLog+="    End hour="+this->hoursOfTheDay[h2]+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    assert(weightPercentage>=0);
    if(d<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Day"));
        return nullptr;
    }
    else if(h1<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("Start_Hour"));
        return nullptr;
    }
    else if(h2<0){
        xmlReader.raiseError(tr("Field missing: %1").arg("End_Hour"));
        return nullptr;
    }
    assert(d>=0 && h1>=0 && h2>=0);

    ConstraintBreakTimes* cn = nullptr;

    bool found=false;
    foreach(TimeConstraint* c, this->timeConstraintsList)
        if(c->type==CONSTRAINT_BREAK_TIMES){
            ConstraintBreakTimes* tna=(ConstraintBreakTimes*) c;
            if(true){
                found=true;

                for(int hh=h1; hh<h2; hh++){
                    int k;
                    for(k=0; k<tna->days.count(); k++)
                        if(tna->days.at(k)==d && tna->hours.at(k)==hh)
                            break;
                    if(k==tna->days.count()){
                        tna->days.append(d);
                        tna->hours.append(hh);
                    }
                }

                assert(tna->days.count()==tna->hours.count());
            }
        }
    if(!found){
        days.clear();
        hours.clear();
        for(int hh=h1; hh<h2; hh++){
            days.append(d);
            hours.append(hh);
        }

        cn=new ConstraintBreakTimes(*this, group, weightPercentage, days, hours);
        cn->active=active;
        cn->comments=comments;

        return cn;
    }
    else
        return nullptr;
}

TimeConstraint* Instance::readBreakTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintBreakTimes");
    ConstraintBreakTimes* cn=new ConstraintBreakTimes(*this);
    int nNotAvailableSlots=-1;
    int i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Essential);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Read weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }

        else if(xmlReader.name()=="Number_of_Break_Times"){
            QString text=xmlReader.readElementText();
            nNotAvailableSlots=text.toInt();
            xmlReadingLog+="    Read number of break times="+utils::strings::number(nNotAvailableSlots)+"\n";
        }

        else if(xmlReader.name()=="Break_Time"){
            xmlReadingLog+="    Read: not available time\n";

            int d=-1;
            int h=-1;

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Day"){
                    QString text=xmlReader.readElementText();
                    for(d=0; d<this->nDaysPerWeek; d++)
                        if(this->daysOfTheWeek[d]==text)
                            break;

                    if(d>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint BreakTimes day corrupt for day %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(d<this->nDaysPerWeek);
                    xmlReadingLog+="    Day="+this->daysOfTheWeek[d]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Hour"){
                    QString text=xmlReader.readElementText();
                    for(h=0; h < this->nHoursPerDay; h++)
                        if(this->hoursOfTheDay[h]==text)
                            break;

                    if(h>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint BreakTimes hour corrupt for hour %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(h>=0 && h < this->nHoursPerDay);
                    xmlReadingLog+="    Hour="+this->hoursOfTheDay[h]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }
            i++;

            cn->days.append(d);
            cn->hours.append(h);

            if(d==-1 || h==-1){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Break_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==nNotAvailableSlots)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Break_Times").arg("Break_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==nNotAvailableSlots);
    return cn;
}

TimeConstraint* Instance::readTeachersNoGaps(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersNoGaps");
    ConstraintTeachersMaxGapsPerWeek* cn=new ConstraintTeachersMaxGapsPerWeek(*this);
    cn->maxGaps=0;
    //ConstraintTeachersNoGaps* cn=new ConstraintTeachersNoGaps();
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMaxGapsPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxGapsPerWeek");
    ConstraintTeachersMaxGapsPerWeek* cn=new ConstraintTeachersMaxGapsPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMaxGapsPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxGapsPerWeek");
    ConstraintTeacherMaxGapsPerWeek* cn=new ConstraintTeacherMaxGapsPerWeek(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMaxGapsPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxGapsPerDay");
    ConstraintTeachersMaxGapsPerDay* cn=new ConstraintTeachersMaxGapsPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMaxGapsPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxGapsPerDay");
    ConstraintTeacherMaxGapsPerDay* cn=new ConstraintTeacherMaxGapsPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsNoGaps(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsNoGaps");

    ConstraintStudentsMaxGapsPerWeek* cn=new ConstraintStudentsMaxGapsPerWeek(*this);

    cn->maxGaps=0;

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetNoGaps(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetNoGaps");

    ConstraintStudentsSetMaxGapsPerWeek* cn=new ConstraintStudentsSetMaxGapsPerWeek(*this);

    cn->maxGaps=0;

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMaxGapsPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxGapsPerWeek");
    ConstraintStudentsMaxGapsPerWeek* cn=new ConstraintStudentsMaxGapsPerWeek(*this);

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxGapsPerWeek(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxGapsPerWeek");
    ConstraintStudentsSetMaxGapsPerWeek* cn=new ConstraintStudentsSetMaxGapsPerWeek(*this);

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMaxGapsPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxGapsPerDay");
    ConstraintStudentsMaxGapsPerDay* cn=new ConstraintStudentsMaxGapsPerDay(*this);

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxGapsPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxGapsPerDay");
    ConstraintStudentsSetMaxGapsPerDay* cn=new ConstraintStudentsSetMaxGapsPerDay(*this);

    //bool compulsory_read=false;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Gaps"){
            QString text=xmlReader.readElementText();
            cn->maxGaps=text.toInt();
            xmlReadingLog+="    Adding max gaps="+utils::strings::number(cn->maxGaps)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
            //compulsory_read=true;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsEarly(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsEarly");
    ConstraintStudentsEarlyMaxBeginningsAtSecondHour* cn=new ConstraintStudentsEarlyMaxBeginningsAtSecondHour(*this);

    cn->maxBeginningsAtSecondHour=0;

    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsEarlyMaxBeginningsAtSecondHour(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsEarlyMaxBeginningsAtSecondHour");
    ConstraintStudentsEarlyMaxBeginningsAtSecondHour* cn=new ConstraintStudentsEarlyMaxBeginningsAtSecondHour(*this);
    cn->maxBeginningsAtSecondHour=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Beginnings_At_Second_Hour"){
            QString text=xmlReader.readElementText();
            cn->maxBeginningsAtSecondHour=text.toInt();
            if(!(cn->maxBeginningsAtSecondHour>=0)){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Max_Beginnings_At_Second_Hour"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Adding max beginnings at second hour="+utils::strings::number(cn->maxBeginningsAtSecondHour)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(cn->maxBeginningsAtSecondHour>=0)){
        xmlReader.raiseError(tr("%1 not found").arg("Max_Beginnings_At_Second_Hour"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxBeginningsAtSecondHour>=0);
    return cn;
}

TimeConstraint* Instance::readStudentsSetEarly(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetEarly");
    ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* cn=new ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(*this);

    cn->maxBeginningsAtSecondHour=0;

    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetEarlyMaxBeginningsAtSecondHour(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour");
    ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* cn=new ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(*this);
    cn->maxBeginningsAtSecondHour=-1;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Beginnings_At_Second_Hour"){
            QString text=xmlReader.readElementText();
            cn->maxBeginningsAtSecondHour=text.toInt();
            if(!(cn->maxBeginningsAtSecondHour>=0)){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Max_Beginnings_At_Second_Hour"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            xmlReadingLog+="    Adding max beginnings at second hour="+utils::strings::number(cn->maxBeginningsAtSecondHour)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(cn->maxBeginningsAtSecondHour>=0)){
        xmlReader.raiseError(tr("%1 not found").arg("Max_Beginnings_At_Second_Hour"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(cn->maxBeginningsAtSecondHour>=0);
    return cn;
}

TimeConstraint* Instance::readActivitiesPreferredTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesPreferredTimes");

    ConstraintActivitiesPreferredStartingTimes* cn=new ConstraintActivitiesPreferredStartingTimes(*this);
    cn->duration=-1;
    cn->nPreferredStartingTimes_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES; i++){
                cn->days[i] = cn->hours[i] = -1;
        }*/
    cn->teacherName="";
    cn->studentsName="";
    cn->subjectName="";
    cn->activityTagName="";

    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->studentsName=text;
            xmlReadingLog+="    Read students name="+cn->studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Times"){
            QString text=xmlReader.readElementText();
            cn->nPreferredStartingTimes_L=text.toInt();
            xmlReadingLog+="    Read number of preferred times="+utils::strings::number(cn->nPreferredStartingTimes_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Time"){
            xmlReadingLog+="    Read: preferred time\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Day"){
                    QString text=xmlReader.readElementText();
                    cn->days_L.append(0);
                    assert(cn->days_L.count()-1==i);
                    for(cn->days_L[i]=0; cn->days_L[i]<this->nDaysPerWeek; cn->days_L[i]++)
                        if(this->daysOfTheWeek[cn->days_L[i]]==text)
                            break;

                    if(cn->days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimes day corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, day %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->hours_L.append(0);
                    assert(cn->hours_L.count()-1==i);
                    for(cn->hours_L[i]=0; cn->hours_L[i] < this->nHoursPerDay; cn->hours_L[i]++)
                        if(this->hoursOfTheDay[cn->hours_L[i]]==text)
                            break;

                    if(cn->hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimes hour corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, hour %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->hours_L[i]>=0 && cn->hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->days_L.count()) || !(i==cn->hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->days_L.count());
            assert(i==cn->hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->nPreferredStartingTimes_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Times").arg("Preferred_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->nPreferredStartingTimes_L);
    return cn;
}

TimeConstraint* Instance::readActivitiesPreferredTimeSlots(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesPreferredTimeSlots");
    ConstraintActivitiesPreferredTimeSlots* cn=new ConstraintActivitiesPreferredTimeSlots(*this);
    cn->duration=-1;
    cn->p_nPreferredTimeSlots_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS; i++){
                cn->p_days[i] = cn->p_hours[i] = -1;
        }*/
    cn->p_teacherName="";
    cn->p_studentsName="";
    cn->p_subjectName="";
    cn->p_activityTagName="";

    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->p_teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->p_teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->p_studentsName=text;
            xmlReadingLog+="    Read students name="+cn->p_studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->p_subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->p_subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->p_activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->p_activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->p_activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->p_activityTagName+"\n";
        }
        else if(xmlReader.name()=="Duration"){
            QString text=xmlReader.readElementText();
            if(!text.isEmpty()){
                cn->duration=text.toInt();
                xmlReadingLog+="    Read duration="+utils::strings::number(cn->duration)+"\n";
            }
            else{
                cn->duration=-1;
            }
        }
        else if(xmlReader.name()=="Number_of_Preferred_Time_Slots"){
            QString text=xmlReader.readElementText();
            cn->p_nPreferredTimeSlots_L=text.toInt();
            xmlReadingLog+="    Read number of preferred times="+utils::strings::number(cn->p_nPreferredTimeSlots_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Time_Slot"){
            xmlReadingLog+="    Read: preferred time slot\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Day"){
                    QString text=xmlReader.readElementText();
                    cn->p_days_L.append(0);
                    assert(cn->p_days_L.count()-1==i);
                    for(cn->p_days_L[i]=0; cn->p_days_L[i]<this->nDaysPerWeek; cn->p_days_L[i]++)
                        if(this->daysOfTheWeek[cn->p_days_L[i]]==text)
                            break;

                    if(cn->p_days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimeSlots day corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, day %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_teacherName)
                                                        .arg(cn->p_studentsName)
                                                        .arg(cn->p_subjectName)
                                                        .arg(cn->p_activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->p_days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->p_hours_L.append(0);
                    assert(cn->p_hours_L.count()-1==i);
                    for(cn->p_hours_L[i]=0; cn->p_hours_L[i] < this->nHoursPerDay; cn->p_hours_L[i]++)
                        if(this->hoursOfTheDay[cn->p_hours_L[i]]==text)
                            break;

                    if(cn->p_hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimeSlots hour corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, hour %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_teacherName)
                                                        .arg(cn->p_studentsName)
                                                        .arg(cn->p_subjectName)
                                                        .arg(cn->p_activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_hours_L[i]>=0 && cn->p_hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->p_hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->p_days_L.count()) || !(i==cn->p_hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Time_Slot"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->p_days_L.count());
            assert(i==cn->p_hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->p_nPreferredTimeSlots_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Time_Slots").arg("Preferred_Time_Slot"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->p_nPreferredTimeSlots_L);
    return cn;
}

TimeConstraint* Instance::readActivitiesPreferredStartingTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesPreferredStartingTimes");
    ConstraintActivitiesPreferredStartingTimes* cn=new ConstraintActivitiesPreferredStartingTimes(*this);
    cn->duration=-1;
    cn->nPreferredStartingTimes_L=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES; i++){
                cn->days[i] = cn->hours[i] = -1;
        }*/
    cn->teacherName="";
    cn->studentsName="";
    cn->subjectName="";
    cn->activityTagName="";

    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->studentsName=text;
            xmlReadingLog+="    Read students name="+cn->studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Duration"){
            QString text=xmlReader.readElementText();
            if(!text.isEmpty()){
                cn->duration=text.toInt();
                xmlReadingLog+="    Read duration="+utils::strings::number(cn->duration)+"\n";
            }
            else{
                cn->duration=-1;
            }
        }
        else if(xmlReader.name()=="Number_of_Preferred_Starting_Times"){
            QString text=xmlReader.readElementText();
            cn->nPreferredStartingTimes_L=text.toInt();
            xmlReadingLog+="    Read number of preferred starting times="+utils::strings::number(cn->nPreferredStartingTimes_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Starting_Time"){
            xmlReadingLog+="    Read: preferred starting time\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Starting_Day"){
                    QString text=xmlReader.readElementText();
                    cn->days_L.append(0);
                    assert(cn->days_L.count()-1==i);
                    for(cn->days_L[i]=0; cn->days_L[i]<this->nDaysPerWeek; cn->days_L[i]++)
                        if(this->daysOfTheWeek[cn->days_L[i]]==text)
                            break;

                    if(cn->days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredStartingTimes day corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, day %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred starting day="+this->daysOfTheWeek[cn->days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Starting_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->hours_L.append(0);
                    assert(cn->hours_L.count()-1==i);
                    for(cn->hours_L[i]=0; cn->hours_L[i] < this->nHoursPerDay; cn->hours_L[i]++)
                        if(this->hoursOfTheDay[cn->hours_L[i]]==text)
                            break;

                    if(cn->hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredStartingTimes hour corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, hour %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->hours_L[i]>=0 && cn->hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred starting hour="+this->hoursOfTheDay[cn->hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->days_L.count()) || !(i==cn->hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Starting_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->days_L.count());
            assert(i==cn->hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->nPreferredStartingTimes_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Starting_Times").arg("Preferred_Starting_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->nPreferredStartingTimes_L);
    return cn;
}

////////////////
TimeConstraint* Instance::readSubactivitiesPreferredTimeSlots(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintSubactivitiesPreferredTimeSlots");
    ConstraintSubactivitiesPreferredTimeSlots* cn=new ConstraintSubactivitiesPreferredTimeSlots(*this);
    cn->p_nPreferredTimeSlots_L=0;
    cn->componentNumber=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS; i++){
                cn->p_days[i] = cn->p_hours[i] = -1;
        }*/
    cn->p_teacherName="";
    cn->p_studentsName="";
    cn->p_subjectName="";
    cn->p_activityTagName="";

    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Component_Number"){
            QString text=xmlReader.readElementText();
            cn->componentNumber=text.toInt();
            xmlReadingLog+="    Adding component number="+utils::strings::number(cn->componentNumber)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->p_teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->p_teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->p_studentsName=text;
            xmlReadingLog+="    Read students name="+cn->p_studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->p_subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->p_subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->p_activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->p_activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->p_activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->p_activityTagName+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Time_Slots"){
            QString text=xmlReader.readElementText();
            cn->p_nPreferredTimeSlots_L=text.toInt();
            xmlReadingLog+="    Read number of preferred times="+utils::strings::number(cn->p_nPreferredTimeSlots_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Time_Slot"){
            xmlReadingLog+="    Read: preferred time slot\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Day"){
                    QString text=xmlReader.readElementText();
                    cn->p_days_L.append(0);
                    assert(cn->p_days_L.count()-1==i);
                    for(cn->p_days_L[i]=0; cn->p_days_L[i]<this->nDaysPerWeek; cn->p_days_L[i]++)
                        if(this->daysOfTheWeek[cn->p_days_L[i]]==text)
                            break;

                    if(cn->p_days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimeSlots day corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, day %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_teacherName)
                                                        .arg(cn->p_studentsName)
                                                        .arg(cn->p_subjectName)
                                                        .arg(cn->p_activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred day="+this->daysOfTheWeek[cn->p_days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->p_hours_L.append(0);
                    assert(cn->p_hours_L.count()-1==i);
                    for(cn->p_hours_L[i]=0; cn->p_hours_L[i] < this->nHoursPerDay; cn->p_hours_L[i]++)
                        if(this->hoursOfTheDay[cn->p_hours_L[i]]==text)
                            break;

                    if(cn->p_hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredTimeSlots hour corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, hour %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->p_teacherName)
                                                        .arg(cn->p_studentsName)
                                                        .arg(cn->p_subjectName)
                                                        .arg(cn->p_activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->p_hours_L[i]>=0 && cn->p_hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred hour="+this->hoursOfTheDay[cn->p_hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->p_days_L.count()) || !(i==cn->p_hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Time_Slot"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->p_days_L.count());
            assert(i==cn->p_hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->p_nPreferredTimeSlots_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Time_Slots").arg("Preferred_Time_Slot"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->p_nPreferredTimeSlots_L);
    return cn;
}

TimeConstraint* Instance::readSubactivitiesPreferredStartingTimes(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintSubactivitiesPreferredStartingTimes");
    ConstraintSubactivitiesPreferredStartingTimes* cn=new ConstraintSubactivitiesPreferredStartingTimes(*this);
    cn->nPreferredStartingTimes_L=0;
    cn->componentNumber=0;
    int i;
    /*for(i=0; i<MAX_N_CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES; i++){
                cn->days[i] = cn->hours[i] = -1;
        }*/
    cn->teacherName="";
    cn->studentsName="";
    cn->subjectName="";
    cn->activityTagName="";

    i=0;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight"){
            //cn->weight=utils::strings::customFETStrToDouble(text);
            xmlReader.skipCurrentElement();
            xmlReadingLog+="    Ignoring old tag - weight - making weight percentage=100\n";
            cn->weightPercentage=100;
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Component_Number"){
            QString text=xmlReader.readElementText();
            cn->componentNumber=text.toInt();
            xmlReadingLog+="    Adding component number="+utils::strings::number(cn->componentNumber)+"\n";
        }
        else if(xmlReader.name()=="Compulsory"){
            QString text=xmlReader.readElementText();
            if(text=="yes"){
                //cn->compulsory=true;
                xmlReadingLog+="    Ignoring old tag - Current constraint is compulsory\n";
                cn->weightPercentage=100;
            }
            else{
                //cn->compulsory=false;
                xmlReadingLog+="    Old tag - current constraint is not compulsory - making weightPercentage=0%\n";
                cn->weightPercentage=0;
            }
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Students_Name"){
            QString text=xmlReader.readElementText();
            cn->studentsName=text;
            xmlReadingLog+="    Read students name="+cn->studentsName+"\n";
        }
        else if(xmlReader.name()=="Subject_Name"){
            QString text=xmlReader.readElementText();
            cn->subjectName=text;
            xmlReadingLog+="    Read subject name="+cn->subjectName+"\n";
        }
        else if(xmlReader.name()=="Subject_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Activity_Tag_Name"){
            QString text=xmlReader.readElementText();
            cn->activityTagName=text;
            xmlReadingLog+="    Read activity tag name="+cn->activityTagName+"\n";
        }
        else if(xmlReader.name()=="Number_of_Preferred_Starting_Times"){
            QString text=xmlReader.readElementText();
            cn->nPreferredStartingTimes_L=text.toInt();
            xmlReadingLog+="    Read number of preferred starting times="+utils::strings::number(cn->nPreferredStartingTimes_L)+"\n";
        }
        else if(xmlReader.name()=="Preferred_Starting_Time"){
            xmlReadingLog+="    Read: preferred starting time\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Preferred_Starting_Day"){
                    QString text=xmlReader.readElementText();
                    cn->days_L.append(0);
                    assert(cn->days_L.count()-1==i);
                    for(cn->days_L[i]=0; cn->days_L[i]<this->nDaysPerWeek; cn->days_L[i]++)
                        if(this->daysOfTheWeek[cn->days_L[i]]==text)
                            break;

                    if(cn->days_L[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredStartingTimes day corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, day %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->days_L[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Preferred starting day="+this->daysOfTheWeek[cn->days_L[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Preferred_Starting_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->hours_L.append(0);
                    assert(cn->hours_L.count()-1==i);
                    for(cn->hours_L[i]=0; cn->hours_L[i] < this->nHoursPerDay; cn->hours_L[i]++)
                        if(this->hoursOfTheDay[cn->hours_L[i]]==text)
                            break;

                    if(cn->hours_L[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesPreferredStartingTimes hour corrupt for teacher name=%1, students names=%2, subject name=%3, activity tag name=%4, hour %5 is inexistent ... ignoring constraint")
                                                        .arg(cn->teacherName)
                                                        .arg(cn->studentsName)
                                                        .arg(cn->subjectName)
                                                        .arg(cn->activityTagName)
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->hours_L[i]>=0 && cn->hours_L[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Preferred starting hour="+this->hoursOfTheDay[cn->hours_L[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->days_L.count()) || !(i==cn->hours_L.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Preferred_Starting_Time"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->days_L.count());
            assert(i==cn->hours_L.count());
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    if(!(i==cn->nPreferredStartingTimes_L)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Preferred_Starting_Times").arg("Preferred_Starting_Time"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }
    assert(i==cn->nPreferredStartingTimes_L);
    return cn;
}

//2011-09-25
TimeConstraint* Instance::readActivitiesOccupyMaxTimeSlotsFromSelection(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesOccupyMaxTimeSlotsFromSelection");
    ConstraintActivitiesOccupyMaxTimeSlotsFromSelection* cn=new ConstraintActivitiesOccupyMaxTimeSlotsFromSelection(*this);

    int ac=0;
    int tsc=0;
    int i=0;

    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            ac=text.toInt();
            xmlReadingLog+="    Read number of activities="+utils::strings::number(ac)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activitiesIds.append(text.toInt());
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesIds[cn->activitiesIds.count()-1])+"\n";
        }
        else if(xmlReader.name()=="Number_of_Selected_Time_Slots"){
            QString text=xmlReader.readElementText();
            tsc=text.toInt();
            xmlReadingLog+="    Read number of selected time slots="+utils::strings::number(tsc)+"\n";
        }
        else if(xmlReader.name()=="Selected_Time_Slot"){
            xmlReadingLog+="    Read: selected time slot\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Selected_Day"){
                    QString text=xmlReader.readElementText();
                    cn->selectedDays.append(0);
                    assert(cn->selectedDays.count()-1==i);
                    for(cn->selectedDays[i]=0; cn->selectedDays[i]<this->nDaysPerWeek; cn->selectedDays[i]++)
                        if(this->daysOfTheWeek[cn->selectedDays[i]]==text)
                            break;

                    if(cn->selectedDays[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesOccupyMaxTimeSlotsFromSelection day corrupt, day %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->selectedDays[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Selected day="+this->daysOfTheWeek[cn->selectedDays[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Selected_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->selectedHours.append(0);
                    assert(cn->selectedHours.count()-1==i);
                    for(cn->selectedHours[i]=0; cn->selectedHours[i] < this->nHoursPerDay; cn->selectedHours[i]++)
                        if(this->hoursOfTheDay[cn->selectedHours[i]]==text)
                            break;

                    if(cn->selectedHours[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Hour %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr(" Constraint ActivitiesOccupyMaxTimeSlotsFromSelection hour corrupt, hour %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->selectedHours[i]>=0 && cn->selectedHours[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Selected hour="+this->hoursOfTheDay[cn->selectedHours[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->selectedDays.count()) || !(i==cn->selectedHours.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Selected_Time_Slot"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->selectedDays.count());
            assert(i==cn->selectedHours.count());
        }
        else if(xmlReader.name()=="Max_Number_of_Occupied_Time_Slots"){
            QString text=xmlReader.readElementText();
            cn->maxOccupiedTimeSlots=text.toInt();
            xmlReadingLog+="    Read max number of occupied time slots="+utils::strings::number(cn->maxOccupiedTimeSlots)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    if(!(ac==cn->activitiesIds.count())){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }

    if(!(i==tsc)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Selected_Time_Slots").arg("Selected_Time_Slot"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }

    assert(ac==cn->activitiesIds.count());
    assert(i==tsc);
    return cn;
}
////////////////

//2011-09-30
TimeConstraint* Instance::readActivitiesMaxSimultaneousInSelectedTimeSlots(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots");
    ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots* cn=new ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots(*this);

    int ac=0;
    int tsc=0;
    int i=0;

    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Number_of_Activities"){
            QString text=xmlReader.readElementText();
            ac=text.toInt();
            xmlReadingLog+="    Read number of activities="+utils::strings::number(ac)+"\n";
        }
        else if(xmlReader.name()=="Activity_Id"){
            QString text=xmlReader.readElementText();
            cn->activitiesIds.append(text.toInt());
            xmlReadingLog+="    Read activity id="+utils::strings::number(cn->activitiesIds[cn->activitiesIds.count()-1])+"\n";
        }
        else if(xmlReader.name()=="Number_of_Selected_Time_Slots"){
            QString text=xmlReader.readElementText();
            tsc=text.toInt();
            xmlReadingLog+="    Read number of selected time slots="+utils::strings::number(tsc)+"\n";
        }
        else if(xmlReader.name()=="Selected_Time_Slot"){
            xmlReadingLog+="    Read: selected time slot\n";

            assert(xmlReader.isStartElement());
            while(xmlReader.readNextStartElement()){
                xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";
                if(xmlReader.name()=="Selected_Day"){
                    QString text=xmlReader.readElementText();
                    cn->selectedDays.append(0);
                    assert(cn->selectedDays.count()-1==i);
                    for(cn->selectedDays[i]=0; cn->selectedDays[i]<this->nDaysPerWeek; cn->selectedDays[i]++)
                        if(this->daysOfTheWeek[cn->selectedDays[i]]==text)
                            break;

                    if(cn->selectedDays[i]>=this->nDaysPerWeek){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr("Constraint ActivitiesMaxSimultaneousInSelectedTimeSlots day corrupt, day %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->selectedDays[i]<this->nDaysPerWeek);
                    xmlReadingLog+="    Selected day="+this->daysOfTheWeek[cn->selectedDays[i]]+"("+utils::strings::number(i)+")"+"\n";
                }
                else if(xmlReader.name()=="Selected_Hour"){
                    QString text=xmlReader.readElementText();
                    cn->selectedHours.append(0);
                    assert(cn->selectedHours.count()-1==i);
                    for(cn->selectedHours[i]=0; cn->selectedHours[i] < this->nHoursPerDay; cn->selectedHours[i]++)
                        if(this->hoursOfTheDay[cn->selectedHours[i]]==text)
                            break;

                    if(cn->selectedHours[i]>=this->nHoursPerDay){
                        xmlReader.raiseError(tr("Day %1 is inexistent").arg(text));
                        /*MessagesManager::confirmation(parent, tr("m-FET information"),
                                                        tr(" Constraint ActivitiesMaxSimultaneousInSelectedTimeSlots hour corrupt, hour %1 is inexistent ... ignoring constraint")
                                                        .arg(text));*/
                        delete cn;
                        cn=nullptr;
                        return nullptr;
                    }

                    assert(cn->selectedHours[i]>=0 && cn->selectedHours[i] < this->nHoursPerDay);
                    xmlReadingLog+="    Selected hour="+this->hoursOfTheDay[cn->selectedHours[i]]+"\n";
                }
                else{
                    xmlReader.skipCurrentElement();
                    xmlReaderNumberOfUnrecognizedFields++;
                }
            }

            i++;

            if(!(i==cn->selectedDays.count()) || !(i==cn->selectedHours.count())){
                xmlReader.raiseError(tr("%1 is incorrect").arg("Selected_Time_Slot"));
                delete cn;
                cn=nullptr;
                return nullptr;
            }
            assert(i==cn->selectedDays.count());
            assert(i==cn->selectedHours.count());
        }
        else if(xmlReader.name()=="Max_Number_of_Simultaneous_Activities"){
            QString text=xmlReader.readElementText();
            cn->maxSimultaneous=text.toInt();
            xmlReadingLog+="    Read max number of simultaneous activities="+utils::strings::number(cn->maxSimultaneous)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }

    if(!(ac==cn->activitiesIds.count())){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Activities").arg("Activity_Id"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }

    if(!(i==tsc)){
        xmlReader.raiseError(tr("%1 does not coincide with the number of read %2").arg("Number_of_Selected_Time_Slots").arg("Selected_Time_Slot"));
        delete cn;
        cn=nullptr;
        return nullptr;
    }

    assert(ac==cn->activitiesIds.count());
    assert(i==tsc);
    return cn;
}
////////////////

/////2017-02-07
TimeConstraint* Instance::readTeacherMaxSpanPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMaxSpanPerDay");
    ConstraintTeacherMaxSpanPerDay* cn=new ConstraintTeacherMaxSpanPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Max_Span"){
            QString text=xmlReader.readElementText();
            cn->maxSpanPerDay=text.toInt();
            xmlReadingLog+="    Adding max span per day="+utils::strings::number(cn->maxSpanPerDay)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMaxSpanPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMaxSpanPerDay");
    ConstraintTeachersMaxSpanPerDay* cn=new ConstraintTeachersMaxSpanPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Span"){
            QString text=xmlReader.readElementText();
            cn->maxSpanPerDay=text.toInt();
            xmlReadingLog+="    Adding max span per day="+utils::strings::number(cn->maxSpanPerDay)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetMaxSpanPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMaxSpanPerDay");
    ConstraintStudentsSetMaxSpanPerDay* cn=new ConstraintStudentsSetMaxSpanPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Max_Span"){
            QString text=xmlReader.readElementText();
            cn->maxSpanPerDay=text.toInt();
            xmlReadingLog+="    Adding max span per day="+utils::strings::number(cn->maxSpanPerDay)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMaxSpanPerDay(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMaxSpanPerDay");
    ConstraintStudentsMaxSpanPerDay* cn=new ConstraintStudentsMaxSpanPerDay(*this);
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Max_Span"){
            QString text=xmlReader.readElementText();
            cn->maxSpanPerDay=text.toInt();
            xmlReadingLog+="    Adding max span per day="+utils::strings::number(cn->maxSpanPerDay)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeacherMinRestingHours(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeacherMinRestingHours");
    ConstraintTeacherMinRestingHours* cn=new ConstraintTeacherMinRestingHours(*this);
    cn->circular=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Circular"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->circular=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Teacher_Name"){
            QString text=xmlReader.readElementText();
            cn->teacherName=text;
            xmlReadingLog+="    Read teacher name="+cn->teacherName+"\n";
        }
        else if(xmlReader.name()=="Minimum_Resting_Hours"){
            QString text=xmlReader.readElementText();
            cn->minRestingHours=text.toInt();
            xmlReadingLog+="    Adding min resting hours="+utils::strings::number(cn->minRestingHours)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readTeachersMinRestingHours(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintTeachersMinRestingHours");
    ConstraintTeachersMinRestingHours* cn=new ConstraintTeachersMinRestingHours(*this);
    cn->circular=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Circular"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->circular=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Minimum_Resting_Hours"){
            QString text=xmlReader.readElementText();
            cn->minRestingHours=text.toInt();
            xmlReadingLog+="    Adding min resting hours="+utils::strings::number(cn->minRestingHours)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsSetMinRestingHours(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsSetMinRestingHours");
    ConstraintStudentsSetMinRestingHours* cn=new ConstraintStudentsSetMinRestingHours(*this);
    cn->circular=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Circular"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->circular=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Students"){
            QString text=xmlReader.readElementText();
            cn->students=text;
            xmlReadingLog+="    Read students name="+cn->students+"\n";
        }
        else if(xmlReader.name()=="Minimum_Resting_Hours"){
            QString text=xmlReader.readElementText();
            cn->minRestingHours=text.toInt();
            xmlReadingLog+="    Adding min resting hours="+utils::strings::number(cn->minRestingHours)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}

TimeConstraint* Instance::readStudentsMinRestingHours(QXmlStreamReader& xmlReader, FakeString& xmlReadingLog){
    assert(xmlReader.isStartElement() && xmlReader.name()=="ConstraintStudentsMinRestingHours");
    ConstraintStudentsMinRestingHours* cn=new ConstraintStudentsMinRestingHours(*this);
    cn->circular=true;
    while(xmlReader.readNextStartElement()){
        xmlReadingLog+="    Found "+xmlReader.name().toString()+" tag\n";

        if(xmlReader.name()=="Group"){
            QString text=xmlReader.readElementText();

            Enums::ConstraintGroup group = utils::enums::enumStrToEnumValue(text.toStdString(), Enums::ConstraintGroup::Desirable);

            xmlReadingLog+="    Read constraint group="+utils::strings::number(group)+"\n";
            cn->setConstraintGroup(group);
        }
        else if(xmlReader.name()=="Weight_Percentage"){
            QString text=xmlReader.readElementText();
            cn->weightPercentage=utils::strings::customFETStrToDouble(text);
            xmlReadingLog+="    Adding weight percentage="+utils::strings::number(cn->weightPercentage)+"\n";
        }
        else if(xmlReader.name()=="Active"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->active=false;
            }
        }
        else if(xmlReader.name()=="Circular"){
            QString text=xmlReader.readElementText();
            if(text=="false"){
                cn->circular=false;
            }
        }
        else if(xmlReader.name()=="Comments"){
            QString text=xmlReader.readElementText();
            cn->comments=text;
        }
        else if(xmlReader.name()=="Minimum_Resting_Hours"){
            QString text=xmlReader.readElementText();
            cn->minRestingHours=text.toInt();
            xmlReadingLog+="    Adding min resting hours="+utils::strings::number(cn->minRestingHours)+"\n";
        }
        else{
            xmlReader.skipCurrentElement();
            xmlReaderNumberOfUnrecognizedFields++;
        }
    }
    return cn;
}
///////////////

void to_json(json &j, const Instance &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, Instance &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

/*
File solution.cpp
*/

/***************************************************************************
                          solution.cpp  -  description
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

//Teachers free periods code contributed by Volker Dirr (http://timetabling.de/)

#include <QFile>
#include <QTextStream>

#include "defs.h"
#include "solution.h"
#include "instance.h"
#include "timeconstraint.h"

#include "matrix.h"

#include "decimalutils.h"
#include "enumutils.h"
#include "listutils.h"
#include "functionaltutils.h"
#include "stringutils.h"

#include <QMap>
#include <QMultiMap>

#include <QCryptographicHash>

#include "tcontext.h"

using namespace utils;

void Solution::allocateTeacherToAct(int ai, int tchi)
{
//    assert(teachers[ai]==UNALLOCATED_TEACHER);

    assert(ai>=0 and ai < instance->activeActivitiesList.size());
    assert(tchi>=0 and tchi < instance->teachersList.size());

    teachers[ai] = tchi;

    Activity* act = instance->activeActivitiesList[ai];

    if (act->isSplit() and !act->allowMultipleTeachers) {
        for (auto actS : *instance->groupActivitiesPointerHash[act->activityGroupId]){
            int sai = instance->activeActivitiesList.indexOf(actS);
            teachers[sai] = tchi;
        }
    }
}

void Solution::allocateTeacherToAct(int ai, Teacher &tch)
{
    int tchi = instance->getTeacherIndex(tch.name);
    allocateTeacherToAct(ai, tchi);  
}

void Solution::allocateSlotToAct(int ai, TimeSlotId timeSlot)
{
    assert(times[ai]==UNALLOCATED_TIME && timeSlot!=UNALLOCATED_TIME);

    assert(ai>=0 and ai < instance->activeActivitiesList.size());
    assert(timeSlot>=0 and timeSlot < instance->getNTimeSlots());

    times[ai] = timeSlot;
}

double Solution::getViolationsFactorForGroupEssential() const
{
    return _violationsFactorForGroupEssential;
}

float Solution::getAlphaWeightUsed() const
{
    return _alpha_weight_used;
}

Solution::Solution()
{
    makeUnallocated();
}

bool Solution::empty() const
{
    bool aux = true;

    for (int i = 0; i < instance->activeActivitiesList.size(); ++i) {
        if (times[i]!=UNALLOCATED_TIME || teachers[i]!=UNALLOCATED_TEACHER) {
            aux = false;
            break;
        }
    }

    return aux;
}

bool Solution::complete() const
{
    bool aux = true;

    for (int i = 0; i < instance->activeActivitiesList.size(); ++i) {
        if (times[i]==UNALLOCATED_TIME || teachers[i]==UNALLOCATED_TEACHER) {
            aux = false;
            break;
        }
    }

    return aux;
}

bool Solution::viable()
{
    if (!costComputed())
        cost();

    if (decimal::round_down(_violationsFactorForGroupEssential,1)==0.0)
        return true;
    else
        return false;
}

int Solution::getTimeSlotForActivity(int activityIndex)
{
    return this->times[activityIndex];
}

void Solution::setTimeSlotForActivity(int activityIndex, int timeSlot)
{
    this->times[activityIndex] = timeSlot;
}

int Solution::getDayForActivityIndex(int i)
{
    return instance->getDayFromUnifiedTime(this->times[i]);
}

int Solution::getHourForActivityIndex(int i)
{
    return instance->getHourFromUnifiedTime(this->times[i]);
}

bool Solution::isAtivityAllocated(int activityIndex)
{
    return (this->times[activityIndex]!=UNALLOCATED_TIME);
}

void Solution::copy(const Solution& other){//critical function here - must be optimized for speed   
    this->instance=other.instance;

    assert(instance!=nullptr);

    std::copy_n(other.times.begin(), instance->activeActivitiesList.size(), this->times.begin());
    std::copy_n(other.teachers.begin(), instance->activeActivitiesList.size(), this->teachers.begin());

    this->_hash=other._hash;

    this->_alpha_weight_used=other._alpha_weight_used;

    this->_cost=other._cost;
    this->_violationsFactorForGroupEssential=other._violationsFactorForGroupEssential;
    this->_violationsFactorForGroupImportant=other._violationsFactorForGroupImportant;
    this->_violationsFactorForGroupDesirable=other._violationsFactorForGroupDesirable;
    this->_violationsFactorForTchPreferences=other._violationsFactorForTchPreferences;

    this->allocation=other.allocation;
	
    this->constraintConflictData=other.constraintConflictData;
    this->subjectPreferenceData=other.subjectPreferenceData;
	
    this->changedForMatrixCalculation=other.changedForMatrixCalculation;

    this->teachersMatrixReady=false;
    this->subgroupsMatrixReady=false;
    this->teachers_schedule_ready=false;
    this->students_schedule_ready=false;
}

void Solution::init(Instance& instance){
//    assert(instance.internalStructureComputed);

    this->instance = &instance;

    makeUnallocated();
}

void Solution::makeUnallocated(){
//    assert(instance->initialized);
//    assert(instance->internalStructureComputed);

    for(int i=0; i<MAX_ACTIVITIES; i++){
		this->times[i]=UNALLOCATED_TIME;
	}

    for(int i=0; i<MAX_ACTIVITIES; i++){
        this->teachers[i]=UNALLOCATED_TEACHER;
    }

    this->stateChanged();

    this->_alpha_weight_used = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
}

double Solution::cost(float essential_constraint_weight_alpha, bool collectConflictsData){
    assert(instance->initialized);
    assert(instance->internalStructureComputed);

    if(costComputed())
		assert(this->changedForMatrixCalculation==false);
		
    //If you want to see the log, you have to recompute the cost, even if it is already computed
    if(costComputed() && !collectConflictsData && IS_EQUAL(this->_alpha_weight_used, essential_constraint_weight_alpha))
        return this->_cost;

    resetCost();

    this->_alpha_weight_used = essential_constraint_weight_alpha;
			
    this->_cost=0;
    this->_violationsFactorForGroupEssential=0;
    this->_violationsFactorForGroupImportant=0;
    this->_violationsFactorForGroupDesirable=0;
    this->_violationsFactorForTchPreferences=0;
	
    this->constraintConflictData.clear();
    this->subjectPreferenceData.clear();

    foreach (TimeConstraint* timeConstraint, instance->validTimeConstraintsList) {

        double constraintViolationsFactor = timeConstraint->violationsFactor(*this, collectConflictsData);

        switch (timeConstraint->constraintGroup()) {
        case Enums::ConstraintGroup::Essential:
            this->_violationsFactorForGroupEssential += constraintViolationsFactor;
            break;
        case Enums::ConstraintGroup::Important:
            this->_violationsFactorForGroupImportant += constraintViolationsFactor;
            break;
        case Enums::ConstraintGroup::Desirable:
            this->_violationsFactorForGroupDesirable += constraintViolationsFactor;
            break;
        default:
            assert(0);
            break;
        }
    }

    this->_violationsFactorForTchPreferences = teachersPreferencesViolationsFactor();

    double ajustedConstraintGroupWeightEssential = instance->getConstraintGroupWeight(Enums::Essential) * this->_alpha_weight_used;

    this->_cost = _violationsFactorForGroupEssential * ajustedConstraintGroupWeightEssential

                + _violationsFactorForTchPreferences * instance->getSubjectPrefferenceWeight()

                + _violationsFactorForGroupImportant * instance->getConstraintGroupWeight(Enums::Important)
                + _violationsFactorForGroupDesirable * instance->getConstraintGroupWeight(Enums::Desirable);


    this->changedForMatrixCalculation=false;

    this->_cost = decimal::round_down(this->_cost, 6);

    return this->_cost;
}

double Solution::cost()
{
    return this->cost(ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE, false);
}

double Solution::cost(float essential_constraint_weight_alpha)
{
    return this->cost(essential_constraint_weight_alpha, false);
}

double Solution::cost(bool collectConflictsData)
{
    return this->cost(ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE, collectConflictsData);
}

bool Solution::costComputed()
{
    return (_cost >= 0.0);
}

double Solution::qualityCost()
{
    if (!complete()) {
        return INFINITY;
    }

    if (!costComputed())
        cost();

    double c = instance->getConstraintGroupWeight(Enums::Important) * _violationsFactorForGroupImportant
             + instance->getConstraintGroupWeight(Enums::Desirable) * _violationsFactorForGroupDesirable
             + instance->getSubjectPrefferenceWeight()              * _violationsFactorForTchPreferences;


    c = decimal::round_down(c, 6);

    return c;
}

double Solution::teachersPreferencesViolationsFactor(){
    int factor = 0;

    for (int ai = 0; ai < instance->activeActivitiesList.size(); ++ai) {
        if (teachers[ai] != UNALLOCATED_TEACHER) {
            Activity* act = instance->activeActivitiesList[ai];

            Subject* sbj = instance->subjectsList[act->subjectIndex];

            Teacher* teacher = instance->teachersList[teachers[ai]];

            factor+= instance->getSubjectPrefferenceFactor(instance->teacher_subject_preference_map[teacher][sbj]);

            subjectPreferenceData.addPreferenceData(instance->teacher_subject_preference_map[teacher][sbj], teachers[ai]);
        }
    }

    int n = instance->activeTeachersList.size();
    return factor / (double) n;
}

void Solution::povoarAllocation()
{
    if (allocation.size()!=0) {
        return;
    }

    vector<int> aux;
    aux.resize(instance->getNTimeSlots(), UNALLOCATED_ACTIVITY);

    allocation.resize(instance->teachersList.size(), aux);

    for (int ai = 0; ai < instance->activeActivitiesList.size(); ++ai){
        if (teachers[ai]==UNALLOCATED_TEACHER)
            continue;

        if (times[ai]==UNALLOCATED_TIME)
            continue;

        int tchi = teachers[ai];

        int tl = times[ai];

        allocation[tchi][tl] = ai;
    }
}

QByteArray Solution::hashCode()
{
    if (!_hash.isEmpty()) {
        return _hash;
    }

    QString auxString;

    for (int i = 0; i < instance->activeActivitiesList.size(); ++i) {
        auxString += QString::number(times[i]) + QString::number(teachers[i]);
    }

    _hash = QCryptographicHash::hash(auxString.toUtf8(),QCryptographicHash::Md5).toHex();

    return _hash;
}

QByteArray Solution::hashCode() const
{
    assert(!_hash.isEmpty());
    return _hash;
}

double Solution::totalConflictsFactors()
{
    if (!costComputed())
        cost();

    return _violationsFactorForGroupImportant
         + _violationsFactorForGroupEssential
         + _violationsFactorForGroupDesirable
         + _violationsFactorForTchPreferences;
}

bool Solution::aplicarMovimento(MoveN1andN2 &mov)
{
    int tchi = mov.tchi;
    int ts1 = mov.ts1;
    int ts2 = mov.ts2;

    povoarAllocation();

    if (allocation[tchi][ts1]==UNALLOCATED_ACTIVITY && allocation[tchi][ts2]==UNALLOCATED_ACTIVITY) {
        return false;
    }

    if (allocation[tchi][ts1]!=UNALLOCATED_ACTIVITY) {
        times[allocation[tchi][ts1]] = ts2;
    }

    if (allocation[tchi][ts2]!=UNALLOCATED_ACTIVITY) {
        times[allocation[tchi][ts2]] = ts1;
    }

    std::swap(allocation[tchi][ts1], allocation[tchi][ts2]);

    stateChanged();

    return true;
}

bool Solution::aplicarMovimento(MoveN3 &mov) //TODO: não testado
{
    int ai1a = mov.ai1;
    int ai2a = mov.ai2;

    if (teachers[ai1a] == teachers[ai2a] or teachers[ai1a] == UNALLOCATED_TEACHER or teachers[ai2a] == UNALLOCATED_TEACHER){
        return false;
    }

    povoarAllocation();

    int tch1 = teachers[ai1a];
    int tch2 = teachers[ai2a];

    int tai1a = times[ai1a];
    int tai2a = times[ai2a];

    std::swap(times[ai1a], times[ai2a]);

    int ai1b = allocation[tch1][tai2a];
    int ai2b = allocation[tch2][tai1a];

    if (ai1b != UNALLOCATED_ACTIVITY and ai2b == UNALLOCATED_ACTIVITY) {
        times[ai1b] = tai2a;
    }
    else if (ai1b == UNALLOCATED_ACTIVITY and ai2b != UNALLOCATED_ACTIVITY) {
        times[ai2b] = tai1a;
    }
    else if (ai1b != UNALLOCATED_ACTIVITY and ai2b != UNALLOCATED_ACTIVITY) {
        std::swap(times[ai1b], times[ai2b]);
    }

    stateChanged();

    return true;
}

bool Solution::aplicarMovimento(MoveN4 &mov)
{
    int ai = mov.ai;
    int newTchi = mov.newTchi;

    if (times[ai] == UNALLOCATED_TIME or teachers[ai] == UNALLOCATED_TEACHER){
        return false;
    }

    allocateTeacherToAct(ai, newTchi);

    //    std::swap(allocation[oldTchi][times[ai]], allocation[newTchi][times[ai]]);

    stateChanged();

    return true;
}

bool Solution::aplicarMovimento(MoveN5 &mov)
{
    int ai1 = mov.ai1;
    int ai2 = mov.ai2;

    if (teachers[ai1] == teachers[ai2] or teachers[ai1] == UNALLOCATED_TEACHER or teachers[ai2] == UNALLOCATED_TEACHER){
        return false;
    }

    int tch1 = teachers[ai1];
    int tch2 = teachers[ai2];

    allocateTeacherToAct(ai1, tch2);
    allocateTeacherToAct(ai2, tch1);

    //    std::swap(allocation[oldTchi][times[ai]], allocation[newTchi][times[ai]]);

    stateChanged();

    return true;
}

bool Solution::adicionarCandidato(TimeSlotId tSlot, CandidateActTch &cand)
{
    int ai = this->instance->getActivityIndex(cand.first->id);

    allocateTeacherToAct(ai, *cand.second);
    allocateSlotToAct(ai, tSlot);

    stateChanged();

    return true;
}

bool Solution::adicionarCandidato(int ai, CandidateTstTch &cand)
{
    allocateTeacherToAct(ai, *cand.second);
    allocateSlotToAct(ai, cand.first);

    stateChanged();

    return true;
}

bool Solution::adicionarCandidato(Activity *act, CandidateTstTch &cand)
{
    int ai = this->instance->getActivityIndex(act->id);
    return adicionarCandidato(ai, cand);
}

void Solution::resetCost()
{
    _cost = -1;

    _violationsFactorForGroupEssential = -1;
    _violationsFactorForGroupImportant = -1;
    _violationsFactorForGroupDesirable = -1;
    _violationsFactorForTchPreferences = -1;
}

void Solution::resetHash()
{
    _hash = QByteArray();
}

void Solution::stateChanged()
{
    changedForMatrixCalculation=true;
    teachersMatrixReady=false;
    subgroupsMatrixReady=false;
    teachers_schedule_ready=false;
    students_schedule_ready=false;

    allocation.clear();

    resetCost();
    resetHash();
}

QString Solution::_auxTeacherPrefferencesDescription(Enums::SubjectPreference pref)
{
    QString ds = "";

    ds+="\n\n  * Teachers allocated to '" + enums::enumIndexToStr(pref) + "' prefference: ";
    ds+=QString::number(subjectPreferenceData.teachersIndexesList(pref).size());
    ds = ds.leftJustified(60, ' ');

    int count = lists::sum(subjectPreferenceData.level_data_map[pref].teachersIndexes_count_map.values().toStdList(), 0);
    ds+= "  -> Total of subactivities: " + QString::number(count);

    for (auto tchi: subjectPreferenceData.teachersIndexesList(pref)){
        ds+= "\n      ¬ " + instance->teachersList[tchi]->name + ": ";
        ds+= QString::number(subjectPreferenceData.level_data_map[pref].teachersIndexes_count_map[tchi]);
        ds+= " subactivities";
    }

    return ds;
}

QString Solution::getCostDetailsString(Enums::VerboseLevel vl, float weight_alpha)
{
    cost(weight_alpha, true);

    subjectPreferenceData.computeData();

    QString ds = "";

    switch (vl) {
    case Enums::Full:
        ds+="\n | Total weighted cost:                     ";
        ds+=strings::number(_cost);
        ds+="\n\n | Essential constraints violation factor:  ";
        ds+=strings::number(_violationsFactorForGroupEssential);
        ds+="\n\n | Important constraints violation factor:  ";
        ds+=strings::number(_violationsFactorForGroupImportant);
        ds+="\n\n | Desirable constraints violation factor:  ";
        ds+=strings::number(_violationsFactorForGroupDesirable);
        ds+="\n\n | Prefference violation factor:            ";
        ds+=strings::number(_violationsFactorForTchPreferences);

        ds+=_auxTeacherPrefferencesDescription(Enums::Very_High);
        ds+=_auxTeacherPrefferencesDescription(Enums::High);
        ds+=_auxTeacherPrefferencesDescription(Enums::Indifferent);
        ds+=_auxTeacherPrefferencesDescription(Enums::Low);
        ds+=_auxTeacherPrefferencesDescription(Enums::Very_Low);

        break;
    case Enums::Short:
        ds+=" | Total weighted cost: ";
        ds+=strings::toFloatString(_cost, 9);
        ds+="\n | Ess. factor:         ";
        ds+=strings::toIntegerString(_violationsFactorForGroupEssential, 9);
        ds+="\n | Imp. factor:         ";
        ds+=strings::toIntegerString(_violationsFactorForGroupImportant, 9);
        ds+="\n | Des. factor:         ";
        ds+=strings::toIntegerString(_violationsFactorForGroupDesirable, 9);
        ds+="\n | Pref. factor:        ";
        ds+=strings::toFloatString(_violationsFactorForTchPreferences, 9);
        break;
    case Enums::Minimal:
        ds+=" | E: ";
        ds+=strings::toIntegerString(_violationsFactorForGroupEssential, 3);
        ds+=" | I: ";
        ds+=strings::toIntegerString(_violationsFactorForGroupImportant, 3);
        ds+=" | D: ";
        ds+=strings::toIntegerString(_violationsFactorForGroupDesirable, 3);
        ds+=" | Pref: ";
        ds+=strings::toFloatString(_violationsFactorForTchPreferences, 6);
        ds+=" | Cost: ";
        ds+=strings::toFloatString(_cost, 9);
        ds+=" (wa=";
        ds+=strings::toFloatString(_alpha_weight_used, 4, 1);
        ds+=")";
        if (decimal::round_down(_violationsFactorForGroupEssential,1)==0.0)
            ds+="   --> Viable ";
        break;
    default:
        assert(0);
        break;
    }

    return ds;
}

QString Solution::getCostDetailsString(Enums::VerboseLevel vl)
{
    return this->getCostDetailsString(vl, ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE);
}

QString Solution::getCostDetailsString(float weight_alpha)
{
    return this->getCostDetailsString(Enums::Minimal, weight_alpha);
}

QString Solution::getCostDetailsString()
{
    return this->getCostDetailsString(Enums::Minimal, ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE);
}

QString Solution::_auxConstraintGroupViolationsDescription(Enums::ConstraintGroup g)
{
    QString ds = "";

    int i;

    QList<QString> cdl = constraintConflictData.conflictsDescriptionList(g);

    if (!cdl.isEmpty()) {
        ds+="\n ¬ " + enums::enumIndexToStr(g) + " constraints conflicts:\n\n";

        i = 1;
        foreach(QString t, cdl)
            ds+= " #" + QString::number(i++) + ": " + t + "\n\n";
    }

    return ds;
}

QString Solution::getConflictsDescriptionString()
{
    this->cost(this->_alpha_weight_used, true);

    constraintConflictData.computeData();
    subjectPreferenceData.computeData();

    QString ds = "";

    ds+=getCostDetailsString(Enums::Full);
    ds+="\n\n";
    QString auxs;
    auxs = auxs.rightJustified(95, '#');
    ds+=auxs;
    ds+="\n\n";
    ds+=tr("Number of broken constraints: %1").arg(constraintConflictData.conflictsCostList().count());
    ds+="\n";
    ds+=tr("Conflicts listing:");
    ds+="\n";

    ds+= _auxConstraintGroupViolationsDescription(Enums::Essential);
    ds+= _auxConstraintGroupViolationsDescription(Enums::Important);
    ds+= _auxConstraintGroupViolationsDescription(Enums::Desirable);

    return ds;
}

int Solution::computeTeachersMatrix(bool collectConflictsData){
    assert(instance->initialized);
    assert(instance->internalStructureComputed);

    if (teachersMatrixReady) {
        return teachers_conflicts;
    }

    Matrix3D<int>& a = teachersMatrix;

    int conflicts=0;

    a.resize(instance->teachersList.size(), instance->nDaysPerWeek, instance->nHoursPerDay);

    int i;
    for(i=0; i<instance->teachersList.size(); i++)
        for(int j=0; j<instance->nDaysPerWeek; j++)
            for(int k=0; k<instance->nHoursPerDay; k++)
                a[i][j][k]=0;

    for(i=0; i<instance->activeActivitiesList.size(); i++)
        if(this->times[i]!=UNALLOCATED_TIME) {
            if(this->teachers[i]!=UNALLOCATED_TEACHER) {
                int hour=getHourForActivityIndex(i);
                int day=getDayForActivityIndex(i);
                Activity* act=instance->activeActivitiesList[i];
                for(int dd=0; dd<act->duration && hour+dd<instance->nHoursPerDay; dd++){
                    int tmp=a[this->teachers[i]][day][hour+dd];
                    conflicts += tmp==0 ? 0 : 1;
                    a[this->teachers[i]][day][hour+dd]++;

                    if(collectConflictsData){
                        if(tmp>0){
                            int parcialViolation = 1;

                            QString s=tr("Time constraint basic compulsory");
                            s+=": ";
                            s+=tr("teacher with name %1 has more than one allocated activity on day %2, hour %3")
                                    .arg(instance->teachersList.at(this->teachers[i])->name)
                                    .arg(instance->daysOfTheWeek[day])
                                    .arg(instance->hoursOfTheDay[hour+dd]);
                            s+=". ";
                            s+=tr("This increases the cost total by %1")
                                    .arg(strings::number(parcialViolation * instance->getConstraintGroupWeight(Enums::Essential)));

                            constraintConflictData.addConflict(Enums::Essential, s, parcialViolation);
                        }
                    }

                }
            }
        }

    this->changedForMatrixCalculation=false;

    this->teachersMatrixReady=true;

    this->teachers_conflicts = conflicts;

    return conflicts;
}

int Solution::computeSubgroupsMatrix(bool collectConflictsData){
    assert(instance->initialized);
    assert(instance->internalStructureComputed);

    if (subgroupsMatrixReady) {
        return subgroups_conflicts;
    }

    Matrix3D<int> &a = subgroupsMatrix;

    int conflicts=0;

    a.resize(instance->directSubgroupsList.size(), instance->nDaysPerWeek, instance->nHoursPerDay);

    int i;
    for(i=0; i<instance->directSubgroupsList.size(); i++)
        for(int j=0; j<instance->nDaysPerWeek; j++)
            for(int k=0; k<instance->nHoursPerDay; k++)
                a[i][j][k]=0;

    for(i=0; i<instance->activeActivitiesList.size(); i++)
        if(this->times[i]!=UNALLOCATED_TIME){
            int hour=getHourForActivityIndex(i);
            int day=getDayForActivityIndex(i);
            Activity* act = instance->activeActivitiesList[i];
            for(int dd=0; dd < act->duration && hour+dd < instance->nHoursPerDay; dd++)
                for(int isg=0; isg < act->iSubgroupsList.count(); isg++){ //isg => index subgroup
                    int sg = act->iSubgroupsList.at(isg); //sg => subgroup
                    int tmp=a[sg][day][hour+dd];
                    conflicts += tmp==0 ? 0 : 1;
                    a[sg][day][hour+dd]++;

                    if(collectConflictsData){
                        if(tmp>0){
                            int parcialViolation = 1;

                            QString s=tr("Time constraint basic compulsory");
                            s+=": ";
                            s+=tr("subgroup %1 has more than one allocated activity on day %2, hour %3")
                                    .arg(instance->directSubgroupsList[sg]->name)
                                    .arg(instance->daysOfTheWeek[day])
                                    .arg(instance->hoursOfTheDay[hour]);
                            s+=". ";
                            s+=tr("This increases the cost total by %1")
                                    .arg(strings::number(parcialViolation * instance->getConstraintGroupWeight(Enums::Essential)));

                            constraintConflictData.addConflict(Enums::Essential, s, parcialViolation);
                        }
                    }
                }
        }

    this->changedForMatrixCalculation=false;

    this->subgroupsMatrixReady=true;

    this->subgroups_conflicts = conflicts;

    return conflicts;
}

//The following 2 functions (GetTeachersTimetable & GetSubgroupsTimetable)
//are very similar to the above 2 ones (GetTeachersMatrix & GetSubgroupsMatrix)
void Solution::computeTeachersTimetable(){
    assert(instance->initialized);
    assert(instance->internalStructureComputed);

    Matrix3D<int>& a = teachers_timetable_weekly;
    Matrix3D<QList<int> >& b = teachers_free_periods_timetable_weekly;
	
    a.resize(instance->teachersList.size(), instance->nDaysPerWeek, instance->nHoursPerDay);
    b.resize(TEACHERS_FREE_PERIODS_N_CATEGORIES, instance->nDaysPerWeek, instance->nHoursPerDay);
	
	int i, j, k;
    for(i=0; i<instance->teachersList.size(); i++)
        for(j=0; j<instance->nDaysPerWeek; j++)
            for(k=0; k<instance->nHoursPerDay; k++)
				a[i][j][k]=UNALLOCATED_ACTIVITY;

    Activity *act;
    for(i=0; i<instance->activeActivitiesList.size(); i++)
        if(this->times[i]!=UNALLOCATED_TIME) {
            act=instance->activeActivitiesList[i];
            int hour=getHourForActivityIndex(i);
            int day=getDayForActivityIndex(i);
            for(int dd=0; dd < act->duration; dd++){
                assert(hour+dd<instance->nHoursPerDay);

//              assert(a[this->teachers[i]][day][hour+dd]==UNALLOCATED_ACTIVITY);
                if(a[this->teachers[i]][day][hour+dd]!=UNALLOCATED_ACTIVITY)
                    std::cout << ">>>>> Tch com conflito de horario " << instance->teachersList[this->teachers[i]]->name.toStdString() << ", " << day << ", " << hour+dd << std::endl;

                a[this->teachers[i]][day][hour+dd]=i;
            }
        }

	//Prepare teachers free periods timetable.
	//Code contributed by Volker Dirr (http://timetabling.de/) BEGIN
	int d,h,tch;
    for(d=0; d<instance->nDaysPerWeek; d++){
        for(h=0; h<instance->nHoursPerDay; h++){
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
				b[tfp][d][h].clear();
			}
		}
	}
    for(tch=0; tch<instance->teachersList.size(); tch++){
        for(d=0; d<instance->nDaysPerWeek; d++){
			int firstPeriod=-1;
			int lastPeriod=-1;
            for(h=0; h<instance->nHoursPerDay; h++){
				if(a[tch][d][h]!=UNALLOCATED_ACTIVITY){
					if(firstPeriod==-1)
						firstPeriod=h;
					lastPeriod=h;
				}
			}
			if(firstPeriod==-1){
                for(h=0; h<instance->nHoursPerDay; h++){
					b[TEACHER_HAS_A_FREE_DAY][d][h]<<tch;
				}
			} else {
				for(h=0; h<firstPeriod; h++){
					if(firstPeriod-h==1){
						b[TEACHER_MUST_COME_EARLIER][d][h]<<tch;
					}
					else {
						b[TEACHER_MUST_COME_MUCH_EARLIER][d][h]<<tch;
					}
				}
				for(; h<lastPeriod+1; h++){
					if(a[tch][d][h]==UNALLOCATED_ACTIVITY){
						if(a[tch][d][h+1]==UNALLOCATED_ACTIVITY){
							if(a[tch][d][h-1]==UNALLOCATED_ACTIVITY){
								b[TEACHER_HAS_BIG_GAP][d][h]<<tch;
							} else {
								b[TEACHER_HAS_BORDER_GAP][d][h]<<tch;
							}
						} else {
							if(a[tch][d][h-1]==UNALLOCATED_ACTIVITY){
								b[TEACHER_HAS_BORDER_GAP][d][h]<<tch;
							} else {
								b[TEACHER_HAS_SINGLE_GAP][d][h]<<tch;
							}
						}
					}
				}
                for(; h<instance->nHoursPerDay; h++){
					if(lastPeriod-h==-1){
						b[TEACHER_MUST_STAY_LONGER][d][h]<<tch;
					}
					else {
						b[TEACHER_MUST_STAY_MUCH_LONGER][d][h]<<tch;
					}
				}
			}
		}
	}
	//care about not available teacher and breaks
    for(tch=0; tch<instance->teachersList.size(); tch++){
        for(d=0; d<instance->nDaysPerWeek; d++){
            for(h=0; h<instance->nHoursPerDay; h++){
                if(instance->teacherNotAvailableDayHour[tch][d][h]==true || instance->breakDayHour[d][h]==true){
					int removed=0;
					for(int tfp=0; tfp<TEACHER_IS_NOT_AVAILABLE; tfp++){
						if(b[tfp][d][h].contains(tch)){
							removed+=b[tfp][d][h].removeAll(tch);
                            if(instance->breakDayHour[d][h]==false)
								b[TEACHER_IS_NOT_AVAILABLE][d][h]<<tch;
						}
					}
//                  assert(removed==1);
                    if(removed!=1)
                        std::cout << ">>>>> Tch em break ou indisponivel " << instance->teachersList[tch]->name.toStdString() << std::endl;
				}
			}
		}
	}
	//END of Code contributed by Volker Dirr (http://timetabling.de/) END
	//bool visited[MAX_TEACHERS];
	Matrix1D<bool> visited;
    visited.resize(instance->teachersList.size());
    for(d=0; d<instance->nDaysPerWeek; d++){
        for(h=0; h<instance->nHoursPerDay; h++){
            for(tch=0; tch<instance->teachersList.size(); tch++)
				visited[tch]=false;
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
				foreach(int tch, b[tfp][d][h]){
					assert(!visited[tch]);
					visited[tch]=true;
				}
			}
		}
	}

    teachers_schedule_ready=true;
}

void Solution::computeSubgroupsTimetable(){
    assert(instance->initialized);
    assert(instance->internalStructureComputed);

    Matrix3D<int>& a = students_timetable_weekly;
	
    a.resize(instance->directSubgroupsList.size(), instance->nDaysPerWeek, instance->nHoursPerDay);
	
	int i, j, k;
    for(i=0; i<instance->directSubgroupsList.size(); i++)
        for(j=0; j<instance->nDaysPerWeek; j++)
            for(k=0; k<instance->nHoursPerDay; k++)
				a[i][j][k]=UNALLOCATED_ACTIVITY;

	Activity *act;
    for(i=0; i<instance->activeActivitiesList.size(); i++)
		if(this->times[i]!=UNALLOCATED_TIME) {
            act=instance->activeActivitiesList[i];
            int hour=getHourForActivityIndex(i);
            int day=getDayForActivityIndex(i);
			for(int dd=0; dd < act->duration; dd++){
                assert(hour+dd<instance->nHoursPerDay);
			
				for(int isg=0; isg < act->iSubgroupsList.count(); isg++){ //isg -> index subgroup
					int sg = act->iSubgroupsList.at(isg); //sg -> subgroup

//					assert(a[sg][day][hour+dd]==UNALLOCATED_ACTIVITY);
                    if(a[sg][day][hour+dd]!=UNALLOCATED_ACTIVITY)
                        std::cout << ">>>> Sg com conflito de horario " << instance->directSubgroupsList[sg]->name.toStdString() << "," << day << ", " << hour+dd << std::endl;

					a[sg][day][hour+dd]=i;
				}
			}
		}

    students_schedule_ready=true;
}

void to_json(json &j, const Solution &s) {
    j = json();

    unsigned int number_of_active_activities = (unsigned int) s.instance->activeActivitiesList.size();

    std::vector<int> times;
    for (unsigned int i = 0; i < number_of_active_activities; ++i) {
        times.push_back(s.times[i]);
    }

    std::vector<int> teachers;
    for (unsigned int i = 0; i < number_of_active_activities; ++i) {
        teachers.push_back(s.teachers[i]);
    }

    QString f = strings::toFloatString(s._violationsFactorForGroupEssential, 6, 1) + "|"
              + strings::toFloatString(s._violationsFactorForGroupImportant, 6, 1) + "|"
              + strings::toFloatString(s._violationsFactorForGroupDesirable, 6, 1) + "|"
              + strings::toFloatString(s._violationsFactorForTchPreferences, 6, 2)
              ;

    j["times"] = times;
    j["teachers"] = teachers;
    j["hash"] = s.hashCode().toStdString();
    j["factors"] = f.toStdString();
}

void from_json(const json &j, Solution &s) {
    s.instance = &TContext::get()->instance;
    std::copy_n(j["times"].begin(), s.instance->activeActivitiesList.size(), s.times.begin());
    std::copy_n(j["teachers"].begin(), s.instance->activeActivitiesList.size(), s.teachers.begin());
    s.stateChanged();
}

Solution &Solution::bestBetween(Solution &sA, Solution &sB)
{
    return Solution::bestBetween(sA, sB, ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE);
}

Solution &Solution::bestBetween(Solution &sA, Solution &sB, float weight_alpha)
{
    if (!sA.viable() and !sB.viable()) {
        double costA = sA.complete() ? sA.cost(weight_alpha) : INFINITY;
        double costB = sB.complete() ? sB.cost(weight_alpha) : INFINITY;

        if (costA > costB) {
            return sB;
        }
        else{
            return sA;
        }
    }
    else if (!sA.viable() and sB.viable()){
        return sB;
    }
    else if (sA.viable() and !sB.viable()){
        return sA;
    }
    else{
        if (sA.cost(weight_alpha) > sB.cost(weight_alpha)) {
            return sB;
        }
        else{
            return sA;
        }
    }
}

bool Solution::isBetterThan(Solution &s)
{
   return this->isBetterThan(s, ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE);
}

bool Solution::isBetterThan(Solution &s, float weight_alpha) // adicionado mas não usado e não testado
{
    Solution& sTmp =  Solution::bestBetween(*this, s, weight_alpha);
    return this->hashCode() == sTmp.hashCode() and !IS_EQUAL(this->cost(weight_alpha), sTmp.cost(weight_alpha));
}

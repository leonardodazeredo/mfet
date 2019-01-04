/*
File solution.h
*/

/***************************************************************************
                          solution.h  -  description
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

#ifndef SOLUTION_H
#define SOLUTION_H

#include <QCoreApplication>

#include "defs.h"

#include "matrix.h"
#include "vector"

#include <QString>
#include <QList>
#include <QTextStream>

#include "general.h"

#include <array>

#include "json.hpp"
using json = nlohmann::json;

class Instance;

/**
This class represents a solution (time and space allocation for the activities).
*/
class Solution{
    Q_DECLARE_TR_FUNCTIONS(Solution)

public:
    static Solution& bestBetween(Solution& sA, Solution& sB);
    static Solution& bestBetween(Solution& sA, Solution& sB, float weight_alpha);

    bool isBetterThan(Solution& s);
    bool isBetterThan(Solution& s, float weight_alpha);

    Solution();

    Instance* instance = nullptr;

    ConstraintConflictData constraintConflictData;

    SubjectPreferenceData subjectPreferenceData;
	
	bool teachersMatrixReady;
	bool subgroupsMatrixReady;

    bool students_schedule_ready, teachers_schedule_ready;

    /**
    The timetable for the teachers
    */
    Matrix3D<int> teachers_timetable_weekly;
    Matrix3D<QList<int>> teachers_free_periods_timetable_weekly;

    /**
    The timetable for the students
    */
    Matrix3D<int> students_timetable_weekly;

    Matrix3D<int> subgroupsMatrix;
    Matrix3D<int> teachersMatrix;

    int teachers_conflicts=-1;
    int subgroups_conflicts=-1;
	
	/*
	You will need to set this to true if altering the times array values.
	The conflicts calculating routine will reset this to false
	at the first teachers matrix and subgroups matrix calculation.
	*/
	bool changedForMatrixCalculation;

    std::array<int, MAX_ACTIVITIES> teachers;

	/**
	This array represents every activity's start time
	(time is a unified representation of hour and day,
	stored as an integer value). We have a special value here:
	UNALLOCATED_TIME, which is a large number.
	*/
    std::array<int, MAX_ACTIVITIES> times;

    bool empty() const;

    bool complete() const;

    bool viable();

    Solution(Instance *instance){
        init(*instance);
    }

    ~Solution(){
    }

    Solution(const Solution& other){ // copy constructor
        copy(other);
    }

    Solution& operator=(const Solution& other){ // copy assignment
        copy(other);

        return *this;
    }
	
    int getTimeSlotForActivity(int activityIndex);

    void setTimeSlotForActivity(int activityIndex, int timeSlot);

    int getDayForActivityIndex(int i);

    int getHourForActivityIndex(int i);

    bool isAtivityAllocated(int activityIndex);

    bool aplicarMovimento(MoveN1andN2 &mov);

    bool aplicarMovimento(MoveN3 &mov);

    bool aplicarMovimento(MoveN4 &mov);

    bool aplicarMovimento(MoveN5 &mov);

    bool adicionarCandidato(TimeSlotId tSlot, CandidateActTch &cand);

    bool adicionarCandidato(int ai, CandidateTstTch &cand);

    bool adicionarCandidato(Activity* act, CandidateTstTch &cand);

	/**
	Initializes, marking all activities as unscheduled (time)
	*/
    void init(Instance& instance);

	/**
	Marks the starting time of all the activities as undefined
	(all activities are unallocated).
	*/
    void makeUnallocated();

	/**
	ATTENTION: if the rules change, the user has to reset _fitness to -1
	<p>
	If conflictsString is not null, then this function will
	append at this string an explanation of the conflicts.
	*/
    double cost(float essential_constraint_weight_alpha, bool collectConflictsData);
    double cost(float essential_constraint_weight_alpha);
    double cost(bool collectConflictsData);
    double cost();

    bool costComputed();

    double qualityCost();

    void resetCost();

    void resetHash();

    void stateChanged();

    double totalConflictsFactors();

    void computeTeachersTimetable();
	//return value is the number of conflicts, which must be 0

    void computeSubgroupsTimetable();
	//return value is the number of conflicts, which must be 0

    int computeSubgroupsMatrix(bool collectConflictsData=false);

    int computeTeachersMatrix(bool collectConflictsData=false);

    QString getCostDetailsString(Enums::VerboseLevel vl, float weight_alpha);
    QString getCostDetailsString(Enums::VerboseLevel vl);
    QString getCostDetailsString(float weight_alpha);
    QString getCostDetailsString();

    QString getConflictsDescriptionString();

    void povoarAllocation();

    std::vector<std::vector<int>> allocation;

    QByteArray hashCode();

    QByteArray hashCode() const;

    friend inline bool operator==(const Solution& lhs, const Solution& rhs);
    friend inline bool operator!=(const Solution& lhs, const Solution& rhs);
    friend inline bool operator< (const Solution& lhs, const Solution& rhs);
    friend inline bool operator> (const Solution& lhs, const Solution& rhs);
    friend inline bool operator<=(const Solution& lhs, const Solution& rhs);
    friend inline bool operator>=(const Solution& lhs, const Solution& rhs);

    friend void to_json(json& j, const Solution& s);
    friend void from_json(const json& j, Solution& p);

    float getAlphaWeightUsed() const;

    double getViolationsFactorForGroupEssential() const;

private:

    int _nActiveActivities;

    void allocateTeacherToAct(int ai, int tchi);

    void allocateTeacherToAct(int ai, Teacher &tch);

    void allocateSlotToAct(int ai, TimeSlotId timeSlot);

    QByteArray _hash;

    /**
    Cost; it is calculated only at the initialization or
    at the modification.
    Important assumption: the rules have to ramain the same;
    otherwise the user has to reset this value to -1
    */
    double _cost;

    float _alpha_weight_used = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;

    double _violationsFactorForGroupEssential;
    double _violationsFactorForGroupImportant;
    double _violationsFactorForGroupDesirable;

    double _violationsFactorForTchPreferences;

    double teachersPreferencesViolationsFactor();

    /**
    Assignment method. We need to have access to the Rules instantiation
    to know the number of activities.
    */
    void copy(const Solution& c);

    QString _auxTeacherPrefferencesDescription(Enums::SubjectPreference pref);
    QString _auxConstraintGroupViolationsDescription(Enums::ConstraintGroup g);
};

inline bool operator==(const Solution& lhs, const Solution& rhs){return lhs.times == rhs.times && lhs.teachers == rhs.teachers;}
inline bool operator!=(const Solution& lhs, const Solution& rhs){return !operator==(lhs,rhs);}
inline bool operator< (const Solution& lhs, const Solution& rhs){
    assert(lhs._cost >= 0);assert(rhs._cost >= 0);assert(IS_EQUAL(lhs._alpha_weight_used, rhs._alpha_weight_used));
    return lhs._cost < rhs._cost;}
inline bool operator> (const Solution& lhs, const Solution& rhs){return  operator< (rhs,lhs);}
inline bool operator<=(const Solution& lhs, const Solution& rhs){return !operator> (lhs,rhs);}
inline bool operator>=(const Solution& lhs, const Solution& rhs){return !operator< (lhs,rhs);}

inline uint qHash(Solution key) {return key.hashCode().toUInt();}

void to_json(json& j, const Solution& s);
void from_json(const json& j, Solution& p);

#endif

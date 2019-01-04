/*
File timeconstraint.h
*/

/***************************************************************************
                          timeconstraint.h  -  description
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

#ifndef TIMECONSTRAINT_H
#define TIMECONSTRAINT_H

#include <QCoreApplication>

#include "defs.h"
#include "general.h"

#include <QString>
#include <QList>
#include <QStringList>

#include "json.hpp"
using json = nlohmann::json;

class Instance;
class Solution;
class TimeConstraint;
class Activity;
class Teacher;
class Subject;
class ActivityTag;
class StudentsSet;

class QWidget;

typedef QList<TimeConstraint*> TimeConstraintsList;

const int CONSTRAINT_GENERIC_TIME										=0;

const int CONSTRAINT_BASIC_COMPULSORY_TIME								=1;
const int CONSTRAINT_BREAK_TIMES										=2;

const int CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES						=3;
const int CONSTRAINT_TEACHERS_MAX_HOURS_DAILY							=4;
const int CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK							=5;
const int CONSTRAINT_TEACHERS_MAX_GAPS_PER_WEEK							=6;
const int CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK							=7;
const int CONSTRAINT_TEACHER_MAX_HOURS_DAILY							=8;
const int CONSTRAINT_TEACHERS_MAX_HOURS_CONTINUOUSLY					=9;
const int CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY						=10;

const int CONSTRAINT_TEACHERS_MIN_HOURS_DAILY							=11;
const int CONSTRAINT_TEACHER_MIN_HOURS_DAILY							=12;
const int CONSTRAINT_TEACHERS_MAX_GAPS_PER_DAY							=13;
const int CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY							=14;

const int CONSTRAINT_STUDENTS_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR		=15;
const int CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR	=16;
const int CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES					=17;
const int CONSTRAINT_STUDENTS_MAX_GAPS_PER_WEEK							=18;
const int CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK						=19;

const int CONSTRAINT_STUDENTS_MAX_HOURS_DAILY							=20;
const int CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY						=21;
const int CONSTRAINT_STUDENTS_MAX_HOURS_CONTINUOUSLY					=22;
const int CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY				=23;

const int CONSTRAINT_STUDENTS_MIN_HOURS_DAILY							=24;
const int CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY						=25;

const int CONSTRAINT_ACTIVITY_ENDS_STUDENTS_DAY							=26;
const int CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME					=27;
const int CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME						=28;
const int CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING							=29;
const int CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES						=30;
const int CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS						=31;
const int CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS					=32;
const int CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES					=33;
const int CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES				=34;
const int CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR						=35;
const int CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY						=36;
const int CONSTRAINT_TWO_ACTIVITIES_CONSECUTIVE							=37;
const int CONSTRAINT_TWO_ACTIVITIES_ORDERED								=38;
const int CONSTRAINT_MIN_GAPS_BETWEEN_ACTIVITIES						=39;
const int CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS					=40;
const int CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES				=41;

const int CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK					=42;
const int CONSTRAINT_TEACHERS_INTERVAL_MAX_DAYS_PER_WEEK				=43;
const int CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK			=44;
const int CONSTRAINT_STUDENTS_INTERVAL_MAX_DAYS_PER_WEEK				=45;

const int CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY						=46;

const int CONSTRAINT_TWO_ACTIVITIES_GROUPED								=47;

const int CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY		=48;
const int CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY		=49;
const int CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY		=50;
const int CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY	=51;

const int CONSTRAINT_TEACHERS_MAX_DAYS_PER_WEEK							=52;

const int CONSTRAINT_THREE_ACTIVITIES_GROUPED							=53;
const int CONSTRAINT_MAX_DAYS_BETWEEN_ACTIVITIES						=54;

const int CONSTRAINT_TEACHERS_MIN_DAYS_PER_WEEK							=55;
const int CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK							=56;

const int CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY				=57;
const int CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY				=58;
const int CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY				=59;
const int CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY			=60;

const int CONSTRAINT_STUDENTS_MAX_GAPS_PER_DAY							=61;
const int CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY						=62;

const int CONSTRAINT_ACTIVITIES_OCCUPY_MAX_TIME_SLOTS_FROM_SELECTION	=63;
const int CONSTRAINT_ACTIVITIES_MAX_SIMULTANEOUS_IN_SELECTED_TIME_SLOTS	=64;

const int CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK						=65;
const int CONSTRAINT_STUDENTS_MAX_DAYS_PER_WEEK							=66;

//2017-02-06
const int CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY							=67;
const int CONSTRAINT_TEACHERS_MAX_SPAN_PER_DAY							=68;
const int CONSTRAINT_TEACHER_MIN_RESTING_HOURS							=69;
const int CONSTRAINT_TEACHERS_MIN_RESTING_HOURS							=70;
const int CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY						=71;
const int CONSTRAINT_STUDENTS_MAX_SPAN_PER_DAY							=72;
const int CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS						=73;
const int CONSTRAINT_STUDENTS_MIN_RESTING_HOURS							=74;

const int CONSTRAINT_MAX_ACTIVITIES_FOR_ALL_TEACHERS					=75;
const int CONSTRAINT_MAX_ACTIVITIES_FOR_TEACHER							=76;

const int CONSTRAINT_TEACHERS_MAX_HOURS_IN_WORK_DAILY					=77;
const int CONSTRAINT_TEACHER_MAX_HOURS_IN_WORK_DAILY					=78;

const int CONSTRAINT_NO_TEACHER_SUBJECT_PREFERENCE                      =79;

const int CONSTRAINT_MIN_ACTIVITIES_FOR_ALL_TEACHERS					=80;
const int CONSTRAINT_MIN_ACTIVITIES_FOR_TEACHER							=81;

/**
This class represents a time constraint
*/
class TimeConstraint : public QObject {
    Q_OBJECT
public:
    Instance& r;

    /**
        Specifies the type of this constraint (using the above constants).
        */
    int type;

    Enums::ConstraintGroup group;

    /**
    The percentage weight of this constraint, 100% compulsory, 0% non-compulsory
    */
    double weightPercentage;

    bool active;

    QString comments;

    /**
        True for mandatory constraints, false for non-mandatory constraints.
        */
    //bool compulsory;

    /**
        Dummy constructor - needed for the static array of constraints.
        Any other use should be avoided.
        */
    TimeConstraint(Instance& r);

    virtual ~TimeConstraint()=0;

    /**
        DEPRECATED COMMENT
        Constructor - please note that the maximum allowed weight is 100.0
        The reason: unallocated activities must have very big conflict weight,
        and any other restrictions must have much more lower weight,
        so that the timetable can evolve when starting with uninitialized activities.
        */
    TimeConstraint(Instance &r, Enums::ConstraintGroup constraintGroup, double wp);

    TimeConstraint(Instance &r, Enums::ConstraintGroup constraintGroup);

    /**
    The function that calculates the violationsFactor of a solution, according to this
    constraint. We need the rules to compute this violationsFactor factor.
        If conflictsString!=nullptr,
        it will be initialized with a text explaining where this restriction is broken.
        */
    virtual double violationsFactor(Solution& c, bool collectConflictsData=false)=0;

    /**
        Returns an XML description of this constraint
        */
    virtual QString getXmlDescription()=0;
    QString getSuperXmlDescription();

    /**
        Computes the internal structure for this constraint.

        It returns false if the constraint is an activity related
        one and it depends on only inactive activities.
        */
    virtual bool computeInternalStructure(QWidget* parent)=0;

    virtual bool hasInactiveActivities()=0;

    /**
        Returns a small description string for this constraint
        */
    virtual QString getDescription()=0;
    QString getSuperDescription();

    /**
        Returns a detailed description string for this constraint
        */
    virtual QString getDetailedDescription()=0;
    QString getSuperDetailedDescription();

    /**
        Returns true if this constraint is related to this activity
        */
    virtual bool isRelatedToActivity( Activity* a)=0;

    /**
        Returns true if this constraint is related to this teacher
        */
    virtual bool isRelatedToTeacher(Teacher* t)=0;

    /**
        Returns true if this constraint is related to this subject
        */
    virtual bool isRelatedToSubject(Subject* s)=0;

    /**
        Returns true if this constraint is related to this activity tag
        */
    virtual bool isRelatedToActivityTag(ActivityTag* s)=0;

    /**
        Returns true if this constraint is related to this students set
        */
    virtual bool isRelatedToStudentsSet( StudentsSet* s)=0;

    virtual bool hasWrongDayOrHour()=0;
    virtual bool canRepairWrongDayOrHour()=0;
    virtual bool repairWrongDayOrHour()=0;

    void setConstraintGroup(Enums::ConstraintGroup constraintGroup);
    Enums::ConstraintGroup constraintGroup() const;

    friend void to_json(json& j, const TimeConstraint& s);
    friend void from_json(const json& j, TimeConstraint& p);
};

void to_json(json& j, const TimeConstraint& s);
void from_json(const json& j, TimeConstraint& p);

/**
This class comprises all the basic compulsory constraints (constraints
which must be fulfilled for any timetable) - the time allocation part
*/
class ConstraintBasicCompulsoryTime: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintBasicCompulsoryTime)

public:
    ConstraintBasicCompulsoryTime(Instance &r);

    ConstraintBasicCompulsoryTime(Instance &r, Enums::ConstraintGroup constraintGroup, double wp);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherNotAvailableTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherNotAvailableTimes)

public:
    QList<int> days;
    QList<int> hours;

    /**
        The teacher's name
        */
    QString teacher;

    /**
        The teacher's id, or index in the rules
        */
    int teacher_ID;

    ConstraintTeacherNotAvailableTimes(Instance &r);

    ConstraintTeacherNotAvailableTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, const QString& tn, QList<int> d, QList<int> h);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetNotAvailableTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetNotAvailableTimes)

public:
    QList<int> days;
    QList<int> hours;

    /**
        The name of the students
        */
    QString students;

    /**
        The subgroups involved in this restriction
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetNotAvailableTimes(Instance &r);

    ConstraintStudentsSetNotAvailableTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, const QString& sn, QList<int> d, QList<int> h);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesSameStartingTime: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesSameStartingTime)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;
    //int activitiesId[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME];

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (indexes in the rules) - internal structure
        */
    //int _activities[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME];
    QList<int> _activities;

    ConstraintActivitiesSameStartingTime(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities' id-s.
        */
    //ConstraintActivitiesSameStartingTime(double wp, int n_act, const int act[]);
    ConstraintActivitiesSameStartingTime(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& act);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It aims at scheduling a set of activities so that they do not overlap.
The number of conflicts is considered the number of overlapping
hours.
*/
class ConstraintActivitiesNotOverlapping: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesNotOverlapping)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;
    //int activitiesId[MAX_CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING];

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    //int _activities[MAX_CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING];
    QList<int> _activities;

    ConstraintActivitiesNotOverlapping(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities.
        */
    ConstraintActivitiesNotOverlapping(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& act);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It aims at scheduling a set of activities so that they
have a minimum of N days between any two of them.
*/
class ConstraintMinDaysBetweenActivities: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMinDaysBetweenActivities)

public:
    bool consecutiveIfSameDay;

    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;
    //int activitiesId[MAX_CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES];

    /**
        The number of minimum days between each 2 activities
        */
    int minDays;

    //internal structure (redundant)

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    QList<int> _activities;
    //int _activities[MAX_CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES];

    ConstraintMinDaysBetweenActivities(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities.
        */
    //ConstraintMinDaysBetweenActivities(double wp, bool adjacentIfBroken, int n_act, const int act[], int n);
    ConstraintMinDaysBetweenActivities(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, bool adjacentIfBroken, int n_act, const QList<int>& act, int n);

    /**
        Comparison operator - to be sure that we do not introduce duplicates
        */
    bool operator==(ConstraintMinDaysBetweenActivities& c);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintMaxDaysBetweenActivities: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMaxDaysBetweenActivities)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;

    /**
        The number of maximum days between each 2 activities
        */
    int maxDays;

    //internal structure (redundant)

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    QList<int> _activities;

    ConstraintMaxDaysBetweenActivities(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities.
        */
    ConstraintMaxDaysBetweenActivities(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& act, int n);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintMinGapsBetweenActivities: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMinGapsBetweenActivities)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;

    /**
        The number of minimum gaps between each 2 activities, if on the same day
        */
    int minGaps;

    //internal structure (redundant)

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    QList<int> _activities;

    ConstraintMinGapsBetweenActivities(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities.
        */
    //ConstraintMinGapsBetweenActivities(double wp, int n_act, const int act[], int ngaps);
    ConstraintMinGapsBetweenActivities(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& actList, int ngaps);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint, aimed at obtaining timetables
which do not allow more than X hours in a day for any teacher
*/
class ConstraintTeachersMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxHoursDaily)

public:
    /**
        The maximum hours daily
        */
    int maxHoursDaily;

    ConstraintTeachersMaxHoursDaily(Instance &r);

    ConstraintTeachersMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxHoursDaily)

public:
    /**
        The maximum hours daily
        */
    int maxHoursDaily;

    QString teacherName;

    int teacher_ID;

    ConstraintTeacherMaxHoursDaily(Instance &r);

    ConstraintTeacherMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint, aimed at obtaining timetables
which do not allow more than X hours in a row for any teacher
*/
class ConstraintTeachersMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxHoursContinuously)

public:
    /**
        The maximum hours continuously
        */
    int maxHoursContinuously;

    ConstraintTeachersMaxHoursContinuously(Instance &r);

    ConstraintTeachersMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxHoursContinuously)

public:
    /**
        The maximum hours continuously
        */
    int maxHoursContinuously;

    QString teacherName;

    int teacher_ID;

    ConstraintTeacherMaxHoursContinuously(Instance &r);

    ConstraintTeacherMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
The resulting timetable must respect the requirement
that this teacher must not have too much working
days per week.
*/
class ConstraintTeacherMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week (-1 for don't care)
        */
    int maxDaysPerWeek;

    /**
        The teacher's name
        */
    QString teacherName;

    /**
        The teacher's id, or index in the rules
        */
    int teacher_ID;

    ConstraintTeacherMaxDaysPerWeek(Instance &r);

    ConstraintTeacherMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString t);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week (-1 for don't care)
        */
    int maxDaysPerWeek;

    ConstraintTeachersMaxDaysPerWeek(Instance &r);

    ConstraintTeachersMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMinDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMinDaysPerWeek)

public:
    int minDaysPerWeek;

    /**
        The teacher's name
        */
    QString teacherName;

    /**
        The teacher's id, or index in the rules
        */
    int teacher_ID;

    ConstraintTeacherMinDaysPerWeek(Instance &r);

    ConstraintTeacherMinDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mindays, const QString& t);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMinDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMinDaysPerWeek)

public:
    int minDaysPerWeek;

    ConstraintTeachersMinDaysPerWeek(Instance &r);

    ConstraintTeachersMinDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mindays);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It constrains the timetable to not schedule any activity
in the specified day, during the start hour and end hour.
*/
class ConstraintBreakTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintBreakTimes)

public:
    QList<int> days;
    QList<int> hours;

    ConstraintBreakTimes(Instance &r);

    ConstraintBreakTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QList<int> d, QList<int> h);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint. It adds, to the violationsFactor of
the solution, a conflicts factor computed from the gaps
existing in the timetable (regarding the students).
The overall result is a timetable having less gaps for the students.
*/
class ConstraintStudentsMaxGapsPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxGapsPerWeek)

public:
    int maxGaps;

    ConstraintStudentsMaxGapsPerWeek(Instance &r);

    ConstraintStudentsMaxGapsPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mg);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint. It adds, to the violationsFactor of
the solution, a conflicts factor computed from the gaps
existing in the timetable (regarding the specified students set).
*/
class ConstraintStudentsSetMaxGapsPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxGapsPerWeek)

public:
    int maxGaps;

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxGapsPerWeek(Instance &r);

    ConstraintStudentsSetMaxGapsPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mg, const QString& st );

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMaxGapsPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxGapsPerWeek)

public:
    int maxGaps;

    ConstraintTeachersMaxGapsPerWeek(Instance &r);

    ConstraintTeachersMaxGapsPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxGaps);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxGapsPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxGapsPerWeek)

public:
    int maxGaps;

    QString teacherName;

    int teacherIndex;

    ConstraintTeacherMaxGapsPerWeek(Instance &r);

    ConstraintTeacherMaxGapsPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QString tn, int maxGaps);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMaxGapsPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxGapsPerDay)

public:
    int maxGaps;

    ConstraintTeachersMaxGapsPerDay(Instance &r);

    ConstraintTeachersMaxGapsPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxGaps);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxGapsPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxGapsPerDay)

public:
    int maxGaps;

    QString teacherName;

    int teacherIndex;

    ConstraintTeacherMaxGapsPerDay(Instance &r);

    ConstraintTeacherMaxGapsPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QString tn, int maxGaps);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint. It adds, to the violationsFactor of
the solution, a violationsFactor factor that is related to how early
the students begin their courses. The result is a timetable
having more activities scheduled at the beginning of the day.
IMPORTANT: fortnightly activities are treated as weekly ones,
for speed and because in normal situations this does not matter.
*/
class ConstraintStudentsEarlyMaxBeginningsAtSecondHour: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsEarlyMaxBeginningsAtSecondHour)

public:

    int maxBeginningsAtSecondHour;

    ConstraintStudentsEarlyMaxBeginningsAtSecondHour(Instance &r);

    ConstraintStudentsEarlyMaxBeginningsAtSecondHour(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mBSH);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour)

public:
    int maxBeginningsAtSecondHour;

    /**
        The name of the students
        */
    QString students;

    /**
        The number of subgroups involved in this restriction
        */
    //int nSubgroups;

    /**
        The subgroups involved in this restriction
        */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(Instance &r);

    ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mBSH, const QString& students);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxHoursDaily)

public:
    int maxHoursDaily;

    ConstraintStudentsMaxHoursDaily(Instance &r);

    ConstraintStudentsMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxHoursDaily)

public:
    int maxHoursDaily;

    /**
        The students set name
        */
    QString students;

    //internal variables

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxHoursDaily(Instance &r);

    ConstraintStudentsSetMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, QString s);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxHoursContinuously)

public:
    int maxHoursContinuously;

    ConstraintStudentsMaxHoursContinuously(Instance &r);

    ConstraintStudentsMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxHoursContinuously)

public:
    int maxHoursContinuously;

    /**
        The students set name
        */
    QString students;

    //internal variables

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxHoursContinuously(Instance &r);

    ConstraintStudentsSetMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, QString s);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMinHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMinHoursDaily)

public:
    int minHoursDaily;

    bool allowEmptyDays;

    ConstraintStudentsMinHoursDaily(Instance &r);

    ConstraintStudentsMinHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minnh, bool _allowEmptyDays);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMinHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMinHoursDaily)

public:
    int minHoursDaily;

    /**
        The students set name
        */
    QString students;

    bool allowEmptyDays;

    //internal variables

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMinHoursDaily(Instance &r);

    ConstraintStudentsSetMinHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minnh, QString s, bool _allowEmptyDays);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It adds, to the violationsFactor of the solution, a violationsFactor factor that
grows as the activity is scheduled farther from the wanted time
For the moment, violationsFactor factor increases with one unit for every hour
and one unit for every day.
*/
class ConstraintActivityPreferredStartingTime: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivityPreferredStartingTime)

public:
    /**
        Activity id
        */
    int activityId;

    /**
        The preferred day. If -1, then the user does not care about the day.
        */
    int day;

    /**
        The preferred hour. If -1, then the user does not care about the hour.
        */
    int hour;

    bool permanentlyLocked; //if this is true, then this activity cannot be unlocked from the timetable view form

    //internal variables
    /**
        The index of the activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int activityIndex;

    ConstraintActivityPreferredStartingTime(Instance &r);

    ConstraintActivityPreferredStartingTime(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int d, int h, bool perm);

    /**
        Comparison operator - to be sure that we do not introduce duplicates
        */
    bool operator==(ConstraintActivityPreferredStartingTime& c);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It returns conflicts if the activity is scheduled in another interval
than the preferred set of times.
*/
class ConstraintActivityPreferredTimeSlots: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivityPreferredTimeSlots)

public:
    /**
        Activity id
        */
    int p_activityId;

    /**
        The number of preferred times
        */
    int p_nPreferredTimeSlots_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int p_days[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS];
    QList<int> p_days_L;

    /**
        The preferred hour. If -1, then the user does not care about the hour.
        */
    //int p_hours[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS];
    QList<int> p_hours_L;

    //internal variables
    /**
        The index of the activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int p_activityIndex;

    ConstraintActivityPreferredTimeSlots(Instance &r);

    //ConstraintActivityPreferredTimeSlots(double wp, int actId, int nPT, int d[], int h[]);
    ConstraintActivityPreferredTimeSlots(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivityPreferredStartingTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivityPreferredStartingTimes)

public:
    /**
        Activity id
        */
    int activityId;

    /**
        The number of preferred times
        */
    int nPreferredStartingTimes_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int days[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES];
    QList<int> days_L;

    /**
        The preferred hour. If -1, then the user does not care about the hour.
        */
    //int hours[MAX_N_CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES];
    QList<int> hours_L;

    //internal variables
    /**
        The index of the activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int activityIndex;

    ConstraintActivityPreferredStartingTimes(Instance &r);

    //ConstraintActivityPreferredStartingTimes(double wp, int actId, int nPT, int d[], int h[]);
    ConstraintActivityPreferredStartingTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

/**
This is a constraint.
It returns conflicts if a set of activities is scheduled in another interval
than the preferred set of times.
The set of activities is specified by a subject, teacher, students or a combination
of these.
*/
class ConstraintActivitiesPreferredTimeSlots: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesPreferredTimeSlots)

public:
    /**
        The teacher. If void, all teachers.
        */
    QString p_teacherName;

    /**
        The students. If void, all students.
        */
    QString p_studentsName;

    /**
        The subject. If void, all subjects.
        */
    QString p_subjectName;

    /**
        The activity tag. If void, all activity tags.
        */
    QString p_activityTagName;

    int duration; //if -1, neglected. Otherwise, >=1.

    /**
        The number of preferred times
        */
    int p_nPreferredTimeSlots_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int p_days[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];
    QList<int> p_days_L;

    /**
        The preferred hours. If -1, then the user does not care about the hour.
        */
    //int p_hours[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];
    QList<int> p_hours_L;

    //internal variables

    /**
        The number of activities which are represented by the subject, teacher and students requirements.
        */
    int p_nActivities;

    /**
        The indices of the activities in the rules (from 0 to rules.nActivities-1)
        These are indices in the internal list -> Instance::internalActivitiesList
        */
    //int p_activitiesIndices[MAX_ACTIVITIES];
    QList<int> p_activitiesIndices;

    ConstraintActivitiesPreferredTimeSlots(Instance &r);

    //ConstraintActivitiesPreferredTimeSlots(double wp, QString te,
    //	QString st, QString su, QString sut, int nPT, int d[], int h[]);
    ConstraintActivitiesPreferredTimeSlots(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QString te,
                                           QString st, QString su, QString sut, int dur, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintSubactivitiesPreferredTimeSlots: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintSubactivitiesPreferredTimeSlots)

public:
    int componentNumber;

    /**
        The teacher. If void, all teachers.
        */
    QString p_teacherName;

    /**
        The students. If void, all students.
        */
    QString p_studentsName;

    /**
        The subject. If void, all subjects.
        */
    QString p_subjectName;

    /**
        The activity tag. If void, all activity tags.
        */
    QString p_activityTagName;

    /**
        The number of preferred times
        */
    int p_nPreferredTimeSlots_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int p_days[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];
    QList<int> p_days_L;

    /**
        The preferred hours. If -1, then the user does not care about the hour.
        */
    //int p_hours[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS];
    QList<int> p_hours_L;

    //internal variables

    /**
        The number of activities which are represented by the subject, teacher and students requirements.
        */
    int p_nActivities;

    /**
        The indices of the activities in the rules (from 0 to rules.nActivities-1)
        These are indices in the internal list -> Instance::internalActivitiesList
        */
    //int p_activitiesIndices[MAX_ACTIVITIES];
    QList<int> p_activitiesIndices;

    ConstraintSubactivitiesPreferredTimeSlots(Instance &r);

    //ConstraintSubactivitiesPreferredTimeSlots(double wp, int compNo, QString te,
    //	QString st, QString su, QString sut, int nPT, int d[], int h[]);
    ConstraintSubactivitiesPreferredTimeSlots(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int compNo, QString te,
                                              QString st, QString su, QString sut, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesPreferredStartingTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesPreferredStartingTimes)

public:
    /**
        The teacher. If void, all teachers.
        */
    QString teacherName;

    /**
        The students. If void, all students.
        */
    QString studentsName;

    /**
        The subject. If void, all subjects.
        */
    QString subjectName;

    /**
        The activity tag. If void, all activity tags.
        */
    QString activityTagName;

    int duration; //if -1, neglected. Otherwise, >=1.

    /**
        The number of preferred times
        */
    int nPreferredStartingTimes_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int days[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES];
    QList<int> days_L;

    /**
        The preferred hours. If -1, then the user does not care about the hour.
        */
    //int hours[MAX_N_CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES];
    QList<int> hours_L;

    //internal variables

    /**
        The number of activities which are represented by the subject, teacher and students requirements.
        */
    int nActivities;

    /**
        The indices of the activities in the rules (from 0 to rules.nActivities-1)
        These are indices in the internal list -> Instance::internalActivitiesList
        */
    //int activitiesIndices[MAX_ACTIVITIES];
    QList<int> activitiesIndices;

    ConstraintActivitiesPreferredStartingTimes(Instance &r);

    //ConstraintActivitiesPreferredStartingTimes(double wp, QString te,
    //	QString st, QString su, QString sut, int nPT, int d[], int h[]);
    ConstraintActivitiesPreferredStartingTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QString te,
                                               QString st, QString su, QString sut, int dur, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintSubactivitiesPreferredStartingTimes: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintSubactivitiesPreferredStartingTimes)

public:
    int componentNumber;

    /**
        The teacher. If void, all teachers.
        */
    QString teacherName;

    /**
        The students. If void, all students.
        */
    QString studentsName;

    /**
        The subject. If void, all subjects.
        */
    QString subjectName;

    /**
        The activity tag. If void, all activity tags.
        */
    QString activityTagName;

    /**
        The number of preferred times
        */
    int nPreferredStartingTimes_L;

    /**
        The preferred days. If -1, then the user does not care about the day.
        */
    //int days[MAX_N_CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES];
    QList<int> days_L;

    /**
        The preferred hours. If -1, then the user does not care about the hour.
        */
    //int hours[MAX_N_CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES];
    QList<int> hours_L;

    //internal variables

    /**
        The number of activities which are represented by the subject, teacher and students requirements.
        */
    int nActivities;

    /**
        The indices of the activities in the rules (from 0 to rules.nActivities-1)
        These are indices in the internal list -> Instance::internalActivitiesList
        */
    //int activitiesIndices[MAX_ACTIVITIES];
    QList<int> activitiesIndices;

    ConstraintSubactivitiesPreferredStartingTimes(Instance &r);

    ConstraintSubactivitiesPreferredStartingTimes(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int compNo, QString te,
                                                  QString st, QString su, QString sut, int nPT_L, QList<int> d_L, QList<int> h_L);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesSameStartingHour: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesSameStartingHour)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;
    //int activitiesId[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR];

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    QList<int> _activities;
    //int _activities[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR];

    ConstraintActivitiesSameStartingHour(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities' id-s.
        */
    //ConstraintActivitiesSameStartingHour(double wp, int n_act, const int act[]);
    ConstraintActivitiesSameStartingHour(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& act);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);


    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesSameStartingDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesSameStartingDay)

public:
    /**
        The number of activities involved in this constraint
        */
    int n_activities;

    /**
        The activities involved in this constraint (id)
        */
    QList<int> activitiesId;
    //int activitiesId[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY];

    /**
        The number of activities involved in this constraint - internal structure
        */
    int _n_activities;

    /**
        The activities involved in this constraint (index in the rules) - internal structure
        */
    //int _activities[MAX_CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY];
    QList<int> _activities;

    ConstraintActivitiesSameStartingDay(Instance &r);

    /**
        Constructor, using:
        the weight, the number of activities and the list of activities' id-s.
        */
    //ConstraintActivitiesSameStartingDay(double wp, int n_act, const int act[]);
    ConstraintActivitiesSameStartingDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int n_act, const QList<int>& act);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTwoActivitiesConsecutive: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTwoActivitiesConsecutive)

public:
    /**
        First activity id
        */
    int firstActivityId;

    /**
        Second activity id
        */
    int secondActivityId;

    //internal variables
    /**
        The index of the first activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int firstActivityIndex;

    /**
        The index of the second activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int secondActivityIndex;

    ConstraintTwoActivitiesConsecutive(Instance &r);

    ConstraintTwoActivitiesConsecutive(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTwoActivitiesGrouped: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTwoActivitiesGrouped)

public:
    /**
        First activity id
        */
    int firstActivityId;

    /**
        Second activity id
        */
    int secondActivityId;

    //internal variables
    /**
        The index of the first activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int firstActivityIndex;

    /**
        The index of the second activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int secondActivityIndex;

    ConstraintTwoActivitiesGrouped(Instance &r);

    ConstraintTwoActivitiesGrouped(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintThreeActivitiesGrouped: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintThreeActivitiesGrouped)

public:
    /**
        First activity id
        */
    int firstActivityId;

    /**
        Second activity id
        */
    int secondActivityId;

    int thirdActivityId;

    //internal variables
    /**
        The index of the first activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int firstActivityIndex;

    /**
        The index of the second activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int secondActivityIndex;

    int thirdActivityIndex;

    ConstraintThreeActivitiesGrouped(Instance &r);

    ConstraintThreeActivitiesGrouped(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId, int thirdActId);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTwoActivitiesOrdered: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTwoActivitiesOrdered)

public:
    /**
        First activity id
        */
    int firstActivityId;

    /**
        Second activity id
        */
    int secondActivityId;

    //internal variables
    /**
        The index of the first activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int firstActivityIndex;

    /**
        The index of the second activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int secondActivityIndex;

    ConstraintTwoActivitiesOrdered(Instance &r);

    ConstraintTwoActivitiesOrdered(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivityEndsStudentsDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivityEndsStudentsDay)

public:
    /**
        Activity id
        */
    int activityId;

    //internal variables
    /**
        The index of the activity in the rules (from 0 to rules.nActivities-1) - it is not the id of the activity
        */
    int activityIndex;

    ConstraintActivityEndsStudentsDay(Instance &r);

    ConstraintActivityEndsStudentsDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int actId);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMinHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMinHoursDaily)

public:
    /**
        The minimum hours daily
        */
    int minHoursDaily;

    bool allowEmptyDays;

    ConstraintTeachersMinHoursDaily(Instance &r);

    ConstraintTeachersMinHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minhours, bool _allowEmptyDays);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMinHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMinHoursDaily)

public:
    /**
        The minimum hours daily
        */
    int minHoursDaily;

    QString teacherName;

    int teacher_ID;

    bool allowEmptyDays;

    ConstraintTeacherMinHoursDaily(Instance &r);

    ConstraintTeacherMinHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minhours, const QString& teacher, bool _allowEmptyDays);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherIntervalMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherIntervalMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    int startHour;

    int endHour; //might be = to Timetable::getInstance()->rules.nHoursPerDay

    /**
        The teacher's name
        */
    QString teacherName;

    /**
        The teacher's id, or index in the rules
        */
    int teacher_ID;

    ConstraintTeacherIntervalMaxDaysPerWeek(Instance &r);

    ConstraintTeacherIntervalMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString tn, int sh, int eh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersIntervalMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersIntervalMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    int startHour;

    int endHour; //might be = to Timetable::getInstance()->rules.nHoursPerDay


    ConstraintTeachersIntervalMaxDaysPerWeek(Instance &r);

    ConstraintTeachersIntervalMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, int sh, int eh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetIntervalMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetIntervalMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    int startHour;

    int endHour; //might be = to Timetable::getInstance()->rules.nHoursPerDay

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetIntervalMaxDaysPerWeek(Instance &r);

    ConstraintStudentsSetIntervalMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString sn, int sh, int eh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsIntervalMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsIntervalMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    int startHour;

    int endHour; //might be = to Timetable::getInstance()->rules.nHoursPerDay


    ConstraintStudentsIntervalMaxDaysPerWeek(Instance &r);

    ConstraintStudentsIntervalMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, int sh, int eh);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesEndStudentsDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesEndStudentsDay)

public:
    /**
        The teacher. If void, all teachers.
        */
    QString teacherName;

    /**
        The students. If void, all students.
        */
    QString studentsName;

    /**
        The subject. If void, all subjects.
        */
    QString subjectName;

    /**
        The activity tag. If void, all activity tags.
        */
    QString activityTagName;


    //internal data

    /**
        The number of activities which are represented by the subject, teacher and students requirements.
        */
    int nActivities;

    /**
        The indices of the activities in the rules (from 0 to rules.nActivities-1)
        These are indices in the internal list -> Instance::internalActivitiesList
        */
    //int activitiesIndices[MAX_ACTIVITIES];
    QList<int> activitiesIndices;

    ConstraintActivitiesEndStudentsDay(Instance &r);

    ConstraintActivitiesEndStudentsDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QString te, QString st, QString su, QString sut);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMaxGapsPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxGapsPerDay)

public:
    int maxGaps;

    ConstraintStudentsMaxGapsPerDay(Instance &r);

    ConstraintStudentsMaxGapsPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mg);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMaxGapsPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxGapsPerDay)

public:
    int maxGaps;

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxGapsPerDay(Instance &r);

    ConstraintStudentsSetMaxGapsPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int mg, const QString& st );

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesOccupyMaxTimeSlotsFromSelection: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesOccupyMaxTimeSlotsFromSelection)

public:
    QList<int> activitiesIds;

    QList<int> selectedDays;
    QList<int> selectedHours;

    int maxOccupiedTimeSlots;

    //internal variables
    QList<int> _activitiesIndices;

    ConstraintActivitiesOccupyMaxTimeSlotsFromSelection(Instance &r);

    ConstraintActivitiesOccupyMaxTimeSlotsFromSelection(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QList<int> a_L, QList<int> d_L, QList<int> h_L, int max_slots);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots)

public:
    QList<int> activitiesIds;

    QList<int> selectedDays;
    QList<int> selectedHours;

    int maxSimultaneous;

    //internal variables
    QList<int> _activitiesIndices;

    ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots(Instance &r);

    ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, QList<int> a_L, QList<int> d_L, QList<int> h_L, int max_simultaneous);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxDaysPerWeek(Instance &r);

    ConstraintStudentsSetMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString sn);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMaxDaysPerWeek: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxDaysPerWeek)

public:
    /**
        The number of maximum allowed working days per week
        */
    int maxDaysPerWeek;

    ConstraintStudentsMaxDaysPerWeek(Instance &r);

    ConstraintStudentsMaxDaysPerWeek(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxSpanPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxSpanPerDay)

public:
    /**
        The maximum span daily
        */
    int maxSpanPerDay;

    QString teacherName;

    int teacher_ID;

    ConstraintTeacherMaxSpanPerDay(Instance &r);

    ConstraintTeacherMaxSpanPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan, const QString& teacher);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMaxSpanPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxSpanPerDay)

public:
    /**
        The maximum span daily
        */
    int maxSpanPerDay;

    ConstraintTeachersMaxSpanPerDay(Instance &r);

    ConstraintTeachersMaxSpanPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMinRestingHours: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMinRestingHours)

public:
    int minRestingHours;

    bool circular;

    QString teacherName;

    int teacher_ID;

    ConstraintTeacherMinRestingHours(Instance &r);

    ConstraintTeacherMinRestingHours(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ, const QString& teacher);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersMinRestingHours: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMinRestingHours)

public:
    int minRestingHours;

    bool circular;

    ConstraintTeachersMinRestingHours(Instance &r);

    ConstraintTeachersMinRestingHours(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMaxSpanPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMaxSpanPerDay)

public:
    int maxSpanPerDay;

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMaxSpanPerDay(Instance &r);

    ConstraintStudentsSetMaxSpanPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan, QString sn);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMaxSpanPerDay: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMaxSpanPerDay)

public:
    int maxSpanPerDay;

    ConstraintStudentsMaxSpanPerDay(Instance &r);

    ConstraintStudentsMaxSpanPerDay(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetMinRestingHours: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetMinRestingHours)

public:
    int minRestingHours;

    bool circular;

    /**
        The name of the students set for this constraint
        */
    QString students;

    //internal redundant data

    /**
        The number of subgroups
        */
    //int nSubgroups;

    /**
        The subgroups
        */
    QList<int> iSubgroupsList;

    ConstraintStudentsSetMinRestingHours(Instance &r);

    ConstraintStudentsSetMinRestingHours(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ, QString sn);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsMinRestingHours: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsMinRestingHours)

public:
    int minRestingHours;

    bool circular;

    ConstraintStudentsMinRestingHours(Instance &r);

    ConstraintStudentsMinRestingHours(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintMinActivitiesForAllTeachers: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMinActivitiesForAllTeachers)

public:

    int minActivities;

    ConstraintMinActivitiesForAllTeachers(Instance &r);

    ConstraintMinActivitiesForAllTeachers(Instance &r, Enums::ConstraintGroup constraintGroup, int minActivities);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};


class ConstraintMinActivitiesForTeacher: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMinActivitiesForTeacher)

public:

    int minActivities;

    QString teacherName;

    int teacher_ID;

    ConstraintMinActivitiesForTeacher(Instance &r);

    ConstraintMinActivitiesForTeacher(Instance &r, Enums::ConstraintGroup constraintGroup, int minActivities, const QString& name);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintMaxActivitiesForAllTeachers: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMaxActivitiesForAllTeachers)

public:

    int maxActivities;

    ConstraintMaxActivitiesForAllTeachers(Instance &r);

    ConstraintMaxActivitiesForAllTeachers(Instance &r, Enums::ConstraintGroup constraintGroup, int maxActivities);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};


class ConstraintMaxActivitiesForTeacher: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintMaxActivitiesForTeacher)

public:

    int maxActivities;

    QString teacherName;

    int teacher_ID;

    ConstraintMaxActivitiesForTeacher(Instance &r);

    ConstraintMaxActivitiesForTeacher(Instance &r, Enums::ConstraintGroup constraintGroup, int maxActivities, const QString& name);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    void removeUseless();

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};


class ConstraintTeachersMaxHoursInWorkDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersMaxHoursInWorkDaily)

public:
    /**
        The maximum hours daily
        */
    int maxHoursInWork;

    ConstraintTeachersMaxHoursInWorkDaily(Instance &r);

    ConstraintTeachersMaxHoursInWorkDaily(Instance &r, Enums::ConstraintGroup constraintGroup, int maxHoursInWork);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherMaxHoursInWorkDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherMaxHoursInWorkDaily)

public:
    /**
        The maximum hours daily
        */
    int maxHoursInWork;

    QString teacherName;

    int teacher_ID;

    ConstraintTeacherMaxHoursInWorkDaily(Instance &r);

    ConstraintTeacherMaxHoursInWorkDaily(Instance &r, Enums::ConstraintGroup constraintGroup, int maxHoursInWork, const QString& teacher);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintNoTeacherInSubjectPreference: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintNoTeacherInSubjectPreference)

public:

    Enums::SubjectPreference pref;

    ConstraintNoTeacherInSubjectPreference(Instance &r);

    ConstraintNoTeacherInSubjectPreference(Instance &r, Enums::ConstraintGroup constraintGroup, Enums::SubjectPreference pref);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeachersActivityTagMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersActivityTagMaxHoursContinuously)

public:
    /**
    The maximum hours continuously
    */
    int maxHoursContinuously;

    QString activityTagName;

    int activityTagIndex;

    QList<int> canonicalTeachersList;

    ConstraintTeachersActivityTagMaxHoursContinuously(Instance &r);

    ConstraintTeachersActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& activityTag);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherActivityTagMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherActivityTagMaxHoursContinuously)

public:
    /**
    The maximum hours continuously
    */
    int maxHoursContinuously;

    QString teacherName;

    QString activityTagName;

    int teacher_ID;

    int activityTagIndex;

    QList<int> canonicalTeachersList;

    ConstraintTeacherActivityTagMaxHoursContinuously(Instance &r);

    ConstraintTeacherActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher, const QString& activityTag);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};


class ConstraintStudentsActivityTagMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsActivityTagMaxHoursContinuously)

public:
    int maxHoursContinuously;

    QString activityTagName;

    int activityTagIndex;

    QList<int> canonicalSubgroupsList;

    ConstraintStudentsActivityTagMaxHoursContinuously(Instance &r);

    ConstraintStudentsActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& activityTag);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetActivityTagMaxHoursContinuously: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetActivityTagMaxHoursContinuously)

public:
    int maxHoursContinuously;

    /**
    The students set name
    */
    QString students;

    QString activityTagName;

    //internal variables

    int activityTagIndex;

    /**
    The number of subgroups
    */
    //int nSubgroups;

    /**
    The subgroups
    */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    QList<int> canonicalSubgroupsList;

    ConstraintStudentsSetActivityTagMaxHoursContinuously(Instance &r);

    ConstraintStudentsSetActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& s, const QString& activityTag);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};


class ConstraintTeachersActivityTagMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeachersActivityTagMaxHoursDaily)

public:
    /**
    The maximum hours daily
    */
    int maxHoursDaily;

    QString activityTagName;

    int activityTagIndex;

    QList<int> canonicalTeachersList;

    ConstraintTeachersActivityTagMaxHoursDaily(Instance &r);

    ConstraintTeachersActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& activityTag);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintTeacherActivityTagMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintTeacherActivityTagMaxHoursDaily)

public:
    /**
    The maximum hours daily
    */
    int maxHoursDaily;

    QString teacherName;

    QString activityTagName;

    int teacher_ID;

    int activityTagIndex;

    QList<int> canonicalTeachersList;

    ConstraintTeacherActivityTagMaxHoursDaily(Instance &r);

    ConstraintTeacherActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher, const QString& activityTag);

    QString getXmlDescription();

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsActivityTagMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsActivityTagMaxHoursDaily)

public:
    int maxHoursDaily;

    QString activityTagName;

    int activityTagIndex;

    QList<int> canonicalSubgroupsList;

    ConstraintStudentsActivityTagMaxHoursDaily(Instance &r);

    ConstraintStudentsActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& activityTag);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity(Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet(StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

class ConstraintStudentsSetActivityTagMaxHoursDaily: public TimeConstraint{
    Q_DECLARE_TR_FUNCTIONS(ConstraintStudentsSetActivityTagMaxHoursDaily)

public:
    int maxHoursDaily;

    /**
    The students set name
    */
    QString students;

    QString activityTagName;

    //internal variables

    int activityTagIndex;

    /**
    The number of subgroups
    */
    //int nSubgroups;

    /**
    The subgroups
    */
    //int subgroups[MAX_SUBGROUPS_PER_CONSTRAINT];
    QList<int> iSubgroupsList;

    QList<int> canonicalSubgroupsList;

    ConstraintStudentsSetActivityTagMaxHoursDaily(Instance &r);

    ConstraintStudentsSetActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& s, const QString& activityTag);

    bool computeInternalStructure(QWidget* parent);

    bool hasInactiveActivities();

    QString getXmlDescription();

    QString getDescription();

    QString getDetailedDescription();

    double violationsFactor(Solution& c, bool collectConflictsData=false);

    bool isRelatedToActivity( Activity* a);

    bool isRelatedToTeacher(Teacher* t);

    bool isRelatedToSubject(Subject* s);

    bool isRelatedToActivityTag(ActivityTag* s);

    bool isRelatedToStudentsSet( StudentsSet* s);

    bool hasWrongDayOrHour();
    bool canRepairWrongDayOrHour();
    bool repairWrongDayOrHour();
};

#endif

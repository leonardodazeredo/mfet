/***************************************************************************
                          generate_pre.h  -  description
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

#ifndef GENERATE_PRE_H
#define GENERATE_PRE_H

#include "defs.h"
#include "tcontext.h"

#include "matrix.h"

#include <QSet>
#include <QHash>
#include <QList>

#include <QTextStream>

class QWidget;

////////BEGIN st. not available, tch not avail., break, activity preferred time,
////////activity preferred times, activities preferred times
//percentage of allowed time, -1 if no restriction
////extern double notAllowedTimesPercentages[MAX_ACTIVITIES][MAX_HOURS_PER_WEEK];

////////END   st. not available, tch not avail., break, activity preferred time,
////////activity preferred time, activities preferred times


//it is practically better to use the variable below and to put it exactly like in generate.cpp,
//the order of activities changes
/////extern QList<int> teacherActivitiesOfTheDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];
//extern Matrix1D<QList<int> > subgroupsWithMaxDaysPerWeekForActivities;


////extern Matrix1D<QList<int> > mustComputeTimetableSubgroups;
////extern Matrix1D<QList<int> > mustComputeTimetableTeachers;
////extern bool mustComputeTimetableSubgroup[MAX_TOTAL_SUBGROUPS];
////extern bool mustComputeTimetableTeacher[MAX_TEACHERS];


//extern bool haveActivitiesOccupyOrSimultaneousConstraints;

/////////////////////////////////////////////////////////////////////////

//extern bool fixedTimeActivity[MAX_ACTIVITIES];

//-----------

bool compareFunctionGeneratePre(int i, int j);


bool processConstraints(QWidget* parent);


////////BEGIN BASIC TIME CONSTRAINTS
////extern bool activitiesConflicting[MAX_ACTIVITIES][MAX_ACTIVITIES];
////extern qint8 activitiesConflictingPercentage[MAX_ACTIVITIES][MAX_ACTIVITIES]; //-1 for not conflicting,
////extern Matrix2D<qint8> activitiesConflictingPercentage; //-1 for not conflicting,
//a value >=0 equal with the weight of the maximum weightPercentage of a basic time constraint
//extern Matrix1D<QHash<int, int> > activitiesConflictingPercentage;

bool computeActivitiesConflictingPercentage(QWidget* parent);
//void computeActivitiesConflicting();

////////END   BASIC TIME CONSTRAINTS


////////BEGIN MIN DAYS TIME CONSTRAINTS
//extern Matrix1D<QList<int> > minDaysListOfActivities;
//extern Matrix1D<QList<int> > minDaysListOfMinDays;
//extern Matrix1D<QList<double> > minDaysListOfWeightPercentages;
//extern Matrix1D<QList<bool> > minDaysListOfConsecutiveIfSameDay;

bool computeMinDays(QWidget* parent);
////////END   MIN DAYS TIME CONSTRAINTS


////////BEGIN MAX DAYS TIME CONSTRAINTS
//extern Matrix1D<QList<int> > maxDaysListOfActivities;
//extern Matrix1D<QList<int> > maxDaysListOfMaxDays;
//extern Matrix1D<QList<double> > maxDaysListOfWeightPercentages;

bool computeMaxDays(QWidget* parent);
////////END   MAX DAYS TIME CONSTRAINTS


////////BEGIN MIN GAPS between activities TIME CONSTRAINTS
//extern Matrix1D<QList<int> > minGapsBetweenActivitiesListOfActivities;
//extern Matrix1D<QList<int> > minGapsBetweenActivitiesListOfMinGaps;
//extern Matrix1D<QList<double> > minGapsBetweenActivitiesListOfWeightPercentages;

bool computeMinGapsBetweenActivities(QWidget* parent);
////////END	 MIN GAPS between activities TIME CONSTRAINTS


////////BEGIN st. not available, tch not avail., break, activity preferred time,
////////activity preferred times, activities preferred times
//percentage of allowed time, -1 if no restriction
////extern double notAllowedTimesPercentages[MAX_ACTIVITIES][MAX_HOURS_PER_WEEK];
//extern Matrix2D<double> notAllowedTimesPercentages;


bool computeNotAllowedTimesPercentages(QWidget* parent);
////////END   st. not available, tch not avail., break, activity preferred time,
////////activity preferred time, activities preferred times


////////BEGIN students max gaps and early
bool computeNHoursPerSubgroup(QWidget* parent);
bool computeSubgroupsEarlyAndMaxGapsPercentages(QWidget* parent);

//extern double subgroupsEarlyMaxBeginningsAtSecondHourPercentage[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[MAX_TOTAL_SUBGROUPS];
//extern double subgroupsMaxGapsPerWeekPercentage[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxGapsPerWeekMaxGaps[MAX_TOTAL_SUBGROUPS];

//extern int nHoursPerSubgroup[MAX_TOTAL_SUBGROUPS]; //used also for students min hours daily

//max gaps per day (not perfect!!!)
bool computeSubgroupsMaxGapsPerDayPercentages(QWidget* parent);

//extern double subgroupsMaxGapsPerDayPercentage[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxGapsPerDayMaxGaps[MAX_TOTAL_SUBGROUPS];
//extern bool haveStudentsMaxGapsPerDay;
////////END   students max gaps and early


////////BEGIN students max days per week
//activities indices (in 0..Timetable::getInstance()->rules.nInternalActivities-1) for each subgroup
//extern int subgroupsMaxDaysPerWeekMaxDays[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//extern double subgroupsMaxDaysPerWeekWeightPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing


////////BEGIN teachers max days per week
//activities indices (in 0..Timetable::getInstance()->rules.nInternalActivities-1) for each teacher
//extern int teachersMaxDaysPerWeekMaxDays[MAX_TEACHERS]; //-1 for not existing
//extern double teachersMaxDaysPerWeekWeightPercentages[MAX_TEACHERS]; //-1 for not existing
//it is practically better to use the variable below and to put it exactly like in generate.cpp,
//the order of activities changes
/////extern QList<int> teacherActivitiesOfTheDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];
//extern Matrix1D<QList<int> > teachersWithMaxDaysPerWeekForActivities;

bool computeMaxDaysPerWeekForTeachers(QWidget* parent);

bool computeMaxDaysPerWeekForStudents(QWidget* parent);
////////END   teachers max days per week


////////BEGIN teachers max gaps per week and per day
////extern QList<int> activitiesForTeachers[MAX_TEACHERS];
//extern double teachersMaxGapsPerWeekPercentage[MAX_TEACHERS];
//extern int teachersMaxGapsPerWeekMaxGaps[MAX_TEACHERS];

//extern double teachersMaxGapsPerDayPercentage[MAX_TEACHERS];
//extern int teachersMaxGapsPerDayMaxGaps[MAX_TEACHERS];

//extern int nHoursPerTeacher[MAX_TEACHERS];
bool computeNHoursPerTeacher(QWidget* parent);
bool computeTeachersMaxGapsPerWeekPercentage(QWidget* parent);
bool computeTeachersMaxGapsPerDayPercentage(QWidget* parent);
////////END   teachers max gaps per week and per day


////////BEGIN activities same starting time
//extern Matrix1D<QList<int> > activitiesSameStartingTimeActivities;
//extern Matrix1D<QList<double> > activitiesSameStartingTimePercentages;
bool computeActivitiesSameStartingTime(QWidget* parent, QHash<int, int> & reprSameStartingTime, QHash<int, QSet<int> > & reprSameActivitiesSet);
////////END   activities same starting time


////////BEGIN activities same starting hour
//extern Matrix1D<QList<int> > activitiesSameStartingHourActivities;
//extern Matrix1D<QList<double> > activitiesSameStartingHourPercentages;
void computeActivitiesSameStartingHour();
////////END   activities same starting hour


////////BEGIN activities same starting day
//extern Matrix1D<QList<int> > activitiesSameStartingDayActivities;
//extern Matrix1D<QList<double> > activitiesSameStartingDayPercentages;
void computeActivitiesSameStartingDay();
////////END   activities same starting hour


////////BEGIN activities not overlapping
//extern Matrix1D<QList<int> > activitiesNotOverlappingActivities;
//extern Matrix1D<QList<double> > activitiesNotOverlappingPercentages;
void computeActivitiesNotOverlapping();
////////END   activities not overlapping


////////BEGIN teacher(s) max hours daily
////extern int teachersGapsPerDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];
////extern int teachersNHoursPerDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

//extern double teachersMaxHoursDailyPercentages1[MAX_TEACHERS];
//extern int teachersMaxHoursDailyMaxHours1[MAX_TEACHERS];
////extern int teachersRealGapsPerDay1[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

//extern double teachersMaxHoursDailyPercentages2[MAX_TEACHERS];
//extern int teachersMaxHoursDailyMaxHours2[MAX_TEACHERS];
////extern int teachersRealGapsPerDay2[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

bool computeTeachersMaxHoursDaily(QWidget* parent);
////////END   teacher(s) max hours daily


////////BEGIN teacher(s) max hours continuously
//extern double teachersMaxHoursContinuouslyPercentages1[MAX_TEACHERS];
//extern int teachersMaxHoursContinuouslyMaxHours1[MAX_TEACHERS];

//extern double teachersMaxHoursContinuouslyPercentages2[MAX_TEACHERS];
//extern int teachersMaxHoursContinuouslyMaxHours2[MAX_TEACHERS];

bool computeTeachersMaxHoursContinuously(QWidget* parent);
////////END   teacher(s) max hours continuously


///////BEGIN teacher(s) activity tag max hours daily
//extern bool haveTeachersActivityTagMaxHoursDaily;

//extern Matrix1D<QList<int> > teachersActivityTagMaxHoursDailyMaxHours;
//extern Matrix1D<QList<int> > teachersActivityTagMaxHoursDailyActivityTag;
//extern Matrix1D<QList<double> > teachersActivityTagMaxHoursDailyPercentage;
//(formerly arrays of size MAX_TEACHERS)

//bool computeTeachersActivityTagMaxHoursDaily(QWidget* parent);
///////END   teacher(s) activity tag max hours daily


///////BEGIN teacher(s) activity tag max hours continuously
//extern bool haveTeachersActivityTagMaxHoursContinuously;

//extern Matrix1D<QList<int> > teachersActivityTagMaxHoursContinuouslyMaxHours;
//extern Matrix1D<QList<int> > teachersActivityTagMaxHoursContinuouslyActivityTag;
//extern Matrix1D<QList<double> > teachersActivityTagMaxHoursContinuouslyPercentage;
//(formerly arrays of size MAX_TEACHERS)

//bool computeTeachersActivityTagMaxHoursContinuously(QWidget* parent);
///////END   teacher(s) activity tag max hours continuously


////////BEGIN teacher(s) min hours daily
//extern double teachersMinHoursDailyPercentages[MAX_TEACHERS];
//extern int teachersMinHoursDailyMinHours[MAX_TEACHERS];

bool computeTeachersMinHoursDaily(QWidget* parent);
////////END   teacher(s) min hours daily


////////BEGIN teacher(s) min days per week
//extern double teachersMinDaysPerWeekPercentages[MAX_TEACHERS];
//extern int teachersMinDaysPerWeekMinDays[MAX_TEACHERS];

bool computeTeachersMinDaysPerWeek(QWidget* parent);
////////END   teacher(s) min days per week


////////BEGIN students (set) max hours daily
//extern double subgroupsMaxHoursDailyPercentages1[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxHoursDailyMaxHours1[MAX_TOTAL_SUBGROUPS];

//extern double subgroupsMaxHoursDailyPercentages2[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxHoursDailyMaxHours2[MAX_TOTAL_SUBGROUPS];

bool computeSubgroupsMaxHoursDaily(QWidget* parent);
////////END   students (set) max hours daily


////////BEGIN students (set) max hours continuously
//extern double subgroupsMaxHoursContinuouslyPercentages1[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxHoursContinuouslyMaxHours1[MAX_TOTAL_SUBGROUPS];

//extern double subgroupsMaxHoursContinuouslyPercentages2[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMaxHoursContinuouslyMaxHours2[MAX_TOTAL_SUBGROUPS];

bool computeStudentsMaxHoursContinuously(QWidget* parent);
////////END   students (set) max hours continuously


///////BEGIN students (set) activity tag max hours daily
//extern bool haveStudentsActivityTagMaxHoursDaily;

//extern Matrix1D<QList<int> > subgroupsActivityTagMaxHoursDailyMaxHours;
//extern Matrix1D<QList<int> > subgroupsActivityTagMaxHoursDailyActivityTag;
//extern Matrix1D<QList<double> > subgroupsActivityTagMaxHoursDailyPercentage;
//(formerly arrays of size MAX_TOTAL_SUBGROUPS)

//bool computeStudentsActivityTagMaxHoursDaily(QWidget* parent);
///////END   students (set) activity tag max hours daily


///////BEGIN students (set) activity tag max hours continuously
//extern bool haveStudentsActivityTagMaxHoursContinuously;

//extern Matrix1D<QList<int> > subgroupsActivityTagMaxHoursContinuouslyMaxHours;
//extern Matrix1D<QList<int> > subgroupsActivityTagMaxHoursContinuouslyActivityTag;
//extern Matrix1D<QList<double> > subgroupsActivityTagMaxHoursContinuouslyPercentage;
//(formerly arrays of size MAX_TOTAL_SUBGROUPS)

//bool computeStudentsActivityTagMaxHoursContinuously(QWidget* parent);
///////END   students (set) activity tag max hours continuously


////////BEGIN students (set) min hours daily
//extern double subgroupsMinHoursDailyPercentages[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsMinHoursDailyMinHours[MAX_TOTAL_SUBGROUPS];
//extern bool subgroupsMinHoursDailyAllowEmptyDays[MAX_TOTAL_SUBGROUPS];
bool computeSubgroupsMinHoursDaily(QWidget* parent);
////////END   students (set) min hours daily


//////////////BEGIN 2 activities consecutive
//index represents the first activity, value in array represents the second activity
//extern Matrix1D<QList<double> > constrTwoActivitiesConsecutivePercentages;
//extern Matrix1D<QList<int> > constrTwoActivitiesConsecutiveActivities;
void computeConstrTwoActivitiesConsecutive();

//index represents the second activity, value in array represents the first activity
//extern Matrix1D<QList<double> > inverseConstrTwoActivitiesConsecutivePercentages;
//extern Matrix1D<QList<int> > inverseConstrTwoActivitiesConsecutiveActivities;
//////////////END   2 activities consecutive


//////////////BEGIN 2 activities grouped
//index represents the first activity, value in array represents the second activity
//extern Matrix1D<QList<double> > constrTwoActivitiesGroupedPercentages;
//extern Matrix1D<QList<int> > constrTwoActivitiesGroupedActivities;
void computeConstrTwoActivitiesGrouped();


//////////////BEGIN 3 activities grouped
//index represents the first activity, value in array represents the second and third activities
//extern Matrix1D<QList<double> > constrThreeActivitiesGroupedPercentages;
//extern Matrix1D<QList<QPair<int, int> > > constrThreeActivitiesGroupedActivities;
void computeConstrThreeActivitiesGrouped();


//////////////BEGIN 2 activities ordered
//index represents the first activity, value in array represents the second activity
//extern Matrix1D<QList<double> > constrTwoActivitiesOrderedPercentages;
//extern Matrix1D<QList<int> > constrTwoActivitiesOrderedActivities;
void computeConstrTwoActivitiesOrdered();

//index represents the second activity, value in array represents the first activity
//extern Matrix1D<QList<double> > inverseConstrTwoActivitiesOrderedPercentages;
//extern Matrix1D<QList<int> > inverseConstrTwoActivitiesOrderedActivities;
//////////////END   2 activities ordered


////////////BEGIN activity ends students day
//extern double activityEndsStudentsDayPercentages[MAX_ACTIVITIES]; //-1 for not existing
bool computeActivityEndsStudentsDayPercentages(QWidget* parent);
//extern bool haveActivityEndsStudentsDay;
////////////END   activity ends students day


bool checkMinDays100Percent(QWidget* parent);
bool checkMinDaysConsecutiveIfSameDay(QWidget* parent);


bool checkMaxHoursForActivityDuration(QWidget* parent);


///////BEGIN teachers interval max days per week
//extern double teachersIntervalMaxDaysPerWeekPercentages1[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekMaxDays1[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalStart1[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalEnd1[MAX_TEACHERS];

//extern double teachersIntervalMaxDaysPerWeekPercentages2[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekMaxDays2[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalStart2[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalEnd2[MAX_TEACHERS];

//extern double teachersIntervalMaxDaysPerWeekPercentages3[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekMaxDays3[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalStart3[MAX_TEACHERS];
//extern int teachersIntervalMaxDaysPerWeekIntervalEnd3[MAX_TEACHERS];

bool computeTeachersIntervalMaxDaysPerWeek(QWidget* parent);
///////END   teachers interval max days per week


///////BEGIN subgroups interval max days per week
//extern double subgroupsIntervalMaxDaysPerWeekPercentages1[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekMaxDays1[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalStart1[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalEnd1[MAX_TOTAL_SUBGROUPS];

//extern double subgroupsIntervalMaxDaysPerWeekPercentages2[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekMaxDays2[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalStart2[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalEnd2[MAX_TOTAL_SUBGROUPS];

//extern double subgroupsIntervalMaxDaysPerWeekPercentages3[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekMaxDays3[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalStart3[MAX_TOTAL_SUBGROUPS];
//extern int subgroupsIntervalMaxDaysPerWeekIntervalEnd3[MAX_TOTAL_SUBGROUPS];

bool computeSubgroupsIntervalMaxDaysPerWeek(QWidget* parent);
///////END   subgroups interval max days per week

//2017-02-06
//extern int teachersMaxSpanPerDayMaxSpan[MAX_TEACHERS]; //-1 for not existing
//extern double teachersMaxSpanPerDayPercentages[MAX_TEACHERS]; //-1 for not existing

//extern int teachersMinRestingHoursCircularMinHours[MAX_TEACHERS]; //-1 for not existing
//extern double teachersMinRestingHoursCircularPercentages[MAX_TEACHERS]; //-1 for not existing
//extern int teachersMinRestingHoursNotCircularMinHours[MAX_TEACHERS]; //-1 for not existing
//extern double teachersMinRestingHoursNotCircularPercentages[MAX_TEACHERS]; //-1 for not existing
////extern bool teachersMinRestingHoursCircular[MAX_TEACHERS]; //true for circular

//extern int subgroupsMaxSpanPerDayMaxSpan[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//extern double subgroupsMaxSpanPerDayPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing

//extern int subgroupsMinRestingHoursCircularMinHours[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//extern double subgroupsMinRestingHoursCircularPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//extern int subgroupsMinRestingHoursNotCircularMinHours[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//extern double subgroupsMinRestingHoursNotCircularPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
////extern bool subgroupsMinRestingHoursCircular[MAX_TOTAL_SUBGROUPS]; //true for circular

bool computeTeachersMaxSpanPerDay(QWidget* parent);
bool computeTeachersMinRestingHours(QWidget* parent);
bool computeSubgroupsMaxSpanPerDay(QWidget* parent);
bool computeSubgroupsMinRestingHours(QWidget* parent);
////////////

////extern Matrix1D<QList<int> > mustComputeTimetableTeachers;

//void computeMustComputeTimetableTeachers();

//2011-09-25 - Constraint activities occupy max time slots from selection

class ActivitiesOccupyMaxTimeSlotsFromSelection_item
{
public:
	//double weight; -> must be 100.0%
	QList<int> activitiesList;
	QSet<int> activitiesSet;
	QList<int> selectedTimeSlotsList;
	QSet<int> selectedTimeSlotsSet;
	int maxOccupiedTimeSlots;
};

//extern QList<ActivitiesOccupyMaxTimeSlotsFromSelection_item> aomtsList;
//extern Matrix1D<QList<ActivitiesOccupyMaxTimeSlotsFromSelection_item*> > aomtsListForActivity;

bool computeActivitiesOccupyMaxTimeSlotsFromSelection(QWidget* parent);

//2011-09-30 - Constraint activities max simultaneous in selected time slots

class ActivitiesMaxSimultaneousInSelectedTimeSlots_item
{
public:
	//double weight; -> must be 100.0%
	QList<int> activitiesList;
	QSet<int> activitiesSet;
	QList<int> selectedTimeSlotsList;
	QSet<int> selectedTimeSlotsSet;
	int maxSimultaneous;
};

//extern QList<ActivitiesMaxSimultaneousInSelectedTimeSlots_item> amsistsList;
//extern Matrix1D<QList<ActivitiesMaxSimultaneousInSelectedTimeSlots_item*> > amsistsListForActivity;

bool computeActivitiesMaxSimultaneousInSelectedTimeSlots(QWidget* parent);

/////////////////////////////////////////////////////////////////////////

bool computeFixedActivities(QWidget* parent);

class GeneratePreTranslate: public QObject{
	Q_OBJECT
};

#endif

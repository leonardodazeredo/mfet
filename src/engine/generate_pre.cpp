/*File generate_pre.cpp
*/

/***************************************************************************
                          generate_pre.cpp  -  description
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

#include "defs.h"
#include "instance.h"
#include "tcontext.h"

#include "generate_pre.h"

#include "matrix.h"

#include <iostream>
#include <algorithm>
#include <cmath>


#include "textmessages.h"

#include <QtAlgorithms>
#include <QtGlobal>

#include <QPair>
#include <QSet>
#include <QHash>
#include <QQueue>

using namespace std;



//#include <QApplication>
#ifndef FET_COMMAND_LINE

#include <QProgressDialog>
#include <QMessageBox>

#endif

////////BEGIN st. not available, tch not avail., break, activity preferred time,
////////activity preferred times, activities preferred times
//percentage of allowed time, -1 if no restriction
//double notAllowedTimesPercentages[MAX_ACTIVITIES][MAX_HOURS_PER_WEEK];

////teacher not available, which is not considered gap, false means available, true means 100% not available
////teacher not available can only be 100% or none
////bool Timetable::getInstance()->instance.teacherNotAvailableDayHour[MAX_TEACHERS][MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
//Matrix3D<bool> Timetable::getInstance()->instance.teacherNotAvailableDayHour;

////////END   st. not available, tch not avail., break, activity preferred time,
////////activity preferred time, activities preferred times


//the order of activities changes
///QList<int> teacherActivitiesOfTheDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];
static Matrix1D<QList<int> > subgroupsWithMaxDaysPerWeekForActivities;


//Matrix1D<QList<int> > mustComputeTimetableSubgroups;
//Matrix1D<QList<int> > mustComputeTimetableTeachers;
//bool mustComputeTimetableSubgroup[MAX_TOTAL_SUBGROUPS];
//bool mustComputeTimetableTeacher[MAX_TEACHERS];


static bool haveActivitiesOccupyOrSimultaneousConstraints;

/////////////////////////////////////////////////////////////////////////

static bool fixedTimeActivity[MAX_ACTIVITIES];

//-----------


//BASIC TIME
static Matrix1D<QHash<int, int> > activitiesConflictingPercentage;

//MIN DAYS BETWEEN ACTIVITIES
static Matrix1D<QList<int> > minDaysListOfActivities;
static Matrix1D<QList<int> > minDaysListOfMinDays;
static Matrix1D<QList<double> > minDaysListOfWeightPercentages;
static Matrix1D<QList<bool> > minDaysListOfConsecutiveIfSameDay;

//MAX DAYS BETWEEN ACTIVITIES
static Matrix1D<QList<int> > maxDaysListOfActivities;
static Matrix1D<QList<int> > maxDaysListOfMaxDays;
static Matrix1D<QList<double> > maxDaysListOfWeightPercentages;

//MIN GAPS BETWEEN ACTIVITIES
static Matrix1D<QList<int> > minGapsBetweenActivitiesListOfActivities;
static Matrix1D<QList<int> > minGapsBetweenActivitiesListOfMinGaps;
static Matrix1D<QList<double> > minGapsBetweenActivitiesListOfWeightPercentages;

//TCH & ST NOT AVAIL, BREAK, ACT(S) PREFERRED TIME(S)
static Matrix2D<double> notAllowedTimesPercentages;

//STUDENTS MAX GAPS & EARLY
static int nHoursPerSubgroup[MAX_TOTAL_SUBGROUPS];
static double subgroupsEarlyMaxBeginningsAtSecondHourPercentage[MAX_TOTAL_SUBGROUPS];
static int subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[MAX_TOTAL_SUBGROUPS];
static double subgroupsMaxGapsPerWeekPercentage[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxGapsPerWeekMaxGaps[MAX_TOTAL_SUBGROUPS];

static double subgroupsMaxGapsPerDayPercentage[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxGapsPerDayMaxGaps[MAX_TOTAL_SUBGROUPS];
static bool haveStudentsMaxGapsPerDay;

//STUDENTS MAX DAYS PER WEEK
static int subgroupsMaxDaysPerWeekMaxDays[MAX_TOTAL_SUBGROUPS];
static double subgroupsMaxDaysPerWeekWeightPercentages[MAX_TOTAL_SUBGROUPS];

//TEACHERS MAX DAYS PER WEEK
static int teachersMaxDaysPerWeekMaxDays[MAX_TEACHERS];
static double teachersMaxDaysPerWeekWeightPercentages[MAX_TEACHERS];
static Matrix1D<QList<int> > teachersWithMaxDaysPerWeekForActivities;

/////////////////care for teachers max gaps
static int nHoursPerTeacher[MAX_TEACHERS];
static double teachersMaxGapsPerWeekPercentage[MAX_TEACHERS];
static int teachersMaxGapsPerWeekMaxGaps[MAX_TEACHERS];

static double teachersMaxGapsPerDayPercentage[MAX_TEACHERS];
static int teachersMaxGapsPerDayMaxGaps[MAX_TEACHERS];

//activities same starting time
static Matrix1D<QList<int> > activitiesSameStartingTimeActivities;
static Matrix1D<QList<double> > activitiesSameStartingTimePercentages;

//activities same starting hour
static Matrix1D<QList<int> > activitiesSameStartingHourActivities;
static Matrix1D<QList<double> > activitiesSameStartingHourPercentages;

//activities same starting day
static Matrix1D<QList<int> > activitiesSameStartingDayActivities;
static Matrix1D<QList<double> > activitiesSameStartingDayPercentages;

//activities not overlapping
static Matrix1D<QList<int> > activitiesNotOverlappingActivities;
static Matrix1D<QList<double> > activitiesNotOverlappingPercentages;

//teacher(s) max hours daily
//int teachersGapsPerDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];
//int teachersNHoursPerDay[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

static double teachersMaxHoursDailyPercentages1[MAX_TEACHERS];
static int teachersMaxHoursDailyMaxHours1[MAX_TEACHERS];
//int teachersRealGapsPerDay1[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

static double teachersMaxHoursDailyPercentages2[MAX_TEACHERS];
static int teachersMaxHoursDailyMaxHours2[MAX_TEACHERS];
//int teachersRealGapsPerDay2[MAX_TEACHERS][MAX_DAYS_PER_WEEK];

//teacher(s) max hours continuously
static double teachersMaxHoursContinuouslyPercentages1[MAX_TEACHERS];
static int teachersMaxHoursContinuouslyMaxHours1[MAX_TEACHERS];

static double teachersMaxHoursContinuouslyPercentages2[MAX_TEACHERS];
static int teachersMaxHoursContinuouslyMaxHours2[MAX_TEACHERS];

//teacher(s) activity tag max hours daily
static bool haveTeachersActivityTagMaxHoursDaily;

static Matrix1D<QList<int> > teachersActivityTagMaxHoursDailyMaxHours;
static Matrix1D<QList<int> > teachersActivityTagMaxHoursDailyActivityTag;
static Matrix1D<QList<double> > teachersActivityTagMaxHoursDailyPercentage;
//(formerly arrays of size MAX_TEACHERS)

//teacher(s) activity tag max hours continuously
static bool haveTeachersActivityTagMaxHoursContinuously;

static Matrix1D<QList<int> > teachersActivityTagMaxHoursContinuouslyMaxHours;
static Matrix1D<QList<int> > teachersActivityTagMaxHoursContinuouslyActivityTag;
static Matrix1D<QList<double> > teachersActivityTagMaxHoursContinuouslyPercentage;
//(formerly arrays of size MAX_TEACHERS)

//teacher(s) min hours daily
static double teachersMinHoursDailyPercentages[MAX_TEACHERS];
static int teachersMinHoursDailyMinHours[MAX_TEACHERS];

//teacher(s) min days per week
static double teachersMinDaysPerWeekPercentages[MAX_TEACHERS];
static int teachersMinDaysPerWeekMinDays[MAX_TEACHERS];

//students (set) max hours continuously
static double subgroupsMaxHoursContinuouslyPercentages1[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxHoursContinuouslyMaxHours1[MAX_TOTAL_SUBGROUPS];

static double subgroupsMaxHoursContinuouslyPercentages2[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxHoursContinuouslyMaxHours2[MAX_TOTAL_SUBGROUPS];

//students (set) activity tag max hours daily
static bool haveStudentsActivityTagMaxHoursDaily;

static Matrix1D<QList<int> > subgroupsActivityTagMaxHoursDailyMaxHours;
static Matrix1D<QList<int> > subgroupsActivityTagMaxHoursDailyActivityTag;
static Matrix1D<QList<double> > subgroupsActivityTagMaxHoursDailyPercentage;
//(formerly arrays of size MAX_TOTAL_SUBGROUPS)

//students (set) activity tag max hours continuously
static bool haveStudentsActivityTagMaxHoursContinuously;

static Matrix1D<QList<int> > subgroupsActivityTagMaxHoursContinuouslyMaxHours;
static Matrix1D<QList<int> > subgroupsActivityTagMaxHoursContinuouslyActivityTag;
static Matrix1D<QList<double> > subgroupsActivityTagMaxHoursContinuouslyPercentage;
//(formerly arrays of size MAX_TOTAL_SUBGROUPS)

//students (set) max hours daily
static double subgroupsMaxHoursDailyPercentages1[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxHoursDailyMaxHours1[MAX_TOTAL_SUBGROUPS];

static double subgroupsMaxHoursDailyPercentages2[MAX_TOTAL_SUBGROUPS];
static int subgroupsMaxHoursDailyMaxHours2[MAX_TOTAL_SUBGROUPS];

//students (set) min hours daily
static double subgroupsMinHoursDailyPercentages[MAX_TOTAL_SUBGROUPS];
static int subgroupsMinHoursDailyMinHours[MAX_TOTAL_SUBGROUPS];
static bool subgroupsMinHoursDailyAllowEmptyDays[MAX_TOTAL_SUBGROUPS];
static bool haveStudentsMinHoursDailyAllowEmptyDays;

// 2 activities consecutive
//index represents the first activity, value in array represents the second activity
static Matrix1D<QList<double> > constrTwoActivitiesConsecutivePercentages;
static Matrix1D<QList<int> > constrTwoActivitiesConsecutiveActivities;

//index represents the second activity, value in array represents the first activity
static Matrix1D<QList<double> > inverseConstrTwoActivitiesConsecutivePercentages;
static Matrix1D<QList<int> > inverseConstrTwoActivitiesConsecutiveActivities;
// 2 activities consecutive

// 2 activities grouped
//index represents the first activity, value in array represents the second activity
static Matrix1D<QList<double> > constrTwoActivitiesGroupedPercentages;
static Matrix1D<QList<int> > constrTwoActivitiesGroupedActivities;

// 3 activities grouped
//index represents the first activity, value in array represents the second activity
static Matrix1D<QList<double> > constrThreeActivitiesGroupedPercentages;
static Matrix1D<QList<QPair<int, int> > > constrThreeActivitiesGroupedActivities;

// 2 activities ordered
//index represents the first activity, value in array represents the second activity
static Matrix1D<QList<double> > constrTwoActivitiesOrderedPercentages;
static Matrix1D<QList<int> > constrTwoActivitiesOrderedActivities;

//index represents the second activity, value in array represents the first activity
static Matrix1D<QList<double> > inverseConstrTwoActivitiesOrderedPercentages;
static Matrix1D<QList<int> > inverseConstrTwoActivitiesOrderedActivities;
// 2 activities consecutive

static double activityEndsStudentsDayPercentages[MAX_ACTIVITIES];
static bool haveActivityEndsStudentsDay;


///////BEGIN teachers interval max days per week
static double teachersIntervalMaxDaysPerWeekPercentages1[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekMaxDays1[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalStart1[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalEnd1[MAX_TEACHERS];

static double teachersIntervalMaxDaysPerWeekPercentages2[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekMaxDays2[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalStart2[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalEnd2[MAX_TEACHERS];

static double teachersIntervalMaxDaysPerWeekPercentages3[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekMaxDays3[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalStart3[MAX_TEACHERS];
static int teachersIntervalMaxDaysPerWeekIntervalEnd3[MAX_TEACHERS];

//bool computeTeachersIntervalMaxDaysPerWeek();
///////END   teachers interval max days per week


///////BEGIN students interval max days per week
static double subgroupsIntervalMaxDaysPerWeekPercentages1[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekMaxDays1[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalStart1[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalEnd1[MAX_TOTAL_SUBGROUPS];

static double subgroupsIntervalMaxDaysPerWeekPercentages2[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekMaxDays2[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalStart2[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalEnd2[MAX_TOTAL_SUBGROUPS];

static double subgroupsIntervalMaxDaysPerWeekPercentages3[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekMaxDays3[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalStart3[MAX_TOTAL_SUBGROUPS];
static int subgroupsIntervalMaxDaysPerWeekIntervalEnd3[MAX_TOTAL_SUBGROUPS];

//bool computeSubgroupsIntervalMaxDaysPerWeek();
///////END   subgroups interval max days per week

//2017-02-06
static int teachersMaxSpanPerDayMaxSpan[MAX_TEACHERS]; //-1 for not existing
static double teachersMaxSpanPerDayPercentages[MAX_TEACHERS]; //-1 for not existing

static int teachersMinRestingHoursCircularMinHours[MAX_TEACHERS]; //-1 for not existing
static double teachersMinRestingHoursCircularPercentages[MAX_TEACHERS]; //-1 for not existing
static int teachersMinRestingHoursNotCircularMinHours[MAX_TEACHERS]; //-1 for not existing
static double teachersMinRestingHoursNotCircularPercentages[MAX_TEACHERS]; //-1 for not existing
//bool teachersMinRestingHoursCircular[MAX_TEACHERS]; //true for circular

static int subgroupsMaxSpanPerDayMaxSpan[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
static double subgroupsMaxSpanPerDayPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing

static int subgroupsMinRestingHoursCircularMinHours[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
static double subgroupsMinRestingHoursCircularPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
static int subgroupsMinRestingHoursNotCircularMinHours[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
static double subgroupsMinRestingHoursNotCircularPercentages[MAX_TOTAL_SUBGROUPS]; //-1 for not existing
//bool subgroupsMinRestingHoursCircular[MAX_TOTAL_SUBGROUPS]; //true for circular
////////////

//void computeMustComputeTimetableSubgroups();
//void computeMustComputeTimetableTeachers();

//2011-09-25 - Constraint activities occupy max time slots from selection
static QList<ActivitiesOccupyMaxTimeSlotsFromSelection_item> aomtsList;
static Matrix1D<QList<ActivitiesOccupyMaxTimeSlotsFromSelection_item*> > aomtsListForActivity;

//2011-09-30 - Constraint activities max simultaneous in selected time slots
static QList<ActivitiesMaxSimultaneousInSelectedTimeSlots_item> amsistsList;
static Matrix1D<QList<ActivitiesMaxSimultaneousInSelectedTimeSlots_item*> > amsistsListForActivity;

static int daysTeacherIsAvailable[MAX_TEACHERS];
static int daysSubgroupIsAvailable[MAX_TOTAL_SUBGROUPS];
static int requestedDaysForTeachers[MAX_TEACHERS];
static int requestedDaysForSubgroups[MAX_TOTAL_SUBGROUPS];
static int nReqForSubgroup[MAX_TOTAL_SUBGROUPS];
/////////////////////////////

/////////used only in sortActivities
static int nIncompatible[MAX_ACTIVITIES];
static double nMinDaysConstraintsBroken[MAX_ACTIVITIES];
////////////////////////////////////

static int nIncompatibleFromFather[MAX_ACTIVITIES];
////////////////////////////////////

inline bool findEquivalentSubgroupsCompareFunction(int i1, int i2)
{
    const QList<int>& a1=TContext::get()->instance.directSubgroupsList[i1]->activitiesForSubgroup;
    const QList<int>& a2=TContext::get()->instance.directSubgroupsList[i2]->activitiesForSubgroup;

    if(a1.count()<a2.count()){
        return true;
    }
    else if(a1.count()>a2.count()){
        return false;
    }
    else{
        assert(a1.count()==a2.count());
        for(int i=0; i<a1.count(); i++){
            if(a1.at(i)<a2.at(i)){
                return true;
            }
            else if(a1.at(i)>a2.at(i)){
                return false;
            }
        }
    }
    return false;
}

////////////////////////////////////

inline bool compareFunctionGeneratePre(int i, int j)
{
    if(nIncompatible[i]>nIncompatible[j] || (nIncompatible[i]==nIncompatible[j] && nMinDaysConstraintsBroken[i]>nMinDaysConstraintsBroken[j]))
        return true;

    return false;
}

inline bool compareFunctionGeneratePreWithGroupedActivities(int i, int j)
{
    //nMinDaysBroken is different from 0.0 only if the activity is fixed
    if( nIncompatibleFromFather[i]>nIncompatibleFromFather[j]
     || (nIncompatibleFromFather[i]==nIncompatibleFromFather[j] && nMinDaysConstraintsBroken[i]>nMinDaysConstraintsBroken[j])
     || (nIncompatibleFromFather[i]==nIncompatibleFromFather[j] && IS_EQUAL(nMinDaysConstraintsBroken[i], nMinDaysConstraintsBroken[j]) && nIncompatible[i]>nIncompatible[j] )
     )
        return true;

    return false;
}


bool processConstraints(QWidget* parent)
{
    assert(TContext::get()->instance.internalStructureComputed);

    ////////////Compute equivalent subgroups
    if(defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES){
        QList<int> tmpSortedSubgroupsList;
        Matrix1D<bool> isSignificantSubgroup;

        isSignificantSubgroup.resize(TContext::get()->instance.directSubgroupsList.size());

        tmpSortedSubgroupsList.clear();
        for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
            tmpSortedSubgroupsList.append(i);

        std::stable_sort(tmpSortedSubgroupsList.begin(), tmpSortedSubgroupsList.end(), findEquivalentSubgroupsCompareFunction);

        QStringList s;

        if(TContext::get()->instance.directSubgroupsList.size()>0)
            isSignificantSubgroup[tmpSortedSubgroupsList.at(0)]=true;
        for(int i=1; i<TContext::get()->instance.directSubgroupsList.size(); i++){
            int s1=tmpSortedSubgroupsList.at(i-1);
            int s2=tmpSortedSubgroupsList.at(i);

            isSignificantSubgroup[s2]=true;

            const QList<int>& l1=TContext::get()->instance.directSubgroupsList[s1]->activitiesForSubgroup;
            const QList<int>& l2=TContext::get()->instance.directSubgroupsList[s2]->activitiesForSubgroup;
            if(l1.count()==l2.count()){
                int i;
                for(i=0; i<l1.count(); i++)
                    if(l1.at(i)!=l2.at(i))
                        break;
                if(i==l1.count() and l1.count() > 0){
                    isSignificantSubgroup[s2]=false;

                    s.append(GeneratePreTranslate::tr("Subgroup %1 has the same activities as subgroup %2.").arg(TContext::get()->instance.directSubgroupsList[s2]->name).arg(TContext::get()->instance.directSubgroupsList[s1]->name));
                }
            }
        }

        int cnt=0;
        for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
            if(!isSignificantSubgroup[i])
                cnt++;

        if(cnt>0){
            QString s0=GeneratePreTranslate::tr("Optimization tip:");
            s0+=" ";
            s0+=GeneratePreTranslate::tr("There are %1 subgroups (from the total of %2 subgroups) which have the same activities as other subgroups. They are listed below."
                " If the constraints relating to these subgroups are also the same, you can make the generation (directly proportional) faster by completely removing the subgroups"
                " which are equivalent to other subgroups (leaving only one representant for each equivalence set). (The generation algorithm will not completely remove the equivalent"
                " subgroups automatically.)").arg(cnt).arg(TContext::get()->instance.directSubgroupsList.size());
            s0+="\n\n";
            s0+=GeneratePreTranslate::tr("If you did not add all the activities yet or if the number of equivalent subgroups compared to the total number of subgroups"
                " is small, probably you can safely ignore this message.");
            s0+="\n\n";
            s0+=GeneratePreTranslate::tr("You can deactivate this message from the 'Settings' menu.");

            MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET warning"), s0+QString("\n\n")+s.join("\n"));
        }
    }
    //////////////////////////////////

    //////////////////begin resizing

    //MIN DAYS BETWEEN ACTIVITIES
    minDaysListOfActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    minDaysListOfMinDays.resize(TContext::get()->instance.activeActivitiesList.size());
    minDaysListOfWeightPercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    minDaysListOfConsecutiveIfSameDay.resize(TContext::get()->instance.activeActivitiesList.size());

    //MAX DAYS BETWEEN ACTIVITIES
    maxDaysListOfActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    maxDaysListOfMaxDays.resize(TContext::get()->instance.activeActivitiesList.size());
    maxDaysListOfWeightPercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    //MIN GAPS BETWEEN ACTIVITIES
    minGapsBetweenActivitiesListOfActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    minGapsBetweenActivitiesListOfMinGaps.resize(TContext::get()->instance.activeActivitiesList.size());
    minGapsBetweenActivitiesListOfWeightPercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    teachersWithMaxDaysPerWeekForActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    subgroupsWithMaxDaysPerWeekForActivities.resize(TContext::get()->instance.activeActivitiesList.size());

    //activities same starting time
    activitiesSameStartingTimeActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    activitiesSameStartingTimePercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    //activities same starting hour
    activitiesSameStartingHourActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    activitiesSameStartingHourPercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    //activities same starting day
    activitiesSameStartingDayActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    activitiesSameStartingDayPercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    //activities not overlapping
    activitiesNotOverlappingActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    activitiesNotOverlappingPercentages.resize(TContext::get()->instance.activeActivitiesList.size());

    // 2 activities consecutive
    //index represents the first activity, value in array represents the second activity
    constrTwoActivitiesConsecutivePercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    constrTwoActivitiesConsecutiveActivities.resize(TContext::get()->instance.activeActivitiesList.size());

    //index represents the second activity, value in array represents the first activity
    inverseConstrTwoActivitiesConsecutivePercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    inverseConstrTwoActivitiesConsecutiveActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    // 2 activities consecutive

    // 2 activities grouped
    //index represents the first activity, value in array represents the second activity
    constrTwoActivitiesGroupedPercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    constrTwoActivitiesGroupedActivities.resize(TContext::get()->instance.activeActivitiesList.size());

    // 3 activities grouped
    //index represents the first activity, value in array represents the second activity
    constrThreeActivitiesGroupedPercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    constrThreeActivitiesGroupedActivities.resize(TContext::get()->instance.activeActivitiesList.size());

    // 2 activities ordered
    //index represents the first activity, value in array represents the second activity
    constrTwoActivitiesOrderedPercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    constrTwoActivitiesOrderedActivities.resize(TContext::get()->instance.activeActivitiesList.size());

    //index represents the second activity, value in array represents the first activity
    inverseConstrTwoActivitiesOrderedPercentages.resize(TContext::get()->instance.activeActivitiesList.size());
    inverseConstrTwoActivitiesOrderedActivities.resize(TContext::get()->instance.activeActivitiesList.size());
    // 2 activities consecutive

//    mustComputeTimetableSubgroups.resize(Timetable::getInstance()->instance.activeActivitiesList.size());
//    mustComputeTimetableTeachers.resize(Timetable::getInstance()->instance.activeActivitiesList.size());

    //////teachers and subgroups activity tag max hours daily and continuously
    teachersActivityTagMaxHoursDailyMaxHours.resize(TContext::get()->instance.teachersList.size());
    teachersActivityTagMaxHoursDailyActivityTag.resize(TContext::get()->instance.teachersList.size());
    teachersActivityTagMaxHoursDailyPercentage.resize(TContext::get()->instance.teachersList.size());

    teachersActivityTagMaxHoursContinuouslyMaxHours.resize(TContext::get()->instance.teachersList.size());
    teachersActivityTagMaxHoursContinuouslyActivityTag.resize(TContext::get()->instance.teachersList.size());
    teachersActivityTagMaxHoursContinuouslyPercentage.resize(TContext::get()->instance.teachersList.size());

    subgroupsActivityTagMaxHoursDailyMaxHours.resize(TContext::get()->instance.directSubgroupsList.size());
    subgroupsActivityTagMaxHoursDailyActivityTag.resize(TContext::get()->instance.directSubgroupsList.size());
    subgroupsActivityTagMaxHoursDailyPercentage.resize(TContext::get()->instance.directSubgroupsList.size());

    subgroupsActivityTagMaxHoursContinuouslyMaxHours.resize(TContext::get()->instance.directSubgroupsList.size());
    subgroupsActivityTagMaxHoursContinuouslyActivityTag.resize(TContext::get()->instance.directSubgroupsList.size());
    subgroupsActivityTagMaxHoursContinuouslyPercentage.resize(TContext::get()->instance.directSubgroupsList.size());

    //2011-09-25
    aomtsListForActivity.resize(TContext::get()->instance.activeActivitiesList.size());
    //2011-09-30
    amsistsListForActivity.resize(TContext::get()->instance.activeActivitiesList.size());

    //////////////////end resizing - new feature

    QHash<int, int> reprSameStartingTime;
    QHash<int, QSet<int> > reprSameActivitiesSet;

    /////1. BASIC TIME CONSTRAINTS
    bool t=computeActivitiesConflictingPercentage(parent);
    if(!t)
        return false;
    //////////////////////////////

    /////2. min days between activities
    t=computeMinDays(parent);
    if(!t)
        return false;
    /////////////////////////////////////

    /////2.3. max days between activities
    t=computeMaxDays(parent);
    if(!t)
        return false;
    /////////////////////////////////////

    /////2.5. min gaps between activities
    t=computeMinGapsBetweenActivities(parent);
    if(!t)
        return false;
    /////////////////////////////////////

    /////3. students not available, teachers not available, break, activity preferred time,
    /////   activity preferred times, activities preferred times
    t=computeNotAllowedTimesPercentages(parent);
    if(!t)
        return false;
    ///////////////////////////////////////////////////////////////

    /////3.5. STUDENTS MAX DAYS PER WEEK
    t=computeMaxDaysPerWeekForStudents(parent);
    if(!t)
        return false;
    //////////////////////////////////

    /////4. students max gaps and early
    t=computeNHoursPerSubgroup(parent);
    if(!t)
        return false;
    t=computeSubgroupsEarlyAndMaxGapsPercentages(parent);
    if(!t)
        return false;
    t=computeSubgroupsMaxGapsPerDayPercentages(parent); //!!!after max gaps per week
    if(!t)
        return false;

    //////////////////////////////////

    /////5. TEACHERS MAX DAYS PER WEEK
    t=computeMaxDaysPerWeekForTeachers(parent);
    if(!t)
        return false;
    //////////////////////////////////


    /////6. TEACHERS MAX GAPS PER WEEK/DAY
    t=computeNHoursPerTeacher(parent);
    if(!t)
        return false;
    t=computeTeachersMaxGapsPerWeekPercentage(parent);
    if(!t)
        return false;
    t=computeTeachersMaxGapsPerDayPercentage(parent);
    if(!t)
        return false;
    //////////////////////////////////

    //must be AFTER basic time constraints (computeActivitiesConflictingPercentage)
    t=computeActivitiesSameStartingTime(parent, reprSameStartingTime, reprSameActivitiesSet);
    if(!t)
        return false;

    computeActivitiesSameStartingHour();

    computeActivitiesSameStartingDay();

    computeActivitiesNotOverlapping();

    //must be after allowed times, after n hours per teacher and after max days per week for teachers
    t=computeTeachersMaxHoursDaily(parent);
    if(!t)
        return false;

    t=computeTeachersMaxHoursContinuously(parent);
    if(!t)
        return false;

//	t=computeTeachersActivityTagMaxHoursDaily(parent);
//	if(!t)
//		return false;

//	t=computeTeachersActivityTagMaxHoursContinuously(parent);
//	if(!t)
//		return false;

    //must be after n hours per teacher
    t=computeTeachersMinHoursDaily(parent);
    if(!t)
        return false;

    //must be after teachers min hours daily
    t=computeTeachersMinDaysPerWeek(parent);
    if(!t)
        return false;

    //must be after allowed times, after n hours per subgroup and after max days per week for subgroups
    t=computeSubgroupsMaxHoursDaily(parent);
    if(!t)
        return false;

    t=computeStudentsMaxHoursContinuously(parent);
    if(!t)
        return false;

//	t=computeStudentsActivityTagMaxHoursDaily(parent);
//	if(!t)
//		return false;

//	t=computeStudentsActivityTagMaxHoursContinuously(parent);
//	if(!t)
//		return false;

    //after max hours daily/continuously without/with an activity tag
    t=checkMaxHoursForActivityDuration(parent);
    if(!t)
        return false;

    t=computeSubgroupsMinHoursDaily(parent);
    if(!t)
        return false;

    computeConstrTwoActivitiesConsecutive();

    computeConstrTwoActivitiesGrouped();

    computeConstrThreeActivitiesGrouped();

    computeConstrTwoActivitiesOrdered();

    t=computeActivityEndsStudentsDayPercentages(parent);
    if(!t)
        return false;

    //check for impossible min days
    t=checkMinDays100Percent(parent);
    if(!t)
        return false;
    t=checkMinDaysConsecutiveIfSameDay(parent);
    if(!t)
        return false;

    //check teachers interval max days per week
    t=computeTeachersIntervalMaxDaysPerWeek(parent);
    if(!t)
        return false;

    //check subgroups interval max days per week
    t=computeSubgroupsIntervalMaxDaysPerWeek(parent);
    if(!t)
        return false;

    //2017-02-06
    t=computeTeachersMaxSpanPerDay(parent);
    if(!t)
        return false;

    t=computeSubgroupsMaxSpanPerDay(parent);
    if(!t)
        return false;

    t=computeTeachersMinRestingHours(parent);
    if(!t)
        return false;

    t=computeSubgroupsMinRestingHours(parent);
    if(!t)
        return false;

    ////////////////
    haveActivitiesOccupyOrSimultaneousConstraints=false;

    //2011-09-25
    t=computeActivitiesOccupyMaxTimeSlotsFromSelection(parent);
    if(!t)
        return false;

    //2011-09-30
    t=computeActivitiesMaxSimultaneousInSelectedTimeSlots(parent);
    if(!t)
        return false;
    ////////////////

//    computeMustComputeTimetableSubgroups();
//    computeMustComputeTimetableTeachers();

    t=computeFixedActivities(parent);
    if(!t)
        return false;

    //must have here repr computed correctly
//	sortActivities(parent, reprSameStartingTime, reprSameActivitiesSet, initialOrderStream);

    if(defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS){
        if(haveStudentsMaxGapsPerDay || haveTeachersActivityTagMaxHoursDaily || haveStudentsActivityTagMaxHoursDaily){
            QString s=GeneratePreTranslate::tr("Your data contains constraints students max gaps per day and/or activity tag max hours daily.");
            s+="\n\n";
            s+=GeneratePreTranslate::tr("These constraints are good, but they are not perfectly optimized for speed. You may obtain a long generation time or even impossible timetables.");
            s+=" ";
            s+=GeneratePreTranslate::tr("It is recommended to use such constraints with caution.");
            s+="\n\n";
            s+=GeneratePreTranslate::tr("Are you sure you want to continue?");

#ifdef FET_COMMAND_LINE
            int b=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s, GeneratePreTranslate::tr("Yes"), GeneratePreTranslate::tr("No"), QString(), 0, 1);
            if(b!=0)
                return false;
#else
            QMessageBox::StandardButton b=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"), s, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if(b!=QMessageBox::Yes)
                return false;
#endif
        }
    }

    if(defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS){
        if(haveStudentsMinHoursDailyAllowEmptyDays){
            QString s=GeneratePreTranslate::tr("Your data contains constraints students min hours daily which allow empty days.");
            s+="\n\n";
            s+=GeneratePreTranslate::tr("These constraints are nonstandard. They are recommended only if the students can have free days and a solution with free days for students exists."
                " Otherwise the solution might be impossible for m-FET to find.");
            s+=" ";
            s+=GeneratePreTranslate::tr("It is recommended to use such constraints with caution.");
            s+="\n\n";
            s+=GeneratePreTranslate::tr("Are you sure you want to continue?");

#ifdef FET_COMMAND_LINE
            int b=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s, GeneratePreTranslate::tr("Yes"), GeneratePreTranslate::tr("No"), QString(), 0, 1);
            if(b!=0)
                return false;
#else
            QMessageBox::StandardButton b=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"), s, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if(b!=QMessageBox::Yes)
                return false;
#endif
        }
    }

    bool ok=true;
    return ok;
}

//must be after allowed times, after n hours per subgroup and after max days per week for subgroups
bool computeSubgroupsMaxHoursDaily(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMaxHoursDailyMaxHours1[i]=-1;
        subgroupsMaxHoursDailyPercentages1[i]=-1;

        subgroupsMaxHoursDailyMaxHours2[i]=-1;
        subgroupsMaxHoursDailyPercentages2[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_HOURS_DAILY){
            ConstraintStudentsMaxHoursDaily* smd=(ConstraintStudentsMaxHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
                if(subgroupsMaxHoursDailyMaxHours1[sb]==-1 ||
                 (subgroupsMaxHoursDailyMaxHours1[sb] >= smd->maxHoursDaily &&
                 subgroupsMaxHoursDailyPercentages1[sb] <= smd->weightPercentage)){
                    subgroupsMaxHoursDailyMaxHours1[sb] = smd->maxHoursDaily;
                    subgroupsMaxHoursDailyPercentages1[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMaxHoursDailyMaxHours1[sb] <= smd->maxHoursDaily &&
                 subgroupsMaxHoursDailyPercentages1[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{
                    if(subgroupsMaxHoursDailyMaxHours2[sb]==-1 ||
                     (subgroupsMaxHoursDailyMaxHours2[sb] >= smd->maxHoursDaily &&
                     subgroupsMaxHoursDailyPercentages2[sb] <= smd->weightPercentage)){
                        subgroupsMaxHoursDailyMaxHours2[sb] = smd->maxHoursDaily;
                        subgroupsMaxHoursDailyPercentages2[sb] = smd->weightPercentage;
                        }
                    else if(subgroupsMaxHoursDailyMaxHours2[sb] <= smd->maxHoursDaily &&
                     subgroupsMaxHoursDailyPercentages2[sb] >= smd->weightPercentage){
                        //nothing
                    }
                    else{
                         //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                         " of type max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                         " be maximum 2 constraints of type max hours daily.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY){
            ConstraintStudentsSetMaxHoursDaily* smd=(ConstraintStudentsSetMaxHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int q=0; q<smd->iSubgroupsList.count(); q++){
                int sb=smd->iSubgroupsList.at(q);
            //for(int sb=0; sb<Timetable::getInstance()->instance.directSubgroupsList.size(); sb++){
                if(subgroupsMaxHoursDailyMaxHours1[sb]==-1 ||
                 (subgroupsMaxHoursDailyMaxHours1[sb] >= smd->maxHoursDaily &&
                 subgroupsMaxHoursDailyPercentages1[sb] <= smd->weightPercentage)){
                    subgroupsMaxHoursDailyMaxHours1[sb] = smd->maxHoursDaily;
                    subgroupsMaxHoursDailyPercentages1[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMaxHoursDailyMaxHours1[sb] <= smd->maxHoursDaily &&
                 subgroupsMaxHoursDailyPercentages1[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{
                    if(subgroupsMaxHoursDailyMaxHours2[sb]==-1 ||
                     (subgroupsMaxHoursDailyMaxHours2[sb] >= smd->maxHoursDaily &&
                     subgroupsMaxHoursDailyPercentages2[sb] <= smd->weightPercentage)){
                        subgroupsMaxHoursDailyMaxHours2[sb] = smd->maxHoursDaily;
                        subgroupsMaxHoursDailyPercentages2[sb] = smd->weightPercentage;
                        }
                    else if(subgroupsMaxHoursDailyMaxHours2[sb] <= smd->maxHoursDaily &&
                     subgroupsMaxHoursDailyPercentages2[sb] >= smd->weightPercentage){
                        //nothing
                    }
                    else{
                        //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                         " of type max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                         " be maximum 2 constraints of type max hours daily.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
    }

    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        if(IS_EQUAL(subgroupsMaxHoursDailyPercentages1[sb], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],subgroupsMaxHoursDailyMaxHours1[sb]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(subgroupsMaxDaysPerWeekMaxDays[sb]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<subgroupsMaxDaysPerWeekMaxDays[sb]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerSubgroup[sb]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint of type"
                 " max %2 hours daily with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, students (set) max days per week, students set not available and/or breaks."
                 " The number of total hours for this subgroup is"
                 " %3 and the number of available slots is, considering max hours daily and all other constraints, %4.")
                 .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
                 .arg(subgroupsMaxHoursDailyMaxHours1[sb])
                 .arg(nHoursPerSubgroup[sb])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        if(IS_EQUAL(subgroupsMaxHoursDailyPercentages2[sb], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],subgroupsMaxHoursDailyMaxHours2[sb]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(subgroupsMaxDaysPerWeekMaxDays[sb]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<subgroupsMaxDaysPerWeekMaxDays[sb]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerSubgroup[sb]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint of type"
                 " max %2 hours daily with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, students (set) max days per week, students set not available and/or breaks."
                 " The number of total hours for this subgroup is"
                 " %3 and the number of available slots is, considering max hours daily and all other constraints, %4.")
                 .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
                 .arg(subgroupsMaxHoursDailyMaxHours2[sb])
                 .arg(nHoursPerSubgroup[sb])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

//must be after allowed times, after n hours per subgroup and after max days per week for subgroups
bool computeSubgroupsMaxSpanPerDay(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMaxSpanPerDayMaxSpan[i]=-1;
        subgroupsMaxSpanPerDayPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY){
            ConstraintStudentsSetMaxSpanPerDay* ssmsd=(ConstraintStudentsSetMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(ssmsd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set max span per day for students %1"
                 " with weight (percentage) below 100. Please make weight 100% and try again").arg(ssmsd->students),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_SPAN_PER_DAY){
            ConstraintStudentsMaxSpanPerDay* smsd=(ConstraintStudentsMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(smsd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students max span per day"
                 " with weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY){
            ConstraintStudentsSetMaxSpanPerDay* ssmsd=(ConstraintStudentsSetMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            foreach(int sbg, ssmsd->iSubgroupsList)
                if(subgroupsMaxSpanPerDayPercentages[sbg] < 0
                 || (subgroupsMaxSpanPerDayPercentages[sbg]>=0 && subgroupsMaxSpanPerDayMaxSpan[sbg]>ssmsd->maxSpanPerDay)){
                    subgroupsMaxSpanPerDayPercentages[sbg]=100.0;
                    subgroupsMaxSpanPerDayMaxSpan[sbg]=ssmsd->maxSpanPerDay;
                }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_SPAN_PER_DAY){
            ConstraintStudentsMaxSpanPerDay* smsd=(ConstraintStudentsMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int sbg=0; sbg<TContext::get()->instance.directSubgroupsList.size(); sbg++){
                if(subgroupsMaxSpanPerDayPercentages[sbg] < 0
                 || (subgroupsMaxSpanPerDayPercentages[sbg]>=0 && subgroupsMaxSpanPerDayMaxSpan[sbg]>smsd->maxSpanPerDay)){
                    subgroupsMaxSpanPerDayPercentages[sbg]=100.0;
                    subgroupsMaxSpanPerDayMaxSpan[sbg]=smsd->maxSpanPerDay;
                }
            }
        }
    }

    //This is similar to subgroups max hours daily checking. It is not a very useful test, but does not hurt.
    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        if(IS_EQUAL(subgroupsMaxSpanPerDayPercentages[sb], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],subgroupsMaxSpanPerDayMaxSpan[sb]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(subgroupsMaxDaysPerWeekMaxDays[sb]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<subgroupsMaxDaysPerWeekMaxDays[sb]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerSubgroup[sb]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint of type"
                 " max %2 span per day with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, students (set) max days per week, students set not available and/or breaks."
                 " The number of total hours for this subgroup is"
                 " %3 and the number of available slots is, considering max span per day and all other constraints, %4.")
                 .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
                 .arg(subgroupsMaxSpanPerDayMaxSpan[sb])
                 .arg(nHoursPerSubgroup[sb])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

bool computeStudentsMaxHoursContinuously(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMaxHoursContinuouslyMaxHours1[i]=-1;
        subgroupsMaxHoursContinuouslyPercentages1[i]=-1;

        subgroupsMaxHoursContinuouslyMaxHours2[i]=-1;
        subgroupsMaxHoursContinuouslyPercentages2[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsMaxHoursContinuously* smd=(ConstraintStudentsMaxHoursContinuously*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
                if(subgroupsMaxHoursContinuouslyMaxHours1[sb]==-1 ||
                 (subgroupsMaxHoursContinuouslyMaxHours1[sb] >= smd->maxHoursContinuously &&
                 subgroupsMaxHoursContinuouslyPercentages1[sb] <= smd->weightPercentage)){
                    subgroupsMaxHoursContinuouslyMaxHours1[sb] = smd->maxHoursContinuously;
                    subgroupsMaxHoursContinuouslyPercentages1[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMaxHoursContinuouslyMaxHours1[sb] <= smd->maxHoursContinuously &&
                 subgroupsMaxHoursContinuouslyPercentages1[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{
                    if(subgroupsMaxHoursContinuouslyMaxHours2[sb]==-1 ||
                     (subgroupsMaxHoursContinuouslyMaxHours2[sb] >= smd->maxHoursContinuously &&
                     subgroupsMaxHoursContinuouslyPercentages2[sb] <= smd->weightPercentage)){
                        subgroupsMaxHoursContinuouslyMaxHours2[sb] = smd->maxHoursContinuously;
                        subgroupsMaxHoursContinuouslyPercentages2[sb] = smd->weightPercentage;
                        }
                    else if(subgroupsMaxHoursContinuouslyMaxHours2[sb] <= smd->maxHoursContinuously &&
                     subgroupsMaxHoursContinuouslyPercentages2[sb] >= smd->weightPercentage){
                        //nothing
                    }
                    else{
                         //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                         " of type max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                         " be maximum 2 constraints of type max hours continuously.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY){
            ConstraintStudentsSetMaxHoursContinuously* smd=(ConstraintStudentsSetMaxHoursContinuously*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int q=0; q<smd->iSubgroupsList.count(); q++){
                int sb=smd->iSubgroupsList.at(q);
            //for(int sb=0; sb<Timetable::getInstance()->instance.directSubgroupsList.size(); sb++){
                if(subgroupsMaxHoursContinuouslyMaxHours1[sb]==-1 ||
                 (subgroupsMaxHoursContinuouslyMaxHours1[sb] >= smd->maxHoursContinuously &&
                 subgroupsMaxHoursContinuouslyPercentages1[sb] <= smd->weightPercentage)){
                    subgroupsMaxHoursContinuouslyMaxHours1[sb] = smd->maxHoursContinuously;
                    subgroupsMaxHoursContinuouslyPercentages1[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMaxHoursContinuouslyMaxHours1[sb] <= smd->maxHoursContinuously &&
                 subgroupsMaxHoursContinuouslyPercentages1[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{
                    if(subgroupsMaxHoursContinuouslyMaxHours2[sb]==-1 ||
                     (subgroupsMaxHoursContinuouslyMaxHours2[sb] >= smd->maxHoursContinuously &&
                     subgroupsMaxHoursContinuouslyPercentages2[sb] <= smd->weightPercentage)){
                        subgroupsMaxHoursContinuouslyMaxHours2[sb] = smd->maxHoursContinuously;
                        subgroupsMaxHoursContinuouslyPercentages2[sb] = smd->weightPercentage;
                        }
                    else if(subgroupsMaxHoursContinuouslyMaxHours2[sb] <= smd->maxHoursContinuously &&
                     subgroupsMaxHoursContinuouslyPercentages2[sb] >= smd->weightPercentage){
                        //nothing
                    }
                    else{
                        //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                         " of type max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                         " be maximum 2 constraints of type max hours continuously.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
    }

    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
        foreach(int sbg, TContext::get()->instance.activeActivitiesList[ai]->iSubgroupsList){
            if(subgroupsMaxHoursContinuouslyPercentages1[sbg]>=0 && TContext::get()->instance.activeActivitiesList[ai]->duration > subgroupsMaxHoursContinuouslyMaxHours1[sbg]){
                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint of type"
                 " max %2 hours continuously which cannot be respected because of activity with id %3 (which has duration %4).")
                 .arg(TContext::get()->instance.directSubgroupsList[sbg]->name)
                 .arg(subgroupsMaxHoursContinuouslyMaxHours1[sbg])
                 .arg(TContext::get()->instance.activeActivitiesList[ai]->id)
                 .arg(TContext::get()->instance.activeActivitiesList[ai]->duration);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            if(subgroupsMaxHoursContinuouslyPercentages2[sbg]>=0 && TContext::get()->instance.activeActivitiesList[ai]->duration > subgroupsMaxHoursContinuouslyMaxHours2[sbg]){
                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint of type"
                 " max %2 hours continuously which cannot be respected because of activity with id %3 (which has duration %4).")
                 .arg(TContext::get()->instance.directSubgroupsList[sbg]->name)
                 .arg(subgroupsMaxHoursContinuouslyMaxHours2[sbg])
                 .arg(TContext::get()->instance.activeActivitiesList[ai]->id)
                 .arg(TContext::get()->instance.activeActivitiesList[ai]->duration);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}
/*
bool computeStudentsActivityTagMaxHoursDaily(QWidget* parent)
{
    haveStudentsActivityTagMaxHoursDaily=false;

    bool ok=true;

    for(int i=0; i<Timetable::getInstance()->instance.directSubgroupsList.size(); i++){
        subgroupsActivityTagMaxHoursDailyMaxHours[i].clear();
        subgroupsActivityTagMaxHoursDailyPercentage[i].clear();
        subgroupsActivityTagMaxHoursDailyActivityTag[i].clear();
    }

    for(int i=0; i<Timetable::getInstance()->instance.validTimeConstraintsList.size(); i++){
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            haveStudentsActivityTagMaxHoursDaily=true;

            ConstraintStudentsActivityTagMaxHoursDaily* samc=(ConstraintStudentsActivityTagMaxHoursDaily*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int sb, samc->canonicalSubgroupsList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<subgroupsActivityTagMaxHoursDailyMaxHours[sb].count(); j++){
                    if(subgroupsActivityTagMaxHoursDailyActivityTag[sb].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    subgroupsActivityTagMaxHoursDailyActivityTag[sb].append(samc->activityTagIndex);
                    subgroupsActivityTagMaxHoursDailyMaxHours[sb].append(samc->maxHoursDaily);
                    subgroupsActivityTagMaxHoursDailyPercentage[sb].append(samc->weightPercentage);
                }
                else{
                    if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos1) <= samc->maxHoursDaily
                     && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos1) >= samc->maxHoursDaily
                     && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos1) <= samc->weightPercentage){

                        subgroupsActivityTagMaxHoursDailyActivityTag[sb][pos1]=samc->activityTagIndex;
                        subgroupsActivityTagMaxHoursDailyMaxHours[sb][pos1]=samc->maxHoursDaily;
                        subgroupsActivityTagMaxHoursDailyPercentage[sb][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            subgroupsActivityTagMaxHoursDailyActivityTag[sb].append(samc->activityTagIndex);
                            subgroupsActivityTagMaxHoursDailyMaxHours[sb].append(samc->maxHoursDaily);
                            subgroupsActivityTagMaxHoursDailyPercentage[sb].append(samc->weightPercentage);
                        }
                        else{

                            if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos2) <= samc->maxHoursDaily
                             && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos2) >= samc->maxHoursDaily
                             && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos2) <= samc->weightPercentage){

                                subgroupsActivityTagMaxHoursDailyActivityTag[sb][pos2]=samc->activityTagIndex;
                                subgroupsActivityTagMaxHoursDailyMaxHours[sb][pos2]=samc->maxHoursDaily;
                                subgroupsActivityTagMaxHoursDailyPercentage[sb][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                                 " of type activity tag max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours daily.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.directSubgroupsList[sb]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY){
            haveStudentsActivityTagMaxHoursDaily=true;

            ConstraintStudentsSetActivityTagMaxHoursDaily* samc=(ConstraintStudentsSetActivityTagMaxHoursDaily*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int sb, samc->canonicalSubgroupsList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<subgroupsActivityTagMaxHoursDailyMaxHours[sb].count(); j++){
                    if(subgroupsActivityTagMaxHoursDailyActivityTag[sb].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    subgroupsActivityTagMaxHoursDailyActivityTag[sb].append(samc->activityTagIndex);
                    subgroupsActivityTagMaxHoursDailyMaxHours[sb].append(samc->maxHoursDaily);
                    subgroupsActivityTagMaxHoursDailyPercentage[sb].append(samc->weightPercentage);
                }
                else{
                    if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos1) <= samc->maxHoursDaily
                     && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos1) >= samc->maxHoursDaily
                     && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos1) <= samc->weightPercentage){

                        subgroupsActivityTagMaxHoursDailyActivityTag[sb][pos1]=samc->activityTagIndex;
                        subgroupsActivityTagMaxHoursDailyMaxHours[sb][pos1]=samc->maxHoursDaily;
                        subgroupsActivityTagMaxHoursDailyPercentage[sb][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            subgroupsActivityTagMaxHoursDailyActivityTag[sb].append(samc->activityTagIndex);
                            subgroupsActivityTagMaxHoursDailyMaxHours[sb].append(samc->maxHoursDaily);
                            subgroupsActivityTagMaxHoursDailyPercentage[sb].append(samc->weightPercentage);
                        }
                        else{

                            if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos2) <= samc->maxHoursDaily
                             && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(subgroupsActivityTagMaxHoursDailyMaxHours[sb].at(pos2) >= samc->maxHoursDaily
                             && subgroupsActivityTagMaxHoursDailyPercentage[sb].at(pos2) <= samc->weightPercentage){

                                subgroupsActivityTagMaxHoursDailyActivityTag[sb][pos2]=samc->activityTagIndex;
                                subgroupsActivityTagMaxHoursDailyMaxHours[sb][pos2]=samc->maxHoursDaily;
                                subgroupsActivityTagMaxHoursDailyPercentage[sb][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                                 " of type activity tag max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours daily.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.directSubgroupsList[sb]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
    }

    int navd[MAX_DAYS_PER_WEEK];

    for(int i=0; i<Timetable::getInstance()->instance.directSubgroupsList.size(); i++){
        for(int d=0; d<Timetable::getInstance()->instance.nDaysPerWeek; d++){
            navd[d]=0;
            for(int h=0; h<Timetable::getInstance()->instance.nHoursPerDay; h++){
                if(!Timetable::getInstance()->instance.breakDayHour[d][h] && !Timetable::getInstance()->instance.subgroupNotAvailableDayHour[i][d][h])
                    navd[d]++;
            }
        }

        for(int j=0; j<subgroupsActivityTagMaxHoursDailyMaxHours[i].count(); j++){
            int mh=subgroupsActivityTagMaxHoursDailyMaxHours[i].at(j);
            double perc=subgroupsActivityTagMaxHoursDailyPercentage[i].at(j);
            int at=subgroupsActivityTagMaxHoursDailyActivityTag[i].at(j);
            if(perc==100.0){
                int totalAt=0;
                foreach(int ai, Timetable::getInstance()->instance.directSubgroupsList[i]->activitiesForSubgroup)
                    if(Timetable::getInstance()->instance.activeActivitiesList[ai].iActivityTagsSet.contains(at))
                        totalAt+=Timetable::getInstance()->instance.activeActivitiesList[ai]->duration;

                int ava=0;
                for(int d=0; d<Timetable::getInstance()->instance.nDaysPerWeek; d++)
                    ava+=min(navd[d], mh);

                if(ava<totalAt){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there is a constraint activity tag %2 max %3 hours daily for it with weight 100%"
                     " which cannot be satisfied, considering the number of available slots (%4) and total duration of activities with this activity tag (%5)"
                     ". Please correct and try again.", "%2 is the activity tag for this constraint, %3 is the max number of hours daily for this constraint")
                     .arg(Timetable::getInstance()->instance.directSubgroupsList[i]->name).arg(Timetable::getInstance()->instance.activityTagsList.at(at)->name).arg(mh).arg(ava).arg(totalAt),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
    }

    return ok;
}
*/
/*
bool computeStudentsActivityTagMaxHoursContinuously(QWidget* parent)
{
    haveStudentsActivityTagMaxHoursContinuously=false;

    bool ok=true;

    for(int i=0; i<Timetable::getInstance()->instance.directSubgroupsList.size(); i++){
        subgroupsActivityTagMaxHoursContinuouslyMaxHours[i].clear();
        subgroupsActivityTagMaxHoursContinuouslyPercentage[i].clear();
        subgroupsActivityTagMaxHoursContinuouslyActivityTag[i].clear();
    }

    for(int i=0; i<Timetable::getInstance()->instance.validTimeConstraintsList.size(); i++){
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            haveStudentsActivityTagMaxHoursContinuously=true;

            ConstraintStudentsActivityTagMaxHoursContinuously* samc=(ConstraintStudentsActivityTagMaxHoursContinuously*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int sb, samc->canonicalSubgroupsList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].count(); j++){
                    if(subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].append(samc->activityTagIndex);
                    subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].append(samc->maxHoursContinuously);
                    subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].append(samc->weightPercentage);
                }
                else{
                    if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos1) <= samc->maxHoursContinuously
                     && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos1) >= samc->maxHoursContinuously
                     && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos1) <= samc->weightPercentage){

                        subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb][pos1]=samc->activityTagIndex;
                        subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb][pos1]=samc->maxHoursContinuously;
                        subgroupsActivityTagMaxHoursContinuouslyPercentage[sb][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].append(samc->activityTagIndex);
                            subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].append(samc->maxHoursContinuously);
                            subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].append(samc->weightPercentage);
                        }
                        else{

                            if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos2) <= samc->maxHoursContinuously
                             && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos2) >= samc->maxHoursContinuously
                             && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos2) <= samc->weightPercentage){

                                subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb][pos2]=samc->activityTagIndex;
                                subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb][pos2]=samc->maxHoursContinuously;
                                subgroupsActivityTagMaxHoursContinuouslyPercentage[sb][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                                 " of type activity tag max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours continuously.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.directSubgroupsList[sb]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            haveStudentsActivityTagMaxHoursContinuously=true;

            ConstraintStudentsSetActivityTagMaxHoursContinuously* samc=(ConstraintStudentsSetActivityTagMaxHoursContinuously*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int sb, samc->canonicalSubgroupsList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].count(); j++){
                    if(subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].append(samc->activityTagIndex);
                    subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].append(samc->maxHoursContinuously);
                    subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].append(samc->weightPercentage);
                }
                else{
                    if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos1) <= samc->maxHoursContinuously
                     && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos1) >= samc->maxHoursContinuously
                     && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos1) <= samc->weightPercentage){

                        subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb][pos1]=samc->activityTagIndex;
                        subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb][pos1]=samc->maxHoursContinuously;
                        subgroupsActivityTagMaxHoursContinuouslyPercentage[sb][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb].append(samc->activityTagIndex);
                            subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].append(samc->maxHoursContinuously);
                            subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].append(samc->weightPercentage);
                        }
                        else{

                            if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos2) <= samc->maxHoursContinuously
                             && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb].at(pos2) >= samc->maxHoursContinuously
                             && subgroupsActivityTagMaxHoursContinuouslyPercentage[sb].at(pos2) <= samc->weightPercentage){

                                subgroupsActivityTagMaxHoursContinuouslyActivityTag[sb][pos2]=samc->activityTagIndex;
                                subgroupsActivityTagMaxHoursContinuouslyMaxHours[sb][pos2]=samc->maxHoursContinuously;
                                subgroupsActivityTagMaxHoursContinuouslyPercentage[sb][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are too many constraints"
                                 " of type activity tag max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a subgroup, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours continuously.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.directSubgroupsList[sb]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
    }

    return ok;
}
*/
bool checkMaxHoursForActivityDuration(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activeActivitiesList[i];

        /* //TODO:
        //teachers
        foreach(int tch, act->iTeachersList){
            if(teachersMaxHoursDailyPercentages1[tch]==100.0){
                int m=teachersMaxHoursDailyMaxHours1[tch];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint teacher(s) max %3 hours daily with weight = 100% for the teacher %4. The activity's duration is"
                     " higher than the teacher's max hours daily. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(Timetable::getInstance()->instance.teachersList[tch]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
            if(teachersMaxHoursDailyPercentages2[tch]==100.0){
                int m=teachersMaxHoursDailyMaxHours2[tch];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint teacher(s) max %3 hours daily with weight = 100% for the teacher %4. The activity's duration is"
                     " higher than the teacher's max hours daily. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(Timetable::getInstance()->instance.teachersList[tch]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            if(teachersMaxHoursContinuouslyPercentages1[tch]==100.0){
                int m=teachersMaxHoursContinuouslyMaxHours1[tch];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint teacher(s) max %3 hours continuously with weight = 100% for the teacher %4. The activity's duration is"
                     " higher than the teacher's max hours continuously. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(Timetable::getInstance()->instance.teachersList[tch]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
            if(teachersMaxHoursContinuouslyPercentages2[tch]==100.0){
                int m=teachersMaxHoursContinuouslyMaxHours2[tch];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint teacher(s) max %3 hours continuously with weight = 100% for the teacher %4. The activity's duration is"
                     " higher than the teacher's max hours continuously. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(Timetable::getInstance()->instance.teachersList[tch]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            for(int j=0; j<teachersActivityTagMaxHoursDailyMaxHours[tch].count(); j++){
                if(teachersActivityTagMaxHoursDailyPercentage[tch].at(j)==100.0){
                    int m=teachersActivityTagMaxHoursDailyMaxHours[tch].at(j);
                    int at=teachersActivityTagMaxHoursDailyActivityTag[tch].at(j);
                    if(act->iActivityTagsSet.contains(at) && act->duration > m){
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                         " a constraint teacher(s) activity tag max %3 hours daily with weight = 100% for the teacher %4 and activity tag %5."
                         " The activity's duration is higher than the teacher's max hours daily with this activity tag (which the activity contains)."
                         " Please correct and try again.")
                         .arg(act->id)
                         .arg(act->duration)
                         .arg(m)
                         .arg(Timetable::getInstance()->instance.teachersList[tch]->name)
                         .arg(Timetable::getInstance()->instance.activityTagsList[at]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }

            for(int j=0; j<teachersActivityTagMaxHoursContinuouslyMaxHours[tch].count(); j++){
                if(teachersActivityTagMaxHoursContinuouslyPercentage[tch].at(j)==100.0){
                    int m=teachersActivityTagMaxHoursContinuouslyMaxHours[tch].at(j);
                    int at=teachersActivityTagMaxHoursContinuouslyActivityTag[tch].at(j);
                    if(act->iActivityTagsSet.contains(at) && act->duration > m){
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                         " a constraint teacher(s) activity tag max %3 hours continuously with weight = 100% for the teacher %4 and activity tag %5."
                         " The activity's duration is higher than the teacher's max hours continuously with this activity tag (which the activity contains)."
                         " Please correct and try again.")
                         .arg(act->id)
                         .arg(act->duration)
                         .arg(m)
                         .arg(Timetable::getInstance()->instance.teachersList[tch]->name)
                         .arg(Timetable::getInstance()->instance.activityTagsList[at]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
        */

        //students
        foreach(int sbg, act->iSubgroupsList){
            if(subgroupsMaxHoursDailyPercentages1[sbg]==100.0){
                int m=subgroupsMaxHoursDailyMaxHours1[sbg];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint students (set) max %3 hours daily with weight = 100% for the subgroup %4. The activity's duration is"
                     " higher than the subgroup's max hours daily. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            if(subgroupsMaxHoursDailyPercentages2[sbg]==100.0){
                int m=subgroupsMaxHoursDailyMaxHours2[sbg];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint students (set) max %3 hours daily with weight = 100% for the subgroup %4. The activity's duration is"
                     " higher than the subgroup's max hours daily. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            if(subgroupsMaxHoursContinuouslyPercentages1[sbg]==100.0){
                int m=subgroupsMaxHoursContinuouslyMaxHours1[sbg];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint students (set) max %3 hours continuously with weight = 100% for the subgroup %4. The activity's duration is"
                     " higher than the subgroup's max hours continuously. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
            if(subgroupsMaxHoursContinuouslyPercentages2[sbg]==100.0){
                int m=subgroupsMaxHoursContinuouslyMaxHours2[sbg];
                if(act->duration > m){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                     " a constraint students (set) max %3 hours continuously with weight = 100% for the subgroup %4. The activity's duration is"
                     " higher than the subgroup's max hours continuously. Please correct and try again.")
                     .arg(act->id)
                     .arg(act->duration)
                     .arg(m)
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            for(int j=0; j<subgroupsActivityTagMaxHoursDailyMaxHours[sbg].count(); j++){
                if(subgroupsActivityTagMaxHoursDailyPercentage[sbg].at(j)==100.0){
                    int m=subgroupsActivityTagMaxHoursDailyMaxHours[sbg].at(j);
                    int at=subgroupsActivityTagMaxHoursDailyActivityTag[sbg].at(j);
                    if(act->iActivityTagsSet.contains(at) && act->duration > m){
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                         " a constraint students (set) activity tag max %3 hours daily with weight = 100% for the subgroup %4 and activity tag %5."
                         " The activity's duration is higher than the subgroup's max hours daily with this activity tag (which the activity contains)."
                         " Please correct and try again.")
                         .arg(act->id)
                         .arg(act->duration)
                         .arg(m)
                         .arg(TContext::get()->instance.directSubgroupsList[sbg]->name)
                         .arg(TContext::get()->instance.activityTagsList[at]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }

            for(int j=0; j<subgroupsActivityTagMaxHoursContinuouslyMaxHours[sbg].count(); j++){
                if(subgroupsActivityTagMaxHoursContinuouslyPercentage[sbg].at(j)==100.0){
                    int m=subgroupsActivityTagMaxHoursContinuouslyMaxHours[sbg].at(j);
                    int at=subgroupsActivityTagMaxHoursContinuouslyActivityTag[sbg].at(j);
                    if(act->iActivityTagsSet.contains(at) && act->duration > m){
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize, because you have activity id = %1 with duration = %2 and"
                         " a constraint students (set) activity tag max %3 hours continuously with weight = 100% for the subgroup %4 and activity tag %5."
                         " The activity's duration is higher than the subgroup's max hours continuously with this activity tag (which the activity contains)."
                         " Please correct and try again.")
                         .arg(act->id)
                         .arg(act->duration)
                         .arg(m)
                         .arg(TContext::get()->instance.directSubgroupsList[sbg]->name)
                         .arg(TContext::get()->instance.activityTagsList[at]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
    }

    return ok;
}

bool computeSubgroupsMinHoursDaily(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMinHoursDailyMinHours[i]=-1;
        subgroupsMinHoursDailyPercentages[i]=-1;
        subgroupsMinHoursDailyAllowEmptyDays[i]=true;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MIN_HOURS_DAILY){
            ConstraintStudentsMinHoursDaily* smd=(ConstraintStudentsMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            if(smd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for students, because the constraint of type min hours daily relating to students"
                 " has no 100% weight"
                 ". Please modify your data accordingly and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            //////////
            if(smd->minHoursDaily>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students min hours daily with"
                 " %1 min hours daily, and the number of working hours per day is only %2. Please correct and try again")
                 .arg(smd->minHoursDaily)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY){
            ConstraintStudentsSetMinHoursDaily* smd=(ConstraintStudentsSetMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            if(smd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for students set %1, because the constraint of type min hours daily relating to him"
                 " has no 100% weight"
                 ". Please modify your data accordingly and try again")
                 .arg(smd->students),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            //////////
            if(smd->minHoursDaily>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set min hours daily for students set %1 with"
                 " %2 min hours daily, and the number of working hours per day is only %3. Please correct and try again")
                 .arg(smd->students)
                 .arg(smd->minHoursDaily)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
        }
    }

    if(!ok)
        return ok;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MIN_HOURS_DAILY){
            ConstraintStudentsMinHoursDaily* smd=(ConstraintStudentsMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
                if(subgroupsMinHoursDailyMinHours[sb]==-1 ||
                 (subgroupsMinHoursDailyMinHours[sb] <= smd->minHoursDaily &&
                 subgroupsMinHoursDailyPercentages[sb] <= smd->weightPercentage)){
                    subgroupsMinHoursDailyMinHours[sb] = smd->minHoursDaily;
                    subgroupsMinHoursDailyPercentages[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMinHoursDailyMinHours[sb] >= smd->minHoursDaily &&
                 subgroupsMinHoursDailyPercentages[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{ //cannot proceed
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are two constraints"
                     " of type min hours daily relating to him, and the weight percentage is higher on the constraint"
                     " with less minimum hours. You are allowed only to have for each subgroup"
                     " the most important constraint with maximum weight percentage and largest minimum hours daily allowed"
                     ". Please modify your data accordingly and try again")
                     .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }

                if(smd->allowEmptyDays==false)
                    subgroupsMinHoursDailyAllowEmptyDays[sb]=false;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY){
            ConstraintStudentsSetMinHoursDaily* smd=(ConstraintStudentsSetMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int q=0; q<smd->iSubgroupsList.count(); q++){
                int sb=smd->iSubgroupsList.at(q);
                if(subgroupsMinHoursDailyMinHours[sb]==-1 ||
                 (subgroupsMinHoursDailyMinHours[sb] <= smd->minHoursDaily &&
                 subgroupsMinHoursDailyPercentages[sb] <= smd->weightPercentage)){
                    subgroupsMinHoursDailyMinHours[sb] = smd->minHoursDaily;
                    subgroupsMinHoursDailyPercentages[sb] = smd->weightPercentage;
                    }
                else if(subgroupsMinHoursDailyMinHours[sb] >= smd->minHoursDaily &&
                 subgroupsMinHoursDailyPercentages[sb] >= smd->weightPercentage){
                    //nothing
                }
                else{ //cannot proceed
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because there are two constraints"
                     " of type min hours daily relating to him, and the weight percentage is higher on the constraint"
                     " with less minimum hours. You are allowed only to have for each subgroup"
                     " the most important constraint with maximum weight percentage and largest minimum hours daily allowed"
                     ". Please modify your data accordingly and try again")
                     .arg(TContext::get()->instance.directSubgroupsList[sb]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
                if(smd->allowEmptyDays==false)
                    subgroupsMinHoursDailyAllowEmptyDays[sb]=false;
            }
        }
    }

    haveStudentsMinHoursDailyAllowEmptyDays=false;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        if(subgroupsMinHoursDailyMinHours[i]>=0 && subgroupsMinHoursDailyAllowEmptyDays[i]==true && !haveStudentsMinHoursDailyAllowEmptyDays)
            haveStudentsMinHoursDailyAllowEmptyDays=true;

        if(subgroupsMinHoursDailyMinHours[i]>=0 && subgroupsMinHoursDailyAllowEmptyDays[i]==false){
            if(TContext::get()->instance.nDaysPerWeek*subgroupsMinHoursDailyMinHours[i] > nHoursPerSubgroup[i]){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("For subgroup %1 you have too little activities to respect the constraint(s)"
                 " of type min hours daily (the constraint(s) do not allow empty days). Please modify your data accordingly and try again.")
                 .arg(TContext::get()->instance.directSubgroupsList[i]->name),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++){
                int freeSlots=0;
                for(int k=0; k<TContext::get()->instance.nHoursPerDay; k++)
                    if(!TContext::get()->instance.subgroupNotAvailableDayHour[i][j][k] && !TContext::get()->instance.breakDayHour[j][k])
                        freeSlots++;
                if(subgroupsMinHoursDailyMinHours[i]>freeSlots){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("For subgroup %1 cannot respect the constraint(s)"
                     " of type min hours daily (the constraint(s) do not allow empty days) on day %2, because of students set not available and/or break."
                     " Please modify your data accordingly and try again")
                     .arg(TContext::get()->instance.directSubgroupsList[i]->name)
                     .arg(TContext::get()->instance.daysOfTheWeek[j]),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        else if(subgroupsMinHoursDailyMinHours[i]>=0 && subgroupsMinHoursDailyAllowEmptyDays[i]==true){

            if(nHoursPerSubgroup[i]>0 && subgroupsMinHoursDailyMinHours[i]>nHoursPerSubgroup[i]){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students min %1 hours daily for subgroup"
                 " %2 (the constraint allows empty days). This subgroup has in total only %3 hours per week, so impossible constraint."
                 " Please correct and try again")
                 .arg(subgroupsMinHoursDailyMinHours[i])
                 .arg(TContext::get()->instance.directSubgroupsList[i]->name)
                 .arg(nHoursPerSubgroup[i])
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            if(subgroupsMinHoursDailyMinHours[i]<2){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students min %1 hours daily for subgroup"
                 " %2 and the constraint allows empty days. The number of min hours daily should be at least 2, to make a non-trivial constraint. Please correct and try again")
                 .arg(subgroupsMinHoursDailyMinHours[i])
                 .arg(TContext::get()->instance.directSubgroupsList[i]->name)
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

//must be after allowed times, after n hours per teacher and after max days per week for teachers
bool computeTeachersMaxHoursDaily(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMaxHoursDailyMaxHours1[i]=-1;
        teachersMaxHoursDailyPercentages1[i]=-1;

        teachersMaxHoursDailyMaxHours2[i]=-1;
        teachersMaxHoursDailyPercentages2[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_HOURS_DAILY){
            ConstraintTeacherMaxHoursDaily* tmd=(ConstraintTeacherMaxHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            /*if(tmd->weightPercentage!=100){
                ok=false;

                int t=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher max hours daily for teacher %1 with"
                 " weight (percentage) below 100. Starting with m-FET version 5.3.0 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(tmd->teacherName),
                 GeneratePreTranslate::tr("Skip rest of max hours problems"), GeneratePreTranslate::tr("See next incompatibility max hours"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }*/
            //////////

            if(teachersMaxHoursDailyMaxHours1[tmd->teacher_ID]==-1 ||
             (teachersMaxHoursDailyMaxHours1[tmd->teacher_ID] >= tmd->maxHoursDaily &&
             teachersMaxHoursDailyPercentages1[tmd->teacher_ID] <= tmd->weightPercentage)){
                teachersMaxHoursDailyMaxHours1[tmd->teacher_ID] = tmd->maxHoursDaily;
                teachersMaxHoursDailyPercentages1[tmd->teacher_ID] = tmd->weightPercentage;
            }
            else if(teachersMaxHoursDailyMaxHours1[tmd->teacher_ID] <= tmd->maxHoursDaily &&
             teachersMaxHoursDailyPercentages1[tmd->teacher_ID] >= tmd->weightPercentage){
                //nothing
            }
            else{
                if(teachersMaxHoursDailyMaxHours2[tmd->teacher_ID]==-1 ||
                 (teachersMaxHoursDailyMaxHours2[tmd->teacher_ID] >= tmd->maxHoursDaily &&
                 teachersMaxHoursDailyPercentages2[tmd->teacher_ID] <= tmd->weightPercentage)){
                    teachersMaxHoursDailyMaxHours2[tmd->teacher_ID] = tmd->maxHoursDaily;
                    teachersMaxHoursDailyPercentages2[tmd->teacher_ID] = tmd->weightPercentage;
                }
                else if(teachersMaxHoursDailyMaxHours2[tmd->teacher_ID] <= tmd->maxHoursDaily &&
                 teachersMaxHoursDailyPercentages2[tmd->teacher_ID] >= tmd->weightPercentage){
                    //nothing
                }
                else{ //cannot proceed
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                     " of type max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                     " Two constraints max hours can be compressed into a single one if the max hours are lower"
                     " in the first one and the weight percentage is higher on the first one."
                     " It is possible to use any number of such constraints for a teacher, but their resultant must"
                     " be maximum 2 constraints of type max hours daily.\n\n"
                     " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                     " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                     " Please modify your data accordingly and try again.")
                     .arg(TContext::get()->instance.teachersList[tmd->teacher_ID]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_HOURS_DAILY){
            ConstraintTeachersMaxHoursDaily* tmd=(ConstraintTeachersMaxHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            /*if(tmd->weightPercentage!=100){
                ok=false;

                int t=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers max hours daily with"
                 " weight (percentage) below 100. Starting with m-FET version 5.3.0 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest of max hours problems"), GeneratePreTranslate::tr("See next incompatibility max hours"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }*/
            //////////

            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersMaxHoursDailyMaxHours1[tch]==-1 ||
                 (teachersMaxHoursDailyMaxHours1[tch] >= tmd->maxHoursDaily &&
                 teachersMaxHoursDailyPercentages1[tch] <= tmd->weightPercentage)){
                    teachersMaxHoursDailyMaxHours1[tch] = tmd->maxHoursDaily;
                    teachersMaxHoursDailyPercentages1[tch] = tmd->weightPercentage;
                    }
                else if(teachersMaxHoursDailyMaxHours1[tch] <= tmd->maxHoursDaily &&
                 teachersMaxHoursDailyPercentages1[tch] >= tmd->weightPercentage){
                    //nothing
                }
                else{
                    if(teachersMaxHoursDailyMaxHours2[tch]==-1 ||
                     (teachersMaxHoursDailyMaxHours2[tch] >= tmd->maxHoursDaily &&
                     teachersMaxHoursDailyPercentages2[tch] <= tmd->weightPercentage)){
                        teachersMaxHoursDailyMaxHours2[tch] = tmd->maxHoursDaily;
                        teachersMaxHoursDailyPercentages2[tch] = tmd->weightPercentage;
                        }
                    else if(teachersMaxHoursDailyMaxHours2[tch] <= tmd->maxHoursDaily &&
                     teachersMaxHoursDailyPercentages2[tch] >= tmd->weightPercentage){
                    //nothing
                    }
                    else{ //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                         " of type max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a teacher, but their resultant must"
                         " be maximum 2 constraints of type max hours daily.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.teachersList[tch]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
    }

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(IS_EQUAL(teachersMaxHoursDailyPercentages1[tc], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],teachersMaxHoursDailyMaxHours1[tc]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(teachersMaxDaysPerWeekMaxDays[tc]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(teachersMaxDaysPerWeekMaxDays[tc]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<teachersMaxDaysPerWeekMaxDays[tc]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerTeacher[tc]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint of type"
                 " max %2 hours daily with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, teacher max days per week, teacher not available and/or breaks."
                 " The number of total hours for this teacher is"
                 " %3 and the number of available slots is, considering max hours daily and all other constraints, %4.")
                 .arg(TContext::get()->instance.teachersList[tc]->name)
                 .arg(teachersMaxHoursDailyMaxHours1[tc])
                 .arg(nHoursPerTeacher[tc])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(IS_EQUAL(teachersMaxHoursDailyPercentages2[tc], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],teachersMaxHoursDailyMaxHours2[tc]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(teachersMaxDaysPerWeekMaxDays[tc]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(teachersMaxDaysPerWeekMaxDays[tc]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<teachersMaxDaysPerWeekMaxDays[tc]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerTeacher[tc]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint of type"
                 " max %2 hours daily with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, teacher max days per week, teacher not available and/or breaks."
                 " The number of total hours for this teacher is"
                 " %3 and the number of available slots is, considering max hours daily and all other constraints, %4.")
                 .arg(TContext::get()->instance.teachersList[tc]->name)
                 .arg(teachersMaxHoursDailyMaxHours2[tc])
                 .arg(nHoursPerTeacher[tc])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

//must be after allowed times, after n hours per teacher and after max days per week for teachers
bool computeTeachersMaxSpanPerDay(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMaxSpanPerDayMaxSpan[i]=-1;
        teachersMaxSpanPerDayPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY){
            ConstraintTeacherMaxSpanPerDay* tmsd=(ConstraintTeacherMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tmsd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher max span per day for teacher %1"
                 " with weight (percentage) below 100. Please make weight 100% and try again").arg(tmsd->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_SPAN_PER_DAY){
            ConstraintTeachersMaxSpanPerDay* tmsd=(ConstraintTeachersMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tmsd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers max span per day"
                 " with weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY){
            ConstraintTeacherMaxSpanPerDay* tmsd=(ConstraintTeacherMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(teachersMaxSpanPerDayPercentages[tmsd->teacher_ID] < 0
             || (teachersMaxSpanPerDayPercentages[tmsd->teacher_ID]>=0 && teachersMaxSpanPerDayMaxSpan[tmsd->teacher_ID]>tmsd->maxSpanPerDay)){
                teachersMaxSpanPerDayPercentages[tmsd->teacher_ID]=100.0;
                teachersMaxSpanPerDayMaxSpan[tmsd->teacher_ID]=tmsd->maxSpanPerDay;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_SPAN_PER_DAY){
            ConstraintTeachersMaxSpanPerDay* tmsd=(ConstraintTeachersMaxSpanPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersMaxSpanPerDayPercentages[tch] < 0
                 || (teachersMaxSpanPerDayPercentages[tch]>=0 && teachersMaxSpanPerDayMaxSpan[tch]>tmsd->maxSpanPerDay)){
                    teachersMaxSpanPerDayPercentages[tch]=100.0;
                    teachersMaxSpanPerDayMaxSpan[tch]=tmsd->maxSpanPerDay;
                }
            }
        }
    }

    //This is similar to teachers max hours daily checking. It is not a very useful test, but does not hurt.
    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(IS_EQUAL(teachersMaxSpanPerDayPercentages[tc], 100)){
            int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                nAllowedSlotsPerDay[d]=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h])
                        nAllowedSlotsPerDay[d]++;
                nAllowedSlotsPerDay[d]=min(nAllowedSlotsPerDay[d],teachersMaxSpanPerDayMaxSpan[tc]);
            }

            int dayAvailable[MAX_DAYS_PER_WEEK];
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=1;
            if(teachersMaxDaysPerWeekMaxDays[tc]>=0){
                //n days per week has 100% weight
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    dayAvailable[d]=0;
                assert(teachersMaxDaysPerWeekMaxDays[tc]<=TContext::get()->instance.nDaysPerWeek);
                for(int k=0; k<teachersMaxDaysPerWeekMaxDays[tc]; k++){
                    int maxPos=-1, maxVal=-1;
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        if(dayAvailable[d]==0)
                            if(maxVal<nAllowedSlotsPerDay[d]){
                                maxVal=nAllowedSlotsPerDay[d];
                                maxPos=d;
                            }
                    assert(maxPos>=0);
                    assert(dayAvailable[maxPos]==0);
                    dayAvailable[maxPos]=1;
                }
            }

            int total=0;
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                if(dayAvailable[d]==1)
                    total+=nAllowedSlotsPerDay[d];
            if(total<nHoursPerTeacher[tc]){
                ok=false;

                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint of type"
                 " max %2 span per day with 100% weight which cannot be respected because of number of days per week,"
                 " number of hours per day, teacher max days per week, teacher not available and/or breaks."
                 " The number of total hours for this teacher is"
                 " %3 and the number of available slots is, considering max span per day and all other constraints, %4.")
                 .arg(TContext::get()->instance.teachersList[tc]->name)
                 .arg(teachersMaxSpanPerDayMaxSpan[tc])
                 .arg(nHoursPerTeacher[tc])
                 .arg(total);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

bool computeTeachersMaxHoursContinuously(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMaxHoursContinuouslyMaxHours1[i]=-1;
        teachersMaxHoursContinuouslyPercentages1[i]=-1;

        teachersMaxHoursContinuouslyMaxHours2[i]=-1;
        teachersMaxHoursContinuouslyPercentages2[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeacherMaxHoursContinuously* tmd=(ConstraintTeacherMaxHoursContinuously*)TContext::get()->instance.validTimeConstraintsList[i];

            if(teachersMaxHoursContinuouslyMaxHours1[tmd->teacher_ID]==-1 ||
             (teachersMaxHoursContinuouslyMaxHours1[tmd->teacher_ID] >= tmd->maxHoursContinuously &&
             teachersMaxHoursContinuouslyPercentages1[tmd->teacher_ID] <= tmd->weightPercentage)){
                teachersMaxHoursContinuouslyMaxHours1[tmd->teacher_ID] = tmd->maxHoursContinuously;
                teachersMaxHoursContinuouslyPercentages1[tmd->teacher_ID] = tmd->weightPercentage;
            }
            else if(teachersMaxHoursContinuouslyMaxHours1[tmd->teacher_ID] <= tmd->maxHoursContinuously &&
             teachersMaxHoursContinuouslyPercentages1[tmd->teacher_ID] >= tmd->weightPercentage){
                //nothing
            }
            else{
                if(teachersMaxHoursContinuouslyMaxHours2[tmd->teacher_ID]==-1 ||
                 (teachersMaxHoursContinuouslyMaxHours2[tmd->teacher_ID] >= tmd->maxHoursContinuously &&
                 teachersMaxHoursContinuouslyPercentages2[tmd->teacher_ID] <= tmd->weightPercentage)){
                    teachersMaxHoursContinuouslyMaxHours2[tmd->teacher_ID] = tmd->maxHoursContinuously;
                    teachersMaxHoursContinuouslyPercentages2[tmd->teacher_ID] = tmd->weightPercentage;
                }
                else if(teachersMaxHoursContinuouslyMaxHours2[tmd->teacher_ID] <= tmd->maxHoursContinuously &&
                 teachersMaxHoursContinuouslyPercentages2[tmd->teacher_ID] >= tmd->weightPercentage){
                    //nothing
                }
                else{ //cannot proceed
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                     " of type max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                     " Two constraints max hours can be compressed into a single one if the max hours are lower"
                     " in the first one and the weight percentage is higher on the first one."
                     " It is possible to use any number of such constraints for a teacher, but their resultant must"
                     " be maximum 2 constraints of type max hours continuously.\n\n"
                     " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                     " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                     " Please modify your data accordingly and try again.")
                     .arg(TContext::get()->instance.teachersList[tmd->teacher_ID]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_HOURS_CONTINUOUSLY){
            ConstraintTeachersMaxHoursContinuously* tmd=(ConstraintTeachersMaxHoursContinuously*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersMaxHoursContinuouslyMaxHours1[tch]==-1 ||
                 (teachersMaxHoursContinuouslyMaxHours1[tch] >= tmd->maxHoursContinuously &&
                 teachersMaxHoursContinuouslyPercentages1[tch] <= tmd->weightPercentage)){
                    teachersMaxHoursContinuouslyMaxHours1[tch] = tmd->maxHoursContinuously;
                    teachersMaxHoursContinuouslyPercentages1[tch] = tmd->weightPercentage;
                    }
                else if(teachersMaxHoursContinuouslyMaxHours1[tch] <= tmd->maxHoursContinuously &&
                 teachersMaxHoursContinuouslyPercentages1[tch] >= tmd->weightPercentage){
                    //nothing
                }
                else{
                    if(teachersMaxHoursContinuouslyMaxHours2[tch]==-1 ||
                     (teachersMaxHoursContinuouslyMaxHours2[tch] >= tmd->maxHoursContinuously &&
                     teachersMaxHoursContinuouslyPercentages2[tch] <= tmd->weightPercentage)){
                        teachersMaxHoursContinuouslyMaxHours2[tch] = tmd->maxHoursContinuously;
                        teachersMaxHoursContinuouslyPercentages2[tch] = tmd->weightPercentage;
                        }
                    else if(teachersMaxHoursContinuouslyMaxHours2[tch] <= tmd->maxHoursContinuously &&
                     teachersMaxHoursContinuouslyPercentages2[tch] >= tmd->weightPercentage){
                    //nothing
                    }
                    else{ //cannot proceed
                        ok=false;

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                         " of type max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                         " Two constraints max hours can be compressed into a single one if the max hours are lower"
                         " in the first one and the weight percentage is higher on the first one."
                         " It is possible to use any number of such constraints for a teacher, but their resultant must"
                         " be maximum 2 constraints of type max hours continuously.\n\n"
                         " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                         " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                         " Please modify your data accordingly and try again.")
                         .arg(TContext::get()->instance.teachersList[tch]->name),
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return false;
                    }
                }
            }
        }
    }
    /*//TODO:
    for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
        foreach(int tch, Timetable::getInstance()->instance.activeActivitiesList[ai]->iTeachersList){
            if(teachersMaxHoursContinuouslyPercentages1[tch]>=0 && Timetable::getInstance()->instance.activeActivitiesList[ai]->duration > teachersMaxHoursContinuouslyMaxHours1[tch]){
                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint of type"
                 " max %2 hours continuously which cannot be respected because of activity with id %3 (which has duration %4).")
                 .arg(Timetable::getInstance()->instance.teachersList[tch]->name)
                 .arg(teachersMaxHoursContinuouslyMaxHours1[tch])
                 .arg(Timetable::getInstance()->instance.activeActivitiesList[ai]->id)
                 .arg(Timetable::getInstance()->instance.activeActivitiesList[ai]->duration);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            if(teachersMaxHoursContinuouslyPercentages2[tch]>=0 && Timetable::getInstance()->instance.activeActivitiesList[ai]->duration > teachersMaxHoursContinuouslyMaxHours2[tch]){
                QString s;
                s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint of type"
                 " max %2 hours continuously which cannot be respected because of activity with id %3 (which has duration %4).")
                 .arg(Timetable::getInstance()->instance.teachersList[tch]->name)
                 .arg(teachersMaxHoursContinuouslyMaxHours2[tch])
                 .arg(Timetable::getInstance()->instance.activeActivitiesList[ai]->id)
                 .arg(Timetable::getInstance()->instance.activeActivitiesList[ai]->duration);
                s+="\n\n";
                s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }
    */

    return ok;
}
/*
bool computeTeachersActivityTagMaxHoursDaily(QWidget* parent)
{
    haveTeachersActivityTagMaxHoursDaily=false;

    bool ok=true;

    for(int i=0; i<Timetable::getInstance()->instance.teachersList.size(); i++){
        teachersActivityTagMaxHoursDailyMaxHours[i].clear();
        teachersActivityTagMaxHoursDailyPercentage[i].clear();
        teachersActivityTagMaxHoursDailyActivityTag[i].clear();
    }

    for(int i=0; i<Timetable::getInstance()->instance.validTimeConstraintsList.size(); i++){
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY){
            haveTeachersActivityTagMaxHoursDaily=true;

            ConstraintTeachersActivityTagMaxHoursDaily* samc=(ConstraintTeachersActivityTagMaxHoursDaily*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int tc, samc->canonicalTeachersList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<teachersActivityTagMaxHoursDailyMaxHours[tc].count(); j++){
                    if(teachersActivityTagMaxHoursDailyActivityTag[tc].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    teachersActivityTagMaxHoursDailyActivityTag[tc].append(samc->activityTagIndex);
                    teachersActivityTagMaxHoursDailyMaxHours[tc].append(samc->maxHoursDaily);
                    teachersActivityTagMaxHoursDailyPercentage[tc].append(samc->weightPercentage);
                }
                else{
                    if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos1) <= samc->maxHoursDaily
                     && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos1) >= samc->maxHoursDaily
                     && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos1) <= samc->weightPercentage){

                        teachersActivityTagMaxHoursDailyActivityTag[tc][pos1]=samc->activityTagIndex;
                        teachersActivityTagMaxHoursDailyMaxHours[tc][pos1]=samc->maxHoursDaily;
                        teachersActivityTagMaxHoursDailyPercentage[tc][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            teachersActivityTagMaxHoursDailyActivityTag[tc].append(samc->activityTagIndex);
                            teachersActivityTagMaxHoursDailyMaxHours[tc].append(samc->maxHoursDaily);
                            teachersActivityTagMaxHoursDailyPercentage[tc].append(samc->weightPercentage);
                        }
                        else{

                            if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos2) <= samc->maxHoursDaily
                             && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos2) >= samc->maxHoursDaily
                             && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos2) <= samc->weightPercentage){

                                teachersActivityTagMaxHoursDailyActivityTag[tc][pos2]=samc->activityTagIndex;
                                teachersActivityTagMaxHoursDailyMaxHours[tc][pos2]=samc->maxHoursDaily;
                                teachersActivityTagMaxHoursDailyPercentage[tc][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                                 " of type activity tag max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a teacher, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours daily.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.teachersList[tc]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY){
            haveTeachersActivityTagMaxHoursDaily=true;

            ConstraintTeacherActivityTagMaxHoursDaily* samc=(ConstraintTeacherActivityTagMaxHoursDaily*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int tc, samc->canonicalTeachersList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<teachersActivityTagMaxHoursDailyMaxHours[tc].count(); j++){
                    if(teachersActivityTagMaxHoursDailyActivityTag[tc].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    teachersActivityTagMaxHoursDailyActivityTag[tc].append(samc->activityTagIndex);
                    teachersActivityTagMaxHoursDailyMaxHours[tc].append(samc->maxHoursDaily);
                    teachersActivityTagMaxHoursDailyPercentage[tc].append(samc->weightPercentage);
                }
                else{
                    if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos1) <= samc->maxHoursDaily
                     && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos1) >= samc->maxHoursDaily
                     && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos1) <= samc->weightPercentage){

                        teachersActivityTagMaxHoursDailyActivityTag[tc][pos1]=samc->activityTagIndex;
                        teachersActivityTagMaxHoursDailyMaxHours[tc][pos1]=samc->maxHoursDaily;
                        teachersActivityTagMaxHoursDailyPercentage[tc][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            teachersActivityTagMaxHoursDailyActivityTag[tc].append(samc->activityTagIndex);
                            teachersActivityTagMaxHoursDailyMaxHours[tc].append(samc->maxHoursDaily);
                            teachersActivityTagMaxHoursDailyPercentage[tc].append(samc->weightPercentage);
                        }
                        else{

                            if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos2) <= samc->maxHoursDaily
                             && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(teachersActivityTagMaxHoursDailyMaxHours[tc].at(pos2) >= samc->maxHoursDaily
                             && teachersActivityTagMaxHoursDailyPercentage[tc].at(pos2) <= samc->weightPercentage){

                                teachersActivityTagMaxHoursDailyActivityTag[tc][pos2]=samc->activityTagIndex;
                                teachersActivityTagMaxHoursDailyMaxHours[tc][pos2]=samc->maxHoursDaily;
                                teachersActivityTagMaxHoursDailyPercentage[tc][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                                 " of type activity tag max hours daily relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a teacher, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours daily.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.teachersList[tc]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
    }

    int navd[MAX_DAYS_PER_WEEK];

    for(int i=0; i<Timetable::getInstance()->instance.teachersList.size(); i++){
        for(int d=0; d<Timetable::getInstance()->instance.nDaysPerWeek; d++){
            navd[d]=0;
            for(int h=0; h<Timetable::getInstance()->instance.nHoursPerDay; h++){
                if(!Timetable::getInstance()->instance.breakDayHour[d][h] && !Timetable::getInstance()->instance.teacherNotAvailableDayHour[i][d][h])
                    navd[d]++;
            }
        }

        for(int j=0; j<teachersActivityTagMaxHoursDailyMaxHours[i].count(); j++){
            int mh=teachersActivityTagMaxHoursDailyMaxHours[i].at(j);
            double perc=teachersActivityTagMaxHoursDailyPercentage[i].at(j);
            int at=teachersActivityTagMaxHoursDailyActivityTag[i].at(j);
            if(perc==100.0){
                int totalAt=0;
                foreach(int ai, Timetable::getInstance()->instance.teachersList[i]->activitiesForTeacher)
                    if(Timetable::getInstance()->instance.activeActivitiesList[ai].iActivityTagsSet.contains(at))
                        totalAt+=Timetable::getInstance()->instance.activeActivitiesList[ai]->duration;

                int ava=0;
                for(int d=0; d<Timetable::getInstance()->instance.nDaysPerWeek; d++)
                    ava+=min(navd[d], mh);

                if(ava<totalAt){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there is a constraint activity tag %2 max %3 hours daily for it with weight 100%"
                     " which cannot be satisfied, considering the number of available slots (%4) and total duration of activities with this activity tag (%5)"
                     ". Please correct and try again.", "%2 is the activity tag for this constraint, %3 is the max number of hours daily for this constraint")
                     .arg(Timetable::getInstance()->instance.teachersList[i]->name).arg(Timetable::getInstance()->instance.activityTagsList.at(at)->name).arg(mh).arg(ava).arg(totalAt),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
    }

    return ok;
}
*/
/*
bool computeTeachersActivityTagMaxHoursContinuously(QWidget* parent)
{
    haveTeachersActivityTagMaxHoursContinuously=false;

    bool ok=true;

    for(int i=0; i<Timetable::getInstance()->instance.teachersList.size(); i++){
        teachersActivityTagMaxHoursContinuouslyMaxHours[i].clear();
        teachersActivityTagMaxHoursContinuouslyPercentage[i].clear();
        teachersActivityTagMaxHoursContinuouslyActivityTag[i].clear();
    }

    for(int i=0; i<Timetable::getInstance()->instance.validTimeConstraintsList.size(); i++){
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            haveTeachersActivityTagMaxHoursContinuously=true;

            ConstraintTeachersActivityTagMaxHoursContinuously* samc=(ConstraintTeachersActivityTagMaxHoursContinuously*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int tc, samc->canonicalTeachersList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<teachersActivityTagMaxHoursContinuouslyMaxHours[tc].count(); j++){
                    if(teachersActivityTagMaxHoursContinuouslyActivityTag[tc].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    teachersActivityTagMaxHoursContinuouslyActivityTag[tc].append(samc->activityTagIndex);
                    teachersActivityTagMaxHoursContinuouslyMaxHours[tc].append(samc->maxHoursContinuously);
                    teachersActivityTagMaxHoursContinuouslyPercentage[tc].append(samc->weightPercentage);
                }
                else{
                    if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos1) <= samc->maxHoursContinuously
                     && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos1) >= samc->maxHoursContinuously
                     && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos1) <= samc->weightPercentage){

                        teachersActivityTagMaxHoursContinuouslyActivityTag[tc][pos1]=samc->activityTagIndex;
                        teachersActivityTagMaxHoursContinuouslyMaxHours[tc][pos1]=samc->maxHoursContinuously;
                        teachersActivityTagMaxHoursContinuouslyPercentage[tc][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            teachersActivityTagMaxHoursContinuouslyActivityTag[tc].append(samc->activityTagIndex);
                            teachersActivityTagMaxHoursContinuouslyMaxHours[tc].append(samc->maxHoursContinuously);
                            teachersActivityTagMaxHoursContinuouslyPercentage[tc].append(samc->weightPercentage);
                        }
                        else{

                            if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos2) <= samc->maxHoursContinuously
                             && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos2) >= samc->maxHoursContinuously
                             && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos2) <= samc->weightPercentage){

                                teachersActivityTagMaxHoursContinuouslyActivityTag[tc][pos2]=samc->activityTagIndex;
                                teachersActivityTagMaxHoursContinuouslyMaxHours[tc][pos2]=samc->maxHoursContinuously;
                                teachersActivityTagMaxHoursContinuouslyPercentage[tc][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                                 " of type activity tag max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a teacher, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours continuously.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.teachersList[tc]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
        if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY){
            haveTeachersActivityTagMaxHoursContinuously=true;

            ConstraintTeacherActivityTagMaxHoursContinuously* samc=(ConstraintTeacherActivityTagMaxHoursContinuously*)Timetable::getInstance()->instance.validTimeConstraintsList[i];

            foreach(int tc, samc->canonicalTeachersList){
                int pos1=-1, pos2=-1;

                for(int j=0; j<teachersActivityTagMaxHoursContinuouslyMaxHours[tc].count(); j++){
                    if(teachersActivityTagMaxHoursContinuouslyActivityTag[tc].at(j)==samc->activityTagIndex){
                        if(pos1==-1){
                            pos1=j;
                        }
                        else{
                            assert(pos2==-1);
                            pos2=j;
                        }
                    }
                }

                if(pos1==-1){
                    teachersActivityTagMaxHoursContinuouslyActivityTag[tc].append(samc->activityTagIndex);
                    teachersActivityTagMaxHoursContinuouslyMaxHours[tc].append(samc->maxHoursContinuously);
                    teachersActivityTagMaxHoursContinuouslyPercentage[tc].append(samc->weightPercentage);
                }
                else{
                    if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos1) <= samc->maxHoursContinuously
                     && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos1) >= samc->weightPercentage){
                        //do nothing
                    }
                    else if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos1) >= samc->maxHoursContinuously
                     && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos1) <= samc->weightPercentage){

                        teachersActivityTagMaxHoursContinuouslyActivityTag[tc][pos1]=samc->activityTagIndex;
                        teachersActivityTagMaxHoursContinuouslyMaxHours[tc][pos1]=samc->maxHoursContinuously;
                        teachersActivityTagMaxHoursContinuouslyPercentage[tc][pos1]=samc->weightPercentage;
                    }
                    else{
                        if(pos2==-1){
                            teachersActivityTagMaxHoursContinuouslyActivityTag[tc].append(samc->activityTagIndex);
                            teachersActivityTagMaxHoursContinuouslyMaxHours[tc].append(samc->maxHoursContinuously);
                            teachersActivityTagMaxHoursContinuouslyPercentage[tc].append(samc->weightPercentage);
                        }
                        else{

                            if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos2) <= samc->maxHoursContinuously
                             && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos2) >= samc->weightPercentage){
                                //do nothing
                            }
                            else if(teachersActivityTagMaxHoursContinuouslyMaxHours[tc].at(pos2) >= samc->maxHoursContinuously
                             && teachersActivityTagMaxHoursContinuouslyPercentage[tc].at(pos2) <= samc->weightPercentage){

                                teachersActivityTagMaxHoursContinuouslyActivityTag[tc][pos2]=samc->activityTagIndex;
                                teachersActivityTagMaxHoursContinuouslyMaxHours[tc][pos2]=samc->maxHoursContinuously;
                                teachersActivityTagMaxHoursContinuouslyPercentage[tc][pos2]=samc->weightPercentage;
                            }
                            else{
                                ok=false;

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are too many constraints"
                                 " of type activity tag max hours continuously relating to him, which cannot be compressed in 2 constraints of this type."
                                 " Two constraints max hours can be compressed into a single one if the max hours are lower"
                                 " in the first one and the weight percentage is higher on the first one."
                                 " It is possible to use any number of such constraints for a teacher, but their resultant must"
                                 " be maximum 2 constraints of type activity tag max hours continuously.\n\n"
                                 " Example: you are allowed to use 3 constraints: 6 hours 95%, 7 hours 100% and 8 hours 100%,"
                                 " which can be compressed into 2 constraints: 6 hours 95%, 7 hours 100%\n\n"
                                 " Please modify your data accordingly and try again.")
                                 .arg(Timetable::getInstance()->instance.teachersList[tc]->name),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return false;
                            }
                        }
                    }
                }
            }
        }
    }

    return ok;
}
*/
//must be after n hours per teacher
bool computeTeachersMinHoursDaily(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMinHoursDailyMinHours[i]=-1;
        teachersMinHoursDailyPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MIN_HOURS_DAILY){
            ConstraintTeacherMinHoursDaily* tmd=(ConstraintTeacherMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min hours daily for teacher %1 with"
                 " weight (percentage) below 100. Starting with m-FET version 5.4.0 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(tmd->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            //////////
            if(tmd->minHoursDaily>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min hours daily for teacher %1 with"
                 " %2 min hours daily, and the number of working hours per day is only %3. Please correct and try again")
                 .arg(tmd->teacherName)
                 .arg(tmd->minHoursDaily)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            if(teachersMinHoursDailyMinHours[tmd->teacher_ID]==-1 || teachersMinHoursDailyMinHours[tmd->teacher_ID]<tmd->minHoursDaily){
                teachersMinHoursDailyMinHours[tmd->teacher_ID]=tmd->minHoursDaily;
                teachersMinHoursDailyPercentages[tmd->teacher_ID]=100;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MIN_HOURS_DAILY){
            ConstraintTeachersMinHoursDaily* tmd=(ConstraintTeachersMinHoursDaily*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min hours daily with"
                 " weight (percentage) below 100. Starting with m-FET version 5.4.0 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            //////////
            if(tmd->minHoursDaily>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min hours daily with"
                 " %1 min hours daily, and the number of working hours per day is only %2. Please correct and try again")
                 .arg(tmd->minHoursDaily)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersMinHoursDailyMinHours[tch]==-1 || teachersMinHoursDailyMinHours[tch]<tmd->minHoursDaily){
                    teachersMinHoursDailyMinHours[tch]=tmd->minHoursDaily;
                    teachersMinHoursDailyPercentages[tch]=100;
                }
            }
        }
    }

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(IS_EQUAL(teachersMinHoursDailyPercentages[tc], 100)){
            assert(teachersMinHoursDailyMinHours[tc]>=0);
            if(nHoursPerTeacher[tc]>0 && teachersMinHoursDailyMinHours[tc]>nHoursPerTeacher[tc]){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min %1 hours daily for teacher"
                 " %2 (the constraint allows empty days). This teacher has in total only %3 hours per week, so impossible constraint."
                 " Please correct and try again")
                 .arg(teachersMinHoursDailyMinHours[tc])
                 .arg(TContext::get()->instance.teachersList[tc]->name)
                 .arg(nHoursPerTeacher[tc])
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            if(teachersMinHoursDailyMinHours[tc]<2){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min %1 hours daily for teacher"
                 " %2 (the constraint allows empty days). The number of min hours daily should be at least 2, to make a non-trivial constraint. Please correct and try again")
                 .arg(teachersMinHoursDailyMinHours[tc])
                 .arg(TContext::get()->instance.teachersList[tc]->name)
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

//must be after min hours for teachers
bool computeTeachersMinDaysPerWeek(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMinDaysPerWeekMinDays[i]=-1;
        teachersMinDaysPerWeekPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK){
            ConstraintTeacherMinDaysPerWeek* tmd=(ConstraintTeacherMinDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min days per week for teacher %1 with"
                 " weight (percentage) below 100. Please make weight 100% and try again")
                 .arg(tmd->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            //////////
            if(tmd->minDaysPerWeek>TContext::get()->instance.nDaysPerWeek){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min days per week for teacher %1 with"
                 " %2 min days per week, and the number of working days per week is only %3. Please correct and try again")
                 .arg(tmd->teacherName)
                 .arg(tmd->minDaysPerWeek)
                 .arg(TContext::get()->instance.nDaysPerWeek),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            if(teachersMinDaysPerWeekMinDays[tmd->teacher_ID]==-1 || teachersMinDaysPerWeekMinDays[tmd->teacher_ID]<tmd->minDaysPerWeek){
                teachersMinDaysPerWeekMinDays[tmd->teacher_ID]=tmd->minDaysPerWeek;
                teachersMinDaysPerWeekPercentages[tmd->teacher_ID]=100;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MIN_DAYS_PER_WEEK){
            ConstraintTeachersMinDaysPerWeek* tmd=(ConstraintTeachersMinDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmd->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min days per week with weight"
                 " (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            //////////
            if(tmd->minDaysPerWeek>TContext::get()->instance.nDaysPerWeek){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min days per week with"
                 " %1 min days per week, and the number of working days per week is only %2. Please correct and try again")
                 .arg(tmd->minDaysPerWeek)
                 .arg(TContext::get()->instance.nDaysPerWeek),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersMinDaysPerWeekMinDays[tch]==-1 || teachersMinDaysPerWeekMinDays[tch]<tmd->minDaysPerWeek){
                    teachersMinDaysPerWeekMinDays[tch]=tmd->minDaysPerWeek;
                    teachersMinDaysPerWeekPercentages[tch]=100;
                }
            }
        }
    }

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(teachersMinDaysPerWeekMinDays[tc]>=0){
            int md=teachersMinDaysPerWeekMinDays[tc];
            //TODO:
//            if(md>Timetable::getInstance()->instance.teachersList[tc]->activitiesForTeacher.count()){
//                ok=false;

//                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
//                 GeneratePreTranslate::tr("Cannot optimize because for teacher %1 you have min days per week %2 and he has only %3 activities - impossible."
//                 " Please correct and try again.")
//                 .arg(Timetable::getInstance()->instance.teachersList[tc]->name)
//                 .arg(md)
//                 .arg(Timetable::getInstance()->instance.teachersList[tc]->activitiesForTeacher.count())
//                 ,
//                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
//                 1, 0 );

//                if(t==0)
//                    return false;
//            }

            if(teachersMinHoursDailyMinHours[tc]>=0){
                int mh=teachersMinHoursDailyMinHours[tc];

                if(md*mh>nHoursPerTeacher[tc]){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize because for teacher %1 you have min days per week %2 and min hours daily %3"
                     " and he has only %4 working hours - impossible. Please correct and try again.")
                     .arg(TContext::get()->instance.teachersList[tc]->name)
                     .arg(md)
                     .arg(mh)
                     .arg(nHoursPerTeacher[tc])
                     ,
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
    }


    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        if(teachersMinDaysPerWeekMinDays[tc]>=0){
            if(teachersMaxDaysPerWeekMaxDays[tc]>=0){
                if(teachersMaxDaysPerWeekMaxDays[tc]<teachersMinDaysPerWeekMinDays[tc]){
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize because for teacher %1 you have min days per week %2 > max days per week %3"
                     " - impossible (min days must be <= max days). Please correct and try again.")
                     .arg(TContext::get()->instance.teachersList[tc]->name)
                     .arg(teachersMinDaysPerWeekMinDays[tc])
                     .arg(teachersMaxDaysPerWeekMaxDays[tc])
                     ,
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }

            int med=1; //minimum each day = 1 hour
            if(teachersMinHoursDailyMinHours[tc]>=0)
                med=teachersMinHoursDailyMinHours[tc];

            int navdays=0;

            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
                int navhours=0;
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                    if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h])
                        navhours++;
                if(navhours>=med)
                    navdays++;
            }

            if(navdays<teachersMinDaysPerWeekMinDays[tc]){
                ok=false;

                QString s;

                if(teachersMinHoursDailyMinHours[tc]>=0){
                    s=GeneratePreTranslate::tr("Cannot optimize because for teacher %1 you have min days per week %2 and only %3"
                     " available days considering breaks and not available and min hours daily for this teacher. Please correct and try again.")
                     .arg(TContext::get()->instance.teachersList[tc]->name)
                     .arg(teachersMinDaysPerWeekMinDays[tc])
                     .arg(navdays);
                }
                else{
                    s=GeneratePreTranslate::tr("Cannot optimize because for teacher %1 you have min days per week %2 and only %3"
                     " available days considering breaks and not available for this teacher. Please correct and try again.")
                     .arg(TContext::get()->instance.teachersList[tc]->name)
                     .arg(teachersMinDaysPerWeekMinDays[tc])
                     .arg(navdays);
                }

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }


    return ok;
}

bool computeTeachersMinRestingHours(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersMinRestingHoursCircularMinHours[i]=-1;
        teachersMinRestingHoursCircularPercentages[i]=-1;
        teachersMinRestingHoursNotCircularMinHours[i]=-1;
        teachersMinRestingHoursNotCircularPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MIN_RESTING_HOURS){
            ConstraintTeacherMinRestingHours* tmrh=(ConstraintTeacherMinRestingHours*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmrh->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min resting hours for teacher %1 with"
                 " weight (percentage) below 100. It is only possible to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(tmrh->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            if(tmrh->minRestingHours>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher min resting hours for teacher %1 with"
                 " %2 min resting hours, and the number of working hours per day is only %3. Please correct and try again")
                 .arg(tmrh->teacherName)
                 .arg(tmrh->minRestingHours)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            if(tmrh->circular==true){
                if(teachersMinRestingHoursCircularMinHours[tmrh->teacher_ID]==-1 ||
                 (teachersMinRestingHoursCircularMinHours[tmrh->teacher_ID]>=0 && teachersMinRestingHoursCircularMinHours[tmrh->teacher_ID]<tmrh->minRestingHours)){
                    teachersMinRestingHoursCircularMinHours[tmrh->teacher_ID]=tmrh->minRestingHours;
                    teachersMinRestingHoursCircularPercentages[tmrh->teacher_ID]=100;
                }
            }
            else{
                if(teachersMinRestingHoursNotCircularMinHours[tmrh->teacher_ID]==-1 ||
                 (teachersMinRestingHoursNotCircularMinHours[tmrh->teacher_ID]>=0 && teachersMinRestingHoursNotCircularMinHours[tmrh->teacher_ID]<tmrh->minRestingHours)){
                    teachersMinRestingHoursNotCircularMinHours[tmrh->teacher_ID]=tmrh->minRestingHours;
                    teachersMinRestingHoursNotCircularPercentages[tmrh->teacher_ID]=100;
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MIN_RESTING_HOURS){
            ConstraintTeachersMinRestingHours* tmrh=(ConstraintTeachersMinRestingHours*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(tmrh->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min resting hours with"
                 " weight (percentage) below 100. It is only possible to use 100% weight for such constraints. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            if(tmrh->minRestingHours>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers min resting hours with"
                 " %1 min resting hours, and the number of working hours per day is only %2. Please correct and try again")
                 .arg(tmrh->minRestingHours)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(tmrh->circular==true){
                    if(teachersMinRestingHoursCircularMinHours[tch]==-1 ||
                     (teachersMinRestingHoursCircularMinHours[tch]>=0 && teachersMinRestingHoursCircularMinHours[tch]<tmrh->minRestingHours)){
                        teachersMinRestingHoursCircularMinHours[tch]=tmrh->minRestingHours;
                        teachersMinRestingHoursCircularPercentages[tch]=100;
                    }
                }
                else{
                    if(teachersMinRestingHoursNotCircularMinHours[tch]==-1 ||
                     (teachersMinRestingHoursNotCircularMinHours[tch]>=0 && teachersMinRestingHoursNotCircularMinHours[tch]<tmrh->minRestingHours)){
                        teachersMinRestingHoursNotCircularMinHours[tch]=tmrh->minRestingHours;
                        teachersMinRestingHoursNotCircularPercentages[tch]=100;
                    }
                }
            }
        }
    }

    //small possible speedup
    for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
        if(teachersMinRestingHoursCircularMinHours[tch]>=0 && teachersMinRestingHoursNotCircularMinHours[tch]>=0 &&
         teachersMinRestingHoursCircularMinHours[tch]>=teachersMinRestingHoursNotCircularMinHours[tch]){
            teachersMinRestingHoursNotCircularMinHours[tch]=-1;
            teachersMinRestingHoursNotCircularPercentages[tch]=-1;
        }
    }

    return ok;
}

bool computeSubgroupsMinRestingHours(QWidget* parent)
{
    bool ok=true;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMinRestingHoursCircularMinHours[i]=-1;
        subgroupsMinRestingHoursCircularPercentages[i]=-1;
        subgroupsMinRestingHoursNotCircularMinHours[i]=-1;
        subgroupsMinRestingHoursNotCircularPercentages[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS){
            ConstraintStudentsSetMinRestingHours* smrh=(ConstraintStudentsSetMinRestingHours*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(smrh->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set min resting hours for students set %1 with"
                 " weight (percentage) below 100. It is only possible to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(smrh->students),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            if(smrh->minRestingHours>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set min resting hours for students set %1 with"
                 " %2 min resting hours, and the number of working hours per day is only %3. Please correct and try again")
                 .arg(smrh->students)
                 .arg(smrh->minRestingHours)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            foreach(int sbg, smrh->iSubgroupsList){
                if(smrh->circular==true){
                    if(subgroupsMinRestingHoursCircularMinHours[sbg]==-1 ||
                     (subgroupsMinRestingHoursCircularMinHours[sbg]>=0 && subgroupsMinRestingHoursCircularMinHours[sbg]<smrh->minRestingHours)){
                        subgroupsMinRestingHoursCircularMinHours[sbg]=smrh->minRestingHours;
                        subgroupsMinRestingHoursCircularPercentages[sbg]=100;
                    }
                }
                else{
                    if(subgroupsMinRestingHoursNotCircularMinHours[sbg]==-1 ||
                     (subgroupsMinRestingHoursNotCircularMinHours[sbg]>=0 && subgroupsMinRestingHoursNotCircularMinHours[sbg]<smrh->minRestingHours)){
                        subgroupsMinRestingHoursNotCircularMinHours[sbg]=smrh->minRestingHours;
                        subgroupsMinRestingHoursNotCircularPercentages[sbg]=100;
                    }
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MIN_RESTING_HOURS){
            ConstraintStudentsMinRestingHours* smrh=(ConstraintStudentsMinRestingHours*)TContext::get()->instance.validTimeConstraintsList[i];

            //////////
            if(smrh->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students min resting hours with"
                 " weight (percentage) below 100. It is only possible to use 100% weight for such constraints. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////
            if(smrh->minRestingHours>TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students min resting hours with"
                 " %1 min resting hours, and the number of working hours per day is only %2. Please correct and try again")
                 .arg(smrh->minRestingHours)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
            //////////

            for(int sbg=0; sbg<TContext::get()->instance.directSubgroupsList.size(); sbg++){
                if(smrh->circular==true){
                    if(subgroupsMinRestingHoursCircularMinHours[sbg]==-1 ||
                     (subgroupsMinRestingHoursCircularMinHours[sbg]>=0 && subgroupsMinRestingHoursCircularMinHours[sbg]<smrh->minRestingHours)){
                        subgroupsMinRestingHoursCircularMinHours[sbg]=smrh->minRestingHours;
                        subgroupsMinRestingHoursCircularPercentages[sbg]=100;
                    }
                }
                else{
                    if(subgroupsMinRestingHoursNotCircularMinHours[sbg]==-1 ||
                     (subgroupsMinRestingHoursNotCircularMinHours[sbg]>=0 && subgroupsMinRestingHoursNotCircularMinHours[sbg]<smrh->minRestingHours)){
                        subgroupsMinRestingHoursNotCircularMinHours[sbg]=smrh->minRestingHours;
                        subgroupsMinRestingHoursNotCircularPercentages[sbg]=100;
                    }
                }
            }
        }
    }

    //small possible speedup
    for(int sbg=0; sbg<TContext::get()->instance.directSubgroupsList.size(); sbg++){
        if(subgroupsMinRestingHoursCircularMinHours[sbg]>=0 && subgroupsMinRestingHoursNotCircularMinHours[sbg]>=0 &&
         subgroupsMinRestingHoursCircularMinHours[sbg]>=subgroupsMinRestingHoursNotCircularMinHours[sbg]){
            subgroupsMinRestingHoursNotCircularMinHours[sbg]=-1;
            subgroupsMinRestingHoursNotCircularPercentages[sbg]=-1;
        }
    }

    return ok;
}

void computeActivitiesNotOverlapping()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        activitiesNotOverlappingActivities[i].clear();
        activitiesNotOverlappingPercentages[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING){
            ConstraintActivitiesNotOverlapping* no=(ConstraintActivitiesNotOverlapping*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<no->_n_activities; j++){
                int ai1=no->_activities[j];
                for(int k=0; k<no->_n_activities; k++){
                    int ai2=no->_activities[k];
                    if(ai1!=ai2){
                        int t=activitiesNotOverlappingActivities[ai1].indexOf(ai2);
                        if(t>=0){
                            if(activitiesNotOverlappingPercentages[ai1].at(t) < no->weightPercentage){
                                activitiesNotOverlappingPercentages[ai1][t]=no->weightPercentage;
                            }
                        }
                        else{
                            activitiesNotOverlappingPercentages[ai1].append(no->weightPercentage);
                            activitiesNotOverlappingActivities[ai1].append(ai2);
                        }
                    }
                }
            }
        }
}

bool computeActivitiesSameStartingTime(QWidget* parent, QHash<int, int> & reprSameStartingTime, QHash<int, QSet<int> > & reprSameActivitiesSet)
{
    bool reportunder100=true;
    bool report100=true;

    bool oktocontinue=true;

    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        activitiesSameStartingTimeActivities[i].clear();
        activitiesSameStartingTimePercentages[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME){
            ConstraintActivitiesSameStartingTime* sst=(ConstraintActivitiesSameStartingTime*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<sst->_n_activities; j++){
                int ai1=sst->_activities[j];
                for(int k=j+1; k<sst->_n_activities; k++){
                    int ai2=sst->_activities[k];

                    if(sst->weightPercentage==100.0 && activitiesConflictingPercentage[ai1].value(ai2, -1)==100)
                        oktocontinue=false;

                    if(sst->weightPercentage<100.0 && reportunder100 && activitiesConflictingPercentage[ai1].value(ai2, -1)==100){
                        QString s;

                        s+=sst->getDetailedDescription();
                        s+="\n";
                        s+=GeneratePreTranslate::tr("The constraint is impossible to respect, because there are the activities with id-s %1 and %2 which "
                         "conflict one with another, because they have common students sets or teachers or must be not overlapping. m-FET will allow you to continue, "
                         "because the weight of this constraint is below 100.0%, "
                         "but anyway most probably you have made a mistake in this constraint, "
                         "so it is recommended to modify it.")
                         .arg(TContext::get()->instance.activeActivitiesList[ai1]->id)
                         .arg(TContext::get()->instance.activeActivitiesList[ai2]->id);

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         s, GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            reportunder100=false;
                    }
                    else if(sst->weightPercentage==100.0 && report100 && activitiesConflictingPercentage[ai1].value(ai2, -1)==100){
                        QString s;

                        s+=sst->getDetailedDescription();
                        s+="\n";
                        s+=GeneratePreTranslate::tr("The constraint is impossible to respect, because there are the activities with id-s %1 and %2 which "
                         "conflict one with another, because they have common students sets or teachers or must be not overlapping. The weight of this constraint is 100.0%, "
                         "so your timetable is impossible. Please correct this constraint.")
                         .arg(TContext::get()->instance.activeActivitiesList[ai1]->id)
                         .arg(TContext::get()->instance.activeActivitiesList[ai2]->id);

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         s, GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            report100=false;
                    }
                }
            }

            for(int j=0; j<sst->_n_activities; j++){
                int ai1=sst->_activities[j];
                for(int k=0; k<sst->_n_activities; k++){
                    int ai2=sst->_activities[k];
                    if(ai1!=ai2){
                        int t=activitiesSameStartingTimeActivities[ai1].indexOf(ai2);
                        double perc=-1;
                        if(t!=-1){
                            perc=activitiesSameStartingTimePercentages[ai1].at(t);
                            assert(perc>=0 && perc<=100);
                        }

                        if(t==-1 /*|| perc<sst->weightPercentage*/){
                            activitiesSameStartingTimeActivities[ai1].append(ai2);
                            activitiesSameStartingTimePercentages[ai1].append(sst->weightPercentage);
                            assert(activitiesSameStartingTimeActivities[ai1].count()==activitiesSameStartingTimePercentages[ai1].count());
                        }
                        else if(t>=0 && perc<sst->weightPercentage){
                            activitiesSameStartingTimePercentages[ai1][t]=sst->weightPercentage;
                        }
                    }
                }
            }
        }


    //added June 2009, m-FET-5.10.0
    bool reportIndirect=true;

    QMultiHash<int, int> adjMatrix;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME
         &&TContext::get()->instance.validTimeConstraintsList[i]->weightPercentage==100.0){
            ConstraintActivitiesSameStartingTime* sst=(ConstraintActivitiesSameStartingTime*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int i=1; i<sst->_n_activities; i++){
                adjMatrix.insert(sst->_activities[0], sst->_activities[i]);
                adjMatrix.insert(sst->_activities[i], sst->_activities[0]);
            }
        }
    }

    QHash<int, int> repr;
    //repr.clear();

    QQueue<int> queue;

    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        int start=i;

        if(repr.value(start, -1)==-1){ //not visited
            repr.insert(start, start);
            queue.enqueue(start);
            while(!queue.isEmpty()){
                int crtHead=queue.dequeue();
                assert(repr.value(crtHead, -1)==start);
                QList<int> neighList=adjMatrix.values(crtHead);
                foreach(int neigh, neighList){
                    if(repr.value(neigh, -1)==-1){
                        queue.enqueue(neigh);
                        repr.insert(neigh, start);
                    }
                    else{
                        assert(repr.value(neigh, -1)==start);
                    }
                }
            }
        }
    }

    //faster than below
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        QHash<int, int> &hashConfl=activitiesConflictingPercentage[i];

        QHashIterator<int, int> it(hashConfl);
        while(it.hasNext()){
            it.next();
            //cout<<it.key()<<": "<<it.value()<<endl;
            int j=it.key();
            if(i!=j){
                if(it.value()==100){
                    if(repr.value(i)==repr.value(j)){
                        oktocontinue=false;

                        if(reportIndirect){
                            QString s=GeneratePreTranslate::tr("You have a set of impossible constraints activities same starting time, considering all the indirect links between"
                             " activities same starting time constraints");
                            s+="\n\n";
                            s+=GeneratePreTranslate::tr("The activities with ids %1 and %2 must be simultaneous (request determined indirectly), but they have common teachers and/or students sets or must be not overlapping")
                             .arg(TContext::get()->instance.activeActivitiesList[i]->id).arg(TContext::get()->instance.activeActivitiesList[j]->id);

                            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                             s, GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                             1, 0 );

                            if(t==0)
                                reportIndirect=false;
                        }
                    }
                }
            }
        }
    }

/*
    for(int i=0; i<Timetable::getInstance()->instance.activeActivitiesList.size(); i++)
        for(int j=i+1; j<Timetable::getInstance()->instance.activeActivitiesList.size(); j++)
            if(repr.value(i) == repr.value(j)){
                if(activitiesConflictingPercentage[i].value(j, -1)==100){
                    oktocontinue=false;

                    if(reportIndirect){
                        QString s=GeneratePreTranslate::tr("You have a set of impossible constraints activities same starting time, considering all the indirect links between"
                         " activities same starting time constraints");
                        s+="\n\n";
                        s+=GeneratePreTranslate::tr("The activities with ids %1 and %2 must be simultaneous (request determined indirectly), but they have common teachers and/or students sets or must be not overlapping")
                         .arg(Timetable::getInstance()->instance.activeActivitiesList[i]->id).arg(Timetable::getInstance()->instance.activeActivitiesList[j]->id);

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                         s, GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            reportIndirect=false;
                    }
                }
            }*/
    ///////////end added 5.10.0, June 2009

    QHash<int, QSet<int> > hashSet;

    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        assert(repr.contains(i));
        int r=repr.value(i);
        hashSet[r].insert(i); //faster
        /*QSet<int> s;
        if(hashSet.contains(r))
            s=hashSet.value(r);
        s.insert(i);
        hashSet.insert(r, s);*/
    }

    reprSameStartingTime=repr;
    reprSameActivitiesSet=hashSet;

    return oktocontinue;
}

void computeActivitiesSameStartingHour()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        activitiesSameStartingHourActivities[i].clear();
        activitiesSameStartingHourPercentages[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR){
            ConstraintActivitiesSameStartingHour* sst=(ConstraintActivitiesSameStartingHour*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<sst->_n_activities; j++){
                int ai1=sst->_activities[j];
                for(int k=0; k<sst->_n_activities; k++){
                    int ai2=sst->_activities[k];
                    if(ai1!=ai2){
                        int t=activitiesSameStartingHourActivities[ai1].indexOf(ai2);
                        double perc=-1;
                        if(t!=-1){
                            perc=activitiesSameStartingHourPercentages[ai1].at(t);
                            assert(perc>=0 && perc<=100);
                        }

                        if(t==-1 /*|| perc<sst->weightPercentage*/){
                            activitiesSameStartingHourActivities[ai1].append(ai2);
                            activitiesSameStartingHourPercentages[ai1].append(sst->weightPercentage);
                            assert(activitiesSameStartingHourActivities[ai1].count()==activitiesSameStartingHourPercentages[ai1].count());
                        }
                        else if(t>=0 && perc<sst->weightPercentage){
                            activitiesSameStartingHourPercentages[ai1][t]=sst->weightPercentage;
                        }
                    }
                }
            }
        }
}

void computeActivitiesSameStartingDay()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        activitiesSameStartingDayActivities[i].clear();
        activitiesSameStartingDayPercentages[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY){
            ConstraintActivitiesSameStartingDay* sst=(ConstraintActivitiesSameStartingDay*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<sst->_n_activities; j++){
                int ai1=sst->_activities[j];
                for(int k=0; k<sst->_n_activities; k++){
                    int ai2=sst->_activities[k];
                    if(ai1!=ai2){
                        int t=activitiesSameStartingDayActivities[ai1].indexOf(ai2);
                        double perc=-1;
                        if(t!=-1){
                            perc=activitiesSameStartingDayPercentages[ai1].at(t);
                            assert(perc>=0 && perc<=100);
                        }

                        if(t==-1 /*|| perc<sst->weightPercentage*/){
                            activitiesSameStartingDayActivities[ai1].append(ai2);
                            activitiesSameStartingDayPercentages[ai1].append(sst->weightPercentage);
                            assert(activitiesSameStartingDayActivities[ai1].count()==activitiesSameStartingDayPercentages[ai1].count());
                        }
                        else if(t>=0 && perc<sst->weightPercentage){
                            activitiesSameStartingDayPercentages[ai1][t]=sst->weightPercentage;
                        }
                    }
                }
            }
        }
}

////////////teachers' max gaps
//important also for other purposes
bool computeNHoursPerTeacher(QWidget* parent)
{
    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++)
        nHoursPerTeacher[i]=0;
    //TODO:
//    for(int i=0; i<Timetable::getInstance()->instance.activeActivitiesList.size(); i++){
//        Activity* act=Timetable::getInstance()->instance.activeActivitiesList[i];
//        for(int j=0; j<act->iTeachersList.count(); j++){
//            int tc=act->iTeachersList.at(j);
//            nHoursPerTeacher[tc]+=act->duration;
//        }
//    }

    bool ok=true;

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++)
        if(nHoursPerTeacher[i]>TContext::get()->instance.nHoursPerWeek){
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for teacher %1, because the number of hours for teacher is %2"
              " and you have only %3 days x %4 hours in a week.")
             .arg(TContext::get()->instance.teachersList[i]->name)
             .arg(nHoursPerTeacher[i])
             .arg(TContext::get()->instance.nDaysPerWeek)
             .arg(TContext::get()->instance.nHoursPerDay),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return ok;
        }

    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        int freeSlots=0;
        for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
            for(int k=0; k<TContext::get()->instance.nHoursPerDay; k++)
                if(!TContext::get()->instance.teacherNotAvailableDayHour[i][j][k] && !TContext::get()->instance.breakDayHour[j][k])
                    freeSlots++;
        if(nHoursPerTeacher[i]>freeSlots){
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for teacher %1, because the number of hours for teacher is %2"
              " and you have only %3 free slots from constraints teacher not available and/or break. Maybe you inputted wrong constraints teacher"
              " not available or break or the number of hours per week is less because of a misunderstanding")
             .arg(TContext::get()->instance.teachersList[i]->name)
             .arg(nHoursPerTeacher[i])
             .arg(freeSlots),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return ok;
        }
    }

    //n days per week has 100% weight
    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++)
        if(teachersMaxDaysPerWeekMaxDays[i]>=0){
            int nd=teachersMaxDaysPerWeekMaxDays[i];
            if(nHoursPerTeacher[i] > nd*TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because the number of hours for teacher is %2"
                  " and you have only %3 allowed days from constraint teacher max days per week x %4 hours in a day."
                  " Probably there is an error in your data")
                 .arg(TContext::get()->instance.teachersList[i]->name)
                 .arg(nHoursPerTeacher[i])
                 .arg(nd)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return ok;
            }
        }

    //n days per week has 100% weight
    //check n days per week together with not available and breaks
    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            nAllowedSlotsPerDay[d]=0;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h])
                    nAllowedSlotsPerDay[d]++;
        }

        int dayAvailable[MAX_DAYS_PER_WEEK];
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
            dayAvailable[d]=1;
        if(teachersMaxDaysPerWeekMaxDays[tc]>=0){
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=0;

            assert(teachersMaxDaysPerWeekMaxDays[tc]<=TContext::get()->instance.nDaysPerWeek);
            for(int k=0; k<teachersMaxDaysPerWeekMaxDays[tc]; k++){
                int maxPos=-1, maxVal=-1;
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    if(dayAvailable[d]==0)
                        if(maxVal<nAllowedSlotsPerDay[d]){
                            maxVal=nAllowedSlotsPerDay[d];
                            maxPos=d;
                        }
                assert(maxPos>=0);
                assert(dayAvailable[maxPos]==0);
                dayAvailable[maxPos]=1;
            }
        }

        int total=0;
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
            if(dayAvailable[d]==1)
                total+=nAllowedSlotsPerDay[d];
        if(total<nHoursPerTeacher[tc]){
            ok=false;

            QString s;
            s=GeneratePreTranslate::tr("Cannot optimize for teacher %1, because of too constrained"
             " teacher max days per week, teacher not available and/or breaks."
             " The number of total hours for this teacher is"
             " %2 and the number of available slots is, considering max days per week and all other constraints, %3.")
             .arg(TContext::get()->instance.teachersList[tc]->name)
             .arg(nHoursPerTeacher[tc])
             .arg(total);
            s+="\n\n";
            s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return false;
        }
    }

    return ok;
}

bool computeTeachersMaxGapsPerWeekPercentage(QWidget* parent)
{
    for(int j=0; j<TContext::get()->instance.teachersList.size(); j++){
        teachersMaxGapsPerWeekMaxGaps[j]=-1;
        teachersMaxGapsPerWeekPercentage[j]=-1;
    }

    bool ok=true;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_GAPS_PER_WEEK){
            ConstraintTeachersMaxGapsPerWeek* tg=(ConstraintTeachersMaxGapsPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tg->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers max gaps per week with"
                 " weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }

        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK){
            ConstraintTeacherMaxGapsPerWeek* tg=(ConstraintTeacherMaxGapsPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tg->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher max gaps per week with"
                 " weight (percentage) below 100 for teacher %1. Please make weight 100% and try again")
                 .arg(tg->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_GAPS_PER_WEEK){
            ConstraintTeachersMaxGapsPerWeek* tg=(ConstraintTeachersMaxGapsPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<TContext::get()->instance.teachersList.size(); j++){
                if(teachersMaxGapsPerWeekMaxGaps[j]==-1
                 ||(teachersMaxGapsPerWeekMaxGaps[j]>=0 && teachersMaxGapsPerWeekMaxGaps[j]>=tg->maxGaps && teachersMaxGapsPerWeekPercentage[j]<=tg->weightPercentage)){
                    teachersMaxGapsPerWeekMaxGaps[j]=tg->maxGaps;
                    teachersMaxGapsPerWeekPercentage[j]=tg->weightPercentage;
                }
                else if(teachersMaxGapsPerWeekMaxGaps[j]>=0 && teachersMaxGapsPerWeekMaxGaps[j]<=tg->maxGaps && teachersMaxGapsPerWeekPercentage[j]>=tg->weightPercentage){
                    //do nothing
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are two constraints"
                     " of type max gaps per week relating to him, and the weight percentage is higher on the constraint"
                     " with more gaps allowed. You are allowed only to have for each teacher"
                     " the most important constraint with maximum weight percentage and minimum gaps allowed"
                     ". Please modify your data accordingly and try again")
                     .arg(TContext::get()->instance.teachersList[j]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK){
            ConstraintTeacherMaxGapsPerWeek* tg=(ConstraintTeacherMaxGapsPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            int j=tg->teacherIndex;
            if(teachersMaxGapsPerWeekMaxGaps[j]==-1
             ||(teachersMaxGapsPerWeekMaxGaps[j]>=0 && teachersMaxGapsPerWeekMaxGaps[j]>=tg->maxGaps && teachersMaxGapsPerWeekPercentage[j]<=tg->weightPercentage)){
                teachersMaxGapsPerWeekMaxGaps[j]=tg->maxGaps;
                teachersMaxGapsPerWeekPercentage[j]=tg->weightPercentage;
            }
            else if(teachersMaxGapsPerWeekMaxGaps[j]>=0 && teachersMaxGapsPerWeekMaxGaps[j]<=tg->maxGaps && teachersMaxGapsPerWeekPercentage[j]>=tg->weightPercentage){
                //do nothing
            }
            else{
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are two constraints"
                 " of type max gaps per week relating to him, and the weight percentage is higher on the constraint"
                 " with more gaps allowed. You are allowed only to have for each teacher"
                 " the most important constraint with maximum weight percentage and minimum gaps allowed"
                 ". Please modify your data accordingly and try again")
                 .arg(TContext::get()->instance.teachersList[j]->name),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}

bool computeTeachersMaxGapsPerDayPercentage(QWidget* parent)
{
    for(int j=0; j<TContext::get()->instance.teachersList.size(); j++){
        teachersMaxGapsPerDayMaxGaps[j]=-1;
        teachersMaxGapsPerDayPercentage[j]=-1;
    }

    bool ok=true;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_GAPS_PER_DAY){
            ConstraintTeachersMaxGapsPerDay* tg=(ConstraintTeachersMaxGapsPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tg->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers max gaps per day with"
                 " weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }

        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY){
            ConstraintTeacherMaxGapsPerDay* tg=(ConstraintTeacherMaxGapsPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tg->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher max gaps per day with"
                 " weight (percentage) below 100 for teacher %1. Please make weight 100% and try again")
                 .arg(tg->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_GAPS_PER_DAY){
            ConstraintTeachersMaxGapsPerDay* tg=(ConstraintTeachersMaxGapsPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<TContext::get()->instance.teachersList.size(); j++){
                if(teachersMaxGapsPerDayMaxGaps[j]==-1
                 ||(teachersMaxGapsPerDayMaxGaps[j]>=0 && teachersMaxGapsPerDayMaxGaps[j]>=tg->maxGaps && teachersMaxGapsPerDayPercentage[j]<=tg->weightPercentage)){
                    teachersMaxGapsPerDayMaxGaps[j]=tg->maxGaps;
                    teachersMaxGapsPerDayPercentage[j]=tg->weightPercentage;
                }
                else if(teachersMaxGapsPerDayMaxGaps[j]>=0 && teachersMaxGapsPerDayMaxGaps[j]<=tg->maxGaps && teachersMaxGapsPerDayPercentage[j]>=tg->weightPercentage){
                    //do nothing
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are two constraints"
                     " of type max gaps per day relating to him, and the weight percentage is higher on the constraint"
                     " with more gaps allowed. You are allowed only to have for each teacher"
                     " the most important constraint with maximum weight percentage and minimum gaps allowed"
                     ". Please modify your data accordingly and try again")
                     .arg(TContext::get()->instance.teachersList[j]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY){
            ConstraintTeacherMaxGapsPerDay* tg=(ConstraintTeacherMaxGapsPerDay*)TContext::get()->instance.validTimeConstraintsList[i];

            int j=tg->teacherIndex;
            if(teachersMaxGapsPerDayMaxGaps[j]==-1
             ||(teachersMaxGapsPerDayMaxGaps[j]>=0 && teachersMaxGapsPerDayMaxGaps[j]>=tg->maxGaps && teachersMaxGapsPerDayPercentage[j]<=tg->weightPercentage)){
                teachersMaxGapsPerDayMaxGaps[j]=tg->maxGaps;
                teachersMaxGapsPerDayPercentage[j]=tg->weightPercentage;
            }
            else if(teachersMaxGapsPerDayMaxGaps[j]>=0 && teachersMaxGapsPerDayMaxGaps[j]<=tg->maxGaps && teachersMaxGapsPerDayPercentage[j]>=tg->weightPercentage){
                //do nothing
            }
            else{
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because there are two constraints"
                 " of type max gaps per day relating to him, and the weight percentage is higher on the constraint"
                 " with more gaps allowed. You are allowed only to have for each teacher"
                 " the most important constraint with maximum weight percentage and minimum gaps allowed"
                 ". Please modify your data accordingly and try again")
                 .arg(TContext::get()->instance.teachersList[j]->name),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
    }

    return ok;
}
/////////////////


///////students' max gaps and early (part 1)
//important also for other purposes
bool computeNHoursPerSubgroup(QWidget* parent)
{
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        nHoursPerSubgroup[i]=0;
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activeActivitiesList[i];
        for(int j=0; j<act->iSubgroupsList.count(); j++){
            int sb=act->iSubgroupsList.at(j);
            nHoursPerSubgroup[sb]+=act->duration;
        }
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        if(nHoursPerSubgroup[i]>TContext::get()->instance.nHoursPerWeek){
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because the number of hours for subgroup is %2"
              " and you have only %3 days x %4 hours in a week.")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name)
             .arg(nHoursPerSubgroup[i])
             .arg(TContext::get()->instance.nDaysPerWeek)
             .arg(TContext::get()->instance.nHoursPerDay),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return ok;
        }

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        int freeSlots=0;
        for(int j=0; j<TContext::get()->instance.nDaysPerWeek; j++)
            for(int k=0; k<TContext::get()->instance.nHoursPerDay; k++)
                if(!TContext::get()->instance.subgroupNotAvailableDayHour[i][j][k] && !TContext::get()->instance.breakDayHour[j][k])
                    freeSlots++;
        if(nHoursPerSubgroup[i]>freeSlots){
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because the number of hours for subgroup is %2"
              " and you have only %3 free slots from constraints students set not available and/or break. Maybe you inputted wrong constraints students set"
              " not available or break or the number of hours per week is less because of a misunderstanding")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name)
             .arg(nHoursPerSubgroup[i])
             .arg(freeSlots),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return ok;
        }
    }

    //n days per week has 100% weight
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++)
        if(subgroupsMaxDaysPerWeekMaxDays[i]>=0){
            int nd=subgroupsMaxDaysPerWeekMaxDays[i];
            if(nHoursPerSubgroup[i] > nd*TContext::get()->instance.nHoursPerDay){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because the number of hours for subgroup is %2"
                  " and you have only %3 allowed days from constraint students (set) max days per week x %4 hours in a day."
                  " Probably there is an error in your data")
                 .arg(TContext::get()->instance.directSubgroupsList[i]->name)
                 .arg(nHoursPerSubgroup[i])
                 .arg(nd)
                 .arg(TContext::get()->instance.nHoursPerDay),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return ok;
            }
        }

    //n days per week has 100% weight
    //check n days per week together with not available and breaks
    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        int nAllowedSlotsPerDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            nAllowedSlotsPerDay[d]=0;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h])
                    nAllowedSlotsPerDay[d]++;
        }

        int dayAvailable[MAX_DAYS_PER_WEEK];
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
            dayAvailable[d]=1;
        if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                dayAvailable[d]=0;

            assert(subgroupsMaxDaysPerWeekMaxDays[sb]<=TContext::get()->instance.nDaysPerWeek);
            for(int k=0; k<subgroupsMaxDaysPerWeekMaxDays[sb]; k++){
                int maxPos=-1, maxVal=-1;
                for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                    if(dayAvailable[d]==0)
                        if(maxVal<nAllowedSlotsPerDay[d]){
                            maxVal=nAllowedSlotsPerDay[d];
                            maxPos=d;
                        }
                assert(maxPos>=0);
                assert(dayAvailable[maxPos]==0);
                dayAvailable[maxPos]=1;
            }
        }

        int total=0;
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
            if(dayAvailable[d]==1)
                total+=nAllowedSlotsPerDay[d];
        if(total<nHoursPerSubgroup[sb]){
            ok=false;

            QString s;
            s=GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because of too constrained"
             " students (set) max days per week, students set not available and/or breaks."
             " The number of total hours for this subgroup is"
             " %2 and the number of available slots is, considering max days per week and all other constraints, %3.")
             .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
             .arg(nHoursPerSubgroup[sb])
             .arg(total);
            s+="\n\n";
            s+=GeneratePreTranslate::tr("Please modify your data accordingly and try again");

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return false;
        }
    }

    return ok;
}

bool computeMaxDaysPerWeekForTeachers(QWidget* parent)
{
    for(int j=0; j<TContext::get()->instance.teachersList.size(); j++){
        teachersMaxDaysPerWeekMaxDays[j]=-1;
        teachersMaxDaysPerWeekWeightPercentages[j]=-1;
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK){
            ConstraintTeacherMaxDaysPerWeek* tn=(ConstraintTeacherMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher max days per week with"
                 " weight (percentage) below 100 for teacher %1. Starting with m-FET version 5.2.17 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(tn->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            if(teachersMaxDaysPerWeekMaxDays[tn->teacher_ID]==-1 ||
             (teachersMaxDaysPerWeekMaxDays[tn->teacher_ID]>=0 && teachersMaxDaysPerWeekMaxDays[tn->teacher_ID] > tn->maxDaysPerWeek)){
                teachersMaxDaysPerWeekMaxDays[tn->teacher_ID]=tn->maxDaysPerWeek;
                teachersMaxDaysPerWeekWeightPercentages[tn->teacher_ID]=tn->weightPercentage;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_MAX_DAYS_PER_WEEK){
            ConstraintTeachersMaxDaysPerWeek* tn=(ConstraintTeachersMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers max days per week with"
                 " weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            for(int t=0; t<TContext::get()->instance.teachersList.size(); t++){
                if(teachersMaxDaysPerWeekMaxDays[t]==-1 ||
                 (teachersMaxDaysPerWeekMaxDays[t]>=0 && teachersMaxDaysPerWeekMaxDays[t] > tn->maxDaysPerWeek)){
                    teachersMaxDaysPerWeekMaxDays[t]=tn->maxDaysPerWeek;
                    teachersMaxDaysPerWeekWeightPercentages[t]=tn->weightPercentage;
                }
            }
        }
    }

    if(ok){
        //TODO:!
//        for(int i=0; i<Timetable::getInstance()->instance.activeActivitiesList.size(); i++){
//            teachersWithMaxDaysPerWeekForActivities[i].clear();

//            Activity* act=Timetable::getInstance()->instance.activeActivitiesList[i];
//            for(int j=0; j<act->iTeachersList.count(); j++){
//                int tch=act->iTeachersList.at(j);

//                if(teachersMaxDaysPerWeekMaxDays[tch]>=0){
//                    assert(teachersWithMaxDaysPerWeekForActivities[i].indexOf(tch)==-1);
//                    teachersWithMaxDaysPerWeekForActivities[i].append(tch);
//                }
//            }
//        }
    }

    return ok;
}

bool computeMaxDaysPerWeekForStudents(QWidget* parent)
{
    for(int j=0; j<TContext::get()->instance.directSubgroupsList.size(); j++){
        subgroupsMaxDaysPerWeekMaxDays[j]=-1;
        subgroupsMaxDaysPerWeekWeightPercentages[j]=-1;
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetMaxDaysPerWeek* cn=(ConstraintStudentsSetMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];
            if(cn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set max days per week with"
                 " weight (percentage) below 100 for students set %1. It is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(cn->students),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            foreach(int sb, cn->iSubgroupsList){
                if(subgroupsMaxDaysPerWeekMaxDays[sb]==-1 ||
                 (subgroupsMaxDaysPerWeekMaxDays[sb]>=0 && subgroupsMaxDaysPerWeekMaxDays[sb] > cn->maxDaysPerWeek)){
                    subgroupsMaxDaysPerWeekMaxDays[sb]=cn->maxDaysPerWeek;
                    subgroupsMaxDaysPerWeekWeightPercentages[sb]=cn->weightPercentage;
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_DAYS_PER_WEEK){
            ConstraintStudentsMaxDaysPerWeek* cn=(ConstraintStudentsMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students max days per week with"
                 " weight (percentage) below 100. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            for(int s=0; s<TContext::get()->instance.directSubgroupsList.size(); s++){
                if(subgroupsMaxDaysPerWeekMaxDays[s]==-1 ||
                 (subgroupsMaxDaysPerWeekMaxDays[s]>=0 && subgroupsMaxDaysPerWeekMaxDays[s] > cn->maxDaysPerWeek)){
                    subgroupsMaxDaysPerWeekMaxDays[s]=cn->maxDaysPerWeek;
                    subgroupsMaxDaysPerWeekWeightPercentages[s]=cn->weightPercentage;
                }
            }
        }
    }

    if(ok){
        for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
            subgroupsWithMaxDaysPerWeekForActivities[i].clear();

            Activity* act=TContext::get()->instance.activeActivitiesList[i];
            for(int j=0; j<act->iSubgroupsList.count(); j++){
                int sb=act->iSubgroupsList.at(j);

                if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){
                    assert(subgroupsWithMaxDaysPerWeekForActivities[i].indexOf(sb)==-1);
                    subgroupsWithMaxDaysPerWeekForActivities[i].append(sb);
                }
            }
        }
    }

    return ok;
}

bool computeSubgroupsEarlyAndMaxGapsPercentages(QWidget* parent) //st max gaps & early - part 2
{
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]=-1;
        subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]=-1;
        subgroupsMaxGapsPerWeekPercentage[i]=-1;
        subgroupsMaxGapsPerWeekMaxGaps[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        //students early
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR){
            ConstraintStudentsEarlyMaxBeginningsAtSecondHour* se=(ConstraintStudentsEarlyMaxBeginningsAtSecondHour*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int j=0; j<TContext::get()->instance.directSubgroupsList.size(); j++){
                if(subgroupsEarlyMaxBeginningsAtSecondHourPercentage[j] < se->weightPercentage)
                    subgroupsEarlyMaxBeginningsAtSecondHourPercentage[j] = se->weightPercentage;
                if(subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j]==-1 || subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j] > se->maxBeginningsAtSecondHour)
                    subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j] = se->maxBeginningsAtSecondHour;
            }
        }

        //students set early
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR){
            ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour* se=(ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int q=0; q<se->iSubgroupsList.count(); q++){
                int j=se->iSubgroupsList.at(q);
                if(subgroupsEarlyMaxBeginningsAtSecondHourPercentage[j] < se->weightPercentage)
                    subgroupsEarlyMaxBeginningsAtSecondHourPercentage[j] = se->weightPercentage;
                if(subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j]==-1 || subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j] > se->maxBeginningsAtSecondHour)
                    subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[j] = se->maxBeginningsAtSecondHour;
            }
        }

        //students max gaps
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_GAPS_PER_WEEK){
            ConstraintStudentsMaxGapsPerWeek* sg=(ConstraintStudentsMaxGapsPerWeek*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int j=0; j<TContext::get()->instance.directSubgroupsList.size(); j++){ //weight is 100% for all of them
                if(subgroupsMaxGapsPerWeekPercentage[j] < sg->weightPercentage)
                    subgroupsMaxGapsPerWeekPercentage[j] = sg->weightPercentage;
                if(subgroupsMaxGapsPerWeekMaxGaps[j]==-1 || subgroupsMaxGapsPerWeekMaxGaps[j] > sg->maxGaps){
                    subgroupsMaxGapsPerWeekMaxGaps[j] = sg->maxGaps;
                }
            }
        }

        //students set max gaps
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK){
            ConstraintStudentsSetMaxGapsPerWeek* sg=(ConstraintStudentsSetMaxGapsPerWeek*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int j=0; j<sg->iSubgroupsList.count(); j++){
                int s=sg->iSubgroupsList.at(j);

                if(subgroupsMaxGapsPerWeekPercentage[s] < sg->weightPercentage)
                    subgroupsMaxGapsPerWeekPercentage[s] = sg->weightPercentage;
                if(subgroupsMaxGapsPerWeekMaxGaps[s]==-1 || subgroupsMaxGapsPerWeekMaxGaps[s] > sg->maxGaps){
                    subgroupsMaxGapsPerWeekMaxGaps[s] = sg->maxGaps;
                }
            }
        }
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        assert((subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i] < 0 &&
         subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]==-1) ||
         (subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]>=0 &&
         subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]>=0));

        assert((subgroupsMaxGapsPerWeekPercentage[i] < 0 &&
         subgroupsMaxGapsPerWeekMaxGaps[i]==-1) ||
         (subgroupsMaxGapsPerWeekPercentage[i]>=0 &&
         subgroupsMaxGapsPerWeekMaxGaps[i]>=0));

        bool oksubgroup=true;
        /*if(subgroupsNoGapsPercentage[i]== -1 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]==-1 ||
            subgroupsNoGapsPercentage[i]>=0 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]==-1 ||
            subgroupsNoGapsPercentage[i]>=0 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]>=0
            && subgroupsNoGapsPercentage[i]==subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i])
                oksubgroup=true;
        else
            oksubgroup=false;*/

        if(subgroupsMaxGapsPerWeekPercentage[i]>=0 && subgroupsMaxGapsPerWeekPercentage[i] < 100){
            oksubgroup=false;
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because you have a max gaps constraint"
             " with weight percentage less than 100%. Currently, the algorithm can only"
             " optimize with not existing constraint max gaps or existing with 100% weight for it"
             ". Please modify your data correspondingly and try again")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                break;
        }
        if(subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]>=0 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i] < 100){
            oksubgroup=false;
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because you have an early max beginnings at second hour constraint"
             " with weight percentage less than 100%. Currently, the algorithm can only"
             " optimize with not existing constraint early m.b.a.s.h. or existing with 100% weight for it"
             ". Please modify your data correspondingly and try again")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                break;
        }
        /*if(subgroupsNoGapsPercentage[i]== -1 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]>=0){
            oksubgroup=false;
            ok=false;

            int t=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because a students early max beginnings at second hour constraint"
             " exists for this subgroup, and you have not 'max gaps' requirements for this subgroup. "
             "The algorithm can 1. optimize with 'early' and 'max gaps'"
             " having the same weight percentage or 2. only 'max gaps' optimization"
             " without 'early'. Please modify your data correspondingly and try again")
             .arg(Timetable::getInstance()->instance.directSubgroupsList[i]->name),
             GeneratePreTranslate::tr("Skip rest of early - max gaps problems"), GeneratePreTranslate::tr("See next incompatibility max gaps - early"), QString(),
             1, 0 );

            if(t==0)
                break;
        }
        if(subgroupsNoGapsPercentage[i]>=0 && subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]>=0
         && subgroupsNoGapsPercentage[i]!=subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]){
            oksubgroup=false;
            ok=false;

            int t=QMessageBox::warning(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because early max beginnings at second hour constraint"
             " has weight percentage %2, and 'max gaps' constraint has weight percentage %3."
             ". The algorithm can:"
             "\n1: Optimize with 'early' and 'max gaps' having the same weight percentage or"
             "\n2. Only 'max gaps' optimization without 'early'."
             "\nPlease modify your data correspondingly and try again")
             .arg(Timetable::getInstance()->instance.directSubgroupsList[i]->name)
             .arg(subgroupsEarlyMaxBeginningsAtSecondHourPercentage[i]).
             arg(subgroupsNoGapsPercentage[i]),
             GeneratePreTranslate::tr("Skip rest of early - max gaps problems"), GeneratePreTranslate::tr("See next incompatibility max gaps - early"), QString(),
             1, 0 );

            if(t==0)
                break;
        }*/

        if(subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]>=0
         && subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]>TContext::get()->instance.nDaysPerWeek){
            oksubgroup=false;
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because early max beginnings at second hour constraint"
             " has max beginnings at second hour %2, and the number of days per week is %3 which is less. It must be that the number of"
             " days per week must be greater or equal with the max beginnings at second hour\n"
             "Please modify your data correspondingly and try again")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name)
             .arg(subgroupsEarlyMaxBeginningsAtSecondHourMaxBeginnings[i]).
             arg(TContext::get()->instance.nDaysPerWeek),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                break;
        }

        if(!oksubgroup)
            ok=false;
    }

    return ok;
}

bool computeSubgroupsMaxGapsPerDayPercentages(QWidget* parent)
{
    haveStudentsMaxGapsPerDay=false;

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsMaxGapsPerDayPercentage[i]=-1;
        subgroupsMaxGapsPerDayMaxGaps[i]=-1;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_MAX_GAPS_PER_DAY){
            haveStudentsMaxGapsPerDay=true;
            ConstraintStudentsMaxGapsPerDay* sg=(ConstraintStudentsMaxGapsPerDay*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int j=0; j<TContext::get()->instance.directSubgroupsList.size(); j++){ //weight is 100% for all of them
                if(subgroupsMaxGapsPerDayPercentage[j] < sg->weightPercentage)
                    subgroupsMaxGapsPerDayPercentage[j] = sg->weightPercentage;
                if(subgroupsMaxGapsPerDayMaxGaps[j]==-1 || subgroupsMaxGapsPerDayMaxGaps[j] > sg->maxGaps){
                    subgroupsMaxGapsPerDayMaxGaps[j] = sg->maxGaps;
                }
            }
        }

        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY){
            haveStudentsMaxGapsPerDay=true;
            ConstraintStudentsSetMaxGapsPerDay* sg=(ConstraintStudentsSetMaxGapsPerDay*) TContext::get()->instance.validTimeConstraintsList[i];
            for(int j=0; j<sg->iSubgroupsList.count(); j++){
                int s=sg->iSubgroupsList.at(j);

                if(subgroupsMaxGapsPerDayPercentage[s] < sg->weightPercentage)
                    subgroupsMaxGapsPerDayPercentage[s] = sg->weightPercentage;
                if(subgroupsMaxGapsPerDayMaxGaps[s]==-1 || subgroupsMaxGapsPerDayMaxGaps[s] > sg->maxGaps){
                    subgroupsMaxGapsPerDayMaxGaps[s] = sg->maxGaps;
                }
            }
        }
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        assert((subgroupsMaxGapsPerDayPercentage[i] < 0 &&
         subgroupsMaxGapsPerDayMaxGaps[i]==-1) ||
         (subgroupsMaxGapsPerDayPercentage[i]>=0 &&
         subgroupsMaxGapsPerDayMaxGaps[i]>=0));

        bool oksubgroup=true;

        if(subgroupsMaxGapsPerDayPercentage[i]>=0 && subgroupsMaxGapsPerDayPercentage[i] < 100){
            oksubgroup=false;
            ok=false;

            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
             GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because you have a max gaps constraint"
             " with weight percentage less than 100%. Currently, the algorithm can only"
             " optimize with not existing constraint max gaps or existing with 100% weight for it"
             ". Please modify your data correspondingly and try again")
             .arg(TContext::get()->instance.directSubgroupsList[i]->name),
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                break;
        }

        if(!oksubgroup)
            ok=false;
    }

    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        if(subgroupsMaxGapsPerDayMaxGaps[i]>=0){
            int mgw=TContext::get()->instance.nDaysPerWeek*subgroupsMaxGapsPerDayMaxGaps[i];
            assert(mgw>=0);
            if(subgroupsMaxGapsPerWeekMaxGaps[i]==-1 || (subgroupsMaxGapsPerWeekMaxGaps[i]>=0 && subgroupsMaxGapsPerWeekMaxGaps[i]>mgw)){
                subgroupsMaxGapsPerWeekMaxGaps[i]=mgw;
                subgroupsMaxGapsPerWeekPercentage[i]=100.0;
            }
        }
    }

    return ok;
}

bool computeNotAllowedTimesPercentages(QWidget* parent)
{
    Matrix1D<bool> allowed;
    allowed.resize(TContext::get()->instance.nHoursPerWeek);

    bool ok=true;

    assert(TContext::get()->instance.internalStructureComputed);

//    Timetable::getInstance()->instance.breakDayHour.resize(Timetable::getInstance()->instance.nDaysPerWeek, Timetable::getInstance()->instance.nHoursPerDay);
//    //BREAK
//    for(int j=0; j<Timetable::getInstance()->instance.nDaysPerWeek; j++)
//        for(int k=0; k<Timetable::getInstance()->instance.nHoursPerDay; k++)
//            Timetable::getInstance()->instance.breakDayHour[j][k]=false;

//    Timetable::getInstance()->instance.subgroupNotAvailableDayHour.resize(Timetable::getInstance()->instance.directSubgroupsList.size(), Timetable::getInstance()->instance.nDaysPerWeek, Timetable::getInstance()->instance.nHoursPerDay);
//    //STUDENTS SET NOT AVAILABLE
//    for(int i=0; i<Timetable::getInstance()->instance.directSubgroupsList.size(); i++)
//        for(int j=0; j<Timetable::getInstance()->instance.nDaysPerWeek; j++)
//            for(int k=0; k<Timetable::getInstance()->instance.nHoursPerDay; k++)
//                Timetable::getInstance()->instance.subgroupNotAvailableDayHour[i][j][k]=false;

//    Timetable::getInstance()->instance.teacherNotAvailableDayHour.resize(Timetable::getInstance()->instance.teachersList.size(), Timetable::getInstance()->instance.nDaysPerWeek, Timetable::getInstance()->instance.nHoursPerDay);
//    //TEACHER NOT AVAILABLE
//    for(int i=0; i<Timetable::getInstance()->instance.teachersList.size(); i++)
//        for(int j=0; j<Timetable::getInstance()->instance.nDaysPerWeek; j++)
//            for(int k=0; k<Timetable::getInstance()->instance.nHoursPerDay; k++)
//                Timetable::getInstance()->instance.teacherNotAvailableDayHour[i][j][k]=false;

    notAllowedTimesPercentages.resize(TContext::get()->instance.activeActivitiesList.size(), TContext::get()->instance.nHoursPerWeek);
    //improvement by Volker Dirr (late activities):
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        Activity* act=TContext::get()->instance.activeActivitiesList[i];
        for(int j=0; j<TContext::get()->instance.nHoursPerWeek; j++){
            int h=j/TContext::get()->instance.nDaysPerWeek;
            if(h+act->duration <= TContext::get()->instance.nHoursPerDay)
                notAllowedTimesPercentages[i][j]=-1;
            else
                notAllowedTimesPercentages[i][j]=100;
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
//            //TEACHER not available
//            if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES){
//                ConstraintTeacherNotAvailableTimes* tn=(ConstraintTeacherNotAvailableTimes*)Timetable::getInstance()->instance.validTimeConstraintsList[i];
//                //TODO:!
////                for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
////                    Activity* act=Timetable::getInstance()->instance.activeActivitiesList[ai];
////                    for(int ti=0; ti<act->iTeachersList.count(); ti++)
////                        if(act->iTeachersList.at(ti)==tn->teacher_ID){
////                            assert(tn->days.count()==tn->hours.count());
////                            for(int kk=0; kk<tn->days.count(); kk++){
////                                int d=tn->days.at(kk);
////                                int h=tn->hours.at(kk);

////                                for(int hh=max(0, h-act->duration+1); hh<=h; hh++)
////                                    if(notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]<tn->weightPercentage)
////                                        notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]=tn->weightPercentage;
////                            }
////                            //break; //search no more for teacher -- careful with that
////                        }
////                }

//                if(tn->weightPercentage!=100){
//                    ok=false;

//                    MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET warning"),
//                     GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
//                     "teacher not available with weight percentage less than 100% for teacher %1. Currently, m-FET can only optimize with "
//                     "constraints teacher not available with 100% weight (or no constraint). Please "
//                     "modify your data accordingly and try again.").arg(tn->teacher));

//                    return ok;
//                }
//                else{
//                    assert(tn->weightPercentage==100);
//                    assert(tn->days.count()==tn->hours.count());
//                    for(int kk=0; kk<tn->days.count(); kk++){
//                        int d=tn->days.at(kk);
//                        int h=tn->hours.at(kk);

//                        Timetable::getInstance()->instance.teacherNotAvailableDayHour[tn->teacher_ID][d][h]=true;
//                    }
//                }
//            }

//            //STUDENTS SET not available
//            if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES){
//                ConstraintStudentsSetNotAvailableTimes* sn=(ConstraintStudentsSetNotAvailableTimes*)Timetable::getInstance()->instance.validTimeConstraintsList[i];
//                for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
//                    Activity* act=Timetable::getInstance()->instance.activeActivitiesList[ai];
//                    for(int sg=0; sg<act->iSubgroupsList.count(); sg++)
//                        for(int j=0; j<sn->iSubgroupsList.count(); j++){
//                            if(act->iSubgroupsList.at(sg)==sn->iSubgroupsList.at(j)){
//                                assert(sn->days.count()==sn->hours.count());
//                                for(int kk=0; kk<sn->days.count(); kk++){
//                                    int d=sn->days.at(kk);
//                                    int h=sn->hours.at(kk);

//                                    for(int hh=max(0, h-act->duration+1); hh<=h; hh++)
//                                        if(notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]<sn->weightPercentage)
//                                            notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]=sn->weightPercentage;
//                                }
//                                //break; //search no more for subgroup - this can bring an improvement in precalculation
//                                //but needs attention
//                            }
//                        }
//                }

//                if(sn->weightPercentage!=100){
//                    ok=false;

//                    MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET warning"),
//                     GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
//                     "students set not available with weight percentage less than 100% for students set %1. Currently, m-FET can only optimize with "
//                     "constraints students set not available with 100% weight (or no constraint). Please "
//                     "modify your data accordingly and try again.").arg(sn->students));

//                    return ok;
//                }
//                else{
//                    assert(sn->weightPercentage==100);
//                    for(int q=0; q<sn->iSubgroupsList.count(); q++){
//                        int ss=sn->iSubgroupsList.at(q);
//                        assert(sn->days.count()==sn->hours.count());
//                        for(int kk=0; kk<sn->days.count(); kk++){
//                            int d=sn->days.at(kk);
//                            int h=sn->hours.at(kk);

//                            Timetable::getInstance()->instance.subgroupNotAvailableDayHour[ss][d][h]=true;
//                        }
//                    }
//                }
//            }

//            //BREAK
//            if(Timetable::getInstance()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_BREAK_TIMES){
//                ConstraintBreakTimes* br=(ConstraintBreakTimes*)Timetable::getInstance()->instance.validTimeConstraintsList[i];
//                for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
//                    Activity* act=Timetable::getInstance()->instance.activeActivitiesList[ai];

//                    assert(br->days.count()==br->hours.count());
//                    for(int kk=0; kk<br->days.count(); kk++){
//                        int d=br->days.at(kk);
//                        int h=br->hours.at(kk);

//                        for(int hh=max(0, h-act->duration+1); hh<=h; hh++)
//                            if(notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]<br->weightPercentage)
//                                notAllowedTimesPercentages[ai][d+hh*Timetable::getInstance()->instance.nDaysPerWeek]=br->weightPercentage;
//                    }
//                }

//                if(br->weightPercentage!=100){
//                    ok=false;

//                    MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET warning"),
//                     GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
//                     "break with weight percentage less than 100%. Currently, m-FET can only optimize with "
//                     "constraints break with 100% weight (or no constraint). Please "
//                     "modify your data accordingly and try again."));

//                    return ok;
//                }
//                else{
//                    assert(br->weightPercentage==100);

//                    assert(br->days.count()==br->hours.count());
//                    for(int kk=0; kk<br->days.count(); kk++){
//                        int d=br->days.at(kk);
//                        int h=br->hours.at(kk);

//                        Timetable::getInstance()->instance.breakDayHour[d][h]=true;
//                    }
//                }
//            }

            //ACTIVITY preferred starting time
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME){
                ConstraintActivityPreferredStartingTime* ap=(ConstraintActivityPreferredStartingTime*)TContext::get()->instance.validTimeConstraintsList[i];

                if(ap->day>=0 && ap->hour>=0){
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                            if(d!=ap->day || h!=ap->hour)
                                if(notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]<ap->weightPercentage)
                                    notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]=ap->weightPercentage;
                }
                else if(ap->day>=0){
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                            if(d!=ap->day)
                                if(notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]<ap->weightPercentage)
                                    notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]=ap->weightPercentage;
                }
                else if(ap->hour>=0){
                    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                        for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                            if(h!=ap->hour)
                                if(notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]<ap->weightPercentage)
                                    notAllowedTimesPercentages[ap->activityIndex][d+h*TContext::get()->instance.nDaysPerWeek]=ap->weightPercentage;
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
                     "activity preferred starting time with no day nor hour selected (for activity with id==%1). "
                     "Please modify your data accordingly (remove or edit constraint) and try again.")
                     .arg(TContext::get()->instance.activeActivitiesList[ap->activityIndex]->id),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                    1, 0 );

                    if(t==0)
                        break;
                    //assert(0);
                }
            }

            //ACTIVITY preferred starting times
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES){
                ConstraintActivityPreferredStartingTimes* ap=(ConstraintActivityPreferredStartingTimes*)TContext::get()->instance.validTimeConstraintsList[i];

                int ai=ap->activityIndex;

                //Matrix1D<bool> allowed;
                //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                //bool allowed[MAX_HOURS_PER_WEEK];
                for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                    allowed[k]=false;

                for(int m=0; m<ap->nPreferredStartingTimes_L; m++){
                    int d=ap->days_L[m];
                    int h=ap->hours_L[m];

                    if(d>=0 && h>=0){
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                    else if(d>=0){
                        for(int hh=0; hh<TContext::get()->instance.nHoursPerDay; hh++)
                            allowed[d+hh*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                    else if(h>=0){
                        for(int dd=0; dd<TContext::get()->instance.nDaysPerWeek; dd++)
                            allowed[dd+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                }

                for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                    if(!allowed[k])
                        if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                            notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
            }

            //ACTIVITIES preferred starting times
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES){
                ConstraintActivitiesPreferredStartingTimes* ap=(ConstraintActivitiesPreferredStartingTimes*)TContext::get()->instance.validTimeConstraintsList[i];

                for(int j=0; j<ap->nActivities; j++){
                    int ai=ap->activitiesIndices[j];

                    //Matrix1D<bool> allowed;
                    //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                    //bool allowed[MAX_HOURS_PER_WEEK];
                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        allowed[k]=false;

                    for(int m=0; m<ap->nPreferredStartingTimes_L; m++){
                        int d=ap->days_L[m];
                        int h=ap->hours_L[m];
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }

                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        if(!allowed[k])
                            if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                                notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
                }
            }
            //subactivities preferred starting times
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES){
                ConstraintSubactivitiesPreferredStartingTimes* ap=(ConstraintSubactivitiesPreferredStartingTimes*)TContext::get()->instance.validTimeConstraintsList[i];

                for(int j=0; j<ap->nActivities; j++){
                    int ai=ap->activitiesIndices[j];

                    //Matrix1D<bool> allowed;
                    //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                    //bool allowed[MAX_HOURS_PER_WEEK];
                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        allowed[k]=false;

                    for(int m=0; m<ap->nPreferredStartingTimes_L; m++){
                        int d=ap->days_L[m];
                        int h=ap->hours_L[m];
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }

                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        if(!allowed[k])
                            if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                                notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
                }
            }
            //ACTIVITY preferred time slots
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS){
                ConstraintActivityPreferredTimeSlots* ap=(ConstraintActivityPreferredTimeSlots*)TContext::get()->instance.validTimeConstraintsList[i];

                int ai=ap->p_activityIndex;

                //Matrix1D<bool> allowed;
                //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                //bool allowed[MAX_HOURS_PER_WEEK];
                for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                    allowed[k]=false;

                for(int m=0; m<ap->p_nPreferredTimeSlots_L; m++){
                    int d=ap->p_days_L[m];
                    int h=ap->p_hours_L[m];

                    if(d>=0 && h>=0){
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                    else if(d>=0){
                        for(int hh=0; hh<TContext::get()->instance.nHoursPerDay; hh++)
                            allowed[d+hh*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                    else if(h>=0){
                        for(int dd=0; dd<TContext::get()->instance.nDaysPerWeek; dd++)
                            allowed[dd+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }
                }

                for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++){
                    int d=k%TContext::get()->instance.nDaysPerWeek;
                    int h=k/TContext::get()->instance.nDaysPerWeek;

                    bool ok=true;

                    for(int dur=0; dur<TContext::get()->instance.activeActivitiesList[ai]->duration && h+dur<TContext::get()->instance.nHoursPerDay; dur++){
                        assert(d+(h+dur)*TContext::get()->instance.nDaysPerWeek<TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay);
                        if(!allowed[d+(h+dur)*TContext::get()->instance.nDaysPerWeek]){
                            ok=false;
                            break;
                        }
                    }

                    if(!ok)
                        if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                            notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
                }
            }

            //ACTIVITIES preferred time slots
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS){
                ConstraintActivitiesPreferredTimeSlots* ap=(ConstraintActivitiesPreferredTimeSlots*)TContext::get()->instance.validTimeConstraintsList[i];

                for(int j=0; j<ap->p_nActivities; j++){
                    int ai=ap->p_activitiesIndices[j];

                    //Matrix1D<bool> allowed;
                    //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                    //bool allowed[MAX_HOURS_PER_WEEK];
                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        allowed[k]=false;

                    for(int m=0; m<ap->p_nPreferredTimeSlots_L; m++){
                        int d=ap->p_days_L[m];
                        int h=ap->p_hours_L[m];
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }

                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++){
                        int d=k%TContext::get()->instance.nDaysPerWeek;
                        int h=k/TContext::get()->instance.nDaysPerWeek;

                        bool ok=true;

                        for(int dur=0; dur<TContext::get()->instance.activeActivitiesList[ai]->duration && h+dur<TContext::get()->instance.nHoursPerDay; dur++){
                            assert(d+(h+dur)*TContext::get()->instance.nDaysPerWeek<TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay);
                            if(!allowed[d+(h+dur)*TContext::get()->instance.nDaysPerWeek]){
                                ok=false;
                                break;
                            }
                        }

                        if(!ok)
                            if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                                notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
                    }
                }
            }
            //subactivities preferred time slots
            if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS){
                ConstraintSubactivitiesPreferredTimeSlots* ap=(ConstraintSubactivitiesPreferredTimeSlots*)TContext::get()->instance.validTimeConstraintsList[i];

                for(int j=0; j<ap->p_nActivities; j++){
                    int ai=ap->p_activitiesIndices[j];

                    //Matrix1D<bool> allowed;
                    //allowed.resize(Timetable::getInstance()->instance.nHoursPerWeek);
                    //bool allowed[MAX_HOURS_PER_WEEK];
                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++)
                        allowed[k]=false;

                    for(int m=0; m<ap->p_nPreferredTimeSlots_L; m++){
                        int d=ap->p_days_L[m];
                        int h=ap->p_hours_L[m];
                        assert(d>=0 && h>=0);
                        allowed[d+h*TContext::get()->instance.nDaysPerWeek]=true;
                    }

                    for(int k=0; k<TContext::get()->instance.nHoursPerWeek; k++){
                        int d=k%TContext::get()->instance.nDaysPerWeek;
                        int h=k/TContext::get()->instance.nDaysPerWeek;

                        bool ok=true;

                        for(int dur=0; dur<TContext::get()->instance.activeActivitiesList[ai]->duration && h+dur<TContext::get()->instance.nHoursPerDay; dur++){
                            assert(d+(h+dur)*TContext::get()->instance.nDaysPerWeek<TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay);
                            if(!allowed[d+(h+dur)*TContext::get()->instance.nDaysPerWeek]){
                                ok=false;
                                break;
                            }
                        }

                        if(!ok)
                            if(notAllowedTimesPercentages[ai][k] < ap->weightPercentage)
                                notAllowedTimesPercentages[ai][k] = ap->weightPercentage;
                    }
                }
            }
    }

    return ok;
}

bool computeMinDays(QWidget* parent)
{
    QSet<ConstraintMinDaysBetweenActivities*> mdset;

    bool ok=true;

    for(int j=0; j<TContext::get()->instance.activeActivitiesList.size(); j++){
        minDaysListOfActivities[j].clear();
        minDaysListOfMinDays[j].clear();
        minDaysListOfConsecutiveIfSameDay[j].clear();
        minDaysListOfWeightPercentages[j].clear();

        //for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList.size(); k++)
        //	minDays[j][k]=0;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES
         /*&&Timetable::getInstance()->instance.validTimeConstraintsList[i]->compulsory==true*/){
            ConstraintMinDaysBetweenActivities* md=
             (ConstraintMinDaysBetweenActivities*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<md->_n_activities; j++){
                int ai1=md->_activities[j];
                for(int k=0; k<md->_n_activities; k++)
                    if(j!=k){
                        int ai2=md->_activities[k];
                        if(ai1==ai2){
                            ok=false;

                            if(!mdset.contains(md)){
                                mdset.insert(md);

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize, because you have a constraint min days with duplicate activities. The constraint "
                                 "is: %1. Please correct that.").arg(md->getDetailedDescription()),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return ok;
                            }
                        }
                        int m=md->minDays;
                        /*if(m>minDays[ai1][ai2])
                            minDays[ai1][ai2]=minDays[ai2][ai1]=m;*/

                        minDaysListOfActivities[ai1].append(ai2);
                        minDaysListOfMinDays[ai1].append(m);
                        assert(md->weightPercentage >=0 && md->weightPercentage<=100);
                        minDaysListOfWeightPercentages[ai1].append(md->weightPercentage);
                        minDaysListOfConsecutiveIfSameDay[ai1].append(md->consecutiveIfSameDay);
                    }
            }
        }

    /*for(int j=0; j<Timetable::getInstance()->instance.activeActivitiesList.size(); j++)
        for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList.size(); k++)
            if(minDays[j][k]>0){
                assert(j!=k);
                minDaysListOfActivities[j].append(k);
                minDaysListOfMinDays[j].append(minDays[j][k]);
            }*/

    return ok;
}

bool computeMaxDays(QWidget* parent)
{
    QSet<ConstraintMaxDaysBetweenActivities*> mdset;

    bool ok=true;

    for(int j=0; j<TContext::get()->instance.activeActivitiesList.size(); j++){
        maxDaysListOfActivities[j].clear();
        maxDaysListOfMaxDays[j].clear();
        maxDaysListOfWeightPercentages[j].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_MAX_DAYS_BETWEEN_ACTIVITIES
         /*&&Timetable::getInstance()->instance.validTimeConstraintsList[i]->compulsory==true*/){
            ConstraintMaxDaysBetweenActivities* md=
             (ConstraintMaxDaysBetweenActivities*)TContext::get()->instance.validTimeConstraintsList[i];

            for(int j=0; j<md->_n_activities; j++){
                int ai1=md->_activities[j];
                for(int k=0; k<md->_n_activities; k++)
                    if(j!=k){
                        int ai2=md->_activities[k];
                        if(ai1==ai2){
                            ok=false;

                            if(!mdset.contains(md)){
                                mdset.insert(md);

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize, because you have a constraint max days between activities with duplicate activities. The constraint "
                                 "is: %1. Please correct that.").arg(md->getDetailedDescription()),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return ok;
                            }
                        }
                        int m=md->maxDays;
                        /*if(m>minDays[ai1][ai2])
                            minDays[ai1][ai2]=minDays[ai2][ai1]=m;*/

                        maxDaysListOfActivities[ai1].append(ai2);
                        maxDaysListOfMaxDays[ai1].append(m);
                        assert(md->weightPercentage >=0 && md->weightPercentage<=100);
                        maxDaysListOfWeightPercentages[ai1].append(md->weightPercentage);
                        //maxDaysListOfConsecutiveIfSameDay[ai1].append(md->consecutiveIfSameDay);
                    }
            }
        }

    /*for(int j=0; j<Timetable::getInstance()->instance.activeActivitiesList.size(); j++)
        for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList.size(); k++)
            if(minDays[j][k]>0){
                assert(j!=k);
                minDaysListOfActivities[j].append(k);
                minDaysListOfMinDays[j].append(minDays[j][k]);
            }*/

    return ok;
}

bool computeMinGapsBetweenActivities(QWidget* parent)
{
    QSet<ConstraintMinGapsBetweenActivities*> mgset;

    bool ok=true;

    for(int j=0; j<TContext::get()->instance.activeActivitiesList.size(); j++){
        minGapsBetweenActivitiesListOfActivities[j].clear();
        minGapsBetweenActivitiesListOfMinGaps[j].clear();
        minGapsBetweenActivitiesListOfWeightPercentages[j].clear();

        //for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList.size(); k++)
        //	minDays[j][k]=0;
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_MIN_GAPS_BETWEEN_ACTIVITIES
         /*&&Timetable::getInstance()->instance.validTimeConstraintsList[i]->compulsory==true*/){
            ConstraintMinGapsBetweenActivities* mg=
             (ConstraintMinGapsBetweenActivities*)TContext::get()->instance.validTimeConstraintsList[i];

            assert(mg->_n_activities==mg->_activities.count());

            for(int j=0; j<mg->_n_activities; j++){
                int ai1=mg->_activities[j];
                for(int k=0; k<mg->_n_activities; k++)
                    if(j!=k){
                        int ai2=mg->_activities[k];
                        if(ai1==ai2){
                            ok=false;

                            if(!mgset.contains(mg)){
                                mgset.insert(mg);

                                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                                 GeneratePreTranslate::tr("Cannot optimize, because you have a constraint min gaps between activities with duplicate activities. The constraint "
                                 "is: %1. Please correct that.").arg(mg->getDetailedDescription()),
                                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                                 1, 0 );

                                if(t==0)
                                    return ok;
                            }
                        }
                        int m=mg->minGaps;

                        minGapsBetweenActivitiesListOfActivities[ai1].append(ai2);
                        minGapsBetweenActivitiesListOfMinGaps[ai1].append(m);
                        assert(mg->weightPercentage >=0 && mg->weightPercentage<=100);
                        minGapsBetweenActivitiesListOfWeightPercentages[ai1].append(mg->weightPercentage);
                    }
            }
        }

    return ok;
}

bool computeActivitiesConflictingPercentage(QWidget* parent)
{
    //get maximum weight percent of a basic time constraint
    double m=-1;

    bool ok=false;
    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_BASIC_COMPULSORY_TIME){
            ok=true;
            if(TContext::get()->instance.validTimeConstraintsList[i]->weightPercentage>m)
                m=TContext::get()->instance.validTimeConstraintsList[i]->weightPercentage;
        }

    if(m<100)
        ok=false;

    if(!ok || m<100){
        MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET warning"),
         GeneratePreTranslate::tr("Cannot generate, because you do not have a constraint of type basic compulsory time or its weight is lower than 100.0%.")
         +" "+
         GeneratePreTranslate::tr("Please add a constraint of this type with weight 100%.")
         +" "+
         GeneratePreTranslate::tr("You can add this constraint from the menu Data -> Time constraints -> Miscellaneous -> Basic compulsory time constraints.")
         +"\n\n"+
         GeneratePreTranslate::tr("Explanation:")
         +" "+
         GeneratePreTranslate::tr("Each time you create a new file, it contains an automatically added constraint of this type.")
         +" "+
         GeneratePreTranslate::tr("For complete flexibility, you are allowed to remove it (even if this is a wrong idea).")
         +" "+
         GeneratePreTranslate::tr("Maybe you removed it by mistake from your file.")
         +" "+
         GeneratePreTranslate::tr("By adding it again, everything should be all right.")
         );
        return false;
    }

    assert(m>=0 && m<=100);
    assert(IS_EQUAL(m, 100));

    //compute conflicting
    activitiesConflictingPercentage.resize(TContext::get()->instance.activeActivitiesList.size());
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++)
        activitiesConflictingPercentage[i].clear();

    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++)
        activitiesConflictingPercentage[i].insert(i, 100);

    QProgressDialog progress(parent);
    progress.setWindowTitle(GeneratePreTranslate::tr("Precomputing", "Title of a progress dialog"));
    progress.setLabelText(GeneratePreTranslate::tr("Precomputing ... please wait"));
    progress.setRange(0, qMax(TContext::get()->instance.teachersList.size()+TContext::get()->instance.directSubgroupsList.size(), 1));
    progress.setModal(true);

    int ttt=0;

    for(int t=0; t<TContext::get()->instance.teachersList.size(); t++){
        progress.setValue(ttt);
        //pqapplication->processEvents();
        if(progress.wasCanceled()){
            MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET information"), GeneratePreTranslate::tr("Canceled"));
            return false;
        }

        ttt++;
//TODO:!
//        foreach(int i, Timetable::getInstance()->instance.teachersList[t]->activitiesForTeacher)
//            foreach(int j, Timetable::getInstance()->instance.teachersList[t]->activitiesForTeacher)
//                activitiesConflictingPercentage[i].insert(j, 100);
    }

    for(int s=0; s<TContext::get()->instance.directSubgroupsList.size(); s++){
        progress.setValue(ttt);
        //pqapplication->processEvents();
        if(progress.wasCanceled()){
            MessagesManager::information(parent, GeneratePreTranslate::tr("m-FET information"), GeneratePreTranslate::tr("Canceled"));
            return false;
        }

        ttt++;

        foreach(int i, TContext::get()->instance.directSubgroupsList[s]->activitiesForSubgroup)
            foreach(int j, TContext::get()->instance.directSubgroupsList[s]->activitiesForSubgroup)
                activitiesConflictingPercentage[i].insert(j, 100);
    }

    progress.setValue(qMax(TContext::get()->instance.teachersList.size()+TContext::get()->instance.directSubgroupsList.size(), 1));

    //new volker (start)
    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        TimeConstraint* tc=TContext::get()->instance.validTimeConstraintsList[i];
        if(tc->type==CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING){
            if(tc->weightPercentage==100.0){
                ConstraintActivitiesNotOverlapping* cno=(ConstraintActivitiesNotOverlapping*) tc;

                for(int a=0; a<cno->_n_activities; a++){
                    for(int b=0; b<cno->_n_activities; b++){
                        if(cno->_activities[a]!=cno->_activities[b]){
                            activitiesConflictingPercentage[cno->_activities[a]].insert(cno->_activities[b], 100);
                        }
                    }
                }
            }
            else{
                ConstraintActivitiesNotOverlapping* cno=(ConstraintActivitiesNotOverlapping*) tc;

                int ww=int(cno->weightPercentage);
                if(ww>100)
                    ww=100;

                for(int a=0; a<cno->_n_activities; a++){
                    for(int b=0; b<cno->_n_activities; b++){
                        if(cno->_activities[a]!=cno->_activities[b]){
                            if(activitiesConflictingPercentage[cno->_activities[a]].value(cno->_activities[b], -1) < ww)
                                activitiesConflictingPercentage[cno->_activities[a]].insert(cno->_activities[b], ww);
                        }
                    }
                }
            }
        }
    }
    //new volker (end)

    return true;
}

void computeConstrTwoActivitiesConsecutive()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        constrTwoActivitiesConsecutivePercentages[i].clear();
        constrTwoActivitiesConsecutiveActivities[i].clear();

        inverseConstrTwoActivitiesConsecutivePercentages[i].clear();
        inverseConstrTwoActivitiesConsecutiveActivities[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TWO_ACTIVITIES_CONSECUTIVE){
            ConstraintTwoActivitiesConsecutive* c2=(ConstraintTwoActivitiesConsecutive*)TContext::get()->instance.validTimeConstraintsList[i];

            int fai=c2->firstActivityIndex;
            int sai=c2->secondActivityIndex;

            //direct
            int j=constrTwoActivitiesConsecutiveActivities[fai].indexOf(sai);
            if(j==-1){
                constrTwoActivitiesConsecutiveActivities[fai].append(sai);
                constrTwoActivitiesConsecutivePercentages[fai].append(c2->weightPercentage);
            }
            else if(j>=0 && constrTwoActivitiesConsecutivePercentages[fai].at(j)<c2->weightPercentage){
                constrTwoActivitiesConsecutivePercentages[fai][j]=c2->weightPercentage;
            }

            //inverse
            j=inverseConstrTwoActivitiesConsecutiveActivities[sai].indexOf(fai);
            if(j==-1){
                inverseConstrTwoActivitiesConsecutiveActivities[sai].append(fai);
                inverseConstrTwoActivitiesConsecutivePercentages[sai].append(c2->weightPercentage);
            }
            else if(j>=0 && inverseConstrTwoActivitiesConsecutivePercentages[sai].at(j)<c2->weightPercentage){
                inverseConstrTwoActivitiesConsecutivePercentages[sai][j]=c2->weightPercentage;
            }
        }
}

void computeConstrTwoActivitiesGrouped()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        constrTwoActivitiesGroupedPercentages[i].clear();
        constrTwoActivitiesGroupedActivities[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TWO_ACTIVITIES_GROUPED){
            ConstraintTwoActivitiesGrouped* c2=(ConstraintTwoActivitiesGrouped*)TContext::get()->instance.validTimeConstraintsList[i];

            int fai=c2->firstActivityIndex;
            int sai=c2->secondActivityIndex;

            //direct
            int j=constrTwoActivitiesGroupedActivities[fai].indexOf(sai);
            if(j==-1){
                constrTwoActivitiesGroupedActivities[fai].append(sai);
                constrTwoActivitiesGroupedPercentages[fai].append(c2->weightPercentage);
            }
            else if(j>=0 && constrTwoActivitiesGroupedPercentages[fai].at(j)<c2->weightPercentage){
                constrTwoActivitiesGroupedPercentages[fai][j]=c2->weightPercentage;
            }

            //inverse
            j=constrTwoActivitiesGroupedActivities[sai].indexOf(fai);
            if(j==-1){
                constrTwoActivitiesGroupedActivities[sai].append(fai);
                constrTwoActivitiesGroupedPercentages[sai].append(c2->weightPercentage);
            }
            else if(j>=0 && constrTwoActivitiesGroupedPercentages[sai].at(j)<c2->weightPercentage){
                constrTwoActivitiesGroupedPercentages[sai][j]=c2->weightPercentage;
            }
        }
}

void computeConstrThreeActivitiesGrouped()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        constrThreeActivitiesGroupedPercentages[i].clear();
        constrThreeActivitiesGroupedActivities[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_THREE_ACTIVITIES_GROUPED){
            ConstraintThreeActivitiesGrouped* c3=(ConstraintThreeActivitiesGrouped*)TContext::get()->instance.validTimeConstraintsList[i];

            int fai=c3->firstActivityIndex;
            int sai=c3->secondActivityIndex;
            int tai=c3->thirdActivityIndex;

            QPair<int, int> p23(sai, tai);
            int j=constrThreeActivitiesGroupedActivities[fai].indexOf(p23);
            if(j==-1){
                constrThreeActivitiesGroupedActivities[fai].append(p23);
                constrThreeActivitiesGroupedPercentages[fai].append(c3->weightPercentage);
            }
            else if(j>=0 && constrThreeActivitiesGroupedPercentages[fai].at(j)<c3->weightPercentage){
                constrThreeActivitiesGroupedPercentages[fai][j]=c3->weightPercentage;
            }

            QPair<int, int> p13(fai, tai);
            j=constrThreeActivitiesGroupedActivities[sai].indexOf(p13);
            if(j==-1){
                constrThreeActivitiesGroupedActivities[sai].append(p13);
                constrThreeActivitiesGroupedPercentages[sai].append(c3->weightPercentage);
            }
            else if(j>=0 && constrThreeActivitiesGroupedPercentages[sai].at(j)<c3->weightPercentage){
                constrThreeActivitiesGroupedPercentages[sai][j]=c3->weightPercentage;
            }

            QPair<int, int> p12(fai, sai);
            j=constrThreeActivitiesGroupedActivities[tai].indexOf(p12);
            if(j==-1){
                constrThreeActivitiesGroupedActivities[tai].append(p12);
                constrThreeActivitiesGroupedPercentages[tai].append(c3->weightPercentage);
            }
            else if(j>=0 && constrThreeActivitiesGroupedPercentages[tai].at(j)<c3->weightPercentage){
                constrThreeActivitiesGroupedPercentages[tai][j]=c3->weightPercentage;
            }
        }
}

void computeConstrTwoActivitiesOrdered()
{
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        constrTwoActivitiesOrderedPercentages[i].clear();
        constrTwoActivitiesOrderedActivities[i].clear();

        inverseConstrTwoActivitiesOrderedPercentages[i].clear();
        inverseConstrTwoActivitiesOrderedActivities[i].clear();
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++)
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TWO_ACTIVITIES_ORDERED){
            ConstraintTwoActivitiesOrdered* c2=(ConstraintTwoActivitiesOrdered*)TContext::get()->instance.validTimeConstraintsList[i];

            int fai=c2->firstActivityIndex;
            int sai=c2->secondActivityIndex;

            //direct
            int j=constrTwoActivitiesOrderedActivities[fai].indexOf(sai);
            if(j==-1){
                constrTwoActivitiesOrderedActivities[fai].append(sai);
                constrTwoActivitiesOrderedPercentages[fai].append(c2->weightPercentage);
            }
            else if(j>=0 && constrTwoActivitiesOrderedPercentages[fai].at(j)<c2->weightPercentage){
                constrTwoActivitiesOrderedPercentages[fai][j]=c2->weightPercentage;
            }

            //inverse
            j=inverseConstrTwoActivitiesOrderedActivities[sai].indexOf(fai);
            if(j==-1){
                inverseConstrTwoActivitiesOrderedActivities[sai].append(fai);
                inverseConstrTwoActivitiesOrderedPercentages[sai].append(c2->weightPercentage);
            }
            else if(j>=0 && inverseConstrTwoActivitiesOrderedPercentages[sai].at(j)<c2->weightPercentage){
                inverseConstrTwoActivitiesOrderedPercentages[sai][j]=c2->weightPercentage;
            }
        }
}

bool computeActivityEndsStudentsDayPercentages(QWidget* parent)
{
    bool ok=true;

    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++)
        activityEndsStudentsDayPercentages[ai]=-1;

    haveActivityEndsStudentsDay=false;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITY_ENDS_STUDENTS_DAY){
            haveActivityEndsStudentsDay=true;

            ConstraintActivityEndsStudentsDay* cae=(ConstraintActivityEndsStudentsDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cae->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
                 "activity activity ends students day for activity with id==%1 with weight percentage under 100%. "
                 "Constraint activity ends students day can only have weight percentage 100%. "
                 "Please modify your data accordingly (remove or edit constraint) and try again.")
                 .arg(TContext::get()->instance.activeActivitiesList[cae->activityIndex]->id),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                1, 0 );

                if(t==0)
                    break;
            }

            int ai=cae->activityIndex;
            if(activityEndsStudentsDayPercentages[ai] < cae->weightPercentage)
                activityEndsStudentsDayPercentages[ai] = cae->weightPercentage;
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY){
            haveActivityEndsStudentsDay=true;

            ConstraintActivitiesEndStudentsDay* cae=(ConstraintActivitiesEndStudentsDay*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cae->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraints of type "
                 "activities end students day with weight percentage under 100%. "
                 "Constraint activities end students day can only have weight percentage 100%. "
                 "Please modify your data accordingly (remove or edit constraint) and try again."),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                1, 0 );

                if(t==0)
                    break;
            }

            for(int i=0; i<cae->nActivities; i++){
                int ai=cae->activitiesIndices[i];
                if(activityEndsStudentsDayPercentages[ai] < cae->weightPercentage)
                    activityEndsStudentsDayPercentages[ai] = cae->weightPercentage;
            }
        }
    }

    return ok;
}

bool checkMinDays100Percent(QWidget* parent)
{
    bool ok=true;

    //int daysTeacherIsAvailable[MAX_TEACHERS];

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        daysTeacherIsAvailable[tc]=0;

        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            bool dayAvailable=false;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h]){
                    dayAvailable=true;
                    break;
                }

            if(dayAvailable)
                daysTeacherIsAvailable[tc]++;
        }

        if(teachersMaxDaysPerWeekMaxDays[tc]>=0){ //it has compulsory 100% weight
            assert(IS_EQUAL(teachersMaxDaysPerWeekWeightPercentages[tc], 100));
            daysTeacherIsAvailable[tc]=min(daysTeacherIsAvailable[tc], teachersMaxDaysPerWeekMaxDays[tc]);
        }
    }

    //int daysSubgroupIsAvailable[MAX_TOTAL_SUBGROUPS];

    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        daysSubgroupIsAvailable[sb]=0;

        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            bool dayAvailable=false;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h]){
                    dayAvailable=true;
                    break;
                }

            if(dayAvailable)
                daysSubgroupIsAvailable[sb]++;
        }

        if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){ //it has compulsory 100% weight
            assert(IS_EQUAL(subgroupsMaxDaysPerWeekWeightPercentages[sb], 100));
            daysSubgroupIsAvailable[sb]=min(daysSubgroupIsAvailable[sb], subgroupsMaxDaysPerWeekMaxDays[sb]);
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES
         &&TContext::get()->instance.validTimeConstraintsList[i]->weightPercentage==100.0){
            ConstraintMinDaysBetweenActivities* md=(ConstraintMinDaysBetweenActivities*)TContext::get()->instance.validTimeConstraintsList[i];

            if(md->minDays>=1){
                int na=md->_n_activities;
                int nd=md->minDays;
                if((na-1)*nd+1 > TContext::get()->instance.nDaysPerWeek){
                    ok=false;

                    QString s=GeneratePreTranslate::tr("%1 cannot be respected because it contains %2 activities,"
                     " has weight 100% and has min number of days between activities=%3. The minimum required number of days per week for"
                     " that would be (nactivities-1)*mindays+1=%4, and you have only %5 days per week - impossible. Please correct this constraint.", "%1 is the detailed description of a constraint"
                    )
                     .arg(md->getDetailedDescription())
                     .arg(na)
                     .arg(nd)
                     .arg((na-1)*nd+1)
                     .arg(TContext::get()->instance.nDaysPerWeek)
                     ;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return ok;
                }
            }

            if(md->minDays>=1){
                //int requestedDaysForTeachers[MAX_TEACHERS];
                for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++)
                    requestedDaysForTeachers[tc]=0;
                //int requestedDaysForSubgroups[MAX_TOTAL_SUBGROUPS];
                for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++)
                    requestedDaysForSubgroups[sb]=0;

                for(int j=0; j<md->_n_activities; j++){
                    int ai=md->_activities[j];
                    //TODO:
//                    for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList[ai]->iTeachersList.count(); k++){
//                        int tc=Timetable::getInstance()->instance.activeActivitiesList[ai]->iTeachersList.at(k);
//                        requestedDaysForTeachers[tc]++;
//                    }
                    for(int k=0; k<TContext::get()->instance.activeActivitiesList[ai]->iSubgroupsList.count(); k++){
                        int sb=TContext::get()->instance.activeActivitiesList[ai]->iSubgroupsList.at(k);
                        requestedDaysForSubgroups[sb]++;
                    }
                }
                //TODO:
//                for(int tc=0; tc<Timetable::getInstance()->instance.teachersList.size(); tc++)
//                    if(requestedDaysForTeachers[tc]>daysTeacherIsAvailable[tc]){
//                        ok=false;

//                        QString s=GeneratePreTranslate::tr("%1 cannot be respected because teacher %2 has at most"
//                         " %3 available days from teacher not available, breaks and teacher max days per week."
//                         " Please lower the weight of this constraint to a value below 100% (it depends"
//                         " on your situation, if 0% is too little, make it 90%, 95% or even 99.75%."
//                         " Even a large weight should not slow down much the program."
//                         " A situation where you may need to make it larger than 0% is for instance if you have 5 activities with 4"
//                         " possible days. You want to spread them 1, 1, 1 and 2, not 2, 2 and 1)", "%1 is the detailed description of a constraint"
//                        )
//                         .arg(md->getDetailedDescription(Timetable::getInstance()->instance))
//                         .arg(Timetable::getInstance()->instance.teachersList[tc]->name)
//                         .arg(daysTeacherIsAvailable[tc]);

//                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
//                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
//                         1, 0 );

//                        if(t==0)
//                            return ok;
//                    }
                for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++)
                    if(requestedDaysForSubgroups[sb]>daysSubgroupIsAvailable[sb]){
                        ok=false;

                        QString s=GeneratePreTranslate::tr("%1 cannot be respected because subgroup %2 has at most"
                         " %3 available days from students set not available and breaks."
                         " Please lower the weight of this constraint to a value below 100% (it depends"
                         " on your situation, if 0% is too little, make it 90%, 95% or even 99.75%."
                         " Even a large weight should not slow down much the program."
                         " A situation where you may need to make it larger than 0% is for instance if you have 5 activities with 4"
                         " possible days. You want to spread them 1, 1, 1 and 2, not 2, 2 and 1)", "%1 is the detailed description of a constraint"
                         )
                         .arg(md->getDetailedDescription())
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
                         .arg(daysSubgroupIsAvailable[sb]);

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return ok;
                    }
            }
        }
    }

    return ok;
}

bool checkMinDaysConsecutiveIfSameDay(QWidget* parent)
{
    bool ok=true;

    //int daysTeacherIsAvailable[MAX_TEACHERS];

    for(int tc=0; tc<TContext::get()->instance.teachersList.size(); tc++){
        daysTeacherIsAvailable[tc]=0;

        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            bool dayAvailable=false;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.teacherNotAvailableDayHour[tc][d][h]){
                    dayAvailable=true;
                    break;
                }

            if(dayAvailable)
                daysTeacherIsAvailable[tc]++;
        }

        if(teachersMaxDaysPerWeekMaxDays[tc]>=0){ //it has compulsory 100% weight
            assert(IS_EQUAL(teachersMaxDaysPerWeekWeightPercentages[tc], 100));
            daysTeacherIsAvailable[tc]=min(daysTeacherIsAvailable[tc], teachersMaxDaysPerWeekMaxDays[tc]);
        }
    }

    //int daysSubgroupIsAvailable[MAX_TOTAL_SUBGROUPS];

    for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
        daysSubgroupIsAvailable[sb]=0;

        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
            bool dayAvailable=false;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
                if(!TContext::get()->instance.breakDayHour[d][h] && !TContext::get()->instance.subgroupNotAvailableDayHour[sb][d][h]){
                    dayAvailable=true;
                    break;
                }

            if(dayAvailable)
                daysSubgroupIsAvailable[sb]++;
        }

        if(subgroupsMaxDaysPerWeekMaxDays[sb]>=0){ //it has compulsory 100% weight
            assert(IS_EQUAL(subgroupsMaxDaysPerWeekWeightPercentages[sb], 100));
            daysSubgroupIsAvailable[sb]=min(daysSubgroupIsAvailable[sb], subgroupsMaxDaysPerWeekMaxDays[sb]);
        }
    }

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES){
            ConstraintMinDaysBetweenActivities* md=(ConstraintMinDaysBetweenActivities*)TContext::get()->instance.validTimeConstraintsList[i];
            if(md->consecutiveIfSameDay){
                //int nReqForTeacher[MAX_TEACHERS];
                //TODO:
//                for(int tc=0; tc<Timetable::getInstance()->instance.teachersList.size(); tc++)
//                    nReqForTeacher[tc]=0;
//                for(int j=0; j<md->_n_activities; j++){
//                    int ai=md->_activities[j];
//                    for(int k=0; k<Timetable::getInstance()->instance.activeActivitiesList[ai]->iTeachersList.count(); k++){
//                        int tc=Timetable::getInstance()->instance.activeActivitiesList[ai]->iTeachersList.at(k);
//                        nReqForTeacher[tc]++;
//                    }
//                }

//                for(int tc=0; tc<Timetable::getInstance()->instance.teachersList.size(); tc++){
//                    if(2*daysTeacherIsAvailable[tc] < nReqForTeacher[tc]){
//                        ok=false;

//                        QString s=GeneratePreTranslate::tr("%1 cannot be respected because teacher %2 has at most"
//                         " %3 available days. You specified for this constraint consecutive if same day=true."
//                         " Currently m-FET cannot put more than 2 activities in the same day"
//                         " if consecutive if same day is true. You have 2*available days<number of activities in this constraint."
//                         " This is a very unlikely situation, that is why I didn't care too much about it."
//                         " If you encounter it, please please modify your file (uncheck consecutive if same day"
//                         " or add other activities with larger duration).", "%1 is the detailed description of a constraint"
//                        )
//                         .arg(md->getDetailedDescription(Timetable::getInstance()->instance))
//                         .arg(Timetable::getInstance()->instance.teachersList[tc]->name)
//                         .arg(daysTeacherIsAvailable[tc]);

//                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
//                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
//                         1, 0 );

//                        if(t==0)
//                            return ok;
//                    }
//                }

                //int nReqForSubgroup[MAX_TOTAL_SUBGROUPS];
                for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++)
                    nReqForSubgroup[sb]=0;
                for(int j=0; j<md->_n_activities; j++){
                    int ai=md->_activities[j];
                    for(int k=0; k<TContext::get()->instance.activeActivitiesList[ai]->iSubgroupsList.count(); k++){
                        int sb=TContext::get()->instance.activeActivitiesList[ai]->iSubgroupsList.at(k);
                        nReqForSubgroup[sb]++;
                    }
                }

                for(int sb=0; sb<TContext::get()->instance.directSubgroupsList.size(); sb++){
                    if(2*daysSubgroupIsAvailable[sb] < nReqForSubgroup[sb]){
                        ok=false;

                        QString s=GeneratePreTranslate::tr("%1 cannot be respected because subgroup %2 has at most"
                         " %3 available days. You specified for this constraint consecutive if same day=true."
                         " Currently m-FET cannot put more than 2 activities in the same day"
                         " if consecutive if same day is true. You have 2*available days<number of activities in this constraint."
                         " This is a very unlikely situation, that is why I didn't care too much about it."
                         " If you encounter it, please modify your file (uncheck consecutive if same day"
                         " or add other activities with larger duration).", "%1 is the detailed description of a constraint"
                        )
                         .arg(md->getDetailedDescription())
                         .arg(TContext::get()->instance.directSubgroupsList[sb]->name)
                         .arg(daysSubgroupIsAvailable[sb]);

                        int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
                         GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                         1, 0 );

                        if(t==0)
                            return ok;
                    }
                }
            }
        }
    }

    return ok;
}

bool computeTeachersIntervalMaxDaysPerWeek(QWidget* parent)
{
    for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
        teachersIntervalMaxDaysPerWeekPercentages1[i]=-1.0;
        teachersIntervalMaxDaysPerWeekMaxDays1[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalStart1[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalEnd1[i]=-1;

        teachersIntervalMaxDaysPerWeekPercentages2[i]=-1.0;
        teachersIntervalMaxDaysPerWeekMaxDays2[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalStart2[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalEnd2[i]=-1;

        teachersIntervalMaxDaysPerWeekPercentages3[i]=-1.0;
        teachersIntervalMaxDaysPerWeekMaxDays3[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalStart3[i]=-1;
        teachersIntervalMaxDaysPerWeekIntervalEnd3[i]=-1;
    }

    bool ok=true;
    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintTeacherIntervalMaxDaysPerWeek* tn=(ConstraintTeacherIntervalMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teacher interval max days per week with"
                 " weight (percentage) below 100 for teacher %1. Starting with m-FET version 5.6.2 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(tn->teacherName),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            if(teachersIntervalMaxDaysPerWeekPercentages1[tn->teacher_ID] < 0){
                teachersIntervalMaxDaysPerWeekPercentages1[tn->teacher_ID]=tn->weightPercentage;
                teachersIntervalMaxDaysPerWeekMaxDays1[tn->teacher_ID]=tn->maxDaysPerWeek;
                teachersIntervalMaxDaysPerWeekIntervalStart1[tn->teacher_ID]=tn->startHour;
                teachersIntervalMaxDaysPerWeekIntervalEnd1[tn->teacher_ID]=tn->endHour;
            }
            else if(teachersIntervalMaxDaysPerWeekPercentages2[tn->teacher_ID] < 0){
                teachersIntervalMaxDaysPerWeekPercentages2[tn->teacher_ID]=tn->weightPercentage;
                teachersIntervalMaxDaysPerWeekMaxDays2[tn->teacher_ID]=tn->maxDaysPerWeek;
                teachersIntervalMaxDaysPerWeekIntervalStart2[tn->teacher_ID]=tn->startHour;
                teachersIntervalMaxDaysPerWeekIntervalEnd2[tn->teacher_ID]=tn->endHour;
            }
            else if(teachersIntervalMaxDaysPerWeekPercentages3[tn->teacher_ID] < 0){
                teachersIntervalMaxDaysPerWeekPercentages3[tn->teacher_ID]=tn->weightPercentage;
                teachersIntervalMaxDaysPerWeekMaxDays3[tn->teacher_ID]=tn->maxDaysPerWeek;
                teachersIntervalMaxDaysPerWeekIntervalStart3[tn->teacher_ID]=tn->startHour;
                teachersIntervalMaxDaysPerWeekIntervalEnd3[tn->teacher_ID]=tn->endHour;
            }
            else{
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize for teacher %1, because it has more than three constraints interval max days per week"
                 ". Please modify your data correspondingly (leave maximum three constraints of type"
                 " constraint teacher(s) interval max days per week for each teacher) and try again")
                 .arg(TContext::get()->instance.teachersList[tn->teacher_ID]->name),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_TEACHERS_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintTeachersIntervalMaxDaysPerWeek* tn=(ConstraintTeachersIntervalMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(tn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint teachers interval max days per week with"
                 " weight (percentage) below 100. Starting with m-FET version 5.6.2 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again"),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            for(int tch=0; tch<TContext::get()->instance.teachersList.size(); tch++){
                if(teachersIntervalMaxDaysPerWeekPercentages1[tch] < 0){
                    teachersIntervalMaxDaysPerWeekPercentages1[tch]=tn->weightPercentage;
                    teachersIntervalMaxDaysPerWeekMaxDays1[tch]=tn->maxDaysPerWeek;
                    teachersIntervalMaxDaysPerWeekIntervalStart1[tch]=tn->startHour;
                    teachersIntervalMaxDaysPerWeekIntervalEnd1[tch]=tn->endHour;
                }
                else if(teachersIntervalMaxDaysPerWeekPercentages2[tch] < 0){
                    teachersIntervalMaxDaysPerWeekPercentages2[tch]=tn->weightPercentage;
                    teachersIntervalMaxDaysPerWeekMaxDays2[tch]=tn->maxDaysPerWeek;
                    teachersIntervalMaxDaysPerWeekIntervalStart2[tch]=tn->startHour;
                    teachersIntervalMaxDaysPerWeekIntervalEnd2[tch]=tn->endHour;
                }
                else if(teachersIntervalMaxDaysPerWeekPercentages3[tch] < 0){
                    teachersIntervalMaxDaysPerWeekPercentages3[tch]=tn->weightPercentage;
                    teachersIntervalMaxDaysPerWeekMaxDays3[tch]=tn->maxDaysPerWeek;
                    teachersIntervalMaxDaysPerWeekIntervalStart3[tch]=tn->startHour;
                    teachersIntervalMaxDaysPerWeekIntervalEnd3[tch]=tn->endHour;
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for teacher %1, because it has more than three constraints interval max days per week"
                     ". Please modify your data correspondingly (leave maximum three constraints of type"
                     " constraint teacher(s) interval max days per week for each teacher) and try again")
                     .arg(TContext::get()->instance.teachersList[tch]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
    }

    return ok;
}

bool computeSubgroupsIntervalMaxDaysPerWeek(QWidget* parent)
{
    for(int i=0; i<TContext::get()->instance.directSubgroupsList.size(); i++){
        subgroupsIntervalMaxDaysPerWeekPercentages1[i]=-1.0;
        subgroupsIntervalMaxDaysPerWeekMaxDays1[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalStart1[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalEnd1[i]=-1;

        subgroupsIntervalMaxDaysPerWeekPercentages2[i]=-1.0;
        subgroupsIntervalMaxDaysPerWeekMaxDays2[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalStart2[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalEnd2[i]=-1;

        subgroupsIntervalMaxDaysPerWeekPercentages3[i]=-1.0;
        subgroupsIntervalMaxDaysPerWeekMaxDays3[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalStart3[i]=-1;
        subgroupsIntervalMaxDaysPerWeekIntervalEnd3[i]=-1;
    }

    bool ok=true;

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintStudentsSetIntervalMaxDaysPerWeek* cn=(ConstraintStudentsSetIntervalMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students set interval max days per week with"
                 " weight (percentage) below 100 for students set %1. Starting with m-FET version 5.6.2 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 .arg(cn->students),
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            foreach(int sbg, cn->iSubgroupsList){
                if(subgroupsIntervalMaxDaysPerWeekPercentages1[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages1[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays1[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart1[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd1[sbg]=cn->endHour;
                }
                else if(subgroupsIntervalMaxDaysPerWeekPercentages2[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages2[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays2[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart2[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd2[sbg]=cn->endHour;
                }
                else if(subgroupsIntervalMaxDaysPerWeekPercentages3[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages3[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays3[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart3[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd3[sbg]=cn->endHour;
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because it has more than three constraints interval max days per week"
                     ". Please modify your data correspondingly (leave maximum three constraints of type"
                     " constraint students (set) interval max days per week for each subgroup) and try again")
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
        else if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_STUDENTS_INTERVAL_MAX_DAYS_PER_WEEK){
            ConstraintStudentsIntervalMaxDaysPerWeek* cn=(ConstraintStudentsIntervalMaxDaysPerWeek*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cn->weightPercentage < 100){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint students interval max days per week with"
                 " weight (percentage) below 100. Starting with m-FET version 5.6.2 it is only possible"
                 " to use 100% weight for such constraints. Please make weight 100% and try again")
                 //.arg(cn->students),
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            for(int sbg=0; sbg<TContext::get()->instance.directSubgroupsList.size(); sbg++){
            //foreach(int sbg, cn->iSubgroupsList){
                if(subgroupsIntervalMaxDaysPerWeekPercentages1[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages1[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays1[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart1[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd1[sbg]=cn->endHour;
                }
                else if(subgroupsIntervalMaxDaysPerWeekPercentages2[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages2[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays2[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart2[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd2[sbg]=cn->endHour;
                }
                else if(subgroupsIntervalMaxDaysPerWeekPercentages3[sbg] < 0){
                    subgroupsIntervalMaxDaysPerWeekPercentages3[sbg]=cn->weightPercentage;
                    subgroupsIntervalMaxDaysPerWeekMaxDays3[sbg]=cn->maxDaysPerWeek;
                    subgroupsIntervalMaxDaysPerWeekIntervalStart3[sbg]=cn->startHour;
                    subgroupsIntervalMaxDaysPerWeekIntervalEnd3[sbg]=cn->endHour;
                }
                else{
                    ok=false;

                    int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                     GeneratePreTranslate::tr("Cannot optimize for subgroup %1, because it has more than three constraints interval max days per week"
                     ". Please modify your data correspondingly (leave maximum three constraints of type"
                     " constraint students (set) interval max days per week for each subgroup) and try again")
                     .arg(TContext::get()->instance.directSubgroupsList[sbg]->name),
                     GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                     1, 0 );

                    if(t==0)
                        return false;
                }
            }
        }
    }

    return ok;
}

//2011-09-25
bool computeActivitiesOccupyMaxTimeSlotsFromSelection(QWidget* parent)
{
    bool ok=true;

    aomtsList.clear();
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++)
        aomtsListForActivity[i].clear();

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_OCCUPY_MAX_TIME_SLOTS_FROM_SELECTION){
            if(!haveActivitiesOccupyOrSimultaneousConstraints)
                haveActivitiesOccupyOrSimultaneousConstraints=true;

            ConstraintActivitiesOccupyMaxTimeSlotsFromSelection* cn=(ConstraintActivitiesOccupyMaxTimeSlotsFromSelection*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cn->weightPercentage!=100.0){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint(s) of type 'activities occupy max time slots from selection'"
                 " with weight (percentage) below 100.0%. Please make the weight 100.0% and try again")
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            ActivitiesOccupyMaxTimeSlotsFromSelection_item item;
            item.activitiesList=cn->_activitiesIndices;
            item.activitiesSet=item.activitiesList.toSet();
            item.maxOccupiedTimeSlots=cn->maxOccupiedTimeSlots;
            for(int t=0; t < cn->selectedDays.count(); t++)
                item.selectedTimeSlotsList.append(cn->selectedDays.at(t)+cn->selectedHours.at(t)*TContext::get()->instance.nDaysPerWeek);
            item.selectedTimeSlotsSet=item.selectedTimeSlotsList.toSet();

            aomtsList.append(item);
            ActivitiesOccupyMaxTimeSlotsFromSelection_item* p_item=&aomtsList[aomtsList.count()-1];
            foreach(int ai, cn->_activitiesIndices)
                aomtsListForActivity[ai].append(p_item);
        }
    }

    return ok;
}

//2011-09-30
bool computeActivitiesMaxSimultaneousInSelectedTimeSlots(QWidget* parent)
{
    bool ok=true;

    amsistsList.clear();
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++)
        amsistsListForActivity[i].clear();

    for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
        if(TContext::get()->instance.validTimeConstraintsList[i]->type==CONSTRAINT_ACTIVITIES_MAX_SIMULTANEOUS_IN_SELECTED_TIME_SLOTS){
            if(!haveActivitiesOccupyOrSimultaneousConstraints)
                haveActivitiesOccupyOrSimultaneousConstraints=true;

            ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots* cn=(ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots*)TContext::get()->instance.validTimeConstraintsList[i];

            if(cn->weightPercentage!=100.0){
                ok=false;

                int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"),
                 GeneratePreTranslate::tr("Cannot optimize, because you have constraint(s) of type 'activities max simultaneous in selected time slots'"
                 " with weight (percentage) below 100.0%. Please make the weight 100.0% and try again")
                 ,
                 GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
                 1, 0 );

                if(t==0)
                    return false;
            }

            ActivitiesMaxSimultaneousInSelectedTimeSlots_item item;
            item.activitiesList=cn->_activitiesIndices;
            item.activitiesSet=item.activitiesList.toSet();
            item.maxSimultaneous=cn->maxSimultaneous;
            for(int t=0; t < cn->selectedDays.count(); t++)
                item.selectedTimeSlotsList.append(cn->selectedDays.at(t)+cn->selectedHours.at(t)*TContext::get()->instance.nDaysPerWeek);
            item.selectedTimeSlotsSet=item.selectedTimeSlotsList.toSet();

            amsistsList.append(item);
            ActivitiesMaxSimultaneousInSelectedTimeSlots_item* p_item=&amsistsList[amsistsList.count()-1];
            foreach(int ai, cn->_activitiesIndices)
                amsistsListForActivity[ai].append(p_item);
        }
    }

    return ok;
}

//void computeMustComputeTimetableSubgroups()
//{
//    for(int sbg=0; sbg<Timetable::getInstance()->instance.directSubgroupsList.size(); sbg++)
//        mustComputeTimetableSubgroup[sbg]=false;

//    for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
//        Activity* act=Timetable::getInstance()->instance.activeActivitiesList[ai];

//        mustComputeTimetableSubgroups[ai].clear();

//        foreach(int sbg, act->iSubgroupsList)
//            if(subgroupsMaxGapsPerWeekPercentage[sbg]>=0 ||
//              subgroupsMaxGapsPerDayPercentage[sbg]>=0 ||
//              subgroupsEarlyMaxBeginningsAtSecondHourPercentage[sbg]>=0 ||
//              subgroupsMaxHoursDailyPercentages1[sbg]>=0 ||
//              subgroupsMaxHoursDailyPercentages2[sbg]>=0 ||
//              subgroupsMaxHoursContinuouslyPercentages1[sbg]>=0 ||
//              subgroupsMaxHoursContinuouslyPercentages2[sbg]>=0 ||
//              subgroupsMinHoursDailyPercentages[sbg]>=0 ||

//              subgroupsMaxSpanPerDayPercentages[sbg]>=0 ||
//              subgroupsMinRestingHoursCircularPercentages[sbg]>=0 ||
//              subgroupsMinRestingHoursNotCircularPercentages[sbg]>=0 ||

//              subgroupsIntervalMaxDaysPerWeekPercentages1[sbg]>=0 ||
//              subgroupsIntervalMaxDaysPerWeekPercentages2[sbg]>=0 ||
//              subgroupsIntervalMaxDaysPerWeekPercentages3[sbg]>=0 ||

//              subgroupsActivityTagMaxHoursContinuouslyPercentage[sbg].count()>0 ||
//              subgroupsActivityTagMaxHoursDailyPercentage[sbg].count()>0
//              ){

//                mustComputeTimetableSubgroups[ai].append(sbg);
////                mustComputeTimetableSubgroup[sbg]=true;
//            }
//    }
//}

//void computeMustComputeTimetableTeachers()
//{
//    for(int tch=0; tch<Timetable::getInstance()->instance.teachersList.size(); tch++)
//        mustComputeTimetableTeacher[tch]=false;

//    for(int ai=0; ai<Timetable::getInstance()->instance.activeActivitiesList.size(); ai++){
//        Activity* act=Timetable::getInstance()->instance.activeActivitiesList[ai];

//        mustComputeTimetableTeachers[ai].clear();
//TODO: !!!
//        foreach(int tch, act->iTeachersList)
//            if(teachersMaxGapsPerWeekPercentage[tch]>=0 ||
//              teachersMaxGapsPerDayPercentage[tch]>=0 ||
//              teachersMaxHoursDailyPercentages1[tch]>=0 ||
//              teachersMaxHoursDailyPercentages2[tch]>=0 ||
//              teachersMaxHoursContinuouslyPercentages1[tch]>=0 ||
//              teachersMaxHoursContinuouslyPercentages2[tch]>=0 ||
//              teachersMinHoursDailyPercentages[tch]>=0 ||

//              teachersMinDaysPerWeekPercentages[tch]>=0 ||

//              teachersMaxSpanPerDayPercentages[tch]>=0 ||
//              teachersMinRestingHoursCircularPercentages[tch]>=0 ||
//              teachersMinRestingHoursNotCircularPercentages[tch]>=0 ||

//              teachersIntervalMaxDaysPerWeekPercentages1[tch]>=0 ||
//              teachersIntervalMaxDaysPerWeekPercentages2[tch]>=0 ||
//              teachersIntervalMaxDaysPerWeekPercentages3[tch]>=0 ||

//              teachersActivityTagMaxHoursContinuouslyPercentage[tch].count()>0 ||
//              teachersActivityTagMaxHoursDailyPercentage[tch].count()>0
//              ){

//                mustComputeTimetableTeachers[ai].append(tch);
//                mustComputeTimetableTeacher[tch]=true;
//            }
//    }
//}

bool computeFixedActivities(QWidget* parent)
{
    bool ok=true;

    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
        int notAllowedSlots=0;
        for(int tim=0; tim<TContext::get()->instance.nHoursPerWeek; tim++)
            if(IS_EQUAL(notAllowedTimesPercentages[ai][tim], 100))
                notAllowedSlots++;

        if(notAllowedSlots==TContext::get()->instance.nHoursPerWeek){
            ok=false;

            QString s=GeneratePreTranslate::tr("Activity with id=%1 has no allowed slot - please correct that").arg(TContext::get()->instance.activeActivitiesList[ai]->id);
            int t=MessagesManager::confirmation(parent, GeneratePreTranslate::tr("m-FET warning"), s,
             GeneratePreTranslate::tr("Skip rest"), GeneratePreTranslate::tr("See next"), QString(),
             1, 0 );

            if(t==0)
                return false;
        }
        else if(notAllowedSlots==TContext::get()->instance.nHoursPerWeek-1)
            fixedTimeActivity[ai]=true;
        else
            fixedTimeActivity[ai]=false;
    }

    return ok;
}

/*
File timeconstraint.cpp
*/

/***************************************************************************
                          timeconstraint.cpp  -  description
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

#include "timeconstraint.h"

#include <QMetaEnum>
#include <QSet>
#include <algorithm> //for min max functions

#include "centerwidgetonscreen.h"

#include "textmessages.h"
#include "defs.h"
#include "matrix.h"

#include "instance.h"
#include "solution.h"
#include "activity.h"
#include "teacher.h"
#include "subject.h"
#include "studentsset.h"
#include "activitytag.h"

#include "enumutils.h"
#include "stringutils.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

TimeConstraint::TimeConstraint(Instance& r) : r(r)
{
    type=CONSTRAINT_GENERIC_TIME;

    active=true;
    comments=QString("");
}

TimeConstraint::~TimeConstraint()
{
}

TimeConstraint::TimeConstraint(Instance& r, Enums::ConstraintGroup constraintGroup, double wp) : r(r)
{
    type=CONSTRAINT_GENERIC_TIME;

    this->setConstraintGroup(constraintGroup);

    assert(wp<=100 && wp>=0);

    active=true;
    comments=QString("");
}

TimeConstraint::TimeConstraint(Instance& r, Enums::ConstraintGroup constraintGroup) : r(r)
{
    type=CONSTRAINT_GENERIC_TIME;

    this->setConstraintGroup(constraintGroup);

    active=true;
    comments=QString("");
}

QString TimeConstraint::getSuperXmlDescription()
{
    QString s = "";
    s+="	<Group>"+ utils::enums::enumIndexToStr(this->constraintGroup()) +"</Group>\n";
    return s;
}

QString TimeConstraint::getSuperDescription()
{
    QString s = "";
    s+=tr("G: %1", "Constraint group").arg(utils::enums::enumIndexToStr(this->constraintGroup()));
    return s;
}

QString TimeConstraint::getSuperDetailedDescription()
{
    QString s = "";
    s+=tr("Constraint group: %1", "Constraint group").arg(utils::enums::enumIndexToStr(this->constraintGroup()));
    return s;
}

void TimeConstraint::setConstraintGroup(Enums::ConstraintGroup constraintGroup)
{
    this->group = constraintGroup;

    switch (constraintGroup) {
    case Enums::ConstraintGroup::Essential:
        this->weightPercentage = 100;
        break;
    case Enums::ConstraintGroup::Important:
        this->weightPercentage = 80;
        break;
    case Enums::ConstraintGroup::Desirable:
        this->weightPercentage = 60;
        break;
    default:
        assert(0);
        break;
    }
}

Enums::ConstraintGroup TimeConstraint::constraintGroup() const
{
    return group;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

ConstraintBasicCompulsoryTime::ConstraintBasicCompulsoryTime(Instance& r): TimeConstraint(r)
{
    this->type=CONSTRAINT_BASIC_COMPULSORY_TIME;
    this->setConstraintGroup(Enums::ConstraintGroup::Essential);
}

ConstraintBasicCompulsoryTime::ConstraintBasicCompulsoryTime(Instance& r, Enums::ConstraintGroup constraintGroup, double wp): TimeConstraint(r, constraintGroup, wp)
{
    this->type=CONSTRAINT_BASIC_COMPULSORY_TIME;
}

bool ConstraintBasicCompulsoryTime::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintBasicCompulsoryTime::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintBasicCompulsoryTime::getXmlDescription()
{
    Q_UNUSED(r);

    QString s = "<ConstraintBasicCompulsoryTime>\n";
    assert(this->weightPercentage==100.0);
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintBasicCompulsoryTime>\n";
    return s;
}

QString ConstraintBasicCompulsoryTime::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    return begin+tr("Basic compulsory constraints (time)") + ", " + getSuperDescription() + ", " + tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage))+end;
}

QString ConstraintBasicCompulsoryTime::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("These are the basic compulsory constraints (referring to time allocation) for any timetable");
    s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("The basic time constraints try to avoid:");s+="\n";
    s+=QString("- ");s+=tr("teachers assigned to more than one activity simultaneously");s+="\n";
    s+=QString("- ");s+=tr("students assigned to more than one activity simultaneously");s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintBasicCompulsoryTime::violationsFactor(Solution& c, bool collectConflictsData){
    assert(r.internalStructureComputed);
    assert(group==Enums::ConstraintGroup::Essential);

    int teachersConflicts, subgroupsConflicts;

    subgroupsConflicts = c.computeSubgroupsMatrix(collectConflictsData);
    teachersConflicts = c.computeTeachersMatrix(collectConflictsData);

    assert(c.subgroups_conflicts>=0);
    assert(c.teachers_conflicts>=0);

    int i,dd;

    qint64 unallocated; //unallocated activities
    int late; //late activities
    int nte; //number of teacher exhaustions
    int nse; //number of students exhaustions

    //Unallocated or late activities
    unallocated=0;
    late=0;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        if(c.times[(unsigned)i]==UNALLOCATED_TIME){
            //Firstly, we consider a big clash each unallocated activity.
            //Needs to be very a large constant, bigger than any other broken constraint.
            //Take care: MAX_ACTIVITIES*this_constant <= INT_MAX
            int parcialViolation = /*r.internalActivitiesList[i].duration * r.internalActivitiesList[i].nSubgroups * */ 10000;

            unallocated += parcialViolation;
            //(an unallocated activity for a year is more important than an unallocated activity for a subgroup)

            if(collectConflictsData){
                QString s= tr("Time constraint basic compulsory broken: unallocated activity with id=%1 (%2)",
                              "%2 is the detailed description of activity - teachers, subject, students")
                        .arg(r.activeActivitiesList[i]->id).arg(r.getActivityDetailedDescription(r.activeActivitiesList[i]->id));
                s+=" - ";
                s += tr("this increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
        else{
            //Calculates the number of activities that are scheduled too late (in fact we
            //calculate a function that increases as the activity is getting late)
            int h=c.getHourForActivityIndex(i);
            dd=r.activeActivitiesList[i]->duration;
            if(h+dd>r.nHoursPerDay){
                int tmp;
                tmp=1;

                int parcialViolation = (h+dd-r.nHoursPerDay) * tmp * r.activeActivitiesList[i]->iSubgroupsList.count();
                //multiplied with the number of subgroups implied, for seeing the importance of the activity

                late += parcialViolation;

                if(collectConflictsData){
                    QString s=tr("Time constraint basic compulsory");
                    s+=": ";
                    s+=tr("activity with id=%1 is late.").arg(r.activeActivitiesList[i]->id);
                    s+=" ";
                    s+=tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));
                    s+="\n";

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    //Calculates the number of teachers exhaustion (when he has to teach more than one activity at the same time)
    nte = teachersConflicts;

    //Calculates the number of subgroups exhaustion (a subgroup cannot attend two activities at the same time)
    nse = subgroupsConflicts;

//    return weightPercentage/100 * (unallocated + qint64(late) + qint64(nte) + qint64(nse)); //conflicts factor
    return (qint64(late) + qint64(nte) + qint64(nse) + unallocated);
}

bool ConstraintBasicCompulsoryTime::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);
    Q_UNUSED(r);

    return false;
}

bool ConstraintBasicCompulsoryTime::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintBasicCompulsoryTime::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintBasicCompulsoryTime::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintBasicCompulsoryTime::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintBasicCompulsoryTime::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintBasicCompulsoryTime::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintBasicCompulsoryTime::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherNotAvailableTimes::ConstraintTeacherNotAvailableTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES;
}

ConstraintTeacherNotAvailableTimes::ConstraintTeacherNotAvailableTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, const QString& tn, QList<int> d, QList<int> h)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->teacher=tn;
    assert(d.count()==h.count());
    this->days=d;
    this->hours=h;
    this->type=CONSTRAINT_TEACHER_NOT_AVAILABLE_TIMES;
}

QString ConstraintTeacherNotAvailableTimes::getXmlDescription(){
    QString s="<ConstraintTeacherNotAvailableTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher>"+utils::strings::parseStrForXml(this->teacher)+"</Teacher>\n";

    s+="	<Number_of_Not_Available_Times>"+utils::strings::number(this->days.count())+"</Number_of_Not_Available_Times>\n";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        s+="	<Not_Available_Time>\n";
        if(this->days.at(i)>=0)
            s+="		<Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days.at(i)])+"</Day>\n";
        if(this->hours.at(i)>=0)
            s+="		<Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours.at(i)])+"</Hour>\n";
        s+="	</Not_Available_Time>\n";
    }

    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherNotAvailableTimes>\n";
    return s;
}

QString ConstraintTeacherNotAvailableTimes::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Teacher not available");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacher);s+=", ";

    s+=tr("NA at:", "Not available at");
    s+=" ";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }

    return begin+s+end;
}

QString ConstraintTeacherNotAvailableTimes::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher is not available");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacher);s+="\n";

    s+=tr("Not available at:", "It refers to a teacher");
    s+="\n";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintTeacherNotAvailableTimes::computeInternalStructure(QWidget* parent){
    //this->teacher_ID=r.searchTeacher(this->teacher);
    teacher_ID=r.getTeacherIndex(teacher);

    if(this->teacher_ID<0){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teacher not available times is wrong because it refers to inexistent teacher."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(days.count()==hours.count());
    for(int k=0; k<days.count(); k++){
        if(this->days.at(k) >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint teacher not available times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours.at(k) >= r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint teacher not available times is wrong because an hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }

    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherNotAvailableTimes::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

double ConstraintTeacherNotAvailableTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //Calculates the number of hours when the teacher is supposed to be teaching, but he is not available
    //This function consideres all the hours, I mean if there are for example 5 weekly courses
    //scheduled on that hour (which is already a broken compulsory restriction - we only
    //are allowed 1 weekly course for a certain teacher at a certain hour) we calculate
    //5 broken restrictions for that function.
    //TODO: decide if it is better to consider only 2 or 10 as a return value in this particular case
    //(currently it is 10)
    int tch=this->teacher_ID;

    int nbroken;

    nbroken=0;

    assert(days.count()==hours.count());
    for(int k=0; k<days.count(); k++){
        int d=days.at(k);
        int h=hours.at(k);

        if(c.teachersMatrix[tch][d][h]>0){
            int parcialViolation = c.teachersMatrix[tch][d][h];

            nbroken+=parcialViolation;

            if(collectConflictsData){
                QString s= tr("Time constraint teacher not available");
                s += " ";
                s += tr("broken for teacher: %1 on day %2, hour %3")
                        .arg(r.teachersList.at(tch)->name)
                        .arg(r.daysOfTheWeek[d])
                        .arg(r.hoursOfTheDay[h]);
                s += ". ";
                s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeacherNotAvailableTimes::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);
    Q_UNUSED(r);

    return false;
}

bool ConstraintTeacherNotAvailableTimes::isRelatedToTeacher(Teacher* t)
{
    if(this->teacher==t->name)
        return true;
    return false;
}

bool ConstraintTeacherNotAvailableTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherNotAvailableTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherNotAvailableTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherNotAvailableTimes::hasWrongDayOrHour()
{
    assert(days.count()==hours.count());

    for(int i=0; i<days.count(); i++)
        if(days.at(i)<0 || days.at(i)>=r.nDaysPerWeek
                || hours.at(i)<0 || hours.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintTeacherNotAvailableTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherNotAvailableTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(days.count()==hours.count());

    QList<int> newDays;
    QList<int> newHours;

    for(int i=0; i<days.count(); i++)
        if(days.at(i)>=0 && days.at(i)<r.nDaysPerWeek
                && hours.at(i)>=0 && hours.at(i)<r.nHoursPerDay){
            newDays.append(days.at(i));
            newHours.append(hours.at(i));
        }

    days=newDays;
    hours=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetNotAvailableTimes::ConstraintStudentsSetNotAvailableTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES;
}

ConstraintStudentsSetNotAvailableTimes::ConstraintStudentsSetNotAvailableTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, const QString& sn, QList<int> d, QList<int> h)
    : TimeConstraint(r, constraintGroup, wp){
    this->students = sn;
    assert(d.count()==h.count());
    this->days=d;
    this->hours=h;
    this->type=CONSTRAINT_STUDENTS_SET_NOT_AVAILABLE_TIMES;
}

bool ConstraintStudentsSetNotAvailableTimes::computeInternalStructure(QWidget* parent){
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set not available is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(days.count()==hours.count());
    for(int k=0; k<days.count(); k++){
        if(this->days.at(k) >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint students set not available times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours.at(k) >= r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint students set not available times is wrong because an hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);
    return true;
}

bool ConstraintStudentsSetNotAvailableTimes::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetNotAvailableTimes::getXmlDescription(){
    QString s="<ConstraintStudentsSetNotAvailableTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";

    s+="	<Number_of_Not_Available_Times>"+utils::strings::number(this->days.count())+"</Number_of_Not_Available_Times>\n";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        s+="	<Not_Available_Time>\n";
        if(this->days.at(i)>=0)
            s+="		<Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days.at(i)])+"</Day>\n";
        if(this->hours.at(i)>=0)
            s+="		<Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours.at(i)])+"</Hour>\n";
        s+="	</Not_Available_Time>\n";
    }

    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetNotAvailableTimes>\n";
    return s;
}

QString ConstraintStudentsSetNotAvailableTimes::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s=tr("Students set not available");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students").arg(this->students);s+=", ";

    s+=tr("NA at:", "Not available at");
    s+=" ";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }

    return begin+s+end;
}

QString ConstraintStudentsSetNotAvailableTimes::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set is not available");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("Students=%1").arg(this->students);s+="\n";

    s+=tr("Not available at:", "It refers to a students set");s+="\n";

    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetNotAvailableTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    for(int m=0; m<this->iSubgroupsList.count(); m++){
        int sbg=this->iSubgroupsList.at(m);

        assert(days.count()==hours.count());
        for(int k=0; k<days.count(); k++){
            int d=days.at(k);
            int h=hours.at(k);

            if(c.subgroupsMatrix[sbg][d][h]>0){
                int parcialViolation = c.subgroupsMatrix[sbg][d][h];

                nbroken+=parcialViolation;

                if(collectConflictsData){
                    QString s= tr("Time constraint students set not available");
                    s += " ";
                    s += tr("broken for subgroup: %1 on day %2, hour %3")
                            .arg(r.directSubgroupsList[sbg]->name)
                            .arg(r.daysOfTheWeek[d])
                            .arg(r.hoursOfTheDay[h]);
                    s += ". ";
                    s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetNotAvailableTimes::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);
    Q_UNUSED(r);

    return false;
}

bool ConstraintStudentsSetNotAvailableTimes::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetNotAvailableTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetNotAvailableTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetNotAvailableTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetNotAvailableTimes::hasWrongDayOrHour()
{
    assert(days.count()==hours.count());

    for(int i=0; i<days.count(); i++)
        if(days.at(i)<0 || days.at(i)>=r.nDaysPerWeek
                || hours.at(i)<0 || hours.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintStudentsSetNotAvailableTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetNotAvailableTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(days.count()==hours.count());

    QList<int> newDays;
    QList<int> newHours;

    for(int i=0; i<days.count(); i++)
        if(days.at(i)>=0 && days.at(i)<r.nDaysPerWeek
                && hours.at(i)>=0 && hours.at(i)<r.nHoursPerDay){
            newDays.append(days.at(i));
            newHours.append(hours.at(i));
        }

    days=newDays;
    hours=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesSameStartingTime::ConstraintActivitiesSameStartingTime(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME;
}

ConstraintActivitiesSameStartingTime::ConstraintActivitiesSameStartingTime(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& act)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    this->type=CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME;
}

bool ConstraintActivitiesSameStartingTime::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*int j;
                Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintActivitiesSameStartingTime::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintActivitiesSameStartingTime::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintActivitiesSameStartingTime::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintActivitiesSameStartingTime>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesSameStartingTime>\n";
    return s;
}

QString ConstraintActivitiesSameStartingTime::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Activities same starting time");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);
        if(i<this->n_activities-1)
            s+=", ";
    }

    return begin+s+end;
}

QString ConstraintActivitiesSameStartingTime::getDetailedDescription(){
    QString s;

    s=tr("Time constraint");s+="\n";
    s+=tr("Activities must have the same starting time");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity").arg(this->activitiesId[i]).arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesSameStartingTime::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the differences in the scheduled time for all pairs of activities.

    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int hour1=c.getHourForActivityIndex(this->_activities[i]);
            int day1=c.getDayForActivityIndex(this->_activities[i]);
            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int hour2=c.getHourForActivityIndex(this->_activities[j]);
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    int tmp=0;

                    tmp = abs(day1-day2) + abs(hour1-hour2);

                    if(tmp>0)
                        tmp=1;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tmp>0 && collectConflictsData){
                        QString s=tr("Time constraint activities same starting time broken, because activity with id=%1 (%2) is not at the same starting time with activity with id=%3 (%4)",
                                     "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]));
                        s+=". ";
                        s+=tr("Conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintActivitiesSameStartingTime::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintActivitiesSameStartingTime::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesSameStartingTime::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingTime::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingTime::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingTime::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivitiesSameStartingTime::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivitiesSameStartingTime::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesNotOverlapping::ConstraintActivitiesNotOverlapping(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING;
}

ConstraintActivitiesNotOverlapping::ConstraintActivitiesNotOverlapping(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& act)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    this->type=CONSTRAINT_ACTIVITIES_NOT_OVERLAPPING;
}

bool ConstraintActivitiesNotOverlapping::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*int j;
                Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintActivitiesNotOverlapping::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintActivitiesNotOverlapping::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintActivitiesNotOverlapping::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintActivitiesNotOverlapping>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesNotOverlapping>\n";
    return s;
}

QString ConstraintActivitiesNotOverlapping::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Activities not overlapping");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);
        if(i<this->n_activities-1)
            s+=", ";
    }

    return begin+s+end;
}

QString ConstraintActivitiesNotOverlapping::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activities must not overlap");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activitiesId[i]).arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesNotOverlapping::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the overlapping hours for all pairs of activities.
    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int hour1=c.getHourForActivityIndex(this->_activities[i]);
            int day1=c.getDayForActivityIndex(this->_activities[i]);
            int duration1=r.activeActivitiesList[this->_activities[i]]->duration;

            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int hour2=c.getHourForActivityIndex(this->_activities[j]);
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    int duration2=r.activeActivitiesList[this->_activities[j]]->duration;

                    //the number of overlapping hours
                    int tt=0;
                    if(day1==day2){
                        int start=max(hour1, hour2);
                        int stop=min(hour1+duration1, hour2+duration2);
                        if(stop>start)
                            tt+=stop-start;
                    }

                    //The overlapping hours, considering weekly activities more important than fortnightly ones
                    int tmp=tt;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tt>0 && collectConflictsData){

                        QString s=tr("Time constraint activities not overlapping broken: activity with id=%1 (%2) overlaps with activity with id=%3 (%4) on a number of %5 periods",
                                     "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]))
                                .arg(tt);
                        s+=", ";
                        s+=tr("conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintActivitiesNotOverlapping::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintActivitiesNotOverlapping::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesNotOverlapping::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesNotOverlapping::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesNotOverlapping::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesNotOverlapping::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivitiesNotOverlapping::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivitiesNotOverlapping::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintMinDaysBetweenActivities::ConstraintMinDaysBetweenActivities(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES;
}

ConstraintMinDaysBetweenActivities::ConstraintMinDaysBetweenActivities(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, bool cisd, int nact, const QList<int>& act, int n)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->consecutiveIfSameDay=cisd;

    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    assert(n>0);
    this->minDays=n;

    this->type=CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES;
}

bool ConstraintMinDaysBetweenActivities::operator==(ConstraintMinDaysBetweenActivities& c){
    assert(this->n_activities==this->activitiesId.count());
    assert(c.n_activities==c.activitiesId.count());

    if(this->n_activities!=c.n_activities)
        return false;
    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]!=c.activitiesId[i])
            return false;
    if(this->minDays!=c.minDays)
        return false;
    if(!IS_EQUAL(this->weightPercentage, c.weightPercentage))
        return false;
    if(this->consecutiveIfSameDay!=c.consecutiveIfSameDay)
        return false;
    return true;
}

bool ConstraintMinDaysBetweenActivities::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintMinDaysBetweenActivities::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintMinDaysBetweenActivities::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintMinDaysBetweenActivities::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintMinDaysBetweenActivities>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Consecutive_If_Same_Day>";s+=utils::strings::trueFalse(this->consecutiveIfSameDay);s+="</Consecutive_If_Same_Day>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<MinDays>"+utils::strings::number(this->minDays)+"</MinDays>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMinDaysBetweenActivities>\n";
    return s;
}

QString ConstraintMinDaysBetweenActivities::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Min days between activities");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);s+=", ";
    }
    s+=tr("mD:%1", "Min days").arg(this->minDays);s+=", ";
    s+=tr("CSD:%1", "Consecutive if same day").arg(utils::strings::yesNoTranslated(this->consecutiveIfSameDay));

    return begin+s+end;
}

QString ConstraintMinDaysBetweenActivities::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Minimum number of days between activities");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activitiesId[i])
                .arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }
    s+=tr("Minimum number of days=%1").arg(this->minDays);s+="\n";
    s+=tr("Consecutive if same day=%1").arg(utils::strings::yesNoTranslated(this->consecutiveIfSameDay));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMinDaysBetweenActivities::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the overlapping hours for all pairs of activities.
    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int hour1=c.getHourForActivityIndex(this->_activities[i]);
            int day1=c.getDayForActivityIndex(this->_activities[i]);
            int duration1=r.activeActivitiesList[this->_activities[i]]->duration;

            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int hour2=c.getHourForActivityIndex(this->_activities[j]);
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    int duration2=r.activeActivitiesList[this->_activities[j]]->duration;

                    int tmp;
                    int tt=0;
                    int dist=abs(day1-day2);
                    if(dist<minDays){
                        tt=minDays-dist;

                        if(this->consecutiveIfSameDay && day1==day2){
                            //                                assert(hour1+duration1==hour2 || hour2+duration2==hour1);
                            if (!(hour1+duration1==hour2 || hour2+duration2==hour1)){
                                tt++;
                            }
                        }
                    }

                    tmp=tt;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tt>0 && collectConflictsData){
                        QString s=tr("Time constraint min days between activities broken: activity with id=%1 (%2) conflicts with activity with id=%3 (%4), being %5 days too close, on days %6 and %7",
                                     "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr. Close here means near")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]))
                                .arg(tt)
                                .arg(r.daysOfTheWeek[day1])
                                .arg(r.daysOfTheWeek[day2]);
                        ;

                        s+=", ";
                        s+=tr("conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));
                        s+=".";

                        if(this->consecutiveIfSameDay && day1==day2){
                            s+=" ";
                            s+=tr("The activities are placed consecutively in the timetable, because you selected this option"
                                  " in case the activities are in the same day");
                        }

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintMinDaysBetweenActivities::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintMinDaysBetweenActivities::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintMinDaysBetweenActivities::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinDaysBetweenActivities::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinDaysBetweenActivities::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinDaysBetweenActivities::hasWrongDayOrHour()
{
    if(minDays>=r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintMinDaysBetweenActivities::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMinDaysBetweenActivities::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minDays>=r.nDaysPerWeek)
        minDays=r.nDaysPerWeek-1;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintMaxDaysBetweenActivities::ConstraintMaxDaysBetweenActivities(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_MAX_DAYS_BETWEEN_ACTIVITIES;
}

ConstraintMaxDaysBetweenActivities::ConstraintMaxDaysBetweenActivities(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& act, int n)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    assert(n>=0);
    this->maxDays=n;

    this->type=CONSTRAINT_MAX_DAYS_BETWEEN_ACTIVITIES;
}

bool ConstraintMaxDaysBetweenActivities::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*int j;
                Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintMaxDaysBetweenActivities::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintMaxDaysBetweenActivities::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintMaxDaysBetweenActivities::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintMaxDaysBetweenActivities>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<MaxDays>"+utils::strings::number(this->maxDays)+"</MaxDays>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMaxDaysBetweenActivities>\n";
    return s;
}

QString ConstraintMaxDaysBetweenActivities::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Max days between activities");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);s+=", ";
    }
    s+=tr("MD:%1", "Abbreviation for maximum days").arg(this->maxDays);

    return begin+s+end;
}

QString ConstraintMaxDaysBetweenActivities::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Maximum number of days between activities");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activitiesId[i])
                .arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }
    s+=tr("Maximum number of days=%1").arg(this->maxDays);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMaxDaysBetweenActivities::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the overlapping hours for all pairs of activities.
    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int day1=c.getDayForActivityIndex(this->_activities[i]);
            //int hour1=t1/r.nDaysPerWeek;
            //int duration1=r.internalActivitiesList[this->_activities[i]].duration;

            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    //int hour2=t2/r.nDaysPerWeek;
                    //int duration2=r.internalActivitiesList[this->_activities[j]].duration;

                    int tmp;
                    int tt=0;
                    int dist=abs(day1-day2);
                    if(dist>maxDays){
                        tt=dist-maxDays;

                        //if(this->consecutiveIfSameDay && day1==day2)
                        //	assert( day1==day2 && (hour1+duration1==hour2 || hour2+duration2==hour1) );
                    }

                    tmp=tt;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tt>0 && collectConflictsData){
                        QString s=tr("Time constraint max days between activities broken: activity with id=%1 (%2) conflicts with activity with id=%3 (%4), being %5 days too far away"
                                     ", on days %6 and %7", "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]))
                                .arg(tt)
                                .arg(r.daysOfTheWeek[day1])
                                .arg(r.daysOfTheWeek[day2]);

                        s+=", ";
                        s+=tr("conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));
                        s+=".";

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintMaxDaysBetweenActivities::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintMaxDaysBetweenActivities::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintMaxDaysBetweenActivities::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxDaysBetweenActivities::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxDaysBetweenActivities::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxDaysBetweenActivities::hasWrongDayOrHour()
{
    if(maxDays>=r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintMaxDaysBetweenActivities::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMaxDaysBetweenActivities::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxDays>=r.nDaysPerWeek)
        maxDays=r.nDaysPerWeek-1;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintMinGapsBetweenActivities::ConstraintMinGapsBetweenActivities(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_MIN_GAPS_BETWEEN_ACTIVITIES;
}

ConstraintMinGapsBetweenActivities::ConstraintMinGapsBetweenActivities(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& actList, int ngaps)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->n_activities=nact;
    assert(nact==actList.count());
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(actList.at(i));

    assert(ngaps>0);
    this->minGaps=ngaps;

    this->type=CONSTRAINT_MIN_GAPS_BETWEEN_ACTIVITIES;
}

bool ConstraintMinGapsBetweenActivities::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*int j;
                Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintMinGapsBetweenActivities::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintMinGapsBetweenActivities::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintMinGapsBetweenActivities::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintMinGapsBetweenActivities>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<MinGaps>"+utils::strings::number(this->minGaps)+"</MinGaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMinGapsBetweenActivities>\n";
    return s;
}

QString ConstraintMinGapsBetweenActivities::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Min gaps between activities");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);s+=", ";
    }
    s+=tr("mG:%1", "Minimum number of gaps").arg(this->minGaps);

    return begin+s+end;
}

QString ConstraintMinGapsBetweenActivities::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Minimum gaps between activities (if activities on the same day)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activitiesId[i])
                .arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }
    s+=tr("Minimum number of gaps=%1").arg(this->minGaps);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMinGapsBetweenActivities::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int hour1=c.getHourForActivityIndex(this->_activities[i]);
            int day1=c.getDayForActivityIndex(this->_activities[i]);
            int duration1=r.activeActivitiesList[this->_activities[i]]->duration;

            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int hour2=c.getHourForActivityIndex(this->_activities[j]);
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    int duration2=r.activeActivitiesList[this->_activities[j]]->duration;

                    int tmp;
                    int tt=0;
                    int dist=abs(day1-day2);

                    if(dist==0){ //same day
                        assert(day1==day2);
                        if(hour2>=hour1){
                            //assert(hour1+duration1<=hour2); not true for activities which are not incompatible
                            if(hour1+duration1+minGaps > hour2)
                                tt = (hour1+duration1+minGaps) - hour2;
                        }
                        else{
                            //assert(hour2+duration2<=hour1); not true for activities which are not incompatible
                            if(hour2+duration2+minGaps > hour1)
                                tt = (hour2+duration2+minGaps) - hour1;
                        }
                    }

                    tmp=tt;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tt>0 && collectConflictsData){
                        QString s=tr("Time constraint min gaps between activities broken: activity with id=%1 (%2) conflicts with activity with id=%3 (%4), they are on the same day %5 and there are %6 extra hours between them",
                                     "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]))
                                .arg(r.daysOfTheWeek[day1])
                                .arg(tt);

                        s+=", ";
                        s+=tr("conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));
                        s+=".";

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintMinGapsBetweenActivities::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintMinGapsBetweenActivities::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintMinGapsBetweenActivities::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinGapsBetweenActivities::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinGapsBetweenActivities::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinGapsBetweenActivities::hasWrongDayOrHour()
{
    if(minGaps>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintMinGapsBetweenActivities::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMinGapsBetweenActivities::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minGaps>r.nHoursPerDay)
        minGaps=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxHoursDaily::ConstraintTeachersMaxHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_DAILY;
}

ConstraintTeachersMaxHoursDaily::ConstraintTeachersMaxHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxhours>0);
    this->maxHoursDaily=maxhours;

    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_DAILY;
}

bool ConstraintTeachersMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxHoursDaily>\n";
    return s;
}

QString ConstraintTeachersMaxHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max hours daily");
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MH:%1", "Maximum hours (daily)").arg(this->maxHoursDaily);

    return begin+s+end;
}

QString ConstraintTeachersMaxHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    for(int i=0; i<r.teachersList.size(); i++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int n_hours_daily=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(c.teachersMatrix[i][d][h]>0)
                    n_hours_daily++;

            if(n_hours_daily>this->maxHoursDaily){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr(
                                   "Time constraint teachers max %1 hours daily broken for teacher %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->maxHoursDaily))
                               .arg(r.teachersList.at(i)->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg(n_hours_daily)
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintTeachersMaxHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxHoursDaily::ConstraintTeacherMaxHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MAX_HOURS_DAILY;
}

ConstraintTeacherMaxHoursDaily::ConstraintTeacherMaxHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxhours>0);
    this->maxHoursDaily=maxhours;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MAX_HOURS_DAILY;
}

bool ConstraintTeacherMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxHoursDaily>\n";
    return s;
}

QString ConstraintTeacherMaxHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MH:%1", "Maximum hours (daily)").arg(this->maxHoursDaily);

    return begin+s+end;
}

QString ConstraintTeacherMaxHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    int i=this->teacher_ID;
    for(int d=0; d<r.nDaysPerWeek; d++){
        int n_hours_daily=0;
        for(int h=0; h<r.nHoursPerDay; h++)
            if(c.teachersMatrix[i][d][h]>0)
                n_hours_daily++;

        if(n_hours_daily>this->maxHoursDaily){
            int parcialViolation = 1;

            nbroken++;

            if(collectConflictsData){
                QString s=(tr(
                               "Time constraint teacher max %1 hours daily broken for teacher %2, on day %3, length=%4.")
                           .arg(utils::strings::number(this->maxHoursDaily))
                           .arg(r.teachersList.at(i)->name)
                           .arg(r.daysOfTheWeek[d])
                           .arg(n_hours_daily)
                           )
                        +" "
                        +
                        (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeacherMaxHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxHoursContinuously::ConstraintTeachersMaxHoursContinuously(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_CONTINUOUSLY;
}

ConstraintTeachersMaxHoursContinuously::ConstraintTeachersMaxHoursContinuously(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxhours>0);
    this->maxHoursContinuously=maxhours;

    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintTeachersMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxHoursContinuously::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxHoursContinuously>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxHoursContinuously>\n";
    return s;
}

QString ConstraintTeachersMaxHoursContinuously::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max hours continuously");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MH:%1", "Maximum hours (continuously)").arg(this->maxHoursContinuously);

    return begin+s+end;
}

QString ConstraintTeachersMaxHoursContinuously::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of hours continuously");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    //TODO: contabilizar as janelas como tempo continuo (talvez criar outra constraint para isso)

    nbroken=0;
    for(int i=0; i<r.teachersList.size(); i++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.teachersMatrix[i][d][h]>0)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        int parcialViolation = 1;

                        nbroken++;

                        if(collectConflictsData){
                            QString s=(tr(
                                           "Time constraint teachers max %1 hours continuously broken for teacher %2, on day %3, length=%4.")
                                       .arg(utils::strings::number(this->maxHoursContinuously))
                                       .arg(r.teachersList.at(i)->name)
                                       .arg(r.daysOfTheWeek[d])
                                       .arg(nc)
                                       )
                                    +
                                    " "
                                    +
                                    (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr(
                                   "Time constraint teachers max %1 hours continuously broken for teacher %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->maxHoursContinuously))
                               .arg(r.teachersList.at(i)->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg(nc)
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintTeachersMaxHoursContinuously::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursContinuously::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxHoursContinuously::ConstraintTeacherMaxHoursContinuously(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY;
}

ConstraintTeacherMaxHoursContinuously::ConstraintTeacherMaxHoursContinuously(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxhours>0);
    this->maxHoursContinuously=maxhours;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintTeacherMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxHoursContinuously::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxHoursContinuously>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxHoursContinuously>\n";
    return s;
}

QString ConstraintTeacherMaxHoursContinuously::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max hours continuously");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MH:%1", "Maximum hours continuously").arg(this->maxHoursContinuously);

    return begin+s+end;
}

QString ConstraintTeacherMaxHoursContinuously::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of hours continuously");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    int i=this->teacher_ID;
    for(int d=0; d<r.nDaysPerWeek; d++){
        int nc=0;
        for(int h=0; h<r.nHoursPerDay; h++){
            if(c.teachersMatrix[i][d][h]>0)
                nc++;
            else{
                if(nc>this->maxHoursContinuously){
                    int parcialViolation = 1;

                    nbroken++;

                    if(collectConflictsData){
                        QString s=(tr(
                                       "Time constraint teacher max %1 hours continuously broken for teacher %2, on day %3, length=%4.")
                                   .arg(utils::strings::number(this->maxHoursContinuously))
                                   .arg(r.teachersList.at(i)->name)
                                   .arg(r.daysOfTheWeek[d])
                                   .arg(nc)
                                   )
                                +
                                " "
                                +
                                (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }

                nc=0;
            }
        }

        if(nc>this->maxHoursContinuously){
            int parcialViolation = 1;

            nbroken++;

            if(collectConflictsData){
                QString s=(tr(
                               "Time constraint teacher max %1 hours continuously broken for teacher %2, on day %3, length=%4.")
                           .arg(utils::strings::number(this->maxHoursContinuously))
                           .arg(r.teachersList.at(i)->name)
                           .arg(r.daysOfTheWeek[d])
                           .arg(nc)
                           )
                        +
                        " "
                        +
                        (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeacherMaxHoursContinuously::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursContinuously::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxDaysPerWeek::ConstraintTeacherMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK;
}

ConstraintTeacherMaxDaysPerWeek::ConstraintTeacherMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString tn)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->teacherName = tn;
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_TEACHER_MAX_DAYS_PER_WEEK;
}

bool ConstraintTeacherMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintTeacherMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Teacher max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MD:%1", "Max days (per week)").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeacherMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    //count sort
    int t=this->teacher_ID;
    int nd[MAX_HOURS_PER_DAY + 1];
    for(int h=0; h<=r.nHoursPerDay; h++)
        nd[h]=0;
    for(int d=0; d<r.nDaysPerWeek; d++){
        int nh=0;
        for(int h=0; h<r.nHoursPerDay; h++)
            nh += c.teachersMatrix[t][d][h]>=1 ? 1 : 0;
        nd[nh]++;
    }
    //return the minimum occupied days which do not respect this constraint
    int i = r.nDaysPerWeek - this->maxDaysPerWeek;
    for(int k=0; k<=r.nHoursPerDay; k++){
        if(nd[k]>0){
            if(i>nd[k]){
                i-=nd[k];
                nbroken+=nd[k]*k;
            }
            else{
                nbroken+=i*k;
                break;
            }
        }
    }

    if(nbroken>0 && collectConflictsData){
        int parcialViolation = nbroken;

        QString s= tr("Time constraint teacher max days per week broken for teacher: %1.")
                .arg(r.teachersList.at(t)->name);
        s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintTeacherMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeacherMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxDaysPerWeek>r.nDaysPerWeek)
        maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxDaysPerWeek::ConstraintTeachersMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MAX_DAYS_PER_WEEK;
}

ConstraintTeachersMaxDaysPerWeek::ConstraintTeachersMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_TEACHERS_MAX_DAYS_PER_WEEK;
}

bool ConstraintTeachersMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintTeachersMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Teachers max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MD:%1", "Max days (per week)").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeachersMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;

    for(int t=0; t<r.teachersList.size(); t++){
        int nbr=0;

        //count sort
        int nd[MAX_HOURS_PER_DAY + 1];
        for(int h=0; h<=r.nHoursPerDay; h++)
            nd[h]=0;
        for(int d=0; d<r.nDaysPerWeek; d++){
            int nh=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                nh += c.teachersMatrix[t][d][h]>=1 ? 1 : 0;
            nd[nh]++;
        }
        //return the minimum occupied days which do not respect this constraint
        int i = r.nDaysPerWeek - this->maxDaysPerWeek;
        for(int k=0; k<=r.nHoursPerDay; k++){
            if(nd[k]>0){
                if(i>nd[k]){
                    i-=nd[k];
                    nbroken+=nd[k]*k;
                    nbr+=nd[k]*k;
                }
                else{
                    nbroken+=i*k;
                    nbr+=i*k;
                    break;
                }
            }
        }

        if(nbr>0 && collectConflictsData){
            int parcialViolation = nbr;

            QString s= tr("Time constraint teachers max days per week broken for teacher: %1.")
                    .arg(r.teachersList.at(t)->name);
            s += tr("This increases the total cost by %1")
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintTeachersMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeachersMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxDaysPerWeek>r.nDaysPerWeek)
        maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxGapsPerWeek::ConstraintTeachersMaxGapsPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TEACHERS_MAX_GAPS_PER_WEEK;
}

ConstraintTeachersMaxGapsPerWeek::ConstraintTeachersMaxGapsPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_TEACHERS_MAX_GAPS_PER_WEEK;
    this->maxGaps=mg;
}

bool ConstraintTeachersMaxGapsPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxGapsPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxGapsPerWeek::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxGapsPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxGapsPerWeek>\n";
    return s;
}

QString ConstraintTeachersMaxGapsPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max gaps per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MG:%1", "Max gaps (per week)").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintTeachersMaxGapsPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of gaps per week");s+="\n";
    s+=tr("(breaks and teacher not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per week=%1").arg(this->maxGaps); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxGapsPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{ 
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tg;
    int i, j, k;
    int totalGaps;

    totalGaps=0;
    for(i=0; i<r.teachersList.size(); i++){
        tg=0;
        for(j=0; j<r.nDaysPerWeek; j++){
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.teachersMatrix[i][j][k]>0){
                    //TODO: tirei o assert
//                    assert(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]);
                    break;
                }

            int cnt=0;
            for(; k<r.nHoursPerDay; k++)
                if(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]){
                    if(c.teachersMatrix[i][j][k]>0){
                        tg+=cnt;
                        cnt=0;
                    }
                    else
                        cnt++;
            }
        }
        if(tg>this->maxGaps){
            int parcialViolation = tg-maxGaps;

            totalGaps+=parcialViolation;
            //assert(this->weightPercentage<100); partial solutions might break this rule
            if(collectConflictsData){
                QString s=tr("Time constraint teachers max gaps per week broken for teacher: %1, conflicts cost increase=%2")
                        .arg(r.teachersList.at(i)->name)
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return totalGaps;
}

bool ConstraintTeachersMaxGapsPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxGapsPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxGapsPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerWeek::hasWrongDayOrHour()
{
    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxGapsPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxGapsPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        maxGaps=r.nDaysPerWeek*r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxGapsPerWeek::ConstraintTeacherMaxGapsPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK;
}

ConstraintTeacherMaxGapsPerWeek::ConstraintTeacherMaxGapsPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QString tn, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_TEACHER_MAX_GAPS_PER_WEEK;
    this->teacherName=tn;
    this->maxGaps=mg;
}

bool ConstraintTeacherMaxGapsPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacherIndex=r.searchTeacher(this->teacherName);
    teacherIndex=r.getTeacherIndex(teacherName);
    assert(this->teacherIndex>=0);
    return true;
}

bool ConstraintTeacherMaxGapsPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxGapsPerWeek::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxGapsPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxGapsPerWeek>\n";
    return s;
}

QString ConstraintTeacherMaxGapsPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max gaps per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName); s+=", ";
    s+=tr("MG:%1", "Max gaps (per week").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintTeacherMaxGapsPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint"); s+="\n";
    s+=tr("A teacher must respect the maximum number of gaps per week"); s+="\n";
    s+=tr("(breaks and teacher not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage)); s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName); s+="\n";
    s+=tr("Maximum gaps per week=%1").arg(this->maxGaps); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxGapsPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{ 
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tg;
    int i, j, k;
    int totalGaps;

    totalGaps=0;

    i=this->teacherIndex;

    tg=0;
    for(j=0; j<r.nDaysPerWeek; j++){
        for(k=0; k<r.nHoursPerDay; k++)
            if(c.teachersMatrix[i][j][k]>0){
                assert(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]);
                break;
            }

        int cnt=0;
        for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]){
            if(c.teachersMatrix[i][j][k]>0){
                tg+=cnt;
                cnt=0;
            }
            else
                cnt++;
        }
    }
    if(tg>this->maxGaps){
        int parcialViolation = tg-maxGaps;

        totalGaps+=parcialViolation;
        //assert(this->weightPercentage<100); partial solutions might break this rule
        if(collectConflictsData){
            QString s=tr("Time constraint teacher max gaps per week broken for teacher: %1, conflicts cost increase=%2")
                    .arg(r.teachersList.at(i)->name)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return totalGaps;
}

bool ConstraintTeacherMaxGapsPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::hasWrongDayOrHour()
{
    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxGapsPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxGapsPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        maxGaps=r.nDaysPerWeek*r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxGapsPerDay::ConstraintTeachersMaxGapsPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TEACHERS_MAX_GAPS_PER_DAY;
}

ConstraintTeachersMaxGapsPerDay::ConstraintTeachersMaxGapsPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_TEACHERS_MAX_GAPS_PER_DAY;
    this->maxGaps=mg;
}

bool ConstraintTeachersMaxGapsPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxGapsPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxGapsPerDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxGapsPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxGapsPerDay>\n";
    return s;
}

QString ConstraintTeachersMaxGapsPerDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max gaps per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MG:%1", "Max gaps (per day)").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintTeachersMaxGapsPerDay::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum gaps per day");s+="\n";
    s+=tr("(breaks and teacher not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per day=%1").arg(this->maxGaps); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxGapsPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{ 
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tg;
    int i, j, k;
    int totalGaps;

    totalGaps=0;
    for(i=0; i<r.teachersList.size(); i++){
        for(j=0; j<r.nDaysPerWeek; j++){
            tg=0;
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.teachersMatrix[i][j][k]>0){
                    assert(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]);
                    break;
                }

            int cnt=0;
            for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]){
                if(c.teachersMatrix[i][j][k]>0){
                    tg+=cnt;
                    cnt=0;
                }
                else
                    cnt++;
            }
            if(tg>this->maxGaps){
                int parcialViolation = tg-maxGaps;

                totalGaps+=parcialViolation;
                //assert(this->weightPercentage<100); partial solutions might break this rule
                if(collectConflictsData){
                    QString s=tr("Time constraint teachers max gaps per day broken for teacher: %1, day: %2, conflicts cost increase=%3")
                            .arg(r.teachersList.at(i)->name)
                            .arg(r.daysOfTheWeek[j])
                            .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return totalGaps;
}

bool ConstraintTeachersMaxGapsPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxGapsPerDay::isRelatedToTeacher(Teacher* t)
{	
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxGapsPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxGapsPerDay::hasWrongDayOrHour()
{
    if(maxGaps>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxGapsPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxGapsPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nHoursPerDay)
        maxGaps=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxGapsPerDay::ConstraintTeacherMaxGapsPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY;
}

ConstraintTeacherMaxGapsPerDay::ConstraintTeacherMaxGapsPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QString tn, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_TEACHER_MAX_GAPS_PER_DAY;
    this->teacherName=tn;
    this->maxGaps=mg;
}

bool ConstraintTeacherMaxGapsPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacherIndex=r.searchTeacher(this->teacherName);
    teacherIndex=r.getTeacherIndex(teacherName);
    assert(this->teacherIndex>=0);
    return true;
}

bool ConstraintTeacherMaxGapsPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxGapsPerDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxGapsPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxGapsPerDay>\n";
    return s;
}

QString ConstraintTeacherMaxGapsPerDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max gaps per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName); s+=", ";
    s+=tr("MG:%1", "Max gaps (per day)").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintTeacherMaxGapsPerDay::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint"); s+="\n";
    s+=tr("A teacher must respect the maximum number of gaps per day"); s+="\n";
    s+=tr("(breaks and teacher not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage)); s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName); s+="\n";
    s+=tr("Maximum gaps per day=%1").arg(this->maxGaps); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxGapsPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{ 
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tg;
    int i, j, k;
    int totalGaps;

    totalGaps=0;

    i=this->teacherIndex;

    for(j=0; j<r.nDaysPerWeek; j++){
        tg=0;
        for(k=0; k<r.nHoursPerDay; k++)
            if(c.teachersMatrix[i][j][k]>0){
                assert(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]);
                break;
            }

        int cnt=0;
        for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->teacherNotAvailableDayHour[i][j][k]){
            if(c.teachersMatrix[i][j][k]>0){
                tg+=cnt;
                cnt=0;
            }
            else
                cnt++;
        }
        if(tg>this->maxGaps){
            int parcialViolation = tg-maxGaps;

            totalGaps+=parcialViolation;
            //assert(this->weightPercentage<100); partial solutions might break this rule
            if(collectConflictsData){
                QString s=tr("Time constraint teacher max gaps per day broken for teacher: %1, day: %2, conflicts cost increase=%3")
                        .arg(r.teachersList.at(i)->name)
                        .arg(r.daysOfTheWeek[j])
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return totalGaps;
}

bool ConstraintTeacherMaxGapsPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxGapsPerDay::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxGapsPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxGapsPerDay::hasWrongDayOrHour()
{
    if(maxGaps>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxGapsPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxGapsPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nHoursPerDay)
        maxGaps=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintBreakTimes::ConstraintBreakTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_BREAK_TIMES;
}

ConstraintBreakTimes::ConstraintBreakTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QList<int> d, QList<int> h)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->days = d;
    this->hours = h;
    this->type = CONSTRAINT_BREAK_TIMES;
}

bool ConstraintBreakTimes::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintBreakTimes::getXmlDescription(){
    QString s="<ConstraintBreakTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";

    s+="	<Number_of_Break_Times>"+utils::strings::number(this->days.count())+"</Number_of_Break_Times>\n";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        s+="	<Break_Time>\n";
        if(this->days.at(i)>=0)
            s+="		<Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days.at(i)])+"</Day>\n";
        if(this->hours.at(i)>=0)
            s+="		<Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours.at(i)])+"</Hour>\n";
        s+="	</Break_Time>\n";
    }

    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintBreakTimes>\n";
    return s;
}

QString ConstraintBreakTimes::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Break times");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";

    s+=tr("B at:", "Break at");
    s+=" ";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }

    return begin+s+end;
}

QString ConstraintBreakTimes::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Break times");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("Break at:"); s+="\n";
    assert(days.count()==hours.count());
    for(int i=0; i<days.count(); i++){
        if(this->days.at(i)>=0){
            s+=r.daysOfTheWeek[this->days.at(i)];
            s+=" ";
        }
        if(this->hours.at(i)>=0){
            s+=r.hoursOfTheDay[this->hours.at(i)];
        }
        if(i<days.count()-1)
            s+="; ";
    }
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintBreakTimes::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(r);

    assert(days.count()==hours.count());
    for(int k=0; k<days.count(); k++){
        if(this->days.at(k) >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint break times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours.at(k) >= r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint break times is wrong because an hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }

    return true;
}

double ConstraintBreakTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //DEPRECATED COMMENT
    //For the moment, this function sums the number of hours each teacher
    //is teaching in this break period.
    //This function consideres all the hours, I mean if there are for example 5 weekly courses
    //scheduled on that hour (which is already a broken hard restriction - we only
    //are allowed 1 weekly course for a certain teacher at a certain hour) we calculate
    //5 broken restrictions for this break period.
    //TODO: decide if it is better to consider only 2 or 10 as a return value in this particular case
    //(currently it is 10)

    int nbroken;

    nbroken=0;

    for(int i=0; i<r.activeActivitiesList.size(); i++){
        int houract=c.getHourForActivityIndex(i);
        int dayact=c.getDayForActivityIndex(i);

        assert(days.count()==hours.count());
        for(int kk=0; kk<days.count(); kk++){
            int d=days.at(kk);
            int h=hours.at(kk);

            int dur=r.activeActivitiesList[i]->duration;
            if(d==dayact && !(houract+dur<=h || houract>h))
            {
                int parcialViolation = 1/*teacher*/ + r.activeActivitiesList[i]->studentSetsNames.size()/*studentsGroups*/;

                nbroken+=parcialViolation;

                if(collectConflictsData){
                    QString s=tr("Time constraint break not respected for activity with id %1, on day %2, hours %3")
                            .arg(r.activeActivitiesList[i]->id)
                            .arg(r.daysOfTheWeek[dayact])
                            .arg(r.daysOfTheWeek[houract]);
                    s+=". ";
                    s+=tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintBreakTimes::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintBreakTimes::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintBreakTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintBreakTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintBreakTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintBreakTimes::hasWrongDayOrHour()
{
    assert(days.count()==hours.count());

    for(int i=0; i<days.count(); i++)
        if(days.at(i)<0 || days.at(i)>=r.nDaysPerWeek
                || hours.at(i)<0 || hours.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintBreakTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintBreakTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(days.count()==hours.count());

    QList<int> newDays;
    QList<int> newHours;

    for(int i=0; i<days.count(); i++)
        if(days.at(i)>=0 && days.at(i)<r.nDaysPerWeek
                && hours.at(i)>=0 && hours.at(i)<r.nHoursPerDay){
            newDays.append(days.at(i));
            newHours.append(hours.at(i));
        }

    days=newDays;
    hours=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxGapsPerWeek::ConstraintStudentsMaxGapsPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MAX_GAPS_PER_WEEK;
}

ConstraintStudentsMaxGapsPerWeek::ConstraintStudentsMaxGapsPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_MAX_GAPS_PER_WEEK;
    this->maxGaps=mg;
}

bool ConstraintStudentsMaxGapsPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMaxGapsPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxGapsPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxGapsPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxGapsPerWeek>\n";
    return s;
}

QString ConstraintStudentsMaxGapsPerWeek::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students max gaps per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MG:%1", "Max gaps (per week)").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintStudentsMaxGapsPerWeek::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the maximum number of gaps per week");s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per week=%1").arg(this->maxGaps);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMaxGapsPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    //returns a number equal to the number of gaps of the subgroups (in hours)
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nGaps;
    int tmp;
    int i;

    int tIllegalGaps=0;

    for(i=0; i<r.directSubgroupsList.size(); i++){
        nGaps=0;
        for(int j=0; j<r.nDaysPerWeek; j++){
            int k;
            tmp=0;
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.subgroupsMatrix[i][j][k]>0){
                    assert(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]);
                    break;
                }
            for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                if(c.subgroupsMatrix[i][j][k]>0){
                    nGaps+=tmp;
                    tmp=0;
                }
                else
                    tmp++;
            }
        }

        int parcialViolation = nGaps-this->maxGaps;
        if(parcialViolation<0)
            parcialViolation=0;

        int illegalGaps = parcialViolation;

        if(illegalGaps>0 && collectConflictsData){
            QString s=tr("Time constraint students max gaps per week broken for subgroup: %1, it has %2 extra gaps, cost increase=%3")
                    .arg(r.directSubgroupsList[i]->name)
                    .arg(illegalGaps)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }

        tIllegalGaps+=illegalGaps;
    }

    return tIllegalGaps;
}

bool ConstraintStudentsMaxGapsPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxGapsPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMaxGapsPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxGapsPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxGapsPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMaxGapsPerWeek::hasWrongDayOrHour()
{
    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMaxGapsPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxGapsPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        maxGaps=r.nDaysPerWeek*r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxGapsPerWeek::ConstraintStudentsSetMaxGapsPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK;
}

ConstraintStudentsSetMaxGapsPerWeek::ConstraintStudentsSetMaxGapsPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg, const QString& st )
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_WEEK;
    this->maxGaps=mg;
    this->students = st;
}

bool ConstraintStudentsSetMaxGapsPerWeek::computeInternalStructure(QWidget* parent){
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max gaps per week is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

bool ConstraintStudentsSetMaxGapsPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxGapsPerWeek::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxGapsPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Students>"; s+=utils::strings::parseStrForXml(this->students); s+="</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxGapsPerWeek>\n";
    return s;
}

QString ConstraintStudentsSetMaxGapsPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set max gaps per week"); s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage)); s+=", ";
    s+=tr("MG:%1", "Max gaps (per week)").arg(this->maxGaps);s+=", ";
    s+=tr("St:%1", "Students").arg(this->students);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxGapsPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the maximum number of gaps per week");s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per week=%1").arg(this->maxGaps);s+="\n";
    s+=tr("Students=%1").arg(this->students); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetMaxGapsPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    //OLD COMMENT
    //returns a number equal to the number of gaps of the subgroups (in hours)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nGaps;
    int tmp;

    int tIllegalGaps=0;

    for(int sg=0; sg<this->iSubgroupsList.count(); sg++){
        nGaps=0;
        int i=this->iSubgroupsList.at(sg);
        for(int j=0; j<r.nDaysPerWeek; j++){
            int k;
            tmp=0;
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.subgroupsMatrix[i][j][k]>0){
                    assert(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]);
                    break;
                }
            for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                if(c.subgroupsMatrix[i][j][k]>0){
                    nGaps+=tmp;
                    tmp=0;
                }
                else
                    tmp++;
            }
        }

        int parcialViolation=nGaps-this->maxGaps;
        if(parcialViolation<0)
            parcialViolation=0;

        int illegalGaps=parcialViolation;

        if(illegalGaps>0 && collectConflictsData){
            QString s=tr("Time constraint students set max gaps per week broken for subgroup: %1, extra gaps=%2, cost increase=%3")
                    .arg(r.directSubgroupsList[i]->name)
                    .arg(illegalGaps)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }

        tIllegalGaps+=illegalGaps;
    }

    return tIllegalGaps;
}

bool ConstraintStudentsSetMaxGapsPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxGapsPerWeek::hasWrongDayOrHour()
{
    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxGapsPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxGapsPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nDaysPerWeek*r.nHoursPerDay)
        maxGaps=r.nDaysPerWeek*r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsEarlyMaxBeginningsAtSecondHour::ConstraintStudentsEarlyMaxBeginningsAtSecondHour(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR;
}

ConstraintStudentsEarlyMaxBeginningsAtSecondHour::ConstraintStudentsEarlyMaxBeginningsAtSecondHour(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mBSH)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR;
    this->maxBeginningsAtSecondHour=mBSH;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsEarlyMaxBeginningsAtSecondHour::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsEarlyMaxBeginningsAtSecondHour>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Beginnings_At_Second_Hour>"+utils::strings::number(this->maxBeginningsAtSecondHour)+"</Max_Beginnings_At_Second_Hour>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsEarlyMaxBeginningsAtSecondHour>\n";
    return s;
}

QString ConstraintStudentsEarlyMaxBeginningsAtSecondHour::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students must arrive early, respecting maximum %1 arrivals at second hour")
            .arg(this->maxBeginningsAtSecondHour);
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintStudentsEarlyMaxBeginningsAtSecondHour::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must begin their activities early, respecting maximum %1 later arrivals, at second hour")
            .arg(this->maxBeginningsAtSecondHour);s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsEarlyMaxBeginningsAtSecondHour::violationsFactor(Solution& c, bool collectConflictsData)
{
    //considers the condition that the hours of subgroups begin as early as possible

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int conflTotal=0;

    for(int i=0; i<r.directSubgroupsList.size(); i++){
        int nGapsFirstHour=0;
        for(int j=0; j<r.nDaysPerWeek; j++){
            int k;
            for(k=0; k<r.nHoursPerDay; k++)
                if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k])
                    break;

            bool firstHourOccupied=false;
            if(k<r.nHoursPerDay && c.subgroupsMatrix[i][j][k]>0)
                firstHourOccupied=true;

            bool dayOccupied=firstHourOccupied;

            bool illegalGap=false;

            for(k++; k<r.nHoursPerDay && !dayOccupied; k++)
                if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                    if(c.subgroupsMatrix[i][j][k]>0)
                        dayOccupied=true;
                    else
                        illegalGap=true;
                }

            if(dayOccupied && illegalGap){
                int parcialViolation = 1;

                if(collectConflictsData){
                    QString s=tr("Constraint students early max %1 beginnings at second hour broken for subgroup %2, on day %3,"
                                 " because students have an illegal gap, increases total cost by %4")
                            .arg(this->maxBeginningsAtSecondHour)
                            .arg(r.directSubgroupsList[i]->name)
                            .arg(r.daysOfTheWeek[j])
                            .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);

                    conflTotal+=parcialViolation; //TODO: ta certo isso dentro desse if?
                }

//                if(c.nPlacedActivities==r.activeActivitiesList.size()){
//                    assert(0);
//                }
            }

            if(dayOccupied && !firstHourOccupied)
                nGapsFirstHour++;
        }

        if(nGapsFirstHour>this->maxBeginningsAtSecondHour){
            int parcialViolation = (nGapsFirstHour-this->maxBeginningsAtSecondHour);

            if(collectConflictsData){
                QString s=tr("Constraint students early max %1 beginnings at second hour broken for subgroup %2,"
                             " because students have too many arrivals at second hour, increases total cost by %3")
                        .arg(this->maxBeginningsAtSecondHour)
                        .arg(r.directSubgroupsList[i]->name)
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);

                conflTotal+=parcialViolation; //TODO: ta certo isso dentro desse if?
            }

//            if(c.nPlacedActivities==r.activeActivitiesList.size()){
//                assert(0);
//            }
        }
    }

    return conflTotal;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::hasWrongDayOrHour()
{
    if(maxBeginningsAtSecondHour>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsEarlyMaxBeginningsAtSecondHour::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxBeginningsAtSecondHour>r.nDaysPerWeek)
        maxBeginningsAtSecondHour=r.nDaysPerWeek;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR;
}

ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mBSH, const QString& students)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_SET_EARLY_MAX_BEGINNINGS_AT_SECOND_HOUR;
    this->students=students;
    this->maxBeginningsAtSecondHour=mBSH;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set early is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);
    return true;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Beginnings_At_Second_Hour>"+utils::strings::number(this->maxBeginningsAtSecondHour)+"</Max_Beginnings_At_Second_Hour>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour>\n";
    return s;
}

QString ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    s+=tr("Students set must arrive early, respecting maximum %1 arrivals at second hour")
            .arg(this->maxBeginningsAtSecondHour); s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students set").arg(this->students);

    return begin+s+end;
}

QString ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";

    s+=tr("A students set must begin its activities early, respecting a maximum number of later arrivals, at second hour"); s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students); s+="\n";
    s+=tr("Maximum number of arrivals at the second hour=%1").arg(this->maxBeginningsAtSecondHour);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::violationsFactor(Solution& c, bool collectConflictsData)
{
    //considers the condition that the hours of subgroups begin as early as possible

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int conflTotal=0;

    foreach(int i, this->iSubgroupsList){
        int nGapsFirstHour=0;
        for(int j=0; j<r.nDaysPerWeek; j++){
            int k;
            for(k=0; k<r.nHoursPerDay; k++)
                if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k])
                    break;

            bool firstHourOccupied=false;
            if(k<r.nHoursPerDay && c.subgroupsMatrix[i][j][k]>0)
                firstHourOccupied=true;

            bool dayOccupied=firstHourOccupied;

            bool illegalGap=false;

            for(k++; k<r.nHoursPerDay && !dayOccupied; k++)
                if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                    if(c.subgroupsMatrix[i][j][k]>0)
                        dayOccupied=true;
                    else
                        illegalGap=true;
                }

            if(dayOccupied && illegalGap){
                int parcialViolation = 1;

                if(collectConflictsData){
                    QString s=tr("Constraint students set early max %1 beginnings at second hour broken for subgroup %2, on day %3,"
                                 " because students have an illegal gap, increases total cost by %4")
                            .arg(this->maxBeginningsAtSecondHour)
                            .arg(r.directSubgroupsList[i]->name)
                            .arg(r.daysOfTheWeek[j])
                            .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);

                    conflTotal+=parcialViolation; //TODO: ta certo isso dentro desse if?
                }

//                if(c.nPlacedActivities==r.activeActivitiesList.size())
//                    assert(0);
            }

            if(dayOccupied && !firstHourOccupied)
                nGapsFirstHour++;
        }

        if(nGapsFirstHour>this->maxBeginningsAtSecondHour){
            int parcialViolation = (nGapsFirstHour-this->maxBeginningsAtSecondHour);

            if(collectConflictsData){
                QString s=tr("Constraint students set early max %1 beginnings at second hour broken for subgroup %2,"
                             " because students have too many arrivals at second hour, increases total cost by %3")
                        .arg(this->maxBeginningsAtSecondHour)
                        .arg(r.directSubgroupsList[i]->name)
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);

                conflTotal+=parcialViolation; //TODO: ta certo isso dentro desse if?
            }

//            if(c.nPlacedActivities==r.activeActivitiesList.size())
//                assert(0);
        }
    }

    return conflTotal;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::hasWrongDayOrHour()
{
    if(maxBeginningsAtSecondHour>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetEarlyMaxBeginningsAtSecondHour::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxBeginningsAtSecondHour>r.nDaysPerWeek)
        maxBeginningsAtSecondHour=r.nDaysPerWeek;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxHoursDaily::ConstraintStudentsMaxHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MAX_HOURS_DAILY;
    this->maxHoursDaily = -1;
}

ConstraintStudentsMaxHoursDaily::ConstraintStudentsMaxHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursDaily = maxnh;
    this->type = CONSTRAINT_STUDENTS_MAX_HOURS_DAILY;
}

bool ConstraintStudentsMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    if(this->maxHoursDaily>=0)
        s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    else
        assert(0);
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxHoursDaily>\n";
    return s;
}

QString ConstraintStudentsMaxHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students max hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MH:%1", "Max hours (daily)").arg(this->maxHoursDaily);

    return begin+s+end;
}

QString ConstraintStudentsMaxHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the maximum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tmp;
    int too_much;

    assert(this->maxHoursDaily>=0);

    if(1){
        too_much=0;
        for(int i=0; i<r.directSubgroupsList.size(); i++)
            for(int j=0; j<r.nDaysPerWeek; j++){
                tmp=0;
                for(int k=0; k<r.nHoursPerDay; k++){
                    //OLD COMMENT
                    //Here we want to see if we have a weekly activity or a 2 weeks activity
                    //We don't do tmp+=c.subgroupsMatrix[i][j][k] because we already counted this as a hard hitness
                    if(c.subgroupsMatrix[i][j][k]>=1)
                        tmp++;
                }
                if(this->maxHoursDaily>=0 && tmp > this->maxHoursDaily){ //we would like no more than maxHoursDaily hours per day.
                    int parcialViolation = 1;

                    too_much += parcialViolation; //tmp - this->maxHoursDaily;

                    if(collectConflictsData){
                        QString s=tr("Time constraint students max hours daily broken for subgroup: %1, day: %2, lenght=%3, conflict increase=%4")
                                .arg(r.directSubgroupsList[i]->name)
                                .arg(r.daysOfTheWeek[j])
                                .arg(utils::strings::number(tmp))
                                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
    }

    assert(too_much>=0);

    return too_much;
}

bool ConstraintStudentsMaxHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxHoursDaily::ConstraintStudentsSetMaxHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY;
    this->maxHoursDaily = -1;
}

ConstraintStudentsSetMaxHoursDaily::ConstraintStudentsSetMaxHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, QString s)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursDaily = maxnh;
    this->students = s;
    this->type = CONSTRAINT_STUDENTS_SET_MAX_HOURS_DAILY;
}

bool ConstraintStudentsSetMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxHoursDaily>\n";
    return s;
}

QString ConstraintStudentsSetMaxHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set max hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students (set)").arg(this->students); s+=", ";
    s+=tr("MH:%1", "Max hours (daily)").arg(this->maxHoursDaily);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the maximum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max hours daily is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

double ConstraintStudentsSetMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tmp;
    int too_much;

    assert(this->maxHoursDaily>=0);

    if(1){
        too_much=0;
        for(int sg=0; sg<this->iSubgroupsList.count(); sg++){
            int i=iSubgroupsList.at(sg);
            for(int j=0; j<r.nDaysPerWeek; j++){
                tmp=0;
                for(int k=0; k<r.nHoursPerDay; k++){
                    //Here we want to see if we have a weekly activity or a 2 weeks activity
                    //We don't do tmp+=c.subgroupsMatrix[i][j][k] because we already counted this as a hard hitness
                    if(c.subgroupsMatrix[i][j][k]>=1)
                        tmp++;
                }
                if(this->maxHoursDaily>=0 && tmp > this->maxHoursDaily){ //we would like no more than max_hours_daily hours per day.
                    int parcialViolation = 1;

                    too_much += parcialViolation; //tmp - this->maxHoursDaily;

                    if(collectConflictsData){
                        QString s=tr("Time constraint students set max hours daily broken for subgroup: %1, day: %2, lenght=%3, cost increase=%4")
                                .arg(r.directSubgroupsList[i]->name)
                                .arg(r.daysOfTheWeek[j])
                                .arg(utils::strings::number(tmp))
                                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    assert(too_much>=0);

    return too_much;
}

bool ConstraintStudentsSetMaxHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxHoursContinuously::ConstraintStudentsMaxHoursContinuously(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MAX_HOURS_CONTINUOUSLY;
    this->maxHoursContinuously = -1;
}

ConstraintStudentsMaxHoursContinuously::ConstraintStudentsMaxHoursContinuously(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursContinuously = maxnh;
    this->type = CONSTRAINT_STUDENTS_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintStudentsMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxHoursContinuously::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxHoursContinuously>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    if(this->maxHoursContinuously>=0)
        s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    else
        assert(0);
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxHoursContinuously>\n";
    return s;
}

QString ConstraintStudentsMaxHoursContinuously::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students max hours continuously");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MH:%1", "Max hours (continuously)").arg(this->maxHoursContinuously);

    return begin+s+end;
}

QString ConstraintStudentsMaxHoursContinuously::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the maximum number of hours continuously");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    for(int i=0; i<r.directSubgroupsList.size(); i++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[i][d][h]>0)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        int parcialViolation = 1;

                        nbroken++;

                        if(collectConflictsData){
                            QString s=(tr(
                                           "Time constraint students max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                                       .arg(utils::strings::number(this->maxHoursContinuously))
                                       .arg(r.directSubgroupsList[i]->name)
                                       .arg(r.daysOfTheWeek[d])
                                       .arg(nc)
                                       )
                                    +
                                    " "
                                    +
                                    (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr(
                                   "Time constraint students max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->maxHoursContinuously))
                               .arg(r.directSubgroupsList[i]->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg(nc)
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsMaxHoursContinuously::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxHoursContinuously::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxHoursContinuously::ConstraintStudentsSetMaxHoursContinuously(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY;
    this->maxHoursContinuously = -1;
}

ConstraintStudentsSetMaxHoursContinuously::ConstraintStudentsSetMaxHoursContinuously(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, QString s)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursContinuously = maxnh;
    this->students = s;
    this->type = CONSTRAINT_STUDENTS_SET_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintStudentsSetMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxHoursContinuously::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxHoursContinuously>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxHoursContinuously>\n";
    return s;
}

QString ConstraintStudentsSetMaxHoursContinuously::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set max hours continuously");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students (set)").arg(this->students);s+=", ";
    s+=tr("MH:%1", "Max hours (continuously)").arg(this->maxHoursContinuously);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxHoursContinuously::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the maximum number of hours continuously");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max hours continuously is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

double ConstraintStudentsSetMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    foreach(int i, this->iSubgroupsList){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[i][d][h]>0)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        int parcialViolation = 1;

                        nbroken++;

                        if(collectConflictsData){
                            QString s=(tr(
                                           "Time constraint students set max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                                       .arg(utils::strings::number(this->maxHoursContinuously))
                                       .arg(r.directSubgroupsList[i]->name)
                                       .arg(r.daysOfTheWeek[d])
                                       .arg(nc)
                                       )
                                    +
                                    " "
                                    +
                                    (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr(
                                   "Time constraint students set max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->maxHoursContinuously))
                               .arg(r.directSubgroupsList[i]->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg(nc)
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetMaxHoursContinuously::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxHoursContinuously::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMinHoursDaily::ConstraintStudentsMinHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MIN_HOURS_DAILY;
    this->minHoursDaily = -1;

    this->allowEmptyDays=false;
}

ConstraintStudentsMinHoursDaily::ConstraintStudentsMinHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minnh, bool _allowEmptyDays)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->minHoursDaily = minnh;
    this->type = CONSTRAINT_STUDENTS_MIN_HOURS_DAILY;

    this->allowEmptyDays=_allowEmptyDays;
}

bool ConstraintStudentsMinHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMinHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMinHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMinHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    if(this->minHoursDaily>=0)
        s+="	<Minimum_Hours_Daily>"+utils::strings::number(this->minHoursDaily)+"</Minimum_Hours_Daily>\n";
    else
        assert(0);
    if(this->allowEmptyDays)
        s+="	<Allow_Empty_Days>true</Allow_Empty_Days>\n";
    else
        s+="	<Allow_Empty_Days>false</Allow_Empty_Days>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMinHoursDaily>\n";
    return s;
}

QString ConstraintStudentsMinHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    if(this->allowEmptyDays)
        s+="! ";
    s+=tr("Students min hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("mH:%1", "Min hours (daily)").arg(this->minHoursDaily);s+=", ";
    s+=tr("AED:%1", "Allow empty days").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));

    return begin+s+end;
}

QString ConstraintStudentsMinHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    if(this->allowEmptyDays==true){
        s+=tr("(nonstandard, students may have empty days)");
        s+="\n";
    }
    s+=tr("All students must respect the minimum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Minimum hours daily=%1").arg(this->minHoursDaily);s+="\n";
    s+=tr("Allow empty days=%1").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMinHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tmp;
    int too_little;

    assert(this->minHoursDaily>=0);

    too_little=0;
    for(int i=0; i<r.directSubgroupsList.size(); i++)
        for(int j=0; j<r.nDaysPerWeek; j++){
            tmp=0;
            for(int k=0; k<r.nHoursPerDay; k++){
                if(c.subgroupsMatrix[i][j][k]>=1)
                    tmp++;
            }

            bool searchDay;
            if(this->allowEmptyDays==true)
                searchDay=(tmp>0);
            else
                searchDay=true;

            if(/*tmp>0*/ searchDay && this->minHoursDaily>=0 && tmp < this->minHoursDaily){ //we would like no less than minHoursDaily hours per day.
                int parcialViolation = - tmp + this->minHoursDaily;

                too_little += parcialViolation;

                if(collectConflictsData){
                    QString s=tr("Time constraint students min hours daily broken for subgroup: %1, day: %2, lenght=%3, conflict increase=%4")
                            .arg(r.directSubgroupsList[i]->name)
                            .arg(r.daysOfTheWeek[j])
                            .arg(utils::strings::number(tmp))
                            .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }

    //should not consider for empty days

    assert(too_little>=0);

    return too_little;
}

bool ConstraintStudentsMinHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMinHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMinHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMinHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMinHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMinHoursDaily::hasWrongDayOrHour()
{
    if(minHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMinHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMinHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minHoursDaily>r.nHoursPerDay)
        minHoursDaily=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMinHoursDaily::ConstraintStudentsSetMinHoursDaily(Instance &r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY;
    this->minHoursDaily = -1;

    this->allowEmptyDays=false;
}

ConstraintStudentsSetMinHoursDaily::ConstraintStudentsSetMinHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minnh, QString s, bool _allowEmptyDays)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->minHoursDaily = minnh;
    this->students = s;
    this->type = CONSTRAINT_STUDENTS_SET_MIN_HOURS_DAILY;

    this->allowEmptyDays=_allowEmptyDays;
}

bool ConstraintStudentsSetMinHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMinHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMinHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Hours_Daily>"+utils::strings::number(this->minHoursDaily)+"</Minimum_Hours_Daily>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    if(this->allowEmptyDays)
        s+="	<Allow_Empty_Days>true</Allow_Empty_Days>\n";
    else
        s+="	<Allow_Empty_Days>false</Allow_Empty_Days>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMinHoursDaily>\n";
    return s;
}

QString ConstraintStudentsSetMinHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    if(this->allowEmptyDays)
        s+="! ";
    s+=tr("Students set min hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students (set)").arg(this->students);s+=", ";
    s+=tr("mH:%1", "Min hours (daily)").arg(this->minHoursDaily);s+=", ";
    s+=tr("AED:%1", "Allow empty days").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));

    return begin+s+end;
}

QString ConstraintStudentsSetMinHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    if(this->allowEmptyDays==true){
        s+=tr("(nonstandard, students may have empty days)");
        s+="\n";
    }
    s+=tr("A students set must respect the minimum number of hours daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Minimum hours daily=%1").arg(this->minHoursDaily);s+="\n";
    s+=tr("Allow empty days=%1").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetMinHoursDaily::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set min hours daily is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

double ConstraintStudentsSetMinHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int tmp;
    int too_little;

    assert(this->minHoursDaily>=0);

    too_little=0;
    for(int sg=0; sg<this->iSubgroupsList.count(); sg++){
        int i=iSubgroupsList.at(sg);
        for(int j=0; j<r.nDaysPerWeek; j++){
            tmp=0;
            for(int k=0; k<r.nHoursPerDay; k++){
                if(c.subgroupsMatrix[i][j][k]>=1)
                    tmp++;
            }

            bool searchDay;
            if(this->allowEmptyDays==true)
                searchDay=(tmp>0);
            else
                searchDay=true;

            if(/*tmp>0*/ searchDay && this->minHoursDaily>=0 && tmp < this->minHoursDaily){
                int parcialViolation = - tmp + this->minHoursDaily;

                too_little += parcialViolation;

                if(collectConflictsData){
                    QString s=tr("Time constraint students set min hours daily broken for subgroup: %1, day: %2, lenght=%3, cost increase=%4")
                            .arg(r.directSubgroupsList[i]->name)
                            .arg(r.daysOfTheWeek[j])
                            .arg(utils::strings::number(tmp))
                            .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    assert(too_little>=0);

    return too_little;
}

bool ConstraintStudentsSetMinHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMinHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMinHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMinHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMinHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMinHoursDaily::hasWrongDayOrHour()
{
    if(minHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMinHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMinHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minHoursDaily>r.nHoursPerDay)
        minHoursDaily=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivityPreferredStartingTime::ConstraintActivityPreferredStartingTime(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME;
}

ConstraintActivityPreferredStartingTime::ConstraintActivityPreferredStartingTime(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int d, int h, bool perm)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->activityId = actId;
    this->day = d;
    this->hour = h;
    this->type = CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME;
    this->permanentlyLocked=perm;
}

bool ConstraintActivityPreferredStartingTime::operator==(ConstraintActivityPreferredStartingTime& c){
    if(this->day!=c.day)
        return false;
    if(this->hour!=c.hour)
        return false;
    if(this->activityId!=c.activityId)
        return false;
    if(!IS_EQUAL(this->weightPercentage, c.weightPercentage))
        return false;
    if(this->active!=c.active)
        return false;
    //no need to care about permanently locked
    return true;
}

bool ConstraintActivityPreferredStartingTime::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->activityId)
                        break;
        }*/

    int i=r.getActivityIndex(activityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because it refers to invalid activity id). Please correct it (maybe removing it is a solution):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    if(this->day >= r.nDaysPerWeek){
        MessagesManager::information(parent, tr("m-FET information"),
                                                         tr("Constraint activity preferred starting time is wrong because it refers to removed day. Please correct"
                                                            " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->hour == r.nHoursPerDay){
        MessagesManager::information(parent, tr("m-FET information"),
                                                         tr("Constraint activity preferred starting time is wrong because preferred hour is too late (after the last acceptable slot). Please correct"
                                                            " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->hour > r.nHoursPerDay){
        MessagesManager::information(parent, tr("m-FET information"),
                                                         tr("Constraint activity preferred starting time is wrong because it refers to removed hour. Please correct"
                                                            " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    this->activityIndex=i;
    return true;
}

bool ConstraintActivityPreferredStartingTime::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->activityId))
        return true;
    return false;
}

QString ConstraintActivityPreferredStartingTime::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintActivityPreferredStartingTime>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Id>"+utils::strings::number(this->activityId)+"</Activity_Id>\n";
    if(this->day>=0)
        s+="	<Preferred_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->day])+"</Preferred_Day>\n";
    if(this->hour>=0)
        s+="	<Preferred_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hour])+"</Preferred_Hour>\n";
    s+="	<Permanently_Locked>";s+=utils::strings::trueFalse(this->permanentlyLocked);s+="</Permanently_Locked>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivityPreferredStartingTime>\n";
    return s;
}

QString ConstraintActivityPreferredStartingTime::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Act. id: %1 (%2) has a preferred starting time: %3", "%1 is the id, %2 is the detailed description of the activity. %3 is time (day and hour)")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId))
            .arg(r.daysOfTheWeek[this->day]+" "+r.hoursOfTheDay[this->hour]);

    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));
    s+=", ";
    s+=tr("PL:%1", "Abbreviation for permanently locked").arg(utils::strings::yesNoTranslated(this->permanentlyLocked));

    return begin+s+end;
}

QString ConstraintActivityPreferredStartingTime::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId));
    s+="\n";

    s+=tr("has a preferred starting time:");
    s+="\n";
    s+=tr("Day=%1").arg(r.daysOfTheWeek[this->day]);
    s+="\n";
    s+=tr("Hour=%1").arg(r.hoursOfTheDay[this->hour]);
    s+="\n";

    s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    if(this->permanentlyLocked){
        s+=tr("This activity is permanently locked, which means you cannot unlock it from the 'Timetable' menu"
              " (you can unlock this activity by removing the constraint from the constraints dialog or by setting the 'permanently"
              " locked' attribute false when editing this constraint)");
    }
    else{
        s+=tr("This activity is not permanently locked, which means you can unlock it from the 'Timetable' menu");
    }
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivityPreferredStartingTime::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->activityIndex]!=UNALLOCATED_TIME){
        int h=c.getHourForActivityIndex(this->activityIndex);
        int d=c.getDayForActivityIndex(this->activityIndex);
        if(this->day>=0)
            nbroken+=abs(this->day-d);
        if(this->hour>=0)
            nbroken+=abs(this->hour-h);
    }

    if(nbroken>0)
        nbroken=1;

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint activity preferred starting time broken for activity with id=%1 (%2), increases total cost by %3",
                     "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activityId)
                .arg(r.getActivityDetailedDescription(this->activityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintActivityPreferredStartingTime::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->activityId==a->id)
        return true;
    return false;
}

bool ConstraintActivityPreferredStartingTime::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivityPreferredStartingTime::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTime::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTime::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTime::hasWrongDayOrHour()
{
    if(day<0 || day>=r.nDaysPerWeek || hour<0 || hour>=r.nHoursPerDay)
        return true;
    return false;
}

bool ConstraintActivityPreferredStartingTime::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return false;
}

bool ConstraintActivityPreferredStartingTime::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivityPreferredTimeSlots::ConstraintActivityPreferredTimeSlots(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS;
}

ConstraintActivityPreferredTimeSlots::ConstraintActivityPreferredTimeSlots(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->p_activityId=actId;
    this->p_nPreferredTimeSlots_L=nPT_L;
    this->p_days_L=d_L;
    this->p_hours_L=h_L;
    this->type=CONSTRAINT_ACTIVITY_PREFERRED_TIME_SLOTS;
}

bool ConstraintActivityPreferredTimeSlots::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->p_activityId)
                        break;
        }*/

    int i=r.getActivityIndex(p_activityId);

    if(i==-1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because it refers to invalid activity id). Please correct it (maybe removing it is a solution):\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    for(int k=0; k<p_nPreferredTimeSlots_L; k++){
        if(this->p_days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred time slots is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred time slots is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred time slots is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }

        if(this->p_hours_L[k]<0 || this->p_days_L[k]<0){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred time slots is wrong because it has hour or day not specified for a slot (-1). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }

    this->p_activityIndex=i;
    return true;
}

bool ConstraintActivityPreferredTimeSlots::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->p_activityId))
        return true;
    return false;
}

QString ConstraintActivityPreferredTimeSlots::getXmlDescription()
{
    QString s="<ConstraintActivityPreferredTimeSlots>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Id>"+utils::strings::number(this->p_activityId)+"</Activity_Id>\n";
    s+="	<Number_of_Preferred_Time_Slots>"+utils::strings::number(this->p_nPreferredTimeSlots_L)+"</Number_of_Preferred_Time_Slots>\n";
    for(int i=0; i<p_nPreferredTimeSlots_L; i++){
        s+="	<Preferred_Time_Slot>\n";
        if(this->p_days_L[i]>=0)
            s+="		<Preferred_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->p_days_L[i]])+"</Preferred_Day>\n";
        if(this->p_hours_L[i]>=0)
            s+="		<Preferred_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->p_hours_L[i]])+"</Preferred_Hour>\n";
        s+="	</Preferred_Time_Slot>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivityPreferredTimeSlots>\n";
    return s;
}

QString ConstraintActivityPreferredTimeSlots::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Act. id: %1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
            .arg(this->p_activityId)
            .arg(r.getActivityDetailedDescription(this->p_activityId));
    s+=" ";

    s+=tr("has a set of preferred time slots:");
    s+=" ";
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivityPreferredTimeSlots::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
            .arg(this->p_activityId)
            .arg(r.getActivityDetailedDescription(this->p_activityId));
    s+="\n";

    s+=tr("has a set of preferred time slots (all hours of the activity must be in the allowed slots):");
    s+="\n";
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivityPreferredTimeSlots::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    Matrix2D<bool> allowed;
    allowed.resize(r.nDaysPerWeek, r.nHoursPerDay);
    //bool allowed[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    for(int d=0; d<r.nDaysPerWeek; d++)
        for(int h=0; h<r.nHoursPerDay; h++)
            allowed[d][h]=false;
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0 && this->p_hours_L[i]>=0)
            allowed[this->p_days_L[i]][this->p_hours_L[i]]=true;
        else
            assert(0);
    }

    nbroken=0;
    if(c.times[(unsigned)this->p_activityIndex]!=UNALLOCATED_TIME){
        int h=c.getHourForActivityIndex(this->p_activityIndex);
        int d=c.getDayForActivityIndex(this->p_activityIndex);
        for(int dur=0; dur<r.activeActivitiesList[this->p_activityIndex]->duration; dur++)
            if(!allowed[d][h+dur])
                nbroken++;
    }

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint activity preferred time slots broken for activity with id=%1 (%2) on %3 hours, increases total cost by %4",
                     "%1 is the id, %2 is the detailed description of the activity.")
                .arg(this->p_activityId)
                .arg(r.getActivityDetailedDescription(this->p_activityId))
                .arg(nbroken)
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintActivityPreferredTimeSlots::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->p_activityId==a->id)
        return true;
    return false;
}

bool ConstraintActivityPreferredTimeSlots::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivityPreferredTimeSlots::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredTimeSlots::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredTimeSlots::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredTimeSlots::hasWrongDayOrHour()
{
    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)<0 || p_days_L.at(i)>=r.nDaysPerWeek
                || p_hours_L.at(i)<0 || p_hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintActivityPreferredTimeSlots::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivityPreferredTimeSlots::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)>=0 && p_days_L.at(i)<r.nDaysPerWeek
                && p_hours_L.at(i)>=0 && p_hours_L.at(i)<r.nHoursPerDay){
            newDays.append(p_days_L.at(i));
            newHours.append(p_hours_L.at(i));
            newNPref++;
        }

    p_nPreferredTimeSlots_L=newNPref;
    p_days_L=newDays;
    p_hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesPreferredTimeSlots::ConstraintActivitiesPreferredTimeSlots(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS;
}

ConstraintActivitiesPreferredTimeSlots::ConstraintActivitiesPreferredTimeSlots(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QString te,
                                                                               QString st, QString su, QString sut, int dur, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(dur==-1 || dur>=1);
    duration=dur;

    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->p_teacherName=te;
    this->p_subjectName=su;
    this->p_activityTagName=sut;
    this->p_studentsName=st;
    this->p_nPreferredTimeSlots_L=nPT_L;
    this->p_days_L=d_L;
    this->p_hours_L=h_L;
    this->type=CONSTRAINT_ACTIVITIES_PREFERRED_TIME_SLOTS;
}

bool ConstraintActivitiesPreferredTimeSlots::computeInternalStructure(QWidget* parent)
{
    this->p_nActivities=0;
    this->p_activitiesIndices.clear();

    Activity* act;
    int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        act=r.activeActivitiesList[i];

        //check if this activity has the corresponding students
        if(this->p_studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, p_studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->p_subjectName!="" && act->subjectName!=this->p_subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->p_activityTagName!="" && !act->activityTagsNames.contains(this->p_activityTagName)){
            continue;
        }

        if(duration>=1 && act->duration!=duration)
            continue;

        assert(this->p_nActivities < MAX_ACTIVITIES);
        this->p_nActivities++;
        this->p_activitiesIndices.append(i);
    }

    assert(this->p_nActivities==this->p_activitiesIndices.count());

    //////////////////////
    for(int k=0; k<p_nPreferredTimeSlots_L; k++){
        if(this->p_days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred time slots is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred time slots is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred time slots is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k]<0 || this->p_days_L[k]<0){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred time slots is wrong because hour or day is not specified for a slot (-1). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->p_nActivities>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintActivitiesPreferredTimeSlots::hasInactiveActivities()
{
    QList<int> localActiveActs;
    QList<int> localAllActs;

    //returns true if all activities are inactive
    Activity* act;
    int i;
    for(i=0; i<r.activitiesList.count(); i++){
        act=r.activitiesList.at(i);

        //check if this activity has the corresponding students
        if(this->p_studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, p_studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->p_subjectName!="" && act->subjectName!=this->p_subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->p_activityTagName!="" && !act->activityTagsNames.contains(this->p_activityTagName)){
            continue;
        }

        if(duration>=1 && act->duration!=duration)
            continue;

        if(!r.inactiveActivitiesIdsSet.contains(act->id))
            localActiveActs.append(act->id);

        localAllActs.append(act->id);
    }

    if(localActiveActs.count()==0 && localAllActs.count()>0)
        //because if this constraint does not refer to any activity,
        //it should be reported as incorrect
        return true;
    else
        return false;
}

QString ConstraintActivitiesPreferredTimeSlots::getXmlDescription()
{
    QString s="<ConstraintActivitiesPreferredTimeSlots>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->p_teacherName)+"</Teacher_Name>\n";
    s+="	<Students_Name>"+utils::strings::parseStrForXml(this->p_studentsName)+"</Students_Name>\n";
    s+="	<Subject_Name>"+utils::strings::parseStrForXml(this->p_subjectName)+"</Subject_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->p_activityTagName)+"</Activity_Tag_Name>\n";
    if(duration>=1)
        s+="	<Duration>"+utils::strings::number(duration)+"</Duration>\n";
    else
        s+="	<Duration></Duration>\n";
    s+="	<Number_of_Preferred_Time_Slots>"+utils::strings::number(this->p_nPreferredTimeSlots_L)+"</Number_of_Preferred_Time_Slots>\n";
    for(int i=0; i<p_nPreferredTimeSlots_L; i++){
        s+="	<Preferred_Time_Slot>\n";
        if(this->p_days_L[i]>=0)
            s+="		<Preferred_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->p_days_L[i]])+"</Preferred_Day>\n";
        if(this->p_hours_L[i]>=0)
            s+="		<Preferred_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->p_hours_L[i]])+"</Preferred_Hour>\n";
        s+="	</Preferred_Time_Slot>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesPreferredTimeSlots>\n";
    return s;
}

QString ConstraintActivitiesPreferredTimeSlots::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    QString tc, st, su, at, dur;

    if(this->p_teacherName!="")
        tc=tr("teacher=%1").arg(this->p_teacherName);
    else
        tc=tr("all teachers");

    if(this->p_studentsName!="")
        st=tr("students=%1").arg(this->p_studentsName);
    else
        st=tr("all students");

    if(this->p_subjectName!="")
        su=tr("subject=%1").arg(this->p_subjectName);
    else
        su=tr("all subjects");

    if(this->p_activityTagName!="")
        at=tr("activity tag=%1").arg(this->p_activityTagName);
    else
        at=tr("all activity tags");

    if(duration>=1)
        dur=tr("duration=%1").arg(duration);
    else
        dur=tr("all durations");

    s+=tr("Activities with %1, %2, %3, %4, %5, have a set of preferred time slots:", "%1...%5 are conditions for the activities").arg(tc).arg(st).arg(su).arg(at).arg(dur);
    s+=" ";
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivitiesPreferredTimeSlots::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activities with:");s+="\n";

    if(this->p_teacherName!="")
        s+=tr("Teacher=%1").arg(this->p_teacherName);
    else
        s+=tr("All teachers");
    s+="\n";
    if(this->p_studentsName!="")
        s+=tr("Students=%1").arg(this->p_studentsName);
    else
        s+=tr("All students");
    s+="\n";
    if(this->p_subjectName!="")
        s+=tr("Subject=%1").arg(this->p_subjectName);
    else
        s+=tr("All subjects");
    s+="\n";
    if(this->p_activityTagName!="")
        s+=tr("Activity tag=%1").arg(this->p_activityTagName);
    else
        s+=tr("All activity tags");
    s+="\n";

    if(duration>=1)
        s+=tr("Duration=%1").arg(duration);
    else
        s+=tr("All durations");
    s+="\n";

    s+=tr("have a set of preferred time slots (all hours of each affected activity must be in the allowed slots):");
    s+="\n";
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesPreferredTimeSlots::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    ///////////////////
    Matrix2D<bool> allowed;
    allowed.resize(r.nDaysPerWeek, r.nHoursPerDay);
    //bool allowed[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    for(int d=0; d<r.nDaysPerWeek; d++)
        for(int h=0; h<r.nHoursPerDay; h++)
            allowed[d][h]=false;
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0 && this->p_hours_L[i]>=0)
            allowed[this->p_days_L[i]][this->p_hours_L[i]]=true;
        else
            assert(0);
    }
    ////////////////////

    nbroken=0;
    int tmp;

    for(int i=0; i<this->p_nActivities; i++){
        tmp=0;
        int ai=this->p_activitiesIndices[i];
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);

            for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++)
                if(!allowed[d][h+dur])
                    tmp++;
        }

        int parcialViolation = tmp;

        nbroken+=parcialViolation;

        if(collectConflictsData && tmp>0){
            QString s=tr("Time constraint activities preferred time slots broken"
                         " for activity with id=%1 (%2) on %3 hours,"
                         " increases total cost by %4", "%1 is the id, %2 is the detailed description of the activity.")
                    .arg(r.activeActivitiesList[ai]->id)
                    .arg(r.getActivityDetailedDescription(r.activeActivitiesList[ai]->id))
                    .arg(tmp)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintActivitiesPreferredTimeSlots::isRelatedToActivity( Activity* a)
{
    //check if this activity has the corresponding students
    if(this->p_studentsName!=""){
        bool commonStudents=false;
        foreach(QString st, a->studentSetsNames){
            if(r.setsShareStudents(st, this->p_studentsName)){
                commonStudents=true;
                break;
            }
        }
        if(!commonStudents)
            return false;
    }
    //check if this activity has the corresponding subject
    if(this->p_subjectName!="" && a->subjectName!=this->p_subjectName)
        return false;
    //check if this activity has the corresponding activity tag
    if(this->p_activityTagName!="" && !a->activityTagsNames.contains(this->p_activityTagName))
        return false;

    if(duration>=1 && a->duration!=duration)
        return false;

    return true;
}

bool ConstraintActivitiesPreferredTimeSlots::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesPreferredTimeSlots::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredTimeSlots::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredTimeSlots::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredTimeSlots::hasWrongDayOrHour()
{
    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)<0 || p_days_L.at(i)>=r.nDaysPerWeek
                || p_hours_L.at(i)<0 || p_hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintActivitiesPreferredTimeSlots::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivitiesPreferredTimeSlots::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)>=0 && p_days_L.at(i)<r.nDaysPerWeek
                && p_hours_L.at(i)>=0 && p_hours_L.at(i)<r.nHoursPerDay){
            newDays.append(p_days_L.at(i));
            newHours.append(p_hours_L.at(i));
            newNPref++;
        }

    p_nPreferredTimeSlots_L=newNPref;
    p_days_L=newDays;
    p_hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintSubactivitiesPreferredTimeSlots::ConstraintSubactivitiesPreferredTimeSlots(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS;
}

ConstraintSubactivitiesPreferredTimeSlots::ConstraintSubactivitiesPreferredTimeSlots(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int compNo, QString te,
                                                                                     QString st, QString su, QString sut, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->componentNumber=compNo;
    this->p_teacherName=te;
    this->p_subjectName=su;
    this->p_activityTagName=sut;
    this->p_studentsName=st;
    this->p_nPreferredTimeSlots_L=nPT_L;
    this->p_days_L=d_L;
    this->p_hours_L=h_L;
    this->type=CONSTRAINT_SUBACTIVITIES_PREFERRED_TIME_SLOTS;
}

bool ConstraintSubactivitiesPreferredTimeSlots::computeInternalStructure(QWidget* parent)
{
    this->p_nActivities=0;
    this->p_activitiesIndices.clear();

    Activity* act;
    int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        act=r.activeActivitiesList[i];

        if(!act->representsComponentNumber(this->componentNumber))
            continue;

        //check if this activity has the corresponding students
        if(this->p_studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, p_studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->p_subjectName!="" && act->subjectName!=this->p_subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->p_activityTagName!="" && !act->activityTagsNames.contains(this->p_activityTagName)){
            continue;
        }

        assert(this->p_nActivities < MAX_ACTIVITIES);
        this->p_nActivities++;
        this->p_activitiesIndices.append(i);
    }

    assert(this->p_nActivities==this->p_activitiesIndices.count());

    //////////////////////
    for(int k=0; k<p_nPreferredTimeSlots_L; k++){
        if(this->p_days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred time slots is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred time slots is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred time slots is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->p_hours_L[k]<0 || this->p_days_L[k]<0){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred time slots is wrong because hour or day is not specified for a slot (-1). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->p_nActivities>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintSubactivitiesPreferredTimeSlots::hasInactiveActivities()
{
    QList<int> localActiveActs;
    QList<int> localAllActs;

    //returns true if all activities are inactive
    Activity* act;
    int i;
    for(i=0; i<r.activitiesList.count(); i++){
        act=r.activitiesList.at(i);

        if(!act->representsComponentNumber(this->componentNumber))
            continue;

        //check if this activity has the corresponding students
        if(this->p_studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, p_studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->p_subjectName!="" && act->subjectName!=this->p_subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->p_activityTagName!="" && !act->activityTagsNames.contains(this->p_activityTagName)){
            continue;
        }

        if(!r.inactiveActivitiesIdsSet.contains(act->id))
            localActiveActs.append(act->id);

        localAllActs.append(act->id);
    }

    if(localActiveActs.count()==0 && localAllActs.count()>0)
        return true;
    else
        return false;
}

QString ConstraintSubactivitiesPreferredTimeSlots::getXmlDescription()
{
    QString s="<ConstraintSubactivitiesPreferredTimeSlots>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Component_Number>"+utils::strings::number(this->componentNumber)+"</Component_Number>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->p_teacherName)+"</Teacher_Name>\n";
    s+="	<Students_Name>"+utils::strings::parseStrForXml(this->p_studentsName)+"</Students_Name>\n";
    s+="	<Subject_Name>"+utils::strings::parseStrForXml(this->p_subjectName)+"</Subject_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->p_activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Number_of_Preferred_Time_Slots>"+utils::strings::number(this->p_nPreferredTimeSlots_L)+"</Number_of_Preferred_Time_Slots>\n";
    for(int i=0; i<p_nPreferredTimeSlots_L; i++){
        s+="	<Preferred_Time_Slot>\n";
        if(this->p_days_L[i]>=0)
            s+="		<Preferred_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->p_days_L[i]])+"</Preferred_Day>\n";
        if(this->p_hours_L[i]>=0)
            s+="		<Preferred_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->p_hours_L[i]])+"</Preferred_Hour>\n";
        s+="	</Preferred_Time_Slot>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintSubactivitiesPreferredTimeSlots>\n";
    return s;
}

QString ConstraintSubactivitiesPreferredTimeSlots::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    QString tc, st, su, at;

    if(this->p_teacherName!="")
        tc=tr("teacher=%1").arg(this->p_teacherName);
    else
        tc=tr("all teachers");

    if(this->p_studentsName!="")
        st=tr("students=%1").arg(this->p_studentsName);
    else
        st=tr("all students");

    if(this->p_subjectName!="")
        su=tr("subject=%1").arg(this->p_subjectName);
    else
        su=tr("all subjects");

    if(this->p_activityTagName!="")
        at=tr("activity tag=%1").arg(this->p_activityTagName);
    else
        at=tr("all activity tags");

    s+=tr("Subactivities with %1, %2, %3, %4, %5, have a set of preferred time slots:", "%1...%5 are conditions for the subactivities")
            .arg(tr("component number=%1").arg(this->componentNumber)).arg(tc).arg(st).arg(su).arg(at);

    s+=" ";

    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintSubactivitiesPreferredTimeSlots::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Subactivities with:");s+="\n";

    s+=tr("Component number=%1").arg(this->componentNumber);
    s+="\n";

    if(this->p_teacherName!="")
        s+=tr("Teacher=%1").arg(this->p_teacherName);
    else
        s+=tr("All teachers");
    s+="\n";

    if(this->p_studentsName!="")
        s+=tr("Students=%1").arg(this->p_studentsName);
    else
        s+=tr("All students");
    s+="\n";

    if(this->p_subjectName!="")
        s+=tr("Subject=%1").arg(this->p_subjectName);
    else
        s+=tr("All subjects");
    s+="\n";

    if(this->p_activityTagName!="")
        s+=tr("Activity tag=%1").arg(this->p_activityTagName);
    else
        s+=tr("All activity tags");
    s+="\n";

    s+=tr("have a set of preferred time slots (all hours of each affected subactivity must be in the allowed slots):");
    s+="\n";
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0){
            s+=r.daysOfTheWeek[this->p_days_L[i]];
            s+=" ";
        }
        if(this->p_hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->p_hours_L[i]];
        }
        if(i<this->p_nPreferredTimeSlots_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintSubactivitiesPreferredTimeSlots::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    ///////////////////
    Matrix2D<bool> allowed;
    allowed.resize(r.nDaysPerWeek, r.nHoursPerDay);
    //bool allowed[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    for(int d=0; d<r.nDaysPerWeek; d++)
        for(int h=0; h<r.nHoursPerDay; h++)
            allowed[d][h]=false;
    for(int i=0; i<this->p_nPreferredTimeSlots_L; i++){
        if(this->p_days_L[i]>=0 && this->p_hours_L[i]>=0)
            allowed[this->p_days_L[i]][this->p_hours_L[i]]=true;
        else
            assert(0);
    }
    ////////////////////

    nbroken=0;
    int tmp;

    for(int i=0; i<this->p_nActivities; i++){
        tmp=0;
        int ai=this->p_activitiesIndices[i];
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);

            for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++)
                if(!allowed[d][h+dur])
                    tmp++;
        }

        int parcialViolation = tmp;

        nbroken+=parcialViolation;

        if(collectConflictsData && tmp>0){
            QString s=tr("Time constraint subactivities preferred time slots broken"
                         " for activity with id=%1 (%2), component number %3, on %4 hours,"
                         " increases total cost by %5", "%1 is the id, %2 is the detailed description of the activity.")
                    .arg(r.activeActivitiesList[ai]->id)
                    .arg(r.getActivityDetailedDescription(r.activeActivitiesList[ai]->id))
                    .arg(componentNumber)
                    .arg(tmp)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintSubactivitiesPreferredTimeSlots::isRelatedToActivity( Activity* a)
{
    if(!a->representsComponentNumber(this->componentNumber))
        return false;

    //check if this activity has the corresponding students
    if(this->p_studentsName!=""){
        bool commonStudents=false;
        foreach(QString st, a->studentSetsNames){
            if(r.setsShareStudents(st, this->p_studentsName)){
                commonStudents=true;
                break;
            }
        }
        if(!commonStudents)
            return false;
    }
    //check if this activity has the corresponding subject
    if(this->p_subjectName!="" && a->subjectName!=this->p_subjectName)
        return false;
    //check if this activity has the corresponding activity tag
    if(this->p_activityTagName!="" && !a->activityTagsNames.contains(this->p_activityTagName))
        return false;

    return true;
}

bool ConstraintSubactivitiesPreferredTimeSlots::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintSubactivitiesPreferredTimeSlots::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredTimeSlots::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredTimeSlots::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredTimeSlots::hasWrongDayOrHour()
{
    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)<0 || p_days_L.at(i)>=r.nDaysPerWeek
                || p_hours_L.at(i)<0 || p_hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintSubactivitiesPreferredTimeSlots::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintSubactivitiesPreferredTimeSlots::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(p_nPreferredTimeSlots_L==p_days_L.count());
    assert(p_nPreferredTimeSlots_L==p_hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<p_nPreferredTimeSlots_L; i++)
        if(p_days_L.at(i)>=0 && p_days_L.at(i)<r.nDaysPerWeek
                && p_hours_L.at(i)>=0 && p_hours_L.at(i)<r.nHoursPerDay){
            newDays.append(p_days_L.at(i));
            newHours.append(p_hours_L.at(i));
            newNPref++;
        }

    p_nPreferredTimeSlots_L=newNPref;
    p_days_L=newDays;
    p_hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivityPreferredStartingTimes::ConstraintActivityPreferredStartingTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES;
}

ConstraintActivityPreferredStartingTimes::ConstraintActivityPreferredStartingTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int actId, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->activityId=actId;
    this->nPreferredStartingTimes_L=nPT_L;
    this->days_L=d_L;
    this->hours_L=h_L;
    this->type=CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIMES;
}

bool ConstraintActivityPreferredStartingTimes::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->activityId)
                        break;
        }*/

    int i=r.getActivityIndex(activityId);

    if(i==-1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because it refers to invalid activity id). Please correct it (maybe removing it is a solution):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    for(int k=0; k<nPreferredStartingTimes_L; k++){
        if(this->days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred starting times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred starting times is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activity preferred starting times is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }

    this->activityIndex=i;
    return true;
}

bool ConstraintActivityPreferredStartingTimes::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->activityId))
        return true;
    return false;
}

QString ConstraintActivityPreferredStartingTimes::getXmlDescription()
{
    QString s="<ConstraintActivityPreferredStartingTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Id>"+utils::strings::number(this->activityId)+"</Activity_Id>\n";
    s+="	<Number_of_Preferred_Starting_Times>"+utils::strings::number(this->nPreferredStartingTimes_L)+"</Number_of_Preferred_Starting_Times>\n";
    for(int i=0; i<nPreferredStartingTimes_L; i++){
        s+="	<Preferred_Starting_Time>\n";
        if(this->days_L[i]>=0)
            s+="		<Preferred_Starting_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days_L[i]])+"</Preferred_Starting_Day>\n";
        if(this->hours_L[i]>=0)
            s+="		<Preferred_Starting_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours_L[i]])+"</Preferred_Starting_Hour>\n";
        s+="	</Preferred_Starting_Time>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivityPreferredStartingTimes>\n";
    return s;
}

QString ConstraintActivityPreferredStartingTimes::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Act. id: %1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId));

    s+=" ";
    s+=tr("has a set of preferred starting times:");
    s+=" ";
    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<nPreferredStartingTimes_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight Percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivityPreferredStartingTimes::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId));

    s+="\n";
    s+=tr("has a set of preferred starting times:");
    s+="\n";
    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<this->nPreferredStartingTimes_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivityPreferredStartingTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->activityIndex]!=UNALLOCATED_TIME){
        int h=c.getHourForActivityIndex(this->activityIndex);
        int d=c.getDayForActivityIndex(this->activityIndex);
        int i;
        for(i=0; i<this->nPreferredStartingTimes_L; i++){
            if(this->days_L[i]>=0 && this->days_L[i]!=d)
                continue;
            if(this->hours_L[i]>=0 && this->hours_L[i]!=h)
                continue;
            break;
        }
        if(i==this->nPreferredStartingTimes_L){
            nbroken=1;
        }
    }

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint activity preferred starting times broken for activity with id=%1 (%2), increases total cost by %3",
                     "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activityId)
                .arg(r.getActivityDetailedDescription(this->activityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintActivityPreferredStartingTimes::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->activityId==a->id)
        return true;
    return false;
}

bool ConstraintActivityPreferredStartingTimes::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivityPreferredStartingTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityPreferredStartingTimes::hasWrongDayOrHour()
{
    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)<0 || days_L.at(i)>=r.nDaysPerWeek
                || hours_L.at(i)<0 || hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintActivityPreferredStartingTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivityPreferredStartingTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)>=0 && days_L.at(i)<r.nDaysPerWeek
                && hours_L.at(i)>=0 && hours_L.at(i)<r.nHoursPerDay){
            newDays.append(days_L.at(i));
            newHours.append(hours_L.at(i));
            newNPref++;
        }

    nPreferredStartingTimes_L=newNPref;
    days_L=newDays;
    hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesPreferredStartingTimes::ConstraintActivitiesPreferredStartingTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES;
}

ConstraintActivitiesPreferredStartingTimes::ConstraintActivitiesPreferredStartingTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QString te,
                                                                                       QString st, QString su, QString sut, int dur, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(dur==-1 || dur>=1);
    duration=dur;

    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->teacherName=te;
    this->subjectName=su;
    this->activityTagName=sut;
    this->studentsName=st;
    this->nPreferredStartingTimes_L=nPT_L;
    this->days_L=d_L;
    this->hours_L=h_L;
    this->type=CONSTRAINT_ACTIVITIES_PREFERRED_STARTING_TIMES;
}

bool ConstraintActivitiesPreferredStartingTimes::computeInternalStructure(QWidget* parent)
{
    this->nActivities=0;
    this->activitiesIndices.clear();

    Activity* act;
    int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        act=r.activeActivitiesList[i];

        //check if this activity has the corresponding students
        if(this->studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->subjectName!="" && act->subjectName!=this->subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->activityTagName!="" && !act->activityTagsNames.contains(this->activityTagName)){
            continue;
        }

        if(duration>=1 && act->duration!=duration)
            continue;

        assert(this->nActivities < MAX_ACTIVITIES);
        //this->activitiesIndices[this->nActivities++]=i;
        this->activitiesIndices.append(i);
        this->nActivities++;
    }

    assert(this->activitiesIndices.count()==this->nActivities);

    //////////////////////
    for(int k=0; k<nPreferredStartingTimes_L; k++){
        if(this->days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred starting times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred starting times is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities preferred starting times is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->nActivities>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintActivitiesPreferredStartingTimes::hasInactiveActivities()
{
    QList<int> localActiveActs;
    QList<int> localAllActs;

    //returns true if all activities are inactive
    Activity* act;
    int i;
    for(i=0; i<r.activitiesList.count(); i++){
        act=r.activitiesList.at(i);

        //check if this activity has the corresponding students
        if(this->studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->subjectName!="" && act->subjectName!=this->subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->activityTagName!="" && !act->activityTagsNames.contains(this->activityTagName)){
            continue;
        }

        if(duration>=1 && act->duration!=duration)
            continue;

        if(!r.inactiveActivitiesIdsSet.contains(act->id))
            localActiveActs.append(act->id);

        localAllActs.append(act->id);
    }

    if(localActiveActs.count()==0 && localAllActs.count()>0)
        return true;
    else
        return false;
}

QString ConstraintActivitiesPreferredStartingTimes::getXmlDescription()
{
    QString s="<ConstraintActivitiesPreferredStartingTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Students_Name>"+utils::strings::parseStrForXml(this->studentsName)+"</Students_Name>\n";
    s+="	<Subject_Name>"+utils::strings::parseStrForXml(this->subjectName)+"</Subject_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    if(duration>=1)
        s+="	<Duration>"+utils::strings::number(duration)+"</Duration>\n";
    else
        s+="	<Duration></Duration>\n";
    s+="	<Number_of_Preferred_Starting_Times>"+utils::strings::number(this->nPreferredStartingTimes_L)+"</Number_of_Preferred_Starting_Times>\n";
    for(int i=0; i<nPreferredStartingTimes_L; i++){
        s+="	<Preferred_Starting_Time>\n";
        if(this->days_L[i]>=0)
            s+="		<Preferred_Starting_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days_L[i]])+"</Preferred_Starting_Day>\n";
        if(this->hours_L[i]>=0)
            s+="		<Preferred_Starting_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours_L[i]])+"</Preferred_Starting_Hour>\n";
        s+="	</Preferred_Starting_Time>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesPreferredStartingTimes>\n";
    return s;
}

QString ConstraintActivitiesPreferredStartingTimes::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    QString tc, st, su, at, dur;

    if(this->teacherName!="")
        tc=tr("teacher=%1").arg(this->teacherName);
    else
        tc=tr("all teachers");

    if(this->studentsName!="")
        st=tr("students=%1").arg(this->studentsName);
    else
        st=tr("all students");

    if(this->subjectName!="")
        su=tr("subject=%1").arg(this->subjectName);
    else
        su=tr("all subjects");

    if(this->activityTagName!="")
        at=tr("activity tag=%1").arg(this->activityTagName);
    else
        at=tr("all activity tags");

    if(duration>=1)
        dur=tr("duration=%1").arg(duration);
    else
        dur=tr("all durations");

    s+=tr("Activities with %1, %2, %3, %4, %5, have a set of preferred starting times:", "%1...%5 are conditions for the activities").arg(tc).arg(st).arg(su).arg(at).arg(dur);
    s+=" ";

    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<this->nPreferredStartingTimes_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivitiesPreferredStartingTimes::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activities with:");s+="\n";

    if(this->teacherName!="")
        s+=tr("Teacher=%1").arg(this->teacherName);
    else
        s+=tr("All teachers");
    s+="\n";

    if(this->studentsName!="")
        s+=tr("Students=%1").arg(this->studentsName);
    else
        s+=tr("All students");
    s+="\n";

    if(this->subjectName!="")
        s+=tr("Subject=%1").arg(this->subjectName);
    else
        s+=tr("All subjects");
    s+="\n";

    if(this->activityTagName!="")
        s+=tr("Activity tag=%1").arg(this->activityTagName);
    else
        s+=tr("All activity tags");
    s+="\n";

    if(duration>=1)
        s+=tr("Duration=%1").arg(duration);
    else
        s+=tr("All durations");
    s+="\n";

    s+=tr("have a set of preferred starting times:");
    s+="\n";
    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<this->nPreferredStartingTimes_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesPreferredStartingTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    int tmp;

    for(int i=0; i<this->nActivities; i++){
        tmp=0;
        int ai=this->activitiesIndices[i];
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);
            int i;
            for(i=0; i<this->nPreferredStartingTimes_L; i++){
                if(this->days_L[i]>=0 && this->days_L[i]!=d)
                    continue;
                if(this->hours_L[i]>=0 && this->hours_L[i]!=h)
                    continue;
                break;
            }
            if(i==this->nPreferredStartingTimes_L){
                tmp=1;
            }
        }

        int parcialViolation = tmp;

        nbroken+=parcialViolation;

        if(collectConflictsData && tmp>0){
            QString s=tr("Time constraint activities preferred starting times broken"
                         " for activity with id=%1 (%2),"
                         " increases total cost by %3", "%1 is the id, %2 is the detailed description of the activity")
                    .arg(r.activeActivitiesList[ai]->id)
                    .arg(r.getActivityDetailedDescription(r.activeActivitiesList[ai]->id))
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintActivitiesPreferredStartingTimes::isRelatedToActivity( Activity* a)
{
    //check if this activity has the corresponding students
    if(this->studentsName!=""){
        bool commonStudents=false;
        foreach(QString st, a->studentSetsNames){
            if(r.setsShareStudents(st, this->studentsName)){
                commonStudents=true;
                break;
            }
        }
        if(!commonStudents)
            return false;
    }
    //check if this activity has the corresponding subject
    if(this->subjectName!="" && a->subjectName!=this->subjectName)
        return false;
    //check if this activity has the corresponding activity tag
    if(this->activityTagName!="" && !a->activityTagsNames.contains(this->activityTagName))
        return false;

    if(duration>=1 && a->duration!=duration)
        return false;

    return true;
}

bool ConstraintActivitiesPreferredStartingTimes::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesPreferredStartingTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredStartingTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredStartingTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesPreferredStartingTimes::hasWrongDayOrHour()
{
    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)<0 || days_L.at(i)>=r.nDaysPerWeek
                || hours_L.at(i)<0 || hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintActivitiesPreferredStartingTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivitiesPreferredStartingTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)>=0 && days_L.at(i)<r.nDaysPerWeek
                && hours_L.at(i)>=0 && hours_L.at(i)<r.nHoursPerDay){
            newDays.append(days_L.at(i));
            newHours.append(hours_L.at(i));
            newNPref++;
        }

    nPreferredStartingTimes_L=newNPref;
    days_L=newDays;
    hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintSubactivitiesPreferredStartingTimes::ConstraintSubactivitiesPreferredStartingTimes(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES;
}

ConstraintSubactivitiesPreferredStartingTimes::ConstraintSubactivitiesPreferredStartingTimes(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int compNo, QString te,
                                                                                             QString st, QString su, QString sut, int nPT_L, QList<int> d_L, QList<int> h_L)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==nPT_L);
    assert(h_L.count()==nPT_L);

    this->componentNumber=compNo;
    this->teacherName=te;
    this->subjectName=su;
    this->activityTagName=sut;
    this->studentsName=st;
    this->nPreferredStartingTimes_L=nPT_L;
    this->days_L=d_L;
    this->hours_L=h_L;
    this->type=CONSTRAINT_SUBACTIVITIES_PREFERRED_STARTING_TIMES;
}

bool ConstraintSubactivitiesPreferredStartingTimes::computeInternalStructure(QWidget* parent)
{
    this->nActivities=0;
    this->activitiesIndices.clear();

    Activity* act;
    int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        act=r.activeActivitiesList[i];

        if(!act->representsComponentNumber(this->componentNumber))
            continue;

        //check if this activity has the corresponding students
        if(this->studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->subjectName!="" && act->subjectName!=this->subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->activityTagName!="" && !act->activityTagsNames.contains(this->activityTagName)){
            continue;
        }

        assert(this->nActivities < MAX_ACTIVITIES);
        //this->activitiesIndices[this->nActivities++]=i;
        this->nActivities++;
        this->activitiesIndices.append(i);
    }

    assert(this->activitiesIndices.count()==this->nActivities);

    //////////////////////
    for(int k=0; k<nPreferredStartingTimes_L; k++){
        if(this->days_L[k] >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred starting times is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred starting times is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->hours_L[k] > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint subactivities preferred starting times is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->nActivities>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintSubactivitiesPreferredStartingTimes::hasInactiveActivities()
{
    QList<int> localActiveActs;
    QList<int> localAllActs;

    //returns true if all activities are inactive
    Activity* act;
    int i;
    for(i=0; i<r.activitiesList.count(); i++){
        act=r.activitiesList.at(i);

        if(!act->representsComponentNumber(this->componentNumber))
            continue;

        //check if this activity has the corresponding students
        if(this->studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->subjectName!="" && act->subjectName!=this->subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->activityTagName!="" && !act->activityTagsNames.contains(this->activityTagName)){
            continue;
        }

        if(!r.inactiveActivitiesIdsSet.contains(act->id))
            localActiveActs.append(act->id);

        localAllActs.append(act->id);
    }

    if(localActiveActs.count()==0 && localAllActs.count()>0)
        return true;
    else
        return false;
}

QString ConstraintSubactivitiesPreferredStartingTimes::getXmlDescription()
{
    QString s="<ConstraintSubactivitiesPreferredStartingTimes>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Component_Number>"+utils::strings::number(this->componentNumber)+"</Component_Number>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Students_Name>"+utils::strings::parseStrForXml(this->studentsName)+"</Students_Name>\n";
    s+="	<Subject_Name>"+utils::strings::parseStrForXml(this->subjectName)+"</Subject_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Number_of_Preferred_Starting_Times>"+utils::strings::number(this->nPreferredStartingTimes_L)+"</Number_of_Preferred_Starting_Times>\n";
    for(int i=0; i<nPreferredStartingTimes_L; i++){
        s+="	<Preferred_Starting_Time>\n";
        if(this->days_L[i]>=0)
            s+="		<Preferred_Starting_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->days_L[i]])+"</Preferred_Starting_Day>\n";
        if(this->hours_L[i]>=0)
            s+="		<Preferred_Starting_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->hours_L[i]])+"</Preferred_Starting_Hour>\n";
        s+="	</Preferred_Starting_Time>\n";
    }
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintSubactivitiesPreferredStartingTimes>\n";
    return s;
}

QString ConstraintSubactivitiesPreferredStartingTimes::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString tc, st, su, at;

    if(this->teacherName!="")
        tc=tr("teacher=%1").arg(this->teacherName);
    else
        tc=tr("all teachers");

    if(this->studentsName!="")
        st=tr("students=%1").arg(this->studentsName);
    else
        st=tr("all students");

    if(this->subjectName!="")
        su=tr("subject=%1").arg(this->subjectName);
    else
        su=tr("all subjects");

    if(this->activityTagName!="")
        at=tr("activity tag=%1").arg(this->activityTagName);
    else
        at=tr("all activity tags");

    QString s;

    s+=tr("Subactivities with %1, %2, %3, %4, %5, have a set of preferred starting times:", "%1...%5 are conditions for the subactivities")
            .arg(tr("component number=%1").arg(this->componentNumber)).arg(tc).arg(st).arg(su).arg(at);
    s+=" ";

    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<this->nPreferredStartingTimes_L-1)
            s+="; ";
    }
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintSubactivitiesPreferredStartingTimes::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Subactivities with:");s+="\n";

    s+=tr("Component number=%1").arg(this->componentNumber);s+="\n";

    if(this->teacherName!="")
        s+=tr("Teacher=%1").arg(this->teacherName);
    else
        s+=tr("All teachers");
    s+="\n";

    if(this->studentsName!="")
        s+=tr("Students=%1").arg(this->studentsName);
    else
        s+=tr("All students");
    s+="\n";

    if(this->subjectName!="")
        s+=tr("Subject=%1").arg(this->subjectName);
    else
        s+=tr("All subjects");
    s+="\n";

    if(this->activityTagName!="")
        s+=tr("Activity tag=%1").arg(this->activityTagName);
    else
        s+=tr("All activity tags");
    s+="\n";

    s+=tr("have a set of preferred starting times:");
    s+="\n";
    for(int i=0; i<this->nPreferredStartingTimes_L; i++){
        if(this->days_L[i]>=0){
            s+=r.daysOfTheWeek[this->days_L[i]];
            s+=" ";
        }
        if(this->hours_L[i]>=0){
            s+=r.hoursOfTheDay[this->hours_L[i]];
        }
        if(i<this->nPreferredStartingTimes_L-1)
            s+=";  ";
    }
    s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintSubactivitiesPreferredStartingTimes::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    int tmp;

    for(int i=0; i<this->nActivities; i++){
        tmp=0;
        int ai=this->activitiesIndices[i];
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);
            int i;
            for(i=0; i<this->nPreferredStartingTimes_L; i++){
                if(this->days_L[i]>=0 && this->days_L[i]!=d)
                    continue;
                if(this->hours_L[i]>=0 && this->hours_L[i]!=h)
                    continue;
                break;
            }
            if(i==this->nPreferredStartingTimes_L){
                tmp=1;
            }
        }

        int parcialViolation = tmp;

        nbroken+=parcialViolation;

        if(collectConflictsData && tmp>0){
            QString s=tr("Time constraint subactivities preferred starting times broken"
                         " for activity with id=%1 (%2), component number %3,"
                         " increases total cost by %4", "%1 is the id, %2 is the detailed description of the activity")
                    .arg(r.activeActivitiesList[ai]->id)
                    .arg(r.getActivityDetailedDescription(r.activeActivitiesList[ai]->id))
                    .arg(this->componentNumber)
                    .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintSubactivitiesPreferredStartingTimes::isRelatedToActivity( Activity* a)
{
    if(!a->representsComponentNumber(this->componentNumber))
        return false;

    //check if this activity has the corresponding students
    if(this->studentsName!=""){
        bool commonStudents=false;
        foreach(QString st, a->studentSetsNames){
            if(r.setsShareStudents(st, this->studentsName)){
                commonStudents=true;
                break;
            }
        }
        if(!commonStudents)
            return false;
    }
    //check if this activity has the corresponding subject
    if(this->subjectName!="" && a->subjectName!=this->subjectName)
        return false;
    //check if this activity has the corresponding activity tag
    if(this->activityTagName!="" && !a->activityTagsNames.contains(this->activityTagName))
        return false;

    return true;
}

bool ConstraintSubactivitiesPreferredStartingTimes::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintSubactivitiesPreferredStartingTimes::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredStartingTimes::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredStartingTimes::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintSubactivitiesPreferredStartingTimes::hasWrongDayOrHour()
{
    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)<0 || days_L.at(i)>=r.nDaysPerWeek
                || hours_L.at(i)<0 || hours_L.at(i)>=r.nHoursPerDay)
            return true;

    return false;
}

bool ConstraintSubactivitiesPreferredStartingTimes::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintSubactivitiesPreferredStartingTimes::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(nPreferredStartingTimes_L==days_L.count());
    assert(nPreferredStartingTimes_L==hours_L.count());

    QList<int> newDays;
    QList<int> newHours;
    int newNPref=0;

    for(int i=0; i<nPreferredStartingTimes_L; i++)
        if(days_L.at(i)>=0 && days_L.at(i)<r.nDaysPerWeek
                && hours_L.at(i)>=0 && hours_L.at(i)<r.nHoursPerDay){
            newDays.append(days_L.at(i));
            newHours.append(hours_L.at(i));
            newNPref++;
        }

    nPreferredStartingTimes_L=newNPref;
    days_L=newDays;
    hours_L=newHours;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesSameStartingHour::ConstraintActivitiesSameStartingHour(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR;
}

ConstraintActivitiesSameStartingHour::ConstraintActivitiesSameStartingHour(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& act)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    this->type=CONSTRAINT_ACTIVITIES_SAME_STARTING_HOUR;
}

bool ConstraintActivitiesSameStartingHour::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintActivitiesSameStartingHour::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintActivitiesSameStartingHour::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintActivitiesSameStartingHour::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintActivitiesSameStartingHour>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesSameStartingHour>\n";
    return s;
}

QString ConstraintActivitiesSameStartingHour::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Activities same starting hour");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);
        if(i<this->n_activities-1)
            s+=", ";
    }

    return begin+s+end;
}

QString ConstraintActivitiesSameStartingHour::getDetailedDescription(){
    QString s;

    s=tr("Time constraint");s+="\n";
    s+=tr("Activities must have the same starting hour");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
                .arg(this->activitiesId[i])
                .arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesSameStartingHour::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the differences in the scheduled hour for all pairs of activities.

    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            //int day1=t1%r.nDaysPerWeek;
            int hour1=c.getHourForActivityIndex(this->_activities[i]);
            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    //int day2=t2%r.nDaysPerWeek;
                    int hour2=c.getHourForActivityIndex(this->_activities[j]);

                    int tmp=0;

                    //	tmp = abs(hour1-hour2);
                    if(hour1!=hour2)
                        tmp=1;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tmp>0 && collectConflictsData){
                        QString s=tr("Time constraint activities same starting hour broken, because activity with id=%1 (%2) is not at the same hour with activity with id=%3 (%4)"
                                     , "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]));
                        s+=". ";
                        s+=tr("Conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintActivitiesSameStartingHour::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintActivitiesSameStartingHour::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesSameStartingHour::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingHour::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingHour::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingHour::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivitiesSameStartingHour::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivitiesSameStartingHour::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesSameStartingDay::ConstraintActivitiesSameStartingDay(Instance& r)
    : TimeConstraint(r)
{
    type=CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY;
}

ConstraintActivitiesSameStartingDay::ConstraintActivitiesSameStartingDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int nact, const QList<int>& act)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(nact>=2);
    assert(act.count()==nact);
    this->n_activities=nact;
    this->activitiesId.clear();
    for(int i=0; i<nact; i++)
        this->activitiesId.append(act.at(i));

    this->type=CONSTRAINT_ACTIVITIES_SAME_STARTING_DAY;
}

bool ConstraintActivitiesSameStartingDay::computeInternalStructure(QWidget* parent)
{
    //compute the indices of the activities,
    //based on their unique ID

    assert(this->n_activities==this->activitiesId.count());

    this->_activities.clear();
    for(int i=0; i<this->n_activities; i++){
        int j=r.getActivityIndex(activitiesId.at(i));
        //assert(j>=0);
        if(j>=0)
            _activities.append(j);
        /*int j;
                Activity* act;
        for(j=0; j<r.activeActivitiesList.size(); j++){
                        act=&r.internalActivitiesList[j];
                        if(act->id==this->activitiesId[i]){
                                this->_activities.append(j);
                                break;
                        }
                }*/
    }
    this->_n_activities=this->_activities.count();

    if(this->_n_activities<=1){
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because you need 2 or more activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        //assert(0);
        return false;
    }

    return true;
}

void ConstraintActivitiesSameStartingDay::removeUseless()
{
    //remove the activitiesId which no longer exist (used after the deletion of an activity)

    assert(this->n_activities==this->activitiesId.count());

    QList<int> tmpList;

    for(int i=0; i<this->n_activities; i++){
        Activity* act=r.activitiesPointerHash.value(activitiesId[i], nullptr);
        if(act!=nullptr)
            tmpList.append(act->id);
        /*for(int k=0; k<r.activitiesList.size(); k++){
                        Activity* act=r.activitiesList[k];
                        if(act->id==this->activitiesId[i]){
                                tmpList.append(act->id);
                                break;
                        }
                }*/
    }

    this->activitiesId=tmpList;
    this->n_activities=this->activitiesId.count();

    r.internalStructureComputed=false;
}

bool ConstraintActivitiesSameStartingDay::hasInactiveActivities()
{
    int count=0;

    for(int i=0; i<this->n_activities; i++)
        if(r.inactiveActivitiesIdsSet.contains(this->activitiesId[i]))
            count++;

    if(this->n_activities-count<=1)
        return true;
    else
        return false;
}

QString ConstraintActivitiesSameStartingDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintActivitiesSameStartingDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Number_of_Activities>"+utils::strings::number(this->n_activities)+"</Number_of_Activities>\n";
    for(int i=0; i<this->n_activities; i++)
        s+="	<Activity_Id>"+utils::strings::number(this->activitiesId[i])+"</Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesSameStartingDay>\n";
    return s;
}

QString ConstraintActivitiesSameStartingDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Activities same starting day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("NA:%1", "Number of activities").arg(this->n_activities);s+=", ";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Id:%1", "Id of activity").arg(this->activitiesId[i]);
        if(i<this->n_activities-1)
            s+=", ";
    }

    return begin+s+end;
}

QString ConstraintActivitiesSameStartingDay::getDetailedDescription(){
    QString s;

    s=tr("Time constraint");s+="\n";
    s+=tr("Activities must have the same starting day");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Number of activities=%1").arg(this->n_activities);s+="\n";
    for(int i=0; i<this->n_activities; i++){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
                .arg(this->activitiesId[i])
                .arg(r.getActivityDetailedDescription(this->activitiesId[i]));
        s+="\n";
    }

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesSameStartingDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    assert(r.internalStructureComputed);

    int nbroken;

    //We do not use the matrices 'c.subgroupsMatrix' nor 'c.teachersMatrix'.

    //sum the differences in the scheduled hour for all pairs of activities.
    nbroken=0;
    for(int i=1; i<this->_n_activities; i++){
        int t1=c.times[(unsigned)this->_activities[i]];
        if(t1!=UNALLOCATED_TIME){
            int day1=c.getDayForActivityIndex(this->_activities[i]);

            //int hour1=t1/r.nDaysPerWeek;
            for(int j=0; j<i; j++){
                int t2=c.times[(unsigned)this->_activities[j]];
                if(t2!=UNALLOCATED_TIME){
                    int day2=c.getDayForActivityIndex(this->_activities[j]);
                    //int hour2=t2/r.nDaysPerWeek;
                    int tmp=0;

                    if(day1!=day2)
                        tmp=1;

                    int parcialViolation = tmp;

                    nbroken+=parcialViolation;

                    if(tmp>0 && collectConflictsData){
                        QString s=tr("Time constraint activities same starting day broken, because activity with id=%1 (%2) is not in the same day with activity with id=%3 (%4)"
                                     , "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                                .arg(this->activitiesId[i])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[i]))
                                .arg(this->activitiesId[j])
                                .arg(r.getActivityDetailedDescription(this->activitiesId[j]));
                        s+=". ";
                        s+=tr("Conflicts cost increase=%1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                        c.constraintConflictData.addConflict(group, s, parcialViolation);
                    }
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintActivitiesSameStartingDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    for(int i=0; i<this->n_activities; i++)
        if(this->activitiesId[i]==a->id)
            return true;
    return false;
}

bool ConstraintActivitiesSameStartingDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesSameStartingDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesSameStartingDay::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivitiesSameStartingDay::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivitiesSameStartingDay::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTwoActivitiesConsecutive::ConstraintTwoActivitiesConsecutive(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TWO_ACTIVITIES_CONSECUTIVE;
}

ConstraintTwoActivitiesConsecutive::ConstraintTwoActivitiesConsecutive(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->firstActivityId = firstActId;
    this->secondActivityId=secondActId;
    this->type = CONSTRAINT_TWO_ACTIVITIES_CONSECUTIVE;
}

bool ConstraintTwoActivitiesConsecutive::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->firstActivityId)
                        break;
        }*/

    int i=r.getActivityIndex(firstActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->firstActivityIndex=i;

    ////////

    /*for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->secondActivityId)
                        break;
        }*/

    i=r.getActivityIndex(secondActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->secondActivityIndex=i;

    if(firstActivityIndex==secondActivityIndex){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to same activities):\n%1").arg(this->getDetailedDescription()));
        return false;
    }
    assert(firstActivityIndex!=secondActivityIndex);

    return true;
}

bool ConstraintTwoActivitiesConsecutive::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->firstActivityId))
        return true;
    if(r.inactiveActivitiesIdsSet.contains(this->secondActivityId))
        return true;
    return false;
}

QString ConstraintTwoActivitiesConsecutive::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTwoActivitiesConsecutive>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<First_Activity_Id>"+utils::strings::number(this->firstActivityId)+"</First_Activity_Id>\n";
    s+="	<Second_Activity_Id>"+utils::strings::number(this->secondActivityId)+"</Second_Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTwoActivitiesConsecutive>\n";
    return s;
}

QString ConstraintTwoActivitiesConsecutive::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    s=tr("Two activities consecutive:");
    s+=" ";

    s+=tr("first act. id: %1", "act.=activity").arg(this->firstActivityId);
    s+=", ";
    s+=tr("second act. id: %1", "act.=activity").arg(this->secondActivityId);
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTwoActivitiesConsecutive::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Two activities consecutive (second activity must be placed immediately after the first"
          " activity, in the same day, possibly separated by breaks)"); s+="\n";s+= getSuperDetailedDescription();s+="\n";

    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("First activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->firstActivityId)
            .arg(r.getActivityDetailedDescription(this->firstActivityId));
    s+="\n";

    s+=tr("Second activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->secondActivityId)
            .arg(r.getActivityDetailedDescription(this->secondActivityId));
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTwoActivitiesConsecutive::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->firstActivityIndex]!=UNALLOCATED_TIME && c.times[(unsigned)this->secondActivityIndex]!=UNALLOCATED_TIME){
        int fd=c.getDayForActivityIndex(this->firstActivityIndex); //the day when first activity was scheduled
        int fh=c.getHourForActivityIndex(this->firstActivityIndex); //the hour
        int sd=c.getDayForActivityIndex(this->secondActivityIndex); //the day when second activity was scheduled
        int sh=c.getHourForActivityIndex(this->secondActivityIndex); //the hour

        if(fd!=sd)
            nbroken=1;
        else if(fh+r.activeActivitiesList[this->firstActivityIndex]->duration>sh)
            nbroken=1;
        else{
            assert(fd==sd);
            int h;
            int d=fd;
            assert(d==sd);
            for(h=fh+r.activeActivitiesList[this->firstActivityIndex]->duration; h<r.nHoursPerDay; h++)
                if(!c.instance->breakDayHour[d][h])
                    break;

            assert(h<=sh);

            if(h!=sh)
                nbroken=1;
        }
    }

    assert(nbroken==0 || nbroken==1);

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint two activities consecutive broken for first activity with id=%1 (%2) and "
                     "second activity with id=%3 (%4), increases total cost by %5", "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                .arg(this->firstActivityId)
                .arg(r.getActivityDetailedDescription(this->firstActivityId))
                .arg(this->secondActivityId)
                .arg(r.getActivityDetailedDescription(this->secondActivityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintTwoActivitiesConsecutive::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->firstActivityId==a->id)
        return true;
    if(this->secondActivityId==a->id)
        return true;
    return false;
}

bool ConstraintTwoActivitiesConsecutive::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintTwoActivitiesConsecutive::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesConsecutive::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesConsecutive::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesConsecutive::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintTwoActivitiesConsecutive::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintTwoActivitiesConsecutive::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTwoActivitiesGrouped::ConstraintTwoActivitiesGrouped(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TWO_ACTIVITIES_GROUPED;
}

ConstraintTwoActivitiesGrouped::ConstraintTwoActivitiesGrouped(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->firstActivityId = firstActId;
    this->secondActivityId=secondActId;
    this->type = CONSTRAINT_TWO_ACTIVITIES_GROUPED;
}

bool ConstraintTwoActivitiesGrouped::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->firstActivityId)
                        break;
        }*/

    int i=r.getActivityIndex(firstActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->firstActivityIndex=i;

    ////////

    /*for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->secondActivityId)
                        break;
        }*/

    i=r.getActivityIndex(secondActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->secondActivityIndex=i;

    if(firstActivityIndex==secondActivityIndex){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to same activities):\n%1").arg(this->getDetailedDescription()));
        return false;
    }
    assert(firstActivityIndex!=secondActivityIndex);

    return true;
}

bool ConstraintTwoActivitiesGrouped::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->firstActivityId))
        return true;
    if(r.inactiveActivitiesIdsSet.contains(this->secondActivityId))
        return true;
    return false;
}

QString ConstraintTwoActivitiesGrouped::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTwoActivitiesGrouped>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<First_Activity_Id>"+utils::strings::number(this->firstActivityId)+"</First_Activity_Id>\n";
    s+="	<Second_Activity_Id>"+utils::strings::number(this->secondActivityId)+"</Second_Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTwoActivitiesGrouped>\n";
    return s;
}

QString ConstraintTwoActivitiesGrouped::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    s=tr("Two activities grouped:");
    s+=" ";

    s+=tr("first act. id: %1", "act.=activity").arg(this->firstActivityId);
    s+=", ";
    s+=tr("second act. id: %1", "act.=activity").arg(this->secondActivityId);
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTwoActivitiesGrouped::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Two activities grouped (the activities must be placed in the same day, "
          "one immediately following the other, in any order, possibly separated by breaks)"); s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("First activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->firstActivityId)
            .arg(r.getActivityDetailedDescription(this->firstActivityId));
    s+="\n";

    s+=tr("Second activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->secondActivityId)
            .arg(r.getActivityDetailedDescription(this->secondActivityId));
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTwoActivitiesGrouped::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->firstActivityIndex]!=UNALLOCATED_TIME && c.times[(unsigned)this->secondActivityIndex]!=UNALLOCATED_TIME){
        int fd=c.getDayForActivityIndex(this->firstActivityIndex); //the day when first activity was scheduled
        int fh=c.getHourForActivityIndex(this->firstActivityIndex); //the hour
        int sd=c.getDayForActivityIndex(this->secondActivityIndex); //the day when second activity was scheduled
        int sh=c.getHourForActivityIndex(this->secondActivityIndex); //the hour

        if(fd!=sd)
            nbroken=1;
        else if(fd==sd && fh+r.activeActivitiesList[this->firstActivityIndex]->duration <= sh){
            int h;
            int d=fd;
            assert(d==sd);
            for(h=fh+r.activeActivitiesList[this->firstActivityIndex]->duration; h<r.nHoursPerDay; h++)
                if(!c.instance->breakDayHour[d][h])
                    break;

            assert(h<=sh);

            if(h!=sh)
                nbroken=1;
        }
        else if(fd==sd && sh+r.activeActivitiesList[this->secondActivityIndex]->duration <= fh){
            int h;
            int d=sd;
            assert(d==fd);
            for(h=sh+r.activeActivitiesList[this->secondActivityIndex]->duration; h<r.nHoursPerDay; h++)
                if(!c.instance->breakDayHour[d][h])
                    break;

            assert(h<=fh);

            if(h!=fh)
                nbroken=1;
        }
        else
            nbroken=1;
    }

    assert(nbroken==0 || nbroken==1);

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint two activities grouped broken for first activity with id=%1 (%2) and "
                     "second activity with id=%3 (%4), increases total cost by %5", "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                .arg(this->firstActivityId)
                .arg(r.getActivityDetailedDescription(this->firstActivityId))
                .arg(this->secondActivityId)
                .arg(r.getActivityDetailedDescription(this->secondActivityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintTwoActivitiesGrouped::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->firstActivityId==a->id)
        return true;
    if(this->secondActivityId==a->id)
        return true;
    return false;
}

bool ConstraintTwoActivitiesGrouped::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintTwoActivitiesGrouped::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesGrouped::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesGrouped::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesGrouped::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintTwoActivitiesGrouped::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintTwoActivitiesGrouped::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintThreeActivitiesGrouped::ConstraintThreeActivitiesGrouped(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_THREE_ACTIVITIES_GROUPED;
}

ConstraintThreeActivitiesGrouped::ConstraintThreeActivitiesGrouped(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId, int thirdActId)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->firstActivityId = firstActId;
    this->secondActivityId=secondActId;
    this->thirdActivityId=thirdActId;
    this->type = CONSTRAINT_THREE_ACTIVITIES_GROUPED;
}

bool ConstraintThreeActivitiesGrouped::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->firstActivityId)
                        break;
        }*/

    int i=r.getActivityIndex(firstActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->firstActivityIndex=i;

    ////////

    /*for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->secondActivityId)
                        break;
        }*/

    i=r.getActivityIndex(secondActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->secondActivityIndex=i;

    ////////

    /*for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->thirdActivityId)
                        break;
        }*/

    i=r.getActivityIndex(thirdActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->thirdActivityIndex=i;

    if(firstActivityIndex==secondActivityIndex || firstActivityIndex==thirdActivityIndex || secondActivityIndex==thirdActivityIndex){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to same activities):\n%1").arg(this->getDetailedDescription()));
        return false;
    }
    assert(firstActivityIndex!=secondActivityIndex && firstActivityIndex!=thirdActivityIndex && secondActivityIndex!=thirdActivityIndex);

    return true;
}

bool ConstraintThreeActivitiesGrouped::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->firstActivityId))
        return true;
    if(r.inactiveActivitiesIdsSet.contains(this->secondActivityId))
        return true;
    if(r.inactiveActivitiesIdsSet.contains(this->thirdActivityId))
        return true;
    return false;
}

QString ConstraintThreeActivitiesGrouped::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintThreeActivitiesGrouped>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<First_Activity_Id>"+utils::strings::number(this->firstActivityId)+"</First_Activity_Id>\n";
    s+="	<Second_Activity_Id>"+utils::strings::number(this->secondActivityId)+"</Second_Activity_Id>\n";
    s+="	<Third_Activity_Id>"+utils::strings::number(this->thirdActivityId)+"</Third_Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintThreeActivitiesGrouped>\n";
    return s;
}

QString ConstraintThreeActivitiesGrouped::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    s=tr("Three activities grouped:");
    s+=" ";

    s+=tr("first act. id: %1", "act.=activity").arg(this->firstActivityId);
    s+=", ";
    s+=tr("second act. id: %1", "act.=activity").arg(this->secondActivityId);
    s+=", ";
    s+=tr("third act. id: %1", "act.=activity").arg(this->thirdActivityId);
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintThreeActivitiesGrouped::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Three activities grouped (the activities must be placed in the same day, "
          "one immediately following the other, as a block of three activities, in any order, possibly separated by breaks)"); s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("First activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->firstActivityId)
            .arg(r.getActivityDetailedDescription(this->firstActivityId));
    s+="\n";

    s+=tr("Second activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->secondActivityId)
            .arg(r.getActivityDetailedDescription(this->secondActivityId));
    s+="\n";

    s+=tr("Third activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->thirdActivityId)
            .arg(r.getActivityDetailedDescription(this->thirdActivityId));
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintThreeActivitiesGrouped::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->firstActivityIndex]!=UNALLOCATED_TIME
            && c.times[(unsigned)this->secondActivityIndex]!=UNALLOCATED_TIME
            && c.times[(unsigned)this->thirdActivityIndex]!=UNALLOCATED_TIME){

        int fd=c.getDayForActivityIndex(this->firstActivityIndex); //the day when first activity was scheduled
        int fh=c.getHourForActivityIndex(this->firstActivityIndex); //the hour
        int sd=c.getDayForActivityIndex(this->secondActivityIndex); //the day when second activity was scheduled
        int sh=c.getHourForActivityIndex(this->secondActivityIndex); //the hour
        int td=c.getDayForActivityIndex(this->thirdActivityIndex); //the day when third activity was scheduled
        int th=c.getHourForActivityIndex(this->thirdActivityIndex); //the hour

        if(!(fd==sd && fd==td))
            nbroken=1;
        else{
            assert(fd==sd && fd==td && sd==td);
            int a1=-1,a2=-1,a3=-1;
            if(fh>=sh && fh>=th && sh>=th){
                a1=thirdActivityIndex;
                a2=secondActivityIndex;
                a3=firstActivityIndex;
                //out<<"321"<<endl;
            }
            else if(fh>=sh && fh>=th && th>=sh){
                a1=secondActivityIndex;
                a2=thirdActivityIndex;
                a3=firstActivityIndex;
                //out<<"231"<<endl;
            }
            else if(sh>=fh && sh>=th && fh>=th){
                a1=thirdActivityIndex;
                a2=firstActivityIndex;
                a3=secondActivityIndex;
                //out<<"312"<<endl;
            }
            else if(sh>=fh && sh>=th && th>=fh){
                a1=firstActivityIndex;
                a2=thirdActivityIndex;
                a3=secondActivityIndex;
                //out<<"132"<<endl;
            }
            else if(th>=fh && th>=sh && fh>=sh){
                a1=secondActivityIndex;
                a2=firstActivityIndex;
                a3=thirdActivityIndex;
                //out<<"213"<<endl;
            }
            else if(th>=fh && th>=sh && sh>=fh){
                a1=firstActivityIndex;
                a2=secondActivityIndex;
                a3=thirdActivityIndex;
                //out<<"123"<<endl;
            }
            else
                assert(0);

            int a1d=c.getDayForActivityIndex(a1); //the day for a1
            int a1h=c.getHourForActivityIndex(a1); //the day for a1
            int a1dur=r.activeActivitiesList[a1]->duration;

            int a2d=c.getDayForActivityIndex(a2); //the day for a2
            int a2h=c.getHourForActivityIndex(a2); //the day for a2
            int a2dur=r.activeActivitiesList[a2]->duration;

            int a3d=c.getDayForActivityIndex(a3); //the day for a3
            int a3h=c.getHourForActivityIndex(a3); //the day for a3
            //int a3dur=r.internalActivitiesList[a3].duration;

            int hoursBetweenThem=-1;

            assert(a1d==a2d && a1d==a3d);

            if(a1h+a1dur<=a2h && a2h+a2dur<=a3h){
                hoursBetweenThem=0;
                for(int hh=a1h+a1dur; hh<a2h; hh++)
                    if(!c.instance->breakDayHour[a1d][hh])
                        hoursBetweenThem++;

                for(int hh=a2h+a2dur; hh<a3h; hh++)
                    if(!c.instance->breakDayHour[a2d][hh])
                        hoursBetweenThem++;
            }

            if(hoursBetweenThem==0)
                nbroken=0;
            else
                nbroken=1;
        }
    }

    assert(nbroken==0 || nbroken==1);

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint three activities grouped broken for first activity with id=%1 (%2), "
                     "second activity with id=%3 (%4) and third activity with id=%5 (%6), increases total cost by %7",
                     "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr., %5 id, %6 det. descr.")
                .arg(this->firstActivityId)
                .arg(r.getActivityDetailedDescription(this->firstActivityId))
                .arg(this->secondActivityId)
                .arg(r.getActivityDetailedDescription(this->secondActivityId))
                .arg(this->thirdActivityId)
                .arg(r.getActivityDetailedDescription(this->thirdActivityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintThreeActivitiesGrouped::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->firstActivityId==a->id)
        return true;
    if(this->secondActivityId==a->id)
        return true;
    if(this->thirdActivityId==a->id)
        return true;
    return false;
}

bool ConstraintThreeActivitiesGrouped::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintThreeActivitiesGrouped::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintThreeActivitiesGrouped::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintThreeActivitiesGrouped::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintThreeActivitiesGrouped::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintThreeActivitiesGrouped::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintThreeActivitiesGrouped::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintTwoActivitiesOrdered::ConstraintTwoActivitiesOrdered(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_TWO_ACTIVITIES_ORDERED;
}

ConstraintTwoActivitiesOrdered::ConstraintTwoActivitiesOrdered(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int firstActId, int secondActId)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->firstActivityId = firstActId;
    this->secondActivityId=secondActId;
    this->type = CONSTRAINT_TWO_ACTIVITIES_ORDERED;
}

bool ConstraintTwoActivitiesOrdered::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->firstActivityId)
                        break;
        }*/

    int i=r.getActivityIndex(firstActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->firstActivityIndex=i;

    ////////

    /*for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->secondActivityId)
                        break;
        }*/

    i=r.getActivityIndex(secondActivityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to inexistent activity ids):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->secondActivityIndex=i;

    if(firstActivityIndex==secondActivityIndex){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to same activities):\n%1").arg(this->getDetailedDescription()));
        return false;
    }
    assert(firstActivityIndex!=secondActivityIndex);

    return true;
}

bool ConstraintTwoActivitiesOrdered::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->firstActivityId))
        return true;
    if(r.inactiveActivitiesIdsSet.contains(this->secondActivityId))
        return true;
    return false;
}

QString ConstraintTwoActivitiesOrdered::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTwoActivitiesOrdered>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<First_Activity_Id>"+utils::strings::number(this->firstActivityId)+"</First_Activity_Id>\n";
    s+="	<Second_Activity_Id>"+utils::strings::number(this->secondActivityId)+"</Second_Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTwoActivitiesOrdered>\n";
    return s;
}

QString ConstraintTwoActivitiesOrdered::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;

    s=tr("Two activities ordered:");
    s+=" ";

    s+=tr("first act. id: %1", "act.=activity").arg(this->firstActivityId);
    s+=", ";
    s+=tr("second act. id: %1", "act.=activity").arg(this->secondActivityId);
    s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTwoActivitiesOrdered::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Two activities ordered (second activity must be placed at any time in the week after the first"
          " activity)"); s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";

    s+=tr("First activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->firstActivityId)
            .arg(r.getActivityDetailedDescription(this->firstActivityId));
    s+="\n";

    s+=tr("Second activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->secondActivityId)
            .arg(r.getActivityDetailedDescription(this->secondActivityId));
    s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTwoActivitiesOrdered::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->firstActivityIndex]!=UNALLOCATED_TIME && c.times[(unsigned)this->secondActivityIndex]!=UNALLOCATED_TIME){
        int fd=c.getDayForActivityIndex(this->firstActivityIndex); //the day when first activity was scheduled
        int fh=c.getHourForActivityIndex(this->firstActivityIndex)
                + r.activeActivitiesList[this->firstActivityIndex]->duration-1; //the end hour of first activity
        int sd=c.getDayForActivityIndex(this->secondActivityIndex); //the day when second activity was scheduled
        int sh=c.getHourForActivityIndex(this->secondActivityIndex); //the start hour of second activity

        if(!(fd<sd || (fd==sd && fh<sh)))
            nbroken=1;
    }

    assert(nbroken==0 || nbroken==1);

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint two activities ordered broken for first activity with id=%1 (%2) and "
                     "second activity with id=%3 (%4), increases total cost by %5", "%1 is the id, %2 is the detailed description of the activity, %3 id, %4 det. descr.")
                .arg(this->firstActivityId)
                .arg(r.getActivityDetailedDescription(this->firstActivityId))
                .arg(this->secondActivityId)
                .arg(r.getActivityDetailedDescription(this->secondActivityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintTwoActivitiesOrdered::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->firstActivityId==a->id)
        return true;
    if(this->secondActivityId==a->id)
        return true;
    return false;
}

bool ConstraintTwoActivitiesOrdered::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintTwoActivitiesOrdered::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesOrdered::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesOrdered::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTwoActivitiesOrdered::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintTwoActivitiesOrdered::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintTwoActivitiesOrdered::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivityEndsStudentsDay::ConstraintActivityEndsStudentsDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITY_ENDS_STUDENTS_DAY;
}

ConstraintActivityEndsStudentsDay::ConstraintActivityEndsStudentsDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int actId)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->activityId = actId;
    this->type = CONSTRAINT_ACTIVITY_ENDS_STUDENTS_DAY;
}

bool ConstraintActivityEndsStudentsDay::computeInternalStructure(QWidget* parent)
{
    /*Activity* act;
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
                act=&r.internalActivitiesList[i];
                if(act->id==this->activityId)
                        break;
        }*/

    int i=r.getActivityIndex(activityId);

    if(i==-1){
        //assert(0);
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (because it refers to invalid activity id). Please correct it (maybe removing it is a solution):\n%1").arg(this->getDetailedDescription()));
        return false;
    }

    this->activityIndex=i;
    return true;
}

bool ConstraintActivityEndsStudentsDay::hasInactiveActivities()
{
    if(r.inactiveActivitiesIdsSet.contains(this->activityId))
        return true;
    return false;
}

QString ConstraintActivityEndsStudentsDay::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintActivityEndsStudentsDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Id>"+utils::strings::number(this->activityId)+"</Activity_Id>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivityEndsStudentsDay>\n";
    return s;
}

QString ConstraintActivityEndsStudentsDay::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Act. id: %1 (%2) must end students' day",
          "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId));
    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivityEndsStudentsDay::getDetailedDescription()
{
    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activity must end students' day");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Activity id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity.")
            .arg(this->activityId)
            .arg(r.getActivityDetailedDescription(this->activityId));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivityEndsStudentsDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    nbroken=0;
    if(c.times[(unsigned)this->activityIndex]!=UNALLOCATED_TIME){
        int h=c.getHourForActivityIndex(this->activityIndex);
        int d=c.getDayForActivityIndex(this->activityIndex);

        int i=this->activityIndex;
        for(int j=0; j<r.activeActivitiesList[i]->iSubgroupsList.count(); j++){
            int sb=r.activeActivitiesList[i]->iSubgroupsList.at(j);
            for(int hh=h+r.activeActivitiesList[i]->duration; hh<r.nHoursPerDay; hh++)
                if(c.subgroupsMatrix[sb][d][hh]>0){
                    nbroken=1;
                    break;
                }
            if(nbroken)
                break;
        }
    }

    if(collectConflictsData && nbroken>0){
        int parcialViolation = nbroken;

        QString s=tr("Time constraint activity ends students' day broken for activity with id=%1 (%2), increases total cost by %3",
                     "%1 is the id, %2 is the detailed description of the activity")
                .arg(this->activityId)
                .arg(r.getActivityDetailedDescription(this->activityId))
                .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

        c.constraintConflictData.addConflict(group, s, parcialViolation);
    }

    return nbroken;
}

bool ConstraintActivityEndsStudentsDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    if(this->activityId==a->id)
        return true;
    return false;
}

bool ConstraintActivityEndsStudentsDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivityEndsStudentsDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityEndsStudentsDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityEndsStudentsDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivityEndsStudentsDay::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivityEndsStudentsDay::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivityEndsStudentsDay::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMinHoursDaily::ConstraintTeachersMinHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MIN_HOURS_DAILY;

    this->allowEmptyDays=true;
}

ConstraintTeachersMinHoursDaily::ConstraintTeachersMinHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minhours, bool _allowEmptyDays)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(minhours>0);
    this->minHoursDaily=minhours;

    this->allowEmptyDays=_allowEmptyDays;

    this->type=CONSTRAINT_TEACHERS_MIN_HOURS_DAILY;
}

bool ConstraintTeachersMinHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(r);

    if(allowEmptyDays==false){
        QString s=tr("Cannot generate a timetable with a constraint teachers min hours daily with allow empty days=false. Please modify it,"
                     " so that it allows empty days. If you need a facility like that, please use constraint teachers min days per week");
        s+="\n\n";
        s+=tr("Constraint is:")+"\n"+this->getDetailedDescription();
        MessagesManager::warning(parent, tr("m-FET warning"), s);

        return false;
    }

    return true;
}

bool ConstraintTeachersMinHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMinHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMinHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Hours_Daily>"+utils::strings::number(this->minHoursDaily)+"</Minimum_Hours_Daily>\n";
    if(this->allowEmptyDays)
        s+="	<Allow_Empty_Days>true</Allow_Empty_Days>\n";
    else
        s+="	<Allow_Empty_Days>false</Allow_Empty_Days>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMinHoursDaily>\n";
    return s;
}

QString ConstraintTeachersMinHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers min hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("mH:%1", "Min hours (daily)").arg(this->minHoursDaily);s+=", ";
    s+=tr("AED:%1", "Allow empty days").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));

    return begin+s+end;
}

QString ConstraintTeachersMinHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the minimum number of hours daily"); s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Minimum hours daily=%1").arg(this->minHoursDaily);s+="\n";
    s+=tr("Allow empty days=%1").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMinHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    assert(this->allowEmptyDays==true);

    int nbroken;

    nbroken=0;
    for(int i=0; i<r.teachersList.size(); i++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int n_hours_daily=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(c.teachersMatrix[i][d][h]>0)
                    n_hours_daily++;

            if(n_hours_daily>0 && n_hours_daily<this->minHoursDaily){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr("Time constraint teachers min %1 hours daily broken for teacher %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->minHoursDaily))
                               .arg(r.teachersList.at(i)->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg(n_hours_daily)
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintTeachersMinHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);
    Q_UNUSED(r);

    return false;
}

bool ConstraintTeachersMinHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMinHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinHoursDaily::hasWrongDayOrHour()
{
    if(minHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMinHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMinHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minHoursDaily>r.nHoursPerDay)
        minHoursDaily=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMinHoursDaily::ConstraintTeacherMinHoursDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MIN_HOURS_DAILY;

    this->allowEmptyDays=true;
}

ConstraintTeacherMinHoursDaily::ConstraintTeacherMinHoursDaily(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minhours, const QString& teacher, bool _allowEmptyDays)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(minhours>0);
    this->minHoursDaily=minhours;
    this->teacherName=teacher;

    this->allowEmptyDays=_allowEmptyDays;

    this->type=CONSTRAINT_TEACHER_MIN_HOURS_DAILY;
}

bool ConstraintTeacherMinHoursDaily::computeInternalStructure(QWidget* parent)
{
    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);

    if(allowEmptyDays==false){
        QString s=tr("Cannot generate a timetable with a constraint teacher min hours daily with allow empty days=false. Please modify it,"
                     " so that it allows empty days. If you need a facility like that, please use constraint teacher min days per week");
        s+="\n\n";
        s+=tr("Constraint is:")+"\n"+this->getDetailedDescription();
        MessagesManager::warning(parent, tr("m-FET warning"), s);

        return false;
    }

    return true;
}

bool ConstraintTeacherMinHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMinHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMinHoursDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Minimum_Hours_Daily>"+utils::strings::number(this->minHoursDaily)+"</Minimum_Hours_Daily>\n";
    if(this->allowEmptyDays)
        s+="	<Allow_Empty_Days>true</Allow_Empty_Days>\n";
    else
        s+="	<Allow_Empty_Days>false</Allow_Empty_Days>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMinHoursDaily>\n";
    return s;
}

QString ConstraintTeacherMinHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher min hours daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("mH:%1", "Minimum hours (daily)").arg(this->minHoursDaily);s+=", ";
    s+=tr("AED:%1", "Allow empty days").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));

    return begin+s+end;
}

QString ConstraintTeacherMinHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the minimum number of hours daily");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Minimum hours daily=%1").arg(this->minHoursDaily);s+="\n";
    s+=tr("Allow empty days=%1").arg(utils::strings::yesNoTranslated(this->allowEmptyDays));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMinHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    assert(this->allowEmptyDays==true);

    int nbroken;
    nbroken=0;

    int i=this->teacher_ID;
    for(int d=0; d<r.nDaysPerWeek; d++){
        int n_hours_daily=0;
        for(int h=0; h<r.nHoursPerDay; h++)
            if(c.teachersMatrix[i][d][h]>0)
                n_hours_daily++;

        if(n_hours_daily>0 && n_hours_daily<this->minHoursDaily){
            int parcialViolation = 1;

            nbroken++;

            if(collectConflictsData){
                QString s=(tr(
                               "Time constraint teacher min %1 hours daily broken for teacher %2, on day %3, length=%4.")
                           .arg(utils::strings::number(this->minHoursDaily))
                           .arg(r.teachersList.at(i)->name)
                           .arg(r.daysOfTheWeek[d])
                           .arg(n_hours_daily)
                           )
                        +" "
                        +
                        tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeacherMinHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMinHoursDaily::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMinHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinHoursDaily::hasWrongDayOrHour()
{
    if(minHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMinHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMinHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minHoursDaily>r.nHoursPerDay)
        minHoursDaily=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMinDaysPerWeek::ConstraintTeacherMinDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK;
}

ConstraintTeacherMinDaysPerWeek::ConstraintTeacherMinDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mindays, const QString& teacher)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(mindays>0);
    this->minDaysPerWeek=mindays;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MIN_DAYS_PER_WEEK;
}

bool ConstraintTeacherMinDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMinDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMinDaysPerWeek::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMinDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Minimum_Days_Per_Week>"+utils::strings::number(this->minDaysPerWeek)+"</Minimum_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMinDaysPerWeek>\n";
    return s;
}

QString ConstraintTeacherMinDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher min days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("mD:%1", "Minimum days per week").arg(this->minDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeacherMinDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the minimum number of days per week");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Minimum days per week=%1").arg(this->minDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMinDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;
    int i=this->teacher_ID;
    int nd=0;
    for(int d=0; d<r.nDaysPerWeek; d++){
        for(int h=0; h<r.nHoursPerDay; h++){
            if(c.teachersMatrix[i][d][h]>0){
                nd++;
                break;
            }
        }
    }

    if(nd<this->minDaysPerWeek){        
        nbroken+=this->minDaysPerWeek-nd;

        int parcialViolation = nbroken;

        if(collectConflictsData){
            QString s=(tr(
                           "Time constraint teacher min %1 days per week broken for teacher %2.")
                       .arg(utils::strings::number(this->minDaysPerWeek))
                       .arg(r.teachersList.at(i)->name)
                       )
                    +" "
                    +
                    tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintTeacherMinDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMinDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMinDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinDaysPerWeek::hasWrongDayOrHour()
{
    if(minDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeacherMinDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMinDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minDaysPerWeek>r.nDaysPerWeek)
        minDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMinDaysPerWeek::ConstraintTeachersMinDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MIN_DAYS_PER_WEEK;
}

ConstraintTeachersMinDaysPerWeek::ConstraintTeachersMinDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mindays)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(mindays>0);
    this->minDaysPerWeek=mindays;

    this->type=CONSTRAINT_TEACHERS_MIN_DAYS_PER_WEEK;
}

bool ConstraintTeachersMinDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMinDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMinDaysPerWeek::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMinDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Days_Per_Week>"+utils::strings::number(this->minDaysPerWeek)+"</Minimum_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMinDaysPerWeek>\n";
    return s;
}

QString ConstraintTeachersMinDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers min days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("mD:%1", "Minimum days per week").arg(this->minDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeachersMinDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the minimum number of days per week");s+="\n";s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Minimum days per week=%1").arg(this->minDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMinDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbrokentotal=0;
    for(int i=0; i<r.teachersList.size(); i++){
        int nbroken;

        nbroken=0;
        //int i=this->teacher_ID;
        int nd=0;
        for(int d=0; d<r.nDaysPerWeek; d++){
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.teachersMatrix[i][d][h]>0){
                    nd++;
                    break;
                }
            }
        }

        if(nd<this->minDaysPerWeek){
            nbroken+=this->minDaysPerWeek-nd;
            nbrokentotal+=nbroken;

            int parcialViolation = nbroken;

            if(collectConflictsData){
                QString s=(tr(
                               "Time constraint teachers min %1 days per week broken for teacher %2.")
                           .arg(utils::strings::number(this->minDaysPerWeek))
                           .arg(r.teachersList.at(i)->name)
                           )
                        +" "
                        +
                        tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbrokentotal;
}

bool ConstraintTeachersMinDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMinDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);
    return true;
}

bool ConstraintTeachersMinDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinDaysPerWeek::hasWrongDayOrHour()
{
    if(minDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeachersMinDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMinDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minDaysPerWeek>r.nDaysPerWeek)
        minDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherIntervalMaxDaysPerWeek::ConstraintTeacherIntervalMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK;
}

ConstraintTeacherIntervalMaxDaysPerWeek::ConstraintTeacherIntervalMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString tn, int sh, int eh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->teacherName = tn;
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_TEACHER_INTERVAL_MAX_DAYS_PER_WEEK;
    this->startHour=sh;
    this->endHour=eh;
    assert(sh<eh);
    assert(sh>=0);
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    if(this->startHour>=this->endHour){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teacher interval max days per week is wrong because start hour >= end hour."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->startHour<0){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teacher interval max days per week is wrong because start hour < first hour of the day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->endHour>r.nHoursPerDay){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teacher interval max days per week is wrong because end hour > number of hours per day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    return true;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherIntervalMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTeacherIntervalMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Interval_Start_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->startHour])+"</Interval_Start_Hour>\n";
    if(this->endHour < r.nHoursPerDay){
        s+="	<Interval_End_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->endHour])+"</Interval_End_Hour>\n";
    }
    else{
        s+="	<Interval_End_Hour></Interval_End_Hour>\n";
        s+="	<!-- Interval_End_Hour void means the end of the day (which has no name) -->\n";
    }
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherIntervalMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintTeacherIntervalMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Teacher interval max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Abbreviation for teacher").arg(this->teacherName);s+=", ";
    s+=tr("ISH:%1", "Abbreviation for interval start hour").arg(r.hoursOfTheDay[this->startHour]);s+=", ";
    if(this->endHour<r.nHoursPerDay)
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(tr("End of the day"));
    s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeacherIntervalMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher respects working in an hourly interval a maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Interval start hour=%1").arg(r.hoursOfTheDay[this->startHour]);s+="\n";

    if(this->endHour<r.nHoursPerDay)
        s+=tr("Interval end hour=%1").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("Interval end hour=%1").arg(tr("End of the day"));
    s+="\n";

    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherIntervalMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    int t=this->teacher_ID;

    nbroken=0;
    bool ocDay[MAX_DAYS_PER_WEEK];
    for(int d=0; d<r.nDaysPerWeek; d++){
        ocDay[d]=false;
        for(int h=startHour; h<endHour; h++){
            if(c.teachersMatrix[t][d][h]>0){
                ocDay[d]=true;
            }
        }
    }
    int nOcDays=0;
    for(int d=0; d<r.nDaysPerWeek; d++)
        if(ocDay[d])
            nOcDays++;
    if(nOcDays > this->maxDaysPerWeek){
        nbroken+=nOcDays-this->maxDaysPerWeek;

        int parcialViolation = nbroken;

        if(nbroken>0){
            QString s= tr("Time constraint teacher interval max days per week broken for teacher: %1, allowed %2 days, required %3 days.")
                    .arg(r.teachersList.at(t)->name)
                    .arg(this->maxDaysPerWeek)
                    .arg(nOcDays);
            s+=" ";
            s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->startHour>=r.nHoursPerDay)
        return true;
    if(this->endHour>r.nHoursPerDay)
        return true;
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherIntervalMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay);

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersIntervalMaxDaysPerWeek::ConstraintTeachersIntervalMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_INTERVAL_MAX_DAYS_PER_WEEK;
}

ConstraintTeachersIntervalMaxDaysPerWeek::ConstraintTeachersIntervalMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, int sh, int eh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_TEACHERS_INTERVAL_MAX_DAYS_PER_WEEK;
    this->startHour=sh;
    this->endHour=eh;
    assert(sh<eh);
    assert(sh>=0);
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    if(this->startHour>=this->endHour){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teachers interval max days per week is wrong because start hour >= end hour."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->startHour<0){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teachers interval max days per week is wrong because start hour < first hour of the day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->endHour>r.nHoursPerDay){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint teachers interval max days per week is wrong because end hour > number of hours per day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    return true;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersIntervalMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintTeachersIntervalMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Interval_Start_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->startHour])+"</Interval_Start_Hour>\n";
    if(this->endHour < r.nHoursPerDay){
        s+="	<Interval_End_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->endHour])+"</Interval_End_Hour>\n";
    }
    else{
        s+="	<Interval_End_Hour></Interval_End_Hour>\n";
        s+="	<!-- Interval_End_Hour void means the end of the day (which has no name) -->\n";
    }
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersIntervalMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintTeachersIntervalMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Teachers interval max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("ISH:%1", "Abbreviation for interval start hour").arg(r.hoursOfTheDay[this->startHour]);
    s+=", ";
    if(this->endHour<r.nHoursPerDay)
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(tr("End of the day"));
    s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintTeachersIntervalMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers respect working in an hourly interval a maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Interval start hour=%1").arg(r.hoursOfTheDay[this->startHour]);s+="\n";

    if(this->endHour<r.nHoursPerDay)
        s+=tr("Interval end hour=%1").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("Interval end hour=%1").arg(tr("End of the day"));
    s+="\n";

    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersIntervalMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken=0;

    for(int t=0; t<r.teachersList.size(); t++){
        bool ocDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<r.nDaysPerWeek; d++){
            ocDay[d]=false;
            for(int h=startHour; h<endHour; h++){
                if(c.teachersMatrix[t][d][h]>0){
                    ocDay[d]=true;
                }
            }
        }
        int nOcDays=0;
        for(int d=0; d<r.nDaysPerWeek; d++)
            if(ocDay[d])
                nOcDays++;
        if(nOcDays > this->maxDaysPerWeek){
            int parcialViolation = nOcDays-this->maxDaysPerWeek;

            nbroken+=parcialViolation;

            if(nOcDays-this->maxDaysPerWeek>0){
                QString s= tr("Time constraint teachers interval max days per week broken for teacher: %1, allowed %2 days, required %3 days.")
                        .arg(r.teachersList.at(t)->name)
                        .arg(this->maxDaysPerWeek)
                        .arg(nOcDays);
                s+=" ";
                s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->startHour>=r.nHoursPerDay)
        return true;
    if(this->endHour>r.nHoursPerDay)
        return true;
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersIntervalMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay);

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetIntervalMaxDaysPerWeek::ConstraintStudentsSetIntervalMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK;
}

ConstraintStudentsSetIntervalMaxDaysPerWeek::ConstraintStudentsSetIntervalMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString sn, int sh, int eh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->students = sn;
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_STUDENTS_SET_INTERVAL_MAX_DAYS_PER_WEEK;
    this->startHour=sh;
    this->endHour=eh;
    assert(sh<eh);
    assert(sh>=0);
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    if(this->startHour>=this->endHour){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set interval max days per week is wrong because start hour >= end hour."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->startHour<0){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set interval max days per week is wrong because start hour < first hour of the day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->endHour>r.nHoursPerDay){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set interval max days per week is wrong because end hour > number of hours per day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    /////////
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set interval max days per week is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetIntervalMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetIntervalMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Interval_Start_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->startHour])+"</Interval_Start_Hour>\n";
    if(this->endHour < r.nHoursPerDay){
        s+="	<Interval_End_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->endHour])+"</Interval_End_Hour>\n";
    }
    else{
        s+="	<Interval_End_Hour></Interval_End_Hour>\n";
        s+="	<!-- Interval_End_Hour void means the end of the day (which has no name) -->\n";
    }
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetIntervalMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintStudentsSetIntervalMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Students set interval max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Abbreviation for students (sets)").arg(this->students);s+=", ";
    s+=tr("ISH:%1", "Abbreviation for interval start hour").arg(r.hoursOfTheDay[this->startHour]);
    s+=", ";
    if(this->endHour<r.nHoursPerDay)
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(tr("End of the day"));
    s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintStudentsSetIntervalMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set respects working in an hourly interval a maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Interval start hour=%1").arg(r.hoursOfTheDay[this->startHour]);s+="\n";

    if(this->endHour<r.nHoursPerDay)
        s+=tr("Interval end hour=%1").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("Interval end hour=%1").arg(tr("End of the day"));
    s+="\n";

    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetIntervalMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int sbg, this->iSubgroupsList){
        bool ocDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<r.nDaysPerWeek; d++){
            ocDay[d]=false;
            for(int h=startHour; h<endHour; h++){
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    ocDay[d]=true;
                }
            }
        }
        int nOcDays=0;
        for(int d=0; d<r.nDaysPerWeek; d++)
            if(ocDay[d])
                nOcDays++;
        if(nOcDays > this->maxDaysPerWeek){
            int parcialViolation = nOcDays-this->maxDaysPerWeek;

            nbroken+=parcialViolation;

            if((nOcDays-this->maxDaysPerWeek)>0){
                QString s= tr("Time constraint students set interval max days per week broken for subgroup: %1, allowed %2 days, required %3 days.")
                        .arg(r.directSubgroupsList[sbg]->name)
                        .arg(this->maxDaysPerWeek)
                        .arg(nOcDays);
                s+=" ";
                s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);
    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->startHour>=r.nHoursPerDay)
        return true;
    if(this->endHour>r.nHoursPerDay)
        return true;
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetIntervalMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay);

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsIntervalMaxDaysPerWeek::ConstraintStudentsIntervalMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_STUDENTS_INTERVAL_MAX_DAYS_PER_WEEK;
}

ConstraintStudentsIntervalMaxDaysPerWeek::ConstraintStudentsIntervalMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, int sh, int eh)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_STUDENTS_INTERVAL_MAX_DAYS_PER_WEEK;
    this->startHour=sh;
    this->endHour=eh;
    assert(sh<eh);
    assert(sh>=0);
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    if(this->startHour>=this->endHour){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students interval max days per week is wrong because start hour >= end hour."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->startHour<0){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students interval max days per week is wrong because start hour < first hour of the day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }
    if(this->endHour>r.nHoursPerDay){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students interval max days per week is wrong because end hour > number of hours per day."
                                                        " Please correct it. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    return true;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsIntervalMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsIntervalMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Interval_Start_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->startHour])+"</Interval_Start_Hour>\n";
    if(this->endHour < r.nHoursPerDay){
        s+="	<Interval_End_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->endHour])+"</Interval_End_Hour>\n";
    }
    else{
        s+="	<Interval_End_Hour></Interval_End_Hour>\n";
        s+="	<!-- Interval_End_Hour void means the end of the day (which has no name) -->\n";
    }
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsIntervalMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintStudentsIntervalMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Students interval max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("ISH:%1", "Abbreviation for interval start hour").arg(r.hoursOfTheDay[this->startHour]);
    s+=", ";
    if(this->endHour<r.nHoursPerDay)
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("IEH:%1", "Abbreviation for interval end hour").arg(tr("End of the day"));
    s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintStudentsIntervalMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students respect working in an hourly interval a maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Interval start hour=%1").arg(r.hoursOfTheDay[this->startHour]);s+="\n";

    if(this->endHour<r.nHoursPerDay)
        s+=tr("Interval end hour=%1").arg(r.hoursOfTheDay[this->endHour]);
    else
        s+=tr("Interval end hour=%1").arg(tr("End of the day"));
    s+="\n";

    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsIntervalMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    for(int sbg=0; sbg<r.directSubgroupsList.size(); sbg++){
        bool ocDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<r.nDaysPerWeek; d++){
            ocDay[d]=false;
            for(int h=startHour; h<endHour; h++){
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    ocDay[d]=true;
                }
            }
        }
        int nOcDays=0;
        for(int d=0; d<r.nDaysPerWeek; d++)
            if(ocDay[d])
                nOcDays++;
        if(nOcDays > this->maxDaysPerWeek){
            int parcialViolation = nOcDays-this->maxDaysPerWeek;

            nbroken+=parcialViolation;

            if((nOcDays-this->maxDaysPerWeek)>0){
                QString s= tr("Time constraint students interval max days per week broken for subgroup: %1, allowed %2 days, required %3 days.")
                        .arg(r.directSubgroupsList[sbg]->name)
                        .arg(this->maxDaysPerWeek)
                        .arg(nOcDays);
                s+=" ";
                s += tr("This increases the total cost by %1")
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);
    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);
    return true;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->startHour>=r.nHoursPerDay)
        return true;
    if(this->endHour>r.nHoursPerDay)
        return true;
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsIntervalMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(this->startHour<r.nHoursPerDay && this->endHour<=r.nHoursPerDay);

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesEndStudentsDay::ConstraintActivitiesEndStudentsDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY;
}

ConstraintActivitiesEndStudentsDay::ConstraintActivitiesEndStudentsDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, QString te,
                                                                       QString st, QString su, QString sut)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->teacherName=te;
    this->subjectName=su;
    this->activityTagName=sut;
    this->studentsName=st;
    this->type=CONSTRAINT_ACTIVITIES_END_STUDENTS_DAY;
}

bool ConstraintActivitiesEndStudentsDay::computeInternalStructure(QWidget* parent)
{
    this->nActivities=0;
    this->activitiesIndices.clear();

    Activity* act;
    int i;
    for(i=0; i<r.activeActivitiesList.size(); i++){
        act=r.activeActivitiesList[i];

        //check if this activity has the corresponding students
        if(this->studentsName!=""){
            bool commonStudents=false;
            foreach(QString st, act->studentSetsNames)
                if(r.setsShareStudents(st, studentsName)){
                    commonStudents=true;
                    break;
                }

            if(!commonStudents)
                continue;
        }
        //check if this activity has the corresponding subject
        if(this->subjectName!="" && act->subjectName!=this->subjectName){
            continue;
        }
        //check if this activity has the corresponding activity tag
        if(this->activityTagName!="" && !act->activityTagsNames.contains(this->activityTagName)){
            continue;
        }

        assert(this->nActivities < MAX_ACTIVITIES);
        this->nActivities++;
        this->activitiesIndices.append(i);
    }

    assert(this->activitiesIndices.count()==this->nActivities);

    if(this->nActivities>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintActivitiesEndStudentsDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintActivitiesEndStudentsDay::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintActivitiesEndStudentsDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Students_Name>"+utils::strings::parseStrForXml(this->studentsName)+"</Students_Name>\n";
    s+="	<Subject_Name>"+utils::strings::parseStrForXml(this->subjectName)+"</Subject_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesEndStudentsDay>\n";
    return s;
}

QString ConstraintActivitiesEndStudentsDay::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString tc, st, su, at;

    if(this->teacherName!="")
        tc=tr("teacher=%1").arg(this->teacherName);
    else
        tc=tr("all teachers");

    if(this->studentsName!="")
        st=tr("students=%1").arg(this->studentsName);
    else
        st=tr("all students");

    if(this->subjectName!="")
        su=tr("subject=%1").arg(this->subjectName);
    else
        su=tr("all subjects");

    if(this->activityTagName!="")
        at=tr("activity tag=%1").arg(this->activityTagName);
    else
        at=tr("all activity tags");

    QString s;
    s+=tr("Activities with %1, %2, %3, %4, must end students' day", "%1...%4 are conditions for the activities").arg(tc).arg(st).arg(su).arg(at);

    s+=", " + getSuperDescription() + ", ";

    s+=tr("WP:%1%", "Abbreviation for Weight Percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintActivitiesEndStudentsDay::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("Activities with:");s+="\n";

    if(this->teacherName!="")
        s+=tr("Teacher=%1").arg(this->teacherName);
    else
        s+=tr("All teachers");
    s+="\n";

    if(this->studentsName!="")
        s+=tr("Students=%1").arg(this->studentsName);
    else
        s+=tr("All students");
    s+="\n";

    if(this->subjectName!="")
        s+=tr("Subject=%1").arg(this->subjectName);
    else
        s+=tr("All subjects");
    s+="\n";

    if(this->activityTagName!="")
        s+=tr("Activity tag=%1").arg(this->activityTagName);
    else
        s+=tr("All activity tags");
    s+="\n";

    s+=tr("must end students' day");
    s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesEndStudentsDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken=0;

    assert(r.internalStructureComputed);

    for(int kk=0; kk<this->nActivities; kk++){
        int tmp=0;
        int ai=this->activitiesIndices[kk];

        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);

            for(int j=0; j<r.activeActivitiesList[ai]->iSubgroupsList.count(); j++){
                int sb=r.activeActivitiesList[ai]->iSubgroupsList.at(j);
                for(int hh=h+r.activeActivitiesList[ai]->duration; hh<r.nHoursPerDay; hh++)
                    if(c.subgroupsMatrix[sb][d][hh]>0){
                        nbroken++;
                        tmp=1;
                        break;
                    }
                if(tmp>0)
                    break;
            }

            if(collectConflictsData && tmp>0){
                int parcialViolation = tmp;

                QString s=tr("Time constraint activities end students' day broken for activity with id=%1 (%2), increases total cost by %3",
                             "%1 is the id, %2 is the detailed description of the activity")
                        .arg(r.activeActivitiesList[ai]->id)
                        .arg(r.getActivityDetailedDescription(r.activeActivitiesList[ai]->id))
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintActivitiesEndStudentsDay::isRelatedToActivity( Activity* a)
{
    //check if this activity has the corresponding students
    if(this->studentsName!=""){
        bool commonStudents=false;
        foreach(QString st, a->studentSetsNames){
            if(r.setsShareStudents(st, this->studentsName)){
                commonStudents=true;
                break;
            }
        }
        if(!commonStudents)
            return false;
    }
    //check if this activity has the corresponding subject
    if(this->subjectName!="" && a->subjectName!=this->subjectName)
        return false;
    //check if this activity has the corresponding activity tag
    if(this->activityTagName!="" && !a->activityTagsNames.contains(this->activityTagName))
        return false;

    return true;
}

bool ConstraintActivitiesEndStudentsDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesEndStudentsDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesEndStudentsDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesEndStudentsDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesEndStudentsDay::hasWrongDayOrHour()
{
    Q_UNUSED(r);
    return false;
}

bool ConstraintActivitiesEndStudentsDay::canRepairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0);

    return true;
}

bool ConstraintActivitiesEndStudentsDay::repairWrongDayOrHour()
{
    Q_UNUSED(r);
    assert(0); //should check hasWrongDayOrHour, firstly

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxGapsPerDay::ConstraintStudentsMaxGapsPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MAX_GAPS_PER_DAY;
}

ConstraintStudentsMaxGapsPerDay::ConstraintStudentsMaxGapsPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_MAX_GAPS_PER_DAY;
    this->maxGaps=mg;
}

bool ConstraintStudentsMaxGapsPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMaxGapsPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxGapsPerDay::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxGapsPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxGapsPerDay>\n";
    return s;
}

QString ConstraintStudentsMaxGapsPerDay::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Students max gaps per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MG:%1", "Max gaps (per day)").arg(this->maxGaps);

    return begin+s+end;
}

QString ConstraintStudentsMaxGapsPerDay::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("All students must respect the maximum number of gaps per day");s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per day=%1").arg(this->maxGaps);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMaxGapsPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    //returns a number equal to the number of gaps of the subgroups (in hours)
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nGaps;
    int tmp;
    int i;

    int tIllegalGaps=0;

    for(i=0; i<r.directSubgroupsList.size(); i++){
        for(int j=0; j<r.nDaysPerWeek; j++){
            nGaps=0;

            int k;
            tmp=0;
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.subgroupsMatrix[i][j][k]>0){
                    assert(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]);
                    break;
                }
            for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                if(c.subgroupsMatrix[i][j][k]>0){
                    nGaps+=tmp;
                    tmp=0;
                }
                else
                    tmp++;
            }

            int illegalGaps=nGaps-this->maxGaps;
            if(illegalGaps<0)
                illegalGaps=0;

            if(illegalGaps>0 && collectConflictsData){
                int parcialViolation = illegalGaps;

                QString s=tr("Time constraint students max gaps per day broken for subgroup: %1, it has %2 extra gaps, on day %3, cost increase=%4")
                        .arg(r.directSubgroupsList[i]->name)
                        .arg(illegalGaps)
                        .arg(r.daysOfTheWeek[j])
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }

            tIllegalGaps+=illegalGaps;
        }
    }

    return tIllegalGaps;
}

bool ConstraintStudentsMaxGapsPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxGapsPerDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMaxGapsPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxGapsPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxGapsPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMaxGapsPerDay::hasWrongDayOrHour()
{
    if(maxGaps>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMaxGapsPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxGapsPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nHoursPerDay)
        maxGaps=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxGapsPerDay::ConstraintStudentsSetMaxGapsPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY;
}

ConstraintStudentsSetMaxGapsPerDay::ConstraintStudentsSetMaxGapsPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int mg, const QString& st )
    : TimeConstraint(r, constraintGroup, wp)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_GAPS_PER_DAY;
    this->maxGaps=mg;
    this->students = st;
}

bool ConstraintStudentsSetMaxGapsPerDay::computeInternalStructure(QWidget* parent){
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max gaps per day is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

bool ConstraintStudentsSetMaxGapsPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxGapsPerDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxGapsPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Gaps>"+utils::strings::number(this->maxGaps)+"</Max_Gaps>\n";
    s+="	<Students>"; s+=utils::strings::parseStrForXml(this->students); s+="</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxGapsPerDay>\n";
    return s;
}

QString ConstraintStudentsSetMaxGapsPerDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Students set max gaps per day"); s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage)); s+=", ";
    s+=tr("MG:%1", "Max gaps (per day)").arg(this->maxGaps);s+=", ";
    s+=tr("St:%1", "Students").arg(this->students);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxGapsPerDay::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("A students set must respect the maximum number of gaps per day");s+="\n";
    s+=tr("(breaks and students set not available not counted)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum gaps per day=%1").arg(this->maxGaps);s+="\n";
    s+=tr("Students=%1").arg(this->students); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetMaxGapsPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    //OLD COMMENT
    //returns a number equal to the number of gaps of the subgroups (in hours)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nGaps;
    int tmp;

    int tIllegalGaps=0;

    for(int sg=0; sg<this->iSubgroupsList.count(); sg++){
        int i=this->iSubgroupsList.at(sg);
        for(int j=0; j<r.nDaysPerWeek; j++){
            nGaps=0;

            int k;
            tmp=0;
            for(k=0; k<r.nHoursPerDay; k++)
                if(c.subgroupsMatrix[i][j][k]>0){
                    assert(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]);
                    break;
                }
            for(; k<r.nHoursPerDay; k++) if(!c.instance->breakDayHour[j][k] && !c.instance->subgroupNotAvailableDayHour[i][j][k]){
                if(c.subgroupsMatrix[i][j][k]>0){
                    nGaps+=tmp;
                    tmp=0;
                }
                else
                    tmp++;
            }

            int illegalGaps=nGaps-this->maxGaps;
            if(illegalGaps<0)
                illegalGaps=0;

            if(illegalGaps>0 && collectConflictsData){
                int parcialViolation = illegalGaps;

                QString s=tr("Time constraint students set max gaps per day broken for subgroup: %1, extra gaps=%2, on day %3, cost increase=%4")
                        .arg(r.directSubgroupsList[i]->name)
                        .arg(illegalGaps)
                        .arg(r.daysOfTheWeek[j])
                        .arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }

            tIllegalGaps+=illegalGaps;
        }
    }

    return tIllegalGaps;
}

bool ConstraintStudentsSetMaxGapsPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxGapsPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxGapsPerDay::hasWrongDayOrHour()
{
    if(maxGaps>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxGapsPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxGapsPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxGaps>r.nHoursPerDay)
        maxGaps=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::ConstraintActivitiesOccupyMaxTimeSlotsFromSelection(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITIES_OCCUPY_MAX_TIME_SLOTS_FROM_SELECTION;
}

ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::ConstraintActivitiesOccupyMaxTimeSlotsFromSelection(Instance& r, Enums::ConstraintGroup constraintGroup, double wp,
                                                                                                         QList<int> a_L, QList<int> d_L, QList<int> h_L, int max_slots)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==h_L.count());

    this->activitiesIds=a_L;
    this->selectedDays=d_L;
    this->selectedHours=h_L;
    this->maxOccupiedTimeSlots=max_slots;

    this->type=CONSTRAINT_ACTIVITIES_OCCUPY_MAX_TIME_SLOTS_FROM_SELECTION;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::computeInternalStructure(QWidget* parent)
{
    //this cares about inactive activities, also, so do not assert this->_actIndices.count()==this->actIds.count()
    _activitiesIndices.clear();
    foreach(int id, activitiesIds){
        int i=r.getActivityIndex(id);
        if(i>=0)
            _activitiesIndices.append(i);
    }

    /*this->_activitiesIndices.clear();

        QSet<int> req=this->activitiesIds.toSet();
        assert(req.count()==this->activitiesIds.count());

        //this cares about inactive activities, also, so do not assert this->_actIndices.count()==this->actIds.count()
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++)
                if(req.contains(r.internalActivitiesList[i].id))
                        this->_activitiesIndices.append(i);*/

    //////////////////////
    assert(this->selectedDays.count()==this->selectedHours.count());

    for(int k=0; k<this->selectedDays.count(); k++){
        if(this->selectedDays.at(k) >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities occupy max time slots from selection is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedHours.at(k) == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities occupy max time slots from selection is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedHours.at(k) > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities occupy max time slots from selection is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedDays.at(k)<0 || this->selectedHours.at(k)<0){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities occupy max time slots from selection is wrong because hour or day is not specified for a slot (-1). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->_activitiesIndices.count()>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::hasInactiveActivities()
{
    //returns true if all activities are inactive

    foreach(int aid, this->activitiesIds)
        if(!r.inactiveActivitiesIdsSet.contains(aid))
            return false;

    return true;
}

QString ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::getXmlDescription()
{
    assert(this->selectedDays.count()==this->selectedHours.count());

    QString s="<ConstraintActivitiesOccupyMaxTimeSlotsFromSelection>\n";

    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";

    s+="	<Number_of_Activities>"+utils::strings::number(this->activitiesIds.count())+"</Number_of_Activities>\n";
    foreach(int aid, this->activitiesIds)
        s+="	<Activity_Id>"+utils::strings::number(aid)+"</Activity_Id>\n";

    s+="	<Number_of_Selected_Time_Slots>"+utils::strings::number(this->selectedDays.count())+"</Number_of_Selected_Time_Slots>\n";
    for(int i=0; i<this->selectedDays.count(); i++){
        s+="	<Selected_Time_Slot>\n";
        s+="		<Selected_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->selectedDays.at(i)])+"</Selected_Day>\n";
        s+="		<Selected_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->selectedHours.at(i)])+"</Selected_Hour>\n";
        s+="	</Selected_Time_Slot>\n";
    }
    s+="	<Max_Number_of_Occupied_Time_Slots>"+utils::strings::number(this->maxOccupiedTimeSlots)+"</Max_Number_of_Occupied_Time_Slots>\n";

    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesOccupyMaxTimeSlotsFromSelection>\n";
    return s;
}

QString ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    assert(this->selectedDays.count()==this->selectedHours.count());

    QString actids=QString("");
    foreach(int aid, this->activitiesIds)
        actids+=utils::strings::number(aid)+QString(", ");
    actids.chop(2);

    QString timeslots=QString("");
    for(int i=0; i<this->selectedDays.count(); i++)
        timeslots+=r.daysOfTheWeek[selectedDays.at(i)]+QString(" ")+r.hoursOfTheDay[selectedHours.at(i)]+QString(", ");
    timeslots.chop(2);

    QString s=tr("Activities occupy max time slots from selection, WP:%1%, NA:%2, A: %3, STS: %4, MTS:%5", "Constraint description. WP means weight percentage, "
                                                                                                           "NA means the number of activities, A means activities list, STS means selected time slots, MTS means max time slots")
            .arg(utils::strings::number(this->weightPercentage))
            .arg(utils::strings::number(this->activitiesIds.count()))
            .arg(actids)
            .arg(timeslots)
            .arg(utils::strings::number(this->maxOccupiedTimeSlots));

    return begin+s+ ", " + getSuperDescription() +end;
}

QString ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::getDetailedDescription()
{
    assert(this->selectedDays.count()==this->selectedHours.count());

    QString actids=QString("");
    foreach(int aid, this->activitiesIds)
        actids+=utils::strings::number(aid)+QString(", ");
    actids.chop(2);

    QString timeslots=QString("");
    for(int i=0; i<this->selectedDays.count(); i++)
        timeslots+=r.daysOfTheWeek[selectedDays.at(i)]+QString(" ")+r.hoursOfTheDay[selectedHours.at(i)]+QString(", ");
    timeslots.chop(2);

    QString s=tr("Time constraint"); s+="\n";
    s+=tr("Activities occupy max time slots from selection"); s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage)); s+="\n";
    s+=tr("Number of activities=%1").arg(utils::strings::number(this->activitiesIds.count())); s+="\n";
    foreach(int id, this->activitiesIds){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(id)
                .arg(r.getActivityDetailedDescription(id));
        s+="\n";
    }
    s+=tr("Selected time slots: %1").arg(timeslots); s+="\n";
    s+=tr("Maximum number of occupied slots from selection=%1").arg(utils::strings::number(this->maxOccupiedTimeSlots)); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    ///////////////////
    Matrix2D<bool> used;
    used.resize(r.nDaysPerWeek, r.nHoursPerDay);
    for(int d=0; d<r.nDaysPerWeek; d++)
        for(int h=0; h<r.nHoursPerDay; h++)
            used[d][h]=false;

    foreach(int ai, this->_activitiesIndices){
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            Activity* act=r.activeActivitiesList[ai];
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);
            for(int dur=0; dur<act->duration; dur++){
                assert(h+dur<r.nHoursPerDay);
                used[d][h+dur]=true;
            }
        }
    }

    int cnt=0;
    assert(this->selectedDays.count()==this->selectedHours.count());
    for(int t=0; t<this->selectedDays.count(); t++){
        int d=this->selectedDays.at(t);
        int h=this->selectedHours.at(t);

        if(used[d][h])
            cnt++;
    }

    nbroken=0;

    if(cnt > this->maxOccupiedTimeSlots){
        int parcialViolation = 1;

        nbroken=1;

        if(collectConflictsData){
            QString s=tr("Time constraint %1 broken - this should not happen, as this kind of constraint should "
                         "have only 100.0% weight. Please report error!").arg(this->getDescription());

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

void ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::removeUseless()
{
    /*QSet<int> validActs;

        foreach(Activity* act, r.activitiesList)
                validActs.insert(act->id);*/

    QList<int> newActs;

    foreach(int aid, activitiesIds){
        Activity* act=r.activitiesPointerHash.value(aid, nullptr);
        if(act!=nullptr)
            //if(validActs.contains(aid))
            newActs.append(aid);
    }

    activitiesIds=newActs;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    return this->activitiesIds.contains(a->id);
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::hasWrongDayOrHour()
{
    assert(selectedDays.count()==selectedHours.count());

    for(int i=0; i<selectedDays.count(); i++)
        if(selectedDays.at(i)<0 || selectedDays.at(i)>=r.nDaysPerWeek
                || selectedHours.at(i)<0 || selectedHours.at(i)>=r.nHoursPerDay)
            return true;

    if(maxOccupiedTimeSlots>r.nDaysPerWeek*r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivitiesOccupyMaxTimeSlotsFromSelection::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(selectedDays.count()==selectedHours.count());

    QList<int> newDays;
    QList<int> newHours;

    for(int i=0; i<selectedDays.count(); i++)
        if(selectedDays.at(i)>=0 && selectedDays.at(i)<r.nDaysPerWeek
                && selectedHours.at(i)>=0 && selectedHours.at(i)<r.nHoursPerDay){
            newDays.append(selectedDays.at(i));
            newHours.append(selectedHours.at(i));
        }

    selectedDays=newDays;
    selectedHours=newHours;

    if(maxOccupiedTimeSlots>r.nDaysPerWeek*r.nHoursPerDay)
        maxOccupiedTimeSlots=r.nDaysPerWeek*r.nHoursPerDay;

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_ACTIVITIES_MAX_SIMULTANEOUS_IN_SELECTED_TIME_SLOTS;
}

ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots(Instance& r, Enums::ConstraintGroup constraintGroup, double wp,
                                                                                                               QList<int> a_L, QList<int> d_L, QList<int> h_L, int max_simultaneous)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(d_L.count()==h_L.count());

    this->activitiesIds=a_L;
    this->selectedDays=d_L;
    this->selectedHours=h_L;
    this->maxSimultaneous=max_simultaneous;

    this->type=CONSTRAINT_ACTIVITIES_MAX_SIMULTANEOUS_IN_SELECTED_TIME_SLOTS;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::computeInternalStructure(QWidget* parent)
{
    //this cares about inactive activities, also, so do not assert this->_actIndices.count()==this->actIds.count()
    _activitiesIndices.clear();
    foreach(int id, activitiesIds){
        int i=r.getActivityIndex(id);
        if(i>=0)
            _activitiesIndices.append(i);
    }

    /*this->_activitiesIndices.clear();

        QSet<int> req=this->activitiesIds.toSet();
        assert(req.count()==this->activitiesIds.count());

        //this cares about inactive activities, also, so do not assert this->_actIndices.count()==this->actIds.count()
        int i;
    for(i=0; i<r.activeActivitiesList.size(); i++)
                if(req.contains(r.internalActivitiesList[i].id))
                        this->_activitiesIndices.append(i);*/

    //////////////////////
    assert(this->selectedDays.count()==this->selectedHours.count());

    for(int k=0; k<this->selectedDays.count(); k++){
        if(this->selectedDays.at(k) >= r.nDaysPerWeek){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities max simultaneous in selected time slots is wrong because it refers to removed day. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedHours.at(k) == r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities max simultaneous in selected time slots is wrong because a preferred hour is too late (after the last acceptable slot). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedHours.at(k) > r.nHoursPerDay){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities max simultaneous in selected time slots is wrong because it refers to removed hour. Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
        if(this->selectedDays.at(k)<0 || this->selectedHours.at(k)<0){
            MessagesManager::information(parent, tr("m-FET information"),
                                                             tr("Constraint activities max simultaneous in selected time slots is wrong because hour or day is not specified for a slot (-1). Please correct"
                                                                " and try again. Correcting means editing the constraint and updating information. Constraint is:\n%1").arg(this->getDetailedDescription()));

            return false;
        }
    }
    ///////////////////////

    if(this->_activitiesIndices.count()>0)
        return true;
    else{
        MessagesManager::warning(parent, tr("m-FET error in data"),
                                                     tr("Following constraint is wrong (refers to no activities). Please correct it:\n%1").arg(this->getDetailedDescription()));
        return false;
    }
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::hasInactiveActivities()
{
    //returns true if all activities are inactive

    foreach(int aid, this->activitiesIds)
        if(!r.inactiveActivitiesIdsSet.contains(aid))
            return false;

    return true;
}

QString ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::getXmlDescription()
{
    assert(this->selectedDays.count()==this->selectedHours.count());

    QString s="<ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots>\n";

    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";

    s+="	<Number_of_Activities>"+utils::strings::number(this->activitiesIds.count())+"</Number_of_Activities>\n";
    foreach(int aid, this->activitiesIds)
        s+="	<Activity_Id>"+utils::strings::number(aid)+"</Activity_Id>\n";

    s+="	<Number_of_Selected_Time_Slots>"+utils::strings::number(this->selectedDays.count())+"</Number_of_Selected_Time_Slots>\n";
    for(int i=0; i<this->selectedDays.count(); i++){
        s+="	<Selected_Time_Slot>\n";
        s+="		<Selected_Day>"+utils::strings::parseStrForXml(r.daysOfTheWeek[this->selectedDays.at(i)])+"</Selected_Day>\n";
        s+="		<Selected_Hour>"+utils::strings::parseStrForXml(r.hoursOfTheDay[this->selectedHours.at(i)])+"</Selected_Hour>\n";
        s+="	</Selected_Time_Slot>\n";
    }
    s+="	<Max_Number_of_Simultaneous_Activities>"+utils::strings::number(this->maxSimultaneous)+"</Max_Number_of_Simultaneous_Activities>\n";

    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots>\n";
    return s;
}

QString ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::getDescription()
{
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    assert(this->selectedDays.count()==this->selectedHours.count());

    QString actids=QString("");
    foreach(int aid, this->activitiesIds)
        actids+=utils::strings::number(aid)+QString(", ");
    actids.chop(2);

    QString timeslots=QString("");
    for(int i=0; i<this->selectedDays.count(); i++)
        timeslots+=r.daysOfTheWeek[selectedDays.at(i)]+QString(" ")+r.hoursOfTheDay[selectedHours.at(i)]+QString(", ");
    timeslots.chop(2);

    QString s=tr("Activities max simultaneous in selected time slots, WP:%1%, NA:%2, A: %3, STS: %4, MS:%5", "Constraint description. WP means weight percentage, "
                                                                                                             "NA means the number of activities, A means activities list, STS means selected time slots, MS means max simultaneous (number of activities in each selected time slot)")
            .arg(utils::strings::number(this->weightPercentage))
            .arg(utils::strings::number(this->activitiesIds.count()))
            .arg(actids)
            .arg(timeslots)
            .arg(utils::strings::number(this->maxSimultaneous));

    return begin+s+ ", " + getSuperDescription() +end;
}

QString ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::getDetailedDescription()
{
    assert(this->selectedDays.count()==this->selectedHours.count());

    QString actids=QString("");
    foreach(int aid, this->activitiesIds)
        actids+=utils::strings::number(aid)+QString(", ");
    actids.chop(2);

    QString timeslots=QString("");
    for(int i=0; i<this->selectedDays.count(); i++)
        timeslots+=r.daysOfTheWeek[selectedDays.at(i)]+QString(" ")+r.hoursOfTheDay[selectedHours.at(i)]+QString(", ");
    timeslots.chop(2);

    QString s=tr("Time constraint"); s+="\n";
    s+=tr("Activities max simultaneous in selected time slots"); s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage)); s+="\n";
    s+=tr("Number of activities=%1").arg(utils::strings::number(this->activitiesIds.count())); s+="\n";
    foreach(int id, this->activitiesIds){
        s+=tr("Activity with id=%1 (%2)", "%1 is the id, %2 is the detailed description of the activity")
                .arg(id)
                .arg(r.getActivityDetailedDescription(id));
        s+="\n";
    }
    s+=tr("Selected time slots: %1").arg(timeslots); s+="\n";
    s+=tr("Maximum number of simultaneous activities in each selected time slot=%1").arg(utils::strings::number(this->maxSimultaneous)); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    assert(r.internalStructureComputed);

    ///////////////////

    Matrix2D<int> count;
    count.resize(r.nDaysPerWeek, r.nHoursPerDay);
    for(int d=0; d<r.nDaysPerWeek; d++)
        for(int h=0; h<r.nHoursPerDay; h++)
            count[d][h]=0;

    foreach(int ai, this->_activitiesIndices){
        if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            Activity* act=r.activeActivitiesList[ai];
            int h=c.getHourForActivityIndex(ai);
            int d=c.getDayForActivityIndex(ai);
            for(int dur=0; dur<act->duration; dur++){
                assert(h+dur<r.nHoursPerDay);
                count[d][h+dur]++;
            }
        }
    }

    nbroken=0;

    assert(this->selectedDays.count()==this->selectedHours.count());
    for(int t=0; t<this->selectedDays.count(); t++){
        int d=this->selectedDays.at(t);
        int h=this->selectedHours.at(t);

        if(count[d][h] > this->maxSimultaneous)
            nbroken++;
    }

    if(nbroken>0){
        if(collectConflictsData){
            int parcialViolation = 1;

            QString s=tr("Time constraint %1 broken - this should not happen, as this kind of constraint should "
                         "have only 100.0% weight. Please report error!").arg(this->getDescription());

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

void ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::removeUseless()
{
    /*QSet<int> validActs;

        foreach(Activity* act, r.activitiesList)
                validActs.insert(act->id);*/

    QList<int> newActs;

    foreach(int aid, activitiesIds){
        Activity* act=r.activitiesPointerHash.value(aid, nullptr);
        if(act!=nullptr)
            //if(validActs.contains(aid))
            newActs.append(aid);
    }

    activitiesIds=newActs;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);

    return this->activitiesIds.contains(a->id);
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::hasWrongDayOrHour()
{
    assert(selectedDays.count()==selectedHours.count());

    for(int i=0; i<selectedDays.count(); i++)
        if(selectedDays.at(i)<0 || selectedDays.at(i)>=r.nDaysPerWeek
                || selectedHours.at(i)<0 || selectedHours.at(i)>=r.nHoursPerDay)
            return true;

    //Do not care about maxSimultaneous, which can be as high as MAX_ACTIVITIES

    return false;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    assert(selectedDays.count()==selectedHours.count());

    QList<int> newDays;
    QList<int> newHours;

    for(int i=0; i<selectedDays.count(); i++)
        if(selectedDays.at(i)>=0 && selectedDays.at(i)<r.nDaysPerWeek
                && selectedHours.at(i)>=0 && selectedHours.at(i)<r.nHoursPerDay){
            newDays.append(selectedDays.at(i));
            newHours.append(selectedHours.at(i));
        }

    selectedDays=newDays;
    selectedHours=newHours;

    //Do not modify maxSimultaneous, which can be as high as MAX_ACTIVITIES

    r.internalStructureComputed=false;
    setRulesModifiedAndOtherThings(&r);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxDaysPerWeek::ConstraintStudentsSetMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK;
}

ConstraintStudentsSetMaxDaysPerWeek::ConstraintStudentsSetMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd, QString sn)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->students = sn;
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_STUDENTS_SET_MAX_DAYS_PER_WEEK;
}

bool ConstraintStudentsSetMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max days per week is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

bool ConstraintStudentsSetMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintStudentsSetMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Students set max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Abbreviation for students (sets)").arg(this->students);s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";

    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsSetMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int sbg, this->iSubgroupsList){
        bool ocDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<r.nDaysPerWeek; d++){
            ocDay[d]=false;
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    ocDay[d]=true;
                }
            }
        }
        int nOcDays=0;
        for(int d=0; d<r.nDaysPerWeek; d++)
            if(ocDay[d])
                nOcDays++;
        if(nOcDays > this->maxDaysPerWeek){
            int parcialViolation = nOcDays-this->maxDaysPerWeek;

            nbroken+=parcialViolation;

            if((nOcDays-this->maxDaysPerWeek)>0){
                QString s= tr("Time constraint students set max days per week broken for subgroup: %1, allowed %2 days, required %3 days.")
                        .arg(r.directSubgroupsList[sbg]->name)
                        .arg(this->maxDaysPerWeek)
                        .arg(nOcDays);
                s+=" ";
                s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);
    return false;
}

bool ConstraintStudentsSetMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxDaysPerWeek::ConstraintStudentsMaxDaysPerWeek(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_STUDENTS_MAX_DAYS_PER_WEEK;
}

ConstraintStudentsMaxDaysPerWeek::ConstraintStudentsMaxDaysPerWeek(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxnd)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxDaysPerWeek=maxnd;
    this->type=CONSTRAINT_STUDENTS_MAX_DAYS_PER_WEEK;
}

bool ConstraintStudentsMaxDaysPerWeek::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintStudentsMaxDaysPerWeek::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxDaysPerWeek::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxDaysPerWeek>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Days_Per_Week>"+utils::strings::number(this->maxDaysPerWeek)+"</Max_Days_Per_Week>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxDaysPerWeek>\n";
    return s;
}

QString ConstraintStudentsMaxDaysPerWeek::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s=tr("Students max days per week");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Abbreviation for weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MD:%1", "Abbreviation for max days").arg(this->maxDaysPerWeek);

    return begin+s+end;
}

QString ConstraintStudentsMaxDaysPerWeek::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the maximum number of days per week");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum days per week=%1").arg(this->maxDaysPerWeek);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsMaxDaysPerWeek::violationsFactor(Solution& c, bool collectConflictsData)
{
    Q_UNUSED(collectConflictsData)

    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    for(int sbg=0; sbg<r.directSubgroupsList.size(); sbg++){
        bool ocDay[MAX_DAYS_PER_WEEK];
        for(int d=0; d<r.nDaysPerWeek; d++){
            ocDay[d]=false;
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    ocDay[d]=true;
                }
            }
        }
        int nOcDays=0;
        for(int d=0; d<r.nDaysPerWeek; d++)
            if(ocDay[d])
                nOcDays++;
        if(nOcDays > this->maxDaysPerWeek){
            int parcialViolation = nOcDays-this->maxDaysPerWeek;

            nbroken+=parcialViolation;

            if((nOcDays-this->maxDaysPerWeek)>0){
                QString s= tr("Time constraint students max days per week broken for subgroup: %1, allowed %2 days, required %3 days.")
                        .arg(r.directSubgroupsList[sbg]->name)
                        .arg(this->maxDaysPerWeek)
                        .arg(nOcDays);
                s+=" ";
                s += tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group)));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsMaxDaysPerWeek::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxDaysPerWeek::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);
    return false;
}

bool ConstraintStudentsMaxDaysPerWeek::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxDaysPerWeek::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxDaysPerWeek::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);
    return true;
}

bool ConstraintStudentsMaxDaysPerWeek::hasWrongDayOrHour()
{
    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        return true;

    return false;
}

bool ConstraintStudentsMaxDaysPerWeek::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxDaysPerWeek::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(this->maxDaysPerWeek>r.nDaysPerWeek)
        this->maxDaysPerWeek=r.nDaysPerWeek;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxSpanPerDay::ConstraintTeacherMaxSpanPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY;
    this->maxSpanPerDay = -1;
}

ConstraintTeacherMaxSpanPerDay::ConstraintTeacherMaxSpanPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan, const QString& teacher)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxspan>0);
    this->maxSpanPerDay=maxspan;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MAX_SPAN_PER_DAY;
}

bool ConstraintTeacherMaxSpanPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMaxSpanPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxSpanPerDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxSpanPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Max_Span>"+utils::strings::number(this->maxSpanPerDay)+"</Max_Span>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxSpanPerDay>\n";
    return s;
}

QString ConstraintTeacherMaxSpanPerDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max span per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MS:%1", "Maximum span (in hours, per day)").arg(this->maxSpanPerDay);

    return begin+s+end;
}

QString ConstraintTeacherMaxSpanPerDay::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of span (in hours) per day");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum span per day=%1").arg(this->maxSpanPerDay);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxSpanPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int d=0; d<r.nDaysPerWeek; d++){
        int begin=-1;
        int end=-1;
        for(int h=0; h<r.nHoursPerDay; h++)
            if(c.teachersMatrix[this->teacher_ID][d][h]>0){
                begin=h;
                break;
            }
        for(int h=r.nHoursPerDay-1; h>=0; h--)
            if(c.teachersMatrix[this->teacher_ID][d][h]>0){
                end=h;
                break;
            }
        if(end>=0 && begin>=0 && end>=begin){
            int span=end-begin+1;
            if(span>this->maxSpanPerDay)
                nbroken++;
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintTeacherMaxSpanPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxSpanPerDay::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxSpanPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxSpanPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxSpanPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxSpanPerDay::hasWrongDayOrHour()
{
    if(maxSpanPerDay>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxSpanPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxSpanPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxSpanPerDay>r.nHoursPerDay)
        maxSpanPerDay=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxSpanPerDay::ConstraintTeachersMaxSpanPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MAX_SPAN_PER_DAY;
    this->maxSpanPerDay = -1;
}

ConstraintTeachersMaxSpanPerDay::ConstraintTeachersMaxSpanPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(maxspan>0);
    this->maxSpanPerDay=maxspan;

    this->type=CONSTRAINT_TEACHERS_MAX_SPAN_PER_DAY;
}

bool ConstraintTeachersMaxSpanPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxSpanPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxSpanPerDay::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxSpanPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Span>"+utils::strings::number(this->maxSpanPerDay)+"</Max_Span>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxSpanPerDay>\n";
    return s;
}

QString ConstraintTeachersMaxSpanPerDay::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max span per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MS:%1", "Maximum span (in hours, per day)").arg(this->maxSpanPerDay);

    return begin+s+end;
}

QString ConstraintTeachersMaxSpanPerDay::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of span (in hours) per day");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum span per day=%1").arg(this->maxSpanPerDay);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxSpanPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int tch=0; tch<r.teachersList.size(); tch++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int begin=-1;
            int end=-1;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(c.teachersMatrix[tch][d][h]>0){
                    begin=h;
                    break;
                }
            for(int h=r.nHoursPerDay-1; h>=0; h--)
                if(c.teachersMatrix[tch][d][h]>0){
                    end=h;
                    break;
                }
            if(end>=0 && begin>=0 && end>=begin){
                int span=end-begin+1;
                if(span>this->maxSpanPerDay)
                    nbroken++;
            }
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintTeachersMaxSpanPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxSpanPerDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxSpanPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxSpanPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxSpanPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxSpanPerDay::hasWrongDayOrHour()
{
    if(maxSpanPerDay>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxSpanPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxSpanPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxSpanPerDay>r.nHoursPerDay)
        maxSpanPerDay=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMaxSpanPerDay::ConstraintStudentsSetMaxSpanPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY;
    this->maxSpanPerDay = -1;
}

ConstraintStudentsSetMaxSpanPerDay::ConstraintStudentsSetMaxSpanPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan, QString sn)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxSpanPerDay = maxspan;
    this->students = sn;
    this->type = CONSTRAINT_STUDENTS_SET_MAX_SPAN_PER_DAY;
}

bool ConstraintStudentsSetMaxSpanPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMaxSpanPerDay::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMaxSpanPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Span>"+utils::strings::number(this->maxSpanPerDay)+"</Max_Span>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMaxSpanPerDay>\n";
    return s;
}

QString ConstraintStudentsSetMaxSpanPerDay::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set max span per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students (set)").arg(this->students); s+=", ";
    s+=tr("MS:%1", "Max span (in hours, per day)").arg(this->maxSpanPerDay);

    return begin+s+end;
}

QString ConstraintStudentsSetMaxSpanPerDay::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the maximum number of span (in hours) per day");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Maximum span per day=%1").arg(this->maxSpanPerDay);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetMaxSpanPerDay::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set max span per day is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

double ConstraintStudentsSetMaxSpanPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    foreach(int sbg, this->iSubgroupsList){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int begin=-1;
            int end=-1;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    begin=h;
                    break;
                }
            for(int h=r.nHoursPerDay-1; h>=0; h--)
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    end=h;
                    break;
                }
            if(end>=0 && begin>=0 && end>=begin){
                int span=end-begin+1;
                if(span>this->maxSpanPerDay)
                    nbroken++;
            }
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintStudentsSetMaxSpanPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMaxSpanPerDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMaxSpanPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxSpanPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMaxSpanPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMaxSpanPerDay::hasWrongDayOrHour()
{
    if(maxSpanPerDay>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMaxSpanPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMaxSpanPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxSpanPerDay>r.nHoursPerDay)
        maxSpanPerDay=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMaxSpanPerDay::ConstraintStudentsMaxSpanPerDay(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MAX_SPAN_PER_DAY;
    this->maxSpanPerDay = -1;
}

ConstraintStudentsMaxSpanPerDay::ConstraintStudentsMaxSpanPerDay(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int maxspan)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxSpanPerDay = maxspan;
    this->type = CONSTRAINT_STUDENTS_MAX_SPAN_PER_DAY;
}

bool ConstraintStudentsMaxSpanPerDay::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMaxSpanPerDay::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMaxSpanPerDay>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Max_Span>"+utils::strings::number(this->maxSpanPerDay)+"</Max_Span>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMaxSpanPerDay>\n";
    return s;
}

QString ConstraintStudentsMaxSpanPerDay::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students max span per day");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("MS:%1", "Max span (in hours, per day)").arg(this->maxSpanPerDay);

    return begin+s+end;
}

QString ConstraintStudentsMaxSpanPerDay::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the maximum number of span (in hours) per day");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Maximum span per day=%1").arg(this->maxSpanPerDay);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsMaxSpanPerDay::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

double ConstraintStudentsMaxSpanPerDay::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int sbg=0; sbg<r.directSubgroupsList.size(); sbg++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int begin=-1;
            int end=-1;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    begin=h;
                    break;
                }
            for(int h=r.nHoursPerDay-1; h>=0; h--)
                if(c.subgroupsMatrix[sbg][d][h]>0){
                    end=h;
                    break;
                }
            if(end>=0 && begin>=0 && end>=begin){
                int span=end-begin+1;
                if(span>this->maxSpanPerDay)
                    nbroken++;
            }
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintStudentsMaxSpanPerDay::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMaxSpanPerDay::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMaxSpanPerDay::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxSpanPerDay::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMaxSpanPerDay::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMaxSpanPerDay::hasWrongDayOrHour()
{
    if(maxSpanPerDay>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMaxSpanPerDay::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMaxSpanPerDay::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxSpanPerDay>r.nHoursPerDay)
        maxSpanPerDay=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMinRestingHours::ConstraintTeacherMinRestingHours(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MIN_RESTING_HOURS;
    this->minRestingHours=-1;
    this->circular=true;
}

ConstraintTeacherMinRestingHours::ConstraintTeacherMinRestingHours(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ, const QString& teacher)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(minrestinghours>0);
    this->minRestingHours=minrestinghours;
    this->circular=circ;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MIN_RESTING_HOURS;
}

bool ConstraintTeacherMinRestingHours::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMinRestingHours::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMinRestingHours::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMinRestingHours>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Minimum_Resting_Hours>"+utils::strings::number(this->minRestingHours)+"</Minimum_Resting_Hours>\n";
    s+="	<Circular>"+utils::strings::trueFalse(circular)+"</Circular>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMinRestingHours>\n";
    return s;
}

QString ConstraintTeacherMinRestingHours::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher min resting hours");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("mRH:%1", "Minimum resting hours").arg(this->minRestingHours);s+=", ";
    s+=tr("C:%1", "Circular").arg(utils::strings::yesNoTranslated(this->circular));

    return begin+s+end;
}

QString ConstraintTeacherMinRestingHours::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the minimum resting hours (between days)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Minimum resting hours=%1").arg(this->minRestingHours);s+="\n";
    s+=tr("Circular=%1").arg(utils::strings::yesNoTranslated(circular));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMinRestingHours::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int d=0; d<=r.nDaysPerWeek-2+(circular?1:0); d++){
        int cnt=0;
        for(int h=r.nHoursPerDay-1; h>=0; h--){
            if(c.teachersMatrix[this->teacher_ID][d][h]>0)
                break;
            else
                cnt++;
        }
        for(int h=0; h<r.nHoursPerDay; h++){
            if(c.teachersMatrix[this->teacher_ID][(d+1<=r.nDaysPerWeek-1?d+1:0)][h]>0)
                break;
            else
                cnt++;
        }
        if(cnt < this->minRestingHours)
            nbroken++;
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintTeacherMinRestingHours::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMinRestingHours::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMinRestingHours::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinRestingHours::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinRestingHours::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMinRestingHours::hasWrongDayOrHour()
{
    if(minRestingHours>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMinRestingHours::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMinRestingHours::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minRestingHours>r.nHoursPerDay)
        minRestingHours=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMinRestingHours::ConstraintTeachersMinRestingHours(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MIN_RESTING_HOURS;
    this->minRestingHours=-1;
    this->circular=true;
}

ConstraintTeachersMinRestingHours::ConstraintTeachersMinRestingHours(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ)
    : TimeConstraint(r, constraintGroup, wp)
{
    assert(minrestinghours>0);
    this->minRestingHours=minrestinghours;
    this->circular=circ;

    this->type=CONSTRAINT_TEACHERS_MIN_RESTING_HOURS;
}

bool ConstraintTeachersMinRestingHours::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMinRestingHours::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMinRestingHours::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMinRestingHours>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Resting_Hours>"+utils::strings::number(this->minRestingHours)+"</Minimum_Resting_Hours>\n";
    s+="	<Circular>"+utils::strings::trueFalse(circular)+"</Circular>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMinRestingHours>\n";
    return s;
}

QString ConstraintTeachersMinRestingHours::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers min resting hours");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("mRH:%1", "Minimum resting hours").arg(this->minRestingHours);s+=", ";
    s+=tr("C:%1", "Circular").arg(utils::strings::yesNoTranslated(this->circular));

    return begin+s+end;
}

QString ConstraintTeachersMinRestingHours::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the minimum resting hours (between days)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Minimum resting hours=%1").arg(this->minRestingHours);s+="\n";
    s+=tr("Circular=%1").arg(utils::strings::yesNoTranslated(circular));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMinRestingHours::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int tch=0; tch<r.teachersList.size(); tch++){
        for(int d=0; d<=r.nDaysPerWeek-2+(circular?1:0); d++){
            int cnt=0;
            for(int h=r.nHoursPerDay-1; h>=0; h--){
                if(c.teachersMatrix[tch][d][h]>0)
                    break;
                else
                    cnt++;
            }
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.teachersMatrix[tch][(d+1<=r.nDaysPerWeek-1?d+1:0)][h]>0)
                    break;
                else
                    cnt++;
            }
            if(cnt < this->minRestingHours)
                nbroken++;
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintTeachersMinRestingHours::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMinRestingHours::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMinRestingHours::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinRestingHours::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinRestingHours::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMinRestingHours::hasWrongDayOrHour()
{
    if(minRestingHours>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMinRestingHours::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMinRestingHours::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minRestingHours>r.nHoursPerDay)
        minRestingHours=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetMinRestingHours::ConstraintStudentsSetMinRestingHours(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS;
    this->minRestingHours = -1;
    this->circular=true;
}

ConstraintStudentsSetMinRestingHours::ConstraintStudentsSetMinRestingHours(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ, QString sn)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->minRestingHours = minrestinghours;
    this->circular=circ;
    this->students = sn;
    this->type = CONSTRAINT_STUDENTS_SET_MIN_RESTING_HOURS;
}

bool ConstraintStudentsSetMinRestingHours::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetMinRestingHours::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetMinRestingHours>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Resting_Hours>"+utils::strings::number(this->minRestingHours)+"</Minimum_Resting_Hours>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Circular>"+utils::strings::trueFalse(circular)+"</Circular>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetMinRestingHours>\n";
    return s;
}

QString ConstraintStudentsSetMinRestingHours::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set min resting hours");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("St:%1", "Students (set)").arg(this->students); s+=", ";
    s+=tr("mRH:%1", "Minimum resting hours").arg(this->minRestingHours);s+=", ";
    s+=tr("C:%1", "Circular").arg(utils::strings::yesNoTranslated(this->circular));

    return begin+s+end;
}

QString ConstraintStudentsSetMinRestingHours::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set must respect the minimum resting hours (between days)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Minimum resting hours=%1").arg(this->minRestingHours);s+="\n";
    s+=tr("Circular=%1").arg(utils::strings::yesNoTranslated(circular));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetMinRestingHours::computeInternalStructure(QWidget* parent)
{
    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
                                                     tr("Constraint students set min resting hours is wrong because it refers to inexistent students set."
                                                        " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    return true;
}

double ConstraintStudentsSetMinRestingHours::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    foreach(int sbg, this->iSubgroupsList){
        for(int d=0; d<=r.nDaysPerWeek-2+(circular?1:0); d++){
            int cnt=0;
            for(int h=r.nHoursPerDay-1; h>=0; h--){
                if(c.subgroupsMatrix[sbg][d][h]>0)
                    break;
                else
                    cnt++;
            }
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[sbg][(d+1<=r.nDaysPerWeek-1?d+1:0)][h]>0)
                    break;
                else
                    cnt++;
            }
            if(cnt < this->minRestingHours)
                nbroken++;
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintStudentsSetMinRestingHours::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetMinRestingHours::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetMinRestingHours::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMinRestingHours::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetMinRestingHours::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetMinRestingHours::hasWrongDayOrHour()
{
    if(minRestingHours>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetMinRestingHours::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetMinRestingHours::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minRestingHours>r.nHoursPerDay)
        minRestingHours=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsMinRestingHours::ConstraintStudentsMinRestingHours(Instance& r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_MIN_RESTING_HOURS;
    this->minRestingHours = -1;
    this->circular=true;
}

ConstraintStudentsMinRestingHours::ConstraintStudentsMinRestingHours(Instance& r, Enums::ConstraintGroup constraintGroup, double wp, int minrestinghours, bool circ)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->minRestingHours = minrestinghours;
    this->circular=circ;
    this->type = CONSTRAINT_STUDENTS_MIN_RESTING_HOURS;
}

bool ConstraintStudentsMinRestingHours::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsMinRestingHours::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsMinRestingHours>\n";
    s+=getSuperXmlDescription();
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Minimum_Resting_Hours>"+utils::strings::number(this->minRestingHours)+"</Minimum_Resting_Hours>\n";
    s+="	<Circular>"+utils::strings::trueFalse(circular)+"</Circular>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsMinRestingHours>\n";
    return s;
}

QString ConstraintStudentsMinRestingHours::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students min resting hours");s+=", " + getSuperDescription() + ", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));s+=", ";
    s+=tr("mRH:%1", "Minimum resting hours").arg(this->minRestingHours);s+=", ";
    s+=tr("C:%1", "Circular").arg(utils::strings::yesNoTranslated(this->circular));

    return begin+s+end;
}

QString ConstraintStudentsMinRestingHours::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students must respect the minimum resting hours (between days)");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Minimum resting hours=%1").arg(this->minRestingHours);s+="\n";
    s+=tr("Circular=%1").arg(utils::strings::yesNoTranslated(circular));s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsMinRestingHours::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

double ConstraintStudentsMinRestingHours::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    //TODO: log
    Q_UNUSED(collectConflictsData);

    int nbroken=0;

    for(int sbg=0; sbg<r.directSubgroupsList.size(); sbg++){
        for(int d=0; d<=r.nDaysPerWeek-2+(circular?1:0); d++){
            int cnt=0;
            for(int h=r.nHoursPerDay-1; h>=0; h--){
                if(c.subgroupsMatrix[sbg][d][h]>0)
                    break;
                else
                    cnt++;
            }
            for(int h=0; h<r.nHoursPerDay; h++){
                if(c.subgroupsMatrix[sbg][(d+1<=r.nDaysPerWeek-1?d+1:0)][h]>0)
                    break;
                else
                    cnt++;
            }
            if(cnt < this->minRestingHours)
                nbroken++;
        }
    }

//    assert(nbroken==0);
//    return nbroken;
    return nbroken;
}

bool ConstraintStudentsMinRestingHours::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsMinRestingHours::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsMinRestingHours::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMinRestingHours::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsMinRestingHours::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsMinRestingHours::hasWrongDayOrHour()
{
    if(minRestingHours>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsMinRestingHours::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsMinRestingHours::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(minRestingHours>r.nHoursPerDay)
        minRestingHours=r.nHoursPerDay;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintMinActivitiesForAllTeachers::ConstraintMinActivitiesForAllTeachers(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_MIN_ACTIVITIES_FOR_ALL_TEACHERS;
}

ConstraintMinActivitiesForAllTeachers::ConstraintMinActivitiesForAllTeachers(Instance& r, Enums::ConstraintGroup constraintGroup, int minActivities)
    : TimeConstraint(r, constraintGroup)
{
    assert(minActivities>0);
    this->minActivities=minActivities;

    this->type=CONSTRAINT_MIN_ACTIVITIES_FOR_ALL_TEACHERS;
}

bool ConstraintMinActivitiesForAllTeachers::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    return true;
}

bool ConstraintMinActivitiesForAllTeachers::hasInactiveActivities()
{
    return false;
}

QString ConstraintMinActivitiesForAllTeachers::getXmlDescription(){
    QString s="<ConstraintMinActivitiesForAllTeachers>\n";
    s+=getSuperXmlDescription();
    s+="	<Minimum_Activities>"+utils::strings::number(this->minActivities)+"</Minimum_Activities>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMinActivitiesForAllTeachers>\n";
    return s;
}

QString ConstraintMinActivitiesForAllTeachers::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers min activities");
    s+=", " + getSuperDescription() + ", ";
    s+=tr("MA:%1", "Minimum activities").arg(this->minActivities);

    return begin+s+end;
}

QString ConstraintMinActivitiesForAllTeachers::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the minimum number of activities allocated to them");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Minimum activities=%1").arg(this->minActivities);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMinActivitiesForAllTeachers::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    c.povoarAllocation();

    QHash<int, int> teacher_activities_count_map;

    for(int i=0; i<c.instance->teachersList.size(); i++){
        int actCount = 0;

        for (int t = 0; t < c.instance->getNTimeSlots(); ++t){
            if (c.allocation[(unsigned)i][(unsigned)t] != UNALLOCATED_ACTIVITY) {
                actCount++;
            }
        }

        teacher_activities_count_map.insert(i, actCount);
    }

    int nbroken;
    nbroken=0;

    for(int i=0; i<r.teachersList.size(); i++){

        if (!c.instance->teachersList[i]->active) {
            continue;
        }

        if(teacher_activities_count_map[i]<this->minActivities){
            nbroken++;

            if(collectConflictsData){
                int parcialViolation = 1;

                QString s=(tr(
                               "Time constraint teachers min %1 activities broken for teacher %2.")
                           .arg(utils::strings::number(this->minActivities))
                           .arg(r.teachersList.at(i)->name)
                           );

                c.constraintConflictData.addConflict(group, s, parcialViolation);


            }
        }
    }

    return nbroken;
}

bool ConstraintMinActivitiesForAllTeachers::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);

    return false;
}

bool ConstraintMinActivitiesForAllTeachers::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintMinActivitiesForAllTeachers::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForAllTeachers::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForAllTeachers::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForAllTeachers::hasWrongDayOrHour()
{
    return false;
}

bool ConstraintMinActivitiesForAllTeachers::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMinActivitiesForAllTeachers::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintMinActivitiesForTeacher::ConstraintMinActivitiesForTeacher(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_MIN_ACTIVITIES_FOR_TEACHER;
}

ConstraintMinActivitiesForTeacher::ConstraintMinActivitiesForTeacher(Instance& r, Enums::ConstraintGroup constraintGroup, int minActivities,  const QString &name)
    : TimeConstraint(r, constraintGroup)
{
    assert(minActivities>0);
    this->minActivities=minActivities;
    this->teacherName=name;

    this->type=CONSTRAINT_MIN_ACTIVITIES_FOR_TEACHER;
}

bool ConstraintMinActivitiesForTeacher::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintMinActivitiesForTeacher::hasInactiveActivities()
{
    return false;
}

QString ConstraintMinActivitiesForTeacher::getXmlDescription(){
    QString s="<ConstraintMinActivitiesForTeacher>\n";
    s+=getSuperXmlDescription();
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Minimum_Activities>"+utils::strings::number(this->minActivities)+"</Minimum_Activities>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMinActivitiesForTeacher>\n";
    return s;
}

QString ConstraintMinActivitiesForTeacher::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher min activities");s+=", " + getSuperDescription() + ", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MA:%1", "Minimum activities").arg(this->minActivities);

    return begin+s+end;
}

QString ConstraintMinActivitiesForTeacher::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the minimum number of activities");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Minimum activities=%1").arg(this->minActivities);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMinActivitiesForTeacher::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;

    int i=this->teacher_ID;

    Teacher* teacher = c.instance->teachersList[i];

    if (teacher->active) {

        c.povoarAllocation();

        QHash<int, int> teacher_activities_count_map;

        int actCount = 0;

        for (int t = 0; t < c.instance->getNTimeSlots(); ++t){
            if (c.allocation[(unsigned)i][(unsigned)t] != UNALLOCATED_ACTIVITY) {
                actCount++;
            }
        }

        teacher_activities_count_map.insert(i, actCount);

        if(teacher_activities_count_map[i]<this->minActivities)
        {
            nbroken++;

            if(collectConflictsData){
                int parcialViolation = 1;

                QString s=(tr(
                               "Time constraint teacher min %1 activities broken for teacher %2.")
                           .arg(utils::strings::number(this->minActivities))
                           .arg(r.teachersList.at(i)->name)
                           );

                c.constraintConflictData.addConflict(group, s, parcialViolation);


            }
        }
    }

    return nbroken;
}

bool ConstraintMinActivitiesForTeacher::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);

    return false;
}

bool ConstraintMinActivitiesForTeacher::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintMinActivitiesForTeacher::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForTeacher::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForTeacher::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMinActivitiesForTeacher::hasWrongDayOrHour()
{
    return false;
}

bool ConstraintMinActivitiesForTeacher::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMinActivitiesForTeacher::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintMaxActivitiesForAllTeachers::ConstraintMaxActivitiesForAllTeachers(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_MAX_ACTIVITIES_FOR_ALL_TEACHERS;
}

ConstraintMaxActivitiesForAllTeachers::ConstraintMaxActivitiesForAllTeachers(Instance& r, Enums::ConstraintGroup constraintGroup, int maxActivities)
    : TimeConstraint(r, constraintGroup)
{
    assert(maxActivities>=0);
    this->maxActivities=maxActivities;

    this->type=CONSTRAINT_MAX_ACTIVITIES_FOR_ALL_TEACHERS;
}

bool ConstraintMaxActivitiesForAllTeachers::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    return true;
}

bool ConstraintMaxActivitiesForAllTeachers::hasInactiveActivities()
{
    return false;
}

QString ConstraintMaxActivitiesForAllTeachers::getXmlDescription(){
    QString s="<ConstraintMaxActivitiesForAllTeachers>\n";
    s+=getSuperXmlDescription();
    s+="	<Maximum_Activities>"+utils::strings::number(this->maxActivities)+"</Maximum_Activities>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMaxActivitiesForAllTeachers>\n";
    return s;
}

QString ConstraintMaxActivitiesForAllTeachers::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max activities");
    s+=", " + getSuperDescription() + ", ";
    s+=tr("MA:%1", "Maximum activities").arg(this->maxActivities);

    return begin+s+end;
}

QString ConstraintMaxActivitiesForAllTeachers::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of activities allocated to them");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Maximum activities=%1").arg(this->maxActivities);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMaxActivitiesForAllTeachers::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    c.povoarAllocation();

    QHash<int, int> teacher_activities_count_map;

    for(int i=0; i<c.instance->teachersList.size(); i++){
        int actCount = 0;

        for (int t = 0; t < c.instance->getNTimeSlots(); ++t){
            if (c.allocation[(unsigned)i][(unsigned)t] != UNALLOCATED_ACTIVITY) {
                actCount++;
            }
        }

        teacher_activities_count_map.insert(i, actCount);
    }

    int nbroken;
    nbroken=0;

    for(int i=0; i<r.teachersList.size(); i++){
        if(teacher_activities_count_map[i]>this->maxActivities){
            nbroken++;

            if(collectConflictsData){
                int parcialViolation = 1;

                QString s=(tr(
                               "Time constraint teachers max %1 activities broken for teacher %2.")
                           .arg(utils::strings::number(this->maxActivities))
                           .arg(r.teachersList.at(i)->name)
                           );

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintMaxActivitiesForAllTeachers::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);

    return false;
}

bool ConstraintMaxActivitiesForAllTeachers::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintMaxActivitiesForAllTeachers::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForAllTeachers::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForAllTeachers::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForAllTeachers::hasWrongDayOrHour()
{
    return false;
}

bool ConstraintMaxActivitiesForAllTeachers::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMaxActivitiesForAllTeachers::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintMaxActivitiesForTeacher::ConstraintMaxActivitiesForTeacher(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_MAX_ACTIVITIES_FOR_TEACHER;
}

ConstraintMaxActivitiesForTeacher::ConstraintMaxActivitiesForTeacher(Instance& r, Enums::ConstraintGroup constraintGroup, int maxActivities,  const QString &name)
    : TimeConstraint(r, constraintGroup)
{
    assert(maxActivities>=0);
    this->maxActivities=maxActivities;
    this->teacherName=name;

    this->type=CONSTRAINT_MAX_ACTIVITIES_FOR_TEACHER;
}

bool ConstraintMaxActivitiesForTeacher::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintMaxActivitiesForTeacher::hasInactiveActivities()
{
    return false;
}

QString ConstraintMaxActivitiesForTeacher::getXmlDescription(){
    QString s="<ConstraintMaxActivitiesForTeacher>\n";
    s+=getSuperXmlDescription();
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Maximum_Activities>"+utils::strings::number(this->maxActivities)+"</Maximum_Activities>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintMaxActivitiesForTeacher>\n";
    return s;
}

QString ConstraintMaxActivitiesForTeacher::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max activities");s+=", " + getSuperDescription() + ", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MA:%1", "Maximum activities").arg(this->maxActivities);

    return begin+s+end;
}

QString ConstraintMaxActivitiesForTeacher::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of activities");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum activities=%1").arg(this->maxActivities);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintMaxActivitiesForTeacher::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    c.povoarAllocation();

    int i=this->teacher_ID;

    QHash<int, int> teacher_activities_count_map;

    int actCount = 0;

    for (int t = 0; t < c.instance->getNTimeSlots(); ++t){
        if (c.allocation[(unsigned)i][(unsigned)t] != UNALLOCATED_ACTIVITY) {
            actCount++;
        }
    }

    teacher_activities_count_map.insert(i, actCount);

    int nbroken;
    nbroken=0;

    if(teacher_activities_count_map[i]>this->maxActivities)
    {
        nbroken++;

        if(collectConflictsData){
            int parcialViolation = 1;

            QString s=(tr(
                           "Time constraint teacher max %1 activities broken for teacher %2.")
                       .arg(utils::strings::number(this->maxActivities))
                       .arg(r.teachersList.at(i)->name)
                       );

            c.constraintConflictData.addConflict(group, s, parcialViolation);
        }
    }

    return nbroken;
}

bool ConstraintMaxActivitiesForTeacher::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);

    return false;
}

bool ConstraintMaxActivitiesForTeacher::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintMaxActivitiesForTeacher::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForTeacher::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForTeacher::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintMaxActivitiesForTeacher::hasWrongDayOrHour()
{
    return false;
}

bool ConstraintMaxActivitiesForTeacher::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintMaxActivitiesForTeacher::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersMaxHoursInWorkDaily::ConstraintTeachersMaxHoursInWorkDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_IN_WORK_DAILY;
}

ConstraintTeachersMaxHoursInWorkDaily::ConstraintTeachersMaxHoursInWorkDaily(Instance& r, Enums::ConstraintGroup constraintGroup, int maxHoursInWork)
    : TimeConstraint(r, constraintGroup)
{
    assert(maxHoursInWork>0);
    this->maxHoursInWork=maxHoursInWork;

    this->type=CONSTRAINT_TEACHERS_MAX_HOURS_IN_WORK_DAILY;
}

bool ConstraintTeachersMaxHoursInWorkDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(r);

    return true;
}

bool ConstraintTeachersMaxHoursInWorkDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersMaxHoursInWorkDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersMaxHoursInWorkDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Maximum_Hours_InWork>"+utils::strings::number(this->maxHoursInWork)+"</Maximum_Hours_InWork>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersMaxHoursInWorkDaily>\n";
    return s;
}

QString ConstraintTeachersMaxHoursInWorkDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers max hours in work daily");
    s+=", " + getSuperDescription() + ", ";
    s+=tr("MH:%1", "Maximum hours in work daily").arg(this->maxHoursInWork);

    return begin+s+end;
}

QString ConstraintTeachersMaxHoursInWorkDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers must respect the maximum number of hours in work daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Maximum hours in work daily=%1").arg(this->maxHoursInWork);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersMaxHoursInWorkDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken=0;

    for(int i=0; i<r.teachersList.size(); i++){
        for(int d=0; d<r.nDaysPerWeek; d++){
            int first=-1;
            int last=-1;

            int h;

            for(h=0; h<r.nHoursPerDay; h++){
                if(c.teachersMatrix[i][d][h]>0){
                    first = h;
                    break;
                }
            }

            for(; h<r.nHoursPerDay; h++){
                if(c.teachersMatrix[i][d][h]>0){
                    last = h;
                }
            }

            if(first!=-1 and (last-first + 1) > this->maxHoursInWork){
                int parcialViolation = 1;

                nbroken++;

                if(collectConflictsData){
                    QString s=(tr(
                                   "Time constraint teachers max %1 hours in work daily broken for teacher %2, on day %3, length=%4.")
                               .arg(utils::strings::number(this->maxHoursInWork))
                               .arg(r.teachersList.at(i)->name)
                               .arg(r.daysOfTheWeek[d])
                               .arg((last-first + 1))
                               )
                            +
                            " "
                            +
                            (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintTeachersMaxHoursInWorkDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersMaxHoursInWorkDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersMaxHoursInWorkDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursInWorkDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursInWorkDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersMaxHoursInWorkDaily::hasWrongDayOrHour()
{
    if(maxHoursInWork>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersMaxHoursInWorkDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersMaxHoursInWorkDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursInWork>r.nHoursPerDay)
        maxHoursInWork=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherMaxHoursInWorkDaily::ConstraintTeacherMaxHoursInWorkDaily(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_MAX_HOURS_IN_WORK_DAILY;
}

ConstraintTeacherMaxHoursInWorkDaily::ConstraintTeacherMaxHoursInWorkDaily(Instance& r, Enums::ConstraintGroup constraintGroup, int maxHoursInWork, const QString& teacher)
    : TimeConstraint(r, constraintGroup)
{
    assert(maxHoursInWork>0);
    this->maxHoursInWork=maxHoursInWork;
    this->teacherName=teacher;

    this->type=CONSTRAINT_TEACHER_MAX_HOURS_IN_WORK_DAILY;
}

bool ConstraintTeacherMaxHoursInWorkDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);
    return true;
}

bool ConstraintTeacherMaxHoursInWorkDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherMaxHoursInWorkDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherMaxHoursInWorkDaily>\n";
    s+=getSuperXmlDescription();
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Maximum_Hours_InWork>"+utils::strings::number(this->maxHoursInWork)+"</Maximum_Hours_InWork>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherMaxHoursInWorkDaily>\n";
    return s;
}

QString ConstraintTeacherMaxHoursInWorkDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher max hours in work daily");s+=", " + getSuperDescription() + ", ";
    s+=tr("T:%1", "Teacher").arg(this->teacherName);s+=", ";
    s+=tr("MH:%1", "Maximum hours in work").arg(this->maxHoursInWork);

    return begin+s+end;
}

QString ConstraintTeacherMaxHoursInWorkDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher must respect the maximum number of hours in work daily");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Maximum hours in work daily=%1").arg(this->maxHoursInWork);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherMaxHoursInWorkDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;
    int i=this->teacher_ID;

    for(int d=0; d<r.nDaysPerWeek; d++){
        int first=-1;
        int last=-1;

        int h;

        for(h=0; h<r.nHoursPerDay; h++){
            if(c.teachersMatrix[i][d][h]>0){
                first = h;
                break;
            }
        }

        for(; h<r.nHoursPerDay; h++){
            if(c.teachersMatrix[i][d][h]>0){
                last = h;
            }
        }

        if(first!=-1 and (last-first + 1) > this->maxHoursInWork){
            int parcialViolation = 1;

            nbroken++;

            if(collectConflictsData){
                QString s=(tr(
                               "Time constraint teacher max %1 hours in work daily broken for teacher %2, on day %3, length=%4.")
                           .arg(utils::strings::number(this->maxHoursInWork))
                           .arg(r.teachersList.at(i)->name)
                           .arg(r.daysOfTheWeek[d])
                           .arg((last-first + 1))
                           )
                        +" "
                        +
                        (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                c.constraintConflictData.addConflict(group, s, parcialViolation);
            }
        }
    }

    return nbroken;
}

bool ConstraintTeacherMaxHoursInWorkDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::hasWrongDayOrHour()
{
    if(maxHoursInWork>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherMaxHoursInWorkDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherMaxHoursInWorkDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursInWork>r.nHoursPerDay)
        maxHoursInWork=r.nHoursPerDay;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintNoTeacherInSubjectPreference::ConstraintNoTeacherInSubjectPreference(Instance& r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_NO_TEACHER_SUBJECT_PREFERENCE;
}

ConstraintNoTeacherInSubjectPreference::ConstraintNoTeacherInSubjectPreference(Instance& r, Enums::ConstraintGroup constraintGroup, Enums::SubjectPreference pref)
    : TimeConstraint(r, constraintGroup)
{
    this->pref=pref;

    this->type=CONSTRAINT_NO_TEACHER_SUBJECT_PREFERENCE;
}

bool ConstraintNoTeacherInSubjectPreference::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    return true;
}

bool ConstraintNoTeacherInSubjectPreference::hasInactiveActivities()
{
    return false;
}

QString ConstraintNoTeacherInSubjectPreference::getXmlDescription(){
    QString s="<ConstraintNoTeacherInSubjectPreference>\n";
    s+=getSuperXmlDescription();
    s+="	<Preference>"+utils::enums::enumIndexToStr(pref)+"</Preference>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintNoTeacherInSubjectPreference>\n";
    return s;
}

QString ConstraintNoTeacherInSubjectPreference::getDescription(){
    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("No teacher in subject preference");s+=", " + getSuperDescription() + ", ";
    s+=tr("SPref:%1", "Preference").arg(utils::enums::enumIndexToStr(this->pref));

    return begin+s+end;
}

QString ConstraintNoTeacherInSubjectPreference::getDetailedDescription(){
    QString s=tr("Time constraint");s+="\n";
    s+=tr("No teacher can be allocated to a subject of a preference");s+="\n";
    s+= getSuperDetailedDescription();s+="\n";
    s+=tr("Subject Preference:%1", "Preference").arg(utils::enums::enumIndexToStr(this->pref));

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }

    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintNoTeacherInSubjectPreference::violationsFactor(Solution& c, bool collectConflictsData)
{
    //TODO: ver se ta tudo certo

    c.povoarAllocation();

    QHash<int, int> teacher_subject_count_map;

    for(int i=0; i<c.instance->teachersList.size(); i++){
        int count = 0;

        for (int t = 0; t < c.instance->getNTimeSlots(); ++t){
            int ai = c.allocation[(unsigned)i][(unsigned)t];
            if (ai != UNALLOCATED_ACTIVITY) {
                Teacher* tch = c.instance->teachersList[i];
                Subject* sbj = c.instance->subjectsList[c.instance->activeActivitiesList[ai]->subjectIndex];

                if (this->pref == c.instance->teacher_subject_preference_map[tch][sbj]) {
                    count++;
                }
            }
        }

        teacher_subject_count_map.insert(i, count);
    }

    int nbroken;
    nbroken=0;

    for(int i=0; i<r.teachersList.size(); i++){
        if(teacher_subject_count_map[i] > 0){
            nbroken++;

            if(collectConflictsData){
                int parcialViolation = 1;

                QString s=(tr(
                               "Constraint no teachers allocated to subjects of preference '%1'.")
                           .arg(utils::enums::enumIndexToStr(this->pref))
                           );

                c.constraintConflictData.addConflict(group, s, parcialViolation);


            }
        }
    }

    return nbroken;
}

bool ConstraintNoTeacherInSubjectPreference::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(a);

    return false;
}

bool ConstraintNoTeacherInSubjectPreference::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintNoTeacherInSubjectPreference::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintNoTeacherInSubjectPreference::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintNoTeacherInSubjectPreference::isRelatedToStudentsSet( StudentsSet* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintNoTeacherInSubjectPreference::hasWrongDayOrHour()
{
    return false;
}

bool ConstraintNoTeacherInSubjectPreference::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintNoTeacherInSubjectPreference::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersActivityTagMaxHoursContinuously::ConstraintTeachersActivityTagMaxHoursContinuously(Instance &r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

ConstraintTeachersActivityTagMaxHoursContinuously::ConstraintTeachersActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& activityTag)
 : TimeConstraint(r, constraintGroup, wp)
 {
    assert(maxhours>0);
    this->maxHoursContinuously=maxhours;
    this->activityTagName=activityTag;

    this->type=CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    /* TODO: colocar nova lógica de professores

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalTeachersList.clear();
    for(int i=0; i<r.teachersList.size(); i++){
        bool found=false;

        Teacher* tch=r.teachersList.at(i);
        foreach(int actIndex, tch->activitiesForTeacher){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalTeachersList.append(i);
    }
    */
    return true;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersActivityTagMaxHoursContinuously::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersActivityTagMaxHoursContinuously>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersActivityTagMaxHoursContinuously>\n";
    return s;
}

QString ConstraintTeachersActivityTagMaxHoursContinuously::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teachers for activity tag %1 have max %2 hours continuously").arg(this->activityTagName).arg(this->maxHoursContinuously);s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTeachersActivityTagMaxHoursContinuously::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All teachers, for an activity tag, must respect the maximum number of hours continuously");s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName); s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersActivityTagMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;
    Q_UNUSED(collectConflictsData)
    /* TODO: colocar nova lógica de professores

    foreach(int i, this->canonicalTeachersList){
        Teacher* tch=r.teachersList.at(i);
        int crtTeacherTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtTeacherTimetableActivityTag[d][h]=-1;

        foreach(int ai, tch->activitiesForTeacher)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtTeacherTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtTeacherTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                bool inc=false;
                if(crtTeacherTimetableActivityTag[d][h]==this->activityTagIndex)
                    inc=true;

                if(inc){
                    nc++;
                }
                else{
                    if(nc>this->maxHoursContinuously){
                        nbroken++;

                        if(collectConflictsData){
                            int parcialViolation = 1;

                            QString s=(tr(
                             "Time constraint teachers activity tag %1 max %2 hours continuously broken for teacher %3, on day %4, length=%5.")
                             .arg(this->activityTagName)
                             .arg(utils::strings::number(this->maxHoursContinuously))
                             .arg(r.teachersList.at(i)->name)
                             .arg(r.daysOfTheWeek[d])
                             .arg(nc)
                             )
                             +
                             " "
                             +
                             (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint teachers activity tag %1 max %2 hours continuously broken for teacher %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursContinuously))
                     .arg(r.teachersList.at(i)->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nc)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }
    */
    return nbroken;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    return s->name==this->activityTagName;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersActivityTagMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
ConstraintTeacherActivityTagMaxHoursContinuously::ConstraintTeacherActivityTagMaxHoursContinuously(Instance &r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

ConstraintTeacherActivityTagMaxHoursContinuously::ConstraintTeacherActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher, const QString& activityTag)
 : TimeConstraint(r, constraintGroup, wp)
 {
    assert(maxhours>0);
    this->maxHoursContinuously=maxhours;
    this->teacherName=teacher;
    this->activityTagName=activityTag;

    this->type=CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    /* TODO: colocar nova lógica de professores

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalTeachersList.clear();
    int i=this->teacher_ID;
    bool found=false;

    Teacher* tch=r.teachersList.at(i);
    foreach(int actIndex, tch->activitiesForTeacher){
        if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
            found=true;
            break;
        }
    }

    if(found)
        this->canonicalTeachersList.append(i);
    */
    return true;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherActivityTagMaxHoursContinuously::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherActivityTagMaxHoursContinuously>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherActivityTagMaxHoursContinuously>\n";
    return s;
}

QString ConstraintTeacherActivityTagMaxHoursContinuously::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Teacher %1 for activity tag %2 has max %3 hours continuously").arg(this->teacherName).arg(this->activityTagName).arg(this->maxHoursContinuously);s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTeacherActivityTagMaxHoursContinuously::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A teacher for an activity tag must respect the maximum number of hours continuously");s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherActivityTagMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;
    Q_UNUSED(collectConflictsData)
    /* TODO: colocar nova lógica de professores

    foreach(int i, this->canonicalTeachersList){
        Teacher* tch=r.teachersList.at(i);
        int crtTeacherTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtTeacherTimetableActivityTag[d][h]=-1;

        foreach(int ai, tch->activitiesForTeacher)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtTeacherTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtTeacherTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                bool inc=false;

                if(crtTeacherTimetableActivityTag[d][h]==this->activityTagIndex)
                    inc=true;

                if(inc)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        nbroken++;

                        if(collectConflictsData){
                            int parcialViolation = 1;

                            QString s=(tr(
                             "Time constraint teacher activity tag max %1 hours continuously broken for teacher %2, activity tag %3, on day %4, length=%5.")
                             .arg(utils::strings::number(this->maxHoursContinuously))
                             .arg(r.teachersList.at(i)->name)
                             .arg(this->activityTagName)
                             .arg(r.daysOfTheWeek[d])
                             .arg(nc)
                             )
                             +
                             " "
                             +
                             (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint teacher activity tag max %1 hours continuously broken for teacher %2, activity tag %3, on day %4, length=%5.")
                     .arg(utils::strings::number(this->maxHoursContinuously))
                     .arg(r.teachersList.at(i)->name)
                     .arg(this->activityTagName)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nc)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }
    */
    return nbroken;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    return this->activityTagName==s->name;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherActivityTagMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsActivityTagMaxHoursContinuously::ConstraintStudentsActivityTagMaxHoursContinuously(Instance &r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
    this->maxHoursContinuously = -1;
}

ConstraintStudentsActivityTagMaxHoursContinuously::ConstraintStudentsActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& activityTag)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursContinuously = maxnh;
    this->activityTagName=activityTag;
    this->type = CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalSubgroupsList.clear();
    for(int i=0; i<r.directSubgroupsList.size(); i++){
        bool found=false;

        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        foreach(int actIndex, sbg->activitiesForSubgroup){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalSubgroupsList.append(i);
    }

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsActivityTagMaxHoursContinuously::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsActivityTagMaxHoursContinuously>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";

    s+="	<Activity_Tag>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag>\n";

    if(this->maxHoursContinuously>=0)
        s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    else
        assert(0);
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsActivityTagMaxHoursContinuously>\n";
    return s;
}

QString ConstraintStudentsActivityTagMaxHoursContinuously::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students for activity tag %1 have max %2 hours continuously")
        .arg(this->activityTagName).arg(this->maxHoursContinuously); s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintStudentsActivityTagMaxHoursContinuously::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("All students, for an activity tag, must respect the maximum number of hours continuously"); s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsActivityTagMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int i, this->canonicalSubgroupsList){
        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        int crtSubgroupTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtSubgroupTimetableActivityTag[d][h]=-1;

        foreach(int ai, sbg->activitiesForSubgroup)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtSubgroupTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtSubgroupTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                bool inc=false;

                if(crtSubgroupTimetableActivityTag[d][h]==this->activityTagIndex)
                    inc=true;

                if(inc)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        nbroken++;

                        if(collectConflictsData){
                            int parcialViolation = 1;

                            QString s=(tr(
                             "Time constraint students, activity tag %1, max %2 hours continuously, broken for subgroup %3, on day %4, length=%5.")
                             .arg(this->activityTagName)
                             .arg(utils::strings::number(this->maxHoursContinuously))
                             .arg(r.directSubgroupsList[i]->name)
                             .arg(r.daysOfTheWeek[d])
                             .arg(nc)
                             )
                             +
                             " "
                             +
                             (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint students, activity tag %1, max %2 hours continuously, broken for subgroup %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursContinuously))
                     .arg(r.directSubgroupsList[i]->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nc)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    return s->name==this->activityTagName;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetActivityTagMaxHoursContinuously::ConstraintStudentsSetActivityTagMaxHoursContinuously(Instance &r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
    this->maxHoursContinuously = -1;
}

ConstraintStudentsSetActivityTagMaxHoursContinuously::ConstraintStudentsSetActivityTagMaxHoursContinuously(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& s, const QString& activityTag)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursContinuously = maxnh;
    this->students = s;
    this->activityTagName=activityTag;
    this->type = CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_CONTINUOUSLY;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetActivityTagMaxHoursContinuously::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetActivityTagMaxHoursContinuously>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Continuously>"+utils::strings::number(this->maxHoursContinuously)+"</Maximum_Hours_Continuously>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Activity_Tag>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetActivityTagMaxHoursContinuously>\n";
    return s;
}

QString ConstraintStudentsSetActivityTagMaxHoursContinuously::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+=tr("Students set %1 for activity tag %2 has max %3 hours continuously").arg(this->students).arg(this->activityTagName).arg(this->maxHoursContinuously);
    s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintStudentsSetActivityTagMaxHoursContinuously::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("A students set, for an activity tag, must respect the maximum number of hours continuously"); s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours continuously=%1").arg(this->maxHoursContinuously);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::computeInternalStructure(QWidget* parent)
{
    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
         tr("Constraint students set max hours continuously is wrong because it refers to inexistent students set."
         " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    /////////////
    this->canonicalSubgroupsList.clear();
    foreach(int i, this->iSubgroupsList){
        bool found=false;

        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        foreach(int actIndex, sbg->activitiesForSubgroup){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalSubgroupsList.append(i);
    }


    return true;
}

double ConstraintStudentsSetActivityTagMaxHoursContinuously::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int i, this->canonicalSubgroupsList){
        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        int crtSubgroupTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtSubgroupTimetableActivityTag[d][h]=-1;

        foreach(int ai, sbg->activitiesForSubgroup)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtSubgroupTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtSubgroupTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nc=0;
            for(int h=0; h<r.nHoursPerDay; h++){
                bool inc=false;

                if(crtSubgroupTimetableActivityTag[d][h]==this->activityTagIndex)
                    inc=true;

                if(inc)
                    nc++;
                else{
                    if(nc>this->maxHoursContinuously){
                        nbroken++;

                        if(collectConflictsData){
                            int parcialViolation = 1;

                            QString s=(tr(
                             "Time constraint students set max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                             .arg(utils::strings::number(this->maxHoursContinuously))
                             .arg(r.directSubgroupsList[i]->name)
                             .arg(r.daysOfTheWeek[d])
                             .arg(nc)
                             )
                             +
                             " "
                             +
                             (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                            c.constraintConflictData.addConflict(group, s, parcialViolation);
                        }
                    }

                    nc=0;
                }
            }

            if(nc>this->maxHoursContinuously){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint students set max %1 hours continuously broken for subgroup %2, on day %3, length=%4.")
                     .arg(utils::strings::number(this->maxHoursContinuously))
                     .arg(r.directSubgroupsList[i]->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nc)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::hasWrongDayOrHour()
{
    if(maxHoursContinuously>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetActivityTagMaxHoursContinuously::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursContinuously>r.nHoursPerDay)
        maxHoursContinuously=r.nHoursPerDay;

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeachersActivityTagMaxHoursDaily::ConstraintTeachersActivityTagMaxHoursDaily(Instance &r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

ConstraintTeachersActivityTagMaxHoursDaily::ConstraintTeachersActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& activityTag)
 : TimeConstraint(r, constraintGroup, wp)
 {
    assert(maxhours>0);
    this->maxHoursDaily=maxhours;
    this->activityTagName=activityTag;

    this->type=CONSTRAINT_TEACHERS_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    /* TODO: colocar nova lógica de professores

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalTeachersList.clear();
    for(int i=0; i<r.teachersList.size(); i++){
        bool found=false;

        Teacher* tch=r.teachersList.at(i);
        foreach(int actIndex, tch->activitiesForTeacher){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalTeachersList.append(i);
    }
    */

    return true;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeachersActivityTagMaxHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeachersActivityTagMaxHoursDaily>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeachersActivityTagMaxHoursDaily>\n";
    return s;
}

QString ConstraintTeachersActivityTagMaxHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Teachers for activity tag %1 have max %2 hours daily").arg(this->activityTagName).arg(this->maxHoursDaily);s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTeachersActivityTagMaxHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("All teachers, for an activity tag, must respect the maximum number of hours daily");s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName); s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeachersActivityTagMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;
    Q_UNUSED(collectConflictsData)
    /* TODO: colocar nova lógica de professores

    foreach(int i, this->canonicalTeachersList){
        Teacher* tch=r.teachersList.at(i);
        int crtTeacherTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtTeacherTimetableActivityTag[d][h]=-1;

        foreach(int ai, tch->activitiesForTeacher)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtTeacherTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtTeacherTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nd=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(crtTeacherTimetableActivityTag[d][h]==this->activityTagIndex)
                    nd++;

            if(nd>this->maxHoursDaily){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr("Time constraint teachers activity tag %1 max %2 hours daily broken for teacher %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursDaily))
                     .arg(r.teachersList.at(i)->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nd)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }
    */
    return nbroken;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return true;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    return s->name==this->activityTagName;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeachersActivityTagMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

ConstraintTeacherActivityTagMaxHoursDaily::ConstraintTeacherActivityTagMaxHoursDaily(Instance &r)
    : TimeConstraint(r)
{
    this->type=CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

ConstraintTeacherActivityTagMaxHoursDaily::ConstraintTeacherActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxhours, const QString& teacher, const QString& activityTag)
 : TimeConstraint(r, constraintGroup, wp)
 {
    assert(maxhours>0);
    this->maxHoursDaily=maxhours;
    this->teacherName=teacher;
    this->activityTagName=activityTag;

    this->type=CONSTRAINT_TEACHER_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);
    /* TODO: colocar nova lógica de professores

    //this->teacher_ID=r.searchTeacher(this->teacherName);
    teacher_ID=r.getTeacherIndex(teacherName);
    assert(this->teacher_ID>=0);

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalTeachersList.clear();
    int i=this->teacher_ID;
    bool found=false;

    Teacher* tch=r.teachersList.at(i);
    foreach(int actIndex, tch->activitiesForTeacher){
        if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
            found=true;
            break;
        }
    }

    if(found)
        this->canonicalTeachersList.append(i);
    */
    return true;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintTeacherActivityTagMaxHoursDaily::getXmlDescription(){
    Q_UNUSED(r);

    QString s="<ConstraintTeacherActivityTagMaxHoursDaily>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Teacher_Name>"+utils::strings::parseStrForXml(this->teacherName)+"</Teacher_Name>\n";
    s+="	<Activity_Tag_Name>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag_Name>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintTeacherActivityTagMaxHoursDaily>\n";
    return s;
}

QString ConstraintTeacherActivityTagMaxHoursDaily::getDescription(){
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Teacher %1 for activity tag %2 has max %3 hours daily").arg(this->teacherName).arg(this->activityTagName).arg(this->maxHoursDaily);s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintTeacherActivityTagMaxHoursDaily::getDetailedDescription(){
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("A teacher for an activity tag must respect the maximum number of hours daily");s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Teacher=%1").arg(this->teacherName);s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily); s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintTeacherActivityTagMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;
    nbroken=0;

    Q_UNUSED(collectConflictsData)
    /* TODO: colocar nova lógica de professores

    foreach(int i, this->canonicalTeachersList){
        Teacher* tch=r.teachersList.at(i);
        int crtTeacherTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtTeacherTimetableActivityTag[d][h]=-1;

        foreach(int ai, tch->activitiesForTeacher)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtTeacherTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtTeacherTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nd=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(crtTeacherTimetableActivityTag[d][h]==this->activityTagIndex)
                    nd++;

            if(nd>this->maxHoursDaily){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr("Time constraint teacher activity tag %1 max %2 hours daily broken for teacher %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursDaily))
                     .arg(r.teachersList.at(i)->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nd)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }
    */
    return nbroken;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    if(this->teacherName==t->name)
        return true;
    return false;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    return this->activityTagName==s->name;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintTeacherActivityTagMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsActivityTagMaxHoursDaily::ConstraintStudentsActivityTagMaxHoursDaily(Instance &r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY;
    this->maxHoursDaily = -1;
}

ConstraintStudentsActivityTagMaxHoursDaily::ConstraintStudentsActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& activityTag)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursDaily = maxnh;
    this->activityTagName=activityTag;
    this->type = CONSTRAINT_STUDENTS_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    Q_UNUSED(parent);

    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    this->canonicalSubgroupsList.clear();
    for(int i=0; i<r.directSubgroupsList.size(); i++){
        bool found=false;

        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        foreach(int actIndex, sbg->activitiesForSubgroup){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalSubgroupsList.append(i);
    }

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsActivityTagMaxHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsActivityTagMaxHoursDaily>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";

    s+="	<Activity_Tag>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag>\n";

    if(this->maxHoursDaily>=0)
        s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    else
        assert(0);
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsActivityTagMaxHoursDaily>\n";
    return s;
}

QString ConstraintStudentsActivityTagMaxHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Students for activity tag %1 have max %2 hours daily")
        .arg(this->activityTagName).arg(this->maxHoursDaily); s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintStudentsActivityTagMaxHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("All students, for an activity tag, must respect the maximum number of hours daily"); s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

double ConstraintStudentsActivityTagMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int i, this->canonicalSubgroupsList){
        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        int crtSubgroupTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtSubgroupTimetableActivityTag[d][h]=-1;
        foreach(int ai, sbg->activitiesForSubgroup)if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
            int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
            int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);
            for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                assert(h+dur<r.nHoursPerDay);
                assert(crtSubgroupTimetableActivityTag[d][h+dur]==-1);
                if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                    crtSubgroupTimetableActivityTag[d][h+dur]=this->activityTagIndex;
            }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nd=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(crtSubgroupTimetableActivityTag[d][h]==this->activityTagIndex)
                    nd++;

            if(nd>this->maxHoursDaily){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint students, activity tag %1, max %2 hours daily, broken for subgroup %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursDaily))
                     .arg(r.directSubgroupsList[i]->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nd)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::isRelatedToActivity(Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    return s->name==this->activityTagName;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::isRelatedToStudentsSet(StudentsSet* s)
{
    Q_UNUSED(r);
    Q_UNUSED(s);

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsActivityTagMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ConstraintStudentsSetActivityTagMaxHoursDaily::ConstraintStudentsSetActivityTagMaxHoursDaily(Instance &r)
    : TimeConstraint(r)
{
    this->type = CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY;
    this->maxHoursDaily = -1;
}

ConstraintStudentsSetActivityTagMaxHoursDaily::ConstraintStudentsSetActivityTagMaxHoursDaily(Instance &r, Enums::ConstraintGroup constraintGroup, double wp, int maxnh, const QString& s, const QString& activityTag)
    : TimeConstraint(r, constraintGroup, wp)
{
    this->maxHoursDaily = maxnh;
    this->students = s;
    this->activityTagName=activityTag;
    this->type = CONSTRAINT_STUDENTS_SET_ACTIVITY_TAG_MAX_HOURS_DAILY;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::hasInactiveActivities()
{
    Q_UNUSED(r);
    return false;
}

QString ConstraintStudentsSetActivityTagMaxHoursDaily::getXmlDescription()
{
    Q_UNUSED(r);

    QString s="<ConstraintStudentsSetActivityTagMaxHoursDaily>\n";
    s+="	<Weight_Percentage>"+utils::strings::number(this->weightPercentage)+"</Weight_Percentage>\n";
    s+="	<Maximum_Hours_Daily>"+utils::strings::number(this->maxHoursDaily)+"</Maximum_Hours_Daily>\n";
    s+="	<Students>"+utils::strings::parseStrForXml(this->students)+"</Students>\n";
    s+="	<Activity_Tag>"+utils::strings::parseStrForXml(this->activityTagName)+"</Activity_Tag>\n";
    s+="	<Active>"+utils::strings::trueFalse(active)+"</Active>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</ConstraintStudentsSetActivityTagMaxHoursDaily>\n";
    return s;
}

QString ConstraintStudentsSetActivityTagMaxHoursDaily::getDescription()
{
    Q_UNUSED(r);

    QString begin=QString("");
    if(!active)
        begin="X - ";

    QString end=QString("");
    if(!comments.isEmpty())
        end=", "+tr("C: %1", "Comments").arg(comments);

    QString s;
    s+="! ";
    s+=tr("Students set %1 for activity tag %2 has max %3 hours daily").arg(this->students).arg(this->activityTagName).arg(this->maxHoursDaily);
    s+=", ";
    s+=tr("WP:%1%", "Weight percentage").arg(utils::strings::number(this->weightPercentage));

    return begin+s+end;
}

QString ConstraintStudentsSetActivityTagMaxHoursDaily::getDetailedDescription()
{
    Q_UNUSED(r);

    QString s=tr("Time constraint");s+="\n";
    s+=tr("(not perfect)", "It refers to a not perfect constraint"); s+="\n";
    s+=tr("A students set, for an activity tag, must respect the maximum number of hours daily"); s+="\n";
    s+=tr("Weight (percentage)=%1%").arg(utils::strings::number(this->weightPercentage));s+="\n";
    s+=tr("Students set=%1").arg(this->students);s+="\n";
    s+=tr("Activity tag=%1").arg(this->activityTagName);s+="\n";
    s+=tr("Maximum hours daily=%1").arg(this->maxHoursDaily);s+="\n";

    if(!active){
        s+=tr("Active=%1", "Refers to a constraint").arg(utils::strings::yesNoTranslated(active));
        s+="\n";
    }
    if(!comments.isEmpty()){
        s+=tr("Comments=%1").arg(comments);
        s+="\n";
    }

    return s;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::computeInternalStructure(QWidget* parent)
{
    //this->activityTagIndex=r.searchActivityTag(this->activityTagName);
    activityTagIndex=r.getActivityTagIndex(activityTagName);
    assert(this->activityTagIndex>=0);

    //StudentsSet* ss=r.searchAugmentedStudentsSet(this->students);
    StudentsSet* ss=r.getStudentsSet(students);

    if(ss==nullptr){
        MessagesManager::warning(parent, tr("m-FET warning"),
         tr("Constraint students set max hours daily is wrong because it refers to inexistent students set."
         " Please correct it (removing it might be a solution). Please report potential bug. Constraint is:\n%1").arg(this->getDetailedDescription()));

        return false;
    }

    assert(ss);

    this->iSubgroupsList.clear();
    if(ss->type==STUDENTS_SUBGROUP){
        int tmp;
        tmp=((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
        assert(tmp>=0);
        assert(tmp<r.directSubgroupsList.size());
        if(!this->iSubgroupsList.contains(tmp))
            this->iSubgroupsList.append(tmp);
    }
    else if(ss->type==STUDENTS_GROUP){
        StudentsGroup* stg=(StudentsGroup*)ss;
        for(int i=0; i<stg->subgroupsList.size(); i++){
            StudentsSubgroup* sts=stg->subgroupsList[i];
            int tmp;
            tmp=sts->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());
            if(!this->iSubgroupsList.contains(tmp))
                this->iSubgroupsList.append(tmp);
        }
    }
    else if(ss->type==STUDENTS_YEAR){
        StudentsYear* sty=(StudentsYear*)ss;
        for(int i=0; i<sty->groupsList.size(); i++){
            StudentsGroup* stg=sty->groupsList[i];
            for(int j=0; j<stg->subgroupsList.size(); j++){
                StudentsSubgroup* sts=stg->subgroupsList[j];
                int tmp;
                tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());
                if(!this->iSubgroupsList.contains(tmp))
                    this->iSubgroupsList.append(tmp);
            }
        }
    }
    else
        assert(0);

    /////////////
    this->canonicalSubgroupsList.clear();
    foreach(int i, this->iSubgroupsList){
        bool found=false;

        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        foreach(int actIndex, sbg->activitiesForSubgroup){
            if(r.activeActivitiesList[actIndex]->iActivityTagsSet.contains(this->activityTagIndex)){
                found=true;
                break;
            }
        }

        if(found)
            this->canonicalSubgroupsList.append(i);
    }


    return true;
}

double ConstraintStudentsSetActivityTagMaxHoursDaily::violationsFactor(Solution& c, bool collectConflictsData)
{
    c.computeSubgroupsMatrix();
    c.computeTeachersMatrix();

    int nbroken;

    nbroken=0;

    foreach(int i, this->canonicalSubgroupsList){
        StudentsSubgroup* sbg=r.directSubgroupsList[i];
        int crtSubgroupTimetableActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

        for(int d=0; d<r.nDaysPerWeek; d++)
            for(int h=0; h<r.nHoursPerDay; h++)
                crtSubgroupTimetableActivityTag[d][h]=-1;

        foreach(int ai, sbg->activitiesForSubgroup)
            if(c.times[(unsigned)ai]!=UNALLOCATED_TIME){
                int d=r.getDayFromUnifiedTime(c.times[(unsigned)ai]);
                int h=r.getHourFromUnifiedTime(c.times[(unsigned)ai]);

                for(int dur=0; dur<r.activeActivitiesList[ai]->duration; dur++){
                    assert(h+dur<r.nHoursPerDay);
                    assert(crtSubgroupTimetableActivityTag[d][h+dur]==-1);
                    if(r.activeActivitiesList[ai]->iActivityTagsSet.contains(this->activityTagIndex))
                        crtSubgroupTimetableActivityTag[d][h+dur]=this->activityTagIndex;
                }
        }

        for(int d=0; d<r.nDaysPerWeek; d++){
            int nd=0;
            for(int h=0; h<r.nHoursPerDay; h++)
                if(crtSubgroupTimetableActivityTag[d][h]==this->activityTagIndex)
                    nd++;

            if(nd>this->maxHoursDaily){
                nbroken++;

                if(collectConflictsData){
                    int parcialViolation = 1;

                    QString s=(tr(
                     "Time constraint students set, activity tag %1, max %2 hours daily, broken for subgroup %3, on day %4, length=%5.")
                     .arg(this->activityTagName)
                     .arg(utils::strings::number(this->maxHoursDaily))
                     .arg(r.directSubgroupsList[i]->name)
                     .arg(r.daysOfTheWeek[d])
                     .arg(nd)
                     )
                     +
                     " "
                     +
                     (tr("This increases the total cost by %1").arg(utils::strings::number(parcialViolation * c.instance->getConstraintGroupWeight(this->group))));

                    c.constraintConflictData.addConflict(group, s, parcialViolation);
                }
            }
        }
    }

    return nbroken;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::isRelatedToActivity( Activity* a)
{
    Q_UNUSED(r);
    Q_UNUSED(a);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::isRelatedToTeacher(Teacher* t)
{
    Q_UNUSED(t);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::isRelatedToSubject(Subject* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::isRelatedToActivityTag(ActivityTag* s)
{
    Q_UNUSED(s);

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::isRelatedToStudentsSet( StudentsSet* s)
{
    return r.setsShareStudents(this->students, s->name);
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::hasWrongDayOrHour()
{
    if(maxHoursDaily>r.nHoursPerDay)
        return true;

    return false;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::canRepairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    return true;
}

bool ConstraintStudentsSetActivityTagMaxHoursDaily::repairWrongDayOrHour()
{
    assert(hasWrongDayOrHour());

    if(maxHoursDaily>r.nHoursPerDay)
        maxHoursDaily=r.nHoursPerDay;

    return true;
}

void to_json(json &j, const TimeConstraint &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, TimeConstraint &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

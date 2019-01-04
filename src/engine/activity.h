/*
File activity.h
*/

/***************************************************************************
                          activity.h  -  description
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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QCoreApplication>

#include <QString>
#include <QSet>
#include <QStringList>

#include "json.hpp"
using json = nlohmann::json;

class Instance;
class Activity;

typedef QList<Activity*> ActivitiesList;

/**
This class represents an activity.
<p>
An activity is a certain course (lecture), taught by a certain teacher (or more),
to a certain year (or group, or subgroup) of students (or more).
*/

class Activity{
	Q_DECLARE_TR_FUNCTIONS(Activity)

public:
    QString activityName;

	QString comments;

    bool allowMultipleTeachers;

	/**
	The name of the subject.
	*/
	QString subjectName;

	/**
	The name of the activity tag.
	*/
	QStringList activityTagsNames;

	/**
	The names of the sets of students involved in this activity (years, groups or subgroups).
	*/
    QStringList studentSetsNames;

	/**
	The duration, in hours.
	*/
	int duration;

	/**
	This value is used only for split activities (for high-schools).
	If totalDuration==duration, then this activity is not split.
	If totalDuration>duration, then this activity is split.
	*/
	int totalDuration;

	/**
	A unique ID for any activity. This is NOT the index (activities might be erased,
	but this ID remains the same).
	*/
	int id;

	/**
	The activities generated from a split activity have the same activityGroupId.
	For non-split activities, activityGroupId==0
	*/
	int activityGroupId;

	/**
	The number of students who are attending this activity.
	If computeNTotalStudentsFromSets is false, this number
	is given. If it is true, this number should be calculated
	from the sets involved.
	*/
	int nTotalStudents;
	
	/**
	If true, we will have to compute the number of total students from the 
	involved students sets. If false, it means that nTotalStudents is given
	and must not be recalculated.
	*/
	bool computeNTotalStudents;
	
	/**
	True if this activity is active, that is it will be taken into consideration
	when generating the timetable.
	*/
	bool active;
	
	/**
	If the teachers, subject, activity tag, students, duration are identical
	and the activity group id of both of them is 0 or of both of them is != 0, returns true.
	TODO: add a more intelligent comparison
	*/
	bool operator==(Activity &a);

	//internal structure

	/**
	The index of the subject.
	*/
	int subjectIndex;

	/**
	The indices of the activity tags.
	*/
	QSet<int> iActivityTagsSet;

	/**
	The indices of the subgroups implied in this activity.
	*/
	QList<int> iSubgroupsList;

    Activity(const Activity &act);
	
	/**
	Simple constructor, used only indirectly by the static variable
	"Activity internalActivitiesList[MAX_ACTIVITIES]".
	Any other use of this function should be avoided.
	*/
	Activity();

	/**
	Complete constructor.
	If _totalDuration!=duration, then this activity is a part of a bigger (split)
	activity.
	<p>
	As a must, for non-split activities, _activityGroupId==0.
	For the split ones, it is >0
	*/
    Activity(Instance& r,
        int _id,
        int _activityGroupId,
        const QString& _activityName,
        bool _allowMultipleTeachers,
        const QString& _subjectName,
        const QStringList& _activityTagsNames,
        const QStringList &_studentsName,
        int _duration,
        int _totalDuration,
        bool _active,
        bool _computeNTotalStudents,
        int _nTotalStudents,
        int _computedNumberOfStudents);
		
	bool searchStudents(const QString& studentsName);

	/**
	Removes this students set from the list of students
	*/
    bool removeStudents(Instance& r, const QString& studentsName, int nStudents);

	/**
	Renames this students set in the list of students and possibly modifies the number of students for the activity, if initialNumberOfStudents!=finalNumberOfStudents
	*/
    void renameStudents(Instance& r, const QString& initialStudentsName, const QString& finalStudentsName, int initialNumberOfStudents, int finalNumberOfStudents);

	/**
	Computes the internal structure
	*/
    void computeInternalStructure(Instance& r);

	/**
	Returns a representation of this activity (xml format).
	*/
    QString getXmlDescription();

	/**
	Returns a representation of this activity.
	*/
    QString getDescription();

	/**
	Returns a representation of this activity (more detailed).
	*/
    QString getDetailedDescription();

	/**
	Returns a representation of this activity (detailed),
	together with the constraints related to this activity.
	*/
    QString getDetailedDescriptionWithConstraints(Instance& r);

	/**
	Returns true if this activity is split into more lessons per week.
	*/
	bool isSplit();
	
	bool representsComponentNumber(int compNumber);

    friend void to_json(json& j, const Activity& s);
    friend void from_json(const json& j, Activity& p);
};

void to_json(json& j, const Activity& s);
void from_json(const json& j, Activity& p);

#endif

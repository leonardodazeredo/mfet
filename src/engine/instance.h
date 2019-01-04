/*
File rules.h
*/

/***************************************************************************
                          rules.h  -  description
                             -------------------
    begin                : 2003
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

#ifndef RULES_H
#define RULES_H

#include "defs.h"

#include "timeconstraint.h"
#include "activity.h"
#include "activitytag.h"
#include "studentsset.h"
#include "teacher.h"
#include "subject.h"
#include "solution.h"

class GenerationStrategy;

#include "matrix.h"

#include <QHash>
#include <QSet>
#include <QList>
#include <QStringList>
#include <QString>
#include <QObject>
#include <QCoreApplication>

#include "settings.h"

#include "json.hpp"
using json = nlohmann::json;

class QXmlStreamReader;
class QWidget;

/*
fake string, so that the output log is not too large
*/
class FakeString{
public:
	FakeString();

	void operator=(const QString& other);
	void operator=(const char* str);
	void operator+=(const QString& other);
	void operator+=(const char* str);
};

/**
This class contains all the information regarding
the institution: teachers, students, activities, constraints, etc.
*/
class Instance : public QObject {
    Q_OBJECT

public:
    GenerationStrategy* getCurrentStrategy();

    settings::GenerationSettings currentGenerationSettings;

    std::vector<GenerationStrategy*> strategyList;

    size_t strategy_index_to_use = 0;

	/**
	The name of the institution
	*/
	QString institutionName;
	
	/**
	The comments
	*/
	QString comments;

	/**
	The number of hours per day
	*/
    int nHoursPerDay;

	/**
	The number of days per week
	*/
    int nDaysPerWeek;

    int getNTimeSlots();

	/**
	The days of the week (names)
	*/
	QString daysOfTheWeek[MAX_DAYS_PER_WEEK];

	/**
	The hours of the day (names).
	*/
	QString hoursOfTheDay[MAX_HOURS_PER_DAY];

	/**
	The number of hours per week
	*/
	int nHoursPerWeek;

	/**
	The list of teachers
	*/
	TeachersList teachersList;

    TeachersList activeTeachersList;

	/**
	The list of subjects
	*/
	SubjectsList subjectsList;

    QHash<const Teacher*, QHash<Subject*, Enums::SubjectPreference>> teacher_subject_preference_map;

    /**
    The list of activity tags
    */
    ActivityTagsList activityTagsList;

	/**
	The list of students (groups and subgroups included).
	Remember that every identifier (year, group or subgroup) must be UNIQUE.
	*/
	StudentsYearsList yearsList;

    StudentsSubgroupsList directSubgroupsList;//  computed in the computeIntelnalStructure method

    //not internal
    QHash<QString, StudentsSet*> permanentStudentsHash;

	/**
	The list of activities
	*/
	ActivitiesList activitiesList;
    //For faster operation
    //not internal, based on activity id / teacher name / students set name and constraints list
    QHash<int, Activity*> activitiesPointerHash; //first is id, second is pointer to Rules::activitiesList

    QHash<int, ActivitiesList*> groupActivitiesPointerHash; //first is groupId

    /**
    Here will be only the active activities.
    */
    ActivitiesList activeActivitiesList; //  computed in the computeIntelnalStructure method
    QSet<int> inactiveActivitiesIdsSet; //  computed in the computeIntelnalStructure method

	/**
	The list of time constraints
	*/
	TimeConstraintsList timeConstraintsList;
    TimeConstraintsList validTimeConstraintsList; //subset of the previous, computed in the computeIntelnalStructure method
		
	/*
    The following variables contain redundant data and are used internally (your definition of "internally" is funny)
	*/
	////////////////////////////////////////////////////////////////////////
	StudentsGroupsList internalGroupsList;
	
	StudentsYearsList augmentedYearsList;

	Matrix1D<QList<int> > activitiesForSubjectList;
	Matrix1D<QSet<int> > activitiesForSubjectSet;

	Matrix1D<QList<int> > activitiesForActivityTagList;
	Matrix1D<QSet<int> > activitiesForActivityTagSet;

    //For faster operation
    //not internal, based on activity id / teacher name / students set name and constraints list
    QSet<ConstraintBasicCompulsoryTime*> bctSet;
    QSet<ConstraintBreakTimes*> btSet;
    QHash<int, QSet<ConstraintActivityPreferredStartingTime*> > apstHash;
    QHash<int, QSet<ConstraintMinDaysBetweenActivities*> > mdbaHash;
    QHash<QString, QSet<ConstraintTeacherNotAvailableTimes*> > tnatHash;
    QHash<QString, QSet<ConstraintStudentsSetNotAvailableTimes*> > ssnatHash;
	///////////////////////////////////////////////////////////////////////

    //students set not available, which is not considered gap, false means available, true means 100% not available
    //students set not available can only be 100% or none
    //bool subgroupNotAvailableDayHour[MAX_TOTAL_SUBGROUPS][MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    Matrix3D<bool> subgroupNotAvailableDayHour;

    //break, which is not considered gap, false means no break, true means 100% break
    //break can only be 100% or none
    //bool breakDayHour[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    Matrix2D<bool> breakDayHour;

    //teacher not available, which is not considered gap, false means available, true means 100% not available
    //teacher not available can only be 100% or none
    //bool teacherNotAvailableDayHour[MAX_TEACHERS][MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
    Matrix3D<bool> teacherNotAvailableDayHour;

	/**
	True if the rules have been initialized in some way (new or loaded).
	*/
	bool initialized;

	/**
	True if the internal structure was computed.
	*/
	bool internalStructureComputed;

	/**
	Initializes the rules (empty)
	*/
	void init();

	/**
	Internal structure initializer.
	<p>
	After any modification of the activities or students or teachers
	or constraints, you need to call this subroutine
	*/
    bool computeInternalStructure(QWidget* parent);

    int getDayFromUnifiedTime(int timeSlot);

    int getHourFromUnifiedTime(int timeSlot);

	/**
	Terminator - basically clears the memory for the constraints.
	*/
	void kill();

    Instance();

    ~Instance();
	
	void setInstitutionName(const QString& newInstitutionName);
	
	void setComments(const QString& newComments);

	/**
	Adds a new teacher
	(if not already in the list).
	Returns false/true (unsuccessful/successful).
	*/
	bool addTeacher(Teacher* teacher);

	/*when reading rules, faster*/
	bool addTeacherFast(Teacher* teacher);

	/**
	Returns the index of this teacher in the teachersList,
	or -1 for inexistent teacher.
	*/
	int searchTeacher(const QString& teacherName);

	/**
	Removes this teacher and all related activities and constraints.
	It returns false on failure. If successful, returns true.
	*/
	bool removeTeacher(const QString& teacherName);

	/**
	Modifies (renames) this teacher and takes care of all related activities and constraints.
	Returns true on success, false on failure (if not found)
	*/
	bool modifyTeacher(const QString& initialTeacherName, const QString& finalTeacherName);

	/**
	A function to sort the teachers alphabetically
	*/
	void sortTeachersAlphabetically();

	/**
	Adds a new subject
	(if not already in the list).
	Returns false/true (unsuccessful/successful).
	*/
	bool addSubject(Subject* subject);

	/*
	When reading rules, faster
	*/
	bool addSubjectFast(Subject* subject);

	/**
	Returns the index of this subject in the subjectsList,
	or -1 if not found.
	*/
	int searchSubject(const QString& subjectName);

	/**
	Removes this subject and all related activities and constraints.
	It returns false on failure.
	If successful, returns true.
	*/
	bool removeSubject(const QString& subjectName);

	/**
	Modifies (renames) this subject and takes care of all related activities and constraints.
	Returns true on success, false on failure (if not found)
	*/
	bool modifySubject(const QString& initialSubjectName, const QString& finalSubjectName);

	/**
	A function to sort the subjects alphabetically
	*/
	void sortSubjectsAlphabetically();

	/**
	Adds a new activity tag to the list of activity tags
	(if not already in the list).
	Returns false/true (unsuccessful/successful).
	*/
	bool addActivityTag(ActivityTag* activityTag);

	/*
	When reading rules, faster
	*/
	bool addActivityTagFast(ActivityTag* activityTag);

	/**
	Returns the index of this activity tag in the activityTagsList,
	or -1 if not found.
	*/
	int searchActivityTag(const QString& activityTagName);

	/**
	Removes this activity tag. In the list of activities, the activity tag will 
	be removed from all activities which posess it.
	It returns false on failure.
	If successful, returns true.
	*/
	bool removeActivityTag(const QString& activityTagName);

	/**
	Modifies (renames) this activity tag and takes care of all related activities.
	Returns true on success, false on failure (if not found)
	*/
	bool modifyActivityTag(const QString& initialActivityTagName, const QString& finalActivityTagName);

	/**
	A function to sort the activity tags alphabetically
	*/
	void sortActivityTagsAlphabetically();
	
	void computePermanentStudentsHash();

	/**
	Returns a pointer to the structure containing this student set
	(year, group or subgroup) or nullptr.
	*/
	StudentsSet* searchStudentsSet(const QString& setName);
	
	StudentsSet* searchAugmentedStudentsSet(const QString& setName);
	
	/**
	True if the students sets contain one common subgroup.
	This function is used in constraints isRelatedToStudentsSet
	*/
	bool setsShareStudents(const QString& studentsSet1, const QString& studentsSet2);

	/**
	Adds a new year of study to the academic structure
	*/
	bool addYear(StudentsYear* year);
	
	/*
	When reading rules, faster
	*/
	bool addYearFast(StudentsYear* year);

//	bool emptyYear(const QString& yearName);
	bool removeYear(const QString& yearName);
//	bool removeYear(const QString& yearName, bool removeAlsoThisYear);
	
	bool removeYearPointerAfterSplit(StudentsYear* yearPointer);

	/**
	Returns -1 if not found or the index of this year in the years list
	*/
	int searchYear(const QString& yearName);

	int searchAugmentedYear(const QString& yearName);

	/**
	Modifies this students set (name, number of students) and takes care of all related
	activities and constraints. Returns true on success, false on failure (if not found)
	*/
	bool modifyStudentsSet(const QString& initialStudentsSetName, const QString& finalStudentsSetName, int finalNumberOfStudents);
	
	bool modifyStudentsSets(const QHash<QString, QString>& oldAndNewStudentsSetNames);
	
	/**
	A function to sort the years alphabetically
	*/
	void sortYearsAlphabetically();

	/**
	Adds a new group in a certain year of study to the academic structure
	*/
	bool addGroup(const QString& yearName, StudentsGroup* group);
	
	/*
	When reading rules, faster
	*/
	bool addGroupFast(StudentsYear* year, StudentsGroup* group);

	bool removeGroup(const QString& yearName, const QString& groupName);

	//Remove this group from all the years in which it exists
	bool purgeGroup(const QString& groupName);

	/**
	Returns -1 if not found or the index of this group in the groups list
	of this year.
	*/
	int searchGroup(const QString& yearName, const QString& groupName);

	int searchAugmentedGroup(const QString& yearName, const QString& groupName);

	/**
	A function to sort the groups of this year alphabetically
	*/
	void sortGroupsAlphabetically(const QString& yearName);

	/**
	Adds a new subgroup to a certain group in a certain year of study to
	the academic structure
	*/
	bool addSubgroup(const QString& yearName, const QString& groupName, StudentsSubgroup* subgroup);

	/*
	When reading rules, faster
	*/
	bool addSubgroupFast(StudentsYear* year, StudentsGroup* group, StudentsSubgroup* subgroup);

	bool removeSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName);

	//Remove this subgroup from all the groups in which it exists
	bool purgeSubgroup(const QString& subgroupName);

	/**
	Returns -1 if not found or the index of the subgroup in the list of subgroups of this group
	*/
	int searchSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName);

	int searchAugmentedSubgroup(const QString& yearName, const QString& groupName, const QString& subgroupName);

	/**
	A function to sort the subgroups of this group alphabetically
	*/
	void sortSubgroupsAlphabetically(const QString& yearName, const QString& groupName);
	
	/**
	Adds a new indivisible activity (not split) to the list of activities.
	(It can add a subactivity of a split activity)
	Returns true if successful or false if the maximum
	number of activities was reached.
	*/
	/*
	Faster (no need to recompute the number of students in activity constructor)
	*/
    bool addSimpleActivityFast(QWidget* parent,
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

	/**
	Adds a new split activity to the list of activities.
	Returns true if successful or false if the maximum
	number of activities was reached.
	If _minDayDistance>0, there will automatically added a compulsory
	ConstraintMinDaysBetweenActivities.
	*/
    bool addSplitActivityFast(QWidget* parent,
        int _firstActivityId,
        int _activityGroupId,
        const QString& _activityName,
        bool _allowMultipleTeachers,
        const QString& _subjectName,
        const QStringList& _activityTagsNames,
        const QStringList &_studentsName,
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
        int _computedNumberOfStudents);

    bool addActivity(Activity* act);

	/**
	Removes only the activity with this id.
	*/
	//void removeActivity(int _id);

	/**
	If _activityGroupId==0, then this is a non-split activity
	(if >0, then this is a single sub-activity from a split activity.
	Removes this activity from the list of activities.
	For split activities, it removes all the sub-activities that are contained in it.
	*/
	void removeActivity(int _id, int _activityGroupId);

    void removeActivities();
	void removeActivities(const QList<int>& _idsList, bool updateConstraints);
	
	/**
	A function to modify the information of a certain activity.
	If this is a sub-activity of a split activity,
	all the sub-activities will be modified.
	*/
    void modifyActivity(int _id,
        int _activityGroupId,
        const QString& _activityName,
        bool _allowMultipleTeachers,
        const QString& _subjectName,
        const QStringList& _activityTagsNames,
        const QStringList &_studentsName,
        int _nSplits,
        int _totalDuration,
        int _durations[],
        bool _active[],
        bool _computeNTotalStudents,
        int nTotalStudents);

    void modifySubactivity(int _id,
        int _activityGroupId,
        const QString& _activityName,
        bool _allowMultipleTeachers,
        const QString& _subjectName,
        const QStringList& _activityTagsNames,
        const QStringList &_studentsName,
        int _duration,
        bool _active,
        bool _computeNTotalStudents,
        int nTotalStudents);

	/**
	Adds a new time constraint (already allocated).
	Returns true on success, false for already existing constraints.
	*/
	bool addTimeConstraint(TimeConstraint* ctr);

	/**
	Removes this time constraint.
	Returns true on success, false on failure (not found).
	*/
	bool removeTimeConstraint(TimeConstraint* ctr);
	
	bool removeTimeConstraints(QList<TimeConstraint*> _tcl);

	int xmlReaderNumberOfUnrecognizedFields;

	/**
	Reads the rules from the xml input file "filename".
	Returns true on success, false on failure (inexistent file or wrong format)
	*/
	bool read(QWidget* parent, const QString& fileName, bool commandLine=false, QString commandLineDirectory=QString());

	/**
	Write the rules to the xml input file "inputfile".
	*/
	bool write(QWidget* parent, const QString& filename);
		
	int activateStudents(const QString& studentsName);
	
	int activateSubject(const QString& subjectName);
	
	int activateActivityTag(const QString& activityTagName);
	
	int deactivateStudents(const QString& studentsName);
	
	int deactivateSubject(const QString& subjectName);
	
	int deactivateActivityTag(const QString& activityTagName);
	
	void makeActivityTagPrintable(const QString& activityTagName);
	void makeActivityTagNotPrintable(const QString& activityTagName);
	
	void updateActivitiesWhenRemovingStudents(const QSet<StudentsSet*>& studentsSets, bool updateConstraints);
	void updateConstraintsAfterRemoval();

    int getActiveTeacherIndex(const Teacher& teacher);

    int getTeacherIndex(const QString& teacherName);
    int getSubjectIndex(const QString& subjectName);
    int getActivityTagIndex(const QString& activityTagName);
    StudentsSet* getStudentsSet(const QString& studentsSetName);
    int getActivityIndex(int activityId);

    bool containsActivityInActiveList(int id);

    QString getDetailedDescription();

    bool isModified() const;
    void setModified(bool value);

    QString getActivityDetailedDescription(int activityId);

    int getConstraintGroupWeight(Enums::ConstraintGroup g);

    int getSubjectPrefferenceWeight();

    int getSubjectPrefferenceFactor(Enums::SubjectPreference pref);

    std::vector<Move*> allMovesN1;
    std::vector<Move*> allMovesN2;
    std::vector<Move*> allMovesN3;
    std::vector<Move*> allMovesN4;
    std::vector<Move*> allMovesN5;

    QByteArray hashCode();

signals:
    void gotModified();

private:
    bool modified;

    std::vector<Move*> _allMovesN1();
    std::vector<Move*> _allMovesN2();
    std::vector<Move*> _allMovesN3();
    std::vector<Move*> _allMovesN4();
    std::vector<Move*> _allMovesN5();

    //internal
    QHash<QString, int> teachersHash;
    QHash<QString, int> subjectsHash;
    QHash<QString, int> activityTagsHash;
    QHash<QString, StudentsSet*> studentsHash;
    QHash<int, int> activeActivitiesHash; //first is id, second is index in internal list

    void renameAllActivitiesForSubject(const QString& inputedSubjectName);

    TimeConstraint* readBasicCompulsoryTime(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherNotAvailable(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherNotAvailableTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMaxDaysPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxDaysPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherMinDaysPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMinDaysPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsIntervalMaxDaysPerWeek(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetNotAvailable(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetNotAvailableTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readMinNDaysBetweenActivities(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readMinDaysBetweenActivities(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readMaxDaysBetweenActivities(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readMinGapsBetweenActivities(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesNotOverlapping(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesSameStartingTime(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesSameStartingHour(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesSameStartingDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
    TimeConstraint* readNoSubjectPreferenceForAllTeachers(QXmlStreamReader& xml, FakeString& xmlReadingLog);

    TimeConstraint* readTeachersMaxHoursInWork(QXmlStreamReader& xml, FakeString& xmlReadingLog);
    TimeConstraint* readTeacherMaxHoursInWork(QXmlStreamReader& xml, FakeString& xmlReadingLog);

    TimeConstraint* readMaxActivitiesForAllTeachers(QXmlStreamReader& xml, FakeString& xmlReadingLog);
    TimeConstraint* readMaxActivitiesForATeacher(QXmlStreamReader& xml, FakeString& xmlReadingLog);

    TimeConstraint* readMinActivitiesForAllTeachers(QXmlStreamReader& xml, FakeString& xmlReadingLog);
    TimeConstraint* readMinActivitiesForATeacher(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherActivityTagMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersActivityTagMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherActivityTagMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersActivityTagMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeachersMinHoursDaily(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMinHoursDaily(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetActivityTagMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsActivityTagMaxHoursContinuously(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readStudentsSetActivityTagMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsActivityTagMaxHoursDaily(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readStudentsMinHoursDaily(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMinHoursDaily(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readActivityPreferredTime(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog,
		bool& reportUnspecifiedPermanentlyLockedTime, bool& reportUnspecifiedDayOrHourPreferredStartingTime);
	TimeConstraint* readActivityPreferredStartingTime(QWidget* parent, QXmlStreamReader& xml, FakeString& xmlReadingLog,
		bool& reportUnspecifiedPermanentlyLockedTime, bool& reportUnspecifiedDayOrHourPreferredStartingTime);

	TimeConstraint* readActivityEndsStudentsDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesEndStudentsDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	
	/*old, with 2 and 3*/
	TimeConstraint* read2ActivitiesConsecutive(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* read2ActivitiesGrouped(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* read3ActivitiesGrouped(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* read2ActivitiesOrdered(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	/*end old*/
	
	TimeConstraint* readTwoActivitiesConsecutive(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTwoActivitiesGrouped(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readThreeActivitiesGrouped(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTwoActivitiesOrdered(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	
	TimeConstraint* readActivityPreferredTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivityPreferredTimeSlots(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivityPreferredStartingTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	
	TimeConstraint* readBreak(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readBreakTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	
	TimeConstraint* readTeachersNoGaps(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxGapsPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMaxGapsPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxGapsPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeacherMaxGapsPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	
	TimeConstraint* readStudentsNoGaps(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetNoGaps(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMaxGapsPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxGapsPerWeek(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readStudentsMaxGapsPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxGapsPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readStudentsEarly(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsEarlyMaxBeginningsAtSecondHour(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetEarly(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetEarlyMaxBeginningsAtSecondHour(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readActivitiesPreferredTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesPreferredTimeSlots(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesPreferredStartingTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readSubactivitiesPreferredTimeSlots(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readSubactivitiesPreferredStartingTimes(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readActivitiesOccupyMaxTimeSlotsFromSelection(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readActivitiesMaxSimultaneousInSelectedTimeSlots(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherMaxSpanPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMaxSpanPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMaxSpanPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMaxSpanPerDay(QXmlStreamReader& xml, FakeString& xmlReadingLog);

	TimeConstraint* readTeacherMinRestingHours(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readTeachersMinRestingHours(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsSetMinRestingHours(QXmlStreamReader& xml, FakeString& xmlReadingLog);
	TimeConstraint* readStudentsMinRestingHours(QXmlStreamReader& xml, FakeString& xmlReadingLog);

    friend void to_json(json& j, const Instance& s);
    friend void from_json(const json& j, Instance& p);
};

void to_json(json& j, const Instance& s);
void from_json(const json& j, Instance& p);

#endif

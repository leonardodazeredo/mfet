//
//
// Description: This file is part of m-FET
//
//
// Author: Liviu Lalescu <Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)>
// Copyright (C) 2003 Liviu Lalescu <http://lalescu.ro/liviu/>
//
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef STUDENTSSET_H
#define STUDENTSSET_H

#include <QCoreApplication>

#include "defs.h"

#include <QList>
#include <QSet>

#include "json.hpp"
using json = nlohmann::json;

const int STUDENTS_SET=0;
const int STUDENTS_YEAR=1;
const int STUDENTS_GROUP=2;
const int STUDENTS_SUBGROUP=3;

class StudentsYear;
class StudentsGroup;
class StudentsSubgroup;

class Instance;

typedef QList<StudentsYear*> StudentsYearsList;

typedef QList<StudentsGroup*> StudentsGroupsList;

typedef QList<StudentsSubgroup*> StudentsSubgroupsList;

/**
This class represents a set of students, for instance years, groups or subgroups.

@author Liviu Lalescu
*/
class StudentsSet
{
	Q_DECLARE_TR_FUNCTIONS(StudentsSet)

public:
	QString name;
	int numberOfStudents;
	int type;
	
	QString comments;

	StudentsSet();
	virtual ~StudentsSet();

    friend void to_json(json& j, const StudentsSet& s);
    friend void from_json(const json& j, StudentsSet& p);
};

void to_json(json& j, const StudentsSet& s);
void from_json(const json& j, StudentsSet& p);

class StudentsYear: public StudentsSet
{
	Q_DECLARE_TR_FUNCTIONS(StudentsYear)
	
public:
	StudentsGroupsList groupsList;

	int indexInAugmentedYearsList; //internal

	StudentsYear();
	~StudentsYear();

	QString getXmlDescription();
	QString getDescription();
	QString getDetailedDescription();
	QString getDetailedDescriptionWithConstraints(Instance& r);

    friend void to_json(json& j, const StudentsYear& s);
    friend void from_json(const json& j, StudentsYear& p);
};

void to_json(json& j, const StudentsYear& s);
void from_json(const json& j, StudentsYear& p);

class StudentsGroup: public StudentsSet
{
	Q_DECLARE_TR_FUNCTIONS(StudentsGroup)

public:
	StudentsSubgroupsList subgroupsList;

	int indexInInternalGroupsList; //internal
	
	StudentsGroup();
	~StudentsGroup();

	QString getXmlDescription();
	QString getDescription();
	QString getDetailedDescription();
	QString getDetailedDescriptionWithConstraints(Instance& r);

    friend void to_json(json& j, const StudentsGroup& s);
    friend void from_json(const json& j, StudentsGroup& p);
};

void to_json(json& j, const StudentsGroup& s);
void from_json(const json& j, StudentsGroup& p);

class StudentsSubgroup: public StudentsSet
{
	Q_DECLARE_TR_FUNCTIONS(StudentsSubgroup)

public:
	int indexInInternalSubgroupsList;
	
	QList<int> activitiesForSubgroup;

    QSet<int> notAvailableTimeSlots;

	StudentsSubgroup();
	~StudentsSubgroup();

	QString getXmlDescription();
	QString getDescription();
	QString getDetailedDescription();
	QString getDetailedDescriptionWithConstraints(Instance& r);

    friend void to_json(json& j, const StudentsSubgroup& s);
    friend void from_json(const json& j, StudentsSubgroup& p);
};

void to_json(json& j, const StudentsSubgroup& s);
void from_json(const json& j, StudentsSubgroup& p);

int yearsAscending(const StudentsYear* y1, const StudentsYear* y2);

int groupsAscending(const StudentsGroup* g1, const StudentsGroup* g2);

int subgroupsAscending(const StudentsSubgroup* s1, const StudentsSubgroup* s2);

#endif

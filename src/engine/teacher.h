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

#ifndef TEACHER_H
#define TEACHER_H

#include <QCoreApplication>

#include "defs.h"

#include <QString>
#include <QList>
#include <QLinkedList>
#include <QHash>
#include <QSet>

#include "json.hpp"
using json = nlohmann::json;

class Teacher;
class Instance;
class Subject;

typedef QList<Teacher*> TeachersList;

/**
@author Liviu Lalescu
*/
class Teacher
{
	Q_DECLARE_TR_FUNCTIONS(Teacher)
	
public:
	QString name;
	
	QString comments;

    bool active = true;
	
	int targetNumberOfHours;

    QSet<Subject*> preferredSubjects;

    QSet<Subject*> restrictedSubjects;

    QSet<Subject*> indifferentSubjects;
	
    QSet<int> notAvailableTimeSlots;

	Teacher();
	~Teacher();

	QString getXmlDescription();
	QString getDescription();
	QString getDetailedDescription();
	QString getDetailedDescriptionWithConstraints(Instance& r);

    friend void to_json(json& j, const Teacher& s);
    friend void from_json(const json& j, Teacher& p);
};

int teachersAscending(const Teacher* t1, const Teacher* t2);

void to_json(json& j, const Teacher& s);
void from_json(const json& j, Teacher& p);

#endif

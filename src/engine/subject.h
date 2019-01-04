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

#ifndef SUBJECT_H
#define SUBJECT_H

#include <QCoreApplication>

#include "defs.h"

#include <QSet>
#include <QString>

#include "json.hpp"
using json = nlohmann::json;

class Subject;
class Instance;
class Teacher;

typedef QList<Subject*> SubjectsList;

/**
This class represents a subject

@author Liviu Lalescu
*/
class Subject{
	Q_DECLARE_TR_FUNCTIONS(Subject)
	
public:
	QString name;
	
	QString comments;

    QSet<Teacher*> preferredTeachers;

    QSet<Teacher*> restrictedTeachers;

    QSet<Teacher*> indifferentTeachers;

	Subject();
	~Subject();

	QString getXmlDescription();
	QString getDescription();
	QString getDetailedDescription();
	QString getDetailedDescriptionWithConstraints(Instance& r);

    friend void to_json(json& j, const Subject& s);
    friend void from_json(const json& j, Subject& p);
};

int subjectsAscending(const Subject* s1, const Subject* s2);

void to_json(json& j, const Subject& s);
void from_json(const json& j, Subject& p);

#endif

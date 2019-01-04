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

#include "subject.h"
#include "instance.h"
#include "stringutils.h"

Subject::Subject()
{
	comments=QString("");
    preferredTeachers.clear();
    restrictedTeachers.clear();
    indifferentTeachers.clear();
}

Subject::~Subject()
{
}

QString Subject::getXmlDescription()
{
	QString s="<Subject>\n";
	s+="	<Name>"+utils::strings::parseStrForXml(this->name)+"</Name>\n";
	s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
	s+="</Subject>\n";

	return s;
}

QString Subject::getDescription()
{
	QString s=tr("N:%1", "The name of the subject").arg(name);
	
	QString end=QString("");
	if(!comments.isEmpty())
		end=", "+tr("C: %1", "Comments").arg(comments);
	
	return s+end;
}

QString Subject::getDetailedDescription()
{
	QString s=tr("Subject");
	s+="\n";
	s+=tr("Name=%1", "The name of the subject").arg(this->name);
	s+="\n";

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString Subject::getDetailedDescriptionWithConstraints(Instance& r)
{
	QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this subject:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
		if(c->isRelatedToSubject(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

	return s;
}

int subjectsAscending(const Subject* s1, const Subject* s2)
{
	return s1->name < s2->name;
}

void to_json(json &j, const Subject &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, Subject &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

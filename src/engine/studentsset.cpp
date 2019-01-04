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

#include "studentsset.h"
#include "instance.h"
#include "tcontext.h"

#include "stringutils.h"



StudentsSet::StudentsSet()
{
	this->type=STUDENTS_SET;
	this->numberOfStudents=0;
	comments=QString("");
}

StudentsSet::~StudentsSet()
{
}

StudentsYear::StudentsYear()
	: StudentsSet()
{
	this->type=STUDENTS_YEAR;

	indexInAugmentedYearsList=-1;
}

StudentsYear::~StudentsYear()
{
	//it is possible that the removed group to be in another year

	/*while(!groupsList.isEmpty()){
		StudentsGroup* g=groupsList[0];
		
		foreach(StudentsYear* year, Timetable::getInstance()->rules.yearsList)
			if(year!=this)
				for(int i=0; i<year->groupsList.size(); i++)
					if(year->groupsList[i]==g)
						year->groupsList[i]=nullptr;
	
		if(g!=nullptr){
			delete groupsList.takeFirst();
		}
		else
			groupsList.removeFirst();
	}*/
}

QString StudentsYear::getXmlDescription()
{
	QString s="";
	s+="<Year>\n";
	s+="	<Name>"+utils::strings::parseStrForXml(this->name)+"</Name>\n";
    s+="	<Number_of_Students>"+utils::strings::number(this->numberOfStudents)+"</Number_of_Students>\n";
	s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
	for(int i=0; i<this->groupsList.size(); i++){
		StudentsGroup* stg=this->groupsList[i];
		s+=stg->getXmlDescription();
	}
	s+="</Year>\n";

	return s;
}

QString StudentsYear::getDescription()
{
	QString s;
	s+=tr("YN:%1", "Year name").arg(this->name);
	s+=", ";
	s+=tr("NoS:%1", "Number of students").arg(this->numberOfStudents);
	
	QString end=QString("");
	if(!comments.isEmpty())
		end=QString(", ")+tr("C: %1", "Comments").arg(comments);

	return s+end;
}

QString StudentsYear::getDetailedDescription()
{
	QString s="";
	s+=tr("Students set - year");
	s+="\n";
	s+=tr("Year name=%1").arg(this->name);
	s+="\n";
	s+=tr("Number of students=%1").arg(this->numberOfStudents);
	s+="\n";

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString StudentsYear::getDetailedDescriptionWithConstraints(Instance& r)
{
	QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this students year:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
        if(c->isRelatedToStudentsSet(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

	return s;
}


StudentsGroup::StudentsGroup()
	: StudentsSet()
{
	this->type=STUDENTS_GROUP;

	indexInInternalGroupsList=-1;
}

StudentsGroup::~StudentsGroup()
{
	/*while(!subgroupsList.isEmpty()){
		StudentsSubgroup* s=subgroupsList[0];
		
		foreach(StudentsYear* year, Timetable::getInstance()->rules.yearsList)
			foreach(StudentsGroup* group, year->groupsList)
				if(group!=this)
					for(int i=0; i<group->subgroupsList.size(); i++)
						if(group->subgroupsList[i]==s){
							cout<<"making nullptr group->subgroupsList[i]->name=="<<qPrintable(group->subgroupsList[i]->name)<<endl;
							group->subgroupsList[i]=nullptr;
						}
						else
							cout<<"ignoring group->subgroupsList[i]->name=="<<qPrintable(group->subgroupsList[i]->name)<<endl;
	
		if(s!=nullptr){
			assert(subgroupsList[0]!=nullptr);
			delete subgroupsList.takeFirst();
		}
		else
			subgroupsList.removeFirst();
	}*/
}

QString StudentsGroup::getXmlDescription()
{
	QString s="";
	s+="	<Group>\n";
	s+="		<Name>"+utils::strings::parseStrForXml(this->name)+"</Name>\n";
    s+="		<Number_of_Students>"+utils::strings::number(this->numberOfStudents)+"</Number_of_Students>\n";
	s+="		<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
	for(int i=0; i<this->subgroupsList.size(); i++){
		StudentsSubgroup* sts=this->subgroupsList[i];
		s+=sts->getXmlDescription();
	}
	s+="	</Group>\n";

	return s;
}

QString StudentsGroup::getDescription()
{
	QString s="";
	s+=tr("GN:%1", "Group name").arg(this->name);
	s+=", ";
	s+=tr("NoS:%1", "Number of students").arg(this->numberOfStudents);

	QString end=QString("");
	if(!comments.isEmpty())
		end=QString(", ")+tr("C: %1", "Comments").arg(comments);

	return s+end;
}

QString StudentsGroup::getDetailedDescription()
{
	QString s="";
	s+=tr("Students set - group");
	s+="\n";
	s+=tr("Group name=%1").arg(this->name);
	s+="\n";
	s+=tr("Number of students=%1").arg(this->numberOfStudents);
	s+="\n";

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString StudentsGroup::getDetailedDescriptionWithConstraints(Instance& r)
{
	QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this students group:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
        if(c->isRelatedToStudentsSet(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

	return s;
}


StudentsSubgroup::StudentsSubgroup()
	: StudentsSet()
{
	this->type=STUDENTS_SUBGROUP;
	
	indexInInternalSubgroupsList=-1;
}

StudentsSubgroup::~StudentsSubgroup()
{
}

QString StudentsSubgroup::getXmlDescription()
{
	QString s="";
	s+="		<Subgroup>\n";
	s+="			<Name>"+utils::strings::parseStrForXml(this->name)+"</Name>\n";
    s+="			<Number_of_Students>"+utils::strings::number(this->numberOfStudents)+"</Number_of_Students>\n";
	s+="			<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
	s+="		</Subgroup>\n";

	return s;
}

QString StudentsSubgroup::getDescription()
{
	QString s="";
	s+=tr("SgN:%1", "Subgroup name").arg(this->name);
	s+=", ";
	s+=tr("NoS:%1", "Number of students").arg(this->numberOfStudents);

	QString end=QString("");
	if(!comments.isEmpty())
		end=QString(", ")+tr("C: %1", "Comments").arg(comments);

	return s+end;
}

QString StudentsSubgroup::getDetailedDescription()
{
	QString s="";
	s+=tr("Students set - subgroup");
	s+="\n";
	s+=tr("Subgroup name=%1").arg(this->name);
	s+="\n";
	s+=tr("Number of students=%1").arg(this->numberOfStudents);
	s+="\n";

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString StudentsSubgroup::getDetailedDescriptionWithConstraints(Instance& r)
{
	QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this students subgroup:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
        if(c->isRelatedToStudentsSet(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

	return s;
}

int yearsAscending(const StudentsYear* y1, const StudentsYear* y2)
{
	return y1->name < y2->name;
}

int groupsAscending(const StudentsGroup* g1, const StudentsGroup* g2)
{
	return g1->name < g2->name;
}

int subgroupsAscending(const StudentsSubgroup* s1, const StudentsSubgroup* s2)
{
	return s1->name < s2->name;
}

void to_json(json &j, const StudentsSubgroup &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, StudentsSubgroup &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

void to_json(json &j, const StudentsGroup &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, StudentsGroup &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

void from_json(const json &j, StudentsYear &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

void to_json(json &j, const StudentsYear &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void to_json(json &j, const StudentsSet &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, StudentsSet &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

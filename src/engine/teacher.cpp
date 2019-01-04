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

#include "teacher.h"
#include "instance.h"
#include "enumutils.h"
#include "tcontext.h"

#include "stringutils.h"



Teacher::Teacher()
{
	targetNumberOfHours=0;
	comments=QString("");
    preferredSubjects.clear();
    restrictedSubjects.clear();
    indifferentSubjects.clear();
}

Teacher::~Teacher()
{
}

QString Teacher::getXmlDescription()
{
    QString s="<Teacher>\n";
    s+="	<Name>"+utils::strings::parseStrForXml(this->name)+"</Name>\n";

    s+="	<Active>";
    if(this->active==true)
        s+="true";
    else
        s+="false";
    s+="</Active>\n";

    s+="	<Target_Number_of_Hours>"+utils::strings::number(targetNumberOfHours)+"</Target_Number_of_Hours>\n";
    s+="	<Subjects_Preferences>\n";
    for(auto sbj: TContext::get()->instance.subjectsList){
        assert(TContext::get()->instance.teacher_subject_preference_map[this].contains(sbj));

        s+="		<Subject>"+utils::strings::parseStrForXml(sbj->name)+"</Subject>\n";

        Enums::SubjectPreference pref = Enums::SubjectPreference::Indifferent;
        pref = TContext::get()->instance.teacher_subject_preference_map[this][sbj];

        s+="		<Preference>"+utils::enums::enumIndexToStr(pref)+"</Preference>\n";
    }
    s+="	</Subjects_Preferences>\n";
    s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";
    s+="</Teacher>\n";

	return s;
}

QString Teacher::getDescription()
{
	QString s=tr("N:%1", "The name of the teacher").arg(name);

    s+=tr("Ac:%1").arg(this->active);
	
	QString end=QString("");
	if(!comments.isEmpty())
		end=", "+tr("C: %1", "Comments").arg(comments);
	
	return s+end;
}

QString Teacher::getDetailedDescription()
{
	QString s=tr("Teacher");
	s+="\n";
	s+=tr("Name=%1", "The name of the teacher").arg(this->name);
	s+="\n";

    s+=tr("Active:%1").arg(this->active);
    s+="\n";
	
	s+=tr("Target number of hours=%1", "The target number of hours for the teacher").arg(targetNumberOfHours);
	s+="\n";

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString Teacher::getDetailedDescriptionWithConstraints(Instance& r)
{
	QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this teacher:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
		if(c->isRelatedToTeacher(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

	return s;
}

int teachersAscending(const Teacher* t1, const Teacher* t2)
{
	return t1->name < t2->name;
}

void to_json(json &j, const Teacher &s)
{
    j = json();

//    std::vector<QString> preferences;
//    for(auto sbj: Timetable::getInstance()->instance.subjectsList){
//        preferences.push_back(utils::strings::parseStrForXml(sbj->name));
//        Enums::SubjectPreference pref = Enums::SubjectPreference::Indifferent;
//        pref = Timetable::getInstance()->instance.teacher_subject_preference_map[&s][sbj];
//        preferences.push_back(utils::enums::enumIndexToStr(pref));
//    }

//    j["name"] = utils::strings::parseStrForXml(s.name);
    j["active"] = s.active;
//    j["subjects_preferences"] = preferences;
//    j["comments"] = s.comments;
}

void from_json(const json &j, Teacher &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

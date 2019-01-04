/*
File activity.cpp
*/

/***************************************************************************
                          activity.cpp  -  description
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

#include "activity.h"

#include "instance.h"

#include "stringutils.h"

Activity::Activity()
{
	comments=QString("");
}

Activity::Activity(const Activity &act)
{
    comments=QString("");

    this->id = act.id;
    this->activityGroupId = act.activityGroupId;
    this->activityName = act.activityName;
    this->allowMultipleTeachers = act.allowMultipleTeachers;
    this->subjectName = act.subjectName;
    this->activityTagsNames = act.activityTagsNames;
    this->studentSetsNames = act.studentSetsNames;
    this->duration=act.duration;
    this->totalDuration=act.totalDuration;
    this->active=act.active;
    this->computeNTotalStudents=act.computeNTotalStudents;
    this->nTotalStudents=act.nTotalStudents;
}

Activity::Activity(Instance& r,
    int _id,
    int _activityGroupId,
    const QString& _activityName,
    bool _allowMultipleTeachers,
    const QString& _subjectName,
    const QStringList& _activityTagsNames,
    const QStringList& _studentsName,
    int _duration,
    int _totalDuration,
    bool _active,
    bool _computeNTotalStudents,
    int _nTotalStudents,
    int _computedNumberOfStudents)
{
	Q_UNUSED(r);
	//Q_UNUSED(_nTotalStudents);

	comments=QString("");

    this->id = _id;
    this->activityGroupId = _activityGroupId;
    this->activityName = _activityName;
    this->allowMultipleTeachers = _allowMultipleTeachers;
	this->subjectName = _subjectName;
	this->activityTagsNames = _activityTagsNames;
    this->studentSetsNames = _studentsName;
	this->duration=_duration;
	this->totalDuration=_totalDuration;
	this->active=_active;
	this->computeNTotalStudents=_computeNTotalStudents;
	
	//assert(_computeNTotalStudents);
	if(_computeNTotalStudents)
		assert(_nTotalStudents==-1);
	else
		assert(_nTotalStudents==_computedNumberOfStudents);
	this->nTotalStudents=_computedNumberOfStudents;
}

bool Activity::operator==(Activity& a)
{
//    if(this->activityName != a.activityName)
//        return false;
	if(this->subjectName != a.subjectName)
		return false;
	if(this->activityTagsNames != a.activityTagsNames)
		return false;
    if(this->studentSetsNames != a.studentSetsNames)
		return false;
	//if(this->duration != a.duration)
	  //  return false;
	if((this->activityGroupId==0 && a.activityGroupId!=0) || (this->activityGroupId!=0 && a.activityGroupId==0))
		return false;
    return true;
}

bool Activity::searchStudents(const QString& studentsName)
{
    return this->studentSetsNames.indexOf(studentsName)!=-1;
}

bool Activity::removeStudents(Instance& r, const QString& studentsName, int nStudents)
{
    Q_UNUSED(r);

    int t=this->studentSetsNames.removeAll(studentsName);

    assert(t<=1);

    if(t>0 && this->computeNTotalStudents==true){
        this->nTotalStudents-=t*nStudents;
        assert(this->nTotalStudents>=0);
    }

    return t>0;
}

void Activity::renameStudents(Instance& r, const QString& initialStudentsName, const QString& finalStudentsName, int initialNumberOfStudents, int finalNumberOfStudents)
{
    Q_UNUSED(r);

    int t=0;
    for(QStringList::iterator it=this->studentSetsNames.begin(); it!=this->studentSetsNames.end(); it++)
        if((*it)==initialStudentsName){
            *it=finalStudentsName;
            t++;

            if(this->computeNTotalStudents){
                assert(initialNumberOfStudents>=0);
                assert(finalNumberOfStudents>=0);

                nTotalStudents-=initialNumberOfStudents;
                assert(nTotalStudents>=0);
                nTotalStudents+=finalNumberOfStudents;
            }
        }
    assert(t<=1);
}

void Activity::computeInternalStructure(Instance& r)
{
	//the internal subgroups list must be computed before entering here.

	//subjects
    this->subjectIndex = r.getSubjectIndex(subjectName); //r.searchSubject(this->subjectName);
	assert(this->subjectIndex>=0);

	//activity tags
	this->iActivityTagsSet.clear();
	foreach(QString tag, this->activityTagsNames){
		assert(tag!="");
        int index=r.getActivityTagIndex(tag); //r.searchActivityTag(tag);
		assert(index>=0);
		if(!iActivityTagsSet.contains(index))
			this->iActivityTagsSet.insert(index);
	}

	//students	
    this->iSubgroupsList.clear();
    QSet<int> iSubgroupsSet;

    for(QStringList::Iterator it=this->studentSetsNames.begin(); it!=this->studentSetsNames.end(); it++){
        StudentsSet* ss=r.getStudentsSet(*it); //r.searchAugmentedStudentsSet(*it);
        assert(ss);

        if(ss->type==STUDENTS_SUBGROUP){
            int tmp = ((StudentsSubgroup*)ss)->indexInInternalSubgroupsList;
            assert(tmp>=0);
            assert(tmp<r.directSubgroupsList.size());

            bool duplicate=false;
            if(iSubgroupsSet.contains(tmp))
                duplicate=true;
            if(duplicate){
                assert(0);
            }
            else{
                iSubgroupsSet.insert(tmp);
                this->iSubgroupsList.append(tmp);
            }
        }
        else if(ss->type==STUDENTS_GROUP){
            StudentsGroup* stg=(StudentsGroup*)ss;
            for(int k=0; k<stg->subgroupsList.size(); k++){
                StudentsSubgroup* sts=stg->subgroupsList[k];

                int tmp=sts->indexInInternalSubgroupsList;
                assert(tmp>=0);
                assert(tmp<r.directSubgroupsList.size());

                bool duplicate=false;
                if(iSubgroupsSet.contains(tmp))
                    duplicate=true;
                if(duplicate){
                    assert(0);
                }
                else{
                    iSubgroupsSet.insert(tmp);
                    this->iSubgroupsList.append(tmp);
                }
            }
        }
        else if(ss->type==STUDENTS_YEAR){
            StudentsYear* sty=(StudentsYear*)ss;
            for(int k=0; k<sty->groupsList.size(); k++){
                StudentsGroup* stg=sty->groupsList[k];
                for(int l=0; l<stg->subgroupsList.size(); l++){
                    StudentsSubgroup* sts=stg->subgroupsList[l];

                    int tmp=sts->indexInInternalSubgroupsList;
                    assert(tmp>=0);
                    assert(tmp<r.directSubgroupsList.size());

                    bool duplicate=false;
                    if(iSubgroupsSet.contains(tmp))
                        duplicate=true;
                    if(duplicate){
                        assert(0);
                    }
                    else{
                        iSubgroupsSet.insert(tmp);
                        this->iSubgroupsList.append(tmp);
                    }
                }
            }
        }
        else{
            assert(0);
        }
    }
}

QString Activity::getXmlDescription()
{
	QString s="<Activity>\n";

    s+="	<Name>" + utils::strings::parseStrForXml(this->activityName) + "</Name>\n";

    s+="	<Allow_Multiple_Teachers>"+utils::strings::number(this->allowMultipleTeachers)+"</Allow_Multiple_Teachers>\n";

	s+="	<Subject>" + utils::strings::parseStrForXml(this->subjectName) + "</Subject>\n";

	foreach(QString tag, this->activityTagsNames)
		s+="	<Activity_Tag>" + utils::strings::parseStrForXml(tag) + "</Activity_Tag>\n";

    for(QStringList::Iterator it=this->studentSetsNames.begin(); it!=this->studentSetsNames.end(); it++)
        s+="	<Students>" + utils::strings::parseStrForXml(*it) + "</Students>\n";

    s+="	<Duration>"+utils::strings::number(this->duration)+"</Duration>\n";
    s+="	<Total_Duration>"+utils::strings::number(this->totalDuration)+"</Total_Duration>\n";
    s+="	<Id>"+utils::strings::number(this->id)+"</Id>\n";
    s+="	<Activity_Group_Id>"+utils::strings::number(this->activityGroupId)+"</Activity_Group_Id>\n";

	if(this->computeNTotalStudents==false)
        s+="	<Number_Of_Students>"+utils::strings::number(this->nTotalStudents)+"</Number_Of_Students>\n";

	s+="	<Active>";
	if(this->active==true)
		s+="true";
	else
		s+="false";
	s+="</Active>\n";

	s+="	<Comments>"+utils::strings::parseStrForXml(comments)+"</Comments>\n";

	s+="</Activity>";

	return s;
}

QString Activity::getDescription()
{
    const int INDENT=2;
	
//	bool _indent;
//	if(this->isSplit() && this->id!=this->activityGroupId)
//		_indent=true;
//	else
//		_indent=false;
		
//	bool indentRepr;
//	if(this->isSplit() && this->id==this->activityGroupId)
//		indentRepr=true;
//	else
//		indentRepr=false;

    QString _name=this->activityName;

    //TODO:
//    QString _teachers="";
//    if(teacherName.size()==0)
//        _teachers=tr("no teachers");
//    else
//        _teachers=this->teacherName;

	QString _subject=this->subjectName;
	
	QString _activityTags=this->activityTagsNames.join(",");

	QString _students="";
    if(studentSetsNames.count()==0)
		_students=tr("no students");
	else
        _students=this->studentSetsNames.join(",");

	QString _id;
    _id = utils::strings::toIntegerString(id, 3);

	QString _agid="";
	if(this->isSplit())
        _agid = utils::strings::toIntegerString(this->activityGroupId, 2);

    QString _duration=utils::strings::toIntegerString(this->duration, 2);
	
	QString _totalDuration="";
	if(this->isSplit())
        _totalDuration = utils::strings::toIntegerString(this->totalDuration, 2);

	QString _active;
	if(this->active==true)
		_active="";
	else
		_active="X";

	QString _nstudents="";
	if(this->computeNTotalStudents==false)
        _nstudents=utils::strings::toIntegerString(this->nTotalStudents, 4);

	/////////
	QString s="";
//	if(_indent)
//		s+=QString(INDENT, ' ');
		
	s+=_id;
	s+=" - ";

//	if(indentRepr)
		s+=QString(INDENT, ' ');
		
	if(_active!=""){
		s+=_active;
		s+=" - ";
	}
	
	s+=_duration;
	if(this->isSplit()){
		s+="/";
		s+=_totalDuration;
	}
	s+=" - ";
	
    s+=_name;
    s+=" - ";
	s+=_subject;
	s+=" - ";
	if(_activityTags!=""){
		s+=_activityTags;
		s+=" - ";
	}
	s+=_students;

	if(_nstudents!=""){
		s+=" - ";
		s+=_nstudents;
	}
//TODO:
//    s+=" - ";
//    s+=_teachers;
	
	if(!comments.isEmpty()){
		s+=" - ";
		s+=comments;
	}

	return s;
}

QString Activity::getDetailedDescription()
{
	QString s;

	s=tr("Activity:");
	s+="\n";

    s += tr("Name=%1").arg(this->activityName);
    s+="\n";

    s+=tr("Allow multiple teachers=%1").arg(utils::strings::number(this->allowMultipleTeachers));
    s+="\n";

	//Id, AGId
    s += tr("Id=%1").arg(utils::strings::number(id));
	s+="\n";
	if(this->isSplit()){
        s += tr("Activity group id=%1").arg(utils::strings::number(this->activityGroupId));
		s+="\n";
	}

	//Dur, TD
    s+=tr("Duration=%1").arg(utils::strings::number(this->duration));
	s+="\n";
	if(this->isSplit()){
        s += tr("Total duration=%1").arg(utils::strings::number(this->totalDuration));
		s+="\n";
	}

	s+=tr("Subject=%1").arg(this->subjectName);
	s+="\n";
	foreach(QString tag, this->activityTagsNames){
		assert(tag!="");
		s+=tr("Activity tag=%1").arg(tag);
		s+="\n";
	}

    if(studentSetsNames.size()==0){
        s+=tr("No students sets for this activity");
        s+="\n";
    }
    else{
        for(QStringList::Iterator it=this->studentSetsNames.begin(); it!=this->studentSetsNames.end(); it++){
            s += tr("Students=%1").arg(*it);
            s+="\n";
        }
    }
		
	if(this->computeNTotalStudents==true){
		/*int nStud=0;
		for(QStringList::Iterator it=this->studentsNames.begin(); it!=this->studentsNames.end(); it++){
			StudentsSet* ss=r.searchStudentsSet(*it);
			nStud += ss->numberOfStudents;
		}*/
		int nStud=this->nTotalStudents;
		s+=tr("Total number of students=%1").arg(nStud);
		s+="\n";
	}
	else{
		s+=tr("Total number of students=%1").arg(this->nTotalStudents);
		s+=" ("+tr("specified", "Specified means that the total number of students was specified separately for the activity")+")";
		s+="\n";
	}
	
    //Not active?
    if(!active){
        s+=tr("Active=%1", "Represents a boolean value, if activity is active or not, %1 is yes or no")
                .arg(this->active==true ? tr("yes") : tr("no"));
        s+="\n";
    }

	//Has comments?
	if(!comments.isEmpty()){
		s+=tr("Comments=%1").arg(comments);
		s+="\n";
	}

	return s;
}

QString Activity::getDetailedDescriptionWithConstraints(Instance& r)
{
    QString s=this->getDetailedDescription();

	s+="--------------------------------------------------\n";
	s+=tr("Time constraints directly related to this activity:");
	s+="\n";
	for(int i=0; i<r.timeConstraintsList.size(); i++){
		TimeConstraint* c=r.timeConstraintsList[i];
        if(c->isRelatedToActivity(this)){
			s+="\n";
            s+=c->getDetailedDescription();
		}
	}

//	if(r.groupActivitiesInInitialOrderList.count()>0){
//		s+=tr("Timetable generation options directly related to this activity:");
//		s+="\n";
//		for(int i=0; i<r.groupActivitiesInInitialOrderList.count(); i++){
//			GroupActivitiesInInitialOrderItem* item=r.groupActivitiesInInitialOrderList[i];
//			if(item->ids.contains(id)){
//				s+="\n";
//				s+=item->getDetailedDescription(r);
//			}
//		}
//		s+="--------------------------------------------------\n";
//	}

	return s;
}

bool Activity::isSplit()
{
	return this->totalDuration != this->duration;
}

bool Activity::representsComponentNumber(int index)
{
	if(this->activityGroupId==0)
		return index==1;
		//return false;
		
	//assert(this->activityGroupId>0);
	
	return index == (this->id - this->activityGroupId + 1);
}

void to_json(json &j, const Activity &s)
{
    Q_UNUSED(j)Q_UNUSED(s)
}

void from_json(const json &j, Activity &p)
{
    Q_UNUSED(j)Q_UNUSED(p)
}

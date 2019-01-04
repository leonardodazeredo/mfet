
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

//**********************************************************************************************************************/
//August 2007
//XHTML generation code by Volker Dirr (timetabling.de)
//Features:   - XHTML 1.0 strict valid
//            - using colspan and rowspan
//            - table of contents with hyperlinks
//            - CSS and JavaScript support
//            - index HTML file
//            - defs::TIMETABLE_HTML_LEVEL
//            - days/time horizontal/vertical
//            - subgroups, groups, years, teachers, rooms, subjects, activities, activity tags timetable
//            - teachers free periods
//            - daily timetable
//            - activities with same starting time
//            - reorganized functions. now they can be also used for printing
//            - split times tables after X names (TIMETABLE_HTML_SPLIT?) and choose if activity tags should be printed (defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS)
//            - teachers and students statistics (gaps, free days, hours)

//TODO: all must be internal here. so maybe also do daysOfTheWeek and hoursPerDay also internal
//maybe TODO: use back_odd and back_even (or back0 and back1, because easier to code!) like in printing. so don't use the table_odd and table_even anymore
//maybe TODO: make TIMETABLE_HTML_SPLIT? (similar to defs::TIMETABLE_HTML_LEVEL)
//maybe TODO: rename augmentedYearsList into internalYearsList to have it similar to others?
//maybe TODO: some "stg" stuff can be replaced by Timetable::getInstance()->rules.internalGroupsList. I don't want to do that now, because id-s will change. That is not critical, but I want to diff tables with old release.

#include "defs.h"
#include "tcontext.h"
#include "timetableexport.h"
#include "solution.h"

#include "matrix.h"

#include <iostream>
using namespace std;

#include <QString>
#include <QTextStream>
#include <QFile>

#include <QList>

#include <QHash>

#include "textmessages.h"


#include <QLocale>
#include <QTime>
#include <QDate>

#include <QDir>

#include "stringutils.h"


extern Instance rules2;

extern Matrix2D<double> notAllowedRoomTimePercentages;

static QList<int> activitiesForCurrentSubject[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];
static QList<int> activitiesForCurrentActivityTag[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

static QList<int> activitiesAtTime[MAX_DAYS_PER_WEEK][MAX_HOURS_PER_DAY];

const QString STRING_EMPTY_SLOT="---";

const QString STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES="???";

const QString STRING_NOT_AVAILABLE_TIME_SLOT="-x-";

const QString STRING_BREAK_SLOT="-X-";

//these hashes are needed to get the IDs for html and css in timetableexport and statistics
static QHash<QString, QString> hashSubjectIDsTimetable;
static QHash<QString, QString> hashActivityTagIDsTimetable;
static QHash<QString, QString> hashStudentIDsTimetable;
static QHash<QString, QString> hashTeacherIDsTimetable;
static QHash<QString, QString> hashRoomIDsTimetable;
static QHash<QString, QString> hashDayIDsTimetable;

//static QHash<QString, QString> hashColorStringIDsTimetable;
QHash<int, int> hashActivityColorBySubject;
QList<int> activeHashActivityColorBySubject;
QHash<int, int> hashActivityColorBySubjectAndStudents;
QList<int> activeHashActivityColorBySubjectAndStudents;
const int COLOR_BY_SUBJECT=1;
const int COLOR_BY_SUBJECT_STUDENTS=2;

//this hash is needed to care about sctivities with same starting time
static QHash<int, QList<int> >activitiesWithSameStartingTime;

//Now the filenames of the output files are following (for xml and all html tables)
const QString SUBGROUPS_TIMETABLE_FILENAME_XML="subgroups.xml";
const QString TEACHERS_TIMETABLE_FILENAME_XML="teachers.xml";
const QString ACTIVITIES_TIMETABLE_FILENAME_XML="activities.xml";
const QString ROOMS_TIMETABLE_FILENAME_XML="rooms.xml";

const QString CONFLICTS_FILENAME="soft_conflicts.txt";
const QString INDEX_HTML="index.html";
const QString STYLESHEET_CSS="stylesheet.css";

const QString SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="subgroups_days_horizontal.html";
const QString SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="subgroups_days_vertical.html";
const QString SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="subgroups_time_horizontal.html";
const QString SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="subgroups_time_vertical.html";

const QString GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="groups_days_horizontal.html";
const QString GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="groups_days_vertical.html";
const QString GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="groups_time_horizontal.html";
const QString GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="groups_time_vertical.html";

const QString YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="years_days_horizontal.html";
const QString YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="years_days_vertical.html";
const QString YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="years_time_horizontal.html";
const QString YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="years_time_vertical.html";

const QString TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="teachers_days_horizontal.html";
const QString TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="teachers_days_vertical.html";
const QString TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="teachers_time_horizontal.html";
const QString TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="teachers_time_vertical.html";

const QString ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="rooms_days_horizontal.html";
const QString ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="rooms_days_vertical.html";
const QString ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="rooms_time_horizontal.html";
const QString ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="rooms_time_vertical.html";

const QString SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="subjects_days_horizontal.html";
const QString SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="subjects_days_vertical.html";
const QString SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="subjects_time_horizontal.html";
const QString SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="subjects_time_vertical.html";

const QString ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="activity_tags_days_horizontal.html";
const QString ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="activity_tags_days_vertical.html";
const QString ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="activity_tags_time_horizontal.html";
const QString ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="activity_tags_time_vertical.html";

const QString ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="activities_days_horizontal.html";
const QString ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="activities_days_vertical.html";
const QString ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML="activities_time_horizontal.html";
const QString ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML="activities_time_vertical.html";

const QString TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML="teachers_free_periods_days_horizontal.html";
const QString TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML="teachers_free_periods_days_vertical.html";

const QString TEACHERS_STATISTICS_FILENAME_HTML="teachers_statistics.html";
const QString STUDENTS_STATISTICS_FILENAME_HTML="students_statistics.html";

const QString MULTIPLE_TIMETABLE_DATA_RESULTS_FILE="data_and_timetable.m-fet";

//now the XML tags used for identification of the output file (is that comment correct? it's the old comment)
const QString STUDENTS_TIMETABLE_TAG="Students_Timetable";
const QString TEACHERS_TIMETABLE_TAG="Teachers_Timetable";
const QString ACTIVITIES_TIMETABLE_TAG="Activities_Timetable";
const QString ROOMS_TIMETABLE_TAG="Rooms_Timetable";

const QString RANDOM_SEED_FILENAME_BEFORE="random_seed_before.txt";
const QString RANDOM_SEED_FILENAME_AFTER="random_seed_after.txt";

extern int XX;
extern int YY;

QString generationLocalizedTime=QString(""); //to be used in timetableprintform.cpp

//similar to code from Marco Vassura, modified by Volker Dirr to get rid of QColor and QBrush, since they need QtGui.
//The command-line version does not have access to QtGui.
void TimetableExport::stringToColor(QString s, int *r, int *g, int *b)
{
	// CRC-24 Based on RFC 2440 Section 6.1
	unsigned long crc = 0xB704CEL;
	int i;
	QChar *data = s.data();
	while (!data->isNull()) {
		crc ^= (data->unicode() & 0xFF) << 16;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if (crc & 0x1000000)
				crc ^= 0x1864CFBL;
		}
		data++;
	}
	*r=(crc>>16);
	*g=((crc>>8) & 0xFF);
	*b=(crc & 0xFF);
}
//similar to code from Marco Vassura, modified by Volker Dirr

bool writeAtLeastATimetable()
{
    bool t = defs::WRITE_TIMETABLE_CONFLICTS ||
	
     (defs::WRITE_TIMETABLES_STATISTICS &&
     (defs::WRITE_TIMETABLES_SUBGROUPS ||
     defs::WRITE_TIMETABLES_GROUPS ||
     defs::WRITE_TIMETABLES_YEARS ||
     defs::WRITE_TIMETABLES_TEACHERS)) ||
	
     (defs::WRITE_TIMETABLES_XML &&
     (defs::WRITE_TIMETABLES_SUBGROUPS ||
     defs::WRITE_TIMETABLES_TEACHERS ||
     defs::defs::WRITE_TIMETABLES_ACTIVITIES)) ||
	
     ((defs::WRITE_TIMETABLES_DAYS_HORIZONTAL ||
     defs::WRITE_TIMETABLES_DAYS_VERTICAL ||
     defs::WRITE_TIMETABLES_TIME_HORIZONTAL ||
     defs::WRITE_TIMETABLES_TIME_VERTICAL) &&
     (defs::WRITE_TIMETABLES_SUBGROUPS ||
     defs::WRITE_TIMETABLES_GROUPS ||
     defs::WRITE_TIMETABLES_YEARS ||
     defs::WRITE_TIMETABLES_TEACHERS ||
     defs::WRITE_TIMETABLES_ROOMS ||
     defs::WRITE_TIMETABLES_SUBJECTS ||
     defs::WRITE_TIMETABLES_ACTIVITY_TAGS ||
     defs::defs::WRITE_TIMETABLES_ACTIVITIES)) ||
	
     ((defs::WRITE_TIMETABLES_DAYS_HORIZONTAL ||
     defs::WRITE_TIMETABLES_DAYS_VERTICAL) &&
     defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS);
	
	 return t;
}

TimetableExport::TimetableExport()
{
}

TimetableExport::~TimetableExport()
{
}

QString TimetableExport::getOutPutDir_single()
{
    QString outPutDir=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";

    outPutDir.append(defs::FILE_SEP);
    if(defs::INPUT_FILENAME_XML=="")
        outPutDir.append("unnamed");
    else{
        outPutDir.append(defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1));
        if(outPutDir.right(4)==".m-fet")
            outPutDir=outPutDir.left(outPutDir.length()-4);
        //else if(defs::INPUT_FILENAME_XML!="")
        //	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
    }
    outPutDir.append("-single");

    return outPutDir;
}

void TimetableExport::writeSimulationResults(QWidget* parent){
	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES = getOutPutDir_single();
	
	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
    assert(defs::TIMETABLE_HTML_LEVEL>=0);
    assert(defs::TIMETABLE_HTML_LEVEL<=7);

	computeHashForIDsTimetable();
	computeActivitiesAtTime();
	computeActivitiesWithSameStartingTime();

	QString s;
	QString bar;
    if(defs::INPUT_FILENAME_XML=="")
		bar="";
	else
		bar="_";
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	
	//now write the solution in xml files
	//subgroups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_FILENAME_XML;
	writeSubgroupsTimetableXml(parent, s);
	//teachers
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_FILENAME_XML;
	writeTeachersTimetableXml(parent, s);
	//activities
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITIES_TIMETABLE_FILENAME_XML;
	writeActivitiesTimetableXml(parent, s);

	//now get the time. TODO: maybe write it in xml too? so do it a few lines earlier!
	QDate dat=QDate::currentDate();
	QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
	QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
	generationLocalizedTime=sTime;
	
	//now get the number of placed activities. TODO: maybe write it in xml too? so do it a few lines earlier!
	int na=0;
	int na2=0;
    TContext::get()->getNumberOfPlacedActivities(na, na2);
	
    if(na==TContext::get()->instance.activeActivitiesList.size() && na==na2){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+MULTIPLE_TIMETABLE_DATA_RESULTS_FILE;
        if(defs::VERBOSE){
			cout<<"Since simulation is complete, m-FET will write also the timetable data file"<<endl;
		}
		writeTimetableDataFile(parent, s);
	}
	
	//write the conflicts in txt mode
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+CONFLICTS_FILENAME;
	writeConflictsTxt(parent, s, sTime, na);
	
	//now write the solution in html files
    if(defs::TIMETABLE_HTML_LEVEL>=1){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+STYLESHEET_CSS;
		writeStylesheetCss(parent, s, sTime, na);
	}
	
	//indexHtml
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+INDEX_HTML;
	writeIndexHtml(parent, s, sTime, na);
	
	//subgroups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//groups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeGroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeGroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//years
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeYearsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeYearsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//rooms
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeRoomsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeRoomsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//subjects
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubjectsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubjectsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//activty_tags
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//all activities
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers free periods
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysVerticalHtml(parent, s, sTime, na);
	//statistics
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_STATISTICS_FILENAME_HTML;
	writeTeachersStatisticsHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+STUDENTS_STATISTICS_FILENAME_HTML;
	writeStudentsStatisticsHtml(parent, s, sTime, na);

/*
	//needed for printing from the interface, so don't clear them! 
	hashSubjectIDsTimetable.clear();
	hashActivityTagIDsTimetable.clear();
	hashStudentIDsTimetable.clear();
	hashTeacherIDsTimetable.clear();
	hashRoomIDsTimetable.clear();
	hashDayIDsTimetable.clear();
	hashActivityColorBySubject.clear();
	hashActivityColorBySubjectAndStudents.clear();
	activeHashActivityColorBySubject.clear();
	activeHashActivityColorBySubjectAndStudents.clear();
*/
    if(defs::VERBOSE){
		cout<<"Writing simulation results to disk completed successfully"<<endl;
	}
}

QString TimetableExport::getOutPutDir_highest()
{
    QString OUTPUT_DIR_TIMETABLES=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";

    OUTPUT_DIR_TIMETABLES.append(defs::FILE_SEP);
    if(defs::INPUT_FILENAME_XML=="")
        OUTPUT_DIR_TIMETABLES.append("unnamed");
    else{
        OUTPUT_DIR_TIMETABLES.append(defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1));
        if(OUTPUT_DIR_TIMETABLES.right(4)==".m-fet")
            OUTPUT_DIR_TIMETABLES=OUTPUT_DIR_TIMETABLES.left(OUTPUT_DIR_TIMETABLES.length()-4);
        //else if(defs::INPUT_FILENAME_XML!="")
        //	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
    }
    OUTPUT_DIR_TIMETABLES.append("-highest");

    return OUTPUT_DIR_TIMETABLES;
}

void TimetableExport::writeHighestStageResults(QWidget* parent){
	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES = getOutPutDir_highest();
	
	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
    assert(defs::TIMETABLE_HTML_LEVEL>=0);
    assert(defs::TIMETABLE_HTML_LEVEL<=7);

	computeHashForIDsTimetable();
	computeActivitiesAtTime();
	computeActivitiesWithSameStartingTime();

	QString s;
	QString bar;
    if(defs::INPUT_FILENAME_XML=="")
		bar="";
	else
		bar="_";
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	
	//now write the solution in xml files
	//subgroups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_FILENAME_XML;
	writeSubgroupsTimetableXml(parent, s);
	//teachers
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_FILENAME_XML;
	writeTeachersTimetableXml(parent, s);
	//activities
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITIES_TIMETABLE_FILENAME_XML;
	writeActivitiesTimetableXml(parent, s);

	//now get the time. TODO: maybe write it in xml too? so do it a few lines earlier!
	QDate dat=QDate::currentDate();
	QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
	QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
	generationLocalizedTime=sTime;
	
	//now get the number of placed activities. TODO: maybe write it in xml too? so do it a few lines earlier!
	int na=0;
	int na2=0;
    TContext::get()->getNumberOfPlacedActivities(na, na2);
	
    if(na==TContext::get()->instance.activeActivitiesList.size() && na==na2){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+MULTIPLE_TIMETABLE_DATA_RESULTS_FILE;
        if(defs::VERBOSE){
			cout<<"Since simulation is complete, m-FET will write also the timetable data file"<<endl;
		}
		writeTimetableDataFile(parent, s);
	}
	
	//write the conflicts in txt mode
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+CONFLICTS_FILENAME;
	writeConflictsTxt(parent, s, sTime, na);
	
	//now write the solution in html files
    if(defs::TIMETABLE_HTML_LEVEL>=1){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+STYLESHEET_CSS;
		writeStylesheetCss(parent, s, sTime, na);
	}
	
	//indexHtml
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+INDEX_HTML;
	writeIndexHtml(parent, s, sTime, na);
	
	//subgroups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//groups
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeGroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeGroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//years
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeYearsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeYearsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//rooms
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeRoomsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeRoomsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//subjects
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubjectsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubjectsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//activity_tags
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//all activities
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
        s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers free periods
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysHorizontalHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysVerticalHtml(parent, s, sTime, na);
	//statistics
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+TEACHERS_STATISTICS_FILENAME_HTML;
	writeTeachersStatisticsHtml(parent, s, sTime, na);
    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+STUDENTS_STATISTICS_FILENAME_HTML;
	writeStudentsStatisticsHtml(parent, s, sTime, na);

	//needed for printing from the interface, so don't clear them! 
/*	hashSubjectIDsTimetable.clear();
	hashActivityTagIDsTimetable.clear();
	hashStudentIDsTimetable.clear();
	hashTeacherIDsTimetable.clear();
	hashRoomIDsTimetable.clear();
	hashDayIDsTimetable.clear();
	hashActivityColorBySubject.clear();
	hashActivityColorBySubjectAndStudents.clear();
	activeHashActivityColorBySubject.clear();
	activeHashActivityColorBySubjectAndStudents.clear();
*/
    if(defs::VERBOSE){
		cout<<"Writing highest stage results to disk completed successfully"<<endl;
	}
}

void TimetableExport::writeRandomSeed(QWidget* parent, bool before)
{
	QString RANDOM_SEED_FILENAME;
	if(before)
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_BEFORE;
	else
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_AFTER;

	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";
	
    OUTPUT_DIR_TIMETABLES.append(defs::FILE_SEP);
    if(defs::INPUT_FILENAME_XML=="")
		OUTPUT_DIR_TIMETABLES.append("unnamed");
	else{
        OUTPUT_DIR_TIMETABLES.append(defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1));
		if(OUTPUT_DIR_TIMETABLES.right(4)==".m-fet")
			OUTPUT_DIR_TIMETABLES=OUTPUT_DIR_TIMETABLES.left(OUTPUT_DIR_TIMETABLES.length()-4);
        //else if(defs::INPUT_FILENAME_XML!="")
		//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	}
	OUTPUT_DIR_TIMETABLES.append("-single");
	
	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

	QString s;
	QString bar;
    if(defs::INPUT_FILENAME_XML=="")
		bar="";
	else
		bar="_";
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);

    s=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+bar+RANDOM_SEED_FILENAME;
	
	writeRandomSeedFile(parent, s, before);
}

void TimetableExport::writeRandomSeedFile(QWidget* parent, const QString& filename, bool before)
{
	QString s=filename;

	QFile file(s);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(s));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	QDate dat=QDate::currentDate();
	QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
	QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
	
	if(before){
		tos<<tr("Generation started on: %1", "%1 is the time").arg(sTime);
		tos<<endl<<endl;
//		tos<<tr("Random seed at the start of generation is: X=%1, Y=%2", "The random seed has two components, X and Y").arg(XX).arg(YY);
		tos<<endl<<endl;
		tos<<tr("This file was automatically generated by m-FET %1.").arg(FET_VERSION);
		tos<<endl;
	}
	else{
		tos<<tr("Generation ended on: %1", "%1 is the time").arg(sTime);
		tos<<endl<<endl;
//		tos<<tr("Random seed at the end of generation is: X=%1, Y=%2", "The random seed has two components, X and Y").arg(XX).arg(YY);
		tos<<endl<<endl;
		tos<<tr("This file was automatically generated by m-FET %1.").arg(FET_VERSION);
		tos<<endl;
	}
	
	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(s).arg(file.error()));
	}
	file.close();
}

void TimetableExport::writeTimetableDataFile(QWidget* parent, const QString& filename){
    if(!TContext::get()->bestSolution().students_schedule_ready || !TContext::get()->bestSolution().teachers_schedule_ready){
        MessagesManager::critical(parent, tr("m-FET - Critical"), tr("Timetable not generated - cannot save it - this should not happen (please report bug)"));
		return;
	}

    Solution tc=TContext::get()->bestSolution();

    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
        //Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
        int time=tc.times[ai];
		if(time==UNALLOCATED_TIME){
            MessagesManager::critical(parent, tr("m-FET - Critical"), tr("Incomplete timetable - this should not happen - please report bug"));
			return;
		}
		
//		int ri=tc->rooms[ai];
//		if(ri==UNALLOCATED_SPACE){
//			MessagesManager::critical(parent, tr("m-FET - Critical"), tr("Incomplete timetable - this should not happen - please report bug"));
//			return;
//		}
	}
	
	rules2.initialized=true;
	
    rules2.institutionName=TContext::get()->instance.institutionName;
    rules2.comments=TContext::get()->instance.comments;
	
    rules2.nHoursPerDay=TContext::get()->instance.nHoursPerDay;
    for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
        rules2.hoursOfTheDay[i]=TContext::get()->instance.hoursOfTheDay[i];

    rules2.nDaysPerWeek=TContext::get()->instance.nDaysPerWeek;
    for(int i=0; i<TContext::get()->instance.nDaysPerWeek; i++)
        rules2.daysOfTheWeek[i]=TContext::get()->instance.daysOfTheWeek[i];
		
    rules2.yearsList=TContext::get()->instance.yearsList;
	
    rules2.teachersList=TContext::get()->instance.teachersList;
	
    rules2.subjectsList=TContext::get()->instance.subjectsList;
	

    rules2.timeConstraintsList=TContext::get()->instance.timeConstraintsList;
	
    rules2.apstHash=TContext::get()->instance.apstHash;
	
//	rules2.groupActivitiesInInitialOrderList=Timetable::getInstance()->rules.groupActivitiesInInitialOrderList;

	//add locking constraints
	TimeConstraintsList lockTimeConstraintsList;

	//bool report=false;
	
	int addedTime=0, duplicatesTime=0;

	//lock selected activities
    for(int ai=0; ai<TContext::get()->instance.activeActivitiesList.size(); ai++){
        Activity* act=TContext::get()->instance.activeActivitiesList[ai];
        int time=tc.times[ai];
        if(time>=0 && time<TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay){
            int hour=TContext::get()->bestSolution().getHourForActivityIndex(ai);
            int day=TContext::get()->bestSolution().getDayForActivityIndex(ai);

            ConstraintActivityPreferredStartingTime* ctr=new ConstraintActivityPreferredStartingTime(TContext::get()->instance, Enums::ConstraintGroup::Essential, 100.0, act->id, day, hour, false); //permanently locked is false
			bool t=rules2.addTimeConstraint(ctr);
						
			if(t){
				addedTime++;
				lockTimeConstraintsList.append(ctr);
			}
			else
				duplicatesTime++;

			QString s;
						
			if(t)
                s=tr("Added the following constraint to saved file:")+"\n"+ctr->getDetailedDescription();
			else{
                s=tr("Constraint\n%1 NOT added to saved file - duplicate").arg(ctr->getDetailedDescription());
				delete ctr;
			}
						
			/*if(report){
				int k;
				if(t)
                    k=MessagesManager::confirmation(parent, tr("m-FET information"), s,
				 	 tr("Skip information"), tr("See next"), QString(), 1, 0 );
				else
                    k=MessagesManager::confirmation(parent, tr("m-FET warning"), s,
				 	 tr("Skip information"), tr("See next"), QString(), 1, 0 );
		 		if(k==0)
					report=false;
			}*/
		}		
	}

	//QMessageBox::information(parent, tr("m-FET information"), tr("Added %1 locking time constraints and %2 locking space constraints to saved file,"
	// " ignored %3 activities which were already fixed in time and %4 activities which were already fixed in space").arg(addedTime).arg(addedSpace).arg(duplicatesTime).arg(duplicatesSpace));
		
	bool result=rules2.write(parent, filename);
	
	while(!lockTimeConstraintsList.isEmpty())
		delete lockTimeConstraintsList.takeFirst();

	//if(result)
	//	QMessageBox::information(parent, tr("m-FET information"),
	//		tr("File saved successfully. You can see it on the hard disk. Current data file remained untouched (of locking constraints),"
	//		" so you can save it also, or generate different timetables."));

	rules2.nHoursPerDay=0;
	rules2.nDaysPerWeek=0;

	rules2.yearsList.clear();
	
	rules2.teachersList.clear();
	
	rules2.subjectsList.clear();
	
	rules2.activitiesList.clear();

	rules2.timeConstraintsList.clear();
	
	rules2.apstHash.clear();
	
//	rules2.groupActivitiesInInitialOrderList.clear();

	if(!result){
        MessagesManager::critical(parent, tr("m-FET critical"), tr("Could not save the data + timetable file on the hard disk - maybe hard disk is full"));
	}
}

void TimetableExport::writeSimulationResults(QWidget* parent, int n){
	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";

	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
    assert(defs::TIMETABLE_HTML_LEVEL>=0);
    assert(defs::TIMETABLE_HTML_LEVEL<=7);

	computeHashForIDsTimetable();
	computeActivitiesAtTime();
	computeActivitiesWithSameStartingTime();

	QString s;
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	
    QString destDir=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+"-multi";
	
	if(!dir.exists(destDir))
		dir.mkpath(destDir);
		
    QString finalDestDir=destDir+defs::FILE_SEP+utils::strings::number(n);

	if(!dir.exists(finalDestDir))
		dir.mkpath(finalDestDir);
		
    finalDestDir+=defs::FILE_SEP;


    QString s3=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);

	if(s3.right(4)==".m-fet")
		s3=s3.left(s3.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	finalDestDir+=s3+"_";
	
	//write data+timetable in .m-fet format
	writeTimetableDataFile(parent, finalDestDir+MULTIPLE_TIMETABLE_DATA_RESULTS_FILE);

	//now write the solution in xml files
	//subgroups
	s=finalDestDir+SUBGROUPS_TIMETABLE_FILENAME_XML;
	writeSubgroupsTimetableXml(parent, s);
	//teachers
	s=finalDestDir+TEACHERS_TIMETABLE_FILENAME_XML;
	writeTeachersTimetableXml(parent, s);
	//activities
	s=finalDestDir+ACTIVITIES_TIMETABLE_FILENAME_XML;
	writeActivitiesTimetableXml(parent, s);

	//now get the time. TODO: maybe write it in xml too? so do it a few lines earlier!
	QDate dat=QDate::currentDate();
	QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
	QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
	generationLocalizedTime=sTime;

	//now get the number of placed activities. TODO: maybe write it in xml too? so do it a few lines earlier!
	int na=0;
	int na2=0;
    TContext::get()->getNumberOfPlacedActivities(na, na2);
	
	//write the conflicts in txt mode
	s=finalDestDir+CONFLICTS_FILENAME;
	writeConflictsTxt(parent, s, sTime, na);
	
	//now write the solution in html files
    if(defs::TIMETABLE_HTML_LEVEL>=1){
		s=finalDestDir+STYLESHEET_CSS;
		writeStylesheetCss(parent, s, sTime, na);
	}
	//indexHtml
	s=finalDestDir+INDEX_HTML;
	writeIndexHtml(parent, s, sTime, na);
	//subgroups
	s=finalDestDir+SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubgroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubgroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//groups
	s=finalDestDir+GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeGroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeGroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeGroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeGroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//years
	s=finalDestDir+YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeYearsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeYearsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeYearsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeYearsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers
	s=finalDestDir+TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeTeachersTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeTeachersTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//rooms
//	s=finalDestDir+ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeRoomsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=finalDestDir+ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeRoomsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=finalDestDir+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=finalDestDir+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=finalDestDir+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeRoomsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=finalDestDir+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeRoomsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//subjects
	s=finalDestDir+SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeSubjectsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeSubjectsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeSubjectsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeSubjectsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//activity_tags
//	s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	writeActivityTagsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=finalDestDir+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		writeActivityTagsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//all activities
	s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeAllActivitiesTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=finalDestDir+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		writeAllActivitiesTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers free periods
	s=finalDestDir+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=finalDestDir+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	writeTeachersFreePeriodsTimetableDaysVerticalHtml(parent, s, sTime, na);
	//statistics
	s=finalDestDir+TEACHERS_STATISTICS_FILENAME_HTML;
	writeTeachersStatisticsHtml(parent, s, sTime, na);
	s=finalDestDir+STUDENTS_STATISTICS_FILENAME_HTML;
	writeStudentsStatisticsHtml(parent, s, sTime, na);

	//needed for printing from the interface, so don't clear them! 	
/*	hashSubjectIDsTimetable.clear();
	hashActivityTagIDsTimetable.clear();
	hashStudentIDsTimetable.clear();
	hashTeacherIDsTimetable.clear();
	hashRoomIDsTimetable.clear();
	hashDayIDsTimetable.clear();
	hashActivityColorBySubject.clear();
	hashActivityColorBySubjectAndStudents.clear();
	activeHashActivityColorBySubject.clear();
	activeHashActivityColorBySubjectAndStudents.clear();
*/
    if(defs::VERBOSE){
		cout<<"Writing multiple simulation results to disk completed successfully"<<endl;
	}
}

void TimetableExport::writeRandomSeed(QWidget* parent, int n, bool before){
	QString RANDOM_SEED_FILENAME;
	if(before)
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_BEFORE;
	else
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_AFTER;

	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";

	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

	QString s;
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	
    QString destDir=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+"-multi";
	
	if(!dir.exists(destDir))
		dir.mkpath(destDir);
		
    QString finalDestDir=destDir+defs::FILE_SEP+utils::strings::number(n);

	if(!dir.exists(finalDestDir))
		dir.mkpath(finalDestDir);
		
    finalDestDir+=defs::FILE_SEP;

    QString s3=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);

	if(s3.right(4)==".m-fet")
		s3=s3.left(s3.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	finalDestDir+=s3+"_";
	
	s=finalDestDir+RANDOM_SEED_FILENAME;

	writeRandomSeedFile(parent, s, before);
}

void TimetableExport::writeReportForMultiple(QWidget* parent, const QString& description, bool begin)
{
	QDir dir;
	
    QString OUTPUT_DIR_TIMETABLES=defs::OUTPUT_DIR+defs::FILE_SEP+"timetables";

	//make sure that the output directory exists
	if(!dir.exists(OUTPUT_DIR_TIMETABLES))
		dir.mkpath(OUTPUT_DIR_TIMETABLES);

	QString s;
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	
    QString destDir=OUTPUT_DIR_TIMETABLES+defs::FILE_SEP+s2+"-multi";
	
	if(!dir.exists(destDir))
		dir.mkpath(destDir);
		
    QString filename=destDir+defs::FILE_SEP+QString("report.txt");

	QFile file(filename);
	if(!file.open(QIODevice::Append)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(filename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	if(begin){
		tos.setGenerateByteOrderMark(true);
	}
	
	tos<<description<<endl;
	
	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(filename).arg(file.error()));
	}
	file.close();
}

void TimetableExport::writeSimulationResultsCommandLine(QWidget* parent, const QString& outputDirectory){ //outputDirectory contains trailing defs::FILE_SEP
    QString add=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(add.right(4)==".m-fet")
		add=add.left(add.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	if(add!="")
		add.append("_");

	/////////

    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
    assert(defs::TIMETABLE_HTML_LEVEL>=0);
    assert(defs::TIMETABLE_HTML_LEVEL<=7);

	computeHashForIDsTimetable();
	computeActivitiesAtTime();
	computeActivitiesWithSameStartingTime();

	TimetableExport::writeSubgroupsTimetableXml(parent, outputDirectory+add+SUBGROUPS_TIMETABLE_FILENAME_XML);
	TimetableExport::writeTeachersTimetableXml(parent, outputDirectory+add+TEACHERS_TIMETABLE_FILENAME_XML);
	TimetableExport::writeActivitiesTimetableXml(parent, outputDirectory+add+ACTIVITIES_TIMETABLE_FILENAME_XML);
	
	//get the time
	QDate dat=QDate::currentDate();
	QTime tim=QTime::currentTime();
    QLocale loc(defs::FET_LANGUAGE);
	QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
	generationLocalizedTime=sTime; //really unneeded, but just to be similar to the other parts
	
	//now get the number of placed activities. TODO: maybe write it in xml too? so do it a few lines earlier!
	int na=0;
	int na2=0;
    TContext::get()->getNumberOfPlacedActivities(na, na2);
	
    if(na==TContext::get()->instance.activeActivitiesList.size() && na==na2){
		QString s=outputDirectory+add+MULTIPLE_TIMETABLE_DATA_RESULTS_FILE;
        if(defs::VERBOSE){
			cout<<"Since simulation is complete, m-FET will write also the timetable data file"<<endl;
		}
		writeTimetableDataFile(parent, s);
	}

	//write the conflicts in txt mode
	QString s=add+CONFLICTS_FILENAME;
	s.prepend(outputDirectory);
	TimetableExport::writeConflictsTxt(parent, s, sTime, na);
	
	//now write the solution in html files
    if(defs::TIMETABLE_HTML_LEVEL>=1){
		s=add+STYLESHEET_CSS;
		s.prepend(outputDirectory);
		TimetableExport::writeStylesheetCss(parent, s, sTime, na);
	}
	//indexHtml
	s=add+INDEX_HTML;
	s.prepend(outputDirectory);
	writeIndexHtml(parent, s, sTime, na);
	//subgroups
	s=add+SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeSubgroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeSubgroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubgroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubgroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubgroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubgroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//groups
	s=add+GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeGroupsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeGroupsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeGroupsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeGroupsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeGroupsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeGroupsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//years
	s=add+YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeYearsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeYearsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeYearsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeYearsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeYearsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeYearsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers
	s=add+TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeTeachersTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeTeachersTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeTeachersTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeTeachersTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeTeachersTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeTeachersTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//rooms
//	s=add+ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	s.prepend(outputDirectory);
//	TimetableExport::writeRoomsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=add+ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	s.prepend(outputDirectory);
//	TimetableExport::writeRoomsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=add+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeRoomsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=add+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeRoomsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=add+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeRoomsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=add+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeRoomsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//subjects
	s=add+SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeSubjectsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeSubjectsTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubjectsTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubjectsTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubjectsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeSubjectsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
//	//activity_tags
//	s=add+ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
//	s.prepend(outputDirectory);
//	TimetableExport::writeActivityTagsTimetableDaysHorizontalHtml(parent, s, sTime, na);
//	s=add+ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
//	s.prepend(outputDirectory);
//	TimetableExport::writeActivityTagsTimetableDaysVerticalHtml(parent, s, sTime, na);
//	if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
//		s=add+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeActivityTagsTimetableTimeHorizontalHtml(parent, s, sTime, na);
//		s=add+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeActivityTagsTimetableTimeVerticalHtml(parent, s, sTime, na);
//	} else {
//		s=add+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeActivityTagsTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
//		s=add+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
//		s.prepend(outputDirectory);
//		TimetableExport::writeActivityTagsTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
//	}
	//all activities
	s=add+ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeAllActivitiesTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeAllActivitiesTimetableDaysVerticalHtml(parent, s, sTime, na);
    if(!defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS){
		s=add+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeAllActivitiesTimetableTimeHorizontalHtml(parent, s, sTime, na);
		s=add+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeAllActivitiesTimetableTimeVerticalHtml(parent, s, sTime, na);
	} else {
		s=add+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeAllActivitiesTimetableTimeHorizontalDailyHtml(parent, s, sTime, na);
		s=add+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML;
		s.prepend(outputDirectory);
		TimetableExport::writeAllActivitiesTimetableTimeVerticalDailyHtml(parent, s, sTime, na);
	}
	//teachers free periods
	s=add+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeTeachersFreePeriodsTimetableDaysHorizontalHtml(parent, s, sTime, na);
	s=add+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeTeachersFreePeriodsTimetableDaysVerticalHtml(parent, s, sTime, na);
	//statistics
	s=add+TEACHERS_STATISTICS_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeTeachersStatisticsHtml(parent, s, sTime, na);
	s=add+STUDENTS_STATISTICS_FILENAME_HTML;
	s.prepend(outputDirectory);
	TimetableExport::writeStudentsStatisticsHtml(parent, s, sTime, na);

	//we can keep it, since it is for the command line version (but in fact we can also clear or delete these lines, since command line doesn't need interface printing)
	/*hashSubjectIDsTimetable.clear();
	hashActivityTagIDsTimetable.clear();
	hashStudentIDsTimetable.clear();
	hashTeacherIDsTimetable.clear();
	hashRoomIDsTimetable.clear();
	hashDayIDsTimetable.clear();
	hashActivityColorBySubject.clear();
	hashActivityColorBySubjectAndStudents.clear();
	activeHashActivityColorBySubject.clear();
	activeHashActivityColorBySubjectAndStudents.clear();*/
}

void TimetableExport::writeRandomSeedCommandLine(QWidget* parent, const QString& outputDirectory, bool before){ //outputDirectory contains trailing defs::FILE_SEP
	QString RANDOM_SEED_FILENAME;
	if(before)
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_BEFORE;
	else
		RANDOM_SEED_FILENAME=RANDOM_SEED_FILENAME_AFTER;

    QString add=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
	if(add.right(4)==".m-fet")
		add=add.left(add.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	if(add!="")
		add.append("_");

	QString s=add+RANDOM_SEED_FILENAME;
	s.prepend(outputDirectory);
	
	writeRandomSeedFile(parent, s, before);
}

//by Volker Dirr (timetabling.de)
void TimetableExport::writeConflictsTxt(QWidget* parent, const QString& filename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLE_CONFLICTS){
		if(QFile::exists(filename))
			QFile::remove(filename);
	
		return;
	}

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(filename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	
    if(placedActivities==TContext::get()->instance.activeActivitiesList.size()){
        QString tt=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
        if(defs::INPUT_FILENAME_XML=="")
			tt=tr("unnamed");
		tos<<TimetableExport::tr("Soft conflicts of %1", "%1 is the file name").arg(tt);
		tos<<"\n";
		tos<<TimetableExport::tr("Generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)<<"\n\n";

        tos<<TimetableExport::tr("Number of broken soft constraints: %1").arg(TContext::get()->bestSolution().constraintConflictData.conflictsCostList().count())<<endl;
        tos<<TimetableExport::tr("Total soft conflicts: %1").arg(utils::strings::number(TContext::get()->bestSolution().constraintConflictData.costTotal()))<<endl<<endl;
        tos<<TimetableExport::tr("Soft conflicts list (in decreasing order):")<<endl<<endl;
        foreach(QString t, TContext::get()->bestSolution().constraintConflictData.conflictsDescriptionList())
            tos<<t<<endl;
		tos<<endl<<TimetableExport::tr("End of file.")<<"\n";
	}
	else{
        QString tt=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
        if(defs::INPUT_FILENAME_XML=="")
			tt=tr("unnamed");
		tos<<TimetableExport::tr("Conflicts of %1").arg(tt);
		tos<<"\n";
        tos<<TimetableExport::tr("Warning! Only %1 out of %2 activities placed!").arg(placedActivities).arg(TContext::get()->instance.activeActivitiesList.size())<<"\n";
		tos<<TimetableExport::tr("Generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)<<"\n\n";

        tos<<TimetableExport::tr("Number of broken constraints: %1").arg(TContext::get()->bestSolution().constraintConflictData.conflictsCostList().count())<<endl;
        tos<<TimetableExport::tr("Total conflicts: %1").arg(utils::strings::number(TContext::get()->bestSolution().constraintConflictData.costTotal()))<<endl<<endl;
        tos<<TimetableExport::tr("Conflicts list (in decreasing order):")<<endl<<endl;
        foreach(QString t, TContext::get()->bestSolution().constraintConflictData.conflictsDescriptionList())
            tos<<t<<endl;
		tos<<endl<<TimetableExport::tr("End of file.")<<"\n";
	}
	
	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(filename).arg(file.error()));
	}
	file.close();
}

void TimetableExport::writeSubgroupsTimetableXml(QWidget* parent, const QString& xmlfilename){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_XML || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(xmlfilename))
			QFile::remove(xmlfilename);
	
		return;
	}

	//Now we print the results to an XML file
	QFile file(xmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(xmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	tos<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    tos<<"<"<<utils::strings::parseStrForXml(STUDENTS_TIMETABLE_TAG)<<">\n";

    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size(); subgroup++){
		tos<< "  <Subgroup name=\"";
        QString subgroup_name = TContext::get()->instance.directSubgroupsList[subgroup]->name;
        tos<< utils::strings::parseStrForXml(subgroup_name) << "\">\n";

        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
            tos<<"   <Day name=\""<<utils::strings::parseStrForXml(TContext::get()->instance.daysOfTheWeek[day])<<"\">\n";
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
                tos << "    <Hour name=\"" << utils::strings::parseStrForXml(TContext::get()->instance.hoursOfTheDay[hour]) << "\">\n";
				tos<<"     ";
                int ai=TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour]; //activity index
				if(ai!=UNALLOCATED_ACTIVITY){
                    //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];
                    //TODO: tos<<" <Teacher name=\""<<utils::strings::parseStrForXml(act->teacherName)<<"\"></Teacher>";
                    tos<<"<Subject name=\""<<utils::strings::parseStrForXml(act->subjectName)<<"\"></Subject>";
					foreach(QString atn, act->activityTagsNames)
                        tos<<"<Activity_Tag name=\""<<utils::strings::parseStrForXml(atn)<<"\"></Activity_Tag>";
				}
				tos<<"\n";
				tos << "    </Hour>\n";
			}
			tos<<"   </Day>\n";
		}
		tos<<"  </Subgroup>\n";
	}

    tos << "</" << utils::strings::parseStrForXml(STUDENTS_TIMETABLE_TAG) << ">\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(xmlfilename).arg(file.error()));
	}
	file.close();
}

void TimetableExport::writeTeachersTimetableXml(QWidget* parent, const QString& xmlfilename){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_XML || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(xmlfilename))
			QFile::remove(xmlfilename);
	
		return;
	}

	//Writing the timetable in xml format
	QFile file(xmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(xmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	tos<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    tos << "<" << utils::strings::parseStrForXml(TEACHERS_TIMETABLE_TAG) << ">\n";

    int i = 0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        tos << "  <Teacher name=\"" << utils::strings::parseStrForXml(teacher->name) << "\">\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
            tos << "   <Day name=\"" << utils::strings::parseStrForXml(TContext::get()->instance.daysOfTheWeek[day]) << "\">\n";
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
                tos << "    <Hour name=\"" << utils::strings::parseStrForXml(TContext::get()->instance.hoursOfTheDay[hour]) << "\">\n";

				tos<<"     ";
                int ai=TContext::get()->bestSolution().teachers_timetable_weekly[i][day][hour]; //activity index
                //Activity* act=Timetable::getInstance()->rules.activitiesList.at(ai);
				if(ai!=UNALLOCATED_ACTIVITY){
                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];
                    tos<<"<Subject name=\""<<utils::strings::parseStrForXml(act->subjectName)<<"\"></Subject>";
					foreach(QString atn, act->activityTagsNames)
                        tos<<"<Activity_Tag name=\""<<utils::strings::parseStrForXml(atn)<<"\"></Activity_Tag>";
                    foreach(QString s, act->studentSetsNames)
                        tos << "<Students name=\"" << utils::strings::parseStrForXml(s) << "\"></Students>";
				}
				tos<<"\n";
				tos << "    </Hour>\n";
			}
			tos << "   </Day>\n";
		}
		tos<<"  </Teacher>\n";

        i++;
	}

    tos << "</" << utils::strings::parseStrForXml(TEACHERS_TIMETABLE_TAG) << ">\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(xmlfilename).arg(file.error()));
	}
	file.close();
}

void TimetableExport::writeActivitiesTimetableXml(QWidget* parent, const QString& xmlfilename){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_XML || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(xmlfilename))
			QFile::remove(xmlfilename);

		return;
	}

	//Writing the timetable in xml format
	QFile file(xmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(xmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	tos<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    tos << "<" << utils::strings::parseStrForXml(ACTIVITIES_TIMETABLE_TAG) << ">\n";
	
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
		tos<<"<Activity>"<<endl;
		
        tos<<"	<Id>"<<TContext::get()->instance.activeActivitiesList[i]->id<<"</Id>"<<endl;
		
		QString day="";
        if(TContext::get()->bestSolution().times[i]!=UNALLOCATED_TIME){
            int d=TContext::get()->bestSolution().getDayForActivityIndex(i);
            day=TContext::get()->instance.daysOfTheWeek[d];
		}
        tos<<"	<Day>"<<utils::strings::parseStrForXml(day)<<"</Day>"<<endl;
		
		QString hour="";
        if(TContext::get()->bestSolution().times[i]!=UNALLOCATED_TIME){
            int h=TContext::get()->bestSolution().getHourForActivityIndex(i);
            hour=TContext::get()->instance.hoursOfTheDay[h];
		}
        tos<<"	<Hour>"<<utils::strings::parseStrForXml(hour)<<"</Hour>"<<endl;
		
		tos<<"</Activity>"<<endl;
	}

    tos << "</" << utils::strings::parseStrForXml(ACTIVITIES_TIMETABLE_TAG) << ">\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(xmlfilename).arg(file.error()));
	}
	file.close();
}

// writing the index html file by Volker Dirr.
void TimetableExport::writeIndexHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

	bool _writeAtLeastATimetable = writeAtLeastATimetable();

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(false, placedActivities, true);

	QString bar;
	QString s2="";
    if(defs::INPUT_FILENAME_XML=="")
		bar="";
	else{
		bar="_";
        s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);

		if(s2.right(4)==".m-fet")
			s2=s2.left(s2.length()-4);
        //else if(defs::INPUT_FILENAME_XML!="")
		//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	}
	tos<<"    <p>\n";
	
	if(!_writeAtLeastATimetable){
		tos<<"      "<<TimetableExport::tr("No timetable was written, because from the settings you disabled writing any timetable.")<<"\n";
		tos<<"      "<<TimetableExport::tr("The exception is that after each successful (complete) timetable generation the %1 file"
		 " will be written.").arg("data_and_timetable.m-fet")<<"\n";
		tos<<"    </p>\n";
		tos<<"    <p>\n";
		tos<<"      "<<TimetableExport::tr("File generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)<<"\n";
		tos<<"    </p>\n";
	}
	else{
        if(defs::WRITE_TIMETABLE_CONFLICTS)
			tos<<"      <a href=\""<<s2+bar+CONFLICTS_FILENAME<<"\">"<<tr("View the soft conflicts list.")<<"</a><br />\n";
		else
			tos<<"      "<<TimetableExport::tr("Soft conflicts list - disabled.")<<"<br />\n";

		///////////////////////////

		QString tmps1, tmps2;
        if(defs::WRITE_TIMETABLES_STATISTICS && (defs::WRITE_TIMETABLES_YEARS || defs::WRITE_TIMETABLES_GROUPS || defs::WRITE_TIMETABLES_SUBGROUPS) )
			tmps1="      <a href=\""+s2+bar+STUDENTS_STATISTICS_FILENAME_HTML+"\">"+tr("students")+"</a>";
		else
			tmps1=tr("students - disabled");
		
        if(defs::WRITE_TIMETABLES_STATISTICS && defs::WRITE_TIMETABLES_TEACHERS)
			tmps2="<a href=\""+s2+bar+TEACHERS_STATISTICS_FILENAME_HTML+"\">"+tr("teachers")+"</a>";
		else
			tmps2=tr("teachers - disabled");
			
		QString tmps3=tr("View statistics: %1, %2.", "%1 and %2 are two files in HTML format, to show statistics for students and teachers. The user can click on one file to view it")
		 .arg(tmps1).arg(tmps2);

		tos<<"      "<<tmps3<<"<br />\n";
		
		///////////////////////////
		
		QString tmp1, tmp2, tmp3;
        if(defs::WRITE_TIMETABLES_XML && defs::WRITE_TIMETABLES_SUBGROUPS)
			tmp1="<a href=\""+s2+bar+SUBGROUPS_TIMETABLE_FILENAME_XML+"\">"+tr("subgroups")+"</a>";
		else
			tmp1=tr("subgroups - disabled", "It means the subgroups XML timetables are disabled");
        if(defs::WRITE_TIMETABLES_XML && defs::WRITE_TIMETABLES_TEACHERS)
			tmp2="<a href=\""+s2+bar+TEACHERS_TIMETABLE_FILENAME_XML+"\">"+tr("teachers")+"</a>";
		else
			tmp2=tr("teachers - disabled", "It means the teachers XML timetables are disabled");
        if(defs::WRITE_TIMETABLES_XML && defs::defs::WRITE_TIMETABLES_ACTIVITIES)
			tmp3="<a href=\""+s2+bar+ACTIVITIES_TIMETABLE_FILENAME_XML+"\">"+tr("activities")+"</a>";
		else
			tmp3=tr("activities - disabled", "It means the activities XML timetables are disabled");
		QString tmp4=TimetableExport::tr("View XML: %1, %2, %3.", "%1, %2 and %3 are three files in XML format, subgroups, teachers and activities timetables. The user can click on one file to view it").arg(tmp1).arg(tmp2).arg(tmp3);
		tos<<"      "<<tmp4<<"\n";

		tos<<"    </p>\n\n";

		tos<<"    <table border=\"1\">\n";

        tos<<"      <caption>"<<utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)<<"</caption>\n";

		tos<<"      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\"4\">"+tr("Timetables")+"</th></tr>\n";
		tos<<"        <tr>\n          <!-- span -->\n";
		tos<<"          <th>"+tr("Days Horizontal")+"</th><th>"+tr("Days Vertical")+"</th><th>"+tr("Time Horizontal")+"</th><th>"+tr("Time Vertical")+"</th>";
		tos<<"        </tr>\n";
		tos<<"      </thead>\n";
		tos<<"      <tbody>\n";
		
		/* workaround
		tos<<"      <tfoot><tr><td></td><td colspan=\"4\">"<<TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)<<"</td></tr></tfoot>\n";
		*/

		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Subgroups")+"</th>\n";
        if(defs::WRITE_TIMETABLES_SUBGROUPS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBGROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBGROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBGROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBGROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Groups")+"</th>\n";
        if(defs::WRITE_TIMETABLES_GROUPS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+GROUPS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+GROUPS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+GROUPS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+GROUPS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Years")+"</th>\n";
        if(defs::WRITE_TIMETABLES_YEARS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+YEARS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+YEARS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+YEARS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+YEARS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Teachers")+"</th>\n";
        if(defs::WRITE_TIMETABLES_TEACHERS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Teachers' Free Periods")+"</th>\n";
        if(defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+TEACHERS_FREE_PERIODS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
        tos<<"          <td>"<<utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)<<"</td>\n";
        tos<<"          <td>"<<utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)<<"</td>\n";
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Rooms")+"</th>\n";
        if(defs::WRITE_TIMETABLES_ROOMS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ROOMS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ROOMS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ROOMS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ROOMS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Subjects")+"</th>\n";
        if(defs::WRITE_TIMETABLES_SUBJECTS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBJECTS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBJECTS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBJECTS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+SUBJECTS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Activity Tags")+"</th>\n";
        if(defs::WRITE_TIMETABLES_ACTIVITY_TAGS){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ACTIVITY_TAGS_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ACTIVITY_TAGS_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		tos<<"        <tr>\n";
		tos<<"          <th>"+tr("Activities")+"</th>\n";
        if(defs::defs::WRITE_TIMETABLES_ACTIVITIES){
            if(defs::WRITE_TIMETABLES_DAYS_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_DAYS_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ALL_ACTIVITIES_TIMETABLE_DAYS_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_HORIZONTAL)
				tos<<"          <td><a href=\""<<s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_HORIZONTAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
            if(defs::WRITE_TIMETABLES_TIME_VERTICAL)
				tos<<"          <td><a href=\""<<s2+bar+ALL_ACTIVITIES_TIMETABLE_TIME_VERTICAL_FILENAME_HTML<<"\">"+tr("view")+"</a></td>\n";
			else
				tos<<"          <td>"+tr("disabled")+"</td>\n";
		} else {
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
			tos<<"          <td>"+tr("disabled")+"</td>\n";
		}
		tos<<"        </tr>\n";
		//workaround begin.
		tos<<"      <tr class=\"foot\"><td></td><td colspan=\"4\">"<<TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)<<"</td></tr>\n";
		//workaround end.
		tos<<"      </tbody>\n";
		tos<<"    </table>\n";
	}

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

// writing the stylesheet in css format to a file by Volker Dirr.
void TimetableExport::writeStylesheetCss(QWidget* parent, const QString& cssfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
	
	bool _writeAtLeastATimetable = writeAtLeastATimetable();

	if(!_writeAtLeastATimetable){
		if(QFile::exists(cssfilename))
			QFile::remove(cssfilename);

		return;
	}

	//get used students	//TODO: do it the same way in statistics.cpp
	QSet<QString> usedStudents;
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        foreach(QString st, TContext::get()->instance.activeActivitiesList[i]->studentSetsNames){
			if(!usedStudents.contains(st))
				usedStudents<<st;
		}
	}

	//Now we print the results to a CSS file
	QFile file(cssfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(cssfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	
	tos<<"@charset \"UTF-8\";"<<"\n\n";

    QString tt=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
    if(defs::INPUT_FILENAME_XML=="")
		tt=tr("unnamed");
	tos<<"/* "<<TimetableExport::tr("CSS Stylesheet of %1", "%1 is the file name").arg(tt);
	tos<<"\n";
    if(placedActivities!=TContext::get()->instance.activeActivitiesList.size())
        tos<<"   "<<TimetableExport::tr("Warning! Only %1 out of %2 activities placed!").arg(placedActivities).arg(TContext::get()->instance.activeActivitiesList.size())<<"\n";
	tos<<"   "<<TimetableExport::tr("Stylesheet generated with m-FET %1 on %2", "%1 is m-FET version, %2 is date and time").arg(FET_VERSION).arg(saveTime)<<" */\n\n";

	tos<<"/* "<<TimetableExport::tr("To do a page-break only after every second timetable, delete \"page-break-before: always;\" in \"table.even_table\".",
		"Please keep fields in quotes as they are, untranslated.")<<" */\n";
	tos<<"/* "<<TimetableExport::tr("Delete \"page-break-before: always;\" in \"table.even_table\" and in \"table.odd_table\" to skip page-breaks.",
		"Please keep fields in quotes as they are, untranslated.")<<" */\n";
	tos<<"/* "<<TimetableExport::tr("To hide an element just write the following phrase into the element: %1 (without quotes).",
		"%1 is a short phrase beginning and ending with quotes, and we want the user to be able to add it, but without quotes").arg("\"display:none;\"")<<" */\n\n";
	tos<<"p.back {\n  margin-top: 4ex;\n  margin-bottom: 5ex;\n}\n\n";
	tos<<"table {\n  text-align: center;\n  page-break-inside: avoid;\n}\n\n";
	tos<<"table.odd_table {\n  page-break-before: always;\n}\n\n";
	tos<<"table.even_table {\n  page-break-before: always;\n}\n\n";
	tos<<"table.detailed {\n  margin-left:auto; margin-right:auto;\n  text-align: center;\n  border: 0px;\n  border-spacing: 0;\n  border-collapse: collapse;\n}\n\n";
	tos<<"caption {\n\n}\n\n";
	tos<<"thead {\n\n}\n\n";
	
	//workaround begin.
	tos<<"/* "<<TimetableExport::tr("Some programs import \"tfoot\" incorrectly. So we use \"tr.foot\" instead of \"tfoot\".",
	 "Please keep tfoot and tr.foot untranslated, as they are in the original English phrase")<<" */\n\n";
	//tos<<"tfoot {\n\n}\n\n";
	tos<<"tr.foot {\n\n}\n\n";
	//workaround end

	tos<<"tbody {\n\n}\n\n";
	tos<<"th {\n\n}\n\n";
	tos<<"td {\n\n}\n\n";
	tos<<"td.detailed {\n  border: 1px dashed silver;\n  border-bottom: 0;\n  border-top: 0;\n}\n\n";
    if(defs::TIMETABLE_HTML_LEVEL>=2){
		tos<<"th.xAxis {\n/* width: 8em; */\n}\n\n";
		tos<<"th.yAxis {\n  height: 8ex;\n}\n\n";
	}
	
	//By Liviu, with ideas from Volker
    if(defs::TIMETABLE_HTML_LEVEL==7){ //must be written before LEVEL 3, because LEVEL 3 should have higher priority
		int cnt=0;
		foreach(int i, activeHashActivityColorBySubject){
            Activity* act=TContext::get()->instance.activeActivitiesList[i];
			
			QString tmpString=act->subjectName;
			
			//similar to the coloring by Marco Vassura (start)
			int r,g,b;
			stringToColor(tmpString, &r, &g, &b);
			tos << "td.c_"<<QString::number(cnt+1)<<" { /* Activity id: "<<QString::number(act->id)<<" (subject) */\n ";
			tos<<"background-color: rgb("<<QString::number(r)<<", "<<QString::number(g)<<", "<<QString::number(b)<<");\n";
			double brightness = (double)r*0.299 + (double)g*0.587 + (double)b*0.114;
			if (brightness<127.5)
				tos<<" color: white;\n";
			else
				tos<<" color: black;\n";
			tos<<"}\n\n";
			//similar to the coloring by Marco Vassura (end)
			cnt++;
		}
		foreach(int i, activeHashActivityColorBySubjectAndStudents){
            Activity* act=TContext::get()->instance.activeActivitiesList[i];
			
            QString tmpString=act->subjectName+" "+act->studentSetsNames.join(",");
			
			//similar to the coloring by Marco Vassura (start)
			int r,g,b;
			stringToColor(tmpString, &r, &g, &b);
			tos << "td.c_"<<QString::number(cnt+1)<<" { /* Activity id: "<<QString::number(act->id)<<" (subject+students) */\n ";
			tos<<"background-color: rgb("<<QString::number(r)<<", "<<QString::number(g)<<", "<<QString::number(b)<<");\n";
			double brightness = (double)r*0.299 + (double)g*0.587 + (double)b*0.114;
			if (brightness<127.5)
				tos<<" color: white;\n";
			else
				tos<<" color: black;\n";
			tos<<"}\n\n";
			//similar to the coloring by Marco Vassura (end)
			cnt++;
		}
	}
//	if(defs::TIMETABLE_HTML_LEVEL==7){ // must be written before LEVEL 3, because LEVEL 3 should have higher priority
//		QHashIterator<QString, QString> i(hashColorStringIDsTimetable);
//		while(i.hasNext()) {
//			i.next();
//			tos << "td.c_"<<i.value()<<" { /* "<<i.key()<<" */\n ";
//			
//			//similar to the coloring by Marco Vassura (start)
//			int r,g,b;
//			stringToColor(i.key(), &r, &g, &b);
//			tos<<"background-color: rgb("<<QString::number(r)<<", "<<QString::number(g)<<", "<<QString::number(b)<<");\n";
//			double brightness = (double)r*0.299 + (double)g*0.587 + (double)b*0.114;
//			if (brightness<127.5)
//				tos<<" color: white;\n";
//			else
//				tos<<" color: black;\n";
//			//similar to the coloring by Marco Vassura (end)
//			tos<<"}\n\n";
//		}
//	}
    else if(defs::TIMETABLE_HTML_LEVEL>=4){ // must be written before LEVEL 3, because LEVEL 3 should have higher priority
        foreach (Subject* subject, TContext::get()->instance.subjectsList) {
            tos << "span.s_"<<hashSubjectIDsTimetable.value(subject->name)<<" { /* subject "<<subject->name<<" */\n\n}\n\n";
        }

        for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
            StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
			if(usedStudents.contains(sty->name))
				tos << "span.ss_"<<hashStudentIDsTimetable.value(sty->name)<<" { /* students set "<<sty->name<<" */\n\n}\n\n";
			for(int j=0; j<sty->groupsList.size(); j++){
				StudentsGroup* stg=sty->groupsList[j];
				if(usedStudents.contains(stg->name))
					tos << "span.ss_"<<hashStudentIDsTimetable.value(stg->name)<<" { /* students set "<<stg->name<<" */\n\n}\n\n";
				for(int k=0; k<stg->subgroupsList.size(); k++){
					StudentsSubgroup* sts=stg->subgroupsList[k];
					if(usedStudents.contains(sts->name))
						tos << "span.ss_"<<hashStudentIDsTimetable.value(sts->name)<<" { /* students set "<<sts->name<<" */\n\n}\n\n";
				}
			}
		}

        foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
            tos << "span.t_"<<hashTeacherIDsTimetable.value(teacher->name)<<" { /* teacher "<<teacher->name<<" */\n\n}\n\n";
        }
	}
    if(defs::TIMETABLE_HTML_LEVEL>=3){
		tos<<"span.subject {\n\n}\n\n";
		tos<<"span.activitytag {\n\n}\n\n";

		tos<<"span.empty {\n  color: gray;\n}\n\n";
		tos<<"td.empty {\n  border-color:silver;\n  border-right-style:none;\n  border-bottom-style:none;\n  border-left-style:dotted;\n  border-top-style:dotted;\n}\n\n";

		tos<<"span.notAvailable {\n  color: gray;\n}\n\n";
		tos<<"td.notAvailable {\n  border-color:silver;\n  border-right-style:none;\n  border-bottom-style:none;\n  border-left-style:dotted;\n  border-top-style:dotted;\n}\n\n";

		tos<<"span.break {\n  color: gray;\n}\n\n";
		tos<<"td.break {\n  border-color:silver;\n  border-right-style:none;\n  border-bottom-style:none;\n  border-left-style:dotted;\n  border-top-style:dotted;\n}\n\n";

		tos<<"tr.studentsset, div.studentsset {\n\n}\n\n";
		tos<<"tr.teacher, div.teacher {\n\n}\n\n";
		tos<<"tr.room, div.room {\n\n}\n\n";
        if(defs::TIMETABLE_HTML_LEVEL!=7){
			tos<<"tr.line0, div.line0 {\n  font-size: smaller;\n}\n\n";
			tos<<"tr.line1, div.line1 {\n\n}\n\n";
			tos<<"tr.line2, div.line2 {\n  font-size: smaller;\n  color: gray;\n}\n\n";
			tos<<"tr.line3, div.line3 {\n  font-size: smaller;\n  color: silver;\n}\n\n";
		} else {
			tos<<"tr.line0, div.line0 {\n  font-size: smaller;\n}\n\n";
			tos<<"tr.line1, div.line1 {\n\n}\n\n";
			tos<<"tr.line2, div.line2 {\n  font-size: smaller;\n}\n\n";
			tos<<"tr.line3, div.line3 {\n  font-size: smaller;\n}\n\n";
		}
	}
    if(defs::TIMETABLE_HTML_LEVEL==6){
		tos<<endl<<"/* "<<TimetableExport::tr("Be careful. You might get mutual and ambiguous styles. CSS means that the last definition will be used.")<<" */\n\n";
        foreach (Subject* subject, TContext::get()->instance.subjectsList) {
            tos << "td.s_"<<hashSubjectIDsTimetable.value(subject->name)<<" { /* subject "<<subject->name<<" */\n\n}\n\n";
		}

        for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
            StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
			if(usedStudents.contains(sty->name))
				tos << "td.ss_"<<hashStudentIDsTimetable.value(sty->name)<<" { /* students set "<<sty->name<<" */\n\n}\n\n";
			for(int j=0; j<sty->groupsList.size(); j++){
				StudentsGroup* stg=sty->groupsList[j];
				if(usedStudents.contains(stg->name))
					tos << "td.ss_"<<hashStudentIDsTimetable.value(stg->name)<<" { /* students set "<<stg->name<<" */\n\n}\n\n";
				for(int k=0; k<stg->subgroupsList.size(); k++){
					StudentsSubgroup* sts=stg->subgroupsList[k];
					if(usedStudents.contains(sts->name))
						tos << "td.ss_"<<hashStudentIDsTimetable.value(sts->name)<<" { /* students set "<<sts->name<<" */\n\n}\n\n";
				}
			}
		}
        foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
            tos << "td.t_"<<hashTeacherIDsTimetable.value(teacher->name)<<" { /* teacher "<<teacher->name<<" */\n\n}\n\n";
		}

		//not included yet
        //for(int room=0; room<Timetable::getInstance()->rules.nInternalRooms; room++){
        //	tos << "span.r_"<<hashRoomIDsTimetable.value(Timetable::getInstance()->rules.internalRoomsList[room]->name)<<" { /* room "<<Timetable::getInstance()->rules.internalRoomsList[room]->name<<" */\n\n}\n\n";
		//}
	}
	tos<<"/* "<<TimetableExport::tr("Style the teachers' free periods")<<" */\n\n";
    if(defs::TIMETABLE_HTML_LEVEL>=2){
		tos<<"div.DESCRIPTION {\n  text-align: left;\n  font-size: smaller;\n}\n\n";
	}
    if(defs::TIMETABLE_HTML_LEVEL>=3){
		tos<<"div.TEACHER_HAS_SINGLE_GAP {\n  color: black;\n}\n\n";
		tos<<"div.TEACHER_HAS_BORDER_GAP {\n  color: gray;\n}\n\n";
		tos<<"div.TEACHER_HAS_BIG_GAP {\n  color: silver;\n}\n\n";
		tos<<"div.TEACHER_MUST_COME_EARLIER {\n  color: purple;\n}\n\n";
		tos<<"div.TEACHER_MUST_COME_MUCH_EARLIER {\n  font-size: smaller;\n  color: fuchsia;\n}\n\n";
		tos<<"div.TEACHER_MUST_STAY_LONGER {\n  color: teal;\n}\n\n";
		tos<<"div.TEACHER_MUST_STAY_MUCH_LONGER {\n  font-size: smaller;\n  color: aqua;\n}\n\n";
		tos<<"div.TEACHER_HAS_A_FREE_DAY {\n  font-size: smaller;\n  color: red;\n}\n\n";
		tos<<"div.TEACHER_IS_NOT_AVAILABLE {\n  font-size: smaller;\n  color: olive;\n}\n\n";
	}
	tos<<"/* "<<TimetableExport::tr("End of file.")<<" */\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(cssfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeSubgroupsTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
        tos<<"      <li>\n        "<<TimetableExport::tr("Year")<<" "<<utils::strings::parseStrForHtml(sty->name)<<"\n        <ul>\n";
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
            tos<<"          <li>\n            "<<TimetableExport::tr("Group")<<" "<<utils::strings::parseStrForHtml(stg->name)<<": \n";
			for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
                tos<<"              <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(sts->name)<<"\">"<<utils::strings::parseStrForHtml(sts->name)<<"</a>\n";
			}
			tos<<"          </li>\n";
		}
		tos<<"        </ul>\n      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size(); subgroup++){
        tos<<singleSubgroupsTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, subgroup, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeSubgroupsTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
        tos<<"      <li>\n        "<<TimetableExport::tr("Year")<<" "<<utils::strings::parseStrForHtml(sty->name)<<"\n        <ul>\n";
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
            tos<<"          <li>\n            "<<TimetableExport::tr("Group")<<" "<<utils::strings::parseStrForHtml(stg->name)<<": \n";
			for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
                tos<<"              <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(sts->name)<<"\">"<<utils::strings::parseStrForHtml(sts->name)<<"</a>\n";
			}
			tos<<"          </li>\n";
		}
		tos<<"        </ul>\n      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n";

    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size(); subgroup++){
        tos<<singleSubgroupsTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, subgroup, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubgroupsTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos << singleSubgroupsTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.directSubgroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}	
	file.close();
}

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeSubgroupsTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos << singleSubgroupsTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.directSubgroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

// by Volker Dirr
void TimetableExport::writeSubgroupsTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleSubgroupsTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.directSubgroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}	
	file.close();
}

// by Volker Dirr
void TimetableExport::writeSubgroupsTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBGROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();
	
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleSubgroupsTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.directSubgroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Now print the groups
//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
        tos<<"      <li>\n        "<<TimetableExport::tr("Year")<<" "<<utils::strings::parseStrForHtml(sty->name)<<"\n        <ul>\n";
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
			tos<<"          <li>\n            "<<TimetableExport::tr("Group");
            tos<<" <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(stg->name)<<"\">"<<utils::strings::parseStrForHtml(stg->name)<<"</a>\n";
			tos<<"          </li>\n";
		}
		tos<<"        </ul>\n      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    for(int group=0; group<TContext::get()->instance.internalGroupsList.size(); group++){
        tos << singleGroupsTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, group, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
        tos<<"      <li>\n        "<<TimetableExport::tr("Year")<<" "<<utils::strings::parseStrForHtml(sty->name)<<"\n        <ul>\n";
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
			tos<<"          <li>\n            "<<TimetableExport::tr("Group");
            tos<<" <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(stg->name)<<"\">"<<utils::strings::parseStrForHtml(stg->name)<<"</a>\n";
			tos<<"          </li>\n";
		}
		tos<<"        </ul>\n      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n";

    for(int group=0; group<TContext::get()->instance.internalGroupsList.size(); group++){
        tos<<singleGroupsTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, group, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos<<singleGroupsTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.internalGroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos<<singleGroupsTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.internalGroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleGroupsTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.internalGroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeGroupsTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_GROUPS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();
	
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleGroupsTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.internalGroupsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Now print the years

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size(); year++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
		tos<<"      <li>\n        "<<TimetableExport::tr("Year");
        tos<<" <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(sty->name)<<"\">"<<utils::strings::parseStrForHtml(sty->name)<<"</a>\n";
		tos<<"      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size(); year++){
        tos << singleYearsTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, year, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size(); year++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
		tos<<"      <li>\n        "<<TimetableExport::tr("Year");
        tos<<" <a href=\""<<"#table_"<<hashStudentIDsTimetable.value(sty->name)<<"\">"<<utils::strings::parseStrForHtml(sty->name)<<"</a>\n";
		tos<<"      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size(); year++){
        tos << singleYearsTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, year, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos<<singleYearsTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.augmentedYearsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	
	QSet<int> tmp;
    tos<<singleYearsTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.augmentedYearsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleYearsTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.augmentedYearsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeYearsTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_YEARS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();
	
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleYearsTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.augmentedYearsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::PRINT_DETAILED_HTML_TIMETABLES, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Print all activities

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
    tos<<singleAllActivitiesTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
    tos<<singleAllActivitiesTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);

    tos<<singleAllActivitiesTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);

    tos<<singleAllActivitiesTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        tos<<singleAllActivitiesTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeAllActivitiesTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::defs::WRITE_TIMETABLES_ACTIVITIES){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        tos<<singleAllActivitiesTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Print the teachers

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeTeachersTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);
	
	tos<<writeHead(true, placedActivities, true);
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        QString teacher_name = teacher->name;
        tos<<"      <li><a href=\""<<"#table_"<<hashTeacherIDsTimetable.value(teacher_name)<<"\">"<<utils::strings::parseStrForHtml(teacher_name)<<"</a></li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    int i=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        Q_UNUSED(teacher)
        tos<<singleTeachersTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, i, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
        i++;
	}
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeTeachersTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true); 
		
	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";

    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        QString teacher_name = teacher->name;
        tos<<"      <li><a href=\""<<"#table_"<<hashTeacherIDsTimetable.value(teacher_name)<<"\">"<<utils::strings::parseStrForHtml(teacher_name)<<"</a></li>\n";
	}

	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    int i=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        Q_UNUSED(teacher)
        tos<<singleTeachersTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, i, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
        i++;
	}

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeTeachersTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	QSet<int> tmp;
    tos<<singleTeachersTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.teachersList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code modified by Volker Dirr (timetabling.de) from old html generation code
void TimetableExport::writeTeachersTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);
	QSet<int> tmp;
    tos<<singleTeachersTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.teachersList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//by Volker Dirr
void TimetableExport::writeTeachersTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleTeachersTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.teachersList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//by Volker Dirr
void TimetableExport::writeTeachersTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleTeachersTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.teachersList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Print the subjects

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);

	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";

    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        tos<<"      <li>\n        "<<TimetableExport::tr("Subject");
        tos<<" <a href=\""<<"#table_"<<hashSubjectIDsTimetable.value(subject->name)<<"\">"<<subject->name<<"</a>\n";
		tos<<"      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    int i=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        Q_UNUSED(subject)
        tos<<singleSubjectsTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, i, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
        i++;
    }
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);

	tos<<"    <p><strong>"<<TimetableExport::tr("Table of contents")<<"</strong></p>\n";
	tos<<"    <ul>\n";

    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
		tos<<"      <li>\n        "<<TimetableExport::tr("Subject");
        tos<<" <a href=\""<<"#table_"<<hashSubjectIDsTimetable.value(subject->name)<<"\">"<<subject->name<<"</a>\n";
		tos<<"      </li>\n";
	}
	tos<<"    </ul>\n    <p>&nbsp;</p>\n\n";

    int i=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        Q_UNUSED(subject)
        tos<<singleSubjectsTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, i, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
        i++;
	}
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableTimeVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);

	QSet<int> tmp;
    tos<<singleSubjectsTimetableTimeVerticalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.subjectsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableTimeHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, false);

	QSet<int> tmp;
    tos<<singleSubjectsTimetableTimeHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, TContext::get()->instance.subjectsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableTimeVerticalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_VERTICAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleSubjectsTimetableTimeVerticalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.subjectsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}

	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeSubjectsTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_SUBJECTS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	tos<<writeTOCDays();

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		QSet<int> tmp;
        tos<<singleSubjectsTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, TContext::get()->instance.subjectsList.size(), tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
	}
	tos << "  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

////XHTML generation code by Volker Dirr (http://timetabling.de/)
//void TimetableExport::writeActivityTagsTimetableTimeHorizontalDailyHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
//	assert(Timetable::getInstance()->rules.initialized && Timetable::getInstance()->rules.internalStructureComputed);
//	assert(Timetable::getInstance()->bestSolution().students_schedule_ready && Timetable::getInstance()->bestSolution().teachers_schedule_ready);

//	if(!defs::WRITE_TIMETABLES_TIME_HORIZONTAL || !defs::WRITE_TIMETABLES_ACTIVITY_TAGS){
//		if(QFile::exists(htmlfilename))
//			QFile::remove(htmlfilename);

//		return;
//	}

//	//Now we print the results to an HTML file
//	QFile file(htmlfilename);
//	if(!file.open(QIODevice::WriteOnly)){
//		MessagesManager::critical(parent, tr("m-FET critical"),
//		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
//		return;
//	}
//	QTextStream tos(&file);
//	tos.setCodec("UTF-8");
//	tos.setGenerateByteOrderMark(true);

//	tos<<writeHead(true, placedActivities, true);
//	tos<<writeTOCDays();

//	for(int day=0; day<Timetable::getInstance()->rules.nDaysPerWeek; day++){
//		QSet<int> tmp;
//		tos<<singleActivityTagsTimetableTimeHorizontalDailyHtml(defs::TIMETABLE_HTML_LEVEL, day, Timetable::getInstance()->rules.nInternalActivityTags, tmp, saveTime, defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS, defs::TIMETABLE_HTML_REPEAT_NAMES);

//		tos<<"    <p class=\"back\"><a href=\""<<"#top\">"<<TimetableExport::tr("back to the top")<<"</a></p>\n\n";
//	}
//	tos << "  </body>\n</html>\n";

//	if(file.error()>0){
//		MessagesManager::critical(parent, tr("m-FET critical"),
//		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
//	}
//	file.close();
//}

//Print the teachers free periods. Code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeTeachersFreePeriodsTimetableDaysHorizontalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_HORIZONTAL || !defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);

	tos<<"    <div class=\"TEACHER_HAS_SINGLE_GAP\">"<<TimetableExport::tr("Teacher has a single gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_BORDER_GAP\">"<<TimetableExport::tr("Teacher has a border gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_BIG_GAP\">"<<TimetableExport::tr("Teacher has a big gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_COME_EARLIER\">"<<TimetableExport::tr("Teacher must come earlier")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_COME_MUCH_EARLIER\">"<<TimetableExport::tr("Teacher must come much earlier")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_STAY_LONGER\">"<<TimetableExport::tr("Teacher must stay longer")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_STAY_MUCH_LONGER\">"<<TimetableExport::tr("Teacher must stay much longer")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_A_FREE_DAY\">"<<TimetableExport::tr("Teacher has a free day")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_IS_NOT_AVAILABLE\">"<<TimetableExport::tr("Teacher is not available")<<"</div>\n";

	tos<<"    <p>&nbsp;</p>\n\n";
	
    tos<<singleTeachersFreePeriodsTimetableDaysHorizontalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS, defs::TIMETABLE_HTML_REPEAT_NAMES);
	
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//XHTML generation code by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeTeachersFreePeriodsTimetableDaysVerticalHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_DAYS_VERTICAL || !defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);

	tos<<"    <div class=\"TEACHER_HAS_SINGLE_GAP\">"<<TimetableExport::tr("Teacher has a single gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_BORDER_GAP\">"<<TimetableExport::tr("Teacher has a border gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_BIG_GAP\">"<<TimetableExport::tr("Teacher has a big gap")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_COME_EARLIER\">"<<TimetableExport::tr("Teacher must come earlier")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_COME_MUCH_EARLIER\">"<<TimetableExport::tr("Teacher must come much earlier")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_STAY_LONGER\">"<<TimetableExport::tr("Teacher must stay longer")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_MUST_STAY_MUCH_LONGER\">"<<TimetableExport::tr("Teacher must stay much longer")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_HAS_A_FREE_DAY\">"<<TimetableExport::tr("Teacher has a free day")<<"</div>\n";
	tos<<"    <div class=\"TEACHER_IS_NOT_AVAILABLE\">"<<TimetableExport::tr("Teacher is not available")<<"</div>\n";

	tos<<"    <p>&nbsp;</p>\n\n";

    tos<<singleTeachersFreePeriodsTimetableDaysVerticalHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS, defs::TIMETABLE_HTML_REPEAT_NAMES);

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Code contributed by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeTeachersStatisticsHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
	
    if(!defs::WRITE_TIMETABLES_STATISTICS || !defs::WRITE_TIMETABLES_TEACHERS){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	
	bool PRINT_DETAILED=true;
    tos<<singleTeachersStatisticsHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, PRINT_DETAILED, defs::TIMETABLE_HTML_REPEAT_NAMES, true);
	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//Code contributed by Volker Dirr (http://timetabling.de/)
void TimetableExport::writeStudentsStatisticsHtml(QWidget* parent, const QString& htmlfilename, const QString& saveTime, int placedActivities){
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

    if(!defs::WRITE_TIMETABLES_STATISTICS || !(defs::WRITE_TIMETABLES_YEARS || defs::WRITE_TIMETABLES_GROUPS || defs::WRITE_TIMETABLES_SUBGROUPS) ){
		if(QFile::exists(htmlfilename))
			QFile::remove(htmlfilename);

		return;
	}

	//Now we print the results to an HTML file
	QFile file(htmlfilename);
	if(!file.open(QIODevice::WriteOnly)){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(htmlfilename));
		return;
	}
	QTextStream tos(&file);
	tos.setCodec("UTF-8");
	tos.setGenerateByteOrderMark(true);

	tos<<writeHead(true, placedActivities, true);
	bool PRINT_DETAILED=true;
    tos<<singleStudentsStatisticsHtml(defs::TIMETABLE_HTML_LEVEL, saveTime, PRINT_DETAILED, defs::TIMETABLE_HTML_REPEAT_NAMES, true);

	tos<<"  </body>\n</html>\n";

	if(file.error()>0){
        MessagesManager::critical(parent, tr("m-FET critical"),
		 TimetableExport::tr("Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(htmlfilename).arg(file.error()));
	}
	file.close();
}

//------------------------------------------------------------------
//------------------------------------------------------------------

void TimetableExport::computeHashForIDsTimetable(){
// by Volker Dirr

//TODO This is unneeded if you use a relational data base, because we can use the primary key id of the database.
//This is very similar to statistics compute hash. So always check it if you change something here!

/*	QSet<QString> usedStudents;
    for(int i=0; i<Timetable::getInstance()->rules.activeActivitiesList.size(); i++){
        foreach(QString st, Timetable::getInstance()->rules.internalActivitiesList[i].studentsNames){
			if(!usedStudents.contains(st))
				usedStudents<<st;
		}
	}*/
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);
	hashStudentIDsTimetable.clear();
	int cnt=1;
    for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
		//if(usedStudents.contains(sty->name)){
		if(!hashStudentIDsTimetable.contains(sty->name)){
            hashStudentIDsTimetable.insert(sty->name, utils::strings::number(cnt));
			cnt++;
		}
		//}
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
		//	if(usedStudents.contains(stg->name)){
			if(!hashStudentIDsTimetable.contains(stg->name)){
                hashStudentIDsTimetable.insert(stg->name, utils::strings::number(cnt));
				cnt++;
			}
		//	}
			for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
		//		if(usedStudents.contains(sts->name)){
				if(!hashStudentIDsTimetable.contains(sts->name)){
                    hashStudentIDsTimetable.insert(sts->name, utils::strings::number(cnt));
					cnt++;
				}
		//		}
			}
		}
	}

    int i=0;
	hashSubjectIDsTimetable.clear();
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        hashSubjectIDsTimetable.insert(subject->name, utils::strings::number(i+1));
        i++;
	}

    i=0;
	hashTeacherIDsTimetable.clear();
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        hashTeacherIDsTimetable.insert(teacher->name, utils::strings::number(i+1));
        i++;
	}

	hashDayIDsTimetable.clear();
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        hashDayIDsTimetable.insert(TContext::get()->instance.daysOfTheWeek[day], utils::strings::number(day+1));
	}
    if(defs::TIMETABLE_HTML_LEVEL==7){
		computeHashActivityColorBySubject();
		computeHashActivityColorBySubjectAndStudents();
	}
}

//By Liviu, with ideas from Volker
void TimetableExport::computeHashActivityColorBySubject(){
	QHash<QString, int> tmpHash;

	hashActivityColorBySubject.clear();
	activeHashActivityColorBySubject.clear();
	
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

	QSet<QString> alreadyAdded;
	
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        if(TContext::get()->bestSolution().times[i]!=UNALLOCATED_TIME){
            Activity* act=TContext::get()->instance.activeActivitiesList[i];
			QString tmpString=act->subjectName;
			if(!alreadyAdded.contains(tmpString)){
				alreadyAdded.insert(tmpString);
				hashActivityColorBySubject.insert(i, alreadyAdded.count());
				activeHashActivityColorBySubject.append(i);
				tmpHash.insert(tmpString, alreadyAdded.count());
			}
			else{
				assert(tmpHash.contains(tmpString));
				int k=tmpHash.value(tmpString);
				hashActivityColorBySubject.insert(i, k);
			}
		}
	}
	
	//cout<<"hashActivityColorBySubject.count()=="<<hashActivityColorBySubject.count()<<endl;
}

//By Liviu, with ideas from Volker
void TimetableExport::computeHashActivityColorBySubjectAndStudents(){
	QHash<QString, int> tmpHash;

	hashActivityColorBySubjectAndStudents.clear();
	activeHashActivityColorBySubjectAndStudents.clear();
	
    assert(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed);
    assert(TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready);

	QSet<QString> alreadyAdded;
	
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
        if(TContext::get()->bestSolution().times[i]!=UNALLOCATED_TIME){
            Activity* act=TContext::get()->instance.activeActivitiesList[i];
			
            QString tmpString=act->subjectName+" "+act->studentSetsNames.join(",");
			if(!alreadyAdded.contains(tmpString)){
				alreadyAdded.insert(tmpString);
				hashActivityColorBySubjectAndStudents.insert(i, alreadyAdded.count());
				activeHashActivityColorBySubjectAndStudents.append(i);
				tmpHash.insert(tmpString, alreadyAdded.count());
			}
			else{
				assert(tmpHash.contains(tmpString));
				int k=tmpHash.value(tmpString);
				hashActivityColorBySubjectAndStudents.insert(i, k);
			}
		}
	}
	
	//cout<<"hashActivityColorBySubjectAndStudents.count()=="<<hashActivityColorBySubjectAndStudents.count()<<endl;
}

/*void TimetableExport::computeHashForColors(QHash<QString, QString>& hashColorStringIDsTimetable){
// by Volker Dirr
	qWarning("compute hash for colors");
	hashColorStringIDsTimetable.clear();
    assert(Timetable::getInstance()->rules.initialized && Timetable::getInstance()->rules.internalStructureComputed);
    assert(Timetable::getInstance()->bestSolution().students_schedule_ready && Timetable::getInstance()->bestSolution().teachers_schedule_ready);
	QSet<QString> alreadyAddedString;
    for(int i=0; i<Timetable::getInstance()->rules.activeActivitiesList.size(); i++) {
        Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[i];
        if(Timetable::getInstance()->bestSolution().times[i]!=UNALLOCATED_TIME) {
			qWarning("add a hash");
			//coloring for students
			QString tmpString=act->subjectName;
			if(!alreadyAddedString.contains(tmpString)){
				alreadyAddedString<<tmpString;
                hashColorStringIDsTimetable.insert(tmpString, utils::strings::number(alreadyAddedString.count()));
			}
			//coloring for teachers
			tmpString=act->subjectName+" "+act->studentsNames.join(", ");
			if(!alreadyAddedString.contains(tmpString)){
				alreadyAddedString<<tmpString;
                hashColorStringIDsTimetable.insert(tmpString, utils::strings::number(alreadyAddedString.count()));
			}
			//coloring for rooms
//			it is similar to students
//			tmpString=act->subjectName+" "+act->studentsNames.join(", ");
//			if(!alreadyAddedString.contains(tmpString)){
//				alreadyAddedString<<tmpString;
//				hashColorStringIDsTimetable.insert(tmpString, utils::strings::number(alreadyAddedString.count()));
//			}
		}
	}
}*/

void TimetableExport::computeActivitiesAtTime(){		// by Liviu Lalescu
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++)
			activitiesAtTime[day][hour].clear();
    for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++) {		//maybe TODO: maybe it is better to do this sorted by students or teachers?
        Activity* act=TContext::get()->instance.activeActivitiesList[i];
        if(TContext::get()->bestSolution().times[i]!=UNALLOCATED_TIME) {
            int hour=TContext::get()->bestSolution().getHourForActivityIndex(i);
            int day=TContext::get()->bestSolution().getDayForActivityIndex(i);

            for(int dd=0; dd < act->duration && hour+dd < TContext::get()->instance.nHoursPerDay; dd++)
				activitiesAtTime[day][hour+dd].append(i);
		}
	}
}


void TimetableExport::computeActivitiesWithSameStartingTime(){
// by Volker Dirr
	activitiesWithSameStartingTime.clear();

    if(defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME){
        for(int i=0; i<TContext::get()->instance.validTimeConstraintsList.size(); i++){
            TimeConstraint* tc=TContext::get()->instance.validTimeConstraintsList.at(i);
			if(tc->type==CONSTRAINT_ACTIVITIES_SAME_STARTING_TIME){ //not needed anymore:  && tc->weightPercentage==100
				ConstraintActivitiesSameStartingTime* c=(ConstraintActivitiesSameStartingTime*) tc;
				for(int a=0; a<c->_n_activities; a++){
					//speed improvement
					QList<int> & tmpList=activitiesWithSameStartingTime[c->_activities[a]];
					for(int b=0; b<c->_n_activities; b++){
						if(a!=b){
                            if(TContext::get()->bestSolution().times[c->_activities[a]]==TContext::get()->bestSolution().times[c->_activities[b]]){ 	//because constraint is maybe not with 100% weight and failed
								if(!tmpList.contains(c->_activities[b])){
									tmpList<<c->_activities[b];
								}
							}
						}
					}
					/*
					QList<int> tmpList;
					if(activitiesWithSameStartingTime.contains(c->_activities[a]))
						tmpList=activitiesWithSameStartingTime.value(c->_activities[a]);
					for(int b=0; b<c->_n_activities; b++){
						if(a!=b){
                            if(Timetable::getInstance()->bestSolution().times[c->_activities[a]]==Timetable::getInstance()->bestSolution().times[c->_activities[b]]){ 	//because constraint is maybe not with 100% weight and failed
								if(!tmpList.contains(c->_activities[b])){
									tmpList<<c->_activities[b];
								}
							}
						}
					}
					activitiesWithSameStartingTime.insert(c->_activities[a], tmpList);
					*/
				}
			}
		}
	}
}

bool TimetableExport::addActivitiesWithSameStartingTime(QList<int>& allActivities, int hour){
// by Volker Dirr
    if(defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME){
		bool activitiesWithSameStartingtime=false;
		QList<int> allActivitiesNew;
		foreach(int tmpAct, allActivities){
			allActivitiesNew<<tmpAct;
			if(activitiesWithSameStartingTime.contains(tmpAct)){
				QList<int> sameTimeList=activitiesWithSameStartingTime.value(tmpAct);
				foreach(int sameTimeAct, sameTimeList){
					if(!allActivitiesNew.contains(sameTimeAct) && !allActivities.contains(sameTimeAct)){
                        if(TContext::get()->bestSolution().times[sameTimeAct]!=UNALLOCATED_TIME){
                            Activity* act=TContext::get()->instance.activeActivitiesList[sameTimeAct];
                            assert(TContext::get()->bestSolution().times[tmpAct]==TContext::get()->bestSolution().times[sameTimeAct]);//{
                                if((TContext::get()->bestSolution().getHourForActivityIndex(sameTimeAct) + (act->duration-1))>=hour){
									allActivitiesNew<<sameTimeAct;
								}
								activitiesWithSameStartingtime=true; //don't add this line in previous if command because of activities with different duration!
							//}
						}
					}
				}
			}
		}
		//allActivities.clear();
		allActivities=allActivitiesNew;
		//allActivitiesNew.clear();
		return activitiesWithSameStartingtime;
	}
	else
		return false;
}

// by Volker Dirr
QString TimetableExport::writeHead(bool java, int placedActivities, bool printInstitution){
	QString tmp;
	tmp+="<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
	tmp+="  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n";

    if(!defs::LANGUAGE_STYLE_RIGHT_TO_LEFT)
        tmp+="<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\""+defs::LANGUAGE_FOR_HTML+"\" xml:lang=\""+defs::LANGUAGE_FOR_HTML+"\">\n";
	else
        tmp+="<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\""+defs::LANGUAGE_FOR_HTML+"\" xml:lang=\""+defs::LANGUAGE_FOR_HTML+"\" dir=\"rtl\">\n";
	tmp+="  <head>\n";
    tmp+="    <title>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</title>\n";
	tmp+="    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
    if(defs::TIMETABLE_HTML_LEVEL>=1){
        QString cssfilename=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
		
		if(cssfilename.right(4)==".m-fet")
			cssfilename=cssfilename.left(cssfilename.length()-4);
        //else if(defs::INPUT_FILENAME_XML!="")
		//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
		
		cssfilename+="_"+STYLESHEET_CSS;
        if(defs::INPUT_FILENAME_XML=="")
			cssfilename=STYLESHEET_CSS;
		tmp+="    <link rel=\"stylesheet\" media=\"all\" href=\""+cssfilename+"\" type=\"text/css\" />\n";
	}
	if(java){
        if(defs::TIMETABLE_HTML_LEVEL>=5 && defs::TIMETABLE_HTML_LEVEL!=7){  // the following JavaScript code is pretty similar to an example of Les Richardson
			tmp+="    <meta http-equiv=\"Content-Script-Type\" content=\"text/javascript\" />\n";
			tmp+="    <script type=\"text/javascript\">\n";
			tmp+="      function highlight(classval) {\n";
			tmp+="        var spans = document.getElementsByTagName('span');\n";
			tmp+="        for(var i=0; spans.length>i; i++) {\n";
			tmp+="          if (spans[i].className == classval) {\n";
			tmp+="            spans[i].style.backgroundColor = 'lime';\n";
			tmp+="          } else {\n";
			tmp+="            spans[i].style.backgroundColor = 'white';\n";
			tmp+="          }\n";
			tmp+="        }\n";
			tmp+="      }\n";
			tmp+="    </script>\n";
		}
	}
	tmp+="  </head>\n\n";
	tmp+="  <body id=\"top\">\n";
    if(placedActivities!=TContext::get()->instance.activeActivitiesList.size())
        tmp+="    <h1>"+TimetableExport::tr("Warning! Only %1 out of %2 activities placed!").arg(placedActivities).arg(TContext::get()->instance.activeActivitiesList.size())+"</h1>\n";
	if(printInstitution){
        tmp+="    <table>\n      <tr align=\"left\" valign=\"top\">\n        <th>"+TimetableExport::tr("Institution name")+":</th>\n        <td>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</td>\n      </tr>\n    </table>\n";
        tmp+="    <table>\n      <tr align=\"left\" valign=\"top\">\n        <th>"+TimetableExport::tr("Comments")+":</th>\n        <td>"+utils::strings::parseStrForHtml(TContext::get()->instance.comments).replace(QString("\n"), QString("<br />\n"))+"</td>\n      </tr>\n    </table>\n";
	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeTOCDays(){
	QString tmp;
	tmp+="    <p><strong>"+TimetableExport::tr("Table of contents")+"</strong></p>\n";
	tmp+="    <ul>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmp+="      <li>\n        ";
        tmp+=" <a href=\"#table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</a>\n";
		tmp+="          </li>\n";
	}
	tmp+="    </ul>\n    <p>&nbsp;</p>\n";
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeStartTagTDofActivities(int htmlLevel, const Activity* act, bool detailed, bool colspan, bool rowspan, int colorBy){
	QString tmp;
	assert(!(colspan && rowspan));
	if(detailed)
		assert(!colspan && !rowspan);
	else
		tmp+="          ";
	tmp+="<td";
	if(rowspan && act->duration>1)
        tmp+=" rowspan=\""+utils::strings::number(act->duration)+"\"";
	if(colspan && act->duration>1)
        tmp+=" colspan=\""+utils::strings::number(act->duration)+"\"";
	if(htmlLevel==6){
		tmp+=" class=\"";
		if(act->subjectName.size()>0){
			tmp+="s_"+hashSubjectIDsTimetable.value(act->subjectName);
		}

		if(act->studentSetsNames.size()>0){
			foreach(QString st, act->studentSetsNames)
				tmp+=" ss_"+hashStudentIDsTimetable.value(st);
		}
        //TODO:
//		if(act->teacherName.size()>0){
//			foreach(QString t, act->teacherName)
//				tmp+=" t_"+hashTeacherIDsTimetable.value(t);
//		}

		if(detailed)
			tmp+=" detailed";
		tmp+="\"";
	}
	if(htmlLevel==7){
        assert(TContext::get()->instance.containsActivityInActiveList(act->id));
        int index=TContext::get()->instance.getActivityIndex(act->id);
		switch(colorBy){
            case COLOR_BY_SUBJECT_STUDENTS:
                tmp+=" class=\"c_"+QString::number(activeHashActivityColorBySubject.count()+hashActivityColorBySubjectAndStudents.value(index));
            break;
            case COLOR_BY_SUBJECT:
                tmp+=" class=\"c_"+QString::number(hashActivityColorBySubject.value(index));
            break;
            default:
                assert(0==1);
        }
		
		if(detailed)
			tmp+=" detailed";
		tmp+="\"";
	}
	if(detailed && htmlLevel>=1 && htmlLevel<=5)
		tmp+=" class=\"detailed\"";
	tmp+=">";
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeSubjectAndActivityTags(int htmlLevel, const Activity* act, const QString& startTag, const QString& startTagAttribute, bool activityTagsOnly, bool printActivityTags){
	QString tmp;
	if(act->subjectName.size()>0||act->activityTagsNames.size()>0){
		if(startTag=="div" && htmlLevel>=3)
			tmp+="<"+startTag+startTagAttribute+">";
		if(act->subjectName.size()>0 && !activityTagsOnly){
			switch(htmlLevel){
                case 3 : tmp+="<span class=\"subject\">"+utils::strings::parseStrForHtml(act->subjectName)+"</span>"; break;
                case 4 : tmp+="<span class=\"subject\"><span class=\"s_"+hashSubjectIDsTimetable.value(act->subjectName)+"\">"+utils::strings::parseStrForHtml(act->subjectName)+"</span></span>"; break;
				case 5 : ;
                case 6 : tmp+="<span class=\"subject\"><span class=\"s_"+hashSubjectIDsTimetable.value(act->subjectName)+"\" onmouseover=\"highlight('s_"+hashSubjectIDsTimetable.value(act->subjectName)+"')\">"+utils::strings::parseStrForHtml(act->subjectName)+"</span></span>"; break;
                case 7 : tmp+="<span class=\"subject\">"+utils::strings::parseStrForHtml(act->subjectName)+"</span>"; break;
                default: tmp+=utils::strings::parseStrForHtml(act->subjectName); break;
			}
			if(act->activityTagsNames.size()>0 && printActivityTags){
				tmp+=" ";
			}
		}
		if(startTag=="div"){
			if(htmlLevel>=3)
				tmp+="</div>";
			else tmp+="<br />";
		}
	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeStudents(int htmlLevel, const Activity* act, const QString& startTag, const QString& startTagAttribute){
	QString tmp;
	if(act->studentSetsNames.size()>0){
		if(startTag=="div" && htmlLevel>=3)
			tmp+="<"+startTag+startTagAttribute+">";
		foreach(QString st, act->studentSetsNames){
			switch(htmlLevel){
                case 4 : tmp+="<span class=\"ss_"+hashStudentIDsTimetable.value(st)+"\">"+utils::strings::parseStrForHtml(st)+"</span>"; break;
				case 5 : ;
                case 6 : tmp+="<span class=\"ss_"+hashStudentIDsTimetable.value(st)+"\" onmouseover=\"highlight('ss_"+hashStudentIDsTimetable.value(st)+"')\">"+utils::strings::parseStrForHtml(st)+"</span>"; break;
                default: tmp+=utils::strings::parseStrForHtml(st); break;
			}
			tmp+=", ";
		}
		tmp.remove(tmp.size()-2, 2);
		if(startTag=="div"){
			if(htmlLevel>=3)
				tmp+="</div>";
			else tmp+="<br />";
		}
	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeTeachers(int htmlLevel, const Activity* act, const QString& startTag, const QString& startTagAttribute){
    Q_UNUSED(htmlLevel)Q_UNUSED(act)Q_UNUSED(startTagAttribute)Q_UNUSED(startTag)

	QString tmp;
    //TODO:
//	if(act->teacherName.size()>0){
//		if(startTag=="div" && htmlLevel>=3)
//			tmp+="<"+startTag+startTagAttribute+">";
//		foreach(QString t, act->teacherName){
//			switch(htmlLevel){
//				case 4 : tmp+="<span class=\"t_"+hashTeacherIDsTimetable.value(t)+"\">"+utils::strings::parseStrForHtml(t)+"</span>"; break;
//				case 5 : ;
//				case 6 : tmp+="<span class=\"t_"+hashTeacherIDsTimetable.value(t)+"\" onmouseover=\"highlight('t_"+hashTeacherIDsTimetable.value(t)+"')\">"+utils::strings::parseStrForHtml(t)+"</span>"; break;
//				default: tmp+=utils::strings::parseStrForHtml(t); break;
//			}
//			tmp+=", ";
//		}
//		tmp.remove(tmp.size()-2, 2);
//		if(startTag=="div"){
//			if(htmlLevel>=3)
//				tmp+="</div>";
//			else tmp+="<br />";
//		}
//	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeNotAvailableSlot(int htmlLevel, const QString& weight){
	QString tmp;
	//weight=" "+weight;
	switch(htmlLevel){
		case 3 : ;
        case 4 : tmp="          <td class=\"notAvailable\"><span class=\"notAvailable\">"+utils::strings::parseStrForHtml(STRING_NOT_AVAILABLE_TIME_SLOT)+weight+"</span></td>\n"; break;
		case 5 : ;
        case 6 : tmp="          <td class=\"notAvailable\"><span class=\"notAvailable\" onmouseover=\"highlight('notAvailable')\">"+utils::strings::parseStrForHtml(STRING_NOT_AVAILABLE_TIME_SLOT)+weight+"</span></td>\n"; break;
        case 7 : tmp="          <td class=\"notAvailable\"><span class=\"notAvailable\">"+utils::strings::parseStrForHtml(STRING_NOT_AVAILABLE_TIME_SLOT)+weight+"</span></td>\n"; break;
        default: tmp="          <td>"+utils::strings::parseStrForHtml(STRING_NOT_AVAILABLE_TIME_SLOT)+weight+"</td>\n";
	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeBreakSlot(int htmlLevel, const QString& weight){
	QString tmp;
	//weight=" "+weight;
	switch(htmlLevel){
		case 3 : ;
        case 4 : tmp="          <td class=\"break\"><span class=\"break\">"+utils::strings::parseStrForHtml(STRING_BREAK_SLOT)+weight+"</span></td>\n"; break;
		case 5 : ;
        case 6 : tmp="          <td class=\"break\"><span class=\"break\" onmouseover=\"highlight('break')\">"+utils::strings::parseStrForHtml(STRING_BREAK_SLOT)+weight+"</span></td>\n"; break;
        case 7 : tmp="          <td class=\"break\"><span class=\"break\">"+utils::strings::parseStrForHtml(STRING_BREAK_SLOT)+weight+"</span></td>\n"; break;
        default: tmp="          <td>"+utils::strings::parseStrForHtml(STRING_BREAK_SLOT)+weight+"</td>\n";
	}
	return tmp;
}

// by Volker Dirr
QString TimetableExport::writeEmpty(int htmlLevel){
	QString tmp;
	switch(htmlLevel){
		case 3 : ;
        case 4 : tmp="          <td class=\"empty\"><span class=\"empty\">"+utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)+"</span></td>\n"; break;
		case 5 : ;
        case 6 : tmp="          <td class=\"empty\"><span class=\"empty\" onmouseover=\"highlight('empty')\">"+utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)+"</span></td>\n"; break;
        case 7 : tmp="          <td class=\"empty\"><span class=\"empty\">"+utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)+"</span></td>\n"; break;
        default: tmp="          <td>"+utils::strings::parseStrForHtml(STRING_EMPTY_SLOT)+"</td>\n";
	}
	return tmp;
}

//by Volker Dirr
QString TimetableExport::writeActivityStudents(int htmlLevel, int ai, int day, int hour, bool notAvailable, bool colspan, bool rowspan, bool printActivityTags, QString skipStudentsSet){
	QString tmp;
    int currentTime=day+TContext::get()->instance.nDaysPerWeek*hour;
	if(ai!=UNALLOCATED_ACTIVITY){
        if(TContext::get()->bestSolution().times[ai]==currentTime){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, false, colspan, rowspan, COLOR_BY_SUBJECT);
			//TODO line0
			bool skipLine0=false;
			if(act->studentSetsNames.size()==1){
				if(act->studentSetsNames.at(0)==skipStudentsSet){
					skipLine0=true;
				}
			}
			if(!skipLine0){
				tmp+=writeStudents(htmlLevel, act, "div", " class=\"studentsset line0\"");
			}
			tmp+=writeSubjectAndActivityTags(htmlLevel, act, "div", " class=\"line1\"", false, printActivityTags);
			tmp+=writeTeachers(htmlLevel, act, "div", " class=\"teacher line2\"");
//			tmp+=writeRoom(htmlLevel, ai, "div", " class=\"room line3\"");
			tmp+="</td>\n";
		} else
			tmp+="          <!-- span -->\n";
	} else {
        if(notAvailable && defs::PRINT_NOT_AVAILABLE_TIME_SLOTS){
			tmp+=writeNotAvailableSlot(htmlLevel, "");
		}
        else if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
			tmp+=writeBreakSlot(htmlLevel, "");
		}
		else{
			tmp+=writeEmpty(htmlLevel);
		}
	}
	return tmp;
}

//by Volker Dirr
QString TimetableExport::writeActivitiesStudents(int htmlLevel, const QList<int>& allActivities, bool printActivityTags){
	QString tmp;
	if(htmlLevel>=1)
		tmp+="          <td><table class=\"detailed\">";
	else
		tmp+="          <td><table>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"studentsset line0\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT)+writeStudents(htmlLevel, act, "", "")+"</td>";
		}
	}
	tmp+="</tr>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"line1\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT)+writeSubjectAndActivityTags(htmlLevel, act, "", "", false, printActivityTags)+"</td>";
		}
	}
	tmp+="</tr>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"teacher line2\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT)+writeTeachers(htmlLevel, act, "", "")+"</td>";
		}
	}
	tmp+="</tr>";
//	if(htmlLevel>=3)
//		tmp+="<tr class=\"room line3\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT)+writeRoom(htmlLevel, ai, "", "")+"</td>";
//		}
//	}
//	tmp+="</tr>";
	tmp+="</table></td>\n";
	return tmp;
}

//by Volker Dirr
QString TimetableExport::writeActivityTeacher(int htmlLevel, int teacher, int day, int hour, bool colspan, bool rowspan, bool printActivityTags, QString skipTeacher){
    Q_UNUSED(skipTeacher)
    QString tmp;
    int ai=TContext::get()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
    int currentTime=day+TContext::get()->instance.nDaysPerWeek*hour;
	if(ai!=UNALLOCATED_ACTIVITY){
        if(TContext::get()->bestSolution().times[ai]==currentTime){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, false, colspan, rowspan, COLOR_BY_SUBJECT_STUDENTS);
			//TODO line0
			bool skipLine0=false;
            //TODO:
//			if(act->teacherName.size()==1){
//				if(act->teacherName.at(0)==skipTeacher){
//					skipLine0=true;
//				}
//			}
			if(!skipLine0){
				tmp+=writeTeachers(htmlLevel, act, "div", " class=\"teacher line0\"");
			}	
			tmp+=writeStudents(htmlLevel, act, "div", " class=\"studentsset line1\"");
			tmp+=writeSubjectAndActivityTags(htmlLevel, act, "div", " class=\"line2\"", false, printActivityTags);
			tmp+="</td>\n";
		} else
			tmp+="          <!-- span -->\n";
	} else {
        if(TContext::get()->instance.teacherNotAvailableDayHour[teacher][day][hour] && defs::PRINT_NOT_AVAILABLE_TIME_SLOTS){
			tmp+=writeNotAvailableSlot(htmlLevel, "");
		}
        else if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
			tmp+=writeBreakSlot(htmlLevel, "");
		}
		else{
			tmp+=writeEmpty(htmlLevel);
		}
	}
	return tmp;
}

//by Volker Dirr
QString TimetableExport::writeActivitiesTeachers(int htmlLevel, const QList<int>& allActivities, bool printActivityTags){
	QString tmp;
	if(htmlLevel>=1)
		tmp+="          <td><table class=\"detailed\">";
	else
		tmp+="          <td><table>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"teacher line0\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeTeachers(htmlLevel, act, "", "")+"</td>";
		}
	}
	tmp+="</tr>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"studentsset line1\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeStudents(htmlLevel, act, "", "")+"</td>";
		}
	}
	tmp+="</tr>";
	if(htmlLevel>=3)
		tmp+="<tr class=\"line2\">";
	else	tmp+="<tr>";
	for(int a=0; a<allActivities.size(); a++){
		int ai=allActivities[a];
		if(ai!=UNALLOCATED_ACTIVITY){
            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeSubjectAndActivityTags(htmlLevel, act, "", "", false, printActivityTags)+"</td>";
		}
	}
	tmp+="</tr>";

//	if(htmlLevel>=3)
//		tmp+="<tr class=\"room line3\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeRoom(htmlLevel, ai, "", "")+"</td>";
//		}
//	}
//	tmp+="</tr>";
	tmp+="</table></td>\n";
	return tmp;
}

////by Volker Dirr
//QString TimetableExport::writeActivityRoom(int htmlLevel, int room, int day, int hour, bool colspan, bool rowspan, bool printActivityTags){
//	QString tmp;
//	int ai=rooms_timetable_weekly[room][day][hour];
//	int currentTime=day+Timetable::getInstance()->rules.nDaysPerWeek*hour;
//	if(ai!=UNALLOCATED_ACTIVITY){
//		if(Timetable::getInstance()->bestSolution().times[ai]==currentTime){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, false, colspan, rowspan, COLOR_BY_SUBJECT_STUDENTS);
//			//Each activity has only a single room. So there is no need for line0. Modify this as soon as m-FET supports multiple rooms per activity.
//			tmp+=writeStudents(htmlLevel, act, "div", " class=\"studentsset line1\"");
//			tmp+=writeTeachers(htmlLevel, act, "div", " class=\"teacher line2\"");
//			tmp+=writeSubjectAndActivityTags(htmlLevel, act, "div", " class=\"line3\"", false, printActivityTags);
//			tmp+="</td>\n";
//		} else
//			tmp+="          <!-- span -->\n";
//	} else {
//		if(notAllowedRoomTimePercentages[room][day+hour*Timetable::getInstance()->rules.nDaysPerWeek]>=0 && defs::PRINT_NOT_AVAILABLE_TIME_SLOTS){
//			tmp+=writeNotAvailableSlot(htmlLevel, "");
//		}
//		else if(Timetable::getInstance()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
//			tmp+=writeBreakSlot(htmlLevel, "");
//		}
//		else{
//			tmp+=writeEmpty(htmlLevel);
//		}
//	}
//	return tmp;
//}

////by Volker Dirr
//QString TimetableExport::writeActivitiesRooms(int htmlLevel, const QList<int>& allActivities, bool printActivityTags){
//	QString tmp;
//	if(htmlLevel>=1)
//		tmp+="          <td><table class=\"detailed\">";
//	else
//		tmp+="          <td><table>";
//	if(htmlLevel>=3)
//		tmp+="<tr class=\"room line0\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeRoom(htmlLevel, ai, "", "")+"</td>";
//		}
//	}
//	tmp+="</tr>";
//	if(htmlLevel>=3)
//		tmp+="<tr class=\"studentsset line1\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeStudents(htmlLevel, act, "", "")+"</td>";
//		}
//	}
//	tmp+="</tr>";
//	if(htmlLevel>=3)
//		tmp+="<tr class=\"teacher line2\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeTeachers(htmlLevel, act, "", "")+"</td>";
//		}
//	}
//	tmp+="</tr>";
//	if(htmlLevel>=3)
//		tmp+="<tr class=\"line3\">";
//	else	tmp+="<tr>";
//	for(int a=0; a<allActivities.size(); a++){
//		int ai=allActivities[a];
//		if(ai!=UNALLOCATED_ACTIVITY){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeSubjectAndActivityTags(htmlLevel, act, "", "", false, printActivityTags)+"</td>";
//		}
//	}
//	tmp+="</tr>";

//	tmp+="</table></td>\n";
//	return tmp;
//}

//by Volker Dirr
QString TimetableExport::writeActivitiesSubjects(int htmlLevel, const QList<int>& allActivities, bool printActivityTags){
	QString tmp;
	if(allActivities.isEmpty()){
		tmp+=writeEmpty(htmlLevel);
	} else {
		if(htmlLevel>=1)
			tmp+="          <td><table class=\"detailed\">";
		else
			tmp+="          <td><table>";
		//Each activity has only a single subject. So there is no need for subjects in line0. Modify this as soon as m-FET supports multiple subjects per activity.
		if(printActivityTags){
			if(htmlLevel>=3)
				tmp+="<tr class=\"line0 activitytag\">";
			else	tmp+="<tr>";
			for(int a=0; a<allActivities.size(); a++){
                Activity* act=TContext::get()->instance.activeActivitiesList[allActivities[a]];
                Activity* act0=TContext::get()->instance.activeActivitiesList[allActivities[0]];	//Because this is always the original subject. We don't need to repeat it, because it is displayed in the tables head
				if(act->subjectName==act0->subjectName){
					tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeSubjectAndActivityTags(htmlLevel, act, "", "", true, printActivityTags)+"</td>";
				} else {
					tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeSubjectAndActivityTags(htmlLevel, act, "", "", false, printActivityTags)+"</td>";
				}
			}
			tmp+="</tr>";
		}
		if(htmlLevel>=3)
			tmp+="<tr class=\"studentsset line1\">";
		else	tmp+="<tr>";
		for(int a=0; a<allActivities.size(); a++){
            Activity* act=TContext::get()->instance.activeActivitiesList[allActivities[a]];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeStudents(htmlLevel, act, "", "")+"</td>";	
		}
		tmp+="</tr>";
		if(htmlLevel>=3)
			tmp+="<tr class=\"teacher line2\">";
		else	tmp+="<tr>";
		for(int a=0; a<allActivities.size(); a++){
            Activity* act=TContext::get()->instance.activeActivitiesList[allActivities[a]];
			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeTeachers(htmlLevel, act, "", "")+"</td>";
		}
		tmp+="</tr>";
//		if(htmlLevel>=3)
//			tmp+="<tr class=\"room line3\">";
//		else	tmp+="<tr>";
//		for(int a=0; a<allActivities.size(); a++){
//			int ai=allActivities[a];
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeRoom(htmlLevel, ai, "", "")+"</td>";
//		}
//		tmp+="</tr>";
		tmp+="</table></td>\n";
	}
	return tmp;
}

////by Volker Dirr
//QString TimetableExport::writeActivitiesActivityTags(int htmlLevel, const QList<int>& allActivities, bool printActivityTags){
//	QString tmp;
//	if(allActivities.isEmpty()){
//		tmp+=writeEmpty(htmlLevel);
//	} else {
//		if(htmlLevel>=1)
//			tmp+="          <td><table class=\"detailed\">";
//		else
//			tmp+="          <td><table>";
		
//		if(htmlLevel>=3)
//			tmp+="<tr class=\"line0\">";
//		else	tmp+="<tr>";
//		for(int a=0; a<allActivities.size(); a++){
//			int ai=allActivities[a];
//			if(ai!=UNALLOCATED_ACTIVITY){
//				Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//				tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeSubjectAndActivityTags(htmlLevel, act, "", "", false, printActivityTags)+"</td>";
//			}
//		}
//		tmp+="</tr>";
		
//		if(htmlLevel>=3)
//			tmp+="<tr class=\"studentsset line1\">";
//		else	tmp+="<tr>";
//		for(int a=0; a<allActivities.size(); a++){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[allActivities[a]];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeStudents(htmlLevel, act, "", "")+"</td>";
//		}
//		tmp+="</tr>";
//		if(htmlLevel>=3)
//			tmp+="<tr class=\"teacher line2\">";
//		else	tmp+="<tr>";
//		for(int a=0; a<allActivities.size(); a++){
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[allActivities[a]];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeTeachers(htmlLevel, act, "", "")+"</td>";
//		}
//		tmp+="</tr>";
//		if(htmlLevel>=3)
//			tmp+="<tr class=\"room line3\">";
//		else	tmp+="<tr>";
//		for(int a=0; a<allActivities.size(); a++){
//			int ai=allActivities[a];
//			Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[ai];
//			tmp+=writeStartTagTDofActivities(htmlLevel, act, true, false, false, COLOR_BY_SUBJECT_STUDENTS)+writeRoom(htmlLevel, ai, "", "")+"</td>";
//		}
//		tmp+="</tr>";
//		tmp+="</table></td>\n";
//	}
//	return tmp;
//}


//the following functions return a single html table (needed for html file export and printing)

//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableDaysHorizontalHtml(int htmlLevel, int subgroup, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(subgroup>=0);
    assert(subgroup<TContext::get()->instance.directSubgroupsList.size());
	QString tmpString;
    QString subgroup_name = TContext::get()->instance.directSubgroupsList[subgroup]->name;
	tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(subgroup_name)+"\" border=\"1\"";
	if(subgroup%2==0) tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";
		
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+utils::strings::parseStrForHtml(subgroup_name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			QList<int> allActivities;
			allActivities.clear();
            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], false, true, printActivityTags, subgroup_name);
			} else{
				tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableDaysVerticalHtml(int htmlLevel, int subgroup, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(subgroup>=0);
    assert(subgroup<TContext::get()->instance.directSubgroupsList.size());
	QString tmpString;
    QString subgroup_name = TContext::get()->instance.directSubgroupsList[subgroup]->name;
	tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(subgroup_name)+"\" border=\"1\"";
	if(subgroup%2==0) tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";
	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(subgroup_name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			QList<int> allActivities;
			allActivities.clear();
            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], true, false, printActivityTags, subgroup_name);
			} else{
				tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableTimeVerticalHtml(int htmlLevel, int maxSubgroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	
	int currentCount=0;
    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
		if(!excludedNames.contains(subgroup)){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=TContext::get()->instance.directSubgroupsList[subgroup]->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\"" +QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";

			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			currentCount=0;
            for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
				if(!excludedNames.contains(subgroup)){
					currentCount++;
                    if(day+1==TContext::get()->instance.nDaysPerWeek && hour+1==TContext::get()->instance.nHoursPerDay)
						excludedNames<<subgroup;
					QList<int> allActivities;
					allActivities.clear();
                    allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], false, true, printActivityTags, TContext::get()->instance.directSubgroupsList[subgroup]->name);
					} else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
				
				if(hour==0)
                    tmpString+="        <th rowspan=\"" +QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="      <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableTimeHorizontalHtml(int htmlLevel, int maxSubgroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        tmpString+="<th colspan=\"" +QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	}
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="        </tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay*Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	
	int currentCount=0;
    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
		if(!excludedNames.contains(subgroup)){
			currentCount++;
			excludedNames<<subgroup;
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.directSubgroupsList[subgroup]->name)+"</th>\n";
            for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
                for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
					QList<int> allActivities;
					allActivities.clear();
                    allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], true, false, printActivityTags, TContext::get()->instance.directSubgroupsList[subgroup]->name);
					} else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.directSubgroupsList[subgroup]->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="      <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableTimeVerticalDailyHtml(int htmlLevel, int day, int maxSubgroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	
	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
		if(!excludedNames.contains(subgroup)){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=TContext::get()->instance.directSubgroupsList[subgroup]->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		currentCount=0;
        for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
			if(!excludedNames.contains(subgroup)){
				currentCount++;
                if(hour+1==TContext::get()->instance.nHoursPerDay)
					excludedNames<<subgroup;
				QList<int> allActivities;
				allActivities.clear();
                allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], false, true, printActivityTags, TContext::get()->instance.directSubgroupsList[subgroup]->name);
				} else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleSubgroupsTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, int maxSubgroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="        </tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size() && currentCount<maxSubgroups; subgroup++){
		if(!excludedNames.contains(subgroup)){
			currentCount++;
			excludedNames<<subgroup;

			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=TContext::get()->instance.directSubgroupsList[subgroup]->name+"</th>\n";
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
				QList<int> allActivities;
				allActivities.clear();
                allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour], day, hour, TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour], true, false, printActivityTags, TContext::get()->instance.directSubgroupsList[subgroup]->name);
				} else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=TContext::get()->instance.directSubgroupsList[subgroup]->name+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleGroupsTimetableDaysHorizontalHtml(int htmlLevel, int group, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(group>=0);
    assert(group<TContext::get()->instance.internalGroupsList.size());
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(TContext::get()->instance.internalGroupsList[group]->name);
	tmpString+="\" border=\"1\"";
	if(group%2) tmpString+=" class=\"even_table\"";
	else tmpString+=" class=\"odd_table\"";
	tmpString+=">\n";
			
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList[group]->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			QList<int> allActivities;
			allActivities.clear();
			bool isNotAvailable=true;
            for(int sg=0; sg<TContext::get()->instance.internalGroupsList[group]->subgroupsList.size(); sg++){
                StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList[group]->subgroupsList[sg];
				int subgroup=sts->indexInInternalSubgroupsList;
                if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                    allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
					isNotAvailable=false;
				}
			assert(!allActivities.isEmpty());
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
			} else{
                if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
				else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleGroupsTimetableDaysVerticalHtml(int htmlLevel, int group, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(group>=0);
    assert(group<TContext::get()->instance.internalGroupsList.size());
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(TContext::get()->instance.internalGroupsList.at(group)->name);
	tmpString+="\" border=\"1\"";
	if(group%2) tmpString+=" class=\"even_table\"";
	else tmpString+=" class=\"odd_table\"";
	tmpString+=">\n";
			
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			QList<int> allActivities;
			allActivities.clear();
			bool isNotAvailable=true;
            for(int sg=0; sg<TContext::get()->instance.internalGroupsList.at(group)->subgroupsList.size(); sg++){
                StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList.at(group)->subgroupsList[sg];
				int subgroup=sts->indexInInternalSubgroupsList;
                if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                    allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
					isNotAvailable=false;
			}
			assert(!allActivities.isEmpty());
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
			} else{
                if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
				else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleGroupsTimetableTimeVerticalHtml(int htmlLevel, int maxGroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table";
	tmpString+="\" border=\"1\">\n";
        tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
		tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
		if(!excludedNames.contains(group)){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			currentCount=0;
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
            for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
				if(!excludedNames.contains(group)){
					currentCount++;
                    if(day+1==TContext::get()->instance.nDaysPerWeek && hour+1==TContext::get()->instance.nHoursPerDay)
						excludedNames<<group;
					QList<int> allActivities;
					allActivities.clear();
					bool isNotAvailable=true;
                    for(int sg=0; sg<TContext::get()->instance.internalGroupsList.at(group)->subgroupsList.size(); sg++){
                        StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList.at(group)->subgroupsList[sg];
						int subgroup=sts->indexInInternalSubgroupsList;
                        if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                        if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
							isNotAvailable=false;
					}
					assert(!allActivities.isEmpty());
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
					} else{
                        if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
						else{
							tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
						}
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
				if(hour==0)
                    tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleGroupsTimetableTimeHorizontalHtml(int htmlLevel, int maxGroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table";
	tmpString+="\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek*Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	
	int currentCount=0;
    for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
		if(!excludedNames.contains(group)){
			currentCount++;
			excludedNames<<group;

			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>\n";
            for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
                for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
					QList<int> allActivities;
					allActivities.clear();
					bool isNotAvailable=true;
                    for(int sg=0; sg<TContext::get()->instance.internalGroupsList.at(group)->subgroupsList.size(); sg++){
                        StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList.at(group)->subgroupsList[sg];
						int subgroup=sts->indexInInternalSubgroupsList;
                        if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                        if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
							isNotAvailable=false;
					}
					assert(!allActivities.isEmpty());
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
					} else{
                        if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
						else{
							tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
						}
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>\n";
			}
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek*TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleGroupsTimetableTimeVerticalDailyHtml(int htmlLevel, int day, int maxGroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day]);
	tmpString+="\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
		if(!excludedNames.contains(group)){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=TContext::get()->instance.internalGroupsList.at(group)->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		currentCount=0;
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">" + utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
			if(!excludedNames.contains(group)){
				currentCount++;
                if(hour+1==TContext::get()->instance.nHoursPerDay)
					excludedNames<<group;
				QList<int> allActivities;
				allActivities.clear();
				bool isNotAvailable=true;
                for(int sg=0; sg<TContext::get()->instance.internalGroupsList.at(group)->subgroupsList.size(); sg++){
                    StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList.at(group)->subgroupsList[sg];
					int subgroup=sts->indexInInternalSubgroupsList;
                    if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                        allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                    if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
						isNotAvailable=false;
				}
				assert(!allActivities.isEmpty());
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
				} else{
                    if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
					else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">" + utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleGroupsTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, int maxGroups, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day]);
	tmpString+="\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	
	int currentCount=0;
    for(int group=0; group<TContext::get()->instance.internalGroupsList.size() && currentCount<maxGroups; group++){
		if(!excludedNames.contains(group)){
			currentCount++;
			excludedNames<<group;

			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>\n";
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
				QList<int> allActivities;
				allActivities.clear();
				bool isNotAvailable=true;
                for(int sg=0; sg<TContext::get()->instance.internalGroupsList.at(group)->subgroupsList.size(); sg++){
                    StudentsSubgroup* sts=TContext::get()->instance.internalGroupsList.at(group)->subgroupsList[sg];
					int subgroup=sts->indexInInternalSubgroupsList;
                    if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                        allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                    if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
						isNotAvailable=false;
				}
				assert(!allActivities.isEmpty());
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.internalGroupsList[group]->name);
				} else{
                    if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
					else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.internalGroupsList.at(group)->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleYearsTimetableDaysHorizontalHtml(int htmlLevel, int year, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(year>=0);
    assert(year<TContext::get()->instance.augmentedYearsList.size());
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(TContext::get()->instance.augmentedYearsList.at(year)->name);
	tmpString+="\" border=\"1\"";
	if(year%2)  tmpString+=" class=\"even_table\"";
	else tmpString+=" class=\"odd_table\"";
	tmpString+=">\n";
				
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.augmentedYearsList.at(year)->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			QList<int> allActivities;
			allActivities.clear();
			bool isNotAvailable=true;
            for(int g=0; g<TContext::get()->instance.augmentedYearsList.at(year)->groupsList.size(); g++){
                StudentsGroup* stg=TContext::get()->instance.augmentedYearsList.at(year)->groupsList[g];
				for(int sg=0; sg<stg->subgroupsList.size(); sg++){
					StudentsSubgroup* sts=stg->subgroupsList[sg];
					int subgroup=sts->indexInInternalSubgroupsList;
                    if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                        allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                    if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
						isNotAvailable=false;
				}
			}
			assert(!allActivities.isEmpty());
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
			} else{
                if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
				else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleYearsTimetableDaysVerticalHtml(int htmlLevel, int year, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(year>=0);
    assert(year<TContext::get()->instance.augmentedYearsList.size());
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashStudentIDsTimetable.value(TContext::get()->instance.augmentedYearsList.at(year)->name);
	tmpString+="\" border=\"1\"";
	if(year%2)  tmpString+=" class=\"even_table\"";
	else tmpString+=" class=\"odd_table\"";
	tmpString+=">\n";
			
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.augmentedYearsList.at(year)->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";

        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			QList<int> allActivities;
			allActivities.clear();
			bool isNotAvailable=true;
            for(int g=0; g<TContext::get()->instance.augmentedYearsList.at(year)->groupsList.size(); g++){
                StudentsGroup* stg=TContext::get()->instance.augmentedYearsList.at(year)->groupsList[g];
				for(int sg=0; sg<stg->subgroupsList.size(); sg++){
					StudentsSubgroup* sts=stg->subgroupsList[sg];
					int subgroup=sts->indexInInternalSubgroupsList;
                    if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                        allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                    if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
						isNotAvailable=false;
				}
			}
			assert(!allActivities.isEmpty());
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
			} else{
                if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
				else{
					tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleYearsTimetableTimeVerticalHtml(int htmlLevel, int maxYears, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table";
	tmpString+="\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
		if(!excludedNames.contains(year)){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.augmentedYearsList.at(year)->name)+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour]) + "</th>\n";
			currentCount=0;
            for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
				if(!excludedNames.contains(year)){
					currentCount++;
                    if(day+1==TContext::get()->instance.nDaysPerWeek && hour+1==TContext::get()->instance.nHoursPerDay)
						excludedNames<<year;
					QList<int> allActivities;
					allActivities.clear();
					bool isNotAvailable=true;
                    StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
					for(int g=0; g<sty->groupsList.size(); g++){
						StudentsGroup* stg=sty->groupsList[g];
						for(int sg=0; sg<stg->subgroupsList.size(); sg++){
							StudentsSubgroup* sts=stg->subgroupsList[sg];
							int subgroup=sts->indexInInternalSubgroupsList;
                            if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                                allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                            if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
								isNotAvailable=false;
						}
					}
					assert(!allActivities.isEmpty());
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
					} else{
                        if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
						else{
							tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
						}
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour]) + "</th>\n";
				if(hour==0)
                    tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleYearsTimetableTimeHorizontalHtml(int htmlLevel, int maxYears, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table";
	tmpString+="\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay*Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	
	int currentCount=0;
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
        StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
		if(!excludedNames.contains(year)){
			currentCount++;
			excludedNames<<year;
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(sty->name)+"</th>\n";
            for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
                for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
					QList<int> allActivities;
					allActivities.clear();
					bool isNotAvailable=true;
					for(int g=0; g<sty->groupsList.size(); g++){
						StudentsGroup* stg=sty->groupsList[g];
						for(int sg=0; sg<stg->subgroupsList.size(); sg++){
							StudentsSubgroup* sts=stg->subgroupsList[sg];
							int subgroup=sts->indexInInternalSubgroupsList;
                            if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                                allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                            if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
								isNotAvailable=false;
						}
					}
					assert(!allActivities.isEmpty());
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
					} else{
                        if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
						else{
							tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
						}
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(sty->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleYearsTimetableTimeVerticalDailyHtml(int htmlLevel, int day, int maxYears, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day]);
	tmpString+="\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
		if(!excludedNames.contains(year)){
			currentCount++;

			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.augmentedYearsList.at(year)->name)+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";

    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		currentCount=0;
        for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
            StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
			if(!excludedNames.contains(year)){
				currentCount++;
                if(hour+1==TContext::get()->instance.nHoursPerDay)
					excludedNames<<year;
				QList<int> allActivities;
				allActivities.clear();
				bool isNotAvailable=true;
				for(int g=0; g<sty->groupsList.size(); g++){
					StudentsGroup* stg=sty->groupsList[g];
					for(int sg=0; sg<stg->subgroupsList.size(); sg++){
						StudentsSubgroup* sts=stg->subgroupsList[sg];
						int subgroup=sts->indexInInternalSubgroupsList;
                        if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                        if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
							isNotAvailable=false;
					}
				}
				assert(!allActivities.isEmpty());
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, false, true, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
				} else{
                    if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
					else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleYearsTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, int maxYears, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool detailed, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day]);
	tmpString+="\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";

    tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    for(int year=0; year<TContext::get()->instance.augmentedYearsList.size() && currentCount<maxYears; year++){
		if(!excludedNames.contains(year)){
			currentCount++;
			excludedNames<<year;
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            StudentsYear* sty=TContext::get()->instance.augmentedYearsList[year];
            tmpString+=utils::strings::parseStrForHtml(sty->name)+"</th>\n";
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
				QList<int> allActivities;
				allActivities.clear();
				bool isNotAvailable=true;
				for(int g=0; g<sty->groupsList.size(); g++){
					StudentsGroup* stg=sty->groupsList[g];
					for(int sg=0; sg<stg->subgroupsList.size(); sg++){
						StudentsSubgroup* sts=stg->subgroupsList[sg];
						int subgroup=sts->indexInInternalSubgroupsList;
                        if(!(allActivities.contains(TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour])))
                            allActivities<<TContext::get()->bestSolution().students_timetable_weekly[subgroup][day][hour];
                        if(!TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][day][hour])
							isNotAvailable=false;
					}
				}
				assert(!allActivities.isEmpty());
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityStudents(htmlLevel, allActivities[0], day, hour, isNotAvailable, true, false, printActivityTags, TContext::get()->instance.augmentedYearsList.at(year)->name);
				} else{
                    if(!detailed) tmpString+="          <td>"+utils::strings::parseStrForHtml(STRING_SEVERAL_ACTIVITIES_IN_LESS_DETAILED_TABLES)+"</td>\n";
					else{
						tmpString+=writeActivitiesStudents(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(sty->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableDaysHorizontalHtml(int htmlLevel, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table border=\"1\">\n";	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+tr("All Activities")+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			if(activitiesAtTime[day][hour].isEmpty()){
                if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
					tmpString+=writeBreakSlot(htmlLevel, "");
				} else {
					tmpString+=writeEmpty(htmlLevel);
				}
			} else {
				tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="      <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableDaysVerticalHtml(int htmlLevel, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+tr("All Activities")+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(activitiesAtTime[day][hour].isEmpty()){
                if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
					tmpString+=writeBreakSlot(htmlLevel, "");
				} else {
					tmpString+=writeEmpty(htmlLevel);
				}
			} else {
				tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="      <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n";
	return tmpString;
}

	
//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableTimeVerticalHtml(int htmlLevel, const QString& saveTime, bool printActivityTags, bool repeatNames){
QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	if(htmlLevel>=2)
		tmpString+="          <th class=\"xAxis\">";
	else
		tmpString+="          <th>";
	tmpString+=tr("All Activities");
	tmpString+="</th>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td>"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(activitiesAtTime[day][hour].isEmpty()){
                if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
					tmpString+=writeBreakSlot(htmlLevel, "");
				} else {
					tmpString+=writeEmpty(htmlLevel);
				}
			} else {
				tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
				if(hour==0)
                    tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="      <tr class=\"foot\"><td colspan=\"2\"></td><td>"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableTimeHorizontalHtml(int htmlLevel, const QString& saveTime, bool printActivityTags, bool repeatNames){

	QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay) +"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	}
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay*Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
		
	tmpString+="        <tr>\n";
	if(htmlLevel>=2)
		tmpString+="          <th class=\"yAxis\">";
	else
		tmpString+="          <th>";
	tmpString+=tr("All Activities")+"</th>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(activitiesAtTime[day][hour].isEmpty()){
                if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
					tmpString+=writeBreakSlot(htmlLevel, "");
				} else {
					tmpString+=writeEmpty(htmlLevel);
				}
			} else {
				tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
			}
		}
	}
	if(repeatNames){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
		tmpString+=tr("All Activities")+"</th>\n";
	}
	tmpString+="        </tr>\n";
	//workaround begin.
    tmpString+="      <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableTimeVerticalDailyHtml(int htmlLevel, int day, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	if(htmlLevel>=2)
		tmpString+="          <th class=\"xAxis\">";
	else
		tmpString+="          <th>";
	tmpString+=tr("All Activities");
	tmpString+="</th>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td>"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";

    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		if(activitiesAtTime[day][hour].isEmpty()){
            if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
				tmpString+=writeBreakSlot(htmlLevel, "");
			} else {
				tmpString+=writeEmpty(htmlLevel);
			}
		} else {
			tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td>"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleAllActivitiesTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
		
	tmpString+="        <tr>\n";
	if(htmlLevel>=2)
		tmpString+="          <th class=\"yAxis\">";
	else
		tmpString+="          <th>";
	tmpString+=tr("All Activities")+"</th>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(activitiesAtTime[day][hour].isEmpty()){
            if(TContext::get()->instance.breakDayHour[day][hour] && defs::PRINT_BREAK_TIME_SLOTS){
				tmpString+=writeBreakSlot(htmlLevel, "");
			} else {
				tmpString+=writeEmpty(htmlLevel);
			}
		} else {
			tmpString+=writeActivitiesStudents(htmlLevel, activitiesAtTime[day][hour], printActivityTags);
		}
	}
	if(repeatNames){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
		tmpString+=tr("All Activities")+"</th>\n";
	}
	tmpString+="        </tr>\n";
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleTeachersTimetableDaysHorizontalHtml(int htmlLevel, int teacherIndex, const QString& saveTime, bool printActivityTags, bool repeatNames){
    assert(teacherIndex>=0);
    assert(teacherIndex<TContext::get()->instance.teachersList.size());
	QString tmpString;
    QString teacher_name = TContext::get()->instance.teachersList.at(teacherIndex)->name;
	tmpString+="    <table id=\"table_"+hashTeacherIDsTimetable.value(teacher_name)+"\" border=\"1\"";
    if(teacherIndex%2==0)  tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";
	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+utils::strings::parseStrForHtml(teacher_name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			QList<int> allActivities;
			allActivities.clear();
            allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[teacherIndex][day][hour];
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityTeacher(htmlLevel, teacherIndex, day, hour, false, true, printActivityTags, teacher_name);
			} else{
				tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}


//by Volker Dirr
QString TimetableExport::singleTeachersTimetableDaysVerticalHtml(int htmlLevel, int teacherIndex, const QString& saveTime, bool printActivityTags, bool repeatNames){
    assert(teacherIndex>=0);
    assert(teacherIndex<TContext::get()->instance.teachersList.size());
	QString tmpString;
    QString teacher_name = TContext::get()->instance.teachersList.at(teacherIndex)->name;
	tmpString+="    <table id=\"table_"+hashTeacherIDsTimetable.value(teacher_name)+"\" border=\"1\"";
    if(teacherIndex%2==0) tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";
	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n";
    tmpString+="        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(teacher_name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			QList<int> allActivities;
			allActivities.clear();
            allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[teacherIndex][day][hour];
			bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
			if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                tmpString+=writeActivityTeacher(htmlLevel, teacherIndex, day, hour, true, false, printActivityTags, teacher_name);
			} else{
				tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

	
//by Volker Dirr
QString TimetableExport::singleTeachersTimetableTimeVerticalHtml(int htmlLevel, int maxTeachers, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        if (currentCount<maxTeachers) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
            currentCount++;
            if(htmlLevel>=2)
                tmpString+="          <th class=\"xAxis\">";
            else
                tmpString+="          <th>";
            tmpString+=teacher->name+"</th>";
        }
    }

	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			currentCount=0;

            foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
                if (currentCount<maxTeachers) {
                    break;
                }

                if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
					currentCount++;
                    if(day+1==TContext::get()->instance.nDaysPerWeek && hour+1==TContext::get()->instance.nHoursPerDay)
                        excludedNames<<TContext::get()->instance.teachersList.indexOf(teacher);
					QList<int> allActivities;
					allActivities.clear();
                    allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(teacher)][day][hour];
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityTeacher(htmlLevel, TContext::get()->instance.teachersList.indexOf(teacher), day, hour, false, true, printActivityTags, teacher->name);
					} else {
						tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
				if(hour==0)
                    tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="      <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleTeachersTimetableTimeHorizontalHtml(int htmlLevel, int maxTeachers, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay*Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        if (currentCount<maxTeachers) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
			currentCount++;
            excludedNames<<TContext::get()->instance.teachersList.indexOf(teacher);
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(teacher->name)+"</th>\n";
            for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
                for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
					QList<int> allActivities;
					allActivities.clear();
                    allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(teacher)][day][hour];
					bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
					if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                        tmpString+=writeActivityTeacher(htmlLevel, TContext::get()->instance.teachersList.indexOf(teacher), day, hour, true, false, printActivityTags, teacher->name);
					} else {
						tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
					}
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(teacher->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="      <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleTeachersTimetableTimeVerticalDailyHtml(int htmlLevel, int day, int maxTeachers, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        if (currentCount<maxTeachers) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=teacher->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";

    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		currentCount=0;
        foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
           if (currentCount<maxTeachers) {
                break;
            }
            if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
				currentCount++;
                if(hour+1==TContext::get()->instance.nHoursPerDay)
                    excludedNames<<TContext::get()->instance.teachersList.indexOf(teacher);
				QList<int> allActivities;
				allActivities.clear();
                allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(teacher)][day][hour];
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityTeacher(htmlLevel, TContext::get()->instance.teachersList.indexOf(teacher), day, hour, false, true, printActivityTags, teacher->name);
				} else {
					tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
				}
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleTeachersTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, int maxTeachers, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";
    tmpString+="<th colspan=\"" +QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    foreach (Teacher* teacher, TContext::get()->instance.teachersList) {
        if (currentCount<maxTeachers) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.teachersList.indexOf(teacher))){
			currentCount++;
            excludedNames<<TContext::get()->instance.teachersList.indexOf(teacher);
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(teacher->name)+"</th>\n";
		
            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
				QList<int> allActivities;
				allActivities.clear();
                allActivities<<TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(teacher)][day][hour];
				bool activitiesWithSameStartingtime=addActivitiesWithSameStartingTime(allActivities, hour);
				if(allActivities.size()==1 && !activitiesWithSameStartingtime){  // because i am using colspan or rowspan!!!
                    tmpString+=writeActivityTeacher(htmlLevel, TContext::get()->instance.teachersList.indexOf(teacher), day, hour, true, false, printActivityTags, teacher->name);
				} else {
					tmpString+=writeActivitiesTeachers(htmlLevel, allActivities, printActivityTags);
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(teacher->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleSubjectsTimetableDaysHorizontalHtml(int htmlLevel, int subject, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(subject>=0);
    assert(subject<TContext::get()->instance.subjectsList.size());
	QString tmpString;
	///////by Liviu Lalescu
    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
        for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
			activitiesForCurrentSubject[d][h].clear();
    foreach(int ai, TContext::get()->instance.activitiesForSubjectList[subject])
        if(TContext::get()->bestSolution().times[ai]!=UNALLOCATED_TIME){
            int h=TContext::get()->bestSolution().getHourForActivityIndex(ai);
            int d=TContext::get()->bestSolution().getDayForActivityIndex(ai);

            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
            for(int dd=0; dd < act->duration && h+dd < TContext::get()->instance.nHoursPerDay; dd++)
				activitiesForCurrentSubject[d][h+dd].append(ai);
		}
	///////end Liviu Lalescu
    tmpString+="    <table id=\"table_"+hashSubjectIDsTimetable.value(TContext::get()->instance.subjectsList.at(subject)->name);
	tmpString+="\" border=\"1\"";
	if(subject%2==0)  tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.subjectsList.at(subject)->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			QList<int> allActivities;
			
			allActivities=activitiesForCurrentSubject[day][hour];
			
			/*
			allActivities.clear();
			//Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
            for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                    Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                    if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                        if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                            allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
						}
				}
			}
			//Now run through the teachers timetable, because activities without a students set are still missing.
            for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                    Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                    if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                        if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
							assert(act->studentsNames.isEmpty());
                            allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
						}
				}
			}*/
			addActivitiesWithSameStartingTime(allActivities, hour);
			tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr	
QString TimetableExport::singleSubjectsTimetableDaysVerticalHtml(int htmlLevel, int subject, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(subject>=0);
    assert(subject<TContext::get()->instance.subjectsList.size());
	QString tmpString;
	///////by Liviu Lalescu
    for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
        for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
			activitiesForCurrentSubject[d][h].clear();
    foreach(int ai, TContext::get()->instance.activitiesForSubjectList[subject])
        if(TContext::get()->bestSolution().times[ai]!=UNALLOCATED_TIME){
            int h=TContext::get()->bestSolution().getHourForActivityIndex(ai);
            int d=TContext::get()->bestSolution().getDayForActivityIndex(ai);

            Activity* act=TContext::get()->instance.activeActivitiesList[ai];
            for(int dd=0; dd < act->duration && h+dd < TContext::get()->instance.nHoursPerDay; dd++)
				activitiesForCurrentSubject[d][h+dd].append(ai);
		}
	///////end Liviu Lalescu
    tmpString+="    <table id=\"table_"+hashSubjectIDsTimetable.value(TContext::get()->instance.subjectsList.at(subject)->name);
	tmpString+="\" border=\"1\"";
	if(subject%2==0) tmpString+=" class=\"odd_table\"";
	else tmpString+=" class=\"even_table\"";
	tmpString+=">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.subjectsList.at(subject)->name)+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			QList<int> allActivities;

			allActivities=activitiesForCurrentSubject[day][hour];

			/*
			allActivities.clear();
			//Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
            for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                    Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                    if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                        if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                            allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
						}
				}
			}
			//Now run through the teachers timetable, because activities without a students set are still missing.
            for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                    Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                    if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                        if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
							assert(act->studentsNames.isEmpty());
                            allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
						}
				}
			}
			*/
			addActivitiesWithSameStartingTime(allActivities, hour);
			tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

	
//by Volker Dirr	
QString TimetableExport::singleSubjectsTimetableTimeVerticalHtml(int htmlLevel, int maxSubjects, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        if (currentCount<maxSubjects) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
			currentCount++;
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=subject->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			tmpString+="        <tr>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";

			currentCount=0;
            foreach (Subject* subject, TContext::get()->instance.subjectsList) {
                if (currentCount<maxSubjects) {
                    break;
                }

                if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
					currentCount++;
                    if(day+1==TContext::get()->instance.nDaysPerWeek && hour+1==TContext::get()->instance.nHoursPerDay)
                        excludedNames<<TContext::get()->instance.subjectsList.indexOf(subject);
					QList<int> allActivities;
					allActivities.clear();
					
                    foreach(int ai, TContext::get()->instance.activitiesForSubjectList[TContext::get()->instance.subjectsList.indexOf(subject)])
						if(activitiesAtTime[day][hour].contains(ai)){
							assert(!allActivities.contains(ai));
							allActivities.append(ai);
						}
					
					/* //Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
                    for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                        if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                            Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                            if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                                if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                                    allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
								}
						}
					}
					//Now run through the teachers timetable, because activities without a students set are still missing.
                    for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                        if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                            Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                            if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                                if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
									assert(act->studentsNames.isEmpty());
                                    allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
								}
						}
					}*/
					addActivitiesWithSameStartingTime(allActivities, hour);
					tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
					tmpString+="          <th class=\"yAxis\">";
				else
					tmpString+="          <th>";
                tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
				if(hour==0)
                    tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
				else tmpString+="          <!-- span -->\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}

//by Volker Dirr	
QString TimetableExport::singleSubjectsTimetableTimeHorizontalHtml(int htmlLevel, int maxSubjects, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table\" border=\"1\">\n";

    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";

    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++)
        tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay*Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        if (currentCount<maxSubjects) {
             break;
        }

        if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
			currentCount++;
            excludedNames<<TContext::get()->instance.subjectsList.indexOf(subject);
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
                tmpString+="        <th class=\"yAxis\">"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
			else
                tmpString+="        <th>"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";

			///////by Liviu Lalescu
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
					activitiesForCurrentSubject[d][h].clear();
            foreach(int ai, TContext::get()->instance.activitiesForSubjectList[TContext::get()->instance.subjectsList.indexOf(subject)])
                if(TContext::get()->bestSolution().times[ai]!=UNALLOCATED_TIME){
                    int h=TContext::get()->bestSolution().getHourForActivityIndex(ai);
                    int d=TContext::get()->bestSolution().getDayForActivityIndex(ai);

                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];
                    for(int dd=0; dd < act->duration && h+dd < TContext::get()->instance.nHoursPerDay; dd++)
						activitiesForCurrentSubject[d][h+dd].append(ai);
				}
			///////end Liviu Lalescu

            for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
                for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
					QList<int> allActivities;

					allActivities=activitiesForCurrentSubject[day][hour];


					/*allActivities.clear();
					//Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
                    for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                        if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                            Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                            if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                                if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                                    allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
								}
						}
					}
					//Now run through the teachers timetable, because activities without a students set are still missing.
                    for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                        if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                            Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                            if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                                if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
									assert(act->studentsNames.isEmpty());
                                    allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
								}
						}
					}*/
					addActivitiesWithSameStartingTime(allActivities, hour);
					tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
				}
			}
			if(repeatNames){
				if(htmlLevel>=2)
                    tmpString+="        <th class=\"yAxis\">"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
				else
                    tmpString+="        <th>"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n    </table>\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleSubjectsTimetableTimeVerticalDailyHtml(int htmlLevel, int day, int maxSubjects, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td colspan=\"2\"></td>";
	int currentCount=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        if (currentCount<maxSubjects) {
             break;
        }

        if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
			currentCount++;

			if(htmlLevel>=2)
				tmpString+="          <th class=\"xAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=subject->name+"</th>";
		}
	}
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n      </thead>\n";
	/*workaround
	tmpString+="      <tfoot><tr><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(hour==0)
            tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		else tmpString+="          <!-- span -->\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";

		currentCount=0;
        foreach (Subject* subject, TContext::get()->instance.subjectsList) {
            if (currentCount<maxSubjects) {
                break;
            }

            if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
				currentCount++;
                if(hour+1==TContext::get()->instance.nHoursPerDay)
                    excludedNames<<TContext::get()->instance.subjectsList.indexOf(subject);
				QList<int> allActivities;
				allActivities.clear();
				
                foreach(int ai, TContext::get()->instance.activitiesForSubjectList[TContext::get()->instance.subjectsList.indexOf(subject)])
					if(activitiesAtTime[day][hour].contains(ai)){
						assert(!allActivities.contains(ai));
						allActivities.append(ai);
					}
				
				/*//Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
                for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                    if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                        Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                        if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                            if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                                allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
							}
					}
				}
				//Now run through the teachers timetable, because activities without a students set are still missing.
                for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                    if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                        Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                        if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                            if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
								assert(act->studentsNames.isEmpty());
                                allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
							}
					}
				}*/
				addActivitiesWithSameStartingTime(allActivities, hour);
				tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
			if(hour==0)
                tmpString+="        <th rowspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+ "\">"+utils::strings::parseStrForHtmlVertical(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
			else tmpString+="          <!-- span -->\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td colspan=\"2\"></td><td colspan=\""+QString::number(currentCount)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td colspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleSubjectsTimetableTimeHorizontalDailyHtml(int htmlLevel, int day, int maxSubjects, QSet<int>& excludedNames, const QString& saveTime, bool printActivityTags, bool repeatNames){
	assert(day>=0);
    assert(day<TContext::get()->instance.nDaysPerWeek);
	QString tmpString;
    tmpString+="    <table id=\"table_"+hashDayIDsTimetable.value(TContext::get()->instance.daysOfTheWeek[day])+"\" border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td>";

    tmpString+="<th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
	int currentCount=0;
    foreach (Subject* subject, TContext::get()->instance.subjectsList) {
        if (currentCount<maxSubjects) {
            break;
        }

        if(!excludedNames.contains(TContext::get()->instance.subjectsList.indexOf(subject))){
			currentCount++;
            excludedNames<<TContext::get()->instance.subjectsList.indexOf(subject);
			tmpString+="        <tr>\n";
			if(htmlLevel>=2)
                tmpString+="        <th class=\"yAxis\">"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
			else
                tmpString+="        <th>"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";

			///////by Liviu Lalescu
            for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++)
                for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++)
					activitiesForCurrentSubject[d][h].clear();
            foreach(int ai, TContext::get()->instance.activitiesForSubjectList[TContext::get()->instance.subjectsList.indexOf(subject)])
                if(TContext::get()->bestSolution().times[ai]!=UNALLOCATED_TIME){
                    int h=TContext::get()->bestSolution().getHourForActivityIndex(ai);
                    int d=TContext::get()->bestSolution().getDayForActivityIndex(ai);

                    Activity* act=TContext::get()->instance.activeActivitiesList[ai];
                    for(int dd=0; dd < act->duration && h+dd < TContext::get()->instance.nHoursPerDay; dd++)
						activitiesForCurrentSubject[d][h+dd].append(ai);
				}
			///////end Liviu Lalescu

            for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
				QList<int> allActivities;

				allActivities=activitiesForCurrentSubject[day][hour];


				/*allActivities.clear();
				//Now get the activitiy ids. I don't run through the InternalActivitiesList, even that is faster. I run through subgroupsList, because by that the activites are sorted by that in the html-table.
                for(int subgroup=0; subgroup<Timetable::getInstance()->rules.directSubgroupsList.size(); subgroup++){
                    if(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]!=UNALLOCATED_ACTIVITY){
                        Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]];
                        if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                            if(!(allActivities.contains(Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour]))){
                                allActivities+Timetable::getInstance()->bestSolution().students_timetable_weekly[subgroup][day][hour];
							}
					}
				}
				//Now run through the teachers timetable, because activities without a students set are still missing.
                for(int teacher=0; teacher<Timetable::getInstance()->rules.nInternalTeachers; teacher++){
                    if(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]!=UNALLOCATED_ACTIVITY){
                        Activity* act=&Timetable::getInstance()->rules.internalActivitiesList[Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]];
                        if(act->subjectName==Timetable::getInstance()->rules.internalSubjectsList[subject]->name)
                            if(!(allActivities.contains(Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour]))){
								assert(act->studentsNames.isEmpty());
                                allActivities+Timetable::getInstance()->bestSolution().teachers_timetable_weekly[teacher][day][hour];
							}
					}
				}*/
				addActivitiesWithSameStartingTime(allActivities, hour);
				tmpString+=writeActivitiesSubjects(htmlLevel, allActivities, printActivityTags);
			}
			if(repeatNames){
				if(htmlLevel>=2)
                    tmpString+="        <th class=\"yAxis\">"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
				else
                    tmpString+="        <th>"+utils::strings::parseStrForHtml(subject->name)+"</th>\n";
			}
			tmpString+="        </tr>\n";
		}
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleTeachersFreePeriodsTimetableDaysHorizontalHtml(int htmlLevel, const QString& saveTime, bool detailed, bool repeatNames){
	QString tmpString;
	tmpString+="    <table id=\"table\" border=\"1\">\n";
	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Teachers' Free Periods")+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nDaysPerWeek+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
        for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
			bool empty_slot;
			empty_slot=true;
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
                if(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size()>0){
					empty_slot=false;
				}
				if(!detailed&&tfp>=TEACHER_MUST_COME_EARLIER) break;
			}
			if(!empty_slot) tmpString+="          <td>";
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
                if(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size()>0){
					if(htmlLevel>=2)
						tmpString+="<div class=\"DESCRIPTION\">";
					switch(tfp){
						case TEACHER_HAS_SINGLE_GAP		: tmpString+=TimetableExport::tr("Single gap"); break;
						case TEACHER_HAS_BORDER_GAP		: tmpString+=TimetableExport::tr("Border gap"); break;
						case TEACHER_HAS_BIG_GAP		: tmpString+=TimetableExport::tr("Big gap"); break;
						case TEACHER_MUST_COME_EARLIER		: tmpString+=TimetableExport::tr("Must come earlier"); break;
						case TEACHER_MUST_STAY_LONGER		: tmpString+=TimetableExport::tr("Must stay longer"); break;
						case TEACHER_MUST_COME_MUCH_EARLIER	: tmpString+=TimetableExport::tr("Must come much earlier"); break;
						case TEACHER_MUST_STAY_MUCH_LONGER	: tmpString+=TimetableExport::tr("Must stay much longer"); break;
						case TEACHER_HAS_A_FREE_DAY		: tmpString+=TimetableExport::tr("Free day"); break;
						case TEACHER_IS_NOT_AVAILABLE		: tmpString+=TimetableExport::tr("Not available", "It refers to a teacher"); break;
						default: assert(0==1); break;
					}
					if(htmlLevel>=2)
						tmpString+=":</div>";
					else tmpString+=":<br />";
					if(htmlLevel>=3)
						switch(tfp){
							case TEACHER_HAS_SINGLE_GAP		: tmpString+="<div class=\"TEACHER_HAS_SINGLE_GAP\">"; break;
							case TEACHER_HAS_BORDER_GAP		: tmpString+="<div class=\"TEACHER_HAS_BORDER_GAP\">"; break;
							case TEACHER_HAS_BIG_GAP		: tmpString+="<div class=\"TEACHER_HAS_BIG_GAP\">"; break;
							case TEACHER_MUST_COME_EARLIER		: tmpString+="<div class=\"TEACHER_MUST_COME_EARLIER\">"; break;
							case TEACHER_MUST_STAY_LONGER		: tmpString+="<div class=\"TEACHER_MUST_STAY_LONGER\">"; break;
							case TEACHER_MUST_COME_MUCH_EARLIER	: tmpString+="<div class=\"TEACHER_MUST_COME_MUCH_EARLIER\">"; break;
							case TEACHER_MUST_STAY_MUCH_LONGER	: tmpString+="<div class=\"TEACHER_MUST_STAY_MUCH_LONGER\">"; break;
							case TEACHER_HAS_A_FREE_DAY		: tmpString+="<div class=\"TEACHER_HAS_A_FREE_DAY\">"; break;
							case TEACHER_IS_NOT_AVAILABLE		: tmpString+="<div class=\"TEACHER_IS_NOT_AVAILABLE\">"; break;
							default: assert(0==1); break;
						}
                    for(int t=0; t<TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size(); t++){
                        QString teacher_name = TContext::get()->instance.teachersList.at(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].at(t))->name;
							switch(htmlLevel){
                                case 4 : tmpString+="<span class=\"t_"+hashTeacherIDsTimetable.value(teacher_name)+"\">"+utils::strings::parseStrForHtml(teacher_name)+"</span>"; break;
								case 5 : ;
                                case 6 : tmpString+="<span class=\"t_"+hashTeacherIDsTimetable.value(teacher_name)+"\" onmouseover=\"highlight('t_"+hashTeacherIDsTimetable.value(teacher_name)+"')\">"+utils::strings::parseStrForHtml(teacher_name)+"</span>"; break;
                                default: tmpString+=utils::strings::parseStrForHtml(teacher_name); break;
							}
						tmpString+="<br />";
					}
					if(htmlLevel>=3)
						tmpString+="</div>";
				}
				if(!detailed&&tfp>=TEACHER_MUST_COME_EARLIER) break;
			}
			if(!empty_slot){
				tmpString+="</td>\n";
			} else {
				tmpString+=writeEmpty(htmlLevel);
			}
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nDaysPerWeek)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}
	
//by Volker Dirr
QString TimetableExport::singleTeachersFreePeriodsTimetableDaysVerticalHtml(int htmlLevel, const QString& saveTime, bool detailed, bool repeatNames){
	QString tmpString;
	
	tmpString+="    <table id=\"table\" border=\"1\">\n";
	
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";

    tmpString+="      <thead>\n        <tr><td rowspan=\"2\"></td><th colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Teachers' Free Periods")+"</th>";
	if(repeatNames){
		tmpString+="<td rowspan=\"2\"></td>";
	}
	tmpString+="</tr>\n";
	
	tmpString+="        <tr>\n          <!-- span -->\n";
    for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
		if(htmlLevel>=2)
			tmpString+="          <th class=\"xAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.hoursOfTheDay[hour])+"</th>\n";
	}
	tmpString+="        </tr>\n";
	tmpString+="      </thead>\n";
	/*workaround
    tmpString+="      <tfoot><tr><td></td><td colspan=\""+Timetable::getInstance()->rules.nHoursPerDay+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr></tfoot>\n";
	*/
	tmpString+="      <tbody>\n";
    for(int day=0; day<TContext::get()->instance.nDaysPerWeek; day++){
		tmpString+="        <tr>\n";
		if(htmlLevel>=2)
			tmpString+="          <th class=\"yAxis\">";
		else
			tmpString+="          <th>";
        tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
        for(int hour=0; hour<TContext::get()->instance.nHoursPerDay; hour++){
			bool empty_slot;
			empty_slot=true;
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
                if(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size()>0){
					empty_slot=false;
				}
				if(!detailed&&tfp>=TEACHER_MUST_COME_EARLIER) break;
			}
			if(!empty_slot) tmpString+="          <td>";
			for(int tfp=0; tfp<TEACHERS_FREE_PERIODS_N_CATEGORIES; tfp++){
                if(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size()>0){
					if(htmlLevel>=2)
						tmpString+="<div class=\"DESCRIPTION\">";
					switch(tfp){
						case TEACHER_HAS_SINGLE_GAP		: tmpString+=TimetableExport::tr("Single gap"); break;
						case TEACHER_HAS_BORDER_GAP		: tmpString+=TimetableExport::tr("Border gap"); break;
						case TEACHER_HAS_BIG_GAP		: tmpString+=TimetableExport::tr("Big gap"); break;
						case TEACHER_MUST_COME_EARLIER		: tmpString+=TimetableExport::tr("Must come earlier"); break;
						case TEACHER_MUST_STAY_LONGER		: tmpString+=TimetableExport::tr("Must stay longer"); break;
						case TEACHER_MUST_COME_MUCH_EARLIER	: tmpString+=TimetableExport::tr("Must come much earlier"); break;
						case TEACHER_MUST_STAY_MUCH_LONGER	: tmpString+=TimetableExport::tr("Must stay much longer"); break;
						case TEACHER_HAS_A_FREE_DAY		: tmpString+=TimetableExport::tr("Free day"); break;
						case TEACHER_IS_NOT_AVAILABLE		: tmpString+=TimetableExport::tr("Not available"); break;
						default: assert(0==1); break;
					}
					if(htmlLevel>=2)
						tmpString+=":</div>";
					else tmpString+=":<br />";
					if(htmlLevel>=3)
						switch(tfp){
							case TEACHER_HAS_SINGLE_GAP		: tmpString+="<div class=\"TEACHER_HAS_SINGLE_GAP\">"; break;
							case TEACHER_HAS_BORDER_GAP		: tmpString+="<div class=\"TEACHER_HAS_BORDER_GAP\">"; break;
							case TEACHER_HAS_BIG_GAP		: tmpString+="<div class=\"TEACHER_HAS_BIG_GAP\">"; break;
							case TEACHER_MUST_COME_EARLIER		: tmpString+="<div class=\"TEACHER_MUST_COME_EARLIER\">"; break;
							case TEACHER_MUST_STAY_LONGER		: tmpString+="<div class=\"TEACHER_MUST_STAY_LONGER\">"; break;
							case TEACHER_MUST_COME_MUCH_EARLIER	: tmpString+="<div class=\"TEACHER_MUST_COME_MUCH_EARLIER\">"; break;
							case TEACHER_MUST_STAY_MUCH_LONGER	: tmpString+="<div class=\"TEACHER_MUST_STAY_MUCH_LONGER\">"; break;
							case TEACHER_HAS_A_FREE_DAY		: tmpString+="<div class=\"TEACHER_HAS_A_FREE_DAY\">"; break;
							case TEACHER_IS_NOT_AVAILABLE		: tmpString+="<div class=\"TEACHER_IS_NOT_AVAILABLE\">"; break;
							default: assert(0==1); break;
						}
                    for(int t=0; t<TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].size(); t++){
                        QString teacher_name = TContext::get()->instance.teachersList.at(TContext::get()->bestSolution().teachers_free_periods_timetable_weekly[tfp][day][hour].at(t))->name;
							switch(htmlLevel){
                                case 4 : tmpString+="<span class=\"t_"+hashTeacherIDsTimetable.value(teacher_name)+"\">"+utils::strings::parseStrForHtml(teacher_name)+"</span>"; break;
								case 5 : ;
                                case 6 : tmpString+="<span class=\"t_"+hashTeacherIDsTimetable.value(teacher_name)+"\" onmouseover=\"highlight('t_"+hashTeacherIDsTimetable.value(teacher_name)+"')\">"+utils::strings::parseStrForHtml(teacher_name)+"</span>"; break;
                                default: tmpString+=utils::strings::parseStrForHtml(teacher_name); break;
							}
						tmpString+="<br />";
					}
					if(htmlLevel>=3)
						tmpString+="</div>";
				}
				if(!detailed&&tfp>=TEACHER_MUST_COME_EARLIER) break;
			}
			if(!empty_slot){
				tmpString+="</td>\n";
			} else
				tmpString+=writeEmpty(htmlLevel);
		}
		if(repeatNames){
			if(htmlLevel>=2)
				tmpString+="          <th class=\"yAxis\">";
			else
				tmpString+="          <th>";
            tmpString+=utils::strings::parseStrForHtml(TContext::get()->instance.daysOfTheWeek[day])+"</th>\n";
		}
		tmpString+="        </tr>\n";
	}
	//workaround begin.
    tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(TContext::get()->instance.nHoursPerDay)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
	if(repeatNames){
		tmpString+="<td></td>";
	}
	tmpString+="</tr>\n";
	//workaround end.
	tmpString+="      </tbody>\n";
	tmpString+="    </table>\n\n";
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleTeachersStatisticsHtml(int htmlLevel, const QString& saveTime, bool detailed, bool repeatNames, bool printAll){
	Q_UNUSED(htmlLevel);
	QString tmpString;
	if(!printAll){
		tmpString+="    <p>\n";
		tmpString+="      <strong>"+tr("This is a teaser only. Values are not correct!")+"</strong>\n";
		tmpString+="    </p>\n";
	}
	tmpString+="    <p>\n";
	tmpString+="      "+tr("This file doesn't list limits that are set by constraints. It contains statistics about the min and max values of the currently calculated solution.")+"\n";
	tmpString+="    </p>\n";
	
	QString teachersString="";
	int freeDaysAllTeachers=0;
    int minFreeDaysAllTeachers=TContext::get()->instance.nDaysPerWeek;
	int maxFreeDaysAllTeachers=0;
	int gapsAllTeachers=0;
    int minGapsPerDayAllTeachers=TContext::get()->instance.nHoursPerDay;
	int maxGapsPerDayAllTeachers=0;
    int minGapsPerWeekAllTeachers=TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek;
	int maxGapsPerWeekAllTeachers=0;
    int minHoursPerDayAllTeachers=TContext::get()->instance.nHoursPerDay;
	int maxHoursPerDayAllTeachers=0;

    foreach (Teacher* tch, TContext::get()->instance.teachersList) {
		int freeDaysSingleTeacher=0;
		int gapsSingleTeacher=0;
        int minGapsPerDaySingleTeacher=TContext::get()->instance.nHoursPerDay;
		int maxGapsPerDaySingleTeacher=0;
        int minHoursPerDaySingleTeacher=TContext::get()->instance.nHoursPerDay;
		int maxHoursPerDaySingleTeacher=0;
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
			int firstPeriod=-1;
			int lastPeriod=-1;
			int gapsPerDaySingleTeacher=0;
			int hoursPerDaySingleTeacher=0;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++){
                if(TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(tch)][d][h]!=UNALLOCATED_ACTIVITY){
					if(firstPeriod==-1)
						firstPeriod=h;
					lastPeriod=h;
					hoursPerDaySingleTeacher++;
				}
			}
			if(firstPeriod==-1){
				freeDaysSingleTeacher++;
			} else {
				for(int h=firstPeriod; h<lastPeriod; h++){
                    if(TContext::get()->bestSolution().teachers_timetable_weekly[TContext::get()->instance.teachersList.indexOf(tch)][d][h]==UNALLOCATED_ACTIVITY && TContext::get()->instance.teacherNotAvailableDayHour[TContext::get()->instance.teachersList.indexOf(tch)][d][h]==false && TContext::get()->instance.breakDayHour[d][h]==false){
						gapsPerDaySingleTeacher++;
					}
				}
			}
			gapsSingleTeacher+=gapsPerDaySingleTeacher;
			if(minGapsPerDaySingleTeacher>gapsPerDaySingleTeacher)
				minGapsPerDaySingleTeacher=gapsPerDaySingleTeacher;
			if(maxGapsPerDaySingleTeacher<gapsPerDaySingleTeacher)
				maxGapsPerDaySingleTeacher=gapsPerDaySingleTeacher;
			if(hoursPerDaySingleTeacher>0){
				if(minHoursPerDaySingleTeacher>hoursPerDaySingleTeacher)
					minHoursPerDaySingleTeacher=hoursPerDaySingleTeacher;
				if(maxHoursPerDaySingleTeacher<hoursPerDaySingleTeacher)
					maxHoursPerDaySingleTeacher=hoursPerDaySingleTeacher;
			}
		}
		if(minFreeDaysAllTeachers>freeDaysSingleTeacher)
			minFreeDaysAllTeachers=freeDaysSingleTeacher;
		if(maxFreeDaysAllTeachers<freeDaysSingleTeacher)
			maxFreeDaysAllTeachers=freeDaysSingleTeacher;
		
		if(minGapsPerDayAllTeachers>minGapsPerDaySingleTeacher)
			minGapsPerDayAllTeachers=minGapsPerDaySingleTeacher;
		if(maxGapsPerDayAllTeachers<maxGapsPerDaySingleTeacher)
			maxGapsPerDayAllTeachers=maxGapsPerDaySingleTeacher;
			
		if(minGapsPerWeekAllTeachers>gapsSingleTeacher)
			minGapsPerWeekAllTeachers=gapsSingleTeacher;
		if(maxGapsPerWeekAllTeachers<gapsSingleTeacher)
			maxGapsPerWeekAllTeachers=gapsSingleTeacher;
		
		if(minHoursPerDayAllTeachers>minHoursPerDaySingleTeacher)
			minHoursPerDayAllTeachers=minHoursPerDaySingleTeacher;
		if(maxHoursPerDayAllTeachers<maxHoursPerDaySingleTeacher)
			maxHoursPerDayAllTeachers=maxHoursPerDaySingleTeacher;

		gapsAllTeachers+=gapsSingleTeacher;
		freeDaysAllTeachers+=freeDaysSingleTeacher;
		
		if(detailed){
            if(freeDaysSingleTeacher==TContext::get()->instance.nDaysPerWeek)
				minHoursPerDaySingleTeacher=0;
            teachersString+="      <tr><th>"+utils::strings::parseStrForHtml(tch->name)
					+"</th><td>"+QString::number(freeDaysSingleTeacher)
								+"</td><td>"+QString::number(gapsSingleTeacher)
								+"</td><td>"+QString::number(minGapsPerDaySingleTeacher)
								+"</td><td>"+QString::number(maxGapsPerDaySingleTeacher)
								+"</td><td>"+QString::number(minHoursPerDaySingleTeacher)
								+"</td><td>"+QString::number(maxHoursPerDaySingleTeacher)
								+"</td>";
			
			if(repeatNames){
                teachersString+="<th>"+utils::strings::parseStrForHtml(tch->name)+"</th>";
			}
			teachersString+="</tr>\n";
		}
        if(!printAll && TContext::get()->instance.teachersList.indexOf(tch)>10){
			break;
		}
	}
	
	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n";
	tmpString+="        <tr><th>"+tr("All teachers")
		  +"</th><th>"+tr("Free days")
		  +"</th><th>"+tr("Gaps")
		  +"</th><th>"+tr("Gaps per day")
		  +"</th><th>"+tr("Hours per day")
		  +"</th></tr>\n";
	tmpString+="      </thead>\n";
	tmpString+="      <tr><th>"+tr("Sum")+"</th>";
	tmpString+="<td>"+QString::number(freeDaysAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(gapsAllTeachers)+"</td>";
	tmpString+="<td>---</td>";
	tmpString+="<td>---</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Average")+"</th>";
    tmpString+="<td>"+QString::number(double(freeDaysAllTeachers)/TContext::get()->instance.teachersList.size(),'f',2)+"</td>";
    tmpString+="<td>"+QString::number(double(gapsAllTeachers)/TContext::get()->instance.teachersList.size(),'f',2)+"</td>";
	tmpString+="<td>---</td>";
	tmpString+="<td>---</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Min")+"</th>";
	tmpString+="<td>"+QString::number(minFreeDaysAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(minGapsPerWeekAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(minGapsPerDayAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(minHoursPerDayAllTeachers)+"</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Max")+"</th>";
	tmpString+="<td>"+QString::number(maxFreeDaysAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(maxGapsPerWeekAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(maxGapsPerDayAllTeachers)+"</td>";
	tmpString+="<td>"+QString::number(maxHoursPerDayAllTeachers)+"</td>";
	tmpString+="</tr>\n";
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(4)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr>\n";
	//workaround end.
        tmpString+="    </table>\n";
	
	if(detailed){
		tmpString+="    <p class=\"back0\"><br /></p>\n\n";
		
		tmpString+="    <table border=\"1\">\n";
        tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
		tmpString+="      <thead>\n";
		tmpString+="        <tr><th>"+tr("Teacher")
			+"</th><th>"+tr("Free days")
			+"</th><th>"+tr("Total gaps")
			+"</th><th>"+tr("Min gaps per day")
			+"</th><th>"+tr("Max gaps per day")
			+"</th><th>"+tr("Min hours per day")
			+"</th><th>"+tr("Max hours per day")
			+"</th>";
		if(repeatNames){
			tmpString+="<th>"+tr("Teacher")+"</th>";
		}
		tmpString+="</tr>\n";
		tmpString+="      </thead>\n";
		tmpString+=teachersString;
		//workaround begin.
		tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(6)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
		if(repeatNames){
			tmpString+="<td></td>";
		}
		tmpString+="</tr>\n";
		//workaround end.
		tmpString+="    </table>\n";
	}
	return tmpString;
}

//by Volker Dirr
QString TimetableExport::singleStudentsStatisticsHtml(int htmlLevel, const QString& saveTime, bool detailed, bool repeatNames, bool printAll){
	Q_UNUSED(htmlLevel);
	QString tmpString;
	if(!printAll){
		tmpString+="    <p>\n";
		tmpString+="      <strong>"+tr("This is a teaser only. Values are not correct!")+"</strong>\n";
		tmpString+="    </p>\n";
	}
	tmpString+="    <p>\n";
	tmpString+="      "+tr("This file doesn't list limits that are set by constraints. It contains statistics about the min and max values of the currently calculated solution.")+"\n";
	tmpString+="    </p>\n";
	
	//subgroups statistics (start)
	QString subgroupsString="";
	int freeDaysAllSubgroups=0;
    int minFreeDaysAllSubgroups=TContext::get()->instance.nDaysPerWeek;
	int maxFreeDaysAllSubgroups=0;
	int gapsAllSubgroups=0;
    int minGapsPerDayAllSubgroups=TContext::get()->instance.nHoursPerDay;
	int maxGapsPerDayAllSubgroups=0;
    int minGapsPerWeekAllSubgroups=TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek;
	int maxGapsPerWeekAllSubgroups=0;
    int minHoursPerDayAllSubgroups=TContext::get()->instance.nHoursPerDay;
	int maxHoursPerDayAllSubgroups=0;
	QList<int> freeDaysPerWeekSubgroupList;
	QList<int> gapsPerWeekSubgroupList;
	QList<int> minGapsPerDaySubgroupList;
	QList<int> maxGapsPerDaySubgroupList;
	QList<int> minHoursPerDaySubgroupList;
	QList<int> maxHoursPerDaySubgroupList;
    for(int subgroup=0; subgroup<TContext::get()->instance.directSubgroupsList.size(); subgroup++){
		int freeDaysSingleSubgroup=0;
		int gapsSingleSubgroup=0;
        int minGapsPerDaySingleSubgroup=TContext::get()->instance.nHoursPerDay;
		int maxGapsPerDaySingleSubgroup=0;
        int minHoursPerDaySingleSubgroup=TContext::get()->instance.nHoursPerDay;
		int maxHoursPerDaySingleSubgroup=0;
        for(int d=0; d<TContext::get()->instance.nDaysPerWeek; d++){
			int firstPeriod=-1;
			int lastPeriod=-1;
			int gapsPerDaySingleSubgroup=0;
			int hoursPerDaySingleSubgroup=0;
            for(int h=0; h<TContext::get()->instance.nHoursPerDay; h++){
                if(TContext::get()->bestSolution().students_timetable_weekly[subgroup][d][h]!=UNALLOCATED_ACTIVITY){
					if(firstPeriod==-1)
						firstPeriod=h;
					lastPeriod=h;
					hoursPerDaySingleSubgroup++;
				}
			}
			if(firstPeriod==-1){
				freeDaysSingleSubgroup++;
			} else {
				for(int h=firstPeriod; h<lastPeriod; h++){
                    if(TContext::get()->bestSolution().students_timetable_weekly[subgroup][d][h]==UNALLOCATED_ACTIVITY && TContext::get()->instance.subgroupNotAvailableDayHour[subgroup][d][h]==false && TContext::get()->instance.breakDayHour[d][h]==false){
						gapsPerDaySingleSubgroup++;
					}
				}
			}
			gapsSingleSubgroup+=gapsPerDaySingleSubgroup;
			if(minGapsPerDaySingleSubgroup>gapsPerDaySingleSubgroup)
				minGapsPerDaySingleSubgroup=gapsPerDaySingleSubgroup;
			if(maxGapsPerDaySingleSubgroup<gapsPerDaySingleSubgroup)
				maxGapsPerDaySingleSubgroup=gapsPerDaySingleSubgroup;
			if(hoursPerDaySingleSubgroup>0){
				if(minHoursPerDaySingleSubgroup>hoursPerDaySingleSubgroup)
					minHoursPerDaySingleSubgroup=hoursPerDaySingleSubgroup;
				if(maxHoursPerDaySingleSubgroup<hoursPerDaySingleSubgroup)
					maxHoursPerDaySingleSubgroup=hoursPerDaySingleSubgroup;
			}
		}
		if(minFreeDaysAllSubgroups>freeDaysSingleSubgroup)
			minFreeDaysAllSubgroups=freeDaysSingleSubgroup;
		if(maxFreeDaysAllSubgroups<freeDaysSingleSubgroup)
			maxFreeDaysAllSubgroups=freeDaysSingleSubgroup;
		
		if(minGapsPerDayAllSubgroups>minGapsPerDaySingleSubgroup)
			minGapsPerDayAllSubgroups=minGapsPerDaySingleSubgroup;
		if(maxGapsPerDayAllSubgroups<maxGapsPerDaySingleSubgroup)
			maxGapsPerDayAllSubgroups=maxGapsPerDaySingleSubgroup;
			
		if(minGapsPerWeekAllSubgroups>gapsSingleSubgroup)
			minGapsPerWeekAllSubgroups=gapsSingleSubgroup;
		if(maxGapsPerWeekAllSubgroups<gapsSingleSubgroup)
			maxGapsPerWeekAllSubgroups=gapsSingleSubgroup;
		
		if(minHoursPerDayAllSubgroups>minHoursPerDaySingleSubgroup)
			minHoursPerDayAllSubgroups=minHoursPerDaySingleSubgroup;
		if(maxHoursPerDayAllSubgroups<maxHoursPerDaySingleSubgroup)
			maxHoursPerDayAllSubgroups=maxHoursPerDaySingleSubgroup;

		gapsAllSubgroups+=gapsSingleSubgroup;
		freeDaysAllSubgroups+=freeDaysSingleSubgroup;
		
        if(freeDaysSingleSubgroup==TContext::get()->instance.nDaysPerWeek)
			minHoursPerDaySingleSubgroup=0;
		if(detailed){
            subgroupsString+="      <tr><th>"+utils::strings::parseStrForHtml(TContext::get()->instance.directSubgroupsList[subgroup]->name)
						+"</th><td>"+QString::number(freeDaysSingleSubgroup)
							 +"</td><td>"+QString::number(gapsSingleSubgroup)
							 +"</td><td>"+QString::number(minGapsPerDaySingleSubgroup)
							 +"</td><td>"+QString::number(maxGapsPerDaySingleSubgroup)
							 +"</td><td>"+QString::number(minHoursPerDaySingleSubgroup)
							 +"</td><td>"+QString::number(maxHoursPerDaySingleSubgroup)
							 +"</td>";
			if(repeatNames){
                subgroupsString+="<th>"+utils::strings::parseStrForHtml(TContext::get()->instance.directSubgroupsList[subgroup]->name)+"</th>";
			}
			subgroupsString+="</tr>\n";
			freeDaysPerWeekSubgroupList<<freeDaysSingleSubgroup;
			gapsPerWeekSubgroupList<<gapsSingleSubgroup;
			minGapsPerDaySubgroupList<<minGapsPerDaySingleSubgroup;
			maxGapsPerDaySubgroupList<<maxGapsPerDaySingleSubgroup;
			minHoursPerDaySubgroupList<<minHoursPerDaySingleSubgroup;
			maxHoursPerDaySubgroupList<<maxHoursPerDaySingleSubgroup;
		}
		if(!printAll && subgroup>10){
			break;
		}
	}

	tmpString+="    <table border=\"1\">\n";
    tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
	tmpString+="      <thead>\n";
	tmpString+="        <tr><th>"+tr("All students")
		  +"</th><th>"+tr("Free days")
		  +"</th><th>"+tr("Gaps")
		  +"</th><th>"+tr("Gaps per day")
		  +"</th><th>"+tr("Hours per day")
		  +"</th></tr>\n";
	tmpString+="      </thead>\n";
	tmpString+="      <tr><th>"+tr("Sum")+"</th>";
	tmpString+="<td>"+QString::number(freeDaysAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(gapsAllSubgroups)+"</td>";
	tmpString+="<td>---</td>";
	tmpString+="<td>---</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Average")+"</th>";
    tmpString+="<td>"+QString::number(double(freeDaysAllSubgroups)/TContext::get()->instance.directSubgroupsList.size(),'f',2)+"</td>";
    tmpString+="<td>"+QString::number(double(gapsAllSubgroups)/TContext::get()->instance.directSubgroupsList.size(),'f',2)+"</td>";
	tmpString+="<td>---</td>";
	tmpString+="<td>---</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Min")+"</th>";
	tmpString+="<td>"+QString::number(minFreeDaysAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(minGapsPerWeekAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(minGapsPerDayAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(minHoursPerDayAllSubgroups)+"</td>";
	tmpString+="</tr>\n";
	tmpString+="      <tr><th>"+tr("Max")+"</th>";
	tmpString+="<td>"+QString::number(maxFreeDaysAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(maxGapsPerWeekAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(maxGapsPerDayAllSubgroups)+"</td>";
	tmpString+="<td>"+QString::number(maxHoursPerDayAllSubgroups)+"</td>";
	tmpString+="</tr>\n";
	//workaround begin.
	tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(4)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td></tr>\n";
	//workaround end.
        tmpString+="    </table>\n";
	
	tmpString+="    <p class=\"back0\"><br /></p>\n\n";
	
	//subgroups statistics (end)
	
	if(detailed){
		if(!printAll){
			//similar to source in else part (start)
			tmpString+="    <p></p>\n";
			tmpString+="    <table border=\"1\">\n";
            tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
			tmpString+="      <thead>\n";
			tmpString+="      <tr><th>"+tr("Subgroup")
					+"</th><th>"+tr("Free days")
					+"</th><th>"+tr("Total gaps")
					+"</th><th>"+tr("Min gaps per day")
					+"</th><th>"+tr("Max gaps per day")
					+"</th><th>"+tr("Min hours per day")
					+"</th><th>"+tr("Max hours per day")
					+"</th>";
			if(repeatNames){
				tmpString+="<td>"+tr("Subgroup")+"</td>";
			}
			tmpString+="</tr>\n";
			tmpString+="      </thead>\n";
			tmpString+=subgroupsString;
			//workaround begin.
			tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(6)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
			if(repeatNames){
				tmpString+="<td></td>";
			}
			tmpString+="</tr>\n";
			//workaround end.
			tmpString+="    </table>\n";
			//similar to source in else part (end)
		} else {
			//groups and years statistics (start)
			QString yearsString="    <table border=\"1\">\n";
            yearsString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
			yearsString+="      <thead>\n";
			yearsString+="      <tr><th>"+tr("Year")
							+"</th><th>"+tr("Min free days")
							+"</th><th>"+tr("Max free days")
							+"</th><th>"+tr("Min hours per day")
							+"</th><th>"+tr("Max hours per day")
							+"</th><th>"+tr("Min gaps per week")
							+"</th><th>"+tr("Max gaps per week")
							+"</th><th>"+tr("Min gaps per day")
							+"</th><th>"+tr("Max gaps per day")
							+"</th>";
			if(repeatNames){
					yearsString+="<th>"+tr("Year")+"</th>";
			}
			yearsString+="</tr>\n";
			yearsString+="      </thead>\n";
			QString groupsString="    <table border=\"1\">\n";
            groupsString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
			groupsString+="      <thead>\n";
			groupsString+="      <tr><th>"+tr("Group")
							+"</th><th>"+tr("Min free days")
							+"</th><th>"+tr("Max free days")
							+"</th><th>"+tr("Min hours per day")
							+"</th><th>"+tr("Max hours per day")
							+"</th><th>"+tr("Min gaps per week")
							+"</th><th>"+tr("Max gaps per week")
							+"</th><th>"+tr("Min gaps per day")
							+"</th><th>"+tr("Max gaps per day")
							+"</th>";
			if(repeatNames){
				groupsString+="<th>"+tr("Group")+"</th>";
			}
			groupsString+="</tr>\n";
			groupsString+="      </thead>\n";
            for(int i=0; i<TContext::get()->instance.augmentedYearsList.size(); i++){
                StudentsYear* sty=TContext::get()->instance.augmentedYearsList[i];
                int minFreeDaysPerWeekYear=TContext::get()->instance.nDaysPerWeek;
				int maxFreeDaysPerWeekYear=0;
                int minGapsPerDayYear=TContext::get()->instance.nHoursPerDay;
				int maxGapsPerDayYear=0;
                int minGapsPerWeekYear=TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek;
				int maxGapsPerWeekYear=0;
                int minHoursPerDayYear=TContext::get()->instance.nHoursPerDay;
				int maxHoursPerDayYear=0;
				for(int g=0; g<sty->groupsList.size(); g++){
					StudentsGroup* stg=sty->groupsList[g];
                    int minFreeDaysPerWeekGroup=TContext::get()->instance.nDaysPerWeek;
					int maxFreeDaysPerWeekGroup=0;
                    int minGapsPerDayGroup=TContext::get()->instance.nHoursPerDay;
					int maxGapsPerDayGroup=0;
                    int minGapsPerWeekGroup=TContext::get()->instance.nHoursPerDay*TContext::get()->instance.nDaysPerWeek;
					int maxGapsPerWeekGroup=0;
                    int minHoursPerDayGroup=TContext::get()->instance.nHoursPerDay;
					int maxHoursPerDayGroup=0;
					for(int sg=0; sg<stg->subgroupsList.size(); sg++){
						StudentsSubgroup* sts=stg->subgroupsList[sg];
						int subgroup=sts->indexInInternalSubgroupsList;

						if(minFreeDaysPerWeekGroup>freeDaysPerWeekSubgroupList.at(subgroup))
							minFreeDaysPerWeekGroup=freeDaysPerWeekSubgroupList.at(subgroup);
						if(maxFreeDaysPerWeekGroup<freeDaysPerWeekSubgroupList.at(subgroup))
							maxFreeDaysPerWeekGroup=freeDaysPerWeekSubgroupList.at(subgroup);
						
						if(minHoursPerDayGroup>minHoursPerDaySubgroupList.at(subgroup))
							minHoursPerDayGroup=minHoursPerDaySubgroupList.at(subgroup);
						if(maxHoursPerDayGroup<maxHoursPerDaySubgroupList.at(subgroup))
							maxHoursPerDayGroup=maxHoursPerDaySubgroupList.at(subgroup);
						
						if(minGapsPerWeekGroup>gapsPerWeekSubgroupList.at(subgroup))
							minGapsPerWeekGroup=gapsPerWeekSubgroupList.at(subgroup);
						if(maxGapsPerWeekGroup<gapsPerWeekSubgroupList.at(subgroup))
							maxGapsPerWeekGroup=gapsPerWeekSubgroupList.at(subgroup);
						
						if(minGapsPerDayGroup>minGapsPerDaySubgroupList.at(subgroup))
							minGapsPerDayGroup=minGapsPerDaySubgroupList.at(subgroup);
						if(maxGapsPerDayGroup<maxGapsPerDaySubgroupList.at(subgroup))
							maxGapsPerDayGroup=maxGapsPerDaySubgroupList.at(subgroup);
					}
					//print groups
                    groupsString+="      <tr><th>"+utils::strings::parseStrForHtml(stg->name)+"</th><td>"
					+QString::number(minFreeDaysPerWeekGroup)+"</td><td>"+QString::number(maxFreeDaysPerWeekGroup)+"</td><td>"
					+QString::number(minHoursPerDayGroup)+"</td><td>"+QString::number(maxHoursPerDayGroup)+"</td><td>"
					+QString::number(minGapsPerWeekGroup)+"</td><td>"+QString::number(maxGapsPerWeekGroup)+"</td><td>"
					+QString::number(minGapsPerDayGroup)+"</td><td>"+QString::number(maxGapsPerDayGroup)+"</td>";
					if(repeatNames){
                        groupsString+="<th>"+utils::strings::parseStrForHtml(stg->name)+"</th>";
					}
					groupsString+="</tr>\n";

					//check years
					if(minFreeDaysPerWeekYear>minFreeDaysPerWeekGroup)
						minFreeDaysPerWeekYear=minFreeDaysPerWeekGroup;
					if(maxFreeDaysPerWeekYear<maxFreeDaysPerWeekGroup)
						maxFreeDaysPerWeekYear=maxFreeDaysPerWeekGroup;
					
					if(minHoursPerDayYear>minHoursPerDayGroup)
						minHoursPerDayYear=minHoursPerDayGroup;
					if(maxHoursPerDayYear<maxHoursPerDayGroup)
						maxHoursPerDayYear=maxHoursPerDayGroup;
						
					if(minGapsPerWeekYear>minGapsPerWeekGroup)
						minGapsPerWeekYear=minGapsPerWeekGroup;
					if(maxGapsPerWeekYear<maxGapsPerWeekGroup)
						maxGapsPerWeekYear=maxGapsPerWeekGroup;
						
					if(minGapsPerDayYear>minGapsPerDayGroup)
						minGapsPerDayYear=minGapsPerDayGroup;
					if(maxGapsPerDayYear<maxGapsPerDayGroup)
						maxGapsPerDayYear=maxGapsPerDayGroup;
				}
				//print years
                    yearsString+="      <tr><th>"+utils::strings::parseStrForHtml(sty->name)+"</th><td>"
					+QString::number(minFreeDaysPerWeekYear)+"</td><td>"+QString::number(maxFreeDaysPerWeekYear)+"</td><td>"
					+QString::number(minHoursPerDayYear)+"</td><td>"+QString::number(maxHoursPerDayYear)+"</td><td>"
					+QString::number(minGapsPerWeekYear)+"</td><td>"+QString::number(maxGapsPerWeekYear)+"</td><td>"
					+QString::number(minGapsPerDayYear)+"</td><td>"+QString::number(maxGapsPerDayYear)+"</td>";
					if(repeatNames){
                        yearsString+="<th>"+utils::strings::parseStrForHtml(sty->name)+"</th>";
					}
					yearsString+="</tr>\n";
			}
			//workaround begin.
			groupsString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(8)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
			if(repeatNames){
				groupsString+="<td></td>";
			}
			groupsString+="</tr>\n";
			//workaround end.
			groupsString+="    </table>\n";
			groupsString+="    <p class=\"back0\"><br /></p>\n\n";
			//workaround begin.
			yearsString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(8)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
			if(repeatNames){
				yearsString+="<td></td>";
			}
			yearsString+="</tr>\n";
			//workaround end.
			yearsString+="    </table>\n";
			yearsString+="    <p class=\"back0\"><br /></p>\n\n";
			tmpString+=yearsString;
			tmpString+=groupsString;
			//similar to source in if part (start)
			tmpString+="    <p></p>\n";
			tmpString+="    <table border=\"1\">\n";
            tmpString+="      <caption>"+utils::strings::parseStrForHtml(TContext::get()->instance.institutionName)+"</caption>\n";
			tmpString+="      <thead>\n";
			tmpString+="      <tr><th>"+tr("Subgroup")
					+"</th><th>"+tr("Free days")
					+"</th><th>"+tr("Total gaps")
					+"</th><th>"+tr("Min gaps per day")
					+"</th><th>"+tr("Max gaps per day")
					+"</th><th>"+tr("Min hours per day")
					+"</th><th>"+tr("Max hours per day")
					+"</th>";
			if(repeatNames){
				tmpString+="<td>"+tr("Subgroup")+"</td>";
			}
			tmpString+="</tr>\n";
			tmpString+="      </thead>\n";
			tmpString+=subgroupsString;
			//workaround begin.
			tmpString+="        <tr class=\"foot\"><td></td><td colspan=\""+QString::number(6)+"\">"+TimetableExport::tr("Timetable generated with m-FET %1 on %2", "%1 is m-FET version, %2 is the date and time of generation").arg(FET_VERSION).arg(saveTime)+"</td>";
			if(repeatNames){
				tmpString+="<td></td>";
			}
			tmpString+="</tr>\n";
			//workaround end.
			tmpString+="    </table>\n";
			//similar to source in if part (end)
		}
	}
	//groups and years statistics (end)
	return tmpString;
}

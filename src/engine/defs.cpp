
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"

QString defs::FET_LANGUAGE="en_US";

bool defs::LANGUAGE_STYLE_RIGHT_TO_LEFT=false;

QString defs::LANGUAGE_FOR_HTML="en_US";

QString defs::INPUT_FILENAME_XML;

int defs::TIMETABLE_HTML_LEVEL=2;

bool defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS=true;

bool defs::PRINT_DETAILED_HTML_TIMETABLES=true;

bool defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS=true;

bool defs::PRINT_NOT_AVAILABLE_TIME_SLOTS=true;

bool defs::PRINT_BREAK_TIME_SLOTS=true;

bool defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS=true;

bool defs::TIMETABLE_HTML_REPEAT_NAMES=true;

bool defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME=true;

bool defs::USE_GUI_COLORS=true;

bool defs::SHOW_SUBGROUPS_IN_COMBO_BOXES=false;
bool defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING=false;

bool defs::WRITE_TIMETABLE_CONFLICTS=true;

bool defs::WRITE_TIMETABLES_STATISTICS=true;
bool defs::WRITE_TIMETABLES_XML=true;
bool defs::WRITE_TIMETABLES_DAYS_HORIZONTAL=true;
bool defs::WRITE_TIMETABLES_DAYS_VERTICAL=true;
bool defs::WRITE_TIMETABLES_TIME_HORIZONTAL=true;
bool defs::WRITE_TIMETABLES_TIME_VERTICAL=true;

bool defs::WRITE_TIMETABLES_SUBGROUPS=true;
bool defs::WRITE_TIMETABLES_GROUPS=true;
bool defs::WRITE_TIMETABLES_YEARS=true;
bool defs::WRITE_TIMETABLES_TEACHERS=true;
bool defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS=true;
bool defs::WRITE_TIMETABLES_ROOMS=true;
bool defs::WRITE_TIMETABLES_SUBJECTS=true;
bool defs::WRITE_TIMETABLES_ACTIVITY_TAGS=true;
bool defs::WRITE_TIMETABLES_ACTIVITIES=true;

bool defs::SHOW_SHORTCUTS_ON_MAIN_WINDOW=true;

bool defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES=true;

bool defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY=true;
bool defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY=true;

bool defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS=true;

bool defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES=true;

bool defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=true;
bool defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=true;

bool defs::CONFIRM_ACTIVITY_PLANNING=true;
bool defs::CONFIRM_SPREAD_ACTIVITIES=true;
bool defs::CONFIRM_REMOVE_REDUNDANT=true;
bool defs::CONFIRM_SAVE_TIMETABLE=true;

bool defs::VERBOSE=true;

QString defs::WORKING_DIRECTORY;

QString defs::IMPORT_DIRECTORY;

QString defs::OUTPUT_DIR;

bool defs::checkForUpdates=false;

QString defs::internetVersion;

const QString defs::COMPANY=QString("m-fet");
const QString defs::PROGRAM=QString("fettimetabling");

/**
A log file explaining how the xml input file was parsed
*/
const QString defs::XML_PARSING_LOG_FILENAME="file_open.log";

/**
The predefined names of the days of the week
*/
const QString defs::PREDEFINED_DAYS_OF_THE_WEEK[]={"Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday", "Sunday", "Monday2",
	"Tuesday2", "Wednesday2", "Thursday2", "Friday2", "Saturday2", "Sunday2",
	"Monday3", "Tuesday3", "Wednesday3",
	"Thursday3", "Friday3", "Saturday3", "Sunday3", "Monday4",
	"Tuesday4", "Wednesday4", "Thursday4", "Friday4", "Saturday4", "Sunday4"};

/**
File and directory separator
*/
const QString defs::FILE_SEP="/";


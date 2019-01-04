/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef TIMETABLE_DEFS_H
#define TIMETABLE_DEFS_H

#include <QString>
#include <cassert>

#ifdef NDEBUG
#undef NDEBUG
#endif

#define M_FET_VERSION QString("1.00.0")

/**
The version number
*/
#define FET_VERSION QString("5.31.6")

#define BREAK if (0){std::cout << "----";}

#define INFINITY_INT std::numeric_limits<int>::max();

#define ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE (1.0)

#define MAX_THREADS (128)

#define IS_EQUAL(a, b) ( (fabs(static_cast<double>(a) - static_cast<double>(b)) < 1e-12) )

#define IS_EQUAL_v2(a, b) ( (fabs(static_cast<double>(a) - static_cast<double>(b)) < 1e-12 * max(fabs(static_cast<double>(a)), fabs(static_cast<double>(b)))) )

#define IS_EQUAL_v3(a, b) ( (fabs(static_cast<double>(a) - static_cast<double>(b)) / max(fabs(static_cast<double>(a)), fabs(static_cast<double>(b))) < 1e-12) )

#define FLOAT(a) (static_cast<float>(a))

#define DOUBLE(a) (static_cast<double>(a))

#define P_VH   0
#define P_H    1
#define P_I    2
#define P_L    3
#define P_VL   4

/**
The maximum total number of different subgroups of students
*/
#define MAX_TOTAL_SUBGROUPS (30000)//MAX_YEARS*MAX_GROUPS_PER_YEAR*MAX_SUBGROUPS_PER_GROUP;

#define MAX_ROOM_CAPACITY (30000)

/**
The maximum number of different teachers
*/
#define MAX_TEACHERS (6000)

/**
The maximum number of activities
DEPRECATED COMMENT BELOW
IMPORTANT: must be qint16 (max 32767), because we are using qint16 for each activity index and for
unallocated activity = max_activities
*/
#define MAX_ACTIVITIES (1000)

/**
if you want to increase this, you also need to modify the add/modify activity dialogs, to permit larger values
(add more pages in the subactivities tab).
*/
#define MAX_SPLIT_OF_AN_ACTIVITY (35)

/**
This constant represents an unallocated activity
*/
#define UNALLOCATED_ACTIVITY (MAX_ACTIVITIES)

/**
The maximum number of working hours per day.
DEPRECATED COMMENT BELOW
IMPORTANT: max hours per day * max days per week = max hours per week must be qint16 (max 32767),
because each time is qint16 and unallocated time is qint16
*/
#define MAX_HOURS_PER_DAY (60)

/**
The maximum number of working days per week.
DEPRECATED COMMENT BELOW
IMPORTANT: max hours per day * max days per week = max hours per week must be qint16 (max 32767)
because each time is qint16 and unallocated time is qint16
*/
#define MAX_DAYS_PER_WEEK (35)

/**
The maximum number of working hours in a week.
DEPRECATED COMMENT BELOW
IMPORTANT: MAX_HOURS_PER_DAY * MAX_DAYS_PER_WEEK == MAX_HOURS_PER_WEEK must be qint16 (max 32767)
because each time is qint16 and unallocated time is qint16
*/
#define MAX_HOURS_PER_WEEK (MAX_HOURS_PER_DAY * MAX_DAYS_PER_WEEK)

/**
This constant represents unallocated time for an activity
*/
#define UNALLOCATED_TIME (MAX_HOURS_PER_WEEK)

#define UNALLOCATED_TEACHER (MAX_TEACHERS)

/**
This constants represents free periods of a teacher in the teachers free periods timetable
*/
#define TEACHER_HAS_SINGLE_GAP 0
#define TEACHER_HAS_BORDER_GAP 1
#define TEACHER_HAS_BIG_GAP 2

#define TEACHER_MUST_COME_EARLIER 4
#define TEACHER_MUST_COME_MUCH_EARLIER 6

#define TEACHER_MUST_STAY_LONGER 3

#define TEACHER_MUST_STAY_MUCH_LONGER 5		// BE CAREFUL, I just print into LESS_DETAILED timetable, if it's smaller than TEACHER_MUST_STAY_MUCH_LONGER

#define TEACHER_HAS_A_FREE_DAY 7

#define TEACHER_IS_NOT_AVAILABLE 8

#define TEACHERS_FREE_PERIODS_N_CATEGORIES 9

struct defs
{
    static const QString COMPANY;
    static const QString PROGRAM;

    /**
    The language
    */
    static QString FET_LANGUAGE;

    static bool LANGUAGE_STYLE_RIGHT_TO_LEFT;

    static QString LANGUAGE_FOR_HTML;

    /**
    The name of the file from where the rules are read.
    */
    static QString INPUT_FILENAME_XML;

    /**
    Timetable html css javaScript Level, added by Volker Dirr
    */
    static int TIMETABLE_HTML_LEVEL;

    static bool TIMETABLE_HTML_PRINT_ACTIVITY_TAGS;

    static bool PRINT_DETAILED_HTML_TIMETABLES;

    static bool PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS;

    static bool PRINT_NOT_AVAILABLE_TIME_SLOTS;

    static bool PRINT_BREAK_TIME_SLOTS;

    static bool DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS;

    static bool TIMETABLE_HTML_REPEAT_NAMES;

    static bool PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME;

    static bool USE_GUI_COLORS;

    static bool SHOW_SUBGROUPS_IN_COMBO_BOXES;
    static bool SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING;

    static bool WRITE_TIMETABLE_CONFLICTS;

    static bool WRITE_TIMETABLES_STATISTICS;
    static bool WRITE_TIMETABLES_XML;
    static bool WRITE_TIMETABLES_DAYS_HORIZONTAL;
    static bool WRITE_TIMETABLES_DAYS_VERTICAL;
    static bool WRITE_TIMETABLES_TIME_HORIZONTAL;
    static bool WRITE_TIMETABLES_TIME_VERTICAL;

    static bool WRITE_TIMETABLES_SUBGROUPS;
    static bool WRITE_TIMETABLES_GROUPS;
    static bool WRITE_TIMETABLES_YEARS;
    static bool WRITE_TIMETABLES_TEACHERS;
    static bool WRITE_TIMETABLES_TEACHERS_FREE_PERIODS;
    static bool WRITE_TIMETABLES_ROOMS;
    static bool WRITE_TIMETABLES_SUBJECTS;
    static bool WRITE_TIMETABLES_ACTIVITY_TAGS;
    static bool WRITE_TIMETABLES_ACTIVITIES;

    static bool SHOW_SHORTCUTS_ON_MAIN_WINDOW;

    static bool SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES;

    static bool ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY;
    static bool ENABLE_STUDENTS_MAX_GAPS_PER_DAY;

    static bool SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS;

    static bool SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES;

    static bool ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS;
    static bool SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS;

    static bool CONFIRM_ACTIVITY_PLANNING;
    static bool CONFIRM_SPREAD_ACTIVITIES;
    static bool CONFIRM_REMOVE_REDUNDANT;
    static bool CONFIRM_SAVE_TIMETABLE;

    static bool VERBOSE;


    /**
    The predefined names of the days of the week
    */
    static const QString PREDEFINED_DAYS_OF_THE_WEEK[];

    /**
    File and directory separator
    */
    static const QString FILE_SEP;

    /**
    The working directory
    */
    static QString WORKING_DIRECTORY;

    /**
    The import directory
    */
    static QString IMPORT_DIRECTORY;

    //OUTPUT FILES

    /**
    The output directory. Please be careful when editing it,
    because the functions add a defs::FILE_SEP sign at the end of it
    and then the name of a file. If you make defs::OUTPUT_DIR="",
    there might be problems.
    */
    static QString OUTPUT_DIR;

    /**
    A log file explaining how the xml input file was parsed
    */
    static const QString XML_PARSING_LOG_FILENAME;

    static bool checkForUpdates;

    static QString internetVersion;
};

#endif

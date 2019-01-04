#ifndef TIMERUTILS_H
#define TIMERUTILS_H

#include <QDateTime>

#include "stringutils.h"

#include <ctime>

namespace utils{

namespace time {
    inline QString secondsToStr(size_t seconds){
        QString s;

        size_t dd=seconds/86400;
        seconds%=86400;

        size_t hh=seconds/3600;
        seconds%=3600;

        size_t mm=seconds/60;
        seconds%=60;

        size_t ss=seconds;

        if (dd > 0)
            s += utils::strings::toIntegerString(dd, 2).replace(" ","0") + "d ";

        s += utils::strings::toIntegerString(hh, 2).replace(" ","0") + "h ";
        s += utils::strings::toIntegerString(mm, 2).replace(" ","0") + "m ";
        s += utils::strings::toIntegerString(ss, 2).replace(" ","0") + "s ";

        return s;
    }

    inline QString currentStrTime(QString localeStr="en_US"){
        QDate dat=QDate::currentDate();
        QTime tim=QTime::currentTime();
        QLocale loc(localeStr);
        QString sTime=loc.toString(dat, QLocale::ShortFormat)+" "+loc.toString(tim, QLocale::ShortFormat);
        return sTime;
    }

    class ClockTimer
    {
        QDateTime startDateTime;
        QDateTime endDateTime;

        clock_t clock_t_inicio = 0, clock_t_fim = 0;

    public:
        ClockTimer() {}

        void init_clock()
        {
            clock_t_fim = 0;

            std::srand(unsigned(std::time(nullptr)));

            clock_t_inicio = clock();
            startDateTime=QDateTime::currentDateTime();
        }

        void end_clock()
        {
            clock_t_fim = clock();
            endDateTime=QDateTime::currentDateTime();
        }

        clock_t elapsedTime()
        {
            return (clock_t_fim == 0 ? clock() : clock_t_fim) - clock_t_inicio;
        }

        double elapsedSeconds()
        {
            return ((double)elapsedTime()) / CLOCKS_PER_SEC ;
        }

        double elapsedMinutes()
        {
            return elapsedSeconds() / 60 ;
        }

        void print_timer_result()
        {
            printf("Elapsed time: %f seconds\n", elapsedSeconds());
        }
    };
}
}
#endif // TIMERUTILS_H

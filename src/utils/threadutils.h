#ifndef THREADUTILS_H
#define THREADUTILS_H

#include <QThread>
#include "defs.h"

namespace utils{

namespace thread {

unsigned int getIdealThreadCount(){

    int number_of_cpu_cores = QThread::idealThreadCount();

    unsigned int nt = 1;

    nt = (number_of_cpu_cores > 0) ? (unsigned int) number_of_cpu_cores : 1;
    nt = (nt <= MAX_THREADS) ? nt : MAX_THREADS;

    return nt;
}

}

}

#endif // THREADUTILS_H

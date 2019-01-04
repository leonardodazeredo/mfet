#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <QString>
#include <cstdarg>
#include <string>
#include <iostream>

using namespace std;

namespace utils{

namespace console {
    inline void cout(string msg, ...)
    {
        QString qS;
        qS = QString::fromStdString(msg);

        int count = qS.count(QLatin1Char('%'));

        va_list ap;

        va_start(ap, msg); //Requires the last fixed parameter (to get the address)

        for(int i=0; i<count; i++){

            string s = va_arg(ap, char*);
            qS = qS.arg(QString::fromStdString(s));
        }

        va_end(ap);

        std::cout << qS.toStdString();
        std::cout.flush();
    }

    inline void coutnl(string msg, ...)
    {
        QString qS;
        qS = QString::fromStdString(msg);

        int count = qS.count(QLatin1Char('%'));

        va_list ap;

        va_start(ap, msg); //Requires the last fixed parameter (to get the address)

        for(int i=0; i<count; i++){

            string s = va_arg(ap, char*);
            qS = qS.arg(QString::fromStdString(s));
        }

        va_end(ap);

        std::cout << qS.toStdString() << endl;
        std::cout.flush();
    }
}

}
#endif // LOGUTILS_H

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include "textmessages.h"

#include "stringutils.h"

#include <iostream>

using namespace std;

namespace utils{

namespace file {
    inline bool exists(QString path) {
        QFileInfo check_file(path);
        // check if file exists and if yes: Is it really a file and no directory?
        if (check_file.exists() && check_file.isFile()) {
            return true;
        } else {
            return false;
        }
    }

    inline QString makeIndexedFilePath(QString originalFilePath) {
        QString newFilePath=originalFilePath;

        if(QFile::exists(newFilePath)){
            for(int i=1; ; i++){
                QString auxStr = newFilePath + " (" + utils::strings::number(i) + ")";

                if(!QFile::exists(auxStr)){
                    newFilePath=auxStr;
                    break;
                }
            }
        }

        return newFilePath;
    }

    inline bool makeDirAndCheckWritePermissions(QString path, QString fileSep="/"){
        QDir dir;

        if(path!="")
            if(!dir.exists(path))
                dir.mkpath(path);

        if(path!="")
            path.append(fileSep);

        QString tmpFileName = QString("test_write_permissions_%1.tmp").arg(utils::strings::randomGenericString());

        QFile test(path+tmpFileName);

        bool existedBefore=test.exists();

        if(!test.open(QIODevice::ReadWrite)){
            MessagesManager::critical(nullptr, "Error", "You don't have write permissions in the directory - (cannot open or create file " + path + tmpFileName);
            return false;
        }
        else{
            test.close();
            if(!existedBefore)
                test.remove();
        }

        return true;
    }
}

}
#endif // FILEUTILS_H

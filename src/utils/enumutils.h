#ifndef ENUMUTILS_H
#define ENUMUTILS_H

#include <QMetaEnum>


namespace utils{

namespace enums {
    template<class E>
    inline E enumStrToEnumValue(std::string text, E defaultValue){
        E e(defaultValue);

        if (text!="") {
            bool ok;

            QMetaEnum metaEnum = QMetaEnum::fromType<E>();
            int eint = metaEnum.keyToValue(text.c_str(), &ok);

            if (ok) {
                e = eint >= 0 ? (E)eint : e;
            }
        }

        return e;
    }

    template<class E>
    inline E enumStrToEnumValue(std::string text){
        if (text!="") {
            bool ok;

            QMetaEnum metaEnum = QMetaEnum::fromType<E>();
            int eint = metaEnum.keyToValue(text.c_str(), &ok);

            if (ok) {
                return (eint >= 0 ? (E)eint : (E)0);
            }
        }

        return (E)0;
    }

    template<class T>
    inline QString enumIndexToStr(T index){
        T e = (T) index;

        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        QString qs(metaEnum.valueToKey(e));

        return qs;
    }

    template<class T>
    inline int enumStrToIndex(QString s, bool *ok){
        if (s.trimmed().size() == 0) {
            return -1;
        }

        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        return metaEnum.keyToValue(s.toStdString().c_str(), ok);
    }

    template<class T>
    inline int enumStrToIndex(std::string s, bool *ok){
        if (s.size() == 0) {
            return -1;
        }

        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        return metaEnum.keyToValue(s.c_str(), ok);
    }
}
}
#endif // ENUMUTILS_H

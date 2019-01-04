#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#define CUSTOM_DOUBLE_PRECISION 6 //number of digits after the decimal dot for the weights

#define AS_PERCENTAGE(v) ((v) * 100)
#define AS_RELATIVE_ASPERCENTAGE(v) ((v - 1) * 100)

#define C_STR(s) s.toStdString().c_str()

#include <QCoreApplication>
#include <QLocale>

#include <cassert>

#include "randutils.h"

namespace utils{

namespace strings {
    inline QString number(unsigned int n)
    {
        return QString::number(n);
    }

    inline QString number(int n)
    {
        return QString::number(n);
    }

    inline QString number(double x, int p)
    {
        return QString::number(x, 'f', p);
    }

    inline QString number(double x)
    {
        QString tmp=QString::number(x, 'f', CUSTOM_DOUBLE_PRECISION);

        //remove trailing zeroes AFTER decimal points
        if(tmp.contains('.')){
            int n=tmp.length()-1;
            int del=0;
            while(tmp.at(n)=='0'){
                n--;
                del++;
            }
            if(tmp.at(n)=='.'){
                n--;
                del++;
            }
            tmp.chop(del);
        }

        return tmp;
    }

    inline double customFETStrToDouble(const QString& str, bool* ok=nullptr)
    {
        QLocale c(QLocale::C);

        //tricks to convert numbers like 97.123456789 to 97.123457, to CUSTOM_DOUBLE_PRECISION (6) decimal digits after decimal point
        double tmpd=c.toDouble(str, ok);
        if(ok!=nullptr)
            if((*ok)==false)
                return tmpd;
        QString tmps=utils::strings::number(tmpd);
        return c.toDouble(tmps, ok);
    }

    inline void weight_sscanf(const QString& str, const char* fmt, double* result)
    {
        assert(QString(fmt)==QString("%lf"));

        bool ok;
        double myres=utils::strings::customFETStrToDouble(str, &ok);
        if(!ok)
            (*result)=-2.5; //any value that does not belong to {>=0.0 and <=100.0} or {-1.0}
                            //not -1.0 because of modify multiple constraints min days between activities,
                            //-1 there represents any weight
                            //potential bug found by Volker Dirr
        else
            (*result)=myres;
    }

    inline QString parseStrForXml(const QString& str) //used for xml
    {
        QString p=str;
        p.replace("&", "&amp;");
        p.replace("\"", "&quot;");
        p.replace(">", "&gt;");
        p.replace("<", "&lt;");
        p.replace("'", "&apos;");
        return p;
    }

    inline QString parseStrForHtml(const QString& str) //used for html
    {
        QString p=str;
        p.replace("&", "&amp;");
        p.replace("\"", "&quot;");
        p.replace(">", "&gt;");
        p.replace("<", "&lt;");
        //p.replace("'", "&apos;");
        return p;
    }

    inline QString parseStrForHtmlVertical(const QString& str) //used for html
    {
        QString p=str;
        p.replace("&", "&amp;");
        p.replace("\"", "&quot;");
        p.replace(">", "&gt;");
        p.replace("<", "&lt;");
        //p.replace("'", "&apos;");

        QString returnstring;
        for(int i=0; i<p.size();i++){
            QString a=p.at(i);
            QString b="<br />";
            returnstring.append(a);
            returnstring.append(b);
        }
        return returnstring;
    }

    inline QString trueFalse(bool x){
        if(!x)
            return QString("false");
        else
            return QString("true");
    }

    inline QString yesNoTranslated(bool x){
        if(!x)
            return QCoreApplication::translate("TimeConstraint", "no", "no - meaning negation");
        else
            return QCoreApplication::translate("TimeConstraint", "yes", "yes - meaning affirmative");
    }

    template<class floatDouble>
    inline QString toFloatString(floatDouble number, int tam){
        QString test = "%1";
        return test.arg(number, tam, 'f', 2);
    }

    template<class floatDouble>
    inline QString toFloatString(floatDouble number, int tam, int presi){
        QString test = "%1";
        return test.arg(number, tam, 'f', presi);
    }

    template<class floatDouble>
    inline QString toPercentageString(floatDouble number, int tam, int presi){
        QString test = "%1%";
        return test.arg(AS_PERCENTAGE(number), tam, 'f', presi);
    }

    template<class Int>
    inline QString toIntegerString(Int number, int tam){
        QString test = "%1";
        return test.arg(number, tam);
    }

    inline QString toFixedSizeString(QString str, int tam){
        QString test = "%1";
        return test.arg(str, tam);
    }

    inline QString makeIndent(QString str){
        QString test = "%1";
        return test.arg("", str.size() * 4);
    }

    inline QString makeIndent(int tam){
        QString test = "%1";
        return test.arg("", tam * 4);
    }

    inline QString randomGenericString(const int randomStringLength=10){
       const QString auxStr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
       QStringList possibleCharacters = auxStr.split("");

       QString randomString;
       for(int i=0; i<randomStringLength; ++i){
           QString nextChar = utils::random::pick(possibleCharacters);
           randomString.append(nextChar);
       }
       return randomString;
    }
}
}
#endif // STRINGUTILS_H

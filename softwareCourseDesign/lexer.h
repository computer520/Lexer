#ifndef LEXER_H
#define LEXER_H

#include<QString>
#include<QVector>
#include "automation.h"

class lexer
{
public :
    struct Token
    {
        int id;
        QString type;
        QString content;
    };
public:
    lexer();
    QVector<QString> divideSourceCode(QString Sourcecontent);
    QVector<Token> lexerCode(QVector<QString> code,QVector<automation::DFA> dfa);
    bool isIdentify(QString str,automation::DFA dfa);

};

#endif // LEXER_H

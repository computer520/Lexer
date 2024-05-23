#ifndef AUTOMATION_H
#define AUTOMATION_H
#include <QVector>
#include <QString>
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <QPair>
class automation
{
public:
    struct Func
    {
        QString start_state;
        QString middle_state;
        QString end_state;
    };
    struct NFA
    {
        QVector<QString> K;
        QVector<QString> Sum;
        QVector<Func> f;
        QVector<QString> S;
        QVector<QString> Z;
    };

    struct DFA
    {
        QVector<QString> K;//有穷集states
        QVector<QString> Sum;//字母表alphabet
        QVector<Func> f;//转换函数transitions
        QVector<QString> S;//初态start_states
        QVector<QString> Z;//终态集accept_states
    };
public:
    automation();
    QVector<QString> divideExpression(QString filecontent);
    NFA grammarToNFA(QVector<QString>& expression);
    QVector<QString> eps_closure(QVector<QString> state_set,NFA nfa);
    QVector<QString> move(QVector<QString> state_set,QVector<Func> fun,QString input);
    DFA NFAToDFA(NFA nfa);
    DFA minDFA(const DFA &dfa);
};

#endif // AUTOMATION_H

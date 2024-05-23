#ifndef LR1_GRAMMAR_H
#define LR1_GRAMMAR_H

#include<QVector>
#include<QSet>
#include<QStack>
#include<QMap>
#include<QQueue>
#include<QDebug>
#include<QString>
#include<QPair>
#include<QRegularExpression>
#include"lexer.h"

class LR1_Grammar
{
public:
    LR1_Grammar();
    struct Grammar_regular{
        QString left;
        QString right;
        bool operator==(const Grammar_regular& lhs) const{
            return lhs.left==left&&lhs.right==right;
        }
    };
    struct Grammar2{
        QVector<QString> Vn;
        QVector<QString> Vt;
        QVector<Grammar_regular> P;
        QString S;
    };

    struct LR1Item{
        QString production;//产生式
        QString lookahead;//向前搜索符
        LR1Item(QString pro,QString look):production(pro),lookahead(look){}
        bool operator==(const LR1Item& lhs) const{
            return lhs.production == production && lhs.lookahead == lookahead;
        }
        bool operator<(const LR1Item& other)const{
            return this->lookahead<other.lookahead&&this->production<other.production;
        }
        QString toString() const {
            // 这里返回你希望在输出中看到的信息，例如产生式
            return production;
        }
    };
    struct GOTO{
        int start;
        QString symbol;
        int end;
    };
    struct ACTION
    {
        QString type;
        int k;
        QString a;
        QString S_j;
    };
    QMap<QString,QSet<QString>> firstset;//first集
    Grammar2 grammar;//上下文无关文法
    QVector<QString> express;//每一个表达式

    typedef QVector<LR1Item> LR1ItemSet;
public:
    QVector<QString> buildExpress(QString grammarContent);//获得文法的每一条产生式
    Grammar2 define2Grammar();
    QMap<QString,QSet<QString>> computeFirst(Grammar2 grammar);



    LR1ItemSet clourse(LR1ItemSet items);//闭包算法
    LR1ItemSet gotoFunction(const LR1ItemSet& itemSet,const QString& symbol);//go函数

    QVector<ACTION> bulidStates(const LR1ItemSet& startSet);//计算ACTION-GOTO表
    int findPos(QVector<LR1ItemSet> state,LR1ItemSet itemset);//计算位置
    QString isLegal(QVector<ACTION> actionGoto,QVector<lexer::Token> Token);//判断token序列是否合法

    ACTION getAction(int state,QString symbol,QVector<ACTION> action_goto);
};

#endif // LR1_GRAMMAR_H

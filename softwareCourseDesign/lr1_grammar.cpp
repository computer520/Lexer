#include "lr1_grammar.h"

LR1_Grammar::LR1_Grammar() {}

QVector<QString> LR1_Grammar::buildExpress(QString grammarContent)
{
    QVector<QString> result;
    QStringList lines = grammarContent.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QStringList parts=line.split("->");
        if(parts.size()==2)
        {
            QString start_state=parts[0];
            QString transitions=parts[1];


            QStringList transitionList=transitions.split('|',Qt::SkipEmptyParts);
            for(const QString& transition:transitionList)
            {
                QString end_newbody=transition;
                QString end_temp=start_state+"->"+end_newbody;
                result.push_back(end_temp);
            }
        }
    }
    express=result;
    return result;
}


LR1_Grammar::Grammar2 LR1_Grammar::define2Grammar()
{
    Grammar2 gram;
    gram.S="S'";
    QVector<QString> specialTerminals = {"==", ">=", "<=", "!="};
    for(const auto& rule:express)
    {
        QStringList ruleList=rule.split("->",Qt::SkipEmptyParts);
        QString left=ruleList[0];
        QString right=ruleList[1];
        Grammar_regular p;
        p.left=left;
        p.right=right;
        if(!gram.P.contains(p))
        {
            gram.P.append(p);
        }
        if(!gram.Vn.contains(left))
        {
            gram.Vn.append(left);
        }
        // Process right-hand side of the production
        int i = 0;
        while (i < right.size()) {
            bool matched = false;

            // Check for multi-character terminals
            for (const QString& term : specialTerminals) {
                if (right.mid(i, term.size()) == term) {
                    if (!gram.Vt.contains(term)) {
                        gram.Vt.append(term);
                    }
                    i += term.size();
                    matched = true;
                    break;
                }
            }

            if (!matched) {
                QString ch = right.mid(i, 1);
                if (ch[0].isLetter() && ch[0].isUpper()) {
                    if (!gram.Vn.contains(ch)) {
                        gram.Vn.append(ch);
                    }
                } else {
                    if (!gram.Vt.contains(ch)&& ch!=" ") {
                        gram.Vt.append(ch);
                    }
                }
                ++i;
            }
        }
    }
    grammar=gram;
    return gram;
}

QMap<QString, QSet<QString> > LR1_Grammar::computeFirst(Grammar2 grammar)
{
    QMap<QString, QSet<QString>> firstSets;

    // 初始化
    for (const QString& terminal : grammar.Vt) {
        firstSets[terminal] = { terminal };
    }

    for (const QString& nonTerminal : grammar.Vn) {
        firstSets[nonTerminal] = {};
    }

    bool changed = true;

    // 迭代直到first不变化
    while (changed) {
        changed = false;

        for (const Grammar_regular& rule : grammar.P) {
            const QString& left = rule.left;
            const QString& right = rule.right;
            QSet<QString> tempSet;
            if(grammar.Vt.contains(right))
            {
                tempSet.unite(firstSets[right]);
            }
            else
            {
                for (int i = 0; i < right.size(); ++i) {
                    QString symbol = right.mid(i, 1);

                    // 合并
                    tempSet.unite(firstSets[symbol]);


                    if (!firstSets[symbol].contains("ε")) {
                        break;
                    }

                    if (i == right.size() - 1) {
                        tempSet.insert("ε");
                    }
                }
            }

            int originalSize = firstSets[left].size();
            firstSets[left].unite(tempSet);

            if (firstSets[left].size() != originalSize) {
                changed = true;
            }
        }
    }
    firstset=firstSets;
    return firstSets;
}


LR1_Grammar::LR1ItemSet LR1_Grammar::clourse(LR1ItemSet items)
{
    QVector<LR1Item> clourseSet=items;
    bool changed=true;
    while(changed)
    {
        changed=false;
        QVector<LR1Item> newItems;
        for(const LR1Item& item:clourseSet)
        {
            int pos=item.production.indexOf("~");
            if(pos==-1||pos+1>=item.production.length())
            {
                continue;
            }

            QString symbol=item.production.mid(pos+1,1);
            if(grammar.Vn.contains(symbol))
            {
                QString remaining=item.production.mid(pos+2);
                QSet<QString> lookaheads;
                if(remaining.isEmpty())
                {
                    lookaheads.insert(item.lookahead);
                }
                else
                {
                    QString firstSymbol=remaining[0];
                    lookaheads=firstset[firstSymbol];
                }
                for(const QString& pro:express)
                {
                    if(pro.startsWith(symbol+"->"))
                    {
                        QString newProduction=symbol+"->~"+pro.mid(symbol.length()+2);
                        if(pro.mid(symbol.length()+2)=="ε")
                        {
                            newProduction=symbol+"->~";
                        }
                        else
                        {
                            newProduction=symbol+"->~"+pro.mid(symbol.length()+2);
                        }

                        for(const QString &lookahead:lookaheads)
                        {
                            LR1Item newItem(newProduction,lookahead);
                            if(!clourseSet.contains(newItem))
                            {
                                newItems.push_back(newItem);
                                changed=true;
                            }
                        }
                    }
                }
            }
        }
        for(const LR1Item& item:newItems)
        {
            if(!clourseSet.contains(item))
            {
                clourseSet.push_back(item);
            }
        }
    }
    return clourseSet;
}


LR1_Grammar::LR1ItemSet LR1_Grammar::gotoFunction(const LR1ItemSet &itemSet, const QString &symbol)
{
    LR1ItemSet movedItems;
    int length=symbol.size();
    QVector<QString> allSymbol=grammar.Vn+grammar.Vt;
    for(const LR1Item& item:itemSet)
    {
        int dotPos=item.production.indexOf("~");
        if(dotPos==-1||dotPos>=item.production.length())
        {
            continue;
        }
        QString str=item.production.mid(dotPos+1);
        if(str.size()!=length && allSymbol.contains(str))
        {
            continue;
        }
        QString nextSymbol=item.production.mid(dotPos+1,length);

        if(nextSymbol==symbol)
        {
            QString newProduction=item.production;
            newProduction.remove(dotPos,1);
            newProduction.insert(dotPos+length,"~");

            movedItems.append(LR1Item(newProduction,item.lookahead));
        }
    }
    return clourse(movedItems);
}

QVector<LR1_Grammar::ACTION> LR1_Grammar::bulidStates(const LR1ItemSet &startSet)
{
    QVector<LR1ItemSet> states;
    QQueue<LR1ItemSet> queue;
    QVector<GOTO> gotoFile;

    LR1ItemSet initialClourse=clourse(startSet);

    queue.enqueue(initialClourse);
    states.append(initialClourse);
    QVector<QString> allSymbol=grammar.Vn+grammar.Vt;

    while(!queue.isEmpty())
    {
        LR1ItemSet currentSet=queue.dequeue();
        int start=findPos(states,currentSet);
        GOTO gotof;

        for(const QString& symbol:allSymbol)
        {
            LR1ItemSet nextSet=gotoFunction(currentSet,symbol);
            if(!nextSet.isEmpty())
            {
                if(states.contains(nextSet))
                {
                    gotof.start=start;
                    gotof.symbol=symbol;
                    gotof.end=findPos(states,nextSet);
                    gotoFile.append(gotof);
                }
                else
                {
                    queue.enqueue(nextSet);
                    states.append(nextSet);

                    gotof.start=start;
                    gotof.symbol=symbol;
                    gotof.end=states.size()-1;
                    gotoFile.append(gotof);
                }
            }
        }
    }
    QVector<ACTION> resultActionGoto;
    QVector<ACTION> Action;
    QVector<GOTO> goTo;
    for(int i=0;i<states.size();i++)
    {
        for(const auto& state:states[i])
        {
            QStringList stateList=state.production.split("->");
            if(stateList[1]=="~")
            {
                QString str=stateList[0]+"->ε";
                for(int j=0;j<express.size();j++)
                {
                    if(express[j]==str)
                    {
                        ACTION act;
                        act.type="ACTION";
                        act.k=i;
                        act.a=state.lookahead;
                        act.S_j="r_"+QString::number(j);
                        Action.append(act);
                    }
                }
            }

        }
    }

    for(const auto& fun:gotoFile)
    {
        if(grammar.Vn.contains(fun.symbol))
        {
            goTo.append(fun);
        }
        else
        {
            ACTION act;
            act.type="ACTION";
            act.k=fun.start;
            act.a=fun.symbol;
            act.S_j="S_"+QString::number(fun.end);
            Action.append(act);
        }
    }
    for(int i=0;i<states.size();i++)
    {
        for(int j=0;j<states[i].size();j++)
        {
            QString str=states[i][j].production;
            if(str=="S'->A~")
            {
                ACTION act;
                act.type="ACTION";
                act.k=i;
                act.a=states[i][j].lookahead;
                act.S_j="acc";
                Action.append(act);
                break;
            }
            int pos=str.indexOf("~");
            if(pos+1==str.size())
            {
                str.chop(1);
                for(int k=0;k<express.size();k++)
                {
                    if(str==express[k])
                    {
                        ACTION act;
                        act.type="ACTION";
                        act.k=i;
                        act.a=states[i][j].lookahead;
                        act.S_j="r_"+QString::number(k);
                        Action.append(act);
                        break;
                    }
                }
            }

        }
    }

    for(const auto& fun:goTo)
    {
        ACTION act;
        act.type="GOTO";
        act.k=fun.start;
        act.a=fun.symbol;
        act.S_j=QString::number(fun.end);
        resultActionGoto.append(act);
    }
    for(const auto& act:Action)
    {
        resultActionGoto.append(act);
    }
    return resultActionGoto;
}

int LR1_Grammar::findPos(QVector<LR1ItemSet> state, LR1ItemSet itemset)
{
    int pos=-1;
    for(int i=0;i<state.size();i++)
    {
        if(state[i]==itemset)
        {
            pos=i;
            break;
        }
    }
    return pos;
}

QString LR1_Grammar::isLegal(QVector<ACTION> actionGoto, QVector<lexer::Token> Token)
{
    QString result="";
    QStack<int> stateStack;
    QStack<QString> symbolStack;
    stateStack.push(0);
    symbolStack.push("#");
    QVector<QString> cinstr;
    bool flag=false;
    QVector<QString> addWord={"+","-"};
    QVector<QString> powWord={"*","/"};
    QVector<QString> operatorword={"<","<=","==",">",">="};
    QVector<QString> keyword={"int","double","char","bool","float"};

    for(const auto& toke:Token)
    {
        QString str="";
        if(toke.content=="(")
        {
            str="x";
            cinstr.append(str);
        }
        if(toke.content==")")
        {
            str="y";
            cinstr.append(str);
        }
        if(toke.content=="const")
        {
            str="a";
            cinstr.append(str);
        }
        if(toke.content==",")
        {
            str="c";
            cinstr.append(str);
        }
        if(toke.content==";")
        {
            str="b";
            cinstr.append(str);
        }
        if(toke.content=="=")
        {
            str="e";
            cinstr.append(str);
        }
        if(toke.content=="do")
        {
            str="o";
            cinstr.append(str);
        }
        if(toke.content=="else")
        {
            str="l";
            cinstr.append(str);
        }
        if(toke.content=="if")
        {
            str="j";
            cinstr.append(str);
        }
        if(toke.content=="while")
        {
            flag=true;
            str="n";
            cinstr.append(str);
        }
        if(addWord.contains(toke.content))
        {
            str="h";
            cinstr.append(str);
        }
        if(powWord.contains(toke.content))
        {
            str="i";
            cinstr.append(str);
        }
        if(operatorword.contains(toke.content))
        {
            str="m";
            cinstr.append(str);
        }
        if(keyword.contains(toke.content))
        {
            str="g";
            cinstr.append(str);
        }
        if(toke.type=="Identify"&&flag==false)
        {
            str="d";
            cinstr.append(str);
        }
        else if(toke.type=="Constant")
        {
            str="f";
            cinstr.append(str);
        }
    }
    QString inputstr="";
    for(const auto& c:cinstr)
    {
        inputstr=inputstr+c;
    }
    inputstr=inputstr+"#";
    qDebug()<<inputstr;
    int index=0;
    int count=0;
    while(true)
    {
        int currentState=stateStack.top();
        QString currentToke=inputstr.mid(index,1);
        if(currentToke=="#")
        {
            count++;
        }
        if(count>0&&currentToke=="")
        {
            currentToke="#";
        }
        ACTION action=getAction(currentState,currentToke,actionGoto);
        qDebug()<<"["<<action.k<<","<<action.a<<"]="<<action.S_j;
        if(action.k==-1)
        {
            qDebug()<<"Error:ACTION-GOTO表不包含";
            QString message="no";
            result=message;
            return result;
        }
        if(action.S_j=="acc")
        {
            qDebug()<<"Accept";
            QString message="yes";
            result=message;
            return result;
        }
        else if(action.S_j[0]=='S')
        {
            qDebug()<<"移进";
            int nextState=action.S_j.mid(2).toInt();
            stateStack.push(nextState);
            symbolStack.push(currentToke);
            index++;
        }
        else if(action.S_j[0]=='r')//规约
        {
            qDebug()<<"规约";
            int ruleIndex=action.S_j.mid(2).toInt();
            QString rule=express[ruleIndex];
            QStringList ruleList=rule.split("->");
            QString left=ruleList[0];
            QString right=ruleList[1];
            int popCount=0;

            if(right!="ε")
            {
                popCount=right.size();
                for (int i = 0; i < popCount; ++i) {
                    stateStack.pop();
                    symbolStack.pop();
                }
            }
            int nextState=stateStack.top();
            QString S=getAction(nextState,left,actionGoto).S_j;
            int state=S.toInt();
            stateStack.push(state);
            symbolStack.push(left);
        }
        else
        {
            qDebug()<<"Other Error";
            QString message="no";
            result=message;
            return result;
        }
    }
    return result;
}

LR1_Grammar::ACTION LR1_Grammar::getAction(int state, QString symbol, QVector<ACTION> action_goto)
{
    for(const auto&action:action_goto)
    {
        if(action.k==state&& action.a==symbol)
        {
            return action;
        }
    }
    return {"",-1,"",""};
}

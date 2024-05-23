#include "automation.h"

automation::automation() {}

QVector<QString> automation::divideExpression(QString filecontent)
{
    QVector<QString> expressions;
    // 使用split函数将filecontent按照换行符分割成多个字符串
    QStringList lines = filecontent.split("\n", Qt::SkipEmptyParts);

    expressions.reserve(lines.size()); // 提前分配足够的内存以提高性能

    for (const QString& line : lines) {
        expressions.append(line);
    }

    return expressions;
}

automation::NFA automation::grammarToNFA(QVector<QString> &expression)
{
    QVector<Func> func;
    NFA nfa;
    for (const QString& rule : expression) {
        QStringList parts = rule.split("->");
        if (parts.size() == 2) {
            QString start_state = parts[0];
            QString transitions = parts[1];

            // 处理多个转换的情况
            QStringList transitionList = transitions.split('|');
            for (const QString& transition : transitionList) {
                QStringList transitionParts;
                QString currentPart;
                for (const QChar& ch : transition) {
                    if (ch.isLower() && !currentPart.isEmpty() && currentPart.back().isLower()) {
                        // 如果当前字符是小写字母且当前正在构建的单词也是小写字母，则追加到当前单词中
                        currentPart.append(ch);
                    }
                    else if (ch.isUpper() && !currentPart.isEmpty()) {
                        // 如果当前字符是大写字母且当前正在构建的单词不为空，则将当前单词加入到列表中，并重新开始构建下一个单词
                        int count=0;
                        for(const auto& part:currentPart)
                        {
                            if(part=='\'')
                            {
                                count++;
                            }
                        }
                        if(count==1)
                        {
                            currentPart.append(ch);
                        }
                        else
                        {
                            transitionParts.append(currentPart);
                            currentPart = ch;
                        }
                    } else {
                        // 其他情况直接追加到当前单词中
                        currentPart.append(ch);
                    }
                }
                // 将最后一个单词加入到列表中
                if (!currentPart.isEmpty()) {
                    transitionParts.append(currentPart);
                }
                transitionParts[0].replace("'","");
                QString middle_state = transitionParts[0];
                QString end_state = (transitionParts.size() > 1) ? transitionParts[1] : "Z";
                func.append({start_state, middle_state, end_state});
            }
        }
    }
    if (!func.isEmpty())
    {
        // 将func的第一个元素的start_state加入到S中
        nfa.S.append(func.first().start_state);

        // 将终态Z加入到Z中
        nfa.Z.append("Z");
    }

    for (const Func &transition : func)
    {
        // 如果start_state不存在于K中，则加入
        if (!nfa.K.contains(transition.start_state))
        {
            nfa.K.append(transition.start_state);
        }

        // 如果end_state不存在于K中，则加入
        if (!nfa.K.contains(transition.end_state))
        {
            nfa.K.append(transition.end_state);
        }

        // 如果middle_state不存在于Sum中，则加入
        if (!nfa.Sum.contains(transition.middle_state)&&transition.middle_state.compare("ε"))
        {
            nfa.Sum.append(transition.middle_state);
        }
    }
    nfa.f = func;
    return nfa;
}

QVector<QString> automation::eps_closure(QVector<QString> state_set,NFA nfa)//闭包算法
{
    bool newStatesAdded = false;
    do {
        newStatesAdded = false;

        for (int i = 0; i < state_set.size(); ++i) {
            QString state = state_set[i];

            for (const Func& transition : nfa.f) {
                if (transition.start_state == state && transition.middle_state.compare("ε")==0 &&
                    !state_set.contains(transition.end_state)) {
                    state_set.append(transition.end_state);
                    newStatesAdded = true;
                }
            }
        }
    } while (newStatesAdded);

    return state_set;
}

QVector<QString> automation::move(QVector<QString> state_set,QVector<Func> fun, QString input)//move转换
{
    QVector<QString> state;
    for(int i=0;i<state_set.size();++i)
    {
        QString temp=state_set[i];
        for (const Func& transition : fun)
        {
            if(transition.start_state==temp && transition.middle_state.compare(input)==0 && !state.contains(transition.end_state))
            {
                state.append(transition.end_state);
            }
        }
    }
    return state;
}

automation::DFA automation::NFAToDFA(NFA nfa)
{
    DFA dfa;
    // 初始化DFA
    dfa.Sum = nfa.Sum; // 假设NFA和DFA的字母表相同

    QMap<QVector<QString>, QString> stateMapping; // 用于映射NFA状态集到DFA状态名
    QVector<QString> startState = eps_closure(nfa.S, nfa); // 计算起始状态的ε-闭包

    QQueue<QVector<QString>> queue; // 状态探索队列
    queue.enqueue(startState);

    QString dfaStartState = "A"; // 假设DFA状态从“A”开始命名
    stateMapping[startState] = dfaStartState;
    dfa.S.append( dfaStartState);
    dfa.K.append(dfaStartState);

    int stateCounter = 1; // 用于生成新的DFA状态名

    while (!queue.isEmpty()) {
        QVector<QString> currentState = queue.dequeue();
        QString currentStateName = stateMapping[currentState];

        for (const QString &input : nfa.Sum) {
            if (input != "ε") { // ε转移已经在ε-闭包中处理
                QVector<QString> moveState=move(currentState, nfa.f, input);
                QVector<QString> newState = eps_closure(moveState, nfa);
                if (!newState.isEmpty()) {
                    QString newStateName;
                    if (!stateMapping.contains(newState)) {
                        newStateName = QString(QChar('A' + stateCounter++));
                        stateMapping[newState] = newStateName;
                        dfa.K.append(newStateName);
                        queue.enqueue(newState);
                    } else {
                        newStateName = stateMapping[newState];
                    }

                    Func transition;
                    transition.start_state = currentStateName;
                    transition.middle_state = input; // 中间状态在DFA中无用，可以不设置或设置为输入符号
                    transition.end_state = newStateName;
                    dfa.f.append(transition);

                    // 检查是否为接受状态
                    for (const QString &z : nfa.Z) {
                        if (newState.contains(z) && !dfa.Z.contains(newStateName)) {
                            dfa.Z.append(newStateName);
                            break;
                        }
                    }
                }
            }
        }
    }

    return dfa;
}

automation::DFA automation::minDFA(const DFA &dfa)
{
    //初始化：划分为接收态和非接收态
    QSet<QString> acceptStates(dfa.Z.begin(),dfa.Z.end());
    QSet<QString> nonAcceptStates;

    for (const auto& state : dfa.K) {
        if (!acceptStates.contains(state)) {
            nonAcceptStates.insert(state);
        }
    }

    //初始化分割
    QVector<QSet<QString>> partitions={acceptStates,nonAcceptStates};
    QVector<QSet<QString>> newPartitions;

    //构建状态映射
    QMap<QString, QMap<QString, QString>> transitionMap;
    for (const auto& transition : dfa.f) {
        transitionMap[transition.start_state][transition.middle_state] = transition.end_state;
    }

    //分割迭代
    bool changed=false;
    do{
        changed=false;
        newPartitions.clear();

        for(const auto& partition:partitions)
        {
            QMap<QString,QSet<QString>> partitionMap;
            for(const auto& state:partition)
            {
                QString key;
                for(const auto& symbol:dfa.Sum)
                {
                    QString endState = transitionMap[state][symbol];
                    for (int i = 0; i < partitions.size(); ++i) {
                        if (partitions[i].contains(endState)) {
                            key += QString::number(i) + ',';
                            break;
                        }
                    }
                }
                partitionMap[key].insert(state);
            }

            if(partitionMap.size()>1)
            {
                changed=true;
            }
            for(const auto& newPartition:partitionMap)
            {
                newPartitions.append(newPartition);
            }
        }
        if(changed)
        {
            partitions=newPartitions;
        }

    }while(changed);
    DFA minDFA;
    QMap<QString, QString> stateMapping;
    newPartitions=partitions;
    for (int i = 0; i < partitions.size(); ++i) {
        QString newName = "Q" + QString::number(i);
        minDFA.K.append(newName);
        for(const auto& part:partitions[i])
        {
            stateMapping[part] = newName;
        }
        if (!(partitions[i].intersect(acceptStates)).isEmpty()) {
            minDFA.Z.append(newName);
        }
    }
    for (const auto& state : minDFA.K) {
        QVector<QString> temp;
        for (const QString &original : stateMapping.keys()) {
            if(stateMapping[original]==state)
            {
                temp.append(original);
            }
        }
        for (const auto& symbol : dfa.Sum) {
            QString nextState;
            for (const auto& partition : newPartitions) {
                for (const auto& statepart : partition) {
                    if (temp.contains(transitionMap[statepart][symbol])) {
                        nextState = stateMapping[statepart];
                        Func transition;
                        transition.start_state=nextState;
                        transition.middle_state=symbol;
                        transition.end_state=state;
                        minDFA.f.append(transition);
                        break;
                    }
                }
            }
        }
    }
    minDFA.Sum = dfa.Sum;
    minDFA.S.append(stateMapping[*dfa.S.begin()]);

    return minDFA;
}


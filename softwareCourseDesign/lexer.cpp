#include "lexer.h"
#include "qregularexpression.h"

lexer::lexer() {}

QVector<QString> lexer::divideSourceCode(QString Sourcecontent)
{
    QVector<QString> content;
    // 使用split函数将filecontent按照换行符分割成多个字符串
    QStringList lines = Sourcecontent.split("\n", Qt::SkipEmptyParts);

    content.reserve(lines.size()); // 提前分配足够的内存以提高性能

    for (const QString& line : lines) {
        content.append(line);
    }

    return content;
}

QVector<lexer::Token> lexer::lexerCode(QVector<QString> code, QVector<automation::DFA> dfa)
{
    QVector<Token> result;
    QVector<Token> end_result;
    Token token;
    int pos=1;

    for(const auto& str:code)
    {
        QStringList lines=str.split(" ",Qt::SkipEmptyParts);//使用空格分割字符串
        QVector<bool> hasOperator;
        int tempnum=0;
        for (int i = 0; i < lines.size(); ++i)
        {
            const QString &line = lines[i];
            bool removed = false;

            // 检查是否属于关键字
            if (dfa[0].Sum.contains(line)) {
                token.type = "Keyword";
                token.id = pos;
                token.content = line;
                result.push_back(token);
                tempnum=line.size();
                // 删除当前元素
                lines.removeAt(i);
                --i;  // 因为移除了一个元素，所以减小索引
                removed = true;
            }

            // 检查是否属于界符
            if (!removed && dfa[2].Sum.contains(line)) {
                token.type = "Boundary";
                token.id = pos;
                token.content = line;
                result.push_back(token);

                // 删除当前元素
                lines.removeAt(i);
                --i;  // 因为移除了一个元素，所以减小索引
            }
        }

        for(QString& strlines:lines)
        {
            bool removeOp=false;
            int length=strlines.size();
            for(int i=0;i<strlines.size();i++)
            {
                if(dfa[2].Sum.contains(strlines[strlines.size()-1]))//判断最后一位是否为界符
                {
                    token.type = "Boundary";
                    token.id = pos;
                    token.content = strlines[strlines.size()-1];
                    result.push_back(token);
                    strlines.remove(strlines.size()-1,1);
                }
                if (isIdentify(strlines,dfa[1]))//判断是否为标识符
                {
                    token.type = "Identify";
                    token.id = pos;
                    token.content = strlines;
                    result.push_back(token);
                    strlines.remove(0,strlines.size());
                    break;
                }
                if(dfa[2].Sum.contains(strlines[i]))//判断界符
                {
                    token.type = "Boundary";
                    token.id = pos;
                    token.content = strlines[i];
                    result.push_back(token);
                    QStringList strlist=strlines.split(strlines[i],Qt::SkipEmptyParts);//运算符分割字符串
                    bool flag=false;
                    if(dfa[0].Sum.contains(strlist[0]))//判断是否为关键字
                    {
                        flag=true;
                        token.type = "Keyword";
                        token.id = pos;
                        token.content = strlist[0];
                        result.push_back(token);
                        strlines.remove(0,strlist[0].size()+1);
                        i=0;
                    }
                    if(!flag&&isIdentify(strlist[0],dfa[1]))
                    {
                        token.type = "Identify";
                        token.id = pos;
                        token.content = strlist[0];
                        result.push_back(token);
                    }
                    if(isIdentify(strlist[1],dfa[4]))
                    {
                        token.type = "Constant";
                        token.id = pos;
                        token.content = strlist[1];
                        result.push_back(token);
                    }

                }
                if(dfa[3].Sum.contains(strlines[i])&&(dfa[3].Sum.contains(strlines[i+1])&&((i+1)<strlines.size())))//判断两位运算符
                {
                    removeOp=true;
                    token.type = "Operator";
                    token.id = pos;
                    QString ch=strlines[i];
                    ch.append(strlines[i+1]);
                    token.content =ch;
                    result.push_back(token);
                    if(ch=="++"||ch=="--")//处理运算符是++和--的方法
                    {
                        strlines.remove(i,2);
                        if(isIdentify(strlines,dfa[1]))
                        {
                            token.type = "Identify";
                            token.id = pos;
                            token.content = strlines;
                            result.push_back(token);
                        }
                    }
                    else//处理其他
                    {
                        QStringList strlist=strlines.split(ch,Qt::SkipEmptyParts);//运算符分割字符串
                        for(const QString& str:strlist)
                        {

                            if(str[0]>='0'&&str[0]<='9')
                            {

                                if(isIdentify(str,dfa[4])){
                                    token.type = "Constant";
                                    token.id = pos;
                                    token.content = str;
                                    result.push_back(token);
                                }
                            }
                            else
                            {
                                if(isIdentify(str,dfa[1]))
                                {
                                    token.type = "Identify";
                                    token.id = pos;
                                    token.content = str;
                                    result.push_back(token);

                                }
                            }
                        }
                    }

                }

                if(!removeOp
                    &&dfa[3].Sum.contains(strlines[i])
                    &&(!dfa[3].Sum.contains(strlines[i+1])
                        &&(i+1<strlines.size()))
                    &&(i-1>=0&&(strlines[i-1]!='e'&&strlines[i-1]!='i')))//判断一位运算符
                {
                    token.type = "Operator";
                    token.id = pos;
                    token.content = strlines[i];
                    result.push_back(token);
                    QStringList strlist=strlines.split(strlines[i],Qt::SkipEmptyParts);

                    for(const QString& str:strlist)
                    {

                        bool flag=false;
                        QChar ch;
                        for(int i=0;i<str.size();i++)
                        {
                            if(dfa[3].Sum.contains(str[i])&&((str[i-1]!='e'&&str[i-1]!='i')&&i-1>=0))
                            {
                                flag=true;
                                ch=str[i];
                                token.type = "Operator";
                                token.id = pos;
                                token.content = ch;
                                result.push_back(token);
                                break;
                            }
                        }
                        if(flag==false)
                        {
                            if(str[0]>='0'&&str[0]<='9')
                            {

                                if(isIdentify(str,dfa[4])){
                                    token.type = "Constant";
                                    token.id = pos;
                                    token.content = str;
                                    result.push_back(token);

                                }
                            }
                            else
                            {
                                if(isIdentify(str,dfa[1]))
                                {
                                    token.type = "Identify";
                                    token.id = pos;
                                    token.content = str;
                                    result.push_back(token);

                                }
                            }
                        }
                        else
                        {

                            QStringList list=str.split(ch,Qt::SkipEmptyParts);

                            for(const QString& s:list)
                            {

                                if(s[0]>='0'&&s[0]<='9')
                                {

                                    if(isIdentify(s,dfa[4])){
                                        token.type = "Constant";
                                        token.id = pos;
                                        token.content = s;
                                        result.push_back(token);

                                    }
                                }
                                else
                                {
                                    if(isIdentify(s,dfa[1]))
                                    {
                                        token.type = "Identify";
                                        token.id = pos;
                                        token.content = s;
                                        result.push_back(token);
                                    }
                                }
                            }
                        }

                    }
                }

                if(dfa[0].Sum.contains(strlines))//判断是否为关键字
                {
                    token.type = "Keyword";
                    token.id = pos;
                    token.content = strlines;
                    result.push_back(token);
                    strlines.remove(0,strlines.size());
                }
                int count=0;
                for(const auto& out:result)
                {
                    if(out.id==pos)
                    {
                        count+=out.content.size();
                    }
                }
                count=count-tempnum;
                if(count==length)//判断字符串是否全部加入token表，防止重复加入
                {
                    break;
                }
            }
        }
        pos++;
    }
    for(int i=0;i<code.size();i++)
    {
        QVector<Token> temp;
        QVector<int> postion;
        for(const auto& toke:result)
        {
            if(toke.id==i+1)
            {
                postion.append(code[i].indexOf(toke.content));
                temp.append(toke);
            }
        }
        for(int j=0;j<postion.size();j++)
        {
            for(int k=0;k<postion.size();k++)
            {
                if(postion[j]<postion[k])
                {
                    int t=postion[j];
                    Token t1=temp[j];
                    postion[j]=postion[k];
                    temp[j]=temp[k];
                    postion[k]=t;
                    temp[k]=t1;
                }
            }
        }
        end_result.append(temp);
    }
    return end_result;
}

bool lexer::isIdentify(QString str, automation::DFA dfa)
{
    QString start=dfa.S[0];
    bool flag=false;
    int count=0;
    int len=dfa.f.size();
    for(int i=0;i<str.size();i++)
    {
        QChar ch=str[i];
        count=0;
        flag=false;
        for(const auto& function:dfa.f)
        {
            count++;
            if(function.middle_state==ch&&function.start_state==start)
            {
                flag=true;
                start=function.end_state;
                //qDebug()<<function.start_state<<","<<function.middle_state<<"->"<<function.end_state;
                break;
            }
        }
        if(count>=len&&flag==false)//判断是否合法
        {
            return false;
        }
    }
    if(dfa.Z.contains(start))
    {
        return true;
    }
    return false;
}

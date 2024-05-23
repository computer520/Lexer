#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "automation.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("文本文件 (*.txt)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "无法打开文件";
            return;
        }
        QTextStream in(&file);
        // 使用正则表达式匹配 // 和 // 之间的内容
        QRegularExpression regex("//(.*?)//", QRegularExpression::DotMatchesEverythingOption);
        QString incompleteMatch;

        while (!in.atEnd()) {
            // 逐行读取文件内容
            QString line = in.readLine();

            // 如果有未完成的匹配，先拼接在当前行上
            if (!incompleteMatch.isEmpty()) {
                line.prepend(incompleteMatch);
                incompleteMatch.clear();
            }

            // 使用正则表达式匹配当前行
            QRegularExpressionMatchIterator it = regex.globalMatch(line);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString extractedContent = match.captured(1);

                // 在提取的内容后添加换行符
                grammarContent.push_back(extractedContent + "\n");

                // 也可以将提取的内容添加到 QTextEdit 控件中

            }

            // 处理跨行匹配
            if (!line.endsWith("//")) {
                int lastMatchEndPos = line.lastIndexOf("//");
                if (lastMatchEndPos >= 0 && lastMatchEndPos != line.size() - 2) {
                    incompleteMatch = line.mid(lastMatchEndPos);
                    // 保持行之间的换行符
                    incompleteMatch += "\n";
                }
            }
        }

        file.close();

        // 在这里，您可以使用提取的内容进行进一步处理
        for (const QString& content : grammarContent) {
            // 在这里可以对提取的内容进行操作，例如显示在 QTextEdit 中
            ui->textEdit->append(content);
        }
    }
}


void MainWindow::on_pushButton_2_clicked() {
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("文本文件 (*.txt)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "无法打开文件";
            return;
        }

        QTextStream in(&file);
        QString content;

        // 分批读取文件内容，并追加到 QTextEdit 控件中显示
        const int batchSize = 50; // 每次读取的行数
        while (!in.atEnd()) {
            for (int i = 0; i < batchSize && !in.atEnd(); ++i) {
                content += in.readLine() + "\n";
            }
            ui->textEdit_2->append(content); // 将批量读取的内容追加到 QTextEdit 控件中显示
            sourceCodeContent+=content;
            content.clear(); // 清空内容，为下一次迭代做准备
        }

        file.close();
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    automation au=automation();
    QVector<QString> tips={"keyword_DFA\n","identify_DFA\n","boundary_DFA\n","operator_DFA\n","constant_DFA\n"};
    int i=0;
    for(const auto& content:grammarContent)
    {
        QVector<QString> expression=au.divideExpression(content);
        automation::NFA nfa=au.grammarToNFA(expression);
        automation::DFA Dfa=au.NFAToDFA(nfa);
        automation::DFA dfa=au.minDFA(Dfa);
        allDFA.push_back(dfa);
    }
    for(const auto& dfa:allDFA)
    {
        QString result;
        result += "K = {";
        for (const auto& state : dfa.K)
        {
            result += state + ", ";
        }
        if (!dfa.K.isEmpty()) result.chop(2); // 删除逗号
        result += "}\n";

        result += "Σ = {";
        for (const auto& symbol : dfa.Sum)
        {
            result += symbol + ", ";
        }
        if (!dfa.Sum.isEmpty()) result.chop(2);
        result += "}\n";

        result += "f = {\n";
        for (const auto& transition : dfa.f)
        {
            result += "  (" + transition.start_state + ", " + transition.middle_state + ") -> " + transition.end_state + "\n";
        }
        result += "}\n";

        result += "S = {";
        for (const auto& start : dfa.S)
        {
            result += start + ", ";
        }
        if (!dfa.S.isEmpty()) result.chop(2);
        result += "}\n";

        result += "Z = {";
        for (const auto& accept : dfa.Z)
        {
            result += accept + ", ";
        }
        if (!dfa.Z.isEmpty()) result.chop(2);
        result += "}\n";
        ui->textEdit_3->append(tips[i]+result);
        i++;
    }
}

void MainWindow::on_pushButton_4_clicked() {
    lexer Lexer=lexer();
    QVector<QString> codeContent=Lexer.divideSourceCode(sourceCodeContent);
    token_result=Lexer.lexerCode(codeContent,allDFA);
    QString output="";
    for(const auto& res:token_result)
    {
        QString temp="("+QString::number(res.id)+" , "+res.type+" , "+res.content+")";
        ui->textEdit_4->append(temp);
    }
}


void MainWindow::on_pushButton_5_clicked()//添加上下文无关文法
{
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("文本文件 (*.txt)"));

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "无法打开文件";
            return;
        }

        QTextStream in(&file);
        QString content;

        // 分批读取文件内容，并追加到 QTextEdit 控件中显示
        const int batchSize = 50; // 每次读取的行数
        while (!in.atEnd()) {
            for (int i = 0; i < batchSize && !in.atEnd(); ++i) {
                content += in.readLine() + "\n";
            }
            ui->textEdit_5->append(content); // 将批量读取的内容追加到 QTextEdit 控件中显示
            grammar2Content+=content;
            content.clear(); // 清空内容，为下一次迭代做准备
        }

        file.close();
    }
}


void MainWindow::on_pushButton_6_clicked()//任务2输出结果
{
    LR1_Grammar lr=LR1_Grammar();
    QVector<QString> express=lr.buildExpress(grammar2Content);//获得产生式.
    QVector<QString> legalContent;
    QString result=lr.isLegal(resultSet,token_result);
    ui->textEdit_6->append(result);
    // for(const auto& token:token_result)
    // {
    //     QString tokenResult=lr.isLegal(resultSet,token);
    //     legalContent.append(tokenResult);
    // }
    // bool flag=true;
    // for(int i=0;i<token_result.size();i++)
    // {
    //     QString temp="("+QString::number(token_result[i].id)+" , "+token_result[i].type+" , "+token_result[i].content+")";
    //     if(legalContent[i]=="no")
    //     {
    //         flag=false;
    //     }
    //     ui->textEdit_6->append(temp+"是否合法："+legalContent[i]);
    // }
    // if(flag)
    // {
    //     ui->textEdit_6->append("源代码字符串符合此2º型文法");
    // }
    // else
    // {
    //     ui->textEdit_6->append("源代码字符串不符合此2º型文法");
    // }

}


void MainWindow::on_pushButton_7_clicked()//生成Action-Goto表
{
    LR1_Grammar lr=LR1_Grammar();

    QVector<QString> express=lr.buildExpress(grammar2Content);//获得产生式.
    LR1_Grammar::Grammar2 grammar=lr.define2Grammar();

    QMap<QString, QSet<QString>> firstSets = lr.computeFirst(grammar);
    LR1_Grammar::LR1ItemSet startSet;
    startSet.append(LR1_Grammar::LR1Item("S'->~A","#"));
    resultSet=lr.bulidStates(startSet);

    for(const auto& actiongoto:resultSet)
    {
        QString result;
        result=actiongoto.type+"["+QString::number(actiongoto.k)+","+actiongoto.a+"]="+actiongoto.S_j;
        ui->textEdit_7->append(result);
    }
}


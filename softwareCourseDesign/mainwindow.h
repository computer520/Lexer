#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"automation.h"
#include <QFileDialog>
#include <QFile>
#include<QRegularExpression>
#include<QRegularExpressionMatch>
#include<QRegularExpressionMatchIterator>
#include <QTextStream>
#include <QVector>
#include "lexer.h"
#include"lr1_grammar.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

private:
    Ui::MainWindow *ui;
    automation::DFA DFa;
    QVector<QString> grammarContent;
    QString grammar2Content;
    QVector<automation::DFA> allDFA;
    QString sourceCodeContent;
    QVector<lexer::Token> token_result;
    QVector<LR1_Grammar::ACTION> resultSet;
};
#endif // MAINWINDOW_H

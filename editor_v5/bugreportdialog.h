#ifndef BUGREPORTDIALOG_H
#define BUGREPORTDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

class BugReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BugReportDialog(QWidget *parent = nullptr);

private slots:
    void submitReport();

private:
    QLineEdit *m_titleEdit;
    QComboBox *m_categoryCombo;
    QTextEdit *m_descriptionEdit;
};

#endif // BUGREPORTDIALOG_H

#ifndef CALCULATORDIALOG_H
#define CALCULATORDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QStack>

class CalculatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalculatorDialog(QWidget *parent = nullptr);

private slots:
    void digitPressed();
    void operatorPressed();
    void equalsPressed();
    void clearPressed();
    void clearEntryPressed();
    void backspacePressed();
    void decimalPressed();
    void signTogglePressed();
    void percentPressed();
    void unaryFunctionPressed();
    void memoryPressed();
    void degRadToggled();

private:
    QPushButton *createButton(const QString &text, const QString &style,
                              const char *slot = nullptr);
    void updateDisplay();
    double evaluateExpression();

    QLineEdit *m_expressionDisplay;
    QLineEdit *m_display;
    QLabel *m_memoryIndicator;

    double m_accumulator = 0.0;
    double m_pendingOperand = 0.0;
    double m_memory = 0.0;
    QString m_pendingOperator;
    bool m_waitingForOperand = true;
    bool m_useDegrees = true;
};

#endif // CALCULATORDIALOG_H

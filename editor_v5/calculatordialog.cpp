#include "calculatordialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QtMath>
#include <cmath>

CalculatorDialog::CalculatorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Rechner");
    setFixedSize(420, 520);
    setStyleSheet("QDialog { background-color: #1e2330; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Expression display (history line)
    m_expressionDisplay = new QLineEdit;
    m_expressionDisplay->setReadOnly(true);
    m_expressionDisplay->setAlignment(Qt::AlignRight);
    m_expressionDisplay->setStyleSheet(
        "QLineEdit { background: #282d3e; color: #8890a0; border: none;"
        "    font-size: 11pt; padding: 4px 8px; border-radius: 6px; }");
    mainLayout->addWidget(m_expressionDisplay);

    // Main display
    m_display = new QLineEdit("0");
    m_display->setReadOnly(true);
    m_display->setAlignment(Qt::AlignRight);
    m_display->setStyleSheet(
        "QLineEdit { background: #282d3e; color: #e8ecf2; border: none;"
        "    font-size: 22pt; font-weight: bold; padding: 6px 10px;"
        "    border-radius: 6px; }");
    mainLayout->addWidget(m_display);

    // Deg/Rad + Memory indicator
    auto *modeRow = new QHBoxLayout;
    auto *degBtn = new QRadioButton("Deg");
    auto *radBtn = new QRadioButton("Rad");
    degBtn->setChecked(true);
    degBtn->setStyleSheet("QRadioButton { color: #8890a0; font-size: 9pt; }");
    radBtn->setStyleSheet("QRadioButton { color: #8890a0; font-size: 9pt; }");
    auto *btnGroup = new QButtonGroup(this);
    btnGroup->addButton(degBtn, 0);
    btnGroup->addButton(radBtn, 1);
    connect(btnGroup, &QButtonGroup::idClicked, this, [this](int id) {
        m_useDegrees = (id == 0);
    });
    modeRow->addWidget(degBtn);
    modeRow->addWidget(radBtn);
    modeRow->addStretch();
    m_memoryIndicator = new QLabel;
    m_memoryIndicator->setStyleSheet("color: #5ba4cf; font-size: 9pt; font-weight: bold;");
    modeRow->addWidget(m_memoryIndicator);
    mainLayout->addLayout(modeRow);

    // Button styles
    QString numStyle =
        "QPushButton { background: #2d3348; color: #e0e4ec; border: none;"
        "    border-radius: 6px; font-size: 14pt; font-weight: bold; }"
        "QPushButton:hover { background: #3a4160; }"
        "QPushButton:pressed { background: #4a5270; }";

    QString opStyle =
        "QPushButton { background: #3a4a6a; color: #7ab5f0; border: none;"
        "    border-radius: 6px; font-size: 13pt; font-weight: bold; }"
        "QPushButton:hover { background: #4a5a7a; }"
        "QPushButton:pressed { background: #5a6a8a; }";

    QString funcStyle =
        "QPushButton { background: #252a3a; color: #8890a0; border: none;"
        "    border-radius: 6px; font-size: 11pt; }"
        "QPushButton:hover { background: #303548; }"
        "QPushButton:pressed { background: #404560; }";

    QString eqStyle =
        "QPushButton { background: #4a90d0; color: #ffffff; border: none;"
        "    border-radius: 6px; font-size: 16pt; font-weight: bold; }"
        "QPushButton:hover { background: #5aa0e0; }"
        "QPushButton:pressed { background: #3a80c0; }";

    QString redStyle =
        "QPushButton { background: #5a2a30; color: #f08080; border: none;"
        "    border-radius: 6px; font-size: 12pt; font-weight: bold; }"
        "QPushButton:hover { background: #6a3a40; }"
        "QPushButton:pressed { background: #7a4a50; }";

    QString memStyle =
        "QPushButton { background: #252a3a; color: #5ba4cf; border: none;"
        "    border-radius: 6px; font-size: 10pt; font-weight: bold; }"
        "QPushButton:hover { background: #303548; }";

    // Button grid
    auto *grid = new QGridLayout;
    grid->setSpacing(3);
    mainLayout->addLayout(grid);

    // Row 0: Memory buttons
    auto *mc = createButton("MC", memStyle); grid->addWidget(mc, 0, 0);
    auto *mr = createButton("MR", memStyle); grid->addWidget(mr, 0, 1);
    auto *ms = createButton("MS", memStyle); grid->addWidget(ms, 0, 2);
    auto *mPlus = createButton("M+", memStyle); grid->addWidget(mPlus, 0, 3);
    auto *mMinus = createButton("M-", memStyle); grid->addWidget(mMinus, 0, 4);

    connect(mc, &QPushButton::clicked, this, &CalculatorDialog::memoryPressed);
    connect(mr, &QPushButton::clicked, this, &CalculatorDialog::memoryPressed);
    connect(ms, &QPushButton::clicked, this, &CalculatorDialog::memoryPressed);
    connect(mPlus, &QPushButton::clicked, this, &CalculatorDialog::memoryPressed);
    connect(mMinus, &QPushButton::clicked, this, &CalculatorDialog::memoryPressed);

    // Row 1: Scientific functions
    auto *sinBtn = createButton("sin", funcStyle); grid->addWidget(sinBtn, 1, 0);
    auto *cosBtn = createButton("cos", funcStyle); grid->addWidget(cosBtn, 1, 1);
    auto *tanBtn = createButton("tan", funcStyle); grid->addWidget(tanBtn, 1, 2);
    auto *lnBtn  = createButton("ln", funcStyle);  grid->addWidget(lnBtn, 1, 3);
    auto *logBtn = createButton("log", funcStyle);  grid->addWidget(logBtn, 1, 4);

    for (auto *b : {sinBtn, cosBtn, tanBtn, lnBtn, logBtn})
        connect(b, &QPushButton::clicked, this, &CalculatorDialog::unaryFunctionPressed);

    // Row 2: More functions
    auto *sqrtBtn = createButton(QString::fromUtf8("√"), funcStyle); grid->addWidget(sqrtBtn, 2, 0);
    auto *x2Btn   = createButton(QString::fromUtf8("x²"), funcStyle); grid->addWidget(x2Btn, 2, 1);
    auto *piBtn   = createButton(QString::fromUtf8("π"), funcStyle); grid->addWidget(piBtn, 2, 2);
    auto *expBtn  = createButton(QString::fromUtf8("eˣ"), funcStyle); grid->addWidget(expBtn, 2, 3);
    auto *invBtn  = createButton("1/x", funcStyle); grid->addWidget(invBtn, 2, 4);

    for (auto *b : {sqrtBtn, x2Btn, piBtn, expBtn, invBtn})
        connect(b, &QPushButton::clicked, this, &CalculatorDialog::unaryFunctionPressed);

    // Row 3: CE, C, Backspace, /, %
    auto *ceBtn = createButton("CE", redStyle); grid->addWidget(ceBtn, 3, 0);
    connect(ceBtn, &QPushButton::clicked, this, &CalculatorDialog::clearEntryPressed);
    auto *cBtn = createButton("C", redStyle); grid->addWidget(cBtn, 3, 1);
    connect(cBtn, &QPushButton::clicked, this, &CalculatorDialog::clearPressed);
    auto *bsBtn = createButton(QString::fromUtf8("⌫"), redStyle); grid->addWidget(bsBtn, 3, 2);
    connect(bsBtn, &QPushButton::clicked, this, &CalculatorDialog::backspacePressed);
    auto *divBtn = createButton(QString::fromUtf8("÷"), opStyle); grid->addWidget(divBtn, 3, 3);
    divBtn->setProperty("op", "/");
    connect(divBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);
    auto *pctBtn = createButton("%", funcStyle); grid->addWidget(pctBtn, 3, 4);
    connect(pctBtn, &QPushButton::clicked, this, &CalculatorDialog::percentPressed);

    // Rows 4-6: Number pad + operators
    QString digits[10] = {"7","8","9","4","5","6","1","2","3","0"};
    int digitRow[] = {4,4,4,5,5,5,6,6,6,7};
    int digitCol[] = {0,1,2,0,1,2,0,1,2,0};

    for (int i = 0; i < 10; ++i) {
        auto *btn = createButton(digits[i], numStyle);
        grid->addWidget(btn, digitRow[i], digitCol[i],
                         (i == 9) ? 1 : 1, (i == 9) ? 2 : 1);
        connect(btn, &QPushButton::clicked, this, &CalculatorDialog::digitPressed);
    }

    auto *mulBtn = createButton(QString::fromUtf8("×"), opStyle); grid->addWidget(mulBtn, 4, 3);
    mulBtn->setProperty("op", "*");
    connect(mulBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);

    auto *modBtn = createButton("mod", funcStyle); grid->addWidget(modBtn, 4, 4);
    modBtn->setProperty("op", "mod");
    connect(modBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);

    auto *subBtn = createButton(QString::fromUtf8("−"), opStyle); grid->addWidget(subBtn, 5, 3);
    subBtn->setProperty("op", "-");
    connect(subBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);

    auto *powBtn = createButton(QString::fromUtf8("xⁿ"), funcStyle); grid->addWidget(powBtn, 5, 4);
    powBtn->setProperty("op", "^");
    connect(powBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);

    auto *addBtn = createButton("+", opStyle); grid->addWidget(addBtn, 6, 3);
    addBtn->setProperty("op", "+");
    connect(addBtn, &QPushButton::clicked, this, &CalculatorDialog::operatorPressed);

    auto *signBtn = createButton(QString::fromUtf8("±"), funcStyle); grid->addWidget(signBtn, 6, 4);
    connect(signBtn, &QPushButton::clicked, this, &CalculatorDialog::signTogglePressed);

    // Row 7: 0 (colspan 2), decimal, =, noop
    auto *decBtn = createButton(",", numStyle); grid->addWidget(decBtn, 7, 2);
    connect(decBtn, &QPushButton::clicked, this, &CalculatorDialog::decimalPressed);

    auto *eqBtn = createButton("=", eqStyle); grid->addWidget(eqBtn, 7, 3, 1, 2);
    connect(eqBtn, &QPushButton::clicked, this, &CalculatorDialog::equalsPressed);

    // Set minimum button sizes
    for (int r = 0; r < grid->rowCount(); ++r)
        for (int c = 0; c < grid->columnCount(); ++c)
            if (auto *item = grid->itemAtPosition(r, c))
                if (auto *w = item->widget())
                    w->setMinimumHeight(44);
}

QPushButton *CalculatorDialog::createButton(const QString &text, const QString &style,
                                             const char *)
{
    auto *btn = new QPushButton(text);
    btn->setStyleSheet(style);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void CalculatorDialog::digitPressed()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString digit = btn->text();

    if (m_display->text() == "0" || m_waitingForOperand) {
        m_display->setText(digit);
        m_waitingForOperand = false;
    } else {
        m_display->setText(m_display->text() + digit);
    }
}

void CalculatorDialog::operatorPressed()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString op = btn->property("op").toString();

    double operand = m_display->text().replace(",", ".").toDouble();

    if (!m_pendingOperator.isEmpty() && !m_waitingForOperand) {
        m_pendingOperand = evaluateExpression();
        m_display->setText(QString::number(m_pendingOperand, 'g', 12).replace(".", ","));
    } else {
        m_pendingOperand = operand;
    }

    m_pendingOperator = op;
    m_waitingForOperand = true;

    // Show expression
    QString displayOp = op;
    if (op == "*") displayOp = QString::fromUtf8("×");
    else if (op == "/") displayOp = QString::fromUtf8("÷");
    else if (op == "-") displayOp = QString::fromUtf8("−");
    else if (op == "^") displayOp = QString::fromUtf8("^");

    m_expressionDisplay->setText(
        QString::number(m_pendingOperand, 'g', 12).replace(".", ",") + " " + displayOp);
}

double CalculatorDialog::evaluateExpression()
{
    double operand = m_display->text().replace(",", ".").toDouble();

    if (m_pendingOperator == "+") return m_pendingOperand + operand;
    if (m_pendingOperator == "-") return m_pendingOperand - operand;
    if (m_pendingOperator == "*") return m_pendingOperand * operand;
    if (m_pendingOperator == "/") return (operand != 0.0) ? m_pendingOperand / operand : 0.0;
    if (m_pendingOperator == "mod") return std::fmod(m_pendingOperand, operand);
    if (m_pendingOperator == "^") return std::pow(m_pendingOperand, operand);

    return operand;
}

void CalculatorDialog::equalsPressed()
{
    if (!m_pendingOperator.isEmpty() && !m_waitingForOperand) {
        double result = evaluateExpression();
        m_display->setText(QString::number(result, 'g', 12).replace(".", ","));
        m_expressionDisplay->clear();
        m_pendingOperator.clear();
        m_pendingOperand = 0.0;
        m_waitingForOperand = true;
    }
}

void CalculatorDialog::clearPressed()
{
    m_display->setText("0");
    m_expressionDisplay->clear();
    m_accumulator = 0.0;
    m_pendingOperand = 0.0;
    m_pendingOperator.clear();
    m_waitingForOperand = true;
}

void CalculatorDialog::clearEntryPressed()
{
    m_display->setText("0");
    m_waitingForOperand = true;
}

void CalculatorDialog::backspacePressed()
{
    QString text = m_display->text();
    if (text.length() > 1)
        m_display->setText(text.left(text.length() - 1));
    else
        m_display->setText("0");
}

void CalculatorDialog::decimalPressed()
{
    if (m_waitingForOperand) {
        m_display->setText("0,");
        m_waitingForOperand = false;
    } else if (!m_display->text().contains(",")) {
        m_display->setText(m_display->text() + ",");
    }
}

void CalculatorDialog::signTogglePressed()
{
    double val = m_display->text().replace(",", ".").toDouble();
    val = -val;
    m_display->setText(QString::number(val, 'g', 12).replace(".", ","));
}

void CalculatorDialog::percentPressed()
{
    double val = m_display->text().replace(",", ".").toDouble();
    val = val / 100.0;
    m_display->setText(QString::number(val, 'g', 12).replace(".", ","));
}

void CalculatorDialog::unaryFunctionPressed()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString func = btn->text();
    double val = m_display->text().replace(",", ".").toDouble();
    double result = 0.0;

    double angle = m_useDegrees ? qDegreesToRadians(val) : val;

    if (func == "sin") result = std::sin(angle);
    else if (func == "cos") result = std::cos(angle);
    else if (func == "tan") result = std::tan(angle);
    else if (func == "ln")  result = std::log(val);
    else if (func == "log") result = std::log10(val);
    else if (func == QString::fromUtf8("√")) result = std::sqrt(val);
    else if (func == QString::fromUtf8("x²")) result = val * val;
    else if (func == QString::fromUtf8("π")) { result = M_PI; }
    else if (func == QString::fromUtf8("eˣ")) result = std::exp(val);
    else if (func == "1/x") result = (val != 0.0) ? 1.0 / val : 0.0;

    m_display->setText(QString::number(result, 'g', 12).replace(".", ","));
    m_waitingForOperand = true;
}

void CalculatorDialog::memoryPressed()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString cmd = btn->text();
    double val = m_display->text().replace(",", ".").toDouble();

    if (cmd == "MC") { m_memory = 0.0; m_memoryIndicator->clear(); }
    else if (cmd == "MR") {
        m_display->setText(QString::number(m_memory, 'g', 12).replace(".", ","));
        m_waitingForOperand = true;
    }
    else if (cmd == "MS") { m_memory = val; m_memoryIndicator->setText("M"); }
    else if (cmd == "M+") { m_memory += val; m_memoryIndicator->setText("M"); }
    else if (cmd == "M-") { m_memory -= val; m_memoryIndicator->setText("M"); }
}

void CalculatorDialog::degRadToggled()
{
    // handled inline via lambda
}

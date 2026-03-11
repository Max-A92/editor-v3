#include "mainwindow.h"
#include "specialchardialog.h"
#include "tabledialog.h"
#include "calculatordialog.h"
#include "bugreportdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QStyle>
#include <QColorDialog>
#include <QPushButton>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QHeaderView>
#include <QTableWidget>
#include <algorithm>
#include <utility>

// JSON Export
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>

// QtCharts
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

// ─── Eigener Icon-Stil: rund/pill-förmig, andere Farben als Q-Examiner ─────
QIcon MainWindow::makeIcon(const QString &text, int size,
                           const QColor &fg, bool bold, bool circle)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    if (circle) {
        p.setBrush(QColor("#e8ecf4"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(1, 1, size - 2, size - 2);
    }

    QFont f("Segoe UI", size * 0.44);
    if (bold) f.setBold(true);
    p.setFont(f);
    p.setPen(fg);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, text);
    return QIcon(pix);
}

// ─── Ausrichtungs-Icon (eigener Stil: abgerundete Ecken) ────────────────────
static QIcon makeAlignIcon(const QString &type, int size = 26)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#3a3a60"), 1.8, Qt::SolidLine, Qt::RoundCap));

    int y = 5, step = 5, margin = 4;
    int fullW = size - 2 * margin;

    for (int i = 0; i < 4; ++i) {
        int lineW, x = margin;
        if (type == "left") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
        } else if (type == "center") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
            x = margin + (fullW - lineW) / 2;
        } else if (type == "right") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
            x = margin + fullW - lineW;
        } else {
            lineW = fullW;
        }
        p.drawLine(x, y, x + lineW, y);
        y += step;
    }
    return QIcon(pix);
}

// ─── Farbiges Marker-Icon ───────────────────────────────────────────────────
static QIcon makeHighlightIcon(const QColor &color, int size = 26)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    // Stift-Form
    p.setPen(QPen(QColor("#3a3a60"), 1.5));
    p.drawLine(size/2, 3, size/2, size - 9);
    p.drawLine(size/2 - 4, size - 9, size/2 + 4, size - 9);
    // Farbleiste
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(3, size - 7, size - 6, 5, 2, 2);
    return QIcon(pix);
}

// ═════════════════════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentHighlightColor(Qt::yellow)
    , remainingSeconds(7 * 60 * 60 - 60)
{
    // Datenbank initialisieren
    dbManager = new DatabaseManager(this);
    dbManager->initialize();
    m_sessionStart = QDateTime::currentDateTime();

    setupHeaderBar();
    setupToolBar();
    setupEditor();

    examTimer = new QTimer(this);
    connect(examTimer, &QTimer::timeout, this, &MainWindow::updateTimer);
    examTimer->start(1000);
    updateTimer();

    resize(1100, 750);
}

MainWindow::~MainWindow() = default;

// ─── Header-Leiste (komplett anders als Q-Examiner) ─────────────────────────
void MainWindow::setupHeaderBar()
{
    auto *headerWidget = new QFrame;
    headerWidget->setFixedHeight(52);
    headerWidget->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #2c3e6b, stop:1 #1a2744);"
        "}"
        "QLabel { color: #d0d8e8; }"
    );

    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    // App-Icon (Buchsymbol statt Dokument)
    auto *appIcon = new QLabel;
    QPixmap iconPix(32, 32);
    iconPix.fill(Qt::transparent);
    {
        QPainter p(&iconPix);
        p.setRenderHint(QPainter::Antialiasing);
        // Offenes Buch
        p.setPen(QPen(QColor("#7aa0d0"), 2));
        p.drawArc(4, 6, 12, 20, 30*16, 120*16);
        p.drawArc(16, 6, 12, 20, 30*16, 120*16);
        p.drawLine(16, 4, 16, 28);
        // Tastatur-Symbol
        p.setPen(QPen(QColor("#90b0d8"), 1.5));
        for (int i = 0; i < 3; ++i)
            p.drawLine(7 + i*8, 14, 10 + i*8, 14);
    }
    appIcon->setPixmap(iconPix);
    headerLayout->addWidget(appIcon);
    headerLayout->addSpacing(8);

    // Titel
    auto *lblTitle = new QLabel("Tipptrainer zur Prüfungsvorbereitung");
    lblTitle->setFont(QFont("Segoe UI", 14, QFont::Bold));
    lblTitle->setStyleSheet("color: #e4ecf6;");
    headerLayout->addWidget(lblTitle);

    headerLayout->addStretch();

    // ── Werkzeuge-Menü ──
    auto *btnTools = new QToolButton;
    btnTools->setText("Werkzeuge");
    btnTools->setFont(QFont("Segoe UI", 9));
    btnTools->setPopupMode(QToolButton::InstantPopup);
    btnTools->setStyleSheet(
        "QToolButton { color: #b0c0da; background: transparent;"
        "    border: 1px solid #3a5080; border-radius: 4px; padding: 5px 10px; }"
        "QToolButton:hover { background: #354a70; border-color: #5a70a0; }"
        "QToolButton::menu-indicator { image: none; width: 0; }"
    );
    auto *toolsMenu = new QMenu(btnTools);
    toolsMenu->setStyleSheet(
        "QMenu { background: #fff; color: #000; border: 1px solid #ccc; }"
        "QMenu::item { padding: 6px 20px; }"
        "QMenu::item:selected { background: #e0e8f4; }"
    );
    toolsMenu->addAction("Rechner", this, &MainWindow::showCalculator);
    toolsMenu->addAction("Bug melden / Feedback", this, &MainWindow::showBugReport);
    btnTools->setMenu(toolsMenu);
    headerLayout->addWidget(btnTools);
    headerLayout->addSpacing(8);

    // ── Auswertung-Menü ──
    auto *btnAuswertung = new QToolButton;
    btnAuswertung->setText("Auswertung");
    btnAuswertung->setFont(QFont("Segoe UI", 9, QFont::Bold));
    btnAuswertung->setPopupMode(QToolButton::InstantPopup);
    btnAuswertung->setStyleSheet(
        "QToolButton { color: #d0daea; background: #3a5580;"
        "    border: 1px solid #5a7aaa; border-radius: 4px; padding: 5px 12px; }"
        "QToolButton:hover { background: #4a6590; border-color: #7a9ac0; }"
        "QToolButton::menu-indicator { image: none; width: 0; }"
    );

    auto *auswertungMenu = new QMenu(btnAuswertung);
    auswertungMenu->setStyleSheet(toolsMenu->styleSheet());

    auswertungMenu->addAction("Performance Dashboard", this, &MainWindow::showPerformanceDashboard);
    auswertungMenu->addAction("Detaillierte Statistiken", this, &MainWindow::showStatistics);
    auswertungMenu->addAction("Fehler-Kreisdiagramm", this, &MainWindow::showErrorPieChart);
    auswertungMenu->addAction("Tastatur-Heatmap", this, &MainWindow::showKeyboardHeatmap);
    auswertungMenu->addAction("WPM-Verlauf", this, &MainWindow::showWPMChart);
    auswertungMenu->addSeparator();
    auswertungMenu->addAction("Session speichern", this, &MainWindow::saveCurrentSession);
    auswertungMenu->addAction("Session-Verlauf", this, &MainWindow::showSessionHistory);
    auswertungMenu->addAction("Trend-Analyse", this, &MainWindow::showTrendAnalysis);
    auswertungMenu->addSeparator();
    auswertungMenu->addAction("Daten exportieren (anonym)", this, &MainWindow::exportAnonymizedData);
    auswertungMenu->addSeparator();
    auswertungMenu->addAction("Tracking zurücksetzen", this, &MainWindow::resetTracking);

    btnAuswertung->setMenu(auswertungMenu);
    headerLayout->addWidget(btnAuswertung);
    headerLayout->addSpacing(12);

    // Keystroke-Zähler
    lblKeystrokeCount = new QLabel("Tasten: 0");
    lblKeystrokeCount->setFont(QFont("Segoe UI", 9));
    lblKeystrokeCount->setStyleSheet("color: #8898b8;");
    headerLayout->addWidget(lblKeystrokeCount);
    headerLayout->addSpacing(12);

    // Timer
    lblTimer = new QLabel("06:59:00");
    lblTimer->setFont(QFont("Consolas", 13, QFont::Bold));
    lblTimer->setStyleSheet("color: #c8d4e8;");
    headerLayout->addWidget(lblTimer);
    headerLayout->addSpacing(12);

    // Beenden-Button (runde Form statt Q-Examiners rechteckig)
    auto *btnEnd = new QPushButton("Beenden");
    btnEnd->setFont(QFont("Segoe UI", 9, QFont::Bold));
    btnEnd->setStyleSheet(
        "QPushButton { background: #e8ecf4; color: #2c3e6b;"
        "    border: none; border-radius: 14px; padding: 6px 18px; }"
        "QPushButton:hover { background: #ffffff; }"
    );
    connect(btnEnd, &QPushButton::clicked, this, [this]() { close(); });
    headerLayout->addWidget(btnEnd);

    setMenuWidget(headerWidget);
}

// ─── Toolbar (visuell komplett anders als Q-Examiner) ───────────────────────
void MainWindow::setupToolBar()
{
    auto *toolbar = new QToolBar("Formatierung");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(26, 26));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setStyleSheet(
        "QToolBar {"
        "    background: #f0f2f8;"
        "    border-bottom: 1px solid #c8ccd8;"
        "    padding: 3px 6px;"
        "    spacing: 2px;"
        "}"
        "QToolButton {"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 4px;"
        "    margin: 1px;"
        "}"
        "QToolButton:hover { background: #d8dce8; }"
        "QToolButton:checked { background: #c0c8dc; }"
        "QToolBar::separator {"
        "    width: 1px; margin: 2px 6px;"
        "    background: #c0c4d0;"
        "}"
    );
    addToolBar(Qt::TopToolBarArea, toolbar);

    // ── Verlauf ──
    actUndo = toolbar->addAction(makeIcon(QString::fromUtf8("↩"), 26, QColor("#4a5070")), "");
    actUndo->setToolTip("Rückgängig (Strg+Z)");
    actUndo->setShortcut(QKeySequence::Undo);

    actRedo = toolbar->addAction(makeIcon(QString::fromUtf8("↪"), 26, QColor("#4a5070")), "");
    actRedo->setToolTip("Wiederholen (Strg+Y)");
    actRedo->setShortcut(QKeySequence::Redo);

    toolbar->addSeparator();

    // ── Zwischenablage ──
    actCut = toolbar->addAction(makeIcon(QString::fromUtf8("✂"), 26, QColor("#6a4060")), "");
    actCut->setToolTip("Ausschneiden (Strg+X)");
    actCut->setShortcut(QKeySequence::Cut);

    actCopy = toolbar->addAction(makeIcon(QString::fromUtf8("⊡"), 26, QColor("#4a5070")), "");
    actCopy->setToolTip("Kopieren (Strg+C)");
    actCopy->setShortcut(QKeySequence::Copy);

    actPaste = toolbar->addAction(makeIcon(QString::fromUtf8("⊞"), 26, QColor("#4a5070")), "");
    actPaste->setToolTip("Einfügen (Strg+V)");
    actPaste->setShortcut(QKeySequence::Paste);

    toolbar->addSeparator();

    // ── Einfügen ──
    actPageBreak = toolbar->addAction(makeIcon(QString::fromUtf8("⤓"), 26, QColor("#4a5070")), "");
    actPageBreak->setToolTip("Seitenumbruch");

    actHighlight = toolbar->addAction(makeHighlightIcon(currentHighlightColor), "");
    actHighlight->setToolTip("Hintergrundfarbe");

    toolbar->addSeparator();

    // ── Textformat (andere Icons als Q-Examiner: mit Kreishintergrund) ──
    actBold = toolbar->addAction(makeIcon("B", 26, QColor("#2a3050"), true, true), "");
    actBold->setCheckable(true);
    actBold->setToolTip("Fett (Strg+B)");
    actBold->setShortcut(QKeySequence::Bold);

    {
        QPixmap pix(26, 26);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#e8ecf4"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(1, 1, 24, 24);
        QFont f("Segoe UI", 13);
        f.setItalic(true);
        p.setFont(f);
        p.setPen(QColor("#2a3050"));
        p.drawText(QRect(0, 0, 26, 26), Qt::AlignCenter, "I");
        actItalic = toolbar->addAction(QIcon(pix), "");
    }
    actItalic->setCheckable(true);
    actItalic->setToolTip("Kursiv (Strg+I)");
    actItalic->setShortcut(QKeySequence::Italic);

    {
        QPixmap pix(26, 26);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor("#e8ecf4"));
        p.setPen(Qt::NoPen);
        p.drawEllipse(1, 1, 24, 24);
        QFont f("Segoe UI", 12);
        p.setFont(f);
        p.setPen(QColor("#2a3050"));
        p.drawText(QRect(0, -1, 26, 24), Qt::AlignCenter, "U");
        p.setPen(QPen(QColor("#2a3050"), 1.5));
        p.drawLine(8, 20, 18, 20);
        actUnderline = toolbar->addAction(QIcon(pix), "");
    }
    actUnderline->setCheckable(true);
    actUnderline->setToolTip("Unterstrichen (Strg+U)");
    actUnderline->setShortcut(QKeySequence::Underline);

    actSuperscript = toolbar->addAction(makeIcon(QString::fromUtf8("X²"), 26, QColor("#2a3050"), false, true), "");
    actSuperscript->setCheckable(true);
    actSuperscript->setToolTip("Hochgestellt");

    toolbar->addSeparator();

    // ── Ausrichtung ──
    actAlignLeft   = toolbar->addAction(makeAlignIcon("left"), "");
    actAlignLeft->setCheckable(true);  actAlignLeft->setToolTip("Linksbündig");
    actAlignCenter = toolbar->addAction(makeAlignIcon("center"), "");
    actAlignCenter->setCheckable(true); actAlignCenter->setToolTip("Zentriert");
    actAlignRight  = toolbar->addAction(makeAlignIcon("right"), "");
    actAlignRight->setCheckable(true);  actAlignRight->setToolTip("Rechtsbündig");
    actJustify     = toolbar->addAction(makeAlignIcon("justify"), "");
    actJustify->setCheckable(true);     actJustify->setToolTip("Blocksatz");

    toolbar->addSeparator();

    // ── Extras ──
    actIndent = toolbar->addAction(makeIcon(QString::fromUtf8("⇥"), 26, QColor("#4a5070")), "");
    actIndent->setToolTip("Einzug vergrößern");

    actNbsp = toolbar->addAction(makeIcon(QString::fromUtf8("⍽"), 26, QColor("#4a5070")), "");
    actNbsp->setToolTip("Geschütztes Leerzeichen");

    actSpecialChar = toolbar->addAction(makeIcon(QString::fromUtf8("Ω"), 26, QColor("#5a4080")), "");
    actSpecialChar->setToolTip("Sonderzeichen");

    toolbar->addSeparator();

    // ── Tabelle ──
    btnTable = new QToolButton;
    btnTable->setIcon(makeIcon(QString::fromUtf8("⊞"), 26, QColor("#4a5070")));
    btnTable->setToolTip("Tabelle");
    btnTable->setPopupMode(QToolButton::InstantPopup);
    btnTable->setStyleSheet("QToolButton::menu-indicator { image: none; width: 0; }");
    auto *tableMenu = new QMenu(btnTable);
    QAction *actInsertTable = tableMenu->addAction("Tabelle einfügen...");
    connect(actInsertTable, &QAction::triggered, this, &MainWindow::insertTable);
    tableMenu->addSeparator();
    tableMenu->addAction("Zeile hinzufügen")->setEnabled(false);
    tableMenu->addAction("Spalte hinzufügen")->setEnabled(false);
    btnTable->setMenu(tableMenu);
    toolbar->addWidget(btnTable);

    toolbar->addSeparator();

    // ── Vorschau ──
    actPdfPreview = toolbar->addAction(makeIcon(QString::fromUtf8("⎙"), 26, QColor("#4a5070")), "");
    actPdfPreview->setToolTip("PDF-Gesamtvorschau");
}

// ─── Editor ─────────────────────────────────────────────────────────────────
void MainWindow::setupEditor()
{
    editor = new RichTextEditor(this);
    setCentralWidget(editor);

    // Signale verbinden
    connect(actUndo,   &QAction::triggered, editor, &QTextEdit::undo);
    connect(actRedo,   &QAction::triggered, editor, &QTextEdit::redo);
    connect(actCut,    &QAction::triggered, editor, &QTextEdit::cut);
    connect(actCopy,   &QAction::triggered, editor, &QTextEdit::copy);
    connect(actPaste,  &QAction::triggered, editor, &QTextEdit::paste);
    connect(actPageBreak,   &QAction::triggered, this, &MainWindow::insertPageBreak);
    connect(actHighlight,   &QAction::triggered, this, &MainWindow::setHighlightColor);
    connect(actBold,        &QAction::triggered, this, &MainWindow::toggleBold);
    connect(actItalic,      &QAction::triggered, this, &MainWindow::toggleItalic);
    connect(actUnderline,   &QAction::triggered, this, &MainWindow::toggleUnderline);
    connect(actSuperscript, &QAction::triggered, this, &MainWindow::toggleSuperscript);
    connect(actAlignLeft,   &QAction::triggered, this, &MainWindow::alignLeft);
    connect(actAlignCenter, &QAction::triggered, this, &MainWindow::alignCenter);
    connect(actAlignRight,  &QAction::triggered, this, &MainWindow::alignRight);
    connect(actJustify,     &QAction::triggered, this, &MainWindow::alignJustify);
    connect(actIndent,      &QAction::triggered, this, &MainWindow::increaseIndent);
    connect(actNbsp,        &QAction::triggered, this, &MainWindow::insertNonBreakingSpace);
    connect(actSpecialChar, &QAction::triggered, this, &MainWindow::insertSpecialChar);
    connect(actPdfPreview,  &QAction::triggered, this, &MainWindow::showPdfPreview);

    connect(editor, &QTextEdit::undoAvailable, actUndo, &QAction::setEnabled);
    connect(editor, &QTextEdit::redoAvailable, actRedo, &QAction::setEnabled);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);

    connect(editor, &QTextEdit::copyAvailable, actCut,  &QAction::setEnabled);
    connect(editor, &QTextEdit::copyAvailable, actCopy, &QAction::setEnabled);
    actCut->setEnabled(false);
    actCopy->setEnabled(false);

    connect(editor, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::onCursorPositionChanged);
    connect(editor, &QTextEdit::currentCharFormatChanged,
            this, &MainWindow::onCurrentCharFormatChanged);

    editor->setFocus();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Formatting
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::toggleBold() {
    QTextCharFormat f; f.setFontWeight(actBold->isChecked() ? QFont::Bold : QFont::Normal);
    editor->mergeCurrentCharFormat(f);
}
void MainWindow::toggleItalic() {
    QTextCharFormat f; f.setFontItalic(actItalic->isChecked());
    editor->mergeCurrentCharFormat(f);
}
void MainWindow::toggleUnderline() {
    QTextCharFormat f; f.setFontUnderline(actUnderline->isChecked());
    editor->mergeCurrentCharFormat(f);
}
void MainWindow::toggleSuperscript() {
    QTextCharFormat f;
    f.setVerticalAlignment(actSuperscript->isChecked()
        ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    editor->mergeCurrentCharFormat(f);
}

void MainWindow::alignLeft()   { editor->setAlignment(Qt::AlignLeft);    updateAlignmentActions(); }
void MainWindow::alignCenter() { editor->setAlignment(Qt::AlignHCenter); updateAlignmentActions(); }
void MainWindow::alignRight()  { editor->setAlignment(Qt::AlignRight);   updateAlignmentActions(); }
void MainWindow::alignJustify(){ editor->setAlignment(Qt::AlignJustify); updateAlignmentActions(); }

void MainWindow::updateAlignmentActions() {
    Qt::Alignment a = editor->alignment();
    actAlignLeft->setChecked(a & Qt::AlignLeft);
    actAlignCenter->setChecked(a & Qt::AlignHCenter);
    actAlignRight->setChecked(a & Qt::AlignRight);
    actJustify->setChecked(a & Qt::AlignJustify);
}

void MainWindow::increaseIndent() {
    QTextCursor c = editor->textCursor();
    QTextBlockFormat bf = c.blockFormat();
    bf.setIndent(bf.indent() + 1);
    c.setBlockFormat(bf);
}

void MainWindow::insertPageBreak() {
    QTextCursor c = editor->textCursor();
    c.insertText("\n");
    QTextBlockFormat bf;
    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    c.insertBlock(bf);
}

void MainWindow::insertNonBreakingSpace() {
    editor->textCursor().insertText(QString(QChar(0x00A0)));
}

void MainWindow::insertSpecialChar() {
    SpecialCharDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted && !dlg.selectedChar().isEmpty())
        editor->textCursor().insertText(dlg.selectedChar());
}

void MainWindow::insertTable() {
    TableDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QTextCursor c = editor->textCursor();
        QTextTableFormat tf;
        tf.setBorder(1);
        tf.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
        tf.setCellPadding(6);
        tf.setCellSpacing(0);
        tf.setBorderBrush(QColor("#999999"));
        c.insertTable(dlg.rows(), dlg.columns(), tf);
    }
}

void MainWindow::setHighlightColor() {
    QColor c = QColorDialog::getColor(currentHighlightColor, this, "Hintergrundfarbe");
    if (!c.isValid()) return;
    currentHighlightColor = c;
    actHighlight->setIcon(makeHighlightIcon(c));
    QTextCharFormat f; f.setBackground(c);
    editor->mergeCurrentCharFormat(f);
}

void MainWindow::showPdfPreview() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("PDF-Gesamtvorschau");
    preview.resize(800, 600);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &MainWindow::printPreview);
    preview.exec();
}

void MainWindow::printPreview(QPrinter *printer) {
    // Drucke Dokument mit Korrekturrand
    QTextDocument *doc = editor->document()->clone();

    // Seitenmaße berechnen
    QPageLayout pl = printer->pageLayout();
    QRectF pageRect = pl.paintRectPixels(printer->resolution());

    // Korrekturrand: rechtes Drittel reservieren
    double corrRatio = editor->correctionMarginRatio();
    double textWidth = pageRect.width() * (1.0 - corrRatio);

    doc->setPageSize(QSizeF(pageRect.width(), pageRect.height()));
    doc->setTextWidth(textWidth);

    // Zeichne Dokument + Korrekturrand-Linie
    QPainter painter(printer);
    QRectF contentRect(0, 0, textWidth, pageRect.height());

    int pageCount = doc->pageCount();
    for (int page = 0; page < pageCount; ++page) {
        if (page > 0) printer->newPage();

        // Rote Trennlinie
        painter.setPen(QPen(QColor("#cc4444"), 2));
        painter.drawLine(QPointF(textWidth, 0), QPointF(textWidth, pageRect.height()));

        // "Korrekturrand" Text
        painter.save();
        painter.setPen(QColor("#ccaaaa"));
        QFont sf("Segoe UI", 7);
        painter.setFont(sf);
        painter.translate(textWidth + 12, 60);
        painter.rotate(90);
        painter.drawText(0, 0, "Korrekturrand");
        painter.restore();

        // Seiteninhalt rendern
        painter.save();
        painter.translate(0, -page * pageRect.height());
        doc->drawContents(&painter, QRectF(0, page * pageRect.height(),
                                            textWidth, pageRect.height()));
        painter.restore();
    }

    painter.end();
    delete doc;
}

void MainWindow::onCursorPositionChanged() { updateAlignmentActions(); }
void MainWindow::onCurrentCharFormatChanged(const QTextCharFormat &f) {
    actBold->setChecked(f.fontWeight() >= QFont::Bold);
    actItalic->setChecked(f.fontItalic());
    actUnderline->setChecked(f.fontUnderline());
    actSuperscript->setChecked(f.verticalAlignment() == QTextCharFormat::AlignSuperScript);
}

void MainWindow::updateTimer() {
    if (remainingSeconds < 0) remainingSeconds = 0;
    int h = remainingSeconds / 3600;
    int m = (remainingSeconds % 3600) / 60;
    int s = remainingSeconds % 60;
    lblTimer->setText(QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    if (remainingSeconds <= 1800 && remainingSeconds > 0)
        lblTimer->setStyleSheet("color: #ff8888; font-weight: bold;");
    remainingSeconds--;
    lblKeystrokeCount->setText(QString("Tasten: %1").arg(editor->analyzer().keystrokeCount()));
}

// ═════════════════════════════════════════════════════════════════════════════
//  Werkzeuge
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showCalculator() {
    CalculatorDialog dlg(this);
    dlg.exec();
}

void MainWindow::showBugReport() {
    BugReportDialog dlg(this);
    dlg.exec();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Session-Verwaltung (SQLite)
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::saveCurrentSession() {
    const NgramAnalyzer &a = editor->analyzer();
    if (a.keystrokeCount() < 10) {
        QMessageBox::information(this, "Zu wenig Daten",
            "Mindestens 10 Tastenanschläge für eine Session nötig.");
        return;
    }

    SessionRecord rec;
    rec.startTime = m_sessionStart;
    rec.durationSeconds = a.totalSessionTimeSeconds();
    rec.totalKeystrokes = a.keystrokeCount();
    rec.grossWPM = a.grossWPM();
    rec.netWPM = a.netWPM();
    rec.accuracy = a.accuracy();
    rec.kspc = a.kspc();
    rec.backspaceCount = a.backspaceCount();
    rec.motoricRate = a.motoricBackspaceRate();
    rec.automatizationRate = a.automatizationBackspaceRate();
    rec.contentRate = a.contentBackspaceRate();

    if (dbManager->saveSession(rec)) {
        QMessageBox::information(this, "Session gespeichert",
            QString("Session gespeichert!\n\nDauer: %1 Min\nWPM: %2\nAccuracy: %3%\n\nDatenbank: %4")
                .arg(rec.durationSeconds / 60.0, 0, 'f', 1)
                .arg(rec.grossWPM, 0, 'f', 1)
                .arg(rec.accuracy * 100.0, 0, 'f', 1)
                .arg(DatabaseManager::databasePath()));
    } else {
        QMessageBox::warning(this, "Fehler", "Session konnte nicht gespeichert werden.");
    }
}

void MainWindow::showSessionHistory() {
    auto sessions = dbManager->getAllSessions();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Session-Verlauf");
    dlg->resize(900, 500);

    auto *layout = new QVBoxLayout(dlg);

    if (sessions.isEmpty()) {
        layout->addWidget(new QLabel("Keine Sessions gespeichert.\n\nSpeichere eine Session über Auswertung → Session speichern."));
    } else {
        auto *table = new QTableWidget(sessions.size(), 8, dlg);
        table->setHorizontalHeaderLabels({"Datum", "Dauer (Min)", "Tasten", "Gross WPM",
                                           "Net WPM", "Accuracy %", "KSPC", "Backspaces"});
        table->horizontalHeader()->setStretchLastSection(true);
        table->setAlternatingRowColors(true);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);

        for (int i = 0; i < sessions.size(); ++i) {
            const auto &s = sessions[i];
            table->setItem(i, 0, new QTableWidgetItem(s.startTime.toString("dd.MM.yyyy HH:mm")));
            table->setItem(i, 1, new QTableWidgetItem(QString::number(s.durationSeconds / 60.0, 'f', 1)));
            table->setItem(i, 2, new QTableWidgetItem(QString::number(s.totalKeystrokes)));
            table->setItem(i, 3, new QTableWidgetItem(QString::number(s.grossWPM, 'f', 1)));
            table->setItem(i, 4, new QTableWidgetItem(QString::number(s.netWPM, 'f', 1)));
            table->setItem(i, 5, new QTableWidgetItem(QString::number(s.accuracy * 100.0, 'f', 1)));
            table->setItem(i, 6, new QTableWidgetItem(QString::number(s.kspc, 'f', 2)));
            table->setItem(i, 7, new QTableWidgetItem(QString::number(s.backspaceCount)));
        }
        table->resizeColumnsToContents();
        layout->addWidget(table);

        auto *infoLabel = new QLabel(QString("Gesamt: %1 Sessions | Datenbank: %2")
            .arg(sessions.size()).arg(DatabaseManager::databasePath()));
        infoLabel->setStyleSheet("color: #666; padding: 4px;");
        layout->addWidget(infoLabel);
    }

    auto *closeBtn = new QPushButton("Schließen", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg->exec();
}

void MainWindow::showTrendAnalysis() {
    TrendInfo trend = dbManager->analyzeTrends();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Trend-Analyse & Diagnose");
    dlg->resize(700, 550);

    auto *layout = new QVBoxLayout(dlg);

    auto *titleLabel = new QLabel("TREND-ANALYSE & DIAGNOSE");
    titleLabel->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    auto *countLabel = new QLabel(QString("Basierend auf %1 gespeicherten Sessions").arg(trend.sessionCount));
    countLabel->setAlignment(Qt::AlignCenter);
    countLabel->setStyleSheet("color: #666; padding: 4px;");
    layout->addWidget(countLabel);

    // Diagnose
    auto *diagBox = new QTextEdit(dlg);
    diagBox->setReadOnly(true);
    diagBox->setPlainText(trend.diagnosis);
    diagBox->setStyleSheet("background: #f8f8f8; font-family: Consolas; font-size: 10pt; padding: 8px;");
    layout->addWidget(diagBox);

    // Empfehlungen
    auto *recLabel = new QLabel("EMPFEHLUNGEN:");
    recLabel->setFont(QFont("Segoe UI", 12, QFont::Bold));
    layout->addWidget(recLabel);

    auto *recBox = new QTextEdit(dlg);
    recBox->setReadOnly(true);
    recBox->setPlainText(trend.recommendations);
    recBox->setMaximumHeight(150);
    recBox->setStyleSheet("background: #f0f8f0; font-size: 10pt; padding: 8px;");
    layout->addWidget(recBox);

    // WPM-Trend-Chart wenn genug Sessions
    if (trend.sessionCount >= 3) {
        auto sessions = dbManager->getAllSessions();
        QChart *chart = new QChart();
        chart->setTitle("WPM-Verlauf über Sessions");

        auto *wpmSeries = new QLineSeries();
        wpmSeries->setName("Gross WPM");
        auto *accSeries = new QLineSeries();
        accSeries->setName("Accuracy %");

        for (int i = 0; i < sessions.size(); ++i) {
            wpmSeries->append(i + 1, sessions[i].grossWPM);
            accSeries->append(i + 1, sessions[i].accuracy * 100.0);
        }

        chart->addSeries(wpmSeries);
        chart->addSeries(accSeries);
        chart->createDefaultAxes();
        chart->setAnimationOptions(QChart::SeriesAnimations);

        auto *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMaximumHeight(250);
        layout->addWidget(chartView);
    }

    auto *closeBtn = new QPushButton("Schließen", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg->exec();
}

void MainWindow::resetTracking() {
    editor->resetTracking();
    m_sessionStart = QDateTime::currentDateTime();
    lblKeystrokeCount->setText("Tasten: 0");
    QMessageBox::information(this, "Tracking zurückgesetzt",
        "Alle Keystroke-Daten gelöscht. Neues Tracking beginnt jetzt.");
}

// ═════════════════════════════════════════════════════════════════════════════
//  Anonymisierter Daten-Export (JSON)
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::exportAnonymizedData() {
    auto sessions = dbManager->getAllSessions();

    if (sessions.isEmpty()) {
        QMessageBox::information(this, "Keine Daten",
            "Keine gespeicherten Sessions vorhanden.\n\n"
            "Speichere zuerst mindestens eine Session über\n"
            "Auswertung → Session speichern.");
        return;
    }

    // Dateiname vorschlagen
    QString defaultName = QString("editor_v5_export_%1.json")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmm"));

    QString filePath = QFileDialog::getSaveFileName(
        this, "Anonymisierte Daten exportieren",
        QDir::homePath() + "/" + defaultName,
        "JSON-Dateien (*.json);;Alle Dateien (*)");

    if (filePath.isEmpty()) return;

    // JSON aufbauen
    QJsonObject root;
    root["export_version"] = "1.0";
    root["app"] = "editor_v5";
    root["export_date"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["session_count"] = sessions.size();

    // Hinweis für den Empfänger
    root["notice"] = "Anonymisierte Tipp-Metriken. Keine Texte, keine persönlichen Daten.";

    // Sessions als Array
    QJsonArray sessionsArray;
    for (const auto &s : sessions) {
        QJsonObject sessionObj;
        sessionObj["date"] = s.startTime.toString(Qt::ISODate);
        sessionObj["duration_seconds"] = s.durationSeconds;
        sessionObj["total_keystrokes"] = s.totalKeystrokes;
        sessionObj["gross_wpm"] = qRound(s.grossWPM * 10.0) / 10.0;
        sessionObj["net_wpm"] = qRound(s.netWPM * 10.0) / 10.0;
        sessionObj["accuracy"] = qRound(s.accuracy * 1000.0) / 1000.0;
        sessionObj["kspc"] = qRound(s.kspc * 100.0) / 100.0;
        sessionObj["backspace_count"] = s.backspaceCount;
        sessionObj["motoric_rate"] = qRound(s.motoricRate * 10.0) / 10.0;
        sessionObj["automatization_rate"] = qRound(s.automatizationRate * 10.0) / 10.0;
        sessionObj["content_rate"] = qRound(s.contentRate * 10.0) / 10.0;
        sessionsArray.append(sessionObj);
    }
    root["sessions"] = sessionsArray;

    // Zusammenfassung
    TrendInfo trend = dbManager->analyzeTrends();
    QJsonObject summary;
    summary["avg_wpm"] = qRound(trend.avgWPM * 10.0) / 10.0;
    summary["avg_accuracy"] = qRound(trend.avgAccuracy * 1000.0) / 1000.0;
    summary["avg_kspc"] = qRound(trend.avgKSPC * 100.0) / 100.0;
    summary["wpm_trend"] = qRound(trend.wpmTrend * 10.0) / 10.0;
    summary["accuracy_trend"] = qRound(trend.accuracyTrend * 1000.0) / 1000.0;
    root["summary"] = summary;

    // Schreiben
    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();

        QMessageBox::information(this, "Export erfolgreich",
            QString("Anonymisierte Daten exportiert!\n\n"
                    "Datei: %1\n"
                    "Sessions: %2\n"
                    "Größe: %3 KB\n\n"
                    "Die Datei enthält nur Metriken (WPM, Accuracy, KSPC etc.),\n"
                    "keine Texte und keine persönlichen Daten.\n\n"
                    "Bitte sende die Datei an den Entwickler.")
                .arg(filePath)
                .arg(sessions.size())
                .arg(QFileInfo(filePath).size() / 1024.0, 0, 'f', 1));
    } else {
        QMessageBox::warning(this, "Fehler",
            "Datei konnte nicht geschrieben werden:\n" + filePath);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Statistiken (identisch zur vorherigen Version)
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showStatistics() {
    const NgramAnalyzer &a = editor->analyzer();
    int backspaces = a.backspaceCount();
    double backspaceRate = a.backspaceRate();
    double accuracy = a.accuracy();
    double grossWPM = a.grossWPM();
    double netWPM = a.netWPM();
    double kspc = a.kspc();
    int motoricBS = a.motoricBackspaces();
    int autoBS = a.automatizationBackspaces();
    int contentBS = a.contentBackspaces();
    double motoricRate = a.motoricBackspaceRate();
    double autoRate = a.automatizationBackspaceRate();
    double contentRate = a.contentBackspaceRate();
    auto pauses = a.findPauses(500);
    auto legalChars = a.analyzeLegalChars();
    auto wpmList = a.calculateWPMOverTime(60);
    double sessionTime = a.totalSessionTimeSeconds();

    QString st;
    st += "=== SESSION STATISTIKEN ===\n\n";
    st += QString("Dauer: %1 Sek (%2 Min)\n\n").arg(sessionTime,0,'f',1).arg(sessionTime/60.0,0,'f',2);
    st += "=== PERFORMANCE ===\n";
    st += QString("Gross WPM: %1\nNet WPM: %2\nAccuracy: %3%\n")
        .arg(grossWPM,0,'f',1).arg(netWPM,0,'f',1).arg(accuracy*100.0,0,'f',1);
    st += QString("KSPC: %1 ").arg(kspc,0,'f',3);
    if (kspc<=1.05) st+="(Exzellent!)"; else if(kspc<=1.15) st+="(Gut)";
    else if(kspc<=1.30) st+="(Normal)"; else st+="(Verbesserungsbedarf)";
    st += "\n\n=== FEHLERANALYSE ===\n";
    st += QString("Backspace: %1 (Rate: %2%)\n").arg(backspaces).arg(backspaceRate,0,'f',1);
    if (backspaces > 0) {
        st += QString("Motorisch: %1 (%2%)\nAutomatisierung: %3 (%4%)\nInhaltlich: %5 (%6%)\n")
            .arg(motoricBS).arg(motoricRate,0,'f',0)
            .arg(autoBS).arg(autoRate,0,'f',0)
            .arg(contentBS).arg(contentRate,0,'f',0);
    }
    st += "\n=== PAUSEN (>500ms) ===\n";
    st += QString("Anzahl: %1\n").arg(pauses.size());
    for (int i=0;i<qMin(5,pauses.size());++i)
        st += QString("  %1ms: \"%2|%3\"\n").arg(pauses[i].durationMs).arg(pauses[i].contextBefore,pauses[i].contextAfter);
    st += "\n=== LEGAL ZEICHEN ===\n";
    if (legalChars.isEmpty()) st += "Keine\n";
    else for (const auto &s : std::as_const(legalChars))
        st += QString("  '%1': %2x (Ø %3ms)\n").arg(s.character).arg(s.count).arg(s.avgTimeMs,0,'f',1);
    st += "\n=== WPM/ZEIT ===\n";
    if (wpmList.isEmpty()) st += "Nicht genug Daten\n";
    else for (int i=0;i<wpmList.size();++i)
        st += QString("  Min %1: %2 WPM\n").arg(i+1).arg(wpmList[i],0,'f',1);
    st += "\n" + generateRecommendations();

    QDialog *d = new QDialog(this); d->setWindowTitle("Statistiken"); d->resize(750,600);
    auto *l = new QVBoxLayout(d);
    auto *te = new QTextEdit(d); te->setReadOnly(true); te->setPlainText(st);
    te->setStyleSheet("background:#fff;color:#000;font-family:Consolas;font-size:10pt;");
    l->addWidget(te);
    auto *cb = new QPushButton("Schließen",d); connect(cb,&QPushButton::clicked,d,&QDialog::accept);
    l->addWidget(cb); d->exec();
}

QString MainWindow::generateRecommendations() {
    const NgramAnalyzer &a = editor->analyzer();
    QString t; t += "=== EMPFEHLUNGEN ===\n\n"; int r=1;
    if (a.automatizationBackspaceRate()>50.0) {
        t+=QString("%1. 10-Finger-Training (%2% Tastsuche)\n\n").arg(r++).arg(a.automatizationBackspaceRate(),0,'f',0);
    }
    if (a.contentBackspaceRate()>5.0) {
        t+=QString("%1. Fachbegriffe wiederholen (%2% Unsicherheit)\n\n").arg(r++).arg(a.contentBackspaceRate(),0,'f',1);
    }
    double acc=a.accuracy()*100.0;
    if (acc<90.0) t+=QString("%1. Genauigkeit steigern (aktuell %2%)\n\n").arg(r++).arg(acc,0,'f',1);
    if (acc>=90.0&&a.grossWPM()<50.0) t+=QString("%1. Geschwindigkeit erhöhen (%2 WPM)\n\n").arg(r++).arg(a.grossWPM(),0,'f',1);
    if (r==1) t+="Alles gut - weiter so!\n";
    return t;
}

// Dashboard, PieChart, Heatmap, WPM Chart — gleiche Logik wie zuvor
void MainWindow::showPerformanceDashboard() {
    const NgramAnalyzer &a = editor->analyzer();
    double gWPM=a.grossWPM(), acc=a.accuracy()*100.0, k=a.kspc();
    QDialog *d=new QDialog(this); d->setWindowTitle("Performance Dashboard"); d->resize(900,420);
    auto *ml=new QVBoxLayout(d);
    auto *t=new QLabel("DEINE TIPP-PERFORMANCE"); t->setFont(QFont("Segoe UI",18,QFont::Bold));
    t->setAlignment(Qt::AlignCenter); ml->addWidget(t);
    auto *cl=new QHBoxLayout; cl->setSpacing(20);
    cl->addWidget(createPerformanceCard("GESCHWINDIGKEIT",QString::number(gWPM,'f',1),"WPM",getWPMRating(gWPM),getWPMColor(gWPM)));
    cl->addWidget(createPerformanceCard("GENAUIGKEIT",QString::number(acc,'f',1),"%",getAccuracyRating(acc),getAccuracyColor(acc)));
    cl->addWidget(createPerformanceCard("EFFIZIENZ",QString::number(k,'f',2),"KSPC",getKSPCRating(k),getKSPCColor(k)));
    ml->addLayout(cl);
    auto *cb=new QPushButton("Schließen",d); connect(cb,&QPushButton::clicked,d,&QDialog::accept); ml->addWidget(cb);
    d->exec();
}

QFrame *MainWindow::createPerformanceCard(const QString &title, const QString &value,
    const QString &unit, const QString &rating, const QColor &color) {
    QFrame *card=new QFrame; card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(QString("QFrame{background:white;border:2px solid %1;border-radius:10px;padding:20px;}").arg(color.name()));
    auto *l=new QVBoxLayout(card); l->setSpacing(10);
    auto *tl=new QLabel(title); tl->setFont(QFont("Segoe UI",14,QFont::Bold)); tl->setAlignment(Qt::AlignCenter); tl->setStyleSheet("color:#555;"); l->addWidget(tl);
    auto *vl=new QLabel(value); vl->setFont(QFont("Segoe UI",36,QFont::Bold)); vl->setAlignment(Qt::AlignCenter); vl->setStyleSheet(QString("color:%1;").arg(color.name())); l->addWidget(vl);
    auto *ul=new QLabel(unit); ul->setFont(QFont("Segoe UI",12)); ul->setAlignment(Qt::AlignCenter); ul->setStyleSheet("color:#888;"); l->addWidget(ul);
    auto *rl=new QLabel(rating); rl->setFont(QFont("Segoe UI",11)); rl->setAlignment(Qt::AlignCenter); rl->setStyleSheet("color:#666;"); rl->setWordWrap(true); l->addWidget(rl);
    return card;
}

QString MainWindow::getWPMRating(double w) const { return w>=60?"Sehr schnell!":w>=50?"Gut":w>=40?"Durchschnitt":"Üben!"; }
QColor MainWindow::getWPMColor(double w) const { return w>=60?QColor(76,175,80):w>=40?QColor(255,193,7):QColor(244,67,54); }
QString MainWindow::getAccuracyRating(double a) const { return a>=95?"Exzellent!":a>=90?"Gut":a>=85?"OK":"Üben nötig"; }
QColor MainWindow::getAccuracyColor(double a) const { return a>=95?QColor(76,175,80):a>=85?QColor(255,193,7):QColor(244,67,54); }
QString MainWindow::getKSPCRating(double k) const { return k<=1.1?"Sehr effizient!":k<=1.2?"Gut":k<=1.3?"Durchschnitt":"Zu viele Korrekturen"; }
QColor MainWindow::getKSPCColor(double k) const { return k<=1.1?QColor(76,175,80):k<=1.3?QColor(255,193,7):QColor(244,67,54); }

void MainWindow::showErrorPieChart() {
    const NgramAnalyzer &a = editor->analyzer();
    int mot=a.motoricBackspaces(), aut=a.automatizationBackspaces(), con=a.contentBackspaces(), tot=a.backspaceCount();
    if (tot==0) { QMessageBox::information(this,"Keine Fehler","Keine Fehler!"); return; }
    auto *s=new QPieSeries(); s->append("Automatisierung",aut); s->append("Motorisch",mot); s->append("Inhaltlich",con);
    s->slices().at(0)->setColor(QColor(255,193,7)); s->slices().at(0)->setBorderColor(Qt::white); s->slices().at(0)->setBorderWidth(2);
    s->slices().at(1)->setColor(QColor(76,175,80)); s->slices().at(1)->setBorderColor(Qt::white); s->slices().at(1)->setBorderWidth(2);
    s->slices().at(2)->setColor(QColor(33,150,243)); s->slices().at(2)->setBorderColor(Qt::white); s->slices().at(2)->setBorderWidth(2);
    for (auto sl : s->slices()) { double p=(sl->value()/tot)*100.0; sl->setLabel(QString("%1\n%2 (%3%)").arg(sl->label()).arg(int(sl->value())).arg(p,0,'f',0)); sl->setLabelVisible(true); sl->setLabelColor(Qt::black); }
    QPieSlice *mx=nullptr; double mv=0; for(auto sl:s->slices()) if(sl->value()>mv){mv=sl->value();mx=sl;} if(mx){mx->setExploded(true);mx->setExplodeDistanceFactor(0.1);}
    auto *ch=new QChart(); ch->addSeries(s); ch->setTitle("Fehlerquellen"); ch->legend()->setAlignment(Qt::AlignBottom); ch->setAnimationOptions(QChart::SeriesAnimations);
    auto *cv=new QChartView(ch); cv->setRenderHint(QPainter::Antialiasing);
    QDialog *d=new QDialog(this); d->setWindowTitle("Fehleranalyse"); d->resize(700,550);
    auto *l=new QVBoxLayout(d); l->addWidget(cv);
    auto *cb=new QPushButton("Schließen",d); connect(cb,&QPushButton::clicked,d,&QDialog::accept); l->addWidget(cb); d->exec();
}

void MainWindow::showWPMChart() {
    auto wl=editor->analyzer().calculateWPMOverTime(60);
    if (wl.isEmpty()) { QMessageBox::information(this,"WPM","Tippe mindestens 1 Minute!"); return; }
    auto *ch=new QChart(); ch->setTitle("WPM über Zeit"); ch->setAnimationOptions(QChart::SeriesAnimations);
    auto *sr=new QLineSeries(); sr->setName("WPM");
    for(int i=0;i<wl.size();++i) sr->append(i+1,wl[i]);
    ch->addSeries(sr);
    auto *ax=new QValueAxis(); ax->setTitleText("Minute"); ax->setLabelFormat("%d"); ax->setRange(0,wl.size()+1); ch->addAxis(ax,Qt::AlignBottom); sr->attachAxis(ax);
    auto *ay=new QValueAxis(); ay->setTitleText("WPM"); ay->setLabelFormat("%d");
    double mn=*std::min_element(wl.begin(),wl.end()), mx=*std::max_element(wl.begin(),wl.end()), rg=mx-mn;
    ay->setRange(qMax(0.0,mn-rg*0.1),mx+rg*0.1); ch->addAxis(ay,Qt::AlignLeft); sr->attachAxis(ay);
    auto *cv=new QChartView(ch); cv->setRenderHint(QPainter::Antialiasing);
    QDialog *d=new QDialog(this); d->setWindowTitle("WPM Verlauf"); d->resize(800,500);
    auto *l=new QVBoxLayout(d); l->addWidget(cv);
    auto *cb=new QPushButton("Schließen",d); connect(cb,&QPushButton::clicked,d,&QDialog::close); l->addWidget(cb); d->exec();
}

void MainWindow::showKeyboardHeatmap() {
    QMap<QChar,int> ke=analyzeKeyErrors();
    if (ke.isEmpty()) { QMessageBox::information(this,"Heatmap","Keine Fehler!"); return; }
    QDialog *d=new QDialog(this); d->setWindowTitle("Tastatur-Heatmap"); d->resize(900,500);
    auto *ml=new QVBoxLayout(d);
    auto *t=new QLabel("PROBLEM-TASTEN"); t->setFont(QFont("Segoe UI",18,QFont::Bold)); t->setAlignment(Qt::AlignCenter); ml->addWidget(t);
    auto *kw=new QWidget(d); auto *kl=new QVBoxLayout(kw); kl->setSpacing(5);
    QString row1Keys = QString::fromUtf8("QWERTZUIOPÜ");
    auto *r1=new QHBoxLayout(); r1->setSpacing(3);
    for(const QChar &k:std::as_const(row1Keys)) r1->addWidget(createKeyButton(k,ke.value(k.toLower(),0)));
    kl->addLayout(r1);
    QString row2Keys = QString::fromUtf8("ASDFGHJKLÖÄ");
    auto *r2=new QHBoxLayout(); r2->setSpacing(3); r2->addSpacing(20);
    for(const QChar &k:std::as_const(row2Keys)) r2->addWidget(createKeyButton(k,ke.value(k.toLower(),0)));
    kl->addLayout(r2);
    QString row3Keys = QString("YXCVBNM");
    auto *r3=new QHBoxLayout(); r3->setSpacing(3); r3->addSpacing(40);
    for(const QChar &k:std::as_const(row3Keys)) r3->addWidget(createKeyButton(k,ke.value(k.toLower(),0)));
    r3->addWidget(createKeyButton(QChar(0xDF),ke.value(QChar(0xDF),0)));
    kl->addLayout(r3);
    auto *r4=new QHBoxLayout(); r4->addSpacing(80);
    auto *sp=createKeyButton(' ',ke.value(' ',0)); sp->setMinimumWidth(300); sp->setText("LEERTASTE"); r4->addWidget(sp);
    kl->addLayout(r4);
    ml->addWidget(kw);
    auto *lg=new QLabel("Rot: >10  |  Orange: 5-10  |  Gelb: 1-5  |  Grau: 0"); lg->setAlignment(Qt::AlignCenter); lg->setStyleSheet("color:#666;padding:6px;"); ml->addWidget(lg);
    auto *cb=new QPushButton("Schließen",d); connect(cb,&QPushButton::clicked,d,&QDialog::accept); ml->addWidget(cb); d->exec();
}

QMap<QChar,int> MainWindow::analyzeKeyErrors() {
    QMap<QChar,int> ke; auto ev=editor->analyzer().getBackspaceEvents();
    for(const auto &e:std::as_const(ev)){QChar c=e.deletedChar.toLower();if(!c.isNull()&&c!='\b')ke[c]++;}
    return ke;
}

QPushButton *MainWindow::createKeyButton(QChar key, int err) {
    auto *b=new QPushButton(QString(key).toUpper()); b->setMinimumSize(50,50); b->setMaximumSize(50,50);
    QString c,tc="black";
    if(err>10){c="rgb(244,67,54)";tc="white";}else if(err>5){c="rgb(255,152,0)";tc="white";}
    else if(err>0) c="rgb(255,193,7)"; else c="rgb(224,224,224)";
    b->setStyleSheet(QString("QPushButton{background:%1;color:%2;border:1px solid #999;border-radius:5px;font-size:14pt;font-weight:bold;}QPushButton:hover{border:2px solid #333;}").arg(c,tc));
    b->setToolTip(err>0?QString("%1x Fehler").arg(err):"Keine Fehler");
    return b;
}

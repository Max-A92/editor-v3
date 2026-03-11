#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QTimer>
#include <QFrame>
#include <QMap>
#include <QDateTime>
#include <QPrinter>
#include <QPushButton>

#include "richtexteditor.h"
#include "databasemanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Formatierung
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void toggleSuperscript();
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();
    void increaseIndent();
    void insertPageBreak();
    void insertNonBreakingSpace();
    void insertSpecialChar();
    void insertTable();
    void setHighlightColor();
    void showPdfPreview();
    void printPreview(QPrinter *printer);
    void onCursorPositionChanged();
    void onCurrentCharFormatChanged(const QTextCharFormat &fmt);
    void updateTimer();

    // Werkzeuge
    void showCalculator();
    void showBugReport();

    // Auswertung
    void showStatistics();
    void showPerformanceDashboard();
    void showErrorPieChart();
    void showKeyboardHeatmap();
    void showWPMChart();
    void resetTracking();
    void saveCurrentSession();
    void showSessionHistory();
    void showTrendAnalysis();
    void exportAnonymizedData();

private:
    void setupHeaderBar();
    void setupToolBar();
    void setupEditor();
    void updateAlignmentActions();

    // Auswertung Hilfsfunktionen
    QString generateRecommendations();
    QFrame *createPerformanceCard(const QString &title, const QString &value,
                                  const QString &unit, const QString &rating,
                                  const QColor &color);
    QString getWPMRating(double wpm) const;
    QColor  getWPMColor(double wpm) const;
    QString getAccuracyRating(double acc) const;
    QColor  getAccuracyColor(double acc) const;
    QString getKSPCRating(double k) const;
    QColor  getKSPCColor(double k) const;
    QMap<QChar, int> analyzeKeyErrors();
    QPushButton *createKeyButton(QChar key, int errorCount);

    // Icon-Erzeugung (eigener Stil)
    static QIcon makeIcon(const QString &text, int size = 26,
                          const QColor &fg = QColor("#3a3a50"),
                          bool bold = false, bool circle = false);

    RichTextEditor *editor;
    DatabaseManager *dbManager;
    QDateTime m_sessionStart;

    // Toolbar-Aktionen
    QAction *actUndo, *actRedo, *actCut, *actCopy, *actPaste;
    QAction *actPageBreak, *actHighlight;
    QAction *actBold, *actItalic, *actUnderline, *actSuperscript;
    QAction *actAlignLeft, *actAlignCenter, *actAlignRight, *actJustify;
    QAction *actIndent, *actNbsp, *actSpecialChar;
    QToolButton *btnTable;
    QAction *actPdfPreview;

    // Header-Elemente
    QLabel *lblTimer;
    QLabel *lblKeystrokeCount;
    QTimer *examTimer;
    int remainingSeconds;
    QColor currentHighlightColor;
};

#endif // MAINWINDOW_H

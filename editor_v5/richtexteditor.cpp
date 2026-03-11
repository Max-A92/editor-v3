#include "richtexteditor.h"
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    setAcceptRichText(true);

    QFont editorFont("Segoe UI", 12);
    setFont(editorFont);
    document()->setDefaultFont(editorFont);

    // Textfarbe explizit schwarz
    QTextCharFormat defaultFmt;
    defaultFmt.setForeground(QColor("#000000"));
    QTextCursor c(document());
    c.select(QTextCursor::Document);
    c.mergeCharFormat(defaultFmt);
    setCurrentCharFormat(defaultFmt);

    setTabStopDistance(40.0);
    setLineWrapMode(QTextEdit::FixedPixelWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    setStyleSheet(
        "QTextEdit {"
        "    background-color: #e0e2e6;"
        "    color: #000000;"
        "    border: none;"
        "    selection-background-color: #5b7fb5;"
        "    selection-color: #ffffff;"
        "}"
    );

    document()->setDocumentMargin(24);
    m_elapsed.start();
}

NgramAnalyzer &RichTextEditor::analyzer() { return m_analyzer; }
const NgramAnalyzer &RichTextEditor::analyzer() const { return m_analyzer; }

void RichTextEditor::resetTracking()
{
    m_analyzer.clear();
    m_elapsed.restart();
}

void RichTextEditor::setCorrectionMarginRatio(double ratio)
{
    m_correctionMarginRatio = qBound(0.0, ratio, 0.5);
    updatePageLayout();
    viewport()->update();
}

double RichTextEditor::correctionMarginRatio() const { return m_correctionMarginRatio; }

void RichTextEditor::keyPressEvent(QKeyEvent *event)
{
    qint64 now = m_elapsed.elapsed();
    if (event->key() == Qt::Key_Backspace) {
        m_analyzer.addBackspace(now);
    } else {
        QChar ch = event->text().isEmpty() ? QChar() : event->text().at(0);
        if (ch.isPrint())
            m_analyzer.addKeystroke(ch, now, -1);
    }
    QTextEdit::keyPressEvent(event);
}

void RichTextEditor::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        QChar ch = event->text().isEmpty() ? QChar() : event->text().at(0);
        if (ch.isPrint()) {
            qint64 now = m_elapsed.elapsed();
            m_analyzer.patchLastUpMs(ch, now);
        }
    }
    QTextEdit::keyReleaseEvent(event);
}

void RichTextEditor::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    updatePageLayout();
}

void RichTextEditor::updatePageLayout()
{
    if (m_inLayoutUpdate) return;
    m_inLayoutUpdate = true;

    int vpWidth = viewport()->width();
    int pageWidth = qMin(780, vpWidth - 20);
    int marginH = qMax(10, (vpWidth - pageWidth) / 2);

    // Textbereich: nur der linke Teil (ohne Korrekturrand)
    int correctionWidth = static_cast<int>(pageWidth * m_correctionMarginRatio);
    int textAreaWidth = pageWidth - correctionWidth;

    setViewportMargins(marginH, 30, marginH + correctionWidth, 30);

    // QTextEdit::FixedPixelWidth nutzt lineWrapColumnOrWidth
    setLineWrapColumnOrWidth(textAreaWidth);

    m_inLayoutUpdate = false;
}

void RichTextEditor::paintEvent(QPaintEvent *event)
{
    QPainter bgPainter(viewport());

    // Weißes Papier
    QRect pageRect = viewport()->rect();
    int correctionWidth = static_cast<int>(
        (pageRect.width() + static_cast<int>(pageRect.width() * m_correctionMarginRatio / (1.0 - m_correctionMarginRatio)))
    );

    // Zeichne Papier über gesamte Breite inkl. Korrekturrand
    int totalPageWidth = pageRect.width() + static_cast<int>(
        (pageRect.width() * m_correctionMarginRatio) / (1.0 - m_correctionMarginRatio));

    QRect fullPage(pageRect.left(), pageRect.top(),
                   totalPageWidth, pageRect.height());
    bgPainter.fillRect(fullPage, QColor("#ffffff"));

    // Korrekturrand: Hintergrund leicht getönt
    int corrW = totalPageWidth - pageRect.width();
    QRect corrRect(pageRect.right(), pageRect.top(), corrW, pageRect.height());
    bgPainter.fillRect(corrRect, QColor("#f5f5f0"));

    // Vertikale Trennlinie
    int lineX = pageRect.right();
    bgPainter.setPen(QPen(QColor("#cc4444"), 1.5));
    bgPainter.drawLine(lineX, pageRect.top(), lineX, pageRect.bottom());

    // Beschriftung "Korrekturrand" (gedreht)
    bgPainter.save();
    bgPainter.setPen(QColor("#bb9999"));
    QFont smallFont("Segoe UI", 8);
    bgPainter.setFont(smallFont);
    bgPainter.translate(lineX + 14, pageRect.top() + 100);
    bgPainter.rotate(90);
    bgPainter.drawText(0, 0, "Korrekturrand");
    bgPainter.restore();

    // Papier-Rahmen
    bgPainter.setPen(QPen(QColor("#d0d0d0"), 1));
    bgPainter.drawRect(pageRect.adjusted(0, 0, corrW - 1, -1));

    bgPainter.end();

    // Normales QTextEdit-Rendering
    QTextEdit::paintEvent(event);
}

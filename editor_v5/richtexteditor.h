#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QTextEdit>
#include <QElapsedTimer>

#include "ngramanalyzer.h"

class RichTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit RichTextEditor(QWidget *parent = nullptr);

    NgramAnalyzer &analyzer();
    const NgramAnalyzer &analyzer() const;
    void resetTracking();

    /// Korrekturrand: Anteil der Seitenbreite (0.0 = kein Rand, 0.33 = ein Drittel)
    void setCorrectionMarginRatio(double ratio);
    double correctionMarginRatio() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void updatePageLayout();

    NgramAnalyzer m_analyzer;
    QElapsedTimer m_elapsed;
    bool m_inLayoutUpdate = false;
    double m_correctionMarginRatio = 0.33;  // 1/3 Korrekturrand
};

#endif // RICHTEXTEDITOR_H

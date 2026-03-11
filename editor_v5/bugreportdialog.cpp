#include "bugreportdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QMessageBox>
#include <QSysInfo>

BugReportDialog::BugReportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Bug Report / Feedback");
    resize(550, 420);

    auto *layout = new QVBoxLayout(this);

    auto *infoLabel = new QLabel(
        "Beschreibe das Problem oder sende Feedback.\n"
        "Der Report wird als GitHub Issue erstellt.");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #555; padding: 6px;");
    layout->addWidget(infoLabel);

    // Title
    layout->addWidget(new QLabel("Titel:"));
    m_titleEdit = new QLineEdit;
    m_titleEdit->setPlaceholderText("Kurze Beschreibung des Problems...");
    layout->addWidget(m_titleEdit);

    // Category
    layout->addWidget(new QLabel("Kategorie:"));
    m_categoryCombo = new QComboBox;
    m_categoryCombo->addItem("Bug / Fehler");
    m_categoryCombo->addItem("Feature-Wunsch");
    m_categoryCombo->addItem("UI / Design");
    m_categoryCombo->addItem("Auswertung / Statistik");
    m_categoryCombo->addItem("Sonstiges");
    layout->addWidget(m_categoryCombo);

    // Description
    layout->addWidget(new QLabel("Beschreibung:"));
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setPlaceholderText(
        "Was ist passiert?\n"
        "Was hast du erwartet?\n"
        "Schritte zum Reproduzieren...");
    layout->addWidget(m_descriptionEdit);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    auto *submitBtn = new QPushButton("Report auf GitHub erstellen");
    submitBtn->setStyleSheet(
        "QPushButton { background: #4a90d0; color: white; border: none;"
        "    border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
        "QPushButton:hover { background: #5aa0e0; }");
    connect(submitBtn, &QPushButton::clicked, this, &BugReportDialog::submitReport);

    auto *cancelBtn = new QPushButton("Abbrechen");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(submitBtn);
    layout->addLayout(btnLayout);
}

void BugReportDialog::submitReport()
{
    QString title = m_titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Titel fehlt", "Bitte gib einen Titel ein.");
        return;
    }

    QString category = m_categoryCombo->currentText();
    QString description = m_descriptionEdit->toPlainText().trimmed();

    // System-Info sammeln
    QString sysInfo = QString("\n\n---\n**System:** %1 %2\n**Qt:** %3\n**App:** editor_v3")
        .arg(QSysInfo::prettyProductName(),
             QSysInfo::currentCpuArchitecture(),
             qVersion());

    // Build GitHub Issue URL
    QString body = QString("**Kategorie:** %1\n\n%2%3")
        .arg(category, description, sysInfo);

    QUrl url("https://github.com/Max-A92/editor-v3/issues/new");
    QUrlQuery query;
    query.addQueryItem("title", "[" + category + "] " + title);
    query.addQueryItem("body", body);
    url.setQuery(query);

    QDesktopServices::openUrl(url);
    accept();
}

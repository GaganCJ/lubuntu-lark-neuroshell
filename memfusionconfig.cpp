#include "memfusionconfig.h"
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QFormLayout>
#include <QLabel>

MemFusionConfig::MemFusionConfig(QWidget *parent) : QDialog(parent) {
    setWindowTitle("LxQt MemFusion Watchdog");
    resize(600, 480);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(25, 25, 25, 25);
    setStyleSheet("background-color: #0d1117; color: #c9d1d9;");

    QLabel* title = new QLabel("MemFusion Watchdog", this);
    title->setStyleSheet("color: #58a6ff; font-size: 16px; font-weight: bold; margin-bottom: 0;");
    QLabel* subtitle = new QLabel("Hardware RAM Expansion & Virtualization Pipeline", this);
    subtitle->setStyleSheet("color: #8b949e; font-size: 13px; margin-top: 0;");

    layout->addWidget(title);
    layout->addWidget(subtitle);

    m_formLayout = new QFormLayout();
    m_formLayout->setContentsMargins(15, 15, 15, 15);
    layout->addLayout(m_formLayout);

    // Refresh system metrics every 3 seconds
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MemFusionConfig::updateStats);
    timer->start(3000); 
    updateStats();
}

MemFusionConfig::~MemFusionConfig() {}

QString MemFusionConfig::readSystemFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return "N/A";
    QTextStream in(&file);
    return in.readLine().trimmed();
}

void MemFusionConfig::updateStats() {
    QString zramSizeRaw = readSystemFile("/sys/block/zram0/disksize");
    QString algoRaw = readSystemFile("/sys/block/zram0/comp_algorithm");
    QString swappinessRaw = readSystemFile("/proc/sys/vm/swappiness");

    qint64 sizeMb = zramSizeRaw != "N/A" ? zramSizeRaw.toLongLong() / (1024 * 1024) : 0;

    // Clear old rows
    while (m_formLayout->rowCount() > 0) {
        m_formLayout->removeRow(0);
    }

    auto addRow = & {
        QLabel* valueLabel = new QLabel(value, this);
        if (!style.isEmpty()) {
            valueLabel->setStyleSheet(style);
        }
        m_formLayout->addRow(label, valueLabel);
    };

    QString status = sizeMb > 0 ? "Active (Tracking)" : "Offline";
    QString statusStyle = sizeMb > 0 ? "color: #3fb950; font-weight: bold;" : "color: #d29922; font-weight: bold;";

    addRow("zRam Target Block:", "/dev/zram0");
    addRow("Subsystem Status:", status, statusStyle);
    addRow("Allocated zRam Size:", QString::number(sizeMb) + " MB");
    addRow("Compression Algorithm:", algoRaw.contains("[") ? algoRaw : "N/A");
    addRow("System Swappiness:", swappinessRaw);
}
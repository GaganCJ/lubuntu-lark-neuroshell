#include "memfusionconfig.h"
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QUltralightView>

MemFusionConfig::MemFusionConfig(QWidget *parent) : QDialog(parent) {
    setWindowTitle("LxQt MemFusion Watchdog");
    resize(600, 480);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_webView = new QUltralightView(this);
    layout->addWidget(m_webView);

    initializeUI();

    // Refresh system metrics every 3 seconds
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MemFusionConfig::updateStats);
    timer->start(3000); 
    updateStats();
}

MemFusionConfig::~MemFusionConfig() {}

void MemFusionConfig::initializeUI() {
    QString html = R"(
        <!DOCTYPE html><html><head><style>
            body { font-family: sans-serif; background-color: #0d1117; color: #c9d1d9; padding: 25px; margin: 0; }
            h2 { color: #58a6ff; margin-bottom: 5px; }
            p { color: #8b949e; font-size: 13px; margin-top: 0; }
            .card { background-color: #161b22; border: 1px solid #30363d; border-radius: 8px; padding: 15px; margin-top: 15px; }
            .metric { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #21262d; }
            .metric:last-child { border-bottom: none; }
            .val { color: #79c0ff; font-weight: bold; font-family: monospace; }
            .status-ok { color: #3fb950; font-weight: bold; }
            .status-warn { color: #d29922; font-weight: bold; }
        </style></head><body>
        <h2>MemFusion Watchdog</h2>
        <p>Hardware RAM Expansion & Virtualization Pipeline</p>
        <div class="card" id="stats">Fetching kernel allocation metrics...</div>
        <script>
            function updateDashboard(data) {
                const obj = JSON.parse(data);
                const zramStatus = obj.zramSize > 0 ? '<span class="status-ok">Active (Tracking)</span>' : '<span class="status-warn">Offline</span>';
                let html = `
                    <div class="metric"><span>zRam Target Block</span><span class="val">/dev/zram0</span></div>
                    <div class="metric"><span>Subsystem Status</span><span>${zramStatus}</span></div>
                    <div class="metric"><span>Allocated zRam Size</span><span class="val">${obj.zramSize} MB</span></div>
                    <div class="metric"><span>Compression Algorithm</span><span class="val">${obj.algo}</span></div>
                    <div class="metric"><span>System Swappiness</span><span class="val">${obj.swappiness}</span></div>
                `;
                document.getElementById('stats').innerHTML = html;
            }
        </script>
        </body></html>
    )";
    m_webView->loadHTML(html);
}

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

    QJsonObject stats;
    stats["zramSize"] = QString::number(sizeMb);
    stats["algo"] = algoRaw.contains("[") ? algoRaw : "N/A"; // Typical output shows [zstd] lz4 ...
    stats["swappiness"] = swappinessRaw;

    QString js = QString("updateDashboard('%1');").arg(QJsonDocument(stats).toJson(QJsonDocument::Compact).replace("'", "\\'"));
    m_webView->evaluateJavaScript(js);
}
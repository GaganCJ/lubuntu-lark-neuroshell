#include "modelmanager.h"
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>
#include <QNetworkRequest>
#include <QUltralightView>

ModelManager::ModelManager(QWidget *parent) 
    : QDialog(parent), m_netManager(new QNetworkAccessManager(this)) {

    setWindowTitle("NeuroShell Local LLM Manager");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_webView = new QUltralightView(this);
    layout->addWidget(m_webView);

    initializeUI();

    // Run periodic check loops targeting Ollama port
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ModelManager::fetchOllamaStats);
    timer->start(3000); 
    fetchOllamaStats();

    connect(m_netManager, &QNetworkAccessManager::finished, this, &ModelManager::handleNetworkReply);
}

ModelManager::~ModelManager() {}

void ModelManager::initializeUI() {
    QString html = R"(
        <!DOCTYPE html><html><head><style>
            body { font-family: sans-serif; background-color: #0d1117; color: #c9d1d9; padding: 20px; }
            h2 { color: #58a6ff; }
            table { width: 100%; border-collapse: collapse; margin-top: 15px; }
            th, td { border: 1px solid #30363d; padding: 12px; text-align: left; }
            th { background-color: #21262d; }
        </style></head><body>
        <h2>Installed Local Models & Hardware Print</h2><div id="models">Loading inference metrics...</div>
        <script>
            function updateModels(data) {
                const obj = JSON.parse(data);
                let html = '<table><tr><th>Model Tag</th><th>Model Size (GB)</th><th>Family</th></tr>';
                if(obj.models) {
                    obj.models.forEach(m => {
                        html += `<tr>
                            <td><strong>${m.name}</strong></td>
                            <td>${(m.size / 1073741824).toFixed(2)} GB</td>
                            <td>${m.details.family}</td>
                        </tr>`;
                    });
                }
                html += '</table>';
                document.getElementById('models').innerHTML = html;
            }
        </script>
        </body></html>
    )";
    m_webView->loadHTML(html);
}

void ModelManager::fetchOllamaStats() {
    QNetworkRequest request(QUrl("http://localhost:11434/api/tags"));
    m_netManager->get(request);
}

void ModelManager::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString js = QString("updateModels('%1');").arg(reply->readAll().replace("'", "\\'").replace("\n", ""));
        m_webView->evaluateJavaScript(js);
    }
    reply->deleteLater();
}
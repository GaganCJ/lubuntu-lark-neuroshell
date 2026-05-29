#include "modelmanager.h"
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>
#include <QNetworkRequest>
#include <QHeaderView>
#include <QTableWidget>
#include <QLabel>

ModelManager::ModelManager(QWidget *parent) 
    : QDialog(parent), m_netManager(new QNetworkAccessManager(this)) {

    setWindowTitle("NeuroShell Local LLM Manager");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    setStyleSheet("background-color: #0d1117; color: #c9d1d9;");

    QLabel* title = new QLabel("Installed Local Models & Hardware Print", this);
    title->setStyleSheet("color: #58a6ff; font-size: 16px; font-weight: bold;");
    layout->addWidget(title);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Model Tag", "Model Size (GB)", "Family"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->hide();
    m_table->setShowGrid(false);
    m_table->setStyleSheet(R"(
        QTableWidget { border: 1px solid #30363d; gridline-color: #30363d; }
        QHeaderView::section { background-color: #21262d; border: 1px solid #30363d; padding: 8px; }
    )");
    layout->addWidget(m_table);
    // Run periodic check loops targeting Ollama port
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ModelManager::fetchOllamaStats);
    timer->start(3000); 
    fetchOllamaStats();

    connect(m_netManager, &QNetworkAccessManager::finished, this, &ModelManager::handleNetworkReply);
}

ModelManager::~ModelManager() {}

void ModelManager::fetchOllamaStats() {
    QNetworkRequest request(QUrl("http://localhost:11434/api/tags"));
    m_netManager->get(request);
}

void ModelManager::handleNetworkReply(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        QJsonArray models = root["models"].toArray();

        m_table->setRowCount(models.count());
        int row = 0;
        for (const QJsonValue &val : models) {
            QJsonObject model = val.toObject();
            QJsonObject details = model["details"].toObject();

            m_table->setItem(row, 0, new QTableWidgetItem(model["name"].toString()));
            double sizeGb = model["size"].toDouble() / 1073741824.0;
            m_table->setItem(row, 1, new QTableWidgetItem(QString::number(sizeGb, 'f', 2) + " GB"));
            m_table->setItem(row, 2, new QTableWidgetItem(details["family"].toString()));
            row++;
        }
    }
    reply->deleteLater();
}
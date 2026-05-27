#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QUltralightView;

class ModelManager : public QDialog {
    Q_OBJECT

public:
    explicit ModelManager(QWidget *parent = nullptr);
    ~ModelManager();

private slots:
    void fetchOllamaStats();
    void handleNetworkReply(QNetworkReply *reply);

private:
    void initializeUI();

    QNetworkAccessManager *m_netManager;
    QUltralightView *m_webView;
};

#endif // MODELMANAGER_H
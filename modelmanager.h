#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Forward declare native Qt widgets
class QTableWidget;

class ModelManager : public QDialog {
    Q_OBJECT

public:
    explicit ModelManager(QWidget *parent = nullptr);
    ~ModelManager();

private slots:
    void fetchOllamaStats();
    void handleNetworkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_netManager;
    QTableWidget *m_table;
};

#endif // MODELMANAGER_H
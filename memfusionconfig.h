#ifndef MEMFUSIONCONFIG_H
#define MEMFUSIONCONFIG_H

#include <QDialog>

class QUltralightView;

class MemFusionConfig : public QDialog {
    Q_OBJECT

public:
    explicit MemFusionConfig(QWidget *parent = nullptr);
    ~MemFusionConfig();

private slots:
    void updateStats();

private:
    void initializeUI();
    QString readSystemFile(const QString &path);

    QUltralightView *m_webView;
};

#endif // MEMFUSIONCONFIG_H
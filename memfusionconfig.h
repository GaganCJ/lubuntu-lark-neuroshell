#ifndef MEMFUSIONCONFIG_H
#define MEMFUSIONCONFIG_H

#include <QDialog>

// Forward declare native Qt widgets
class QFormLayout;

class MemFusionConfig : public QDialog {
    Q_OBJECT

public:
    explicit MemFusionConfig(QWidget *parent = nullptr);
    ~MemFusionConfig();

private slots:
    void updateStats();

private:
    QString readSystemFile(const QString &path);

    QFormLayout *m_formLayout;
};

#endif // MEMFUSIONCONFIG_H
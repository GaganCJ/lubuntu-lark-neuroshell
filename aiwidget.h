#ifndef AIWIDGET_H
#define AIWIDGET_H

#include <lxqtpanelplugin.h>
#include <QDialog>
#include <QFrame>
#include <QPushButton>
#include <QWebEngineView>

class AIChatWindow : public QDialog {
    Q_OBJECT
public:
    AIChatWindow(QWidget *parent = nullptr);

private slots:
    void switchAIProvider(const QString &provider);
    void purgeSessionData();

private:
    QFrame *m_container;
    QPushButton *m_geminiBtn;
    QPushButton *m_copilotBtn;
    QWebEngineView *m_webView;
};

#endif // AIWIDGET_H
#ifndef AIWIDGET_H
#define AIWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QDBusInterface>
#include <QDBusReply>

// Forward declare native Qt widgets
class QTextBrowser;

class AIChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit AIChatWindow(QWidget *parent = nullptr);

private slots:
    void sendPrompt();

private:
    QTextBrowser *m_textBrowser;
    QLineEdit *m_inputField;
};

#endif // AIWIDGET_H
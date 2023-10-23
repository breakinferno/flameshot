#pragma once
#include "src/tools/abstractactiontool.h"
#include <QNetworkReply>

class CopilotTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit CopilotTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    // bool needPainter() const override;
    // void paint(QPainter& painter, CaptureWidget& widget) override;
    CaptureTool* copy(QObject* parent = nullptr) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;
    void handleReply(QNetworkReply* reply);

private:
    void handleErrorReply(QNetworkReply::NetworkError error);
    QRect m_geometry;
    QPixmap m_pixmap;
    // CaptureWidget m_widget;
    // QPainter m_painter;
    void showBtn();
    static void generateCopilotMessage(QRect& offsetPoint);
};

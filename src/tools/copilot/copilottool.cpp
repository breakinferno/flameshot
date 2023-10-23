#include "copilottool.h"
#include "src/widgets/orientablepushbutton.h"
#include <QApplication>
#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPen>
#include <QScreen>
#include <QShortcut>
#include <QUrlQuery>
#include <confighandler.h>
#include <iostream>
#include <overlaymessage.h>

CopilotTool::CopilotTool(QObject* parent)
  : AbstractActionTool(parent)
{
    // m_painter = nullptr;
}

bool CopilotTool::closeOnButtonPressed() const
{
    return false;
}

QIcon CopilotTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "copilot.svg");
}
QString CopilotTool::name() const
{
    return tr("Copilot Tool");
}

CaptureTool::Type CopilotTool::type() const
{
    return CaptureTool::TYPE_COPILOT;
}

QString CopilotTool::description() const
{
    return tr("Use copilot to boost you productivity");
}

// bool CopilotTool::needPainter() const
// {
//     return true;
// }

// void CopilotTool::paint(QPainter& painter, CaptureWidget& widget) {}

CaptureTool* CopilotTool::copy(QObject* parent)
{
    return new CopilotTool(parent);
}

void CopilotTool::generateCopilotMessage(QRect& selection)
{
    QList<QPair<QString, QString>> keyMap;
    keyMap << QPair(tr("Mouse"), tr("Select screenshot area"));
    using CT = CaptureTool;
    keyMap << QPair(tr("Mouse Wheel"), tr("Change tool size"));
    keyMap << QPair(tr("Right Click"), tr("Show color picker"));
    keyMap << QPair(tr("Esc"), tr("Exit"));
    keyMap << QPair(
      tr("Long text Test"),
      tr("ExitExitExitExitExitExitExit ExitExitExitExitExitExitExit "));
    keyMap << QPair(tr("X"), tr(std::to_string(selection.x()).c_str()));
    keyMap << QPair(tr("Y"), tr(std::to_string(selection.y()).c_str()));
    keyMap << QPair(tr("X"), tr(std::to_string(selection.width()).c_str()));
    keyMap << QPair(tr("Y"), tr(std::to_string(selection.height()).c_str()));
    auto message = OverlayMessage::compileFromKeyMap(keyMap);
    // QLabel* label = new QLabel();
    // label->setText(message);
    // label->show();
    //
    //
    // QPainter painter;
    // QFontMetrics fm = painter.fontMetrics();
    // auto xybox = fm.boundingRect(message);
    // xybox.adjust(0, 0, 10, 12);
    // auto x0 = selection.left() + (selection.width() - xybox.width()) / 2;
    // auto y0 = selection.top() + (selection.height() - xybox.height()) / 2;
    // QColor uicolor = ConfigHandler().uiColor();
    // uicolor.setAlpha(200);
    // painter.fillRect(x0, y0, xybox.width(), xybox.height(), QBrush(uicolor));
    // painter.setPen(Qt::white);
    // painter.drawText(x0,
    //                 y0,
    //                 xybox.width(),
    //                 xybox.height(),
    //                 Qt::AlignVCenter | Qt::AlignHCenter,
    //                 message);

    QPainter painter;
    QFontMetrics fm = painter.fontMetrics();
    auto xybox = fm.boundingRect(message);

    QRect geom =
      QRect(selection.x() + selection.width(), selection.y(), 400, 400);

    OverlayMessage::forcePushIn(message, geom);

    // OverlayMessage::push(message);
}

void CopilotTool::showBtn()
{
    //        auto* panelToggleButton =
    //          new OrientablePushButton(tr("Tool Settings"), this);
    //        makeChild(panelToggleButton);
    //        panelToggleButton->setColor(m_uiColor);
    //        panelToggleButton->setOrientation(
    //          OrientablePushButton::VerticalBottomToTop);
    // #if defined(Q_OS_MACOS)
    //        panelToggleButton->move(
    //          0,
    //          static_cast<int>(panelRect.height() / 2) -
    //            static_cast<int>(panelToggleButton->width() / 2));
    // #else
    //        panelToggleButton->move(panelRect.x(),
    //                                panelRect.y() + panelRect.height() / 2 -
    //                                  panelToggleButton->width() / 2);
    // #endif
    //        panelToggleButton->setCursor(Qt::ArrowCursor);
    //        (new
    //        DraggableWidgetMaker(this))->makeDraggable(panelToggleButton);
    //        connect(panelToggleButton,
    //                &QPushButton::clicked,
    //                this,
    //                &CaptureWidget::togglePanel);
}

void CopilotTool::handleReply(QNetworkReply* reply)
{
    QLabel* label = new QLabel;
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        QJsonObject data = json[QStringLiteral("data")].toObject();

        QJsonObject console = json[QStringLiteral("console")].toObject();
        QJsonArray tables = json[QStringLiteral("tables")].toArray();

        foreach (const QJsonValue& table, tables) {
            qDebug() << "Hobby:" << table.toObject();
        }
        auto jsonStr =
          QString(QJsonDocument(json).toJson(QJsonDocument::Indented));
        label->setText(jsonStr);
    } else {
        qDebug() << "Network Error: " << reply->error();
        label->setText("error now!!!");
        // setInfoLabelText(reply->errorString());
    }

    label->show();
    // new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void CopilotTool::handleErrorReply(QNetworkReply::NetworkError error)
{
    qDebug() << "Network Error: " << error;
}

void CopilotTool::pressed(CaptureContext& context)
{
    generateCopilotMessage(context.selection);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QPixmap pix = context.selectedScreenshotArea();
    QImage image = pix.toImage();
    image.save("flamexxxxx.png", "png", 100);
    pix.save(&buffer, "PNG");
    auto const encoded = QString::fromLatin1(buffer.data().toBase64().data());

    QUrl url(
      QStringLiteral("https://screencopilot-api.azurewebsites.net/image"));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto manager = new QNetworkAccessManager(this);
    connect(manager,
            &QNetworkAccessManager::finished,
            this,
            &CopilotTool::handleReply);

    QJsonObject json;
    json.insert("Base64String", encoded);
    auto data = QJsonDocument(json).toJson();

    // auto jsonStr = QString(data);
    // QLabel* label = new QLabel;
    // label->setText(jsonStr);
    // label->show();

    // openssl 诊断代码
    qDebug() << "QT supported: " << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "OpenSSL supports?: " << QSslSocket::supportsSsl();
    qDebug() << "QT version?: " << QT_VERSION_STR;
    qDebug() << "OpenSSL runtime version: "
             << QSslSocket::sslLibraryBuildVersionString();

    QNetworkReply* reply = manager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [=] {
        if (reply->error()) {
            qDebug() << reply->errorString();
        } else {
            qDebug() << reply->readAll();
        }
    });
    // connect(reply,
    //         &QNetworkReply::error,
    //         this,
    //         &CopilotTool::handleErrorReply);
    //  while (!reply->isFinished()) {
    //      qApp->processEvents();
    //  }

    // QByteArray response_data = reply->readAll();
    // QJsonDocument response_json = QJsonDocument::fromJson(response_data);
    // reply->deleteLater();

    // emit requestAction(REQ_CLEAR_SELECTION);
    // emit requestAction(REQ_CAPTURE_DONE_OK);
    // context.request.addTask(CaptureRequest::COPILOT);
    // emit requestAction(REQ_CLOSE_GUI);
}

// bool CopilotTool::isSelectable() const
//{
//     return true;
// }
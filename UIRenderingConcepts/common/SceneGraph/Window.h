#pragma once

#include <QWindow>
#include <QTimer>
#include <QElapsedtimer>

class BaseRenderer;
class Window : public QWindow
{
    Q_OBJECT

public:
    Window(BaseRenderer* parent = NULL);
    ~Window() { delete renderTimer; }

    public slots:
    void Run();
    void resizeEvent(QResizeEvent* pEvent);
    virtual void mousePressEvent(QMouseEvent* p_Event);
    virtual void mouseReleaseEvent(QMouseEvent* p_Event);
    virtual void mouseMoveEvent(QMouseEvent* p_Event);

private:
    QTimer* renderTimer;    // Refresh timer
    BaseRenderer* m_VulkanApp; // Used to call run() by the timer
};

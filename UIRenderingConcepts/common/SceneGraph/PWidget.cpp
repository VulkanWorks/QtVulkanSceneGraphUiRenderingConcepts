#include "PWidget.h"

CustomWidget::CustomWidget(Scene *p_Scene, Node *p_Parent, const BoundingRegion &p_BoundedRegion, const QString &p_Name, QObject *p_ParentObj)
    : Node(p_Scene, p_Parent, p_BoundedRegion, p_Name, SHAPE::SHAPE_CUSTOM), QObject(p_ParentObj)
{
}

CustomWidget::~CustomWidget()
{
}


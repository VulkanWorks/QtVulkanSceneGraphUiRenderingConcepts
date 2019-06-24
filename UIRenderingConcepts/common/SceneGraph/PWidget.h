#pragma once

#include "Node.h"
class CustomWidget : public QObject, public Node
{
    Q_OBJECT

public:
    CustomWidget(Scene* p_Scene, Node* p_Parent, const BoundingRegion& p_BoundedRegion, const QString& p_Name = "", QObject* p_ParentObj = NULL);

    virtual ~CustomWidget();
};

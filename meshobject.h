/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include "pickobject.h"
#include <QtGui/qevent.h>

#include "qglpainter.h"
#include "qglabstractscene.h"

QT_BEGIN_NAMESPACE
class GLView;
class QGLSceneNode;
QT_END_NAMESPACE

class MeshObject : public PickObject
{
    Q_OBJECT
public:
    enum PickType { Normal, Anchor, Picked };

    explicit MeshObject(MeshObject *other, GLView *view = NULL, int id = -1);
    explicit MeshObject(QGLSceneNode *solidMesh, GLView *view = NULL, int id = -1);
    explicit MeshObject(QGLSceneNode *solidMesh, QGLSceneNode *animMesh, GLView *view = NULL, int id = -1);
    virtual ~MeshObject();

    QGLSceneNode *mesh() const { return m_solidMesh; }
    QGLSceneNode *solidMesh() const { return m_solidMesh; }
    QGLSceneNode *animMesh() const { return m_animMesh; }

    void setMesh(QGLSceneNode *solidMesh, QGLSceneNode *animMesh = NULL) {
        m_solidMesh = solidMesh;
        m_animMesh = animMesh;
    }

    void setMesh(MeshObject *other) {
        m_solidMesh = other->m_solidMesh;
        m_animMesh = other->m_animMesh;
    }

    void setAnimMesh(QGLSceneNode *animMesh) { m_animMesh = animMesh; }

    QVector3D position() const { return m_position; }
    void setPosition(const QVector3D &value) { m_position = value; }
    void setPosition(float x, float y, float z) { m_position = QVector3D(x, y, z); }

    float scale() const { return m_scale; }
    void setScale(float value) { m_scale = value; }
    void setScale(float x,float y,float z) {m_scaleX=x; m_scaleY=y; m_scaleZ=z;}

    float rotationAngle() const { return m_rotationAngle; }
    void setRotationAngle(float value) { m_rotationAngle = value; }

    QVector3D rotationVector() const { return m_rotationVector; }
    void setRotationVector(const QVector3D& value) { m_rotationVector = value; }

    float animAngle() const { return m_animAngle; }
    void setAnimAngle(float value) { m_animAngle = value; }

    QVector3D animVector() const { return m_animVector; }
    void setAnimVector(const QVector3D &value) { m_animVector = value; }
    void setAnimVector(float x, float y, float z) { m_animVector = QVector3D(x, y, z); }

    QVector3D animCenter() const { return m_animCenter; }
    void setAnimCenter(const QVector3D &value) { m_animCenter = value; }
    void setAnimCenter(float x, float y, float z) { m_animCenter = QVector3D(x, y, z); }

    PickType pickType() const { return m_type; }
    void setPickType(PickType value) { m_type = value; }

    void draw(QGLPainter *painter);

private:
    QGLSceneNode *m_solidMesh;
    QGLSceneNode *m_animMesh;
    QVector3D m_position;
    float m_scale;
    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;
    float m_rotationAngle;
    QVector3D m_rotationVector;
    float m_animAngle;
    QVector3D m_animVector;
    QVector3D m_animCenter;
    PickType m_type;
};

#endif

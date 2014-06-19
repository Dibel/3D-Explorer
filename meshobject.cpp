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

#include "meshobject.h"
#include "lib/glview.h"

#include <QtCore/QDebug>

MeshObject::MeshObject(MeshObject *other, GLView *view, int id) :
    PickObject(view, id), m_solidMesh(other->solidMesh()), m_animMesh(other->animMesh()),
    m_scale(1.0f), m_rotationAngle(0.0f),
    m_animAngle(other->animAngle()), m_animVector(other->animVector()), m_animCenter(other->animCenter()),
    m_type(Normal)
{ }

MeshObject::MeshObject(QGLSceneNode *meshObject, GLView *view, int id) :
    PickObject(view, id), m_solidMesh(meshObject), m_animMesh(NULL),
    m_scale(1.0f), m_rotationAngle(0.0f), m_animAngle(0.0f), m_type(Normal)
{ }

MeshObject::MeshObject(QGLSceneNode *scene, QGLSceneNode *anim, GLView *view, int id) :
    PickObject(view, id), m_solidMesh(scene), m_animMesh(anim),
    m_scale(1.0f), m_rotationAngle(0.0f), m_animAngle(0.0f), m_type(Normal)
{ }

MeshObject::~MeshObject() { }

void MeshObject::draw(QGLPainter *painter)
{
    //if (m_type == Anchor && !painter->isPicking()) return;
    //if (m_type == Picked && painter->isPicking()) return;

    // Position the model at its designated position, scale, and orientation.
    painter->modelViewMatrix().push();

    painter->modelViewMatrix().translate(m_position);

    if (m_scale != 1.0f)
        painter->modelViewMatrix().scale(m_scale);

    if (m_rotationAngle != 0.0f)
        painter->modelViewMatrix().rotate(m_rotationAngle, m_rotationVector);

    painter->setStandardEffect(QGL::LitMaterial);

    // Mark the object for object picking purposes.
    int prevObjectId = painter->objectPickId();
    if (objectId() != -1)
        painter->setObjectPickId(objectId());

    if (m_solidMesh)
        m_solidMesh->draw(painter);

    if (m_animMesh) {
        if (m_animAngle != 0.0f) {
            painter->modelViewMatrix().translate(m_animCenter);
            painter->modelViewMatrix().rotate(m_animAngle, m_animVector);
            painter->modelViewMatrix().translate(-m_animCenter);
        }
        m_animMesh->draw(painter);
    }

    // Revert to the previous object identifier.
    painter->setObjectPickId(prevObjectId);

    // Restore the modelview matrix.
    painter->modelViewMatrix().pop();
}

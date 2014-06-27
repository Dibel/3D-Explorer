#ifndef OUTLINEPAINTER_H
#define OUTLINEPAINTER_H

class QGLPainter;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLFramebufferObjectSurface;
class QOpenGLFramebufferObject;

class OutlinePainter {
public:
    OutlinePainter();
    void draw(QGLPainter *painter, int tex);

private:
    QGLSceneNode *node;
    QGLShaderProgramEffect *hblur, *vblur;
    QOpenGLFramebufferObject *fbo;
    QGLFramebufferObjectSurface *surface;
};

#endif

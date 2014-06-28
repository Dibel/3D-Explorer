#ifndef OUTLINEPAINTER_H
#define OUTLINEPAINTER_H

class QGLPainter;
class QGLSceneNode;
class QGLShaderProgramEffect;
class QGLFramebufferObjectSurface;
class QOpenGLFramebufferObject;

/**
 * \brief The painter of outline (glow) effect
 *
 * This class paints outline around hovering objects by following steps
 * (logically):
 *
 * 1. Take the picking buffer as a texture and make it transparent;
 *
 * 2. Detect the object (area) of given color and make it opaque;
 *
 * 3. Blur the alpha channel;
 *
 * 4. Make the detected area transparent;
 *
 * 5. Set the frame to single color (without changing alpha value);
 *
 * 6. Paint the frame at top of view.
 *
 * In practice, only two passes are needed. The first pass detects area,
 * sets the color, blur it horizontally and save the result into a FBO.
 * The second pass blur the FBO vertically, removes central area, and
 * paint it to screen. See the shaders for details.
 *
 * TODO: Currently the outline is not multisampled.
 */

class OutlinePainter {
public:
    /// Initialize outline painter.
    OutlinePainter();
    /// Take the picking buffer as @p texture, draw outline to @p painter.
    void draw(QGLPainter *painter, int texture);

private:
    QGLSceneNode *node;
    QGLShaderProgramEffect *hblur, *vblur;
    QOpenGLFramebufferObject *fbo;
    QGLFramebufferObjectSurface *surface;
};

#endif

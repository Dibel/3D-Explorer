#include "outlinepainter.h"
#include "common.h"
#include <Qt3D/QGLBuilder>
#include <Qt3D/QGLFramebufferObjectSurface>
#include <Qt3D/QGLShaderProgramEffect>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLShaderProgram>

OutlinePainter::OutlinePainter()
{
    QGLBuilder builder;
    builder.newSection(QGL::Faceted);
    builder.addPane(QSizeF(2, 2));
    node = builder.finalizedSceneNode();

    node->setMaterial(new QGLMaterial());

    hblur = new QGLShaderProgramEffect();
    hblur->setVertexShaderFromFile(":/shader/ortho.vsh");
    hblur->setFragmentShaderFromFile(":/shader/hblur.fsh");

    vblur = new QGLShaderProgramEffect();
    vblur->setVertexShaderFromFile(":/shader/ortho.vsh");
    vblur->setFragmentShaderFromFile(":/shader/vblur.fsh");

    /* Can't initialize fbo and surface now */
    fbo = NULL;
    surface = NULL;
}

void OutlinePainter::draw(QGLPainter *painter, int tex)
{
    if (!fbo) {
        fbo = new QOpenGLFramebufferObject(800, 600, QOpenGLFramebufferObject::CombinedDepthStencil);
        surface = new QGLFramebufferObjectSurface(fbo);
    }

    if (painter->isPicking() && paintingOutline == -1) return;

    /* Pass 1: detect target and horizontal blur, from picking buffer to fbo */

    // initialize fbo and surface
    painter->pushSurface(surface);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // initialize shader
    node->setUserEffect(hblur);
    painter->setUserEffect(hblur);
    hblur->program()->setUniformValue("target", hoveringPickColor);

    glEnable(GL_BLEND);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_TEXTURE_2D);

    node->draw(painter);

    // clean up
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    painter->setUserEffect(NULL);
    painter->popSurface();

    /* Pass 2: vertical blur and remove central area, from fbo to main buffer */

    // initialize shader
    node->setUserEffect(vblur);
    painter->setUserEffect(vblur);
    vblur->program()->setUniformValue("target", hoveringPickColor);

    glEnable(GL_BLEND);

    // bind texture 0 (h-blurred fbo)
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fbo->texture());
    // bind texture 1 (picking buffer)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex);

    node->draw(painter);

    // clean up
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);
    painter->setUserEffect(NULL);
}

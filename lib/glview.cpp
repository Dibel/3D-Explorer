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

#include "glview.h"
#include "qglsubsurface.h"
#include "glmaskedsurface.h"
#include "qglwindowsurface.h"
#include "gldrawbuffersurface.h"
#include "qray3d.h"
#include "qgltexture2d.h"

#include <QOpenGLFramebufferObject>
#include <QEvent>
#include <QMap>
#include <QGuiApplication>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QResizeEvent>
#include <QExposeEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>

QT_BEGIN_NAMESPACE

/*!
    \class GLView
    \brief The GLView class extends QGLWidget with support for 3D viewing.
    \since 4.8
    \ingroup qt3d
    \ingroup qt3d::viewing

    \section1 Navigating

    Navigation in 3D space is possible under keyboard and mouse control.

    Holding down the left mouse button and dragging it will rotate the
    camera() position around the object being viewed.  Holding down the
    left mouse button and the Shift key pans the view in a plane without
    rotating the viewed object.

    Using the mouse wheel, the view can be zoomed in or out.  If the
    system does not have a mouse wheel, then holding down the left mouse
    button and the Control key and moving the mouse up and down will
    also zoom in and out.

    On the keyboard, the left, right, up, and down keys can also be used
    to shift the camera() position around the object being viewed.  Shift
    and Control modify keys the same way they modify the left mouse
    button above.

    \section1 Stereo viewing support

    Note - Stereo viewing is experimental and unsupported.

    If the hardware supports stereo buffers, then each time the scene needs
    to be painted, GLView renders it twice: first from the perspective of
    the left eye, and then from the perspective of the right eye.
    The separation between the eye positions is specified by
    QGLCamera::eyeSeparation().  If the eye separation is zero,
    then stereo viewing is disabled and only a single image will
    be rendered per frame.

    Three kinds of stereo viewing are possible: hardware stereo,
    anaglyph stereo, and double image stereo.

    Hardware stereo relies upon specialized hardware that can render
    the left and right eye images into separate buffers and then show
    them independently to each eye through the use of polarized glasses
    or similar technology.  Hardware stereo is used if the \c{-stereo-hw}
    command-line option is supplied or if the user explicitly requests
    stereo buffers when the GLView is constructed:

    \code
    QGLFormat format(QGLFormat::defaultFormat());
    format.setOption(QGL::StereoBuffers);
    GLView view(format);
    \endcode

    Anaglyph stereo is used when the hardware doesn't have specialized
    stereo buffer support.  The left eye image is masked by a red
    filter and the right eye image is masked by a cyan filter.  This makes
    the resulting images suitable for viewing with standard red-cyan
    anaglyph glasses.

    When using red-cyan anaglyphs, it is recommended that the
    scene use non-primary object colors.  Pure primary colors such
    as red, green, and blue will only appear to one of the viewer's
    eyes and will inhibit the 3D effect.  Non-primary colors or
    grayscale should be used to get the best effects.

    Red-cyan anaglyphs can be disorienting to some viewers.  If hardware
    stereo is not available and stereo viewing is not critical to
    the application, then stereo can be disabled by setting
    QGLCamera::eyeSeparation() to zero.

    Double image stereo involves drawing the left and right eye
    images in a double-wide or double-high window, with the hardware
    combining the images.  Four different configurations are available:
    LeftRight, RightLeft, TopBottom,
    and BottomTop, according to the layout of the eye images.
    Double image stereo is selected by calling setStereoType().  It is
    the responsibility of the application to resize the window to
    twice its normal size to accommodate the images.

    Ctrl-Left and Ctrl-Right can be used to make the eye separation
    smaller or larger under keyboard control.

    A number of command-line options are available to select the
    stereo mode of the GLView so that the application does not
    need to select the mode itself:

    \table
    \row \li \c{-stereo-hw} \li \l Hardware.
    \row \li \c{-stereo-lr} \li LeftRight.
    \row \li \c{-stereo-rl} \li RightLeft.
    \row \li \c{-stereo-tb} \li TopBottom.
    \row \li \c{-stereo-bt} \li BottomTop.
    \row \li \c{-stereo-stretched-lr} \li StretchedLeftRight.
    \row \li \c{-stereo-stretched-rl} \li StretchedRightLeft.
    \row \li \c{-stereo-stretched-tb} \li StretchedTopBottom.
    \row \li \c{-stereo-stretched-bt} \li StretchedBottomTop.
    \endtable

    The option can also be supplied in the \c{QT3D_OPTIONS} environment
    variable:

    \code
    $ QT3D_OPTIONS="-stereo-lr" ./cubehouse
    \endcode

    If the application sets the stereo type with setStereoType(),
    that will be used.  Next is the command-line setting, and finally
    the contents of the environment variable.
*/

/*!
    \enum GLView::Option
    This enum defines an option for GLView.

    \value ObjectPicking Object picking is enabled.  Disabled by default.
    \value ShowPicking Objects are rendered with their pick colors instead
           of their normal colors and materials.  This can help debug
           problems with object picking.  Disabled by default.
    \value CameraNavigation Camera navigation using the keyboard and mouse
           is enabled.  Enabled by default.
    \omitvalue PaintingLog
    \value FOVZoom Enables zooming by changing field of view instead of
           physically moving the camera.
*/

/*!
    \enum GLView::StereoType
    This enum defines the type of stereo viewing technology being used by GLView.

    \value Hardware Specialized stereo hardware is being used.
    \value RedCyanAnaglyph Stereo is being simulated for viewing by
        red-cyan anaglyph classes.
    \value LeftRight The view is double-wide with the left eye
        image on the left of the window.
    \value RightLeft The view is double-wide with the left eye
        image on the right of the window.
    \value TopBottom The view is double-high with the left eye
        image on the top of the window.
    \value BottomTop The view is double-high with the left eye
        image on the bottom of the window.
    \value StretchedLeftRight Same as LeftRight, but with the
        left and right eye images stretched to double their width.
    \value StretchedRightLeft Same as RightLeft, but with the
        left and right eye images stretched to double their width.
    \value StretchedTopBottom Same as TopBottom, but with the
        left and right eye images stretched to double their height.
    \value StretchedBottomTop Same as BottomTop, but with the
        left and right eye images stretched to double their height.
*/

class GLViewPrivate
{
public:
    GLViewPrivate(GLView *parent)
        : view(parent)
        , context(0)
        , initialized(false)
        , mainSurface(parent)
        , visible(false)
    {
        options = GLView::CameraNavigation;
        fbo = 0;
        leftSurface = 0;
        rightSurface = 0;

        if (parent->format().stereo())
            stereoType = GLView::Hardware;
        else
            stereoType = GLView::RedCyanAnaglyph;

        pickBufferForceUpdate = true;
        pickBufferMaybeInvalid = true;
        updateQueued = false;

        pressedObject = 0;
        pressedButton = Qt::NoButton;
        enteredObject = 0;

        defaultCamera = new QGLCamera(parent);
        camera = defaultCamera;

        panning = false;
        startPan = QPoint(-1, -1);
        lastPan = QPoint(-1, -1);
        panModifiers = Qt::NoModifier;
        QObject::connect(defaultCamera, SIGNAL(projectionChanged()),
                         parent, SLOT(cameraChanged()));
        QObject::connect(defaultCamera, SIGNAL(viewChanged()),
                         parent, SLOT(cameraChanged()));

        logTime.start();
        lastFrameTime.start();
        QByteArray env = qgetenv("QT3D_LOG_EVENTS");
        if (env == "1")
            options |= GLView::PaintingLog;
    }
    ~GLViewPrivate()
    {
        delete fbo;
        delete leftSurface;
        delete rightSurface;
    }

    GLView *view;
    QOpenGLContext *context;
    QSurfaceFormat format;
    bool initialized;
    QRect viewport;
    GLView::Options options;
    GLView::StereoType stereoType;
    QOpenGLFramebufferObject *fbo;
    QGLWindowSurface mainSurface;
    bool visible;
    QGLAbstractSurface *leftSurface;
    QGLAbstractSurface *rightSurface;
    bool pickBufferForceUpdate;
    bool pickBufferMaybeInvalid;
    bool updateQueued;
    QMap<int, QObject *> objects;
    QObject *pressedObject;
    Qt::MouseButton pressedButton;
    QObject *enteredObject;
    QGLCamera *defaultCamera;
    QGLCamera *camera;
    bool panning;
    QPoint startPan;
    QPoint lastPan;
    QVector3D startEye;
    QVector3D startCenter;
    QVector3D startUpVector;
    Qt::KeyboardModifiers panModifiers;
    QTime logTime;
    QTime enterTime;
    QTime lastFrameTime;

    inline void logEnter(const char *message);
    inline void logLeave(const char *message);

    void processStereoOptions(GLView *view);
    void processStereoOptions(GLView *view, const QString &arg);

    QGLAbstractSurface *leftEyeSurface(const QSize &size);
    QGLAbstractSurface *rightEyeSurface(const QSize &size);
    QGLAbstractSurface *bothEyesSurface();

    void ensureContext();
};

inline void GLViewPrivate::logEnter(const char *message)
{
    if ((options & GLView::PaintingLog) == 0)
        return;
    int ms = logTime.elapsed();
    enterTime.start();
    int sinceLast = lastFrameTime.restart();
    qDebug("LOG[%d:%02d:%02d.%03d]: ENTER: %s (%d ms since last enter)",
           ms / 3600000, (ms / 60000) % 60,
           (ms / 1000) % 60, ms % 1000, message, sinceLast);
}

inline void GLViewPrivate::logLeave(const char *message)
{
    if ((options & GLView::PaintingLog) == 0)
        return;
    int ms = logTime.elapsed();
    int duration = enterTime.elapsed();
    qDebug("LOG[%d:%02d:%02d.%03d]: LEAVE: %s (%d ms elapsed)",
           ms / 3600000, (ms / 60000) % 60,
           (ms / 1000) % 60, ms % 1000, message, duration);
}

inline void GLViewPrivate::ensureContext()
{
    if (!context)
    {
        context = new QOpenGLContext();
        context->setFormat(format);
#ifndef QT_NO_DEBUG_STREAM
        QSurfaceFormat oldFormat = format;
#endif
        bool success = context->create();
        // TODO: is it possible that the platform will downgrade the actual
        // format, or will it just fail if it can't deliver the actual format
        format = context->format();
#ifndef QT_NO_DEBUG_STREAM
        if (!success)
            qWarning() << "Context was not successfully created";
        if (oldFormat != format)
            qWarning() << "Could not create requested format:\n"
                       << oldFormat << "\n\tgot format:\n\t" << format;
#endif
    }
    context->makeCurrent(view);
}

static QString qt_gl_stereo_arg()
{
    QStringList args = QGuiApplication::arguments();
    foreach (QString arg, args) {
        if (arg.startsWith(QLatin1String("-stereo-")))
            return arg;
    }
    QByteArray options(qgetenv("QT3D_OPTIONS"));
    args = QString::fromLocal8Bit
        (options.constData(), options.size()).split(QLatin1Char(' '));
    foreach (QString arg, args) {
        if (arg.startsWith(QLatin1String("-stereo-")))
            return arg;
    }
    return QString();
}

void GLViewPrivate::processStereoOptions(GLView *view)
{
    if (stereoType == GLView::Hardware)
        return;
    QString arg = qt_gl_stereo_arg();
    if (!arg.isEmpty())
        processStereoOptions(view, arg);
}

void GLViewPrivate::processStereoOptions(GLView *view, const QString &arg)
{
    // If the command-line contains an option that starts with "-stereo-",
    // then convert it into options that define the size and type of
    // stereo window to use for a top-level GLView.  Possible options:
    //
    //      hw - use hardware stereo
    //      lr, rl, tb, bt - specify the eye order (default is left-right)
    //      stretched - used stretched versions of double wide/high modes.
    //
    QStringList opts = arg.mid(8).split(QLatin1Char('-'));
    GLView::StereoType stereoType;
    bool stretched = opts.contains(QLatin1String("stretched"));
    if (opts.contains(QLatin1String("rl"))) {
        stereoType = stretched ? GLView::StretchedRightLeft : GLView::RightLeft;
    } else if (opts.contains(QLatin1String("tb"))) {
        stereoType = stretched ? GLView::StretchedTopBottom : GLView::TopBottom;
    } else if (opts.contains(QLatin1String("bt"))) {
        stereoType = stretched ? GLView::StretchedBottomTop : GLView::BottomTop;
    } else {
        stereoType = stretched ? GLView::StretchedLeftRight : GLView::LeftRight;
    }
    view->setStereoType(stereoType);
}

class GLViewSubsurface : public QGLSubsurface
{
public:
    GLViewSubsurface(QGLAbstractSurface *surface, const QRect &region,
                      float adjust)
        : QGLSubsurface(surface, region), m_adjust(adjust) {}

    float aspectRatio() const;

private:
    float m_adjust;
};

float GLViewSubsurface::aspectRatio() const
{
    return QGLSubsurface::aspectRatio() * m_adjust;
}

// Returns the surface to use to render the left eye image.
QGLAbstractSurface *GLViewPrivate::leftEyeSurface(const QSize &size)
{
    QRect viewport;
    float adjust = 1.0f;
    switch (stereoType) {
    case GLView::Hardware:
#if defined(GL_BACK_LEFT) && defined(GL_BACK_RIGHT)
        if (!leftSurface)
        {
            if (format.swapBehavior() == QSurfaceFormat::DoubleBuffer)
                leftSurface = new GLDrawBufferSurface(&mainSurface, GL_BACK_LEFT);
            else
                leftSurface = new GLDrawBufferSurface(&mainSurface, GL_FRONT_LEFT);
        }
        return leftSurface;
#endif
    case GLView::RedCyanAnaglyph:
        if (!leftSurface) {
            leftSurface = new GLMaskedSurface
                (&mainSurface,
                 GLMaskedSurface::RedMask | GLMaskedSurface::AlphaMask);
        }
        return leftSurface;
    case GLView::LeftRight:
        viewport = QRect(0, 0, size.width() / 2, size.height());
        break;
    case GLView::RightLeft:
        viewport = QRect(size.width() / 2, 0, size.width() / 2, size.height());
        break;
    case GLView::TopBottom:
        viewport = QRect(0, 0, size.width(), size.height() / 2);
        break;
    case GLView::BottomTop:
        viewport = QRect(0, size.height() / 2, size.width(), size.height() / 2);
        break;
    case GLView::StretchedLeftRight:
        viewport = QRect(0, 0, size.width() / 2, size.height());
        adjust = 2.0f;
        break;
    case GLView::StretchedRightLeft:
        viewport = QRect(size.width() / 2, 0, size.width() / 2, size.height());
        adjust = 2.0f;
        break;
    case GLView::StretchedTopBottom:
        viewport = QRect(0, 0, size.width(), size.height() / 2);
        adjust = 0.5f;
        break;
    case GLView::StretchedBottomTop:
        viewport = QRect(0, size.height() / 2, size.width(), size.height() / 2);
        adjust = 0.5f;
        break;
    }
    if (!leftSurface) {
        if (adjust == 1.0f)
            leftSurface = new QGLSubsurface(&mainSurface, viewport);
        else
            leftSurface = new GLViewSubsurface(&mainSurface, viewport, adjust);
    } else {
        static_cast<QGLSubsurface *>(leftSurface)->setRegion(viewport);
    }
    return leftSurface;
}

// Returns the surface to use to render the right eye image.
QGLAbstractSurface *GLViewPrivate::rightEyeSurface(const QSize &size)
{
    QRect viewport;
    float adjust = 1.0f;
    switch (stereoType) {
    case GLView::Hardware:
#if defined(GL_BACK_LEFT) && defined(GL_BACK_RIGHT)
        if (!rightSurface) {
            rightSurface = new GLDrawBufferSurface
                (&mainSurface,
                 format.swapBehavior() == QSurfaceFormat::DoubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT);
        }
        return rightSurface;
#endif
    case GLView::RedCyanAnaglyph:
        if (!rightSurface) {
            rightSurface = new GLMaskedSurface
                (&mainSurface,
                 GLMaskedSurface::GreenMask | GLMaskedSurface::BlueMask);
        }
        return rightSurface;
    case GLView::LeftRight:
        viewport = QRect(size.width() / 2, 0, size.width() / 2, size.height());
        break;
    case GLView::RightLeft:
        viewport = QRect(0, 0, size.width() / 2, size.height());
        break;
    case GLView::TopBottom:
        viewport = QRect(0, size.height() / 2, size.width(), size.height() / 2);
        break;
    case GLView::BottomTop:
        viewport = QRect(0, 0, size.width(), size.height() / 2);
        break;
    case GLView::StretchedLeftRight:
        viewport = QRect(size.width() / 2, 0, size.width() / 2, size.height());
        adjust = 2.0f;
        break;
    case GLView::StretchedRightLeft:
        viewport = QRect(0, 0, size.width() / 2, size.height());
        adjust = 2.0f;
        break;
    case GLView::StretchedTopBottom:
        viewport = QRect(0, size.height() / 2, size.width(), size.height() / 2);
        adjust = 0.5f;
        break;
    case GLView::StretchedBottomTop:
        viewport = QRect(0, 0, size.width(), size.height() / 2);
        adjust = 0.5f;
        break;
    }
    if (!rightSurface) {
        if (adjust == 1.0f)
            rightSurface = new QGLSubsurface(&mainSurface, viewport);
        else
            rightSurface = new GLViewSubsurface(&mainSurface, viewport, adjust);
    } else {
        static_cast<QGLSubsurface *>(rightSurface)->setRegion(viewport);
    }
    return rightSurface;
}

// Returns a surface that can be used to render a non-stereoscopic
// image into both eyes at the same time.  Returns null if the eyes
// must be rendered one at a time.
QGLAbstractSurface *GLViewPrivate::bothEyesSurface()
{
    switch (stereoType) {
    case GLView::Hardware:
#if defined(GL_BACK_LEFT) && defined(GL_BACK_RIGHT)
        return 0;
#endif
    case GLView::RedCyanAnaglyph:
        return &mainSurface;
    default:
        return 0;
    }
}

static QSurfaceFormat makeStereoGLFormat(const QSurfaceFormat& format)
{
    QSurfaceFormat fmt(format);
#if defined(GL_BACK_LEFT) && defined(GL_BACK_RIGHT)
    if (qt_gl_stereo_arg() == QLatin1String("-stereo-hw"))
        fmt.setOption(QSurfaceFormat::StereoBuffers);
#else
    qWarning("No option to clear stereo buffers");
#endif
    return fmt;
}

/*!
    Constructs a new view widget and attaches it to \a parent.

    This constructor will request a stereo rendering context if
    the hardware supports it (and the -stereo-hw option is set).
*/
GLView::GLView(QWindow *parent)
    : QWindow(parent)
{
    d = new GLViewPrivate(this);
    d->format = makeStereoGLFormat(QSurfaceFormat());
    d->format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    d->format.setDepthBufferSize(24);
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(d->format);
    // TODO: No mouse tracking available
    // setMouseTracking(true);
    if (!parent)
        d->processStereoOptions(this);
}

/*!
    Constructs a new view widget and attaches it to \a parent.
    The \a format argument specifies the desired QGLFormat
    rendering options.

    If \a format does not include the stereo option, then a stereo
    viewing context will not be requested.

    The format will be set onto the window, and also onto the underlying
    OpenGL context.
*/
GLView::GLView(const QSurfaceFormat& format, QWindow *parent)
    : QWindow(parent)
{
    d = new GLViewPrivate(this);
    d->format = format;
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(d->format);
    // TODO: No mouse tracking available
    // setMouseTracking(true);
    if (!parent)
        d->processStereoOptions(this);
}

/*!
    Destroys this view widget.
*/
GLView::~GLView()
{
    delete d;
}

/*!
    Returns the options for this view.  The default value is
    CameraNavigation.

    \sa setOptions(), setOption()
*/
GLView::Options GLView::options() const
{
    return d->options;
}

/*!
    Sets the options for this view to \a value.

    \sa options(), setOption()
*/
void GLView::setOptions(GLView::Options value)
{
    d->options = value;
}

/*!
    Enables or disables \a option according to \a value.

    \sa options(), setOptions()
*/
void GLView::setOption(GLView::Option option, bool value)
{
    if (value)
        d->options |= option;
    else
        d->options &= ~option;
}

/*!
    Returns the type of stereo viewing technology that is in use.

    \sa setStereoType()
*/
GLView::StereoType GLView::stereoType() const
{
    return d->stereoType;
}

/*!
    Sets the \a type of stereo viewing technology that is in use.
    The request takes effect at the next repaint.

    The request is ignored stereoType() or \a type is Hardware,
    because hardware stereo can only be enabled if the hardware
    supports it, and then it can never be disabled.

    \sa stereoType()
*/
void GLView::setStereoType(GLView::StereoType type)
{
    if (d->stereoType == Hardware || type == Hardware)
        return;
    if (d->stereoType == type)
        return;
    d->stereoType = type;

    // Destroy the current surface objects so that they will
    // be re-generated the next time we paint the widget.
    delete d->leftSurface;
    delete d->rightSurface;
    d->leftSurface = 0;
    d->rightSurface = 0;
}

/*!
    Registers an \a object with this view to be notified when
    \a objectId is selected with the mouse.  The \a object must
    persist for the lifetime of the GLView, or until
    deregisterObject() is called for \a objectId.

    \sa deregisterObject(), objectForPoint()
*/
void GLView::registerObject(int objectId, QObject *object)
{
    d->objects[objectId] = object;
}

/*!
    Deregisters the object associated with \a objectId.

    \sa registerObject()
*/
void GLView::deregisterObject(int objectId)
{
    d->objects.remove(objectId);
}

/*!
    Returns the camera parameters.  The camera defines the projection
    to apply to convert eye co-ordinates into window co-ordinates,
    and the position and orientation of the viewer's eye.

    \sa setCamera()
*/
QGLCamera *GLView::camera() const
{
    return d->camera;
}

/*!
    Sets the camera parameters to \a value.  The camera defines the
    projection to apply to convert eye co-ordinates into window
    co-ordinates, and the position and orientation of the viewer's eye.

    If \a value is null, then the default camera object will be used.

    This function will call update() to force the view to
    update with the new camera parameters upon the next event loop.

    \sa camera()
*/
void GLView::setCamera(QGLCamera *value)
{
    if (!value)
        value = d->defaultCamera;

    if (d->camera == value)
        return;

    disconnect(d->camera, SIGNAL(projectionChanged()),
               this, SLOT(cameraChanged()));
    disconnect(d->camera, SIGNAL(viewChanged()),
               this, SLOT(cameraChanged()));

    d->camera = value;

    connect(d->camera, SIGNAL(projectionChanged()),
            this, SLOT(cameraChanged()));
    connect(d->camera, SIGNAL(viewChanged()),
            this, SLOT(cameraChanged()));

    cameraChanged();
}

/*!
    Maps \a point from viewport co-ordinates to eye co-ordinates.

    The returned vector will have its x and y components set to the
    position of the point on the near plane, and the z component
    set to the inverse of the camera's near plane.

    This function is used for converting a mouse event's position
    into eye co-ordinates within the current camera view.

    \sa QGLCamera::mapPoint()
*/
QVector3D GLView::mapPoint(const QPoint &point) const
{
    QSize viewportSize(size());
    float aspectRatio;

    // Get the size of the underlying paint device.
    int width = viewportSize.width();
    int height = viewportSize.height();

    // TODO: previous implementations catered for non-square pixels

    // Derive the aspect ratio based on window size.
    if (width <= 0 || height <= 0)
        aspectRatio = 1.0f;
    else
        aspectRatio = float(width) / float(height);

    // Map the point into eye co-ordinates.
    return d->camera->mapPoint(point, aspectRatio, viewportSize);
}

void GLView::cameraChanged()
{
    // The pick buffer will need to be refreshed at the new camera position.
    d->pickBufferForceUpdate = true;

    // Queue an update for the next event loop.

    update();
}

void GLView::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);
    d->visible = true;
    d->ensureContext();
    if (!d->initialized)
        initializeGL();
}

void GLView::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    d->visible = false;
}

void GLView::exposeEvent(QExposeEvent *e)
{
    Q_UNUSED(e);

    d->updateQueued = false;
    d->ensureContext();
    if (!d->initialized)
        initializeGL();

    paintGL();

    d->context->swapBuffers(this);
}

void GLView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    QRect r = geometry();
    Q_ASSERT(e->size() == r.size());
    if (r.size() != d->viewport.size())
    {
        d->ensureContext();
        if (!d->initialized)
            initializeGL();
        resizeGL(r.width(), r.height());
        d->viewport = r;
    }
}

/*!
    \internal
*/
void GLView::initializeGL()
{
    d->logEnter("GLView::initializeGL");
    QGLPainter painter;
    painter.begin();

    // Set the default depth buffer options.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
#if defined(QT_OPENGL_ES)
    glDepthRangef(0.0f, 1.0f);
#else
    glDepthRange(0.0f, 1.0f);
#endif

    // Set the default blend options.
    if (painter.hasOpenGLFeature(QOpenGLFunctions::BlendColor))
        painter.glBlendColor(0, 0, 0, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (painter.hasOpenGLFeature(QOpenGLFunctions::BlendEquation))
        painter.glBlendEquation(GL_FUNC_ADD);
    else if (painter.hasOpenGLFeature(QOpenGLFunctions::BlendEquationSeparate))
        painter.glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    glDisable(GL_CULL_FACE);
    QRect r = geometry();
    resizeGL(r.width(), r.height());
    initializeGL(&painter);
    d->initialized = true;
    d->logLeave("GLView::initializeGL");
}

/*!
    \internal
*/
void GLView::resizeGL(int w, int h)
{
    // Set up the standard viewport for the new window size.
    glViewport(0, 0, w, h);


    // We will need to regenerate the pick buffer.
    d->pickBufferForceUpdate = true;
}

/*!
    \internal
*/
void GLView::paintGL()
{
    d->logEnter("GLView::paintGL");

    QGLTexture2D::processPendingResourceDeallocations();

    // We may need to regenerate the pick buffer on the next mouse event.
    d->pickBufferMaybeInvalid = true;

    // Paint the scene contents.
    QGLPainter painter;
    QGLAbstractSurface *surface;
    painter.begin();
    if (d->options & GLView::ShowPicking &&
            d->stereoType == GLView::RedCyanAnaglyph) {
        // If showing picking, then render normally.  This really
        // only works if we aren't using hardware or double stereo.
        painter.setPicking(true);
        painter.clearPickObjects();
        painter.setEye(QGL::NoEye);
        earlyPaintGL(&painter);
        painter.setCamera(d->camera);
        paintGL(&painter);
        painter.setPicking(false);
    } else if (d->camera->eyeSeparation() == 0.0f &&
               (surface = d->bothEyesSurface()) != 0) {
        // No camera separation, so render the same image into both buffers.
        painter.pushSurface(surface);
        painter.setEye(QGL::NoEye);
        earlyPaintGL(&painter);
        painter.setCamera(d->camera);
        paintGL(&painter);
        painter.popSurface();
    } else {
        // Paint the scene twice, from the perspective of each camera.
        QSize size(this->size());
        painter.setEye(QGL::LeftEye);
        if (d->stereoType != GLView::Hardware)
            earlyPaintGL(&painter);     // Clear both eyes at the same time.
        painter.pushSurface(d->leftEyeSurface(size));
        if (d->stereoType == GLView::Hardware)
            earlyPaintGL(&painter);     // Clear the left eye only.
        earlyPaintGL(&painter);
        painter.setCamera(d->camera);
        paintGL(&painter);
        if (d->stereoType == GLView::RedCyanAnaglyph)
            glClear(GL_DEPTH_BUFFER_BIT);
        painter.setEye(QGL::RightEye);
        painter.setSurface(d->rightEyeSurface(size));
        if (d->stereoType == GLView::Hardware)
            earlyPaintGL(&painter);     // Clear the right eye only.
        painter.setCamera(d->camera);
        paintGL(&painter);
        painter.popSurface();
    }
    d->logLeave("GLView::paintGL");
}

void GLView::update()
{
    if (!d->updateQueued)
    {
        d->updateQueued = true;
        QGuiApplication::postEvent(this, new QExposeEvent(geometry()));
    }
}

/*!
    Initializes the current GL context represented by \a painter.

    \sa paintGL()
*/
void GLView::initializeGL(QGLPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Performs early painting operations just after \a painter
    is initialized but before the camera is set up.  The default
    implementation clears the color buffer and depth buffer.

    This function is typically overridden to draw scene backdrops
    on the color buffer before the rest of the scene is drawn
    by paintGL().

    \sa paintGL()
*/
void GLView::earlyPaintGL(QGLPainter *painter)
{
    Q_UNUSED(painter);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*!
    \fn void GLView::paintGL(QGLPainter *painter)

    Paints the scene onto \a painter.  The color and depth buffers
    will have already been cleared, and the camera() position set.

    If QGLPainter::isPicking() is set for \a painter, then the
    function should paint the scene onto \a painter in
    "object picking mode".  The scene will be rendered into a
    background buffer using flat colors so that mouse events
    can determine which object lies under the mouse pointer.

    The default implementation of picking will typically just
    render the scene normally.  However, some applications
    may wish to render a simpler scene that omits unselectable
    objects and uses simpler meshes for the selectable objects.

    The base default implementation does nothing.  Sub-classes should
    re-implement this function to paint GL content.

    \sa earlyPaintGL()
*/
void GLView::paintGL(QGLPainter *painter)
{
    Q_UNUSED(painter);
}

/*!
    Processes the mouse press event \a e.
*/
void GLView::mousePressEvent(QMouseEvent *e)
{
    QObject *object;
    if (!d->panning && (d->options & GLView::ObjectPicking) != 0)
        object = objectForPoint(e->pos());
    else
        object = 0;
    if (d->pressedObject) {
        // Send the press event to the pressed object.  Use a position
        // of (0, 0) if the mouse is still within the pressed object,
        // or (-1, -1) if the mouse is no longer within the pressed object.
        QMouseEvent event
            (QEvent::MouseButtonPress,
             (d->pressedObject == object) ? QPoint(0, 0) : QPoint(-1, -1),
             e->globalPos(), e->button(), e->buttons(), e->modifiers());
        QCoreApplication::sendEvent(d->pressedObject, &event);
    } else if (object) {
        // Record the object that was pressed and forward the event.
        d->pressedObject = object;
        d->enteredObject = 0;
        d->pressedButton = e->button();

        // Send a mouse press event for (0, 0).
        QMouseEvent event(QEvent::MouseButtonPress, QPoint(0, 0),
                          e->globalPos(), e->button(), e->buttons(),
                          e->modifiers());
        QCoreApplication::sendEvent(object, &event);
    } else if ((d->options & GLView::CameraNavigation) != 0 &&
                    e->button() == Qt::LeftButton) {
        d->panning = true;
        d->lastPan = d->startPan = e->pos();
        d->startEye = d->camera->eye();
        d->startCenter = d->camera->center();
        d->startUpVector = d->camera->upVector();
        d->panModifiers = e->modifiers();
#ifndef QT_NO_CURSOR
        // TODO: not supported under QWindow
        //setCursor(Qt::ClosedHandCursor);
#endif
    }
    QWindow::mousePressEvent(e);
}

/*!
    Processes the mouse release event \a e.
*/
void GLView::mouseReleaseEvent(QMouseEvent *e)
{
    if (d->panning && e->button() == Qt::LeftButton) {
        d->panning = false;
#ifndef QT_NO_CURSOR
        // TODO: unsetCursor not support with QWindow
        //unsetCursor();
#endif
    }
    if (d->pressedObject) {
        // Notify the previously pressed object about the release.
        QObject *object = objectForPoint(e->pos());
        QObject *pressed = d->pressedObject;
        if (e->button() == d->pressedButton) {
            d->pressedObject = 0;
            d->pressedButton = Qt::NoButton;
            d->enteredObject = object;

            // Send the release event to the pressed object.  Use a position
            // of (0, 0) if the mouse is still within the pressed object,
            // or (-1, -1) if the mouse is no longer within the pressed object.
            QMouseEvent event
                (QEvent::MouseButtonRelease,
                 (pressed == object) ? QPoint(0, 0) : QPoint(-1, -1),
                 e->globalPos(), e->button(), e->buttons(), e->modifiers());
            QCoreApplication::sendEvent(pressed, &event);

            // Send leave and enter events if necessary.
            if (object != pressed) {
                sendLeaveEvent(pressed);
                if (object)
                    sendEnterEvent(object);
            }
        } else {
            // Some other button than the original was released.
            // Forward the event to the pressed object.
            QMouseEvent event
                (QEvent::MouseButtonRelease,
                 (pressed == object) ? QPoint(0, 0) : QPoint(-1, -1),
                 e->globalPos(), e->button(), e->buttons(), e->modifiers());
            QCoreApplication::sendEvent(pressed, &event);
        }
    }
    QWindow::mouseReleaseEvent(e);
}

/*!
    Processes the mouse double click event \a e.
*/
void GLView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if ((d->options & GLView::ObjectPicking) != 0) {
        QObject *object = objectForPoint(e->pos());
        if (object) {
            // Simulate a double click event for (0, 0).
            QMouseEvent event
                (QEvent::MouseButtonDblClick, QPoint(0, 0),
                 e->globalPos(), e->button(), e->buttons(), e->modifiers());
            QCoreApplication::sendEvent(object, &event);
        }
    }
    QWindow::mouseDoubleClickEvent(e);
}

/*!
    Processes the mouse move event \a e.
*/
void GLView::mouseMoveEvent(QMouseEvent *e)
{
    if (d->panning) {
        QPoint delta = e->pos() - d->startPan;
        if (e->modifiers() == d->panModifiers) {
            d->camera->setEye(d->startEye);
            d->camera->setCenter(d->startCenter);
            d->camera->setUpVector(d->startUpVector);
        } else {
            d->startPan = d->lastPan;
            delta = e->pos() - d->startPan;
            d->startEye = d->camera->eye();
            d->startCenter = d->camera->center();
            d->startUpVector = d->camera->upVector();
            d->panModifiers = e->modifiers();
        }
        d->lastPan = e->pos();
        if ((e->modifiers() & Qt::ControlModifier) != 0)
            wheel(delta.y() * -60);
        else if ((e->modifiers() & Qt::ShiftModifier) != 0)
            pan(delta.x(), delta.y());
        else
            rotate(delta.x(), delta.y());
    } else if ((d->options & GLView::ObjectPicking) != 0) {
        QObject *object = objectForPoint(e->pos());
        if (d->pressedObject) {
            // Send the move event to the pressed object.  Use a position
            // of (0, 0) if the mouse is still within the pressed object,
            // or (-1, -1) if the mouse is no longer within the pressed object.
            QMouseEvent event
                (QEvent::MouseMove,
                 (d->pressedObject == object) ? QPoint(0, 0) : QPoint(-1, -1),
                 e->globalPos(), e->button(), e->buttons(), e->modifiers());
            QCoreApplication::sendEvent(d->pressedObject, &event);
        } else if (object) {
            if (object != d->enteredObject) {
                if (d->enteredObject)
                    sendLeaveEvent(d->enteredObject);
                d->enteredObject = object;
                sendEnterEvent(d->enteredObject);
            }
            QMouseEvent event
                (QEvent::MouseMove, QPoint(0, 0),
                 e->globalPos(), e->button(), e->buttons(), e->modifiers());
            QCoreApplication::sendEvent(object, &event);
        } else if (d->enteredObject) {
            sendLeaveEvent(d->enteredObject);
            d->enteredObject = 0;
        }
    }
    QWindow::mouseMoveEvent(e);
}

#ifndef QT_NO_WHEELEVENT

/*!
    Processes the wheel event \a e.
*/
void GLView::wheelEvent(QWheelEvent *e)
{
    if ((d->options & GLView::CameraNavigation) != 0)
        wheel(e->delta());
    QWindow::wheelEvent(e);
}

#endif

/*!
    Processes the key press event \a e.
*/
void GLView::keyPressEvent(QKeyEvent *e)
{
    QGLCamera *camera;
    float sep;

    // process the key only if we're doing camera navigation, or we
    // have received a Quit action
    if ((d->options & GLView::CameraNavigation) ||
            e->key() == Qt::Key_Escape || e->key() == Qt::Key_Q)
    {
        switch (e->key())
        {
        case Qt::Key_Escape:
        case Qt::Key_Q:
        {
            emit quit();
        }
        break;

        case Qt::Key_Left:
        {
            if ((e->modifiers() & Qt::ShiftModifier) != 0) {
                pan(-10, 0);
            } else if ((e->modifiers() & Qt::ControlModifier) != 0) {
                camera = this->camera();
                sep = camera->eyeSeparation();
                sep -= (sep / 10.0f);
                if (sep < 0.0f)
                    sep = 0.0f;
                camera->setEyeSeparation(sep);
                e->accept();
                return;
            } else {
                rotate(-10, 0);
            }
        }
        break;

        case Qt::Key_Right:
        {
            if ((e->modifiers() & Qt::ShiftModifier) != 0) {
                pan(10, 0);
            } else if ((e->modifiers() & Qt::ControlModifier) != 0) {
                camera = this->camera();
                sep = camera->eyeSeparation();
                sep += (sep / 10.0f);
                camera->setEyeSeparation(sep);
                e->accept();
                return;
            } else {
                rotate(10, 0);
            }
        }
        break;

        case Qt::Key_Up:
        {
            if ((e->modifiers() & Qt::ControlModifier) != 0)
                wheel(120);
            else if ((e->modifiers() & Qt::ShiftModifier) != 0)
                pan(0, -10);
            else
                rotate(0, -10);
        }
        break;

        case Qt::Key_Down:
        {
            if ((e->modifiers() & Qt::ControlModifier) != 0)
                wheel(-120);
            else if ((e->modifiers() & Qt::ShiftModifier) != 0)
                pan(0, 10);
            else
                rotate(0, 10);
        }
        break;
        }
    }
    QWindow::keyPressEvent(e);
}

class GLViewPickSurface : public QGLAbstractSurface
{
public:
    GLViewPickSurface(GLView *view, QOpenGLFramebufferObject *fbo,
                       const QSize &areaSize);

    QPaintDevice *device() const;
    bool activate(QGLAbstractSurface *prevSurface);
    void deactivate(QGLAbstractSurface *nextSurface);
    QRect viewportGL() const;

private:
    GLView *m_view;
    QOpenGLFramebufferObject *m_fbo;
    QRect m_viewportGL;
};

GLViewPickSurface::GLViewPickSurface
        (GLView *view, QOpenGLFramebufferObject *fbo, const QSize &areaSize)
    : QGLAbstractSurface(504)
    , m_view(view)
    , m_fbo(fbo)
    , m_viewportGL(QPoint(0, 0), areaSize)
{
}

bool GLViewPickSurface::activate(QGLAbstractSurface *prevSurface)
{
    Q_UNUSED(prevSurface);
    if (m_fbo)
        m_fbo->bind();
    return true;
}

void GLViewPickSurface::deactivate(QGLAbstractSurface *nextSurface)
{
    Q_UNUSED(nextSurface);
    if (m_fbo)
        m_fbo->release();
}

QRect GLViewPickSurface::viewportGL() const
{
    return m_viewportGL;
}

/*!
    Returns the registered object that is under the mouse position
    specified by \a point.  This function may need to regenerate
    the contents of the pick buffer by repainting the scene
    with paintGL().

    \sa registerObject()
*/
QObject *GLView::objectForPoint(const QPoint &point)
{
    return d->objects.value(objectIdForPoint(point), 0);
}

int GLView::objectIdForPoint(const QPoint &point)
{
    QPoint pt(point);

    // What is the size of the drawing area after correcting for stereo?
    // Also adjust the mouse position to always be in the left half.
    const QSize areaSize = size();

    // Check the area boundaries in case a mouse move has
    // moved the pointer outside the window.
    if (pt.x() < 0 || pt.x() >= areaSize.width() ||
            pt.y() < 0 || pt.y() >= areaSize.height())
        return 0;

    // Do we need to refresh the pick buffer contents?
    QGLPainter painter(this);
    if (d->pickBufferForceUpdate) {
        // Initialize the painter, which will make the window context current.
        painter.setPicking(true);
        painter.clearPickObjects();

        // Create a framebuffer object as big as the window to act
        // as the pick buffer if we are single buffered.  If we are
        // double-buffered, then use the window back buffer.
        QSize fbosize = QGL::nextPowerOfTwo(areaSize);
        if (!d->fbo) {
            d->fbo = new QOpenGLFramebufferObject(fbosize, QOpenGLFramebufferObject::CombinedDepthStencil);
        } else if (d->fbo->size() != fbosize) {
            delete d->fbo;
            d->fbo = new QOpenGLFramebufferObject(fbosize, QOpenGLFramebufferObject::CombinedDepthStencil);
        }

        // Render the pick version of the scene.
        GLViewPickSurface surface(this, d->fbo, areaSize);
        painter.pushSurface(&surface);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        painter.setEye(QGL::NoEye);
        painter.setCamera(d->camera);
        paintGL(&painter);
        painter.setPicking(false);
        painter.popSurface();

        // The pick buffer contents are now valid, unless we are using
        // the back buffer - we cannot rely upon it being valid next time.
        d->pickBufferForceUpdate = false;
        d->pickBufferMaybeInvalid = false;
    }

    // Pick the object under the mouse.
    if (d->fbo)
        d->fbo->bind();
    int objectId = painter.pickObject(pt.x(), areaSize.height() - 1 - pt.y());
    //QObject *object = d->objects.value(objectId, 0);
    if (d->fbo)
        d->fbo->release();

    // Release the framebuffer object and return.
    painter.end();
    return objectId;
}

void GLView::sendEnterEvent(QObject *object)
{
    QEvent event(QEvent::Enter);
    QCoreApplication::sendEvent(object, &event);
}

void GLView::sendLeaveEvent(QObject *object)
{
    QEvent event(QEvent::Leave);
    QCoreApplication::sendEvent(object, &event);
}

// Zoom in and out according to the change in wheel delta.
void GLView::wheel(int delta)
{
    if (d->options & GLView::FOVZoom) {     
        //Use field-of view as zoom (much like a traditional camera)
        float scale = qAbs(viewDelta(delta, delta).x());
        if (delta < 0)
            scale = -scale;
        if (scale >= 0.0f)
            scale += 1.0f;
        else
            scale = 1.0f / (1.0f - scale);
        float fov = d->camera->fieldOfView();
        if (fov != 0.0f)
            d->camera->setFieldOfView(d->camera->fieldOfView() / scale);
        else
            d->camera->setViewSize(d->camera->viewSize() / scale);
    } else {
        // enable this to get wheel navigation that actually zooms by moving the
        // camera back, as opposed to making the angle of view wider.        
        QVector3D viewVector= camera()->eye() - camera()->center();
        float zoomMag = viewVector.length();
        float zoomIncrement = -float(delta) / 100.0f;
        if (!qFuzzyIsNull(zoomIncrement))
        {
            zoomMag += zoomIncrement;
            if (zoomMag < 1.0f)
                zoomMag = 1.0f;

            QRay3D viewLine(camera()->center(), viewVector.normalized());
            camera()->setEye(viewLine.point(zoomMag));
        }
    }

}

// Pan left/right/up/down without rotating about the object.
void GLView::pan(int deltax, int deltay)
{
    QPointF delta = viewDelta(deltax, deltay);
    QVector3D t = d->camera->translation(delta.x(), -delta.y(), 0.0f);

    // Technically panning the eye left should make the object appear to
    // move off to the right, but this looks weird on-screen where the user
    // actually thinks they are picking up the object and dragging it rather
    // than moving the eye.  We therefore apply the inverse of the translation
    // to make it "look right".
    d->camera->setEye(d->camera->eye() - t);
    d->camera->setCenter(d->camera->center() - t);
}

// Rotate about the object being viewed.
void GLView::rotate(int deltax, int deltay)
{
    int rotation = d->camera->screenRotation();
    if (rotation == 90 || rotation == 270) {
        qSwap(deltax, deltay);
    }
    if (rotation == 90 || rotation == 180) {
        deltax = -deltax;
    }
    if (rotation == 180 || rotation == 270) {
        deltay = -deltay;
    }
    float anglex = deltax * 90.0f / width();
    float angley = deltay * 90.0f / height();
    QQuaternion q = d->camera->pan(-anglex);
    q *= d->camera->tilt(-angley);
    d->camera->rotateCenter(q);
}

/*!
    Converts \a deltax and \a deltay into percentages of the
    view width and height.  Returns a QPointF containing the
    percentage values, typically between -1 and 1.

    This function is typically used by subclasses to convert a
    change in mouse position into a relative distance travelled
    across the field of view.

    The returned value is corrected for the camera() screen
    rotation and view size.
*/
QPointF GLView::viewDelta(int deltax, int deltay) const
{
    int w = width();
    int h = height();
    bool scaleToWidth;
    float scaleFactor, scaleX, scaleY;
    QSizeF viewSize = d->camera->viewSize();
    if (w >= h) {
        if (viewSize.width() >= viewSize.height())
            scaleToWidth = true;
        else
            scaleToWidth = false;
    } else {
        if (viewSize.width() >= viewSize.height())
            scaleToWidth = false;
        else
            scaleToWidth = true;
    }
    int rotation = d->camera->screenRotation();
    if (rotation == 90 || rotation == 270) {
        scaleToWidth = !scaleToWidth;
        qSwap(deltax, deltay);
    }
    if (rotation == 90 || rotation == 180) {
        deltax = -deltax;
    }
    if (rotation == 180 || rotation == 270) {
        deltay = -deltay;
    }
    if (scaleToWidth) {
        scaleFactor = 2.0f / viewSize.width();
        scaleX = scaleFactor * float(h) / float(w);
        scaleY = scaleFactor;
    } else {
        scaleFactor = 2.0f / viewSize.height();
        scaleX = scaleFactor;
        scaleY = scaleFactor * float(w) / float(h);
    }
    return QPointF(deltax * scaleX / w, deltay * scaleY / h);
}

/*!
    \fn QPointF GLView::viewDelta(const QPoint &delta) const
    \overload

    Converts the x and y components of \a delta into percentages
    of the view width and height.  Returns a QPointF containing
    the percentage values, typically between -1 and 1.
*/

/*!
    Returns the OpenGL context object associated with this view, or null
    if one has not been associated yet.  The default is null.
*/
QOpenGLContext *GLView::context()
{
    return d->context;
}

/*!
    Returns true if the view is visible, that is its show event has been called
    and was not followed by a hide event.  Note that this cannot be relied apon
    for the visibility of the window, as it depends on whether hide and show
    events are triggered, which is platform dependent.
*/
bool GLView::isVisible() const
{
    return d->visible;
}

QT_END_NAMESPACE

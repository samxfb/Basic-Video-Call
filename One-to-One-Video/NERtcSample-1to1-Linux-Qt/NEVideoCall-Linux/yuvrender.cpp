#include "yuvrender.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#define VERTEXIN 0
#define TEXTUREIN 1

YuvRender::YuvRender(QWidget *parent):
    QOpenGLWidget(parent)
{

}

YuvRender::~YuvRender()
{
    makeCurrent();
    vbo.destroy();
    if (textureY) {
        textureY->destroy();
    }
    if (textureU) {
        textureU->destroy();
    }
    if (textureV) {
        textureV->destroy();
    }
    doneCurrent();
}

void YuvRender::showYuv(std::vector<unsigned char> &data_y_, std::vector<unsigned char> &data_u_, std::vector<unsigned char> &data_v_, uint width, uint height)
{
    data_y = data_y_;
    data_u = data_u_;
    data_v = data_v_;
    if (video_w != width || video_h != height) {
        int dst_x = 0;
        int dst_y = 0;
        int dst_w = 0;
        int dst_h = 0;
        // resize opengl widget - mode: kNERtcVideoScaleFit
        double wnd_ratio = show_w * 1.0 / show_h;
        double texture_ratio = width * 1.0 / height;
        if (wnd_ratio > texture_ratio) {
            int rendering_box_width = show_h * width / height;
            dst_w = rendering_box_width;
            dst_h = show_h;
            dst_x = show_x + (show_w - rendering_box_width) / 2;
            dst_y = show_y;
        } else {
            int rendering_box_height = show_w * height / width;
            dst_w = show_w;
            dst_h = rendering_box_height;
            dst_x = show_x;
            dst_y = show_y + (show_h - rendering_box_height) / 2;
        }
        setGeometry(dst_x, dst_y, dst_w, dst_h);

        video_w = width;
        video_h = height;
    }
    update();
}

void YuvRender::initializeGL()
{
    show_x = geometry().x();
    show_y = geometry().y();
    show_w = geometry().width();
    show_h = geometry().height();

    initializeOpenGLFunctions();
//    glEnable(GL_DEPTH_TEST);

    static const GLfloat vertices[]{
        //顶点坐标
        -1.0f, -1.0f,
        -1.0f, +1.0f,
        +1.0f, +1.0f,
        +1.0f, -1.0f,
        //纹理坐标
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertices,sizeof(vertices));

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex,this);
    const char *vsrc =
   "attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment,this);
    const char *fsrc = "varying vec2 textureOut;\
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertexIn", VERTEXIN);
    program->bindAttributeLocation("textureIn", TEXTUREIN);
    program->link();
    program->bind();
    program->enableAttributeArray(VERTEXIN);
    program->enableAttributeArray(TEXTUREIN);
    program->setAttributeBuffer(VERTEXIN, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    program->setAttributeBuffer(TEXTUREIN,GL_FLOAT, 8 * sizeof(GLfloat), 2, 2 * sizeof(GLfloat));

    textureUniformY = program->uniformLocation("tex_y");
    textureUniformU = program->uniformLocation("tex_u");
    textureUniformV = program->uniformLocation("tex_v");
    textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureY->create();
    textureU->create();
    textureV->create();
    idY = textureY->textureId();
    idU = textureU->textureId();
    idV = textureV->textureId();
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void YuvRender::paintGL()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // fix render abnormal when resolution is special, such as 1226*784

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, idY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_w, video_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data_y.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, idU);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_w >> 1, video_h >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data_u.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,idV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, video_w >> 1, video_h >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data_v.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(textureUniformY, 0);
    glUniform1i(textureUniformU, 1);
    glUniform1i(textureUniformV, 2);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

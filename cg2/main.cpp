#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "ObjLoader.h"

const int NumCrvVertices = 1024;

int Index = 0;

GLfloat BezierBasis[NumCrvVertices][4];

/* Bezier Curve */
struct BezierCurve {
    //vec2 BndPos[2];
    //vec2 BndTan[2];
    vec3 CtrlPts[4];

    vec3 points[NumCrvVertices + 4];
    vec3 colors[NumCrvVertices + 3];

    GLuint vao;
    GLuint vbo;

    BezierCurve() {
    }
    BezierCurve(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3)
    {
        CtrlPts[0] = p0;
        CtrlPts[1] = p1;
        CtrlPts[2] = p2;
        CtrlPts[3] = p3;
    }

    ~BezierCurve() {

    }

    vec2 evaulate(const int i)
    {
        vec2 pts;
        for (int j = 0; j < 2; j++)
            pts = BezierBasis[i][0] * CtrlPts[0][j]
            + BezierBasis[i][1] * CtrlPts[1][j]
            + BezierBasis[i][2] * CtrlPts[2][j]
            + BezierBasis[i][3] * CtrlPts[3][j];
        return pts;
    }

    void evalulate() {
        for (int i = 0; i < NumCrvVertices; i++) {
            for (int j = 0; j < 3; j++)
                points[i][j] = BezierBasis[i][0] * CtrlPts[0][j]
                + BezierBasis[i][1] * CtrlPts[1][j]
                + BezierBasis[i][2] * CtrlPts[2][j]
                + BezierBasis[i][3] * CtrlPts[3][j];
        }
    }
    void updateForRendering() {
        evalulate();

        // for drawing tangent handles
        points[NumCrvVertices] = CtrlPts[3];
        points[NumCrvVertices + 1] = CtrlPts[2];
        points[NumCrvVertices + 2] = CtrlPts[1];
        points[NumCrvVertices + 3] = CtrlPts[0];
    }
};

BezierCurve curve;
int crv_edit_handle = -1;

// precompute polynomial bases for Bezier spline
void precomputeBezierBasis()
{
    GLfloat t, t2, t3;
    for (int i = 0; i < NumCrvVertices; i++) {
        t = i * 1.0 / (GLfloat)(NumCrvVertices - 1.0);
        t2 = t * t;
        t3 = 1 - t;

        BezierBasis[i][0] = t3 * t3 * t3;
        BezierBasis[i][1] = 3.0 * t3 * t3 * t;
        BezierBasis[i][2] = 3.0 * t3 * t * t;
        BezierBasis[i][3] = t * t * t;
    }
}

// bazier curve

//viewing transformation parameters

GLfloat radius = 1.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

const GLfloat dr = 5.0 * DegreesToRadians;

GLuint bunny_view; // model-view matrix uniform shader variable location
GLuint bunny_model;

// Projection transformation parameters
GLfloat fovy = 45.0; //field-of-view in y direction angle (in degrees)
GLfloat aspect; //Viewport aspect ratio
GLfloat zNear = 0.1, zFar = 10.0;

GLuint bunny_projection; //projection matrix uniform shader variable location
GLuint color_loc;

class TriMesh
{
public:
    int NumVertices;
    int NumTris;

    GLuint vao;
    GLuint vbo;
    GLuint ebo; // element buffer object for indices

    //vec4* vertices;
    //vec4 vertices[8];
    std::vector<vec4> vertices;

    vec3 color;
    mat4 modelTransform;

    //unsigned int* indices;
    //unsigned int indices[6];
    std::vector<unsigned int> indices;

    TriMesh() {
        NumVertices = 0;
        NumTris = 0;
        //vertices = NULL;
        //indices = NULL;
    }

    ~TriMesh() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);

        //if (vertices != NULL)
         //   delete[] vertices;
        //if (indices != NULL)
          //  delete[] indices;
    }

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glBindVertexArray(0);
    }

    void Render() {
        //set a constant color
        glUniform3fv(color_loc, 1, color);
        glUniformMatrix4fv(bunny_model, 1, GL_TRUE, modelTransform);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};


int MakeCylinder(TriMesh* mesh, GLfloat radius, GLfloat height, int nR, int nV)
{
    if (nR < 2 || nV < 1)
        return -1;

    GLfloat deltaR = 2.0 * M_PI / (GLfloat)nR;
    GLfloat deltaV = height / (GLfloat)nV;

    mesh->NumVertices = (nR + 1) * (nV + 1);
    mesh->NumTris = nR * nV * 2;

    mesh->vertices.resize(mesh->NumVertices);
    mesh->indices.resize(mesh->NumTris * 3);

    for (int j = 0; j <= nV; j++) {
        for (int i = 0; i <= nR; i++) {
            vec3 pt;
            pt.x = radius * cos(deltaR * (GLfloat)i);
            pt.y = radius * sin(deltaR * (GLfloat)i);
            pt.z = deltaV * (GLfloat)j;
            mesh->vertices[(nR + 1) * j + i] = vec4(pt, 1.0);
        }
    }

    for (int j = 0; j < nV; j++) {
        for (int i = 0; i < nR; i++) {
            int quadId = nR * j + i;
            mesh->indices[6 * quadId] = (nR + 1) * j + i;
            mesh->indices[6 * quadId + 1] = (nR + 1) * (j + 1) + i;
            mesh->indices[6 * quadId + 2] = (nR + 1) * j + i + 1;
            mesh->indices[6 * quadId + 3] = (nR + 1) * (j + 1) + i;
            mesh->indices[6 * quadId + 4] = (nR + 1) * (j + 1) + i + 1;
            mesh->indices[6 * quadId + 5] = (nR + 1) * j + i + 1;
        }
    }
    return 0;
}

int Width;
int Height;

int HitIndex(BezierCurve* curve, int x, int y)
{
    GLfloat left = -1.0, right = 1.0;
    GLfloat bottom = -1.0, top = 1.0;

    int ret = -1;
    GLfloat dist[3], mindist = FLT_MAX;

    // a size of one pixel in the curve coordinate system.
    vec2 pixelLen((right - left) / (GLfloat)(Width), (top - bottom) / (GLfloat)Height);

    // the current mouse point in the curve coordinate
    vec2 mousePt, tmpVec;
    mousePt.x = left + pixelLen[0] * (GLfloat)x;
    mousePt.y = bottom + pixelLen[1] * (GLfloat)y;

    // Measure the squared distance between mouse point and handles
    for (int i = 0; i < 3; i++) {
        tmpVec = curve->CtrlPts[i] - mousePt;
        dist[i] = dot(tmpVec, tmpVec);
    }
    //tmpVec = curve->BndPos[0] - mousePt;
    //dist[0] = dot(tmpVec, tmpVec);
    //tmpVec = curve->BndPos[1] - mousePt;
    //dist[1] = dot(tmpVec, tmpVec);

    //tmpVec = (curve->BndPos[0] + curve->BndTan[0]) - mousePt;
    //dist[2] = dot(tmpVec, tmpVec);
    //tmpVec = (curve->BndPos[1] + curve->BndTan[1]) - mousePt;
    //dist[3] = dot(tmpVec, tmpVec);

    for (int i = 0; i < 3; i++) {
        if (mindist > dist[i]) {
            ret = i;
            mindist = dist[i];
        }
    }

    // if clicked within 10 pixel radius of one of handles then return
    if (mindist < 100.0 * dot(pixelLen, pixelLen))
        return ret;
    else
        return -1;
}

//TriMesh Cylinder;
//TriMesh Cylinder2;
TriMesh bunny;

GLuint curve_model, curve_view, curve_projection;


void init(void)
{
    CObjLoader* loader = new CObjLoader();
    loader->Load("bunny.obj", NULL);

    bunny.NumVertices = loader->vertexes.size();
    bunny.NumTris = loader->parts[0].faces.size();

    bunny.vertices.resize(bunny.NumVertices);
    vec4 center(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < bunny.NumVertices; i++) {
        tVertex vtx = loader->getVertex(i);
        vec4 v(vtx.x, vtx.y, vtx.z, 1.0);
        center = center + v;
        bunny.vertices[i] = v;
    }

    center.x = center.x / bunny.NumVertices;
    center.y = center.y / bunny.NumVertices;
    center.z = center.z / bunny.NumVertices;
    center.w = 1.0;

    bunny.indices.resize(bunny.NumTris * 3);
    for (int i = 0; i < bunny.NumTris; i++) {
        tFace* face = &(loader->parts[0].faces[i]);

        for (int j = 0; j < 3; j++)
            bunny.indices[3 * i + j] = (unsigned int)(face->v[j] - 1);
    }
    delete loader;


    //MakeCylinder(&Cylinder, 1.0, 1.0, 12, 2);
    bunny.color = vec3(0.2, 0.3, 0.5);
    // bunny.modelTransform = Translate(0, 0, 0) * RotateX(45.0) * Scale(0.5, 0.5, 1.2);
    bunny.modelTransform = Translate(-center.x, -center.y, -center.z) * Scale(6.0);

    /*MakeCylinder(&Cylinder2, 0.5, 1.0, 20, 2);
    Cylinder2.color = vec3(1.0, 0.0, 0.0);*/

    GLuint program = InitShader("vshader_multishape.glsl", "fshader_multishape.glsl");
    glUseProgram(program);

    bunny.init();
    //Cylinder2.init();

    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);

    //buffer object
    //GLuint buffer, ebo;
    //glGenBuffers(1, &buffer);
    //glGenBuffers(1, &ebo);

    //glBindBuffer(GL_ARRAY_BUFFER, buffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * Cylinder.vertices.size(), Cylinder.vertices.data(), GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Cylinder.indices.size(), Cylinder.indices.data(), GL_STATIC_DRAW);

    //load shaders
    //GLuint program = InitShader("vshader_multishape.glsl", "fshader_multishape.glsl");
    //glUseProgram(program);

    //initialize vertex position attribute from vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //initialize uniform variable from vertex shander
    bunny_model = glGetUniformLocation(program, "model");
    bunny_view = glGetUniformLocation(program, "view");
    bunny_projection = glGetUniformLocation(program, "projection");
    color_loc = glGetUniformLocation(program, "segColor");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    // bazier curve

    precomputeBezierBasis();

    curve = BezierCurve(vec3(-1.0, 0.0, 3.0), vec3(-2.0, 0.0, 3.5), vec3(-3.0, 0.0, 4.0), vec3(-4.0, 0.0, 4.5));

    curve.updateForRendering();

    //for colors
    for (int i = 0; i < NumCrvVertices; i++)
        curve.colors[i] = vec3(0.0, 0.0, 0.0);
    curve.colors[NumCrvVertices] = vec3(1.0, 0.0, 0.0);
    curve.colors[NumCrvVertices + 1] = vec3(1.0, 0.0, 0.0);
    curve.colors[NumCrvVertices + 2] = vec3(1.0, 0.0, 0.0);

    glGenVertexArrays(1, &(curve.vao));
    glBindVertexArray(curve.vao);

    glGenBuffers(1, &(curve.vbo));
    glBindBuffer(GL_ARRAY_BUFFER, curve.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(curve.points) + sizeof(curve.colors), NULL,
        GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.points), curve.points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve.points), sizeof(curve.colors), curve.colors);
    // 
    //load shaders
    glUseProgram(program);

    //initialize vertex position attribute from vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.points)));

    //initialize uniform variable from vertex shander
    curve_model = glGetUniformLocation(program, "model");
    curve_view = glGetUniformLocation(program, "view");
    curve_projection = glGetUniformLocation(program, "projection");

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

GLfloat left = -1.0,right = 1.0;
GLfloat bottom = -1.0, top = 1.0;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, 512, 512);

    vec4 eye0(-1.5, 2.0 , -1.5,
        1.0);
    vec4 eye1(-3.0, 0, 7.0, 1.0);
    vec4 at(0.0, 0.0, 0.0, 1.0);
    vec4 at0(0.0, -5.0, 0.0, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);
    vec4 up0(-1.0, 0.0, 0.0, 0.0);

    mat4 vmat0 = LookAt(eye0, at0, up0);
    mat4 vmat1 = LookAt(eye1, at, up);

    glUniformMatrix4fv(bunny_view, 1, GL_TRUE, vmat0);

    mat4 p0 = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(bunny_projection, 1, GL_TRUE, p0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //Cylinder.Render();
    bunny.Render();

    mat4 p = Ortho2D(left, right, bottom, top);
    glUniformMatrix4fv(curve_projection, 1, GL_TRUE, p);

    glBindVertexArray(curve.vao);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.points), curve.points);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_STRIP, 0, NumCrvVertices + 3);
    glBindVertexArray(0);

    glViewport(512, 0, 512, 512);

    glUniformMatrix4fv(bunny_view, 1, GL_TRUE, vmat1);

    mat4 p1 = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(bunny_projection, 1, GL_TRUE, p1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //Cylinder.Render();
    bunny.Render();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //Cylinder2.Render();
    glutSwapBuffers();
}

void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033:
        exit(EXIT_SUCCESS);
        break;
    case 'z': zNear *= 1.1; zFar *= 1.1; break;
    case 'Z': zNear *= 0.9; zFar *= 0.9; break;
    case 'r': radius *= 2.0; break;
    case 'R': radius *= 0.5; break;
    case 'o': theta += dr; break;
    case 'O': theta -= dr; break;
    case 'p': phi += dr; break;
    case 'P': phi -= dr; break;

    case ' ':  // reset values to their defaults
        zNear = 0.5;
        zFar = 3.0;

        radius = 1.0;
        theta = 0.0;
        phi = 0.0;
        break;
    }
    glutPostRedisplay();
}

void mouse(GLint button, GLint action, GLint x, GLint y)
{
    if (GLUT_LEFT_BUTTON == button)
    {
        if (GLUT_LEFT_BUTTON == button)
        {
            switch (action)
            {
            case GLUT_DOWN:
                crv_edit_handle = HitIndex(&curve, x, Height - y);
                break;
            case GLUT_UP:
                crv_edit_handle = -1;
                break;
            default: break;
            }
        }
        glutPostRedisplay();
    }
}

void mouseMove(GLint x, GLint z)
{

    if (crv_edit_handle != -1) {
        vec2 pixelLen((right - left) / (GLfloat)(Width), (top - bottom) / (GLfloat)Height);
        vec3 mousePt;
        mousePt.x = left + (GLfloat)x * pixelLen[0];
        mousePt.z = bottom + (GLfloat)(Height - z) * pixelLen[1];

        curve.CtrlPts[crv_edit_handle] = mousePt;

        /*if (crv_edit_handle < 2) {
            curve.BndPos[crv_edit_handle] = mousePt;
        }
        else {
            curve.BndTan[crv_edit_handle - 2] = mousePt - curve.BndPos[crv_edit_handle - 2];
        }*/
        curve.updateForRendering();

        glutPostRedisplay();
    }
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = GLfloat(width) / height;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 512);

    aspect = 512.0 / 512.0;

    glutInitContextVersion(4, 3);
    //In case of MacOS
    //glutInitContextProfie(GLUT_CORE_PROFILE);
    glutCreateWindow("Drawing MultiShapes");

    glewInit();
    init();

    /*
    printf("OpenGL %s, GLSL %s\n",
        glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));
    */

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);

    glutMainLoop();

    return 0;
}
// rotating cube with two texture objects
// change textures with 1 and 2 keys

#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef vec4 point4;
typedef vec4 color4;

// Texture objects and storage for texture image
GLuint textures[3];

std::vector<vec4> points;
std::vector<vec3> normals;
std::vector<vec2> texCoords;
std::vector<int> indices;


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint ModelView, Projection;
GLfloat fovy = 45.0;
GLfloat aspect;
GLfloat zNear = 0.1, zFar = 10.0;
//----------------------------------------------------------------------------

int Index = 0;


//----------------------------------------------------------------------------

void createSphere(GLfloat radius, int NumSectors, int NumStacks)
{
    GLfloat x, y, z, xy;

    GLfloat stackStep = M_PI / (GLfloat)NumStacks;
    GLfloat sectorStep = 2.0 * M_PI / (GLfloat)NumSectors;

    // compute vertices
    for (int i = 0; i <= NumStacks; i++) {
        vec4 pt;
        vec3 nor;
        vec2 tex;

        float stackAngle = M_PI_2 - (GLfloat)i * stackStep;
        xy = cosf(stackAngle);
        z = sinf(stackAngle);

        for (int j = 0; j <= NumSectors; j++) {
            float sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            pt = vec4(radius * x, radius * y, radius * z, 1.0);
            nor = vec3(x, y, z);
            tex = vec2((GLfloat)j / NumSectors, (GLfloat)i / NumStacks);

            points.push_back(pt);
            normals.push_back(nor);
            texCoords.push_back(tex);
        }
    }

    //create indices
    // k1 - k1 + 1
    // |   /   |
    // k2 - k2 + 1
    for (int i = 0; i < NumStacks; i++) {
        int k1 = i * (NumSectors + 1);
        int k2 = k1 + NumSectors + 1;
        for (int j = 0; j < NumSectors; j++, k1++, k2++) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != NumStacks - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}


//----------------------------------------------------------------------------
void
init()
{
    createSphere(0.5, 18, 36);


    // Initialize texture objects
    glGenTextures(3, textures);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    int texWidth, texHeight, texChannels;
    unsigned char* data = stbi_load("earth_basic.jpg", &texWidth, &texHeight, &texChannels, 0);

    std::cout << texChannels << "\n";
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load earth_basic.jpg\n";
    }
    stbi_image_free(data);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = stbi_load("cloud.jpg", &texWidth, &texHeight, &texChannels, 0);
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load cloud.jpg\n";
    }
    stbi_image_free(data);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    data = stbi_load("earth_terrain.jpg", &texWidth, &texHeight, &texChannels, 0);
    if (data) {

        //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
            //	  GL_RGB, GL_UNSIGNED_BYTE, image );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Fail to load earth_terrain.jpg\n";
    }
    stbi_image_free(data);

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and initialize a buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(vec4) * points.size() +
        sizeof(vec3) * normals.size() +
        sizeof(vec2) * texCoords.size(),
        NULL, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
        indices.data(), GL_STATIC_DRAW);

    // Specify an offset to keep track of where we're placing data in our
    //   vertex array buffer.  We'll use the same technique when we
    //   associate the offsets with vertex attribute pointers.
    GLintptr offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec4) * points.size(),
        points.data());
    offset += sizeof(vec4) * points.size();

    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * normals.size(),
        normals.data());
    offset += sizeof(vec3) * normals.size();

    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * texCoords.size(),
        texCoords.data());

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader_texturemapping4.glsl", "fshader_texturemapping4.glsl");
    glUseProgram(program);

    // set up vertex arrays
    offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(vec4) * points.size();

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(vec3) * normals.size();

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));


    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    // Set the value of the fragment shader texture sampler variable
    //   ("texture") to the the appropriate texture unit. In this case,
    //   zero, for GL_TEXTURE0 which was previously set by calling
    //   glActiveTexture().
    glUniform1i(glGetUniformLocation(program, "mainTex"), 0);
    glUniform1i(glGetUniformLocation(program, "cloudTex"), 1);
    glUniform1i(glGetUniformLocation(program, "terrainTex"), 2);

    //theta = glGetUniformLocation( program, "theta" );

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glUniform3fv( theta, 1, Theta );

    const vec3 viewer_pos(0.0, 0.0, 2.0);
    mat4  model_view = (Translate(-viewer_pos) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    mat4 p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p);

    //glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
        case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
        case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------

void
idle(void)
{
    Theta[Axis] += 0.01;

    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int mousex, int mousey)
{
    switch (key) {
    case 033: // Escape Key
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
        //case '1':
          //  glBindTexture( GL_TEXTURE_2D, textures[0] );
            //break;

        //case '2':
          //  glBindTexture( GL_TEXTURE_2D, textures[1] );
            //break;
    }

    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = GLfloat(width) / height;
}
//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);

    aspect = 512.0 / 512.0;

    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Color Cube");

    glewInit();

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}

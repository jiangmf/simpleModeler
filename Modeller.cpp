/*
    COMPSCI 3GC3 Assignment 3
    Name: Mingfei Jiang
    Student Number: 1320376

    Additional features: Additional model loaded via obj loader, texture mapping

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <memory>
#include <limits>
#include <fstream>
using namespace std;

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

/* TEXTURE */
GLubyte* image;
int twidth, theight, tmax;
GLuint myTex[1];

GLubyte* LoadPPM(char* file, int* width, int* height, int* max)
{
    GLubyte* img;
    FILE *fd;
    int n, m;
    int  k, nm;
    char c;
    int i;
    char b[100];
    float s;
    int red, green, blue;
    
    /* first open file and check if it's an ASCII PPM (indicated by P3 at the start) */
    fd = fopen(file, "r");
    fscanf(fd,"%[^\n] ",b);
    if(b[0]!='P'|| b[1] != '3')
    {
        exit(0);
    }
    fscanf(fd, "%c",&c);
    
    /* next, skip past the comments - any line starting with #*/
    while(c == '#')
    {
        fscanf(fd, "%[^\n] ", b);
        fscanf(fd, "%c",&c);
    }
    ungetc(c,fd);
    
    /* now get the dimensions and max colour value from the image */
    fscanf(fd, "%d %d %d", &n, &m, &k);
    
    /* calculate number of pixels and allocate storage for this */
    nm = n*m;
    img = (GLubyte*)malloc(3*sizeof(GLuint)*nm);
    s=255.0/k;
    
    /* for every pixel, grab the read green and blue values, storing them in the image data array */
    for(i=0;i<nm;i++)
    {
        fscanf(fd,"%d %d %d",&red, &green, &blue );
        img[3*nm-3*i-3]=red*s;
        img[3*nm-3*i-2]=green*s;
        img[3*nm-3*i-1]=blue*s;
    }
    
    /* finally, set the "return parameters" (width, height, max) and return the image array */
    *width = n;
    *height = m;
    *max = k;
    
    return img;
}

/**********************
  STRUCTURES & CLASSES
***********************/
// Vertex/Vector, 3 floats representing 
struct Vertex{
    float x;
    float y;
    float z;
};
// a face, consisting of the indexes of vectors, texture coordinates, and normals
struct Face{
    int v1;
    int v2;
    int v3;
    int t1;
    int t2;
    int t3;
    int n1;
    int n2;
    int n3;
};
// Bounding box, defined by 1 min vertex and 1 max vertex
struct Bound{
    Vertex minV;
    Vertex maxV;
};

// material, defined by ambient, diffuse, specular, emission, and shiny values
class Material {
private:
    vector<float> mat_amb; 
    vector<float> mat_dif;
    vector<float> mat_spe;
    vector<float> mat_emi;
    float mat_shi;
public:
    Material(
        float amb1, float amb2, float amb3, float amb4,
        float dif1, float dif2, float dif3, float dif4,
        float spe1, float spe2, float spe3, float spe4,
        float emi1, float emi2, float emi3, float emi4,
        float shi
    ) {

        mat_amb.insert(mat_amb.end(), {amb1, amb2, amb3, amb4});
        mat_dif.insert(mat_dif.end(), {dif1, dif2, dif3, dif4});
        mat_spe.insert(mat_spe.end(), {spe1, spe2, spe3, spe4});
        mat_emi.insert(mat_emi.end(), {emi1, emi2, emi3, emi4});
        mat_shi = shi;
    }
    // set the current material to this material
    void set(void){
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  &mat_amb[0]);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  &mat_dif[0]);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &mat_spe[0]);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &mat_emi[0]);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shi);
    }

    bool operator==(const Material& rhs) const {
        return
           mat_amb == rhs.mat_amb
           && mat_dif == rhs.mat_dif
           && mat_spe == rhs.mat_spe
           && mat_emi == rhs.mat_emi
           && mat_shi == rhs.mat_shi
        ;
    }
};

// define the materials
Material defaultMaterial = Material(
    0.20f, 0.20f, 0.20f, 1.00f,
    0.80f, 0.80f, 0.80f, 1.00f,
    0.00f, 0.00f, 0.00f, 1.00f,
    0.0f,  0.0f,  0.0f,  0.0f,
    0.00f
);

Material floorMaterial = Material(
    0.19225f, 0.19225f, 0.19225f, 1.0f,
    0.50754f, 0.50754f, 0.50754f, 1.0f,
    0.508273f, 0.508273f, 0.508273f, 1.0f,
    0,0,0,0,
    51.2f
);

Material yellowMaterial = Material(
    1.00f, 0.92f, 0.23f, 1.00f,
    0.75f, 0.61f, 0.23f, 1.00f,
    0.63f, 0.56f, 0.37f, 1.00f,
    0.0f,  0.0f,  0.0f,  0.0f,
    51.2f
);
Material blueMaterial = Material(
    0.25f, 0.32f, 0.71f, 0.55f,
    0.10f, 0.14f, 0.49f, 0.55f,
    0.77f, 0.79f, 0.91f, 0.55f,
    0.0f,  0.0f,  0.0f,  0.0f,
    76.8f
);
Material redMaterial = Material(
    0.96f, 0.26f, 0.21f, 0.55f,
    0.61f, 0.04f, 0.04f, 0.55f,
    0.73f, 0.63f, 0.63f, 0.55f,
    0.0f,  0.0f,  0.0f,  0.0f,
    76.8f
);

Material greenMaterial = Material(
    0.0215f,  0.1745f,   0.0215f,  0.55f,
    0.07568f, 0.61424f,  0.07568f, 0.55f,
    0.633f,   0.727811f, 0.633f,   0.55f,
    0.0f,     0.0f,      0.0f,     0.0f,
    76.8f
);

Material blackMaterial = Material(
    0.02f, 0.02f, 0.02f, 1.0f,
    0.01f, 0.01f, 0.01f, 1.0f,
    0.4f,  0.4f,  0.4f,  1.0f,
    0.0f,  0.0f,  0.0f,  0.0f,
    10.0f
);

Material pureCyanMaterial = Material(
    0.0f,  1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  1.0f,  1.0f,
    76.8f
);

// a model loaded by an obj file
class Model{
private:
public:
    vector<Vertex> vertexes; 
    vector<Vertex> normals;
    vector<Vertex> textureCordinates;
    vector<Face> faces;
    string file_name;
    Model(string fname){
        file_name = fname;
        load();
    }
    // parse the obj file and load it into 
    void load(void){
        FILE * file = fopen(file_name.c_str(), "r");
        while(true){
            char lineHeader[128];
            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break; // EOF = End Of File. Quit the loop.
            // parse vertex
            if (strcmp( lineHeader, "v" )==0){
                Vertex vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
                vertexes.push_back(vertex);
            // parse texture coordinate
            }else if ( strcmp( lineHeader, "vt" ) == 0 ){
                Vertex textureCordinate;
                fscanf(file, "%f %f\n", &textureCordinate.x, &textureCordinate.y );
                textureCordinates.push_back(textureCordinate);
            // parse normals
            }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                Vertex normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
                normals.push_back(normal);
            // parse faces
            }else if ( strcmp( lineHeader, "f" ) == 0 ){
                Face face;
                int temp[9];
                char line[128];
                fgets (line , 128 , file);
                
                int matches = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &temp[0],&temp[1],&temp[2],&temp[3],&temp[4],&temp[5],&temp[6],&temp[7],&temp[8]);

                face.v1 = temp[0] - 1;
                face.t1 = temp[1] - 1;
                face.n1 = temp[2] - 1;
                face.v2 = temp[3] - 1;
                face.t2 = temp[4] - 1;
                face.n2 = temp[5] - 1;
                face.v3 = temp[6] - 1;
                face.t3 = temp[7] - 1;
                face.n3 = temp[8] - 1;

                faces.push_back(face);
            // eat up the rest of the line
            }else{
                char buffer[1000];
                fgets(buffer, 1000, file);
            }
        }
    }
    // draw the loaded model
    void draw(void){
        for(int i=0; i<faces.size(); i++){
            glBegin(GL_POLYGON);
                Face face = faces[i];
                glTexCoord2f(textureCordinates[face.n1].x,  textureCordinates[face.n1].y);
                glNormal3f( normals[face.n1].x,  normals[face.n1].y,  normals[face.n1].z);
                glVertex3f(vertexes[face.v1].x, vertexes[face.v1].y, vertexes[face.v1].z);
                glTexCoord2f(textureCordinates[face.n2].x,  textureCordinates[face.n2].y);
                glNormal3f( normals[face.n2].x,  normals[face.n2].y,  normals[face.n2].z);
                glVertex3f(vertexes[face.v2].x, vertexes[face.v2].y, vertexes[face.v2].z);
                glTexCoord2f(textureCordinates[face.n3].x,  textureCordinates[face.n3].y);
                glNormal3f( normals[face.n3].x,  normals[face.n3].y,  normals[face.n3].z);
                glVertex3f(vertexes[face.v3].x, vertexes[face.v3].y, vertexes[face.v3].z);
            glEnd();
        }
    }
};
// load the model
Model keyModel = Model("key2.obj");

// an object to be drawn in the scene
class Object{
private:
    // the bounding vertexes of the bounding box
    vector<Vertex> boundingVertexes;
public:
    // what kind of object it is (teapot, cube, etc)
    char type;
    Material material;
    Vertex position;
    Vertex orientation;
    Vertex scale;
    // if the object is selected then draw the bounding box
    bool selected;
    // if the object has a texture
    bool texture = false;
    // constructor, set to the specified type and set everything else to default
    Object(char t):
    material(defaultMaterial),
    type(t){
        // place the object at the the origin
        Vertex pos = {0,1,0};
        Vertex ori = {0,0,0};
        Vertex sca = {1,1,1};
        position = pos;
        orientation = ori;
        scale = sca;
        // set the bounding box depending on what type of object this is
        switch(type){
            case '!':
                boundingVertexes.push_back((Vertex){-1.5, -1, -1});
                boundingVertexes.push_back((Vertex){-1.5, -1,  1});
                boundingVertexes.push_back((Vertex){-1.5,  1, -1});
                boundingVertexes.push_back((Vertex){-1.5,  1,  1});
                boundingVertexes.push_back((Vertex){ 1.6, -1, -1});
                boundingVertexes.push_back((Vertex){ 1.6, -1,  1});
                boundingVertexes.push_back((Vertex){ 1.6,  1, -1});
                boundingVertexes.push_back((Vertex){ 1.6,  1,  1});
                break;
            case '@':
                boundingVertexes.push_back((Vertex){-1, -1, -1});
                boundingVertexes.push_back((Vertex){-1, -1,  1});
                boundingVertexes.push_back((Vertex){-1,  1, -1});
                boundingVertexes.push_back((Vertex){-1,  1,  1});
                boundingVertexes.push_back((Vertex){ 1, -1, -1});
                boundingVertexes.push_back((Vertex){ 1, -1,  1});
                boundingVertexes.push_back((Vertex){ 1,  1, -1});
                boundingVertexes.push_back((Vertex){ 1,  1,  1});
                break;
            case '#':
                boundingVertexes.push_back((Vertex){-1.1, -1.1, -1.1});
                boundingVertexes.push_back((Vertex){-1.1, -1.1,  1.1});
                boundingVertexes.push_back((Vertex){-1.1,  1.1, -1.1});
                boundingVertexes.push_back((Vertex){-1.1,  1.1,  1.1});
                boundingVertexes.push_back((Vertex){ 1.1, -1.1, -1.1});
                boundingVertexes.push_back((Vertex){ 1.1, -1.1,  1.1});
                boundingVertexes.push_back((Vertex){ 1.1,  1.1, -1.1});
                boundingVertexes.push_back((Vertex){ 1.1,  1.1,  1.1});
                break;
            case '$':
                boundingVertexes.push_back((Vertex){-1, -1, -0.5});
                boundingVertexes.push_back((Vertex){-1, -1,  0.5});
                boundingVertexes.push_back((Vertex){-1,  1, -0.5});
                boundingVertexes.push_back((Vertex){-1,  1,  0.5});
                boundingVertexes.push_back((Vertex){ 1, -1, -0.5});
                boundingVertexes.push_back((Vertex){ 1, -1,  0.5});
                boundingVertexes.push_back((Vertex){ 1,  1, -0.5});
                boundingVertexes.push_back((Vertex){ 1,  1,  0.5});
                break;
            case '%':
                boundingVertexes.push_back((Vertex){-1, -1, -1});
                boundingVertexes.push_back((Vertex){-1, -1,  1});
                boundingVertexes.push_back((Vertex){-1,  1, -1});
                boundingVertexes.push_back((Vertex){-1,  1,  1});
                boundingVertexes.push_back((Vertex){ 1, -1, -1});
                boundingVertexes.push_back((Vertex){ 1, -1,  1});
                boundingVertexes.push_back((Vertex){ 1,  1, -1});
                boundingVertexes.push_back((Vertex){ 1,  1,  1});
                break;
            case '^':
                boundingVertexes.push_back((Vertex){-0.3, -1, -1});
                boundingVertexes.push_back((Vertex){-0.3, -1,  1});
                boundingVertexes.push_back((Vertex){-0.3,  1, -1});
                boundingVertexes.push_back((Vertex){-0.3,  1,  1});
                boundingVertexes.push_back((Vertex){ 0.3, -1, -1});
                boundingVertexes.push_back((Vertex){ 0.3, -1,  1});
                boundingVertexes.push_back((Vertex){ 0.3,  1, -1});
                boundingVertexes.push_back((Vertex){ 0.3,  1,  1});
                break;
        }
    }
    // draw the bounding box using the min/max vertexes
    void drawBoundingBox(Bound b){
        Vertex minV = b.minV;
        Vertex maxV = b.maxV;
        glBegin(GL_LINES);
            // draw all edges of the bounding box using lines
            glVertex3f(minV.x, minV.y, minV.z);
            glVertex3f(maxV.x, minV.y, minV.z);

            glVertex3f(minV.x, minV.y, minV.z);
            glVertex3f(minV.x, maxV.y, minV.z);

            glVertex3f(minV.x, minV.y, minV.z);
            glVertex3f(minV.x, minV.y, maxV.z);


            glVertex3f(minV.x, maxV.y, minV.z);
            glVertex3f(maxV.x, maxV.y, minV.z);

            glVertex3f(minV.x, maxV.y, minV.z);
            glVertex3f(minV.x, maxV.y, maxV.z);


            glVertex3f(maxV.x, maxV.y, maxV.z);
            glVertex3f(minV.x, maxV.y, maxV.z);

            glVertex3f(maxV.x, maxV.y, maxV.z);
            glVertex3f(maxV.x, minV.y, maxV.z);

            glVertex3f(maxV.x, maxV.y, maxV.z);
            glVertex3f(maxV.x, maxV.y, minV.z);


            glVertex3f(maxV.x, minV.y, maxV.z);
            glVertex3f(minV.x, minV.y, maxV.z);

            glVertex3f(maxV.x, minV.y, maxV.z);
            glVertex3f(maxV.x, minV.y, minV.z);


            glVertex3f(maxV.x, minV.y, minV.z);
            glVertex3f(maxV.x, maxV.y, minV.z);

            glVertex3f(minV.x, minV.y, maxV.z);
            glVertex3f(minV.x, maxV.y, maxV.z);
        glEnd();
    }
    // draw the object
    void draw(void){
        glPushMatrix();
            if(texture){
                // set the texture
                glEnable(GL_TEXTURE_2D);
                glGenTextures(2, myTex);
                
                /* Set the image parameters*/
                glBindTexture(GL_TEXTURE_2D, myTex[0]);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                /*Get and save image*/
                image = LoadPPM("marble.ppm",&twidth, &theight,&tmax);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB,
                             GL_UNSIGNED_BYTE, image);
            } else {
                glDisable(GL_TEXTURE_2D);
            }

            // set the material
            material.set();
            // apply transformations
            glTranslatef(position.x, position.y, position.z);
            glRotatef(orientation.x, 1, 0, 0);
            glRotatef(orientation.y, 0, 1, 0);
            glRotatef(orientation.z, 0, 0, 1);
            glScalef(scale.x, scale.y, scale.z);
            // draw the object depending on the type
            switch(type){
                case '!': glutSolidTeapot(1); break;
                case '@': glutSolidSphere(1, 100, 100); break;
                case '#': glutSolidCube(2); break;
                case '$': glutSolidTorus(0.25, 0.75, 100, 100); break;
                case '%': glutSolidIcosahedron(); break;
                case '^': keyModel.draw(); break;
            }
        glPopMatrix();
        // draw the bounding box if the object is selected
        if(selected){
            glPushMatrix();
                pureCyanMaterial.set();
                drawBoundingBox(getBounds());
            glPopMatrix();
        }
    }
    // apply transformations to a vertex
    // used to find the new coordinates of the bounding box
    Vertex applyTransformations(Vertex v){
        float oldX, oldY, oldZ;
        // scale
        v.x = v.x * scale.x;
        v.y = v.y * scale.y;
        v.z = v.z * scale.z;
        // rotate z
        oldX = v.x;
        oldY = v.y;
        v.x = oldX*cos(orientation.z*3.14/180)-oldY*sin(orientation.z*3.14/180);
        v.y = oldX*sin(orientation.z*3.14/180)+oldY*cos(orientation.z*3.14/180);
        // rotate y
        oldX = v.x;
        oldZ = v.z;
        v.x = oldX*cos(orientation.y*3.14/180)+oldZ*sin(orientation.y*3.14/180);
        v.z = oldZ*cos(orientation.y*3.14/180)-oldX*sin(orientation.y*3.14/180);
        // rotate x
        oldY = v.y;
        oldZ = v.z;
        v.y = oldY*cos(orientation.x*3.14/180)-oldZ*sin(orientation.x*3.14/180);
        v.z = oldY*sin(orientation.x*3.14/180)+oldZ*cos(orientation.x*3.14/180);
        // translate
        v.x = v.x + position.x;
        v.y = v.y + position.y;
        v.z = v.z + position.z;

        return v;
    }
    // calculates the min/max vertexes of the bounding box
    Bound getBounds(void){
        float minX = 9999999;
        float minY = 9999999;
        float minZ = 9999999;
        float maxX = -9999999;
        float maxY = -9999999;
        float maxZ = -9999999;
        // apply transformations to the bounding vertexes
        for(int i=0; i<boundingVertexes.size(); i++){
            Vertex v = applyTransformations(boundingVertexes[i]);
            // set the min/max values
            if(v.x < minX){minX = v.x;}
            if(v.y < minY){minY = v.y;}
            if(v.z < minZ){minZ = v.z;}
            if(v.x > maxX){maxX = v.x;}
            if(v.y > maxY){maxY = v.y;}
            if(v.z > maxZ){maxZ = v.z;}
        }

        Bound ret = {(Vertex){minX, minY, minZ}, (Vertex){maxX, maxY, maxZ}};
        return ret;
    }

    // checks if a click is within the bounds of this object
    bool inBounds(Vertex v){
        Bound b = getBounds();
        Vertex minV = b.minV;
        Vertex maxV = b.maxV;
        // check that the click is smaller than all max bounds and bigger than mall min bounds
        if(v.x > minV.x && v.y > minV.y && v.z > minV.z && 
           v.x < maxV.x && v.y < maxV.y && v.z < maxV.z) {
            return true;
        }
        return false;
    }
};

// project a ray from where the cursor clicked to find the corresponding world coordinate
Vertex getWorldCoordinates(int x, int y){
    printf("getWorldCoordinates\n");
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    
    printf("WORLD COORDINATES %f %f %f\n", posX, posY, posZ);
    Vertex ret = {posX, posY, posZ};
    return ret;
}

/************************
     GLOBAL VARIABLES
************************/

// camera angleA, used to rotate the camera about the y axis
int angleA = 90;
// camera angleB, used to rotate the camera about the x-z plane
int angleB = 20;

int simulationSpeed = 100;

// light position
float lightPos1[] = {-10,10,-10};
float lightPos2[] = {10,10,10};

int camDistance = 20;
int selectedObject = -1;

// stores the list of objects
vector<shared_ptr<Object>> objects;

// stores the current material the user set
Material currentMaterial = defaultMaterial;

//OpenGL functions
void keyboard(unsigned char key, int xIn, int yIn) {
    int mod = glutGetModifiers();
    switch (key)
    {
        case 'q':
        case 27:
            // exit the application
            exit(0);
            break;
        case '!':
        case '@':
        case '#':
        case '$':
        case '%':
        case '^': {
            // create the object and add it to the object list
            objects.push_back(shared_ptr<Object>(new Object(key)));
            // set it to the currently selected object
            selectedObject = objects.size() -1;
            // and remove the selected status from any other objects
            for(int i=0; i<objects.size(); i++){
                objects[i]->selected = false;
            }
            objects.back()->selected = true;
            break;
        }
        // set the currently selected material
        case '1':
            currentMaterial = yellowMaterial;
            break;
        case '2':
            currentMaterial = blueMaterial;
            break;
        case '3':
            currentMaterial = redMaterial;
            break;
        case '4':
            currentMaterial = greenMaterial;
            break;
        case '5':
            currentMaterial = blackMaterial;
            break;
        // find the object the user is hovering over and set its material to the current material
        case 'm': {
            Vertex v = getWorldCoordinates(xIn, yIn);
            for(int i=0; i<objects.size(); i++){
                if(objects[i]->inBounds(v)){
                    objects[i]->material = currentMaterial;
                    break;
                }
            }
        }
        break;
        // save the state
        case 's':{
            ofstream myfile;
            myfile.open ("save.txt", ofstream::trunc);
            myfile << "\n";
            // for every object, write the type, material, texture, position, orientation and scale to the file
            for(int i=0; i<objects.size(); i++){
                myfile << "typ " + to_string(objects[i]->type) + "\n";
                if     (objects[i]->material == defaultMaterial){myfile << "mat 0\n";}
                else if(objects[i]->material == yellowMaterial ){myfile << "mat 1\n";}
                else if(objects[i]->material == blueMaterial   ){myfile << "mat 2\n";}
                else if(objects[i]->material == redMaterial    ){myfile << "mat 3\n";}
                else if(objects[i]->material == greenMaterial  ){myfile << "mat 4\n";}
                else if(objects[i]->material == blackMaterial  ){myfile << "mat 5\n";}
                if     (objects[i]->texture){myfile << "tex 1\n";}
                else                        {myfile << "tex 0\n";}
                myfile << "pos " + to_string(objects[i]->position.x) + " " + to_string(objects[i]->position.y) + " " + to_string(objects[i]->position.z) + "\n";
                myfile << "ori " + to_string(objects[i]->orientation.x) + " " + to_string(objects[i]->orientation.y) + " " + to_string(objects[i]->orientation.z) + "\n";
                myfile << "sca " + to_string(objects[i]->scale.x) + " " + to_string(objects[i]->scale.y) + " " + to_string(objects[i]->scale.z) + "\n";
            }
            myfile.close();
            break;
        }
        // load the state
        case 'd':{
            objects.clear();
            FILE * file = fopen("save.txt", "r");
            while(true){
                char lineHeader[128];
                // read the first word of the line
                int res = fscanf(file, "%s", lineHeader);
                if (res == EOF)
                    break; // EOF = End Of File. Quit the loop.
                printf("%s\n", lineHeader);
                // if the line starts with type, create an object with that type and add it to the objects list
                if (strcmp(lineHeader, "typ")==0){
                    char type;
                    fscanf(file, "%d\n", &type);
                    objects.push_back(shared_ptr<Object>(new Object(type)));
                // parse the material and set it
                }else if (strcmp(lineHeader, "mat")==0){
                    int mat;
                    fscanf(file, "%d\n", &mat);
                    if     (mat == 0){objects.back()->material =defaultMaterial;}
                    else if(mat == 1){objects.back()->material =yellowMaterial ;}
                    else if(mat == 2){objects.back()->material =blueMaterial   ;}
                    else if(mat == 3){objects.back()->material =redMaterial    ;}
                    else if(mat == 4){objects.back()->material =greenMaterial  ;}
                    else if(mat == 5){objects.back()->material =blackMaterial  ;}
                // parse the texture and set it
                }else if (strcmp(lineHeader, "tex")==0){
                    bool tex;
                    fscanf(file, "%d\n", &tex);
                    objects.back()->texture = tex;
                // parse the position and set it
                }else if (strcmp(lineHeader, "pos")==0){
                    Vertex pos;
                    fscanf(file, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
                    objects.back()->position = pos;
                // parse the orientation and set it
                }else if (strcmp(lineHeader, "ori")==0){
                    Vertex ori;
                    fscanf(file, "%f %f %f\n", &ori.x, &ori.y, &ori.z);
                    objects.back()->orientation = ori;
                // parse the scale and set it
                }else if (strcmp(lineHeader, "sca")==0){
                    Vertex sca;
                    fscanf(file, "%f %f %f\n", &sca.x, &sca.y, &sca.z);
                    objects.back()->scale = sca;
                }else{
                    // eat up the rest of the line
                    char buffer[1000];
                    fgets(buffer, 1000, file);
                }
            }
            break;
        }
    }
    // functions that only work if there is a selected object
    if(selectedObject>=0){
        switch(key){
            // translate/scale the selected object
            case 'p':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.z +=0.25;
                } else {
                    objects[selectedObject]->position.z -=0.25;
                }
                break;
            case ';':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.z -=0.25;
                } else {
                    objects[selectedObject]->position.z +=0.25;
                }
                break;
            case 'l':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.x -=0.25;
                } else {
                    objects[selectedObject]->position.x -=0.25;
                }
                break;
            case '\'':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.x +=0.25;
                } else {
                    objects[selectedObject]->position.x +=0.25;
                }
                break;
            case '.':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.y -=0.25;
                } else {
                    objects[selectedObject]->position.y -=0.25;
                }
                break;
            case '/':
                if(mod == GLUT_ACTIVE_ALT){
                    objects[selectedObject]->scale.y +=0.25;
                } else {
                    objects[selectedObject]->position.y +=0.25;
                }
                break;
            // rotate the selected object
            case 'x':
                objects[selectedObject]->orientation.x +=1;
                break;
            case 'X':
                objects[selectedObject]->orientation.x -=1;
                break;
            case 'y':
                objects[selectedObject]->orientation.y +=1;
                break;
            case 'Y':
                objects[selectedObject]->orientation.y -=1;
                break;
            case 'z':
                objects[selectedObject]->orientation.z +=1;
                break;
            case 'Z':
                objects[selectedObject]->orientation.z -=1;
                break;
            // toggle the texture on and off
            case 't':
                objects[selectedObject]->texture = !objects[selectedObject]->texture;
                break;
        }
    }
}

// Camera controls
void special(int key, int x, int y) {
    int mod = glutGetModifiers();
    if(mod == GLUT_ACTIVE_ALT){
        switch (key) {
            // move LIGHT0 along the x axis
            case GLUT_KEY_RIGHT: lightPos1[0]++; break;
            case GLUT_KEY_LEFT:  lightPos1[0]--; break;
            // move LIGHT0 along the z axis
            case GLUT_KEY_UP:    lightPos1[2]--; break;
            case GLUT_KEY_DOWN:  lightPos1[2]++; break;
            // move LIGHT0 along the y axis
            case GLUT_KEY_PAGE_UP:   lightPos1[1]++; break;
            case GLUT_KEY_PAGE_DOWN: lightPos1[1]--; break;
        }
    } else if (mod == GLUT_ACTIVE_CTRL) {
        switch (key) {
            // move LIGHT1 along the x axis
            case GLUT_KEY_RIGHT: lightPos2[0]++; break;
            case GLUT_KEY_LEFT:  lightPos2[0]--; break;
            // move LIGHT1 along the z axis
            case GLUT_KEY_UP:    lightPos2[2]--; break;
            case GLUT_KEY_DOWN:  lightPos2[2]++; break;
            // move LIGHT1 along the y axis
            case GLUT_KEY_PAGE_UP:   lightPos2[1]++; break;
            case GLUT_KEY_PAGE_DOWN: lightPos2[1]--; break;
        }
    } else {
        switch (key) {
            // rotate camera about the y axis
            case GLUT_KEY_RIGHT:
                angleA--;
                break;
            case GLUT_KEY_LEFT:
                angleA++;
                break;
            // rotate camera about the x-z plane
            case GLUT_KEY_UP:
                if(angleB <90){
                    angleB++;
                }
                break;
            case GLUT_KEY_DOWN:
                if(angleB > 0){
                    angleB--;
                }
                break;
            // zoom the camera in and out
            case GLUT_KEY_PAGE_UP:
                if(camDistance>5){
                    camDistance--;
                }
                break;
            case GLUT_KEY_PAGE_DOWN:
                if (camDistance<100){
                    camDistance++;
                }
                break;
        }
    }

}

void mouse(int btn, int state, int x, int y) {
    // left click selects an object
    if (btn == GLUT_LEFT_BUTTON){
        if (state == GLUT_UP){
            // find the world coordinate
            Vertex v = getWorldCoordinates(x, y);
            // iterate through all the objects and check if it's within bounds
            for(int i=0; i<objects.size(); i++){
                if(objects[i]->inBounds(v)){
                    // if it's in bounds then de-select any other object and select it
                    for(int j=0; j<objects.size(); j++){
                        objects[j]->selected = false;
                    }
                    objects[i]->selected = true;
                    selectedObject = i;
                    break;
                } else {
                    objects[i]->selected = false;
                    selectedObject = -1;
                }
            }
        }
    }
    // right click deletes an object
    if (btn == GLUT_RIGHT_BUTTON){
        if (state == GLUT_UP){
            // find the world coordinate
            Vertex v = getWorldCoordinates(x, y);
            // iterate through all the objects and check if it's within bounds
            for(int i=0; i<objects.size(); i++){
                if(objects[i]->inBounds(v)){
                    // if it's in bounds then de-select every object and delete the selected object 
                    for(int j=0; j<objects.size(); j++){
                        objects[j]->selected = false;
                    }
                    selectedObject = -1;
                    objects.erase(objects.begin() + i);
                    break;
                }
            }
        }
    }
}

// glut init function
void init(void) {
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 1000);
}

// glut display function
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw lights at the light positions set by the user
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos1);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);

    // where the camera looks at
    float lookX = 0;
    float lookY = 0;
    float lookZ = 0;

    // calculate the camera position
    // calculate where the camera is based on the 2 angles and the camera distance
    float eyeX = lookX + camDistance*cos(angleA*3.14/180)*cos(angleB*3.14/180);
    float eyeY = lookY + camDistance*sin(angleB*3.14/180);
    float eyeZ = lookZ + camDistance*sin(angleA*3.14/180)*cos(angleB*3.14/180);

    // set up the camera
    gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
    
    // draw spheres at the light positions to represent the lights
    glPushMatrix();
        pureCyanMaterial.set();
        glTranslatef(lightPos1[0], lightPos1[1], lightPos1[2]);
        glutSolidSphere(0.5,100,100);
    glPopMatrix();
    
    glPushMatrix();
        pureCyanMaterial.set();
        glTranslatef(lightPos2[0], lightPos2[1], lightPos2[2]);
        glutSolidSphere(0.5,100,100);
    glPopMatrix();

    // draw the floor plane
    floorMaterial.set();
    for (int x=-50; x<50; x++){
        glBegin(GL_TRIANGLE_STRIP);
            for (int z=-50; z<50; z++){
                glNormal3f(0,1,0);
                glVertex3f(x,0,z);
                glNormal3f(0,1,0);
                glVertex3f(x+1,0,z);
            }
        glEnd();
        }

    // draw all the objects in the object list
    for(int i=0; i<objects.size(); i++){
        objects[i]->draw();
    }

    //switch our buffers for a smooth animation
    glutSwapBuffers();
}

// glut timer function
void FPS(int val) {
    glutPostRedisplay();
    glutTimerFunc(simulationSpeed, FPS, 0);
}

// glut reshape function
void reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)((w+0.0f)/h), 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

// initialize glut callbacks
void callBackInit() {
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutTimerFunc(0, FPS, 0);
}

/* main function - program entry point */
int main(int argc, char** argv)
{
    printf("\n\n========================== SIMPLE MODELER INSTRUCTIONS ==========================\n\n");
    printf("  CAMERA CONTROL:\n");
    printf("  Use the UP and DOWN arrow keys to rotate the camera about the x-axis.\n");
    printf("  Use the LEFT and RIGHT arrow keys to rotate the camera about the y-axis.\n");
    printf("  Use the PAGE_UP and PAGE_DOWN keys to zoom the camera in and out.\n\n");
    printf("  TRANSLATION AND SCALING:\n");
    printf("  Use the p and ; keys to move the selected object parallel to the x-axis\n");
    printf("  Use the l and ' keys to move the selected object parallel to the z-axis\n");
    printf("  Use the . and / keys to move the selected object parallel to the y-axis\n");
    printf("  Hold ALT while pressing the p, ;, l, ', ., / keys to scale it instead\n\n");
    printf("  LIGHTING:\n");
    printf("  HOLD CTRL while pressing the UP and DOWN keys to move LIGHT0 parallel to the x-axis\n");
    printf("  HOLD CTRL while pressing the LEFT and RIGHT keys to move LIGHT0 parallel to the z-axis\n");
    printf("  HOLD CTRL while pressing the PAGE_UP and PAGE_DOWN keys to move LIGHT0 parallel to the y-axis\n");
    printf("  Hold ALT while pressing the UP, DOWN, LEFT, RIGHT, PAGE_UP, PAGE_DOWN keys to move LIGHT1 instead\n\n");
    printf("  ROTATION:\n");
    printf("  Press x, y, and z KEYS to rotate the selected object about its respective axis.\n");
    printf("  Hold SHIFT while pressing the x, y, z keys to rotate it in the opposite direction\n\n");
    printf("  MATERIALS:\n");
    printf("  Press 1 to change the current material to gold\n");
    printf("  Press 2 to change the current material to sapphire\n");
    printf("  Press 3 to change the current material to ruby\n");
    printf("  Press 4 to change the current material to emerald\n");
    printf("  Press 5 to change the current material to obsidian\n");
    printf("  Press m to change the material the mouse is selecting to the current material\n\n");
    printf("  OBJECTS:\n");
    printf("  Hold SHIFT while pressing 1 to add a teapot to the origin\n");
    printf("  Hold SHIFT while pressing 2 to add a sphere to the origin\n");
    printf("  Hold SHIFT while pressing 3 to add a cube to the origin\n");
    printf("  Hold SHIFT while pressing 4 to add a torus to the origin\n");
    printf("  Hold SHIFT while pressing 5 to add a icosahedron the origin\n");
    printf("  Hold SHIFT while pressing 6 to add a key to the origin\n\n");
    printf("  OTHERS:\n");
    printf("  Press t to toggle texture on and off for the selected object.\n");
    printf("  Press s to save the current state.\n");
    printf("  Press d to load the saved state.\n");
    printf("  Press r to clear all objects.\n");
    printf("  Press q or esc to exit the program.\n\n");
    printf("================================================================================\n\n");

    cout << "Press Enter to Continue...";
    cin.ignore();

    glutInit(&argc, argv);      //starts up GLUT
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Simple Modeler");  //creates the main window
    callBackInit();
    init();

    glutMainLoop();             //starts the event glutMainLoop
    return(0);                  //return may not be necessary on all compilers
}
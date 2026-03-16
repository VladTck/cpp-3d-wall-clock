#include<iostream>
#include <GLFW/glfw3.h>
#include <ctime>
#include <cmath>
#include <vector>

enum {
    FULL_WINDOW,
    ASPECT_1_1,
    ORTO,
    FRUST
};

GLint skala = FULL_WINDOW;
GLint rzut = ORTO;


using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/stb_image.h>

#define LICZBA_OB_TEXTUR 1
GLuint obiektyTextur[LICZBA_OB_TEXTUR];
unsigned int texture;
const char* plikiTextur[LICZBA_OB_TEXTUR] = {"D://Grafika_komputerowa//wood_texture_3.jfif"}; //tutaj należy podać pełne ścieżki do plików tekstur

const float PI = 3.14159265f;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;


bool useRealTime = true;
bool showMechanism = false;
bool lightingEnabled = true;
float timeAdjustment = 0.0f;
float pendulumAngle = 0.0f;
int simulatedSeconds = 0;


//invert true || false
bool invertX = true;
bool invertY = true;



float cameraAngleX = 30.0f;
float cameraAngleY = 0.0f;
float cameraDistance = 500.0f;
bool mouseLeftPressed = false;
double lastMouseX, lastMouseY;


float xx = 0.0f, yy = 0.0f, zz = 0.0f;
double r1 = 0.0, r2 = 0.0, r3 = 0.0;


GLfloat lightDiffuse[] = { 1.0f, 1.0f, 0.9f, 1.0f }; 
GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f }; 
bool showReferenceObject = true;


void setupLighting() {
    if (!lightingEnabled) {
        glDisable(GL_LIGHTING);
        return;
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat lightPos[] = { 200.0f, 200.0f, 200.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse); // змінене
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

    GLfloat matSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat matShininess[] = { 50.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
}




void drawCylinder(float radius, float height, int segments = 32) {
    std::vector<float> vertices;

    // Side surface
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * PI * i / segments;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        vertices.push_back(x);
        vertices.push_back(0);
        vertices.push_back(z);

        vertices.push_back(x);
        vertices.push_back(height);
        vertices.push_back(z);
    }

    glBegin(GL_TRIANGLE_STRIP);
    for (size_t i = 0; i < vertices.size(); i += 3) {
        glNormal3f(vertices[i] / radius, 0, vertices[i + 2] / radius);
        glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    glEnd();

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * PI * i / segments;
        glVertex3f(cos(angle) * radius, height, sin(angle) * radius);
    }
    glEnd();

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = segments; i >= 0; i--) {
        float angle = 2 * PI * i / segments;
        glVertex3f(cos(angle) * radius, 0, sin(angle) * radius);
    }
    glEnd();
}


void drawSphere(float radius, int segments = 32) {
    for (int i = 0; i < segments; i++) {
        float lat0 = PI * (-0.5f + (float)i / segments);
        float z0 = sin(lat0) * radius;
        float zr0 = cos(lat0) * radius;

        float lat1 = PI * (-0.5f + (float)(i + 1) / segments);
        float z1 = sin(lat1) * radius;
        float zr1 = cos(lat1) * radius;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= segments; j++) {
            float lng = 2 * PI * (float)j / segments;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0 / radius, y * zr0 / radius, z0 / radius);
            glVertex3f(x * zr0, y * zr0, z0);

            glNormal3f(x * zr1 / radius, y * zr1 / radius, z1 / radius);
            glVertex3f(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
}

void drawBox(float width, float height, float depth) {
    float w = width / 2;
    float h = height / 2;
    float d = depth / 2;

    glDepthMask(GL_FALSE); // Unikamy z-fightingu na przedniej ścianie

    // Przód (patrzysz na +Z)
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    // Tekstura od lewej do prawej, od dołu do góry
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, d); // Lewy-dół
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h, d); // Prawy-dół
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d); // Prawy-góra
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d); // Lewy-góra
    glEnd();
    glDepthMask(GL_TRUE);

    // Tyl (patrzysz na -Z)
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    // Tekstura od prawej do lewej (odwrotnie)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, -h, -d); // Lewy-dół
    glTexCoord2f(0.0f, 0.0f); glVertex3f(w, -h, -d); // Prawy-dół
    glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, -d); // Prawy-góra
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, -d); // Lewy-góra
    glEnd();

    // Lewa ściana (-X)
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, -d); // Tyl-dół
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w, -h, d); // Przód-dół
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w, h, d); // Przód-góra
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, -d); // Tyl-góra
    glEnd();

    // Prawa ściana (+X)
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(w, -h, d); // Przód-dół
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h, -d); // Tyl-dół
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, -d); // Tyl-góra
    glTexCoord2f(0.0f, 1.0f); glVertex3f(w, h, d); // Przód-góra
    glEnd();

    // Góra (+Y)
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, h, -d); // Lewy-tyl
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, h, -d); // Prawy-tyl
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, h, d); // Prawy-przód
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, h, d); // Lewy-przód
    glEnd();

    // Dół (-Y)
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h, d); // Lewy-przód
    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h, d); // Prawy-przód
    glTexCoord2f(1.0f, 1.0f); glVertex3f(w, -h, -d); // Prawy-tyl
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w, -h, -d); // Lewy-tyl
    glEnd();
}

void drawClockHand(float angle, float length, float width, float depth) {
    glPushMatrix();
    glRotatef(-angle, 0, 0, 1);

    // Draw the hand as a thin box
    glTranslatef(0, length / 2, 0);
    drawBox(width, length, depth);

    glPopMatrix();
}

void drawClockCase() {

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, obiektyTextur[0]);

    // Tylna część obudowy
    glPushMatrix();
    glTranslatef(0, -150, 0);
    drawBox(240, 600, 50);
    //drawBox(240, 500, 50);
    glPopMatrix();


    // Przednia część obudowy z wycięciem 
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    // Górna część przedniego panelu 
    
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-120, 150, 25);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(120, 150, 25);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(120, -100, 25);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-120, -100, 25);

    // Dolna część przedniego panelu 
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-120, -250, 25);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(120, -250, 25);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(120, -450, 25);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-120, -450, 25);

    // Lewa część wycięcia
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-120, -100, 25);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-80, -100, 25);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-80, -250, 25);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-120, -250, 25);

    // Prawa część wycięcia
    glTexCoord2f(0.0f, 0.0f); glVertex3f(120, -100, 25);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(80, -100, 25);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(80, -250, 25);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(120, -250, 25);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void drawClockFace() {
    

    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glRotated(1, 1, 0, 45);
    glTranslatef(0, 100, 30);
    glRotatef(90, 90, 0, 0);
    drawCylinder(90, 5, 32);

    glPopMatrix();

    
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * PI / 180.0f;
        float x = sin(angle) * 80;
        float y = cos(angle) * 80 + 100;

        glPushMatrix();
        glTranslatef(x, y, 35);
        if (i % 3 == 0) {
            
            drawBox(4, 15, 2);
        }
        else {
            
            drawBox(2, 10, 2);
        }
        glPopMatrix();
    }

    
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0, 100, 35);
    drawSphere(5, 16);
    glPopMatrix();


}

void drawPendulum(float swingAngle) {
    glPushMatrix();
    
    glTranslatef(0, -5, 0);
    glRotatef(swingAngle, 0, 0, 1);

    
    glColor3f(0.7f, 0.7f, 0.1f);
    glPushMatrix();
    glTranslatef(0, -75, 0);
    drawBox(6, 150, 2);
    glPopMatrix();

    
    glColor3f(0.5f, 0.5f, 0.1f);
    glPushMatrix();
    glTranslatef(0, -150, 0);
    drawSphere(20, 16);
    glPopMatrix();

    glPopMatrix();
}

void drawGear(float x, float y, float z, float radius, float thickness, int teeth, float rotation) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotation, 0, 0, 1);

    
    std::vector<float> vertices;

    for (int i = 0; i < teeth * 2; i++) {
        float angle = 2 * PI * i / (teeth * 2);
        float r = (i % 2 == 0) ? radius * 1.2f : radius * 0.8f;
        vertices.push_back(cos(angle) * r);
        vertices.push_back(sin(angle) * r);
    }

    
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, 1);
    for (size_t i = 0; i < vertices.size(); i += 2) {
        glVertex3f(vertices[i], vertices[i + 1], thickness / 2);
    }
    glEnd();

    glBegin(GL_POLYGON);
    glNormal3f(0, 0, -1);
    for (int i = vertices.size() - 2; i >= 0; i -= 2) {
        glVertex3f(vertices[i], vertices[i + 1], -thickness / 2);
    }
    glEnd();

    
    glBegin(GL_QUAD_STRIP);
    for (size_t i = 0; i < vertices.size(); i += 2) {
        size_t next = (i + 2) % vertices.size();

        float nx = vertices[i + 1] - vertices[next + 1];
        float ny = vertices[next] - vertices[i];
        float len = sqrt(nx * nx + ny * ny);
        nx /= len;
        ny /= len;

        glNormal3f(nx, ny, 0);
        glVertex3f(vertices[i], vertices[i + 1], thickness / 2);
        glVertex3f(vertices[i], vertices[i + 1], -thickness / 2);
    }
    glEnd();

    glPopMatrix();
}

void drawInternalMechanism() {
    if (!showMechanism) return;

    glPushAttrib(GL_CURRENT_BIT); 
    glColor3f(0.3f, 0.3f, 0.3f); 

    
    drawGear(0, -50, 10, 30, 8, 20, glfwGetTime() * 20.0f);
    drawGear(50, -80, 10, 20, 6, 16, -glfwGetTime() * 30.0f);
    drawGear(-40, -100, 10, 25, 7, 18, glfwGetTime() * 25.0f);

    
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_STRIP);
    for (float x = -20; x <= 20; x += 0.5f) {
        float y = -150 + sin(x * 0.5f + glfwGetTime() * 5.0f) * 10.0f;
        glVertex3f(x, y, 10);
    }
    glEnd();

    glPopAttrib(); 
}

void drawGoldenCenter(float radius) {
    const int segments = 100;
    GLfloat goldColor[] = { 0.83f, 0.69f, 0.22f, 1.0f };

    glDisable(GL_TEXTURE_2D); 
    glDisable(GL_LIGHTING); 
    glColor3f(0.83f, 0.69f, 0.22f);
    glNormal3f(0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.01f);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, 0.01f);
    }
    glEnd();

    
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mouseLeftPressed = (action == GLFW_PRESS);
        if (mouseLeftPressed) {
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (mouseLeftPressed) {
        double dx = xpos - lastMouseX;
        double dy = ypos - lastMouseY;
        dx *= (invertX ? -1.0 : 1.0);
        dy *= (invertY ? -1.0 : 1.0);

        
        cameraAngleY += dx * 0.5f;
        cameraAngleX += dy * 0.5f;

        
        if (cameraAngleY > 90.0f) cameraAngleY = 90.0f;
        if (cameraAngleY < -90.0f) cameraAngleY = -90.0f;

        
        if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
        if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraDistance -= yoffset * 10.0f;
    if (cameraDistance < 100.0f) cameraDistance = 100.0f;
    if (cameraDistance > 1000.0f) cameraDistance = 1000.0f;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_EQUAL:
                //timeAdjustment += 0.5f;
                timeAdjustment += 5.0f;
                break;
            case GLFW_KEY_MINUS:
                //timeAdjustment -= 0.5f;
                timeAdjustment -= 5.0f;
                break;

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_F2:
            showMechanism = !showMechanism;
            break;
        case GLFW_KEY_F3:
            lightingEnabled = !lightingEnabled;
            break;
        case GLFW_KEY_W:
            cameraDistance -= 50.0f; 
            if (cameraDistance < 100.0f) cameraDistance = 100.0f;
            break;
        case GLFW_KEY_S:
            cameraDistance += 50.0f; 
            if (cameraDistance > 1000.0f) cameraDistance = 1000.0f;
            break;
        case GLFW_KEY_A:
            cameraAngleY -= 2.0f; 
            break;
        case GLFW_KEY_D:
            cameraAngleY += 2.0f; 
            break;
        case GLFW_KEY_UP: 
            cameraAngleX += 2.0f;
            
            break;
        case GLFW_KEY_DOWN: 
            cameraAngleX -= 2.0f;
            
            break;
        case GLFW_KEY_LEFT: 
            xx -= 10.0f;
            
            break;
        case GLFW_KEY_RIGHT: 
            xx += 10.0f;
            
            break;
        case GLFW_KEY_E:
            r3 -= 5.0;
            
            break;
        case GLFW_KEY_Q:
            r3 += 5.0;
            
            break;
        case GLFW_KEY_X:
            zz += 0.5;
            
            break;
        case GLFW_KEY_Z:
            zz -= 0.5;
            
            break;
        case GLFW_KEY_I:
            lightDiffuse[0] = min(1.0f, lightDiffuse[0] + 0.1f);
            break;
        case GLFW_KEY_K:
            lightDiffuse[0] = max(0.0f, lightDiffuse[0] - 0.1f);
            break;
        case GLFW_KEY_O:
            lightDiffuse[1] = min(1.0f, lightDiffuse[1] + 0.1f);
            break;
        case GLFW_KEY_L:
            lightDiffuse[1] = max(0.0f, lightDiffuse[1] - 0.1f);
            break;
        case GLFW_KEY_P:
            lightDiffuse[2] = min(1.0f, lightDiffuse[2] + 0.1f);
            break;
        case GLFW_KEY_F4:
            showReferenceObject = !showReferenceObject;
            break;
        
        }
    }
}

void myPerspective(double fovY, double aspect, double zNear, double zFar) {
    const double pi = 3.14159265358979323846;
    double fH = tan(fovY / 360 * pi) * zNear;
    double fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void myLookAt(double eyeX, double eyeY, double eyeZ,double centerX, double centerY, double centerZ,double upX, double upY, double upZ) {
    double forward[3], side[3], upVec[3]; 

    
    forward[0] = centerX - eyeX;
    forward[1] = centerY - eyeY;
    forward[2] = centerZ - eyeZ;
    double len = sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    forward[0] /= len;
    forward[1] /= len;
    forward[2] /= len;

    
    upVec[0] = upX;
    upVec[1] = upY;
    upVec[2] = upZ;
    side[0] = forward[1] * upVec[2] - forward[2] * upVec[1];
    side[1] = forward[2] * upVec[0] - forward[0] * upVec[2];
    side[2] = forward[0] * upVec[1] - forward[1] * upVec[0];
    len = sqrt(side[0] * side[0] + side[1] * side[1] + side[2] * side[2]);
    side[0] /= len;
    side[1] /= len;
    side[2] /= len;

    
    upVec[0] = side[1] * forward[2] - side[2] * forward[1];
    upVec[1] = side[2] * forward[0] - side[0] * forward[2];
    upVec[2] = side[0] * forward[1] - side[1] * forward[0];
    len = sqrt(upVec[0] * upVec[0] + upVec[1] * upVec[1] + upVec[2] * upVec[2]);
    upVec[0] /= len;
    upVec[1] /= len;
    upVec[2] /= len;

    
    double m[16];
    m[0] = side[0]; m[4] = side[1]; m[8] = side[2]; m[12] = -side[0] * eyeX - side[1] * eyeY - side[2] * eyeZ;
    m[1] = upVec[0]; m[5] = upVec[1]; m[9] = upVec[2]; m[13] = -upVec[0] * eyeX - upVec[1] * eyeY - upVec[2] * eyeZ;
    m[2] = -forward[0]; m[6] = -forward[1]; m[10] = -forward[2]; m[14] = forward[0] * eyeX + forward[1] * eyeY + forward[2] * eyeZ;
    m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 1;

    glLoadMatrixd(m);
}


void drawClock() {
    glPushMatrix();
    
    time_t now = time(0);
    tm localTime;
    localtime_s(&localTime, &now);

    
    int currentSecond = useRealTime ? localTime.tm_sec : (simulatedSeconds % 60);
    int currentMinute = localTime.tm_min + (int)(timeAdjustment / 60);
    int currentHour = (localTime.tm_hour % 12) + (int)(timeAdjustment / 3600);

    
    float hourAngle = (currentHour + currentMinute / 60.0f) * 30.0f;
    float minuteAngle = (currentMinute + currentSecond / 60.0f) * 6.0f;
    float secondAngle = currentSecond * 6.0f;

   
    glTranslated(xx, yy, zz);
    glRotated(r1, 0.0, 1.0, 0.0);
    
    glRotated(r2, 1.0, 0.0, 0.0);
    glRotated(r3, 0.0, 0.0, 1.0);

    drawClockCase();

    glTranslatef(0, -65, -7);
    drawClockFace();



    glPushMatrix();
    glTranslated(0, 102, 36);
    drawGoldenCenter(45.2f);
    glPopMatrix();


    glPushMatrix();

    glTranslated(0, 0, 2);

    
    glPushMatrix();
    glTranslatef(0, 100, 35);

    
    glColor3f(0.1f, 0.1f, 0.1f);
    drawClockHand(hourAngle, 50, 6, 1);

    
    drawClockHand(minuteAngle, 70, 4, 1);

    
    glColor3f(0.8f, 0.1f, 0.1f);
    drawClockHand(secondAngle, 80, 2, 0.5);

    glPopMatrix();


    glPopMatrix();

    
    pendulumAngle = sin(glfwGetTime() * 2.5f) * 15.0f;

    drawPendulum(pendulumAngle);

    
    //glRotated(1, 1, 0, 45);
    drawInternalMechanism();

    
    glPopMatrix();
    
}

void drawReferenceObject() {
    if (!showReferenceObject) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f); 
    glTranslatef(0.0f, 150.0f, 200.0f); 

    
    drawBox(30.0f, 30.0f, 30.0f); 
    glPopMatrix();
    glPopAttrib();
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Projekt z Grafiky", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glGenTextures(LICZBA_OB_TEXTUR, obiektyTextur);
    for (int i = 0; i < LICZBA_OB_TEXTUR; i++) {
        GLint iw, ih, nc;
        glBindTexture(GL_TEXTURE_2D, obiektyTextur[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        unsigned char* data = stbi_load(plikiTextur[i], &iw, &ih, &nc, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iw, ih, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        myPerspective(45.0, (double)WINDOW_WIDTH / WINDOW_HEIGHT, 1.0, 2000.0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        //cameraDistance = 500.0f;

        double camX = cameraDistance * sin(cameraAngleY * PI / 180.0) * cos(cameraAngleX * PI / 180.0) + xx;
        double camY = cameraDistance * sin(cameraAngleX * PI / 180.0) + yy;
        double camZ = cameraDistance * cos(cameraAngleY * PI / 180.0) * cos(cameraAngleX * PI / 180.0) + zz;

        myLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);

        setupLighting();

        drawClock();

        if (showReferenceObject) {
            drawReferenceObject();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (!useRealTime) simulatedSeconds++;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    glDeleteTextures(LICZBA_OB_TEXTUR, obiektyTextur);
    return 0;
}







#include "include/Angel.h"
#include <iostream>
#include <assert.h>
#include <cstring>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;
const GLfloat ThetaDelta = 5.0;
const GLfloat Pi = 3.141592653589793;
const color4 BACKGROUND_COLOR = vec4(0.5, 0.5, 0.5, 1);

// Shader transformation matrices
mat4 model;
mat4 view;
GLuint uniModel, uniProjection, uniView;

// Array of rotation angles (in degrees) for each rotation axis
enum { BASE = 0, LOWER_ARM = 1, UPPER_ARM = 2, NumAngles = 3 };
int      Axis = BASE;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  QUIT = 5;
const int  SWITCH_VIEW = 4;

// ROBOT_MODE
enum {
    FREE = 0,
    FETCH = 1
};
int ROBOT_MODE;

// VIEW_MODE
enum {
    TOP = 0,
    SIDE = 1,
};
int VIEW_MODE = SIDE;

// sphere status
enum {
    ABOSOLUTE = 0,
    ATTACHED = 1,
};
int SPHERE_STATUS = ABOSOLUTE;
int MOVE_DELAY= 100;

// fetch status
bool FETCH_STATUS[2][NumAngles] = {{false}, {false}};

// position of sphere
vec3 targets[2] = {
    vec3(0, 0, 0),
    vec3(0, 0, 0),
};
int current_target_index = 0;
//----------------------------------------------------------------------------

int Index = 0;

void move_base(int);
void move_lower_arm(int);
void move_upper_arm(int);
void reset_arms(int);
void reset_arm(int);

inline float degree_to_radian(float degree) {
    return degree * Pi / 180;
}

inline float radian_to_degree(float radian) {
    return radian * 180 / Pi;
}

void quad( int a, int b, int c, int d ) {
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

// generate a cube
void colorcube() {
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void draw_base() {
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void draw_upper_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void draw_lower_arm() {
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------
void draw_sphere() {
    mat4 instance;
    if(SPHERE_STATUS == ATTACHED) {
        instance = Translate(0, 0, 0) * Scale(UPPER_ARM_WIDTH / 2, UPPER_ARM_WIDTH / 2, UPPER_ARM_WIDTH / 2);
        glUniformMatrix4fv( uniModel, 1, GL_TRUE, model * instance ); 
    } else {
        instance = Translate(targets[current_target_index]) * Scale(UPPER_ARM_WIDTH / 2, UPPER_ARM_WIDTH / 2, UPPER_ARM_WIDTH / 2);
        glUniformMatrix4fv(uniModel, 1, GL_TRUE, instance); 
    }
    glutSolidSphere (1.0, 20, 20);
}

inline bool arms_are_reset() {
    for(int axis = 0; axis < NumAngles; axis++) {
        if(Theta[axis] != 0) {
            return false;
        }
    }
    return true;
}

void reset_arm(int axis) {
    if(Theta[axis] <= 0) {
        Theta[axis] = 0;
    } else {
        Theta[axis] -= ThetaDelta;
    }
}
void reset_arms(int value) {

    for(int axis = 0; axis < NumAngles; axis++) {
        reset_arm(axis);
    }

    if(arms_are_reset()) {
        cout << "Finish all!\n";
    } else {
        glutTimerFunc(MOVE_DELAY, reset_arms, 0);
    }
    glutPostRedisplay();
}

void move_upper_arm(int value) {
    cout << "executing move_upper_arm()\n";
    vec4 upper_arm_top_center_relative_position = vec4(0, 0.5, 0, 1);
    vec4 upper_arm_top_center_world_position = RotateY(Theta[BASE]) * Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LOWER_ARM]) * Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
		    RotateZ(Theta[UPPER_ARM]) * Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) * Scale(UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH) * upper_arm_top_center_relative_position;
        vec4 sphere_position = vec4(targets[current_target_index], 1);
    GLfloat distance = length(upper_arm_top_center_world_position - sphere_position);
    if(fabs(distance) > UPPER_ARM_WIDTH) {
        // if already search 180 degree
        if(Theta[UPPER_ARM] == 360) {
            // should not happen!
            cout << "Error: the sphere is too far!!\n";
            cout << "This is a problem caused by calculation precision\n";
            return;
        }
        // the upper arm cannot fetch
        // rotate
        Theta[UPPER_ARM] += ThetaDelta;
        // set timer
        glutTimerFunc(MOVE_DELAY, move_upper_arm, 0);
    } else {
        // the upper arm can fetch
        // set the flag
        FETCH_STATUS[current_target_index][UPPER_ARM] = true;
        cout << "Fetch status [" << current_target_index << "] base finished!\n";
        // if finish state 1, continue state 2
        if(current_target_index == 0) {
            current_target_index++;
            SPHERE_STATUS = ATTACHED;
            glutTimerFunc(2 * MOVE_DELAY, move_base, 0);

        } else if(current_target_index == 1) {
            // drop the sphere and return to init position
            SPHERE_STATUS = ABOSOLUTE;
            // call reset function
            glutTimerFunc(2 * MOVE_DELAY, reset_arms, 0);
        }
    }
    glutPostRedisplay();
}

void move_lower_arm(int value) {
    cout << "executing move_lower_arm()\n";
    vec4 lower_arm_top_center_relative_position = vec4(0, 0.5, 0, 1);
    vec4 upper_arm_top_center_world_position = RotateY(Theta[BASE]) * Translate(0.0, BASE_HEIGHT, 0.0) * RotateZ(Theta[LOWER_ARM]) * Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) * Scale(LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH) * lower_arm_top_center_relative_position;
    vec4 sphere_position = vec4(targets[current_target_index], 1);
    GLfloat distance = length(upper_arm_top_center_world_position - sphere_position) - UPPER_ARM_WIDTH / 2;
    if(fabs(distance - UPPER_ARM_HEIGHT) > UPPER_ARM_WIDTH / 5) {
        // if already search 360 degree
        if(Theta[LOWER_ARM] == 360) {
            // the sphere cannot be fetched
            cout << "Error: the sphere is too far!!\n";
            return;
        }
        // the upper arm cannot fetch
        // rotate
        Theta[LOWER_ARM] += ThetaDelta;
        glutPostRedisplay();
        // set timer
        glutTimerFunc(MOVE_DELAY, move_lower_arm, 0);
    } else {
        // the upper arm can fetch
        // set the flag
        FETCH_STATUS[current_target_index][LOWER_ARM] = true;
        cout << "Fetch status [" << current_target_index << "] base finished!\n";
        // start upper arm
        glutTimerFunc(MOVE_DELAY, move_upper_arm, 0);
    }
}

void move_base(int value) {
    cout << "executing move_base()\n";
    // only execute once move, and call many times
    // calculate the direction: clock-wise (+1) or not (-1)
    int base_direction = targets[current_target_index][2] > 0 ? 1 : -1;
    // calculate the -x vector (point to the direction which arms can move)
    // using RotateY to convert
    vec4 base_vector_4d = RotateY(Theta[BASE]) * vec4(-1, 0, 0, 0);
    vec2 base_vector = vec2(base_vector_4d[0], base_vector_4d[2]);
    // easy to make mistakes here: current_target_x, current_target_z axis
    // vector of sphere in current_target_x-current_target_z axis
    vec2 sphere_vector = vec2(targets[current_target_index][0], targets[current_target_index][2]);
    // alpha is the radian of the difference between the two vectors
    float alpha;
    if(length(sphere_vector) == 0) {
        alpha = 0;
    } else {
        alpha = acos(dot(base_vector, sphere_vector) / length(sphere_vector)); // note length(base_vector) == 1
    }
    cout << "Distance angle: " << radian_to_degree(alpha) << endl;
    assert(alpha >= 0);
    if(alpha > degree_to_radian(ThetaDelta) / 2) {
        // need to rotate
        Theta[BASE] += base_direction * ThetaDelta;
        glutPostRedisplay();
        // delay for a period and call myself again
        glutTimerFunc(MOVE_DELAY, move_base, 0);
    } else {
        // no need to rotate, set the flag
        FETCH_STATUS[current_target_index][BASE] = true;
        cout << "Fetch status [" << current_target_index << "] base finished!\n";
        // start next arm
        glutTimerFunc(MOVE_DELAY, move_lower_arm, 0);
    }
}

void display( void ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(VIEW_MODE == TOP) {
        view = LookAt(vec4(1, 2, 5, 1), vec4(1, 0, 5, 1), vec4(0, 0, -1, 0));
    } else {
        view = mat4(1);
    }
    glUniformMatrix4fv(uniView, 1, GL_TRUE, view); 

    // Accumulate uniModel Matrix as we traverse the tree
    model = RotateY(Theta[BASE] );
    draw_base();

    model *= ( Translate(0.0, BASE_HEIGHT, 0.0) *
		    RotateZ(Theta[LOWER_ARM]) );
    draw_lower_arm();

    model *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
		    RotateZ(Theta[UPPER_ARM]) );
    draw_upper_arm();

    model *= (Translate(0.0, UPPER_ARM_HEIGHT, 0.0));
    // draw sphere
    draw_sphere();

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init( void ) {
    colorcube();
    
    // Create a vertex array object
    GLuint vao;
    glGenVertexArraysAPPLE( 1, &vao );
    glBindVertexArrayAPPLE( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		  NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );
    
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    uniModel = glGetUniformLocation( program, "uniModel" );
    uniView = glGetUniformLocation(program, "uniView");
    uniProjection = glGetUniformLocation( program, "uniProjection" );

    glEnable( GL_DEPTH );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

    glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], BACKGROUND_COLOR[3]); 
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int current_target_x, int y ) {

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
	// Incrase the joint angle
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
	// Decrase the joint angle
    }

}

void onSpecialKeyPressed(int key, int current_target_x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            Theta[Axis] += ThetaDelta;
            if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
            break;
        case GLUT_KEY_RIGHT:
            Theta[Axis] -= ThetaDelta;
            if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu( int option ) {
    if ( option == QUIT ) {
        exit( EXIT_SUCCESS );
    } else if(option == SWITCH_VIEW) {
        VIEW_MODE = 1 - VIEW_MODE;
        glutPostRedisplay();
    } else {
        Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape( int width, int height ) {
    glViewport( 0, 0, width, height );

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width) / height;

    if ( aspect > 1.0 ) {
	left *= aspect;
	right *= aspect;
    }
    else {
	bottom /= aspect;
	top /= aspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( uniProjection, 1, GL_TRUE, projection );

    model = mat4( 1.0 );  // An Identity matrix
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int current_target_x, int y ) {
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

int main( int argc, char **argv ) {
    glutInit( &argc, argv );
    // parse argv
    cout << "Number of args: " << argc << endl;
    if(argc == 8) {
        ROBOT_MODE = FETCH;
        cout << "Enter Fetch mode" << endl;

        // assign sphere position
        targets[0] = vec3(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
        targets[1] = vec3(atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));

        // perspective
        if(strcmp("-tv", argv[7]) == 0) {
            VIEW_MODE = TOP;
            cout << "Use top view\n";
        } else {
            VIEW_MODE = SIDE;
            cout << "Use side view\n";
        }
    } else {
        ROBOT_MODE = FREE;
        cout << "Enter Free mode" << endl;
    }
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "robot" );

    glewExperimental = GL_TRUE; 
    glewInit();
    init();

    glutDisplayFunc( display );
    // triggered when window is reshaped
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutSpecialFunc(onSpecialKeyPressed);
    glutMouseFunc( mouse );

    glutCreateMenu( menu );
    // Set the menu values to the relevant rotation axis values (or QUIT)
    glutAddMenuEntry( "base", BASE );
    glutAddMenuEntry( "lower arm", LOWER_ARM );
    glutAddMenuEntry( "upper arm", UPPER_ARM );
    glutAddMenuEntry( "switch view", SWITCH_VIEW);
    glutAddMenuEntry( "quit", QUIT );
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    if(ROBOT_MODE == FETCH) {
        glutTimerFunc(MOVE_DELAY, move_base, 0);
    }

    glutMainLoop();
    return 0;
}

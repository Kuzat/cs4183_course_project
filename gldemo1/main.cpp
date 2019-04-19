/* Link the libraries code-wise. */
#ifdef _MSC_VER
#	pragma comment(lib, "OpenGL32.lib")
#	pragma comment(lib, "GLu32.lib")

#	pragma comment(lib, "SDL.lib")
#	pragma comment(lib, "SDLmain.lib")
#	pragma comment(lib, "SDL_image.lib")
#endif //_MSC_VER

#include <string>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <GL\glut.h>

#define PI 3.141592653589793

#define EARTH_TX		4
#define MOON_TX			5
#define JUPITER_TX		6
#define MARS_TX			7
#define MERCURY_TX		8
#define NEPTUNE_TX		9
#define SATURN_TX		10
#define SATURN_RING_TX	11
#define URANUS_TX		12
#define VENUS_TX		13

const unsigned textureLength(14);
unsigned Textures[textureLength];

/* These will define the player's position and view angle. */
double X(0.0), Y(0.0), Z(0.0);
double ViewAngleHor(0.0), ViewAngleVer(0.0);

// Movements variables
double movementSpeed(0.005);

// Room properties
double roomLength(32);
double roomWidth(1800);
double roomHeight(1000);

// Orbits
double moonOrbit(0);
double planetSpinn(0);
double ringSpin(0);

/*
 * DegreeToRadian
 *	Converts a specified amount of degrees to radians.
 */
inline double DegreeToRadian(double degrees)
{
	return (degrees / 180.f * PI);
}

/*
 * GrabTexObjFromFile
 *	This function will use SDL to load the specified image, create an OpenGL
 *	texture object from it and return the texture object number.
 */
GLuint GrabTexObjFromFile(const std::string& fileName, unsigned format)
{
	/* Use SDL_image to load the PNG image. */
	SDL_Surface *Image = IMG_Load(fileName.c_str());
	
	/* Image doesn't exist or failed loading? Return 0. */
	if(!Image)
		return 0;

	unsigned Object(0);

	/* Generate one texture (we're creating only one). */
	glGenTextures(1, &Object);

	/* Set that texture as current. */
	glBindTexture(GL_TEXTURE_2D, Object);

	/* You can use these values to specify mipmaps if you want to, such as 'GL_LINEAR_MIPMAP_LINEAR'. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* We're setting textures to be repeated here. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //NEW!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //NEW!

	/* Create the actual texture object. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image->w, Image->h, 0, format, GL_UNSIGNED_BYTE, Image->pixels);

	/* Free the surface, we are finished with it. */
	SDL_FreeSurface(Image);

	return Object;
}

/*
 *	CompileLists
 *		Compiles the display lists used by our application.
 */
unsigned createBoxList()
{
	/* Let's generate a display list for a box. */
	unsigned BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

		/*
		 * Render everything as you usually would, without texture binding. We're rendering the box from the
		 * '3D Objects' tutorial here.
		 */
		glBegin(GL_QUADS);
			/* Front */
			glTexCoord2d(0, 0); glVertex3d(400, 125, 0.4);
			glTexCoord2d(1, 0); glVertex3d(750, 125, 0.4);
			glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
			glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);

			/* Left side */
			glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
			glTexCoord2d(1, 0); glVertex3d(400, 125, 0.4);
			glTexCoord2d(1, 1); glVertex3d(400, 475, 0.4);
			glTexCoord2d(0, 1); glVertex3d(400, 475, -0.4);

			/* Back */
			glTexCoord2d(0, 0); glVertex3d(750, 125, -0.4);
			glTexCoord2d(1, 0); glVertex3d(400, 125, -0.4);
			glTexCoord2d(1, 1); glVertex3d(400, 475, -0.4);
			glTexCoord2d(0, 1); glVertex3d(750, 475, -0.4);

			/* Right side */
			glTexCoord2d(0, 0); glVertex3d(750, 125, 0.4);
			glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
			glTexCoord2d(1, 1); glVertex3d(750, 475, -0.4);
			glTexCoord2d(0, 1); glVertex3d(750, 475, 0.4);

			/* Top */
			glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
			glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
			glTexCoord2d(1, 1); glVertex3d(750, 125, 0.4);
			glTexCoord2d(0, 1); glVertex3d(400, 125, 0.4);

			/* Bottom */
			glTexCoord2d(0, 0); glVertex3d(400, 475, -0.4);
			glTexCoord2d(1, 0); glVertex3d(750, 475, -0.4);
			glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
			glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);
		glEnd();
	glEndList();

	return BoxList;
}

void DrawTable(unsigned int texture) {
	// Material property
	GLfloat tb_ambient[] = { 0.05, 0.05, 0.05, 1 };
	GLfloat tb_diffuse[] = { 0.8, 0.8, 0.8, 1 };
	GLfloat tb_specular[] = { 0.6, 0.6, 0.6, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tb_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tb_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tb_specular);
	
	// The four legs of the chair
	glPushMatrix();

	glTranslatef(200, 300, 0);
	glRotatef(180, 1, 0, 0);

	glScalef(20, 10, 0.05);

	glPushMatrix();
		GLUquadricObj *ob = gluNewQuadric();
		glBindTexture(GL_TEXTURE_2D, texture);
		gluQuadricTexture(ob, texture);
		glPushMatrix();
		glTranslatef(0, -20, -45);
		glRotatef(-90, 1, 0, 0);
		gluCylinder(ob, 0.5, 0.5, 10, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(1, -20, -53);
		glRotatef(-90, 1, 0, 0);
		gluCylinder(ob, 0.5, 0.5, 10, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(8, -20, -53);
		glRotatef(-90, 1, 0, 0);
		gluCylinder(ob, 0.5, 0.5, 10, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(8, -20, -45);
		glRotatef(-90, 1, 0, 0);
		gluCylinder(ob, 0.5, 0.5, 10, 20, 20);
		glPopMatrix();

		// Surface of the table
		glPushMatrix();
		glTranslatef(4, -9.5, -49);
		glScalef(1, 0.1, 1);
		glutSolidCube(10);
		glPopMatrix();
	glPopMatrix();


	glPopMatrix();
}


void DrawSpiningPlanet(double posX, double posY, double posZ, unsigned texture, double scale) {
	GLfloat tb_ambient[] = { 0.05, 0.05, 0.05, 1 };
	GLfloat tb_diffuse[] = { 0.8, 0.8, 0.8, 1 };
	GLfloat tb_specular[] = { 0.6, 0.6, 0.6, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tb_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tb_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tb_specular);

	GLUquadricObj *ob = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, texture);
	gluQuadricTexture(ob, texture);
	glPushMatrix();

	glLoadIdentity();
	glRotated(ViewAngleVer, 1, 0, 0);
	glRotated(ViewAngleHor, 0, 1, 0);
	glTranslated(-X, -Y, -Z);

	glRotatef(90, 1, 0, 0);
	planetSpinn = planetSpinn - 0.1/(19*scale);
	if (planetSpinn > 360) {
		planetSpinn = planetSpinn - 360;
	}

	glTranslatef(posX, posY, posZ);
	glRotatef(planetSpinn, 0, 0, 1);
	glScalef(-scale, scale, scale);

	gluSphere(ob, 1, 20, 20);
	glPopMatrix();
}

void DrawEarth(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[EARTH_TX], 0.3);
	glPopMatrix();
}

void DrawMoon(double posX, double posY, double posZ) {
	GLfloat tb_ambient[] = { 0.05, 0.05, 0.05, 1 };
	GLfloat tb_diffuse[] = { 0.8, 0.8, 0.8, 1 };
	GLfloat tb_specular[] = { 0.6, 0.6, 0.6, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tb_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tb_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tb_specular);

	GLUquadricObj *ob = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, Textures[MOON_TX]);
	gluQuadricTexture(ob, Textures[MOON_TX]);
	glPushMatrix();

	glLoadIdentity();
	glRotated(ViewAngleVer, 1, 0, 0);
	glRotated(ViewAngleHor, 0, 1, 0);
	glTranslated(-X, -Y, -Z);
	
	glRotatef(90, 1, 0, 0);
	moonOrbit = moonOrbit - 0.006;
	if (moonOrbit > 360) {
		moonOrbit = moonOrbit - 360;
	}

	
	glTranslatef(posX, posY, posZ);
	glRotatef(moonOrbit, 0, 0, 1);
	glTranslatef(0.6, 0, 0);
	glScalef(-0.1, 0.1, 0.1);

	gluSphere(ob, 1, 20, 20);
	glPopMatrix();
}

void MoonOrbit(double posX, double posY, double posZ) {
	DrawMoon(posX, posY, posZ);
}

// 
/// Animated planets
//
void DrawAnimatedEarth(double posX, double posY, double posZ) {
	// Draw a sphere with a eart texture
	glPushMatrix();
	DrawEarth(posX, posY, posZ);
	MoonOrbit(posX, posY, posZ);
	glPopMatrix();
}

void DrawAnimatedMars(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[MARS_TX], 0.29);
	glPopMatrix();
}

void DrawAnimatedJupiter(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[JUPITER_TX], 0.9);
	glPopMatrix();
}

void DrawAnimatedMercury(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[MERCURY_TX], 0.2);
	glPopMatrix();
}

void DrawAnimatedNeptune(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[NEPTUNE_TX], 0.5);
	glPopMatrix();
}

void DrawAnimatedSaturn(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[SATURN_TX], 0.7);

	GLfloat tb_ambient[] = { 0.05, 0.05, 0.05, 1 };
	GLfloat tb_diffuse[] = { 0.8, 0.8, 0.8, 1 };
	GLfloat tb_specular[] = { 0.6, 0.6, 0.6, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tb_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tb_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tb_specular);

	GLUquadricObj *ob = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, Textures[SATURN_RING_TX]);
	gluQuadricTexture(ob, Textures[SATURN_RING_TX]);
	glPushMatrix();

	glLoadIdentity();
	glRotated(ViewAngleVer, 1, 0, 0);
	glRotated(ViewAngleHor, 0, 1, 0);
	glTranslated(-X, -Y, -Z);

	glRotatef(90, 1, 0, 0);
	ringSpin = ringSpin - 0.01;
	if (ringSpin > 360) {
		ringSpin = ringSpin - 360;
	}


	glTranslatef(posX, posY, posZ);
	glRotatef(ringSpin, 0, 0, 1);
	glScalef(-1, 1, 1);

	gluDisk(ob, 0.8, 1.2, 20, 20);
	glPopMatrix();

	glPopMatrix();
}

void DrawAnimatedUranus(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[URANUS_TX], 0.45);
	glPopMatrix();
}

void DrawAnimatedVenus(double posX, double posY, double posZ) {
	glPushMatrix();
	DrawSpiningPlanet(posX, posY, posZ, Textures[VENUS_TX], 0.28);
	glPopMatrix();
}


void DrawBoxStack(unsigned boxList) {
	/* Now we're going to render some boxes using display lists. */
	glPushMatrix();
		/* Let's make it a bit smaller... */
		glScaled(0.5, 0.4, 0.5);

		/* Can't bind textures while generating a display list, but we can give it texture coordinates and bind it now. */
		glBindTexture(GL_TEXTURE_2D, Textures[2]);

		/*
		* Because display lists have preset coordinates, we'll need to translate it to move it around. Note that we're
		* moving the small version of the cube around, not the big version (because we scaled *before* translating).
		*/
		glTranslated(-700, 750, 10);

		/*
		* Let's draw a whole lot of boxes. Note that because we're not pushing and popping matrices, translations
		* and changes will 'accumulate' and add to the previous translation.
		*/
		for (short i(0); i < 12; ++i)
		{
			glTranslated(350, 0, 0);

			/* These make sure that every once in a while, a new row is started. */
			if (i == 5)		glTranslated(-1575, -350, 0);
			if (i == 9)		glTranslated(-1225, -350, 0);

			/*
			* glCallList is all that is really needed to execute the display list. Remember to try the 'K' button
			* to turn on wireframe mode, with these extra polygons, it looks pretty neat!
			*/
			glCallList(boxList);
		}

	glPopMatrix();
}

/*
 * DrawRoom
 *	This will render the entire scene (in other words, draw the room).
 */
void DrawRoom()
{
	glPushMatrix();
	/* You also could do this at front by using the SDL surface's values or in an array. */
	static float WallTexWidth(0.f);
	static float WallTexHeight(0.f);

	static float FloorTexWidth(0.f);
	static float FloorTexHeight(0.f);

	static bool Once(false);

	/* Perform this check only once. */
	if(!Once)
	{
		/* Bind the wall texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[0]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &WallTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &WallTexHeight);

		/* Bind the floor texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[1]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &FloorTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &FloorTexHeight);

		Once = true;
	}

		/* Draw walls. */
		glBindTexture(GL_TEXTURE_2D, Textures[0]);

		glBegin(GL_QUADS);
			/* Wall in front of you when the app starts. */
			glTexCoord2f(0, 0);
			glVertex3d(-500, 500 - roomHeight, roomLength/2.0);

			glTexCoord2f(1200.f / WallTexWidth, 0);
			glVertex3d(roomWidth - 500, 500 - roomHeight, roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
			glVertex3d(roomWidth - 500, 500, roomLength / 2.0);

			glTexCoord2f(0, 400.f / WallTexHeight);
			glVertex3d(-500, 500, roomLength / 2.0);

			/* Wall left of you. */
			glTexCoord2f(0, 0);
			glVertex3d(-500, 500 - roomHeight,-roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 0);
			glVertex3d(-500, 500 - roomHeight, roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
			glVertex3d(-500, 500, roomLength / 2.0);

			glTexCoord2f(0, 400.f / WallTexHeight);
			glVertex3d(-500, 500,-roomLength / 2.0);

			/* Wall right of you. */
			glTexCoord2f(0, 0);
			glVertex3d(roomWidth - 500, 500 - roomHeight, roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 0);
			glVertex3d(roomWidth - 500, 500 - roomHeight,-roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
			glVertex3d(roomWidth - 500, 500,-roomLength / 2.0);

			glTexCoord2f(0, 400.f / WallTexHeight);
			glVertex3d(roomWidth - 500, 500, roomLength / 2.0);

			/* Wall behind you (you won't be able to see this just yet, but you will later). */
			glTexCoord2f(0, 0);
			glVertex3d(roomWidth - 500, 500 - roomHeight, -roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 0);
			glVertex3d(-500, 500 - roomHeight, -roomLength / 2.0);

			glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
			glVertex3d(-500, 500, -roomLength / 2.0);

			glTexCoord2f(0, 400.f / WallTexHeight);
			glVertex3d(roomWidth - 500, 500, -roomLength / 2.0);
		glEnd();

		/* Draw the floor and the ceiling, this is done separatly because glBindTexture isn't allowed inside glBegin. */
		glBindTexture(GL_TEXTURE_2D, Textures[1]);

		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);
			glVertex3d(-500, 500, roomLength / 2.0);

			glTexCoord2f(1200.f / FloorTexWidth, 0);
			glVertex3d(roomWidth - 500, 500, roomLength / 2.0);

			glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
			glVertex3d(roomWidth - 500, 500, -roomLength / 2.0);

			glTexCoord2f(0, (8.f / 2.f * 600.f) / FloorTexHeight);
			glVertex3d(-500, 500, -roomLength / 2.0);

			/* Ceiling. */
			glTexCoord2f(0, 0);
			glVertex3d(-500, 500 - roomHeight, roomLength / 2.0);

			glTexCoord2f(1200.f / FloorTexWidth, 0);
			glVertex3d(roomWidth - 500, 500 - roomHeight, roomLength / 2.0);

			glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
			glVertex3d(roomWidth - 500, 500 - roomHeight, -roomLength / 2.0);

			glTexCoord2f(0, (8.f / 2.f * 600.f)  / FloorTexHeight);
			glVertex3d(-500, 500 - roomHeight, -roomLength / 2.0);
		glEnd();

		glPopMatrix();
}




int main(int argc, char **argv)
{
	/* Initialize SDL and set up a window. */
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_WM_SetCaption("Computer Graphics Project", 0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	SDL_ShowCursor(SDL_DISABLE);

	SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);


	

	/* Basic OpenGL initialization, handled in 'The Screen'. */
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 1);

	glViewport(0, 0, 800, 600);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(80.0, 800.0/600.0, 0.1, 1000.0);

	/* We now switch to the modelview matrix. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* Enable 2D texturing. */
	glEnable(GL_TEXTURE_2D);

	/* Set up alpha blending. */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4d(1, 1, 1, 1);

	Textures[0] = GrabTexObjFromFile("Data/Wall.png", GL_RGBA);
	Textures[1] = GrabTexObjFromFile("Data/Floor.png", GL_RGBA);
	Textures[2] = GrabTexObjFromFile("Data/Box.png", GL_RGBA);
	Textures[3] = GrabTexObjFromFile("Data/Table.jpg", GL_RGB);
	Textures[4] = GrabTexObjFromFile("Data/Earth.jpg", GL_RGB);
	Textures[5] = GrabTexObjFromFile("Data/Moon.jpg", GL_RGB);
	Textures[6] = GrabTexObjFromFile("Data/Jupiter.jpg", GL_RGB);
	Textures[7] = GrabTexObjFromFile("Data/Mars.jpg", GL_RGB);
	Textures[8] = GrabTexObjFromFile("Data/Mercury.jpg", GL_RGB);
	Textures[9] = GrabTexObjFromFile("Data/Neptune.jpg", GL_RGB);
	Textures[10] = GrabTexObjFromFile("Data/Saturn.jpg", GL_RGB);
	Textures[11] = GrabTexObjFromFile("Data/Saturn_ring.png", GL_RGBA);
	Textures[12] = GrabTexObjFromFile("Data/Uranus.jpg", GL_RGB);
	Textures[13] = GrabTexObjFromFile("Data/Venus.jpg", GL_RGB);

	//Replaced this with a loop that immediately checks the entire array.
	//sizeof(Textures) is the size of the entire array in bytes (unsigned int = 4 bytes)
	//so sizeof(Textures) would give 3 * 4 = 12 bytes, divide this by 4 bytes and you
	//have 3.
	for(unsigned i(0); i < sizeof(Textures) / sizeof(unsigned); ++i)
	{
		if(Textures[i] == 0)
		{
#ifdef _WIN32
		MessageBoxA(0, "Something went seriously wrong!", "Fatal Error!", MB_OK | MB_ICONERROR);
#endif //_WIN32

		return 1;
		}
	}

	/* Compile the display lists. */
	unsigned boxList = createBoxList();

	SDL_Event event;

	int RelX(0), RelY(0);
	int MovementDelay(SDL_GetTicks());

	bool Wireframe(false);
	bool Keys[6] =
	{
		false, /* Up arrow down? */
		false, /* Down arrow down? */
		false, /* Left arrow down? */
		false  /* Right arrow down? */
	};

	/* Application loop. */
	for(;;)
	{
		/* Handle events with SDL. */
		if(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				break;

			/* Mouse events? */
			else if(event.type == SDL_MOUSEMOTION)
			{
				/* Get the relative mouse movement of the mouse (based on CurMouseCoord - PrevMouseCoord). */
				SDL_GetRelativeMouseState(&RelX, &RelY);

				ViewAngleHor += RelX / 4;
				ViewAngleVer += RelY / 4;

				/* Prevent the horizontal angle from going over 360 degrees or below 0 degrees. */
				if(ViewAngleHor >= 360.0)		ViewAngleHor = 0.0;
				else if(ViewAngleHor < 0.0)		ViewAngleHor = 360.0;

				/* Prevent the vertical view from moving too far (comment this out to get a funny effect). */
				if(ViewAngleVer > 60.0)			ViewAngleVer = 60.0; /* 60 degrees is when you're looking down. */
				else if(ViewAngleVer < -60.0)	ViewAngleVer = -60.0; /* This is when you're looking up. */

				/* This delay might seem strange, but it helps smoothing out the mouse if you're experiencing jittering. */
				SDL_Delay(5);
			}

			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					break;

				if(event.key.keysym.sym == SDLK_k)
					glPolygonMode(GL_FRONT_AND_BACK, ((Wireframe = !Wireframe)? GL_LINE : GL_FILL));

				if(event.key.keysym.sym == SDLK_w)		Keys[0] = true;
				if(event.key.keysym.sym == SDLK_s)		Keys[1] = true;
				if(event.key.keysym.sym == SDLK_a)		Keys[2] = true;
				if(event.key.keysym.sym == SDLK_d)		Keys[3] = true;

				//q event
				if(event.key.keysym.sym == SDLK_q)		Keys[4] = true;
				if(event.key.keysym.sym == SDLK_e)		Keys[5] = true;
			}

			else if(event.type == SDL_KEYUP)
			{
				if(event.key.keysym.sym == SDLK_w)		Keys[0] = false;
				if(event.key.keysym.sym == SDLK_s)		Keys[1] = false;
				if(event.key.keysym.sym == SDLK_a)		Keys[2] = false;
				if(event.key.keysym.sym == SDLK_d)		Keys[3] = false;

				//q event
				if(event.key.keysym.sym == SDLK_q)		Keys[4] = false;
				if(event.key.keysym.sym == SDLK_e)		Keys[5] = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glPushMatrix();
			/* Move the world and rotate the view. */
			glRotated(ViewAngleVer, 1, 0, 0);
			glRotated(ViewAngleHor, 0, 1, 0);

			glTranslated(-X, -Y, -Z);

			/* Set the coordinate system. */
			glOrtho(0, 800, 600, 0, -1, 1);
			
			// Draw the stuff we want to
			// Default Room and boxes
			DrawRoom();
			DrawBoxStack(boxList);

			// Draw the table should also take the position in the room
			DrawTable(Textures[3]);
			// Choose the position of the earth with the animated moon
			DrawAnimatedMercury(0, -4, 0.1);
			DrawAnimatedVenus(0, -2, 0.1);
			DrawAnimatedEarth(0, 0, 0.1);
			DrawAnimatedMars(0, 2, 0.1);
			DrawAnimatedJupiter(0, 4, 0.1);
			DrawAnimatedSaturn(0, 6, 0.1);
			DrawAnimatedUranus(0, 8, 0.1);
			DrawAnimatedNeptune(0, 10, 0.1);

		glPopMatrix();
		



		/* Move if the keys are pressed, this is explained in the tutorial. */
		if(Keys[0])
		{
			X -= cos(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
			Z -= sin(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
		}

		if(Keys[1])
		{
			X += cos(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
			Z += sin(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
		}

		if(Keys[2])
		{
			X += cos(DegreeToRadian(ViewAngleHor + 180.0)) * movementSpeed;
			Z += sin(DegreeToRadian(ViewAngleHor + 180.0)) * movementSpeed;
		}

		if(Keys[3])
		{
			X -= cos(DegreeToRadian(ViewAngleHor + 180.0)) * movementSpeed;
			Z -= sin(DegreeToRadian(ViewAngleHor + 180.0)) * movementSpeed;
		}

		//q response: go up
		if(Keys[4])
		{
			Y += sin(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
		}

		//e response: go down
		if(Keys[5])
		{
			Y -= sin(DegreeToRadian(ViewAngleHor + 90.0)) * movementSpeed;
		}

		/* Swap the display buffers. */
		SDL_GL_SwapBuffers();
	}



	/* Delete the created textures. */
	glDeleteTextures(textureLength, Textures);
	glDeleteLists(boxList, 1);

	/* Clean up. */
	SDL_Quit();

	return 0;
}
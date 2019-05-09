/*
 * Modified: Duyen Ky Tran
 * Lab 04 
 * -Credits
 * -getScore, displayScore
 * CMPS3350
 *
 */
#include <stdio.h>
#include <bitset>   //bit string library
#include <string.h> //memset
#include <time.h>
#include <math.h> // ""
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "global.h"

//defined types
typedef double Vec[3];

//macros
#define rnd() (((double)rand())/(double)RAND_MAX)
#define random(a) (rand()%a)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]

//constants
const float timeslice = 1.0f;
const float gravity = -0.2f;
#define ALPHA 1

using namespace std;



/////////////////////////////////////////////////////////////

////////////___________RAIN_DISPLAY_________////////////////
static Global& gl = Global::getInstance();

class global {
public:
	int showRain;
	global() {
		logOpen();
		showRain=1;
	}
	~global() {
		logClose();
	}
} r;

class Raindrop {
public:
	int type;
	int linewidth;
	int sound;
	Vec pos;
	Vec lastpos;
	Vec vel;
	Vec maxvel;
	Vec force;
	float length;
	float color[4];
	Raindrop *prev;
	Raindrop *next;
} *rainhead = NULL;

int ndrops=1;
int totrain=0;
int maxrain=0;

void cleanupRaindrops()
{
	Raindrop *s;
	while (rainhead) {
		s = rainhead->next;
		free(rainhead);
		rainhead = s;
	}
	rainhead=NULL;
}
void deleteRain(Raindrop *node)
{
	//remove a node from linked list
	//Log("deleteRain()...\n");
	if (node->prev == NULL) {
		if (node->next == NULL) {
			//Log("only 1 item in list.\n");
			rainhead = NULL;
		} else {
			//Log("at beginning of list.\n");
			node->next->prev = NULL;
			rainhead = node->next;
		}
	} else {
		if (node->next == NULL) {
			//Log("at end of list.\n");
			node->prev->next = NULL;
		} else {
			//Log("in middle of list.\n");
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
	}
	free(node);
	node = NULL;
}

void createRaindrop(const int n)
{
	//create new rain drops...
	int i;
	for (i=0; i<n; i++) {
		Raindrop *node = (Raindrop *)malloc(sizeof(Raindrop));
		if (node == NULL) {
			Log("error allocating node.\n");
			exit(EXIT_FAILURE);
		}
		node->prev = NULL;
		node->next = NULL;
		node->sound=0;
		node->pos[0] = rnd() * (float)gl.xres;
		node->pos[1] = rnd() * 100.0f + (float)gl.yres;
		VecCopy(node->pos, node->lastpos);
		node->vel[0] = 
		node->vel[1] = 0.0f;
		node->color[0] = rnd() * 0.2f + 0.8f;
		node->color[1] = rnd() * 0.2f + 0.8f;
		node->color[2] = rnd() * 0.2f + 0.8f;
		node->color[3] = rnd() * 0.5f + 0.3f; //alpha
		node->linewidth = random(8)+1;
		//larger linewidth = faster speed
		node->maxvel[1] = (float)(node->linewidth*16);
		node->length = node->maxvel[1] * 0.2f + rnd();
		//put raindrop into linked list
		node->next = rainhead;
		if (rainhead != NULL)
			rainhead->prev = node;
		rainhead = node;
		++totrain;
	}
}

void checkRaindrops()
{
	if (random(100) < 50) {
		createRaindrop(ndrops);
	}
	//
	//move rain droplets
	Raindrop *node = rainhead;
	while (node) {
		//force is toward the ground
		node->vel[1] += gravity;
		VecCopy(node->pos, node->lastpos);
		node->pos[0] += node->vel[0] * timeslice;
		node->pos[1] += node->vel[1] * timeslice;
		if (fabs(node->vel[1]) > node->maxvel[1])
			node->vel[1] *= 0.96;
		node->vel[0] *= 0.999;
		node = node->next;
	}
	//}
	//
	//check rain droplets
	int n=0;
	node = rainhead;
	while (node) {
		n++;
		#ifdef USE_SOUND
		if (node->pos[1] < 0.0f) {
			//raindrop hit ground
			if (!node->sound && play_sounds) {
				//small chance that a sound will play
				int r = random(50);
				if (r==1) {
					//play sound here...


				}
				//sound plays once per raindrop
				node->sound=1;
			}
		}
		#endif //USE_SOUND
		//collision detection for raindrop on umbrella
		
		if (node->pos[1] < -20.0f) {
			//rain drop is below the visible area
			Raindrop *savenode = node->next;
			deleteRain(node);
			node = savenode;
			continue;
		}
		node = node->next;
	}
	if (maxrain < n)
		maxrain = n;
}
void displayRain(){
	if(r.showRain){
		checkRaindrops();
	}
};

void drawRaindrops()
{
	Raindrop *node = rainhead;
	while (node) {
		glPushMatrix();
		glTranslated(node->pos[0],node->pos[1],node->pos[2]);
		glColor4fv(node->color);
		glLineWidth(node->linewidth);
		glBegin(GL_LINES);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(0.0f, node->length);
		glEnd();
		glPopMatrix();
		node = node->next;
	}
	glLineWidth(1);
}

void callRaindrops(){
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	if (r.showRain)
		drawRaindrops();
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

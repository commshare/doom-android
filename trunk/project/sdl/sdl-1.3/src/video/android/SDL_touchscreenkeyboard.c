/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include <jni.h>
#include <android/log.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <netinet/in.h>

#include "SDL_config.h"

#include "SDL_version.h"

//#include "SDL_opengles.h"
#include "SDL_screenkeyboard.h"
#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"

#include "touchscreenfont.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

enum {
FONT_LEFT = 0, FONT_RIGHT = 1, FONT_UP = 2, FONT_DOWN = 3,
FONT_BTN1 = 4, FONT_BTN2 = 5, FONT_BTN3 = 6, FONT_BTN4 = 7
};

static GLshort fontGL[sizeof(font)/sizeof(font[0])][FONT_MAX_LINES_PER_CHAR * 4 + 1];
enum { FONT_CHAR_LINES_COUNT = FONT_MAX_LINES_PER_CHAR * 4 };

enum { MAX_BUTTONS = SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_MAX, MAX_BUTTONS_AUTOFIRE = 2 } ; // Max amount of custom buttons

int SDL_ANDROID_isTouchscreenKeyboardUsed = 0;
static int touchscreenKeyboardTheme = 1;
static int touchscreenKeyboardShown = 1;
static int DpadAsturn = 0;//if use dpad left/right direction to turn charactor
static int AutoFireButtonsNum = 0;
static int nbuttons = 4;
static int buttonsize = 1;
static int transparency = 128;

//niuzb define the positon of each icon controller
//arrow is the background cirle
//turn:store the left and right turning icon posiont
//buttons:store 4 icon. open door, fire, show gun, show map
static SDL_Rect arrows, turn[2], buttons[MAX_BUTTONS];
static SDLKey buttonKeysyms[MAX_BUTTONS] = { 
SDL_KEY(SPACE),//open door
0x30,//chang weapon
9,//tab...map
SDL_KEY(RCTRL),//j fire

// 4 and 5 are MENU and BACK, always available as HW keys
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_6)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_7)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_8))
};

static SDLKey arrowKey[] = {
	SDL_KEY(LEFT),//left
	SDL_KEY(RIGHT),//right
	SDL_KEY(UP),//up
	SDL_KEY(DOWN),//down
	SDL_KEY(COMMA),//strafe left
	SDL_KEY(PERIOD),//strafe right

};
enum { ARROW_LEFT = 1, ARROW_RIGHT = 2, ARROW_UP = 4, ARROW_DOWN = 8 };
static int oldArrows = 0;
static int oldTurn = 0;
static int ButtonAutoFire[MAX_BUTTONS_AUTOFIRE] = {0, 0};
static int ButtonAutoFireX[MAX_BUTTONS_AUTOFIRE*2] = {0, 0, 0, 0};
static int ButtonAutoFireRot[MAX_BUTTONS_AUTOFIRE] = {0, 0};
static int ButtonAutoFireDecay[MAX_BUTTONS_AUTOFIRE] = {0, 0};

static SDL_Rect * OldCoords[MAX_MULTITOUCH_POINTERS] = { NULL };

typedef struct
{
    GLuint id;
    GLfloat w;
    GLfloat h;
} GLTexture_t;
//0 store dpad, 1 store left right
static GLTexture_t arrowImages[5] = { {0, 0, 0}, };
static GLTexture_t buttonAutoFireImages[MAX_BUTTONS_AUTOFIRE*2] = { {0, 0, 0}, };
static GLTexture_t buttonImages[20] = { {0, 0, 0}, };
//conrol if we show the gun number
static int show_gun = 0;
//record witch gun is choosen now
static int pressed_num = -1;
static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}
struct timespec now;

long get_current_milisecond() {

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (long)now.tv_sec*1000 + now.tv_nsec/1000000L;

}

// Should be called on each char of font before drawing
static void prepareFontCharWireframe(int idx, int w, int h)
{
    int i, count = 0;
    float fw = (float) w / 255.0f;
    float fh = (float) h / 255.0f;

	//for( idx = 0; idx < sizeof(font)/sizeof(font[0]); idx++ )
	{
		for( i = 0; i < FONT_MAX_LINES_PER_CHAR; i++ )
			if( font[idx][i].x1 == 0 && font[idx][i].y1 == 0 && 
				font[idx][i].x2 == 0 && font[idx][i].y2 == 0 )
				break;
		count = i;
		for (i = 0; i < count; ++i) 
		{
			fontGL[idx][4*i+0] = (GLshort)(((int)font[idx][i].x1 - 128) * fw);
			fontGL[idx][4*i+1] = (GLshort)(((int)font[idx][i].y1 - 128) * fh);
			fontGL[idx][4*i+2] = (GLshort)(((int)font[idx][i].x2 - 128) * fw);
			fontGL[idx][4*i+3] = (GLshort)(((int)font[idx][i].y2 - 128) * fh);
		}
		fontGL[idx][FONT_CHAR_LINES_COUNT] = count*2;
	}
};


static inline void beginDrawingWireframe()
{
    glPushMatrix();
    glLoadIdentity();
    glOrthox( 0, SDL_ANDROID_sWindowWidth * 0x10000, SDL_ANDROID_sWindowHeight * 0x10000, 0, 0, 1 * 0x10000 );
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
}
static inline void endDrawingWireframe()
{
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
    glPopMatrix();
}

// Draws a char on screen using embedded line font, (x, y) are center of char, not upper-left corner
// TODO: use SDL 1.3 renderer routines? It will not be pixel-aligned then, if the screen is resized
static inline void drawCharWireframe(int idx, Uint16 x, Uint16 y, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    glColor4x(r * 0x100, g * 0x100, b * 0x100, a * 0x100);

    glVertexPointer(2, GL_SHORT, 0, fontGL[idx]);
    glPopMatrix();
    glPushMatrix();
    glTranslatex( x * 0x10000, y * 0x10000, 0 );
    if(rotation != 0)
        glRotatex( rotation, 0, 0, 0x10000 );
    glDrawArrays(GL_LINES, 0, fontGL[idx][FONT_CHAR_LINES_COUNT]);
}

static inline void beginDrawingTex()
{
	glEnable(GL_TEXTURE_2D);
}

static inline void endDrawingTex()
{
	/*
	GLfloat texColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texColor);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_BLEND);
	*/

	glDisable(GL_TEXTURE_2D);
}


static inline void drawCharTex(GLTexture_t * tex, SDL_Rect * src, SDL_Rect * dest, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	GLint cropRect[4];
	/*
	GLfloat texColor[4];
	static const float onediv255 = 1.0f / 255.0f;
	*/
	if( !dest->h || !dest->w )
		return;

	glBindTexture(GL_TEXTURE_2D, tex->id);

	glColor4x(r * 0x100, g * 0x100, b * 0x100,  a * 0x100 );

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	/*
	texColor[0] = r * onediv255;
	texColor[1] = g * onediv255;
	texColor[2] = b * onediv255;
	texColor[3] = a * onediv255;
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, texColor);
	*/

	cropRect[0] = 0;
	cropRect[1] = tex->h;
	cropRect[2] = tex->w;
	cropRect[3] = -tex->h;
	if(src)
	{
		cropRect[0] = src->x;
		cropRect[1] = src->h; // TODO: check if height works as expected in inverted GL coords
		cropRect[2] = src->w;
		cropRect[3] = -src->h; // TODO: check if height works as expected in inverted GL coords
	}
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
	glDrawTexiOES(dest->x, SDL_ANDROID_sWindowHeight - dest->y - dest->h, 0, dest->w, dest->h);
}

int SDL_ANDROID_drawTouchscreenKeyboard()
{
	int i;
	if( !SDL_ANDROID_isTouchscreenKeyboardUsed || !touchscreenKeyboardShown )
		return 0;

	{//niuzb we use theme 1
		int blendFactor =	( SDL_GetKeyboardState(NULL)[SDL_KEY(A)] ? 1 : 0 ) + 
							( SDL_GetKeyboardState(NULL)[SDL_KEY(D)] ? 1 : 0 ) +
							( SDL_GetKeyboardState(NULL)[SDL_KEY(W)] ? 1 : 0 ) +
							( SDL_GetKeyboardState(NULL)[SDL_KEY(S)] ? 1 : 0 );
		beginDrawingTex();
		//draw the dpad and turn arrow
		if( blendFactor == 0 ) {
	      drawCharTex( &arrowImages[0], NULL, &arrows, 255, 255, 255, transparency );
	      drawCharTex( &arrowImages[1], NULL, &turn[0], 255, 255, 255, transparency );
		  //drawCharTex( &arrowImages[2], NULL, &turn[1], 255, 255, 255, transparency );
    	}
		else
		{
		  drawCharTex( &arrowImages[0], NULL, &arrows, 255, 255, 255, transparency / blendFactor );
		  drawCharTex( &arrowImages[1], NULL, &turn[0], 255, 255, 255, transparency / blendFactor );
		  //drawCharTex( &arrowImages[2], NULL, &turn[1], 255, 255, 255, transparency / blendFactor );
    	}

		for( i = 0; i < nbuttons; i++ )
		{
		#if 0
			if( i < AutoFireButtonsNum )
			{
				if( ButtonAutoFire[i] == 1 && SDL_GetTicks() - ButtonAutoFireDecay[i] > 1000 )
				{
					ButtonAutoFire[i] = 0;
				}
				if( ! ButtonAutoFire[i] && SDL_GetTicks() - ButtonAutoFireDecay[i] > 300 )
				{
					if( ButtonAutoFireX[i*2] > 0 )
						ButtonAutoFireX[i*2] --;
					if( ButtonAutoFireX[i*2+1] > 0 )
						ButtonAutoFireX[i*2+1] --;
					ButtonAutoFireDecay[i] = SDL_GetTicks();
				}
			}

			if( i < AutoFireButtonsNum && ! ButtonAutoFire[i] && 
				( ButtonAutoFireX[i*2] > 0 || ButtonAutoFireX[i*2+1] > 0 ) )
			{
				int pos1src = buttonImages[i*2+1].w / 2 - ButtonAutoFireX[i*2];
				int pos1dst = buttons[i].w * pos1src / buttonImages[i*2+1].w;
				int pos2src = buttonImages[i*2+1].w - ( buttonImages[i*2+1].w / 2 - ButtonAutoFireX[i*2+1] );
				int pos2dst = buttons[i].w * pos2src / buttonImages[i*2+1].w;
				
				SDL_Rect autoFireCrop = { 0, 0, pos1src, buttonImages[i*2+1].h };
				SDL_Rect autoFireDest = buttons[i];
				autoFireDest.w = pos1dst;
				
				drawCharTex( &buttonImages[i*2+1],
							&autoFireCrop, &autoFireDest, 255, 255, 255, 128 );

				autoFireCrop.x = pos2src;
				autoFireCrop.w = buttonImages[i*2+1].w - pos2src;
				autoFireDest.x = buttons[i].x + pos2dst;
				autoFireDest.w = buttons[i].w - pos2dst;

				drawCharTex( &buttonImages[i*2+1],
							&autoFireCrop, &autoFireDest, 255, 255, 255, 128 );
				
				autoFireCrop.x = pos1src;
				autoFireCrop.w = pos2src - pos1src;
				autoFireDest.x = buttons[i].x + pos1dst;
				autoFireDest.w = pos2dst - pos1dst;

				drawCharTex( &buttonAutoFireImages[i*2+1],
							&autoFireCrop, &autoFireDest, 255, 255, 255, 128 );
			}
			else
#endif
			{
				drawCharTex(  &buttonImages[i],
							NULL, &buttons[i], 255, 255, 255, transparency );
        
			}
		}
    
    if(show_gun) {
          int i;
          SDL_Rect gun_position={0, 0, 32, 32};
          int tap = (SDL_ANDROID_sWindowWidth-32*10)/10;
          
          for(i = 0; i < 10; i++) {
            gun_position.x = tap/2+(32+tap)*i;
            drawCharTex(&buttonImages[i+4],
							NULL, &gun_position, 255, 255, 255, 128 );
          }
          
    }
		endDrawingTex();
	}
	return 1;
};

static inline int TurnKeyPressed(int x, int y)
{
	if(x < turn[0].x+turn[0].w/2)
		return 1;
	else
		return 2;
	
}
static inline int ArrowKeysPressed(int x, int y)
{
	int ret = 0, dx, dy;
	dx = x - arrows.x - arrows.w / 2;
	dy = y - arrows.y - arrows.h / 2;
	// Single arrow key pressed
	if( abs(dy / 2) >= abs(dx) )
	{
		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	else
	if( abs(dx / 2) >= abs(dy) )
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;
	}
	else // Two arrow keys pressed
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;

		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	return ret;
}

void SDL_ANDROID_reset_arrow_key() {
  SDL_keysym keysym;
  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[2], &keysym) );
  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[3], &keysym) );
  if(DpadAsturn == 0) {
	SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[4], &keysym) );
	SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[5], &keysym) );
  } else {
	SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[0], &keysym) );
	SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(  arrowKey[1], &keysym) );
  }
  

}

//weather usr touch the gun number, retern the number 
int SDL_ANDROID_get_number(int x, int y) {
  int num = -1;
  int tap = (SDL_ANDROID_sWindowWidth-32*10)/10;

  if(y < 32) {
    num = 1+ x/(tap+32);
  }
  return num;
}


//DpadAsturn
//#define SDL_SendKeyboardKey_TURN
//store the tick at whitch last move touch occur 
//use for dapd left/right dir icon
void SDL_send_dpad_key_event(int press, int dir) {
	SDL_keysym keysym;

	if(DpadAsturn == 1) {
		if(dir == ARROW_LEFT) 
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[0], &keysym) );
		else
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[1], &keysym) );
	} else if(DpadAsturn == 0) {
		if(dir == ARROW_LEFT) 
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[4], &keysym) );
		else
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[5], &keysym) );

		}
}

//used for turn icon
void SDL_send_turn_key_event(int press, int dir) {
	SDL_keysym keysym;

	if(DpadAsturn == 0) {
		if(dir == ARROW_LEFT) 
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[0], &keysym) );
		else
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[1], &keysym) );
	} else if(DpadAsturn == 1) {
		if(dir == ARROW_LEFT) 
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[4], &keysym) );
		else
			SDL_SendKeyboardKey( press, GetKeysym( arrowKey[5], &keysym) );

	}
}

int SDL_ANDROID_processTouchscreenKeyboard(int x, int y, int action, int pointerId)
{
	int i;
	SDL_keysym keysym;

	
	if( !touchscreenKeyboardShown )
		return 0;

	if( action == MOUSE_DOWN )
	{
	
		if( OldCoords[pointerId] )
		{
			SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_UP, pointerId);
		}
		//do the move stuff
		if( InsideRect( &arrows, x, y ) )
		{
			OldCoords[pointerId] = &arrows;
			//i = ArrowKeysPressed(x, y);
			i = ArrowKeysPressed(x, y);
			
	 		
			
			 SDL_ANDROID_reset_arrow_key();
			 if( i & ARROW_UP ) {
			   SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( arrowKey[2], &keysym) );
			   
			 }
			 if( i & ARROW_DOWN ) {
			   SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym(arrowKey[3], &keysym) );
			  
			 }
			 if( i & ARROW_LEFT ) {
			   		//SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(Q), &keysym) );
			   		SDL_send_dpad_key_event(SDL_PRESSED, ARROW_LEFT);
				 
			 }
			 if( i & ARROW_RIGHT ) {
				   //SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(E), &keysym) );
					SDL_send_dpad_key_event(SDL_PRESSED, ARROW_RIGHT);
				 
			 }
			 oldArrows = i;
			//__android_log_print(ANDROID_LOG_INFO, "doom", "down arrow %d",pointerId);
			return 1;
		} else if( InsideRect( &turn[0], x, y ) )
		{
			OldCoords[pointerId] = &turn[0]; 
			//this icon being pressed
			//oldTurn = 1;
			
			
			//we do not use this icon to turn
			if(DpadAsturn == 1) {
				
			 SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[4], &keysym) );
			 SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[5], &keysym) );
			} else {
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[0], &keysym) );
			 SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[1], &keysym) );
			}
			if(x < turn[0].x+turn[0].w/2) {
				//SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(A), &keysym) );
				SDL_send_turn_key_event(SDL_PRESSED, ARROW_LEFT);
				oldTurn = 1;
			}
			else { 
				//SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(D), &keysym) );
				SDL_send_turn_key_event(SDL_PRESSED, ARROW_RIGHT);
				oldTurn = 2;
			}
			
			//__android_log_print(ANDROID_LOG_INFO, "doom", "down turn %d",pointerId);
			return 1;
		}
#if 0
		else if( InsideRect( &turn[1], x, y ) )
		{
			OldCoords[pointerId] = &turn[1];
			SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(D), &keysym) );
			return 1;
		}
#endif
	    if(show_gun) {
	      int num = SDL_ANDROID_get_number(x, y);
	      
	      if (num > 0) {
	        //__android_log_print(ANDROID_LOG_INFO, "libSDL", "pressed num:%d", pressed_num);
	        if(num == 10) {
#if 0
				//god mode
#define sendkey(x) \
	do {\
		SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(x), &keysym) );\
		SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(x), &keysym) );\
	}while(0)
				sendkey(I);
				sendkey(D);
				sendkey(D);
				sendkey(Q);
				sendkey(D);
#endif
				SDL_ANDROID_ToggleScreenKeyboardTextInput();
				return 1;
			}
			
		  	//change the gun
	        pressed_num = num;
	        OldCoords[pointerId] = &pressed_num;
	        GetKeysym( SDL_KEY(0), &keysym);
	        keysym.scancode += num;
	        keysym.sym += num;
	        SDL_SendKeyboardKey( SDL_PRESSED, &keysym );
	        return 1;
	      }
	    }
		for( i = 0; i < nbuttons; i++ )
		{
			if( InsideRect( &buttons[i], x, y) )
			{
				OldCoords[pointerId] = &buttons[i];
		        if (i == 1) {
		          //switch gun
		          show_gun = show_gun==0 ? 1 : 0;
		          return 1;
		        }
        		//SDL_keysym keysym;
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym(buttonKeysyms[i], &keysym) );
				if( i < AutoFireButtonsNum )
				{
					ButtonAutoFire[i] = 0;
					if(touchscreenKeyboardTheme == 0)
					{
						ButtonAutoFireX[i] = x;
						ButtonAutoFireRot[i] = 0;
					}
					else
					{
						ButtonAutoFireX[i*2] = 0;
						ButtonAutoFireX[i*2+1] = 0;
						ButtonAutoFireRot[i] = x;
						ButtonAutoFireDecay[i] = SDL_GetTicks();
					}
				}
				return 1;
			}
		}

    
	}
	else if( action == MOUSE_UP )
	{
		if( OldCoords[pointerId] == &arrows)
		{
			OldCoords[pointerId] = NULL;
			//we draw use arrows_move, when touch up.return to the center
      		//arrows_move = arrows_cricle;
      		
         	SDL_ANDROID_reset_arrow_key();
      		oldArrows = 0;
			//__android_log_print(ANDROID_LOG_INFO, "doom", "up arrow %d",pointerId);
			return 1;
		} else if( OldCoords[pointerId] == &turn[0]) //stop turn left
		{
			OldCoords[pointerId] = NULL;
			if(DpadAsturn == 1) {
			  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[4], &keysym) );
			  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[5], &keysym) );
			} else {
			  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[1], &keysym) );
			  SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[0], &keysym) );
			}
			oldTurn = 0;
			//__android_log_print(ANDROID_LOG_INFO, "doom", "up turn %d",pointerId);
			return 1;
		} 
#if 0
		else if( OldCoords[pointerId] == &turn[1])//stop turn right
		{
			OldCoords[pointerId] = NULL;
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(D), &keysym) );
			return 1;
		} 
#endif	
		for( i = 0; i < nbuttons; i++ )
		{
			if( OldCoords[pointerId] == &buttons[i] )
			{
			
		        if(i == 1) {
		          OldCoords[pointerId] = NULL;
		          //show_gun = 0;
		          return 1;
		        }
				if( ! ( i < AutoFireButtonsNum && ButtonAutoFire[i] ) )
				{
					SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(buttonKeysyms[i] ,&keysym) );
				}
				else
				{
					ButtonAutoFire[i] = 2;
				}
				OldCoords[pointerId] = NULL;
				if(touchscreenKeyboardTheme == 0)
				{
					ButtonAutoFireX[i] = 0;
				}
				else
				{
					ButtonAutoFireX[i*2] = 0;
					ButtonAutoFireX[i*2+1] = 0;
				}
				return 1;
			}
		}
    	//process the gun number
	    if(show_gun && OldCoords[pointerId] == &pressed_num) {
	      //int num = SDL_ANDROID_get_number(x, y);
	      if (pressed_num>0) {
	        
	        GetKeysym( SDL_KEY(0), &keysym);
	        keysym.scancode += pressed_num;
	        keysym.sym += pressed_num;
	        SDL_SendKeyboardKey( SDL_RELEASED, &keysym );
	        //SDL_SendKeyboardKey(SDL_RELEASED, GetKeysym( SDL_KEY(0), &keysym)+pressed_num );
	        pressed_num = -1;
	        return 1;
	      }
	    }
	}
	else if( action == MOUSE_MOVE )
	{

		if( OldCoords[pointerId] && !InsideRect(OldCoords[pointerId], x, y) )
		{
			SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_UP, pointerId);
			return SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
		}

     if( OldCoords[pointerId] == &arrows )
	 {
		i = ArrowKeysPressed(x, y);
		if( i == oldArrows )
			return 1;
		

		if( oldArrows & ARROW_UP && ! (i & ARROW_UP) )
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[2], &keysym) );
		if( oldArrows & ARROW_DOWN && ! (i & ARROW_DOWN) )
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( arrowKey[3], &keysym) );
		if( oldArrows & ARROW_LEFT && ! (i & ARROW_LEFT) ) {
			   //SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(Q), &keysym) );
			   SDL_send_dpad_key_event(SDL_RELEASED, ARROW_LEFT);
		}
		if( oldArrows & ARROW_RIGHT && ! (i & ARROW_RIGHT) ) {
			   //SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(E), &keysym) );
			   SDL_send_dpad_key_event(SDL_RELEASED, ARROW_RIGHT);

		}
		if( i & ARROW_UP )
			SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( arrowKey[2], &keysym) );
		if( i & ARROW_DOWN )
			SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( arrowKey[3], &keysym) );
		if( i & ARROW_LEFT ) {
			   //SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(Q), &keysym) );
			SDL_send_dpad_key_event(SDL_PRESSED, ARROW_LEFT);
		}
		if( i & ARROW_RIGHT ) {
			  // SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(E), &keysym) );
			SDL_send_dpad_key_event(SDL_PRESSED, ARROW_RIGHT);
		}
		oldArrows = i;
	} else if( OldCoords[pointerId] == &turn[0]) //read turn left icon
	{
		//not use this code
		i = TurnKeyPressed(x, y);
		if( i == oldTurn)
			return 1;
		if(oldTurn == 1 && i == 2) {
			//SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(A), &keysym) );
			//SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(D), &keysym) );
			SDL_send_turn_key_event(SDL_RELEASED, ARROW_LEFT);
			SDL_send_turn_key_event(SDL_PRESSED, ARROW_RIGHT);
		} else if(oldTurn == 2 && i == 1) {
			//SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(D), &keysym) );
			//SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(A), &keysym) );
			SDL_send_turn_key_event(SDL_RELEASED, ARROW_RIGHT);
			SDL_send_turn_key_event(SDL_PRESSED, ARROW_LEFT);

		}
		oldTurn =  i;
	} 
#if 0
	else if( OldCoords[pointerId] == &turn[1])
	{
		if( !InsideRect( &turn[1], x, y ) ) {
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(D), &keysym) );
		}
	} 
#endif
	else
	{
		for(i = 0; i < AutoFireButtonsNum; i++)
		if( OldCoords[pointerId] == &buttons[i] )
		{
			if(touchscreenKeyboardTheme == 0)
			{
				ButtonAutoFire[i] = abs(ButtonAutoFireX[i] - x) > buttons[i].w / 2;
				if( !ButtonAutoFire[i] )
					ButtonAutoFireRot[i] = ButtonAutoFireX[i] - x;
			}
			else
			{
				int coeff = (buttonAutoFireImages[i*2+1].w > buttons[i].w) ? buttonAutoFireImages[i*2+1].w / buttons[i].w + 1 : 1;
				if( ButtonAutoFireRot[i] < x )
					ButtonAutoFireX[i*2+1] += (x - ButtonAutoFireRot[i]) * coeff;
				if( ButtonAutoFireRot[i] > x )
					ButtonAutoFireX[i*2] += (ButtonAutoFireRot[i] - x) * coeff;

				ButtonAutoFireRot[i] = x;

				if( ButtonAutoFireX[i*2] < 0 )
					ButtonAutoFireX[i*2] = 0;
				if( ButtonAutoFireX[i*2+1] < 0 )
					ButtonAutoFireX[i*2+1] = 0;
				if( ButtonAutoFireX[i*2] > buttonAutoFireImages[i*2+1].w / 2 )
					ButtonAutoFireX[i*2] = buttonAutoFireImages[i*2+1].w / 2;
				if( ButtonAutoFireX[i*2+1] > buttonAutoFireImages[i*2+1].w / 2 )
					ButtonAutoFireX[i*2+1] = buttonAutoFireImages[i*2+1].w / 2;

				if( ButtonAutoFireX[i*2] == buttonAutoFireImages[i*2+1].w / 2 &&
					ButtonAutoFireX[i*2+1] == buttonAutoFireImages[i*2+1].w / 2 )
				{
					if( ! ButtonAutoFire[i] )
						ButtonAutoFireDecay[i] = SDL_GetTicks();
					ButtonAutoFire[i] = 1;
				}
			}
		}
	}

	if( OldCoords[pointerId] )
		return 1;

		return SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
	}
	return 0;
};


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboard) ( JNIEnv*  env, jobject thiz, jint size, jint tt, jint _nbuttons, 
jint array_invalide, jint _transparency, jintArray arr )
{
	int i, ii;
	int nbuttons1row, nbuttons2row;
	nbuttons = _nbuttons;
//	touchscreenKeyboardTheme = theme;
	DpadAsturn = tt;

	if( nbuttons > MAX_BUTTONS )
		nbuttons = MAX_BUTTONS;
	//AutoFireButtonsNum = nbuttonsAutoFire;
	if( AutoFireButtonsNum > MAX_BUTTONS_AUTOFIRE )
		AutoFireButtonsNum = MAX_BUTTONS_AUTOFIRE;
	// TODO: works for horizontal screen orientation only!
	buttonsize = size;
	#if 0 
	//no need to just trasparency
	switch(_transparency)
	{
		case 0: transparency = 16; break;
		case 1: transparency = 32; break;
		case 2: transparency = 128; break;
		case 3: transparency = 150; break;
		case 4: transparency = 192; break;
		default: transparency = 128; break;
	}
	#endif
	if(array_invalide)
	{
	__android_log_print(ANDROID_LOG_INFO, "doom", "use default icon");
		//determine the positon of each icon
		// Arrows to the lower-left part of screen
		int scale = SDL_ANDROID_sWindowHeight/480;
		float iconscale = (size+1.0)/3.0;
		int tempx, tempy;
		#if 0
		arrows.x = SDL_ANDROID_sWindowWidth / 4;
		arrows.y = SDL_ANDROID_sWindowHeight - SDL_ANDROID_sWindowWidth / 5;
		arrows.w = SDL_ANDROID_sWindowWidth / (size + 2);
		arrows.h = arrows.w;
		arrows.x -= arrows.w/2;
		arrows.y -= arrows.h/2;
		// Move arrows from the center of the screen
		arrows.x -= size * SDL_ANDROID_sWindowWidth / 32;
		arrows.y += size * SDL_ANDROID_sWindowWidth / 32;
		#endif
		//lprintf(LO_INFO, "**********icon size:%f", iconscale);
		#if 0
		arrows.w = (int)(250.*iconscale);
		arrows.h = (int)(200.*iconscale);
		tempx = 35+arrows.w/2;
		tempy = SDL_ANDROID_sWindowHeight-30 - arrows.h/2;
		setRect(&arrows, tempx, tempy, arrows.w, arrows.h);
		
		turn[0].w = (int)(250.*iconscale);
		turn[0].h = (int)(80.*iconscale);
		tempx = SDL_ANDROID_sWindowWidth-30 - turn[0].w/2;
		tempy = SDL_ANDROID_sWindowHeight-30 - turn[0].h/2;
			
		setRect(&turn[0], tempx, tempy, turn[0].w, turn[0].h);
		#endif
		
		//int tempx, tempy;
		arrows.w = (int)(323.*iconscale);
		arrows.h = (int)(200.*iconscale);
		tempx = SDL_ANDROID_sWindowWidth/30+arrows.w/2;
		tempy = SDL_ANDROID_sWindowHeight*15/16 - arrows.h/2;
		setRect(&arrows, tempx, tempy, arrows.w, arrows.h);
		//setRect(&turn[1], arrows.x+arrows.w*3/4+2,  arrows.y-arrows.w/8, arrows.w/2, arrows.w/4);
		

    
		//postion the buttons
	    
        int w=(int)(80*iconscale);
		tempy = SDL_ANDROID_sWindowHeight/10+w/2;
		//turn left/right
		setRect(&turn[0], SDL_ANDROID_sWindowWidth-w*3/2, 
		MIN(SDL_ANDROID_sWindowHeight*15/16 - arrows.h/2, SDL_ANDROID_sWindowHeight-w-10)-(int)(60.*iconscale)-w/2-15, 2*w, w);
        //open door
        setRect(&buttons[0], SDL_ANDROID_sWindowWidth-w/2, SDL_ANDROID_sWindowHeight-w/2, w, w);
        //switch gun
        setRect(&buttons[1], SDL_ANDROID_sWindowWidth/30 + w/2, tempy, w, w);
        //map
        setRect(&buttons[2], SDL_ANDROID_sWindowWidth-w/2, tempy, w, w);
        //fire
        setRect(&buttons[3], MIN(SDL_ANDROID_sWindowWidth-(int)(100.*iconscale),SDL_ANDROID_sWindowWidth-w-10), 
        MIN(SDL_ANDROID_sWindowHeight*15/16 - arrows.h/2, SDL_ANDROID_sWindowHeight-w-10), (int)(100.*iconscale), (int)(100.*iconscale));
	    
	} else {
		 int i;
  		 jsize len = (*env)->GetArrayLength(env, arr);
		 jint *body = (*env)->GetIntArrayElements(env, arr, 0);
		 
		 __android_log_print(ANDROID_LOG_INFO, "doom", "use user set icon");
		 #define SETRect(r) \
		   do {\
		   (r).x = body[i];    \
		   (r).y = body[i+1]; \
		   (r).w = body[i+2]-body[i];  \
		   (r).h = body[i+3] - body[i+1];  \
		   }while(0)
		 for (i=0; i<len; i+=4) {
		 	if(i==0) {
				SETRect(arrows);
			}
			else if(i==4){
				SETRect(turn[0]);
			}
			else {
				SETRect(buttons[(i-8)/4]);
			}
        	//sum += body[i];
    	 }
		 (*env)->ReleaseIntArrayElements(env, arr, body, 0);
	}
};
//set position for a SDL_Rect
//x, y : the centor position of the rect
void setRect(SDL_Rect *r, int x, int y, int w, int h) {
  r->w = w;
  r->h = h;
  r->x = x-w/2;
  r->y = y-h/2;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTouchscreenKeyboardUsed) ( JNIEnv*  env, jobject thiz)
{
	SDL_ANDROID_isTouchscreenKeyboardUsed = 1;
}

static int
power_of_2(int input)
{
    int value = 1;

    while (value < input) {
        value <<= 1;
    }
    return value;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboardButton) ( JNIEnv*  env, jobject thiz, jint buttonID, jbyteArray charBufJava )
{
	// TODO: softstretch with antialiasing
	jboolean isCopy = JNI_TRUE;
	Uint8 * charBuf = NULL;
	int w, h, len, format;
	GLTexture_t * data = NULL;
	int texture_w, texture_h;
	len = (*env)->GetArrayLength(env, charBufJava);
	charBuf = (Uint8 *) (*env)->GetByteArrayElements(env, charBufJava, &isCopy);
	
	w = ntohl(((Uint32 *) charBuf)[0]);
	h = ntohl(((Uint32 *) charBuf)[1]);
	format = ntohl(((Uint32 *) charBuf)[2]);
	if( buttonID < 5 ) {
    	data = &(arrowImages[buttonID]);
    }
	else
	if( buttonID < 9 )
		data = &(buttonAutoFireImages[buttonID-5]);
	else
		data = &(buttonImages[buttonID-9]);
	
	texture_w = power_of_2(w);
	texture_h = power_of_2(h);
	data->w = w;
	data->h = h;

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &data->id);
	glBindTexture(GL_TEXTURE_2D, data->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA,
					format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1, NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
						format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1,
						charBuf + 12 );

	glDisable(GL_TEXTURE_2D);

	(*env)->ReleaseByteArrayElements(env, charBufJava, (jbyte *)charBuf, 0);
}


int SDL_ANDROID_SetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId > SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_MAX || ! pos )
		return 0;
	
	if( buttonId == SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_DPAD )
	{
		arrows = *pos;
		if(touchscreenKeyboardTheme == 0)
		{
			prepareFontCharWireframe(FONT_LEFT, arrows.w / 2, arrows.h / 2);
			prepareFontCharWireframe(FONT_RIGHT, arrows.w / 2, arrows.h / 2);
			prepareFontCharWireframe(FONT_UP, arrows.w / 2, arrows.h / 2);
			prepareFontCharWireframe(FONT_DOWN, arrows.w / 2, arrows.h / 2);
		}
	}
	else
	{
		buttons[buttonId - SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_0] = *pos;
		if(touchscreenKeyboardTheme == 0)
		{
			int i = buttonId - SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_0;
			prepareFontCharWireframe(FONT_BTN1 + i, MIN(buttons[i].h, buttons[i].w), MIN(buttons[i].h, buttons[i].w));
		}
	}
	return 1;
};

int SDL_ANDROID_GetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId > SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_MAX || ! pos )
		return 0;
	
	if( buttonId == SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_DPAD )
	{
		*pos = arrows;
	}
	else
	{
		*pos = buttons[buttonId - SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_0];
	}
	return 1;
};

int SDL_ANDROID_SetScreenKeyboardButtonKey(int buttonId, SDLKey key)
{
	if( buttonId < 0 || buttonId > SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_MAX || ! key )
		return 0;
	if( buttonId == SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_DPAD )
		return 0;
	buttonKeysyms[buttonId - SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_0] = key;
	return 1;
};

SDLKey SDL_ANDROID_GetScreenKeyboardButtonKey(int buttonId)
{
	if( buttonId < 0 || buttonId > SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_MAX )
		return SDLK_UNKNOWN;
	if( buttonId == SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_DPAD )
		return SDLK_UNKNOWN;
	return buttonKeysyms[buttonId - SDL_ANDRIOD_SCREENKEYBOARD_BUTTON_0];
};

int SDL_ANDROID_SetScreenKeyboardAutoFireButtonsAmount(int nbuttons)
{
	if( nbuttons < 0 || nbuttons > MAX_BUTTONS_AUTOFIRE )
		return 0;
	AutoFireButtonsNum = nbuttons;
	return 1;
};

int SDL_ANDROID_GetScreenKeyboardAutoFireButtonsAmount()
{
	return AutoFireButtonsNum;
};

int SDL_ANDROID_SetScreenKeyboardShown(int shown)
{
	touchscreenKeyboardShown = shown;
};

int SDL_ANDROID_GetScreenKeyboardShown()
{
	return touchscreenKeyboardShown;
};

int SDL_ANDROID_GetScreenKeyboardSize()
{
	return buttonsize;
};

int SDL_ANDROID_ToggleScreenKeyboardTextInput()
{
	SDL_ANDROID_CallJavaShowScreenKeyboard();
	return 1;
};


// Wator.cpp : Defines the entry point for the console application.
//

#include <GL/gl.h>		//include OpnGL Dependencies
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>		//include Standard Library Dependencies
#include <time.h>





typedef int bool;
#define true 1				//Define Bool
#define false 0

#define sX 		1600		// Screen maxX
#define sY 		800		// Screen maxY. Both used for setting the VP.

#define width 100
#define height 100

//..............................................................
typedef struct{

	short m_x;
	short m_y;

}Position;

//..............................................................
typedef struct{

	Position m_pos;
	short m_breed;
	short m_starve;
	bool m_moved;
	
}Shark;


//..............................................................
typedef struct{

	Position m_pos;
	short m_breed;
	bool m_moved;

}Fish;


//..............................................................
typedef struct{

	Shark* m_shark;
	Fish* m_fish;
	bool m_updated;
	bool occupied;
}Map;

//.............................................................
typedef struct{

	int x;
	int y;

}Camera;


//..............................................................
#define ROWS 5000
#define COLUMNS 5000
#define maxFishBreedTime 8
#define maxSharkBreedTime 25
#define sharkStarveTime 12

//..............................................................

void initaliseWorld();
void updateMap();
void destroyMap();

Shark* createShark( short x, short y );
void updateSharks();
void sharkMove(short, short );
void breed( char type, short x, short y );
void killShark( );

Fish* createFish( short x, short y  );
void updateFish();
void moveFish( short, short );
void killFish( );

void draw();
short wrapAround(short );
bool sharkHunt( short, short );

//..............................................................
Map map[ROWS][COLUMNS];
bool running;
short totalSharks;
short totalFish;

int frameCount;
int currentTime;
int previousTime;
int fps;

// Opengl SDL..........................
SDL_Surface*    Surf_Display;
SDL_Event mEvent;
Camera cam;
void DrawGrid();
void event(SDL_Event* Event);




int main(void)
{
	char *myargv [1];
	int myargc=1;
	myargv [0]=strdup ("WATOR");
	glutInit(&myargc, myargv);
	cam.x = 0;
	cam.y = 0;

	srand( time( NULL ) );

	running = true;
	totalSharks = 3500;
	totalFish = 25000;
	initaliseWorld();

	while ( running )
	{
		updateMap();

		//Increase frame count
	        frameCount++;
	 
	        //  Get the number of milliseconds since glutInit called
	        //  (or first call to glutGet(GLUT ELAPSED TIME)).
	        currentTime = glutGet(GLUT_ELAPSED_TIME);
	 
	        //  Calculate time passed
	        int timeInterval = currentTime - previousTime;
	 
	        if(timeInterval > 1000)
	        {
	             //  calculate the number of frames per second
	             fps = frameCount / (timeInterval / 1000.0f);
	 
	             //  Set time
	             previousTime = currentTime;
	 
	             //  Reset frame count
	             frameCount = 0;
		     printf("#########################################################\n");
		     printf("FPS = %d\n", fps);
	        }
		
		//DrawGrid();
		
		while( SDL_PollEvent( &mEvent ) ){

			event(&mEvent);
		}
		

		GLint err = glGetError(); 
		if ( err != GL_NO_ERROR ){
		
			printf( "There was an opengl error, code : %d",err);
			return(0);
		}
		

		// Update the projection.
	    	glMatrixMode(GL_PROJECTION);
	    	glLoadIdentity();
	    	glOrtho(cam.x, cam.x + sX, cam.y + sY, cam.y, 10, -10);
	    	glMatrixMode(GL_MODELVIEW);	
	}

	return (0);
}

void DrawGrid(){

	int x,y,boxSize,space;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    	glLoadIdentity();

	boxSize = 4;
	space = 1;
	
	glTranslatef( 100,100 - (boxSize+space),0 );
	
	for( y = 0; y < height; y++ ){

		for( x = 0; x < width; x++ ){			

			//glColor3d(1, 1, 1);
			if(map[x][y].m_shark != 0)
			{
				glColor3d(1, 0, 0);
			}
			else if (map[x][y].m_fish != 0)
			{
				glColor3d(0, 0, 1);
			}
			else
			{
				glColor3d(1, 1, 1);
			}
			
			glTranslatef( boxSize+space,0,0 );

		    	glBegin(GL_QUADS);
				glVertex3d(-boxSize/2,  boxSize/2,  0);
				glVertex3d( boxSize/2,  boxSize/2,  0);
				glVertex3d( boxSize/2, -boxSize/2,  0);
				glVertex3d(-boxSize/2, -boxSize/2,  0);
		    	glEnd();
		}
		glLoadIdentity();	
		glTranslatef( 100,100,0 );
		glTranslatef( 0,y*(boxSize+space),0 );
 	}
	
	//SDL_GL_SwapBuffers which basically takes the buffer and moves it to the primary display.
    	SDL_GL_SwapBuffers();
	

}// End func.

void initaliseWorld()
{
	//srand( (int)time(NULL) );
	short x = 0;
	short y = 0;
	short i = 0;

	#pragma omp parallel for
	for(;i < totalSharks; i++ )
	{
		x = rand() % ROWS;
		y = rand() % COLUMNS;
		while( map[x][y].occupied == true )
		{
			x = rand() % ROWS;
			y = rand() % COLUMNS;
		}
		map[x][y].m_shark = createShark(x,y);
		map[x][y].occupied = true;
	}

	i=0;
	
	#pragma omp parallel for
	for(;i < totalFish; i++ )
	{
		x = rand() % ROWS;
		y = rand() % COLUMNS;
		while( map[x][y].occupied == true )
		{
			x = rand() % ROWS;
			y = rand() % COLUMNS;
		}
		map[x][y].m_fish = createFish(x,y);
		map[x][y].occupied = true;
	}

	Surf_Display = NULL;

	// We are telling SDL to Initialize everything it has.
	if( SDL_Init( SDL_INIT_EVERYTHING ) < 0) {
		printf("Something went wrong in the SDL init function : %s\n", SDL_GetError() );
        	
    	}

    	if( (Surf_Display = SDL_SetVideoMode( sX, sY, 32, SDL_OPENGL)) == NULL) {
        	printf("Couldn't set GL mode: %s\n", SDL_GetError() );
		
    	}
 	
	// OpenGL.

	// The colour of the Window.(R,G,B,A)
	glClearColor(0,0,0.5, 0);
 	
	// The view port size. (X,Y,W,H)
        glViewport(0, 0, sX, sY);
 	
	// Projection mode.
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
 
	// Orthographic projection.(L,R,Top,Bott,Near,Far)
    	glOrtho(cam.x, cam.x + sX, cam.y + sY, cam.y, 10, -10);
 	
	// Switch to model view.
    	glMatrixMode( GL_MODELVIEW );
 	
	// Texturing.
    	glEnable( GL_TEXTURE_2D );
}


void updateMap(){
	short x = 0;
	short y = 0;

	#pragma omp parallel for
	for( x=0; x < COLUMNS; x++)
		for( y=0; y < ROWS; y++)
			map[x][y].m_updated = false;

	updateSharks();

	updateFish();
}



Shark* createShark(short x, short y){
	Shark* newShark = malloc( sizeof( Shark ) );
	newShark->m_breed = rand() % maxSharkBreedTime;
	newShark->m_pos.m_x = x;
	newShark->m_pos.m_y = y;
	newShark->m_starve = sharkStarveTime;
	return newShark;
}


void updateSharks(){
	int x = 0;
	int y = 0;

	for( x=0;x< ROWS; x++)
	{
		for( y=0;y<COLUMNS;y++)
		{
			if( !map[x][y].m_updated )
			{
				if( map[x][y].m_shark != 0 )
				{
					// Hunt
					if ( sharkHunt( x , y ) )
					{
		
					}
					else
					{
						// Move the shark.
						sharkMove( x , y );
					}

				}
			}
		}
	}
}


bool sharkHunt( short x, short y ){
	bool ate = false;

	// Check n,e,s,w
	short newElement = 0;
	short randomPath = 0;

	randomPath = rand() % 4;
	switch(randomPath)
	{
			// North
			case(0):
			if( map[x][wrapAround( y - 1 )].m_fish != 0 )
			{
				newElement = wrapAround( y - 1 );
				// Copy the Shark to the new position.
				map[x][newElement].m_shark = map[x][y].m_shark;
				// Set the old Shark position to null
				map[x][y].m_shark = 0;
				// Set the old dead fish to 0
				map[x][newElement].m_fish = 0;
				// Update the sharks starve time as he just ate.
				map[x][newElement].m_shark->m_starve = sharkStarveTime;
				map[x][newElement].m_shark->m_breed -= 1;
				if( map[x][newElement].m_shark->m_breed <= 0)
				{
					breed( 's' , x, y);
					map[x][newElement].m_shark->m_breed = maxSharkBreedTime;
				}		
				// Mark the tile as updated for this turn.
				map[x][newElement].m_updated = true;	
				ate = true;						
			}		
			break;
			case(1):	
			if( map[ wrapAround( x + 1 ) ][ y ].m_fish != 0 )
			{
				newElement = wrapAround( x + 1 );
				// Copy the Shark to the new position.
				map[newElement][y].m_shark = map[x][y].m_shark;
				// Set the old Shark position to null
				map[x][y].m_shark = 0;
				// Set the old dead fish to 0
				map[newElement][y].m_fish = 0;
				// Update the sharks starve time as he just ate.
				map[newElement][y].m_shark->m_starve = sharkStarveTime;
				map[newElement][y].m_shark->m_breed -= 1;
				if( map[newElement][y].m_shark->m_breed <= 0)
				{
					breed( 's' , x, y);
					map[newElement][y].m_shark->m_breed = maxSharkBreedTime;
				}	
				// Mark the tile as updated for this turn.
				map[newElement][y].m_updated = true;
				ate = true;
			}	
			break;
			case(2):		
			if( map[ x ][ wrapAround( y + 1 ) ].m_fish != 0 )
			{
				newElement = wrapAround( y + 1 );
				// Copy the Shark to the new position.
				map[x][newElement].m_shark = map[x][y].m_shark;
				// Set the old Shark position to null
				//delete ( map[x][y].m_shark  );
				map[x][y].m_shark = 0;
				// Set the old dead fish to 0
				map[x][newElement].m_fish = 0;
				// Update the sharks starve time as he just ate.
				map[x][newElement].m_shark->m_starve = sharkStarveTime;
				map[x][newElement].m_shark->m_breed -= 1;
				if( map[x][newElement].m_shark->m_breed <= 0)
				{
					breed( 's' , x, y);
					map[x][newElement].m_shark->m_breed = maxSharkBreedTime;
				}	
				// Mark the tile as updated for this turn.
				map[x][newElement].m_updated = true;
				ate = true;
			}
			break;
			case(3):
			if( map[ wrapAround( x -1 ) ][ y ].m_fish != 0 )
			{
				newElement = wrapAround( x - 1 );
				// Copy the Shark to the new position.
				map[newElement][y].m_shark = map[x][y].m_shark;
				// Set the old Shark position to null
				//delete ( map[x][y].m_shark  );
				map[x][y].m_shark = 0;
				// Set the old dead fish to 0
				map[newElement][y].m_fish = 0;
				// Update the sharks starve time as he just ate.
				map[newElement][y].m_shark->m_starve = sharkStarveTime;
				map[newElement][y].m_shark->m_breed -= 1;
				if( map[newElement][y].m_shark->m_breed <= 0)
				{
					breed( 's' , x, y);
					map[newElement][y].m_shark->m_breed = maxSharkBreedTime;
				}	
				// Mark the tile as updated for this turn.
				map[newElement][y].m_updated = true;	
				ate = true;			
			}
			break;
			//else
			//{
			//	ate = false;
			//}
	}
		
		

	return ate;

}


void sharkMove( short x, short y ){
	// Check n,e,s,w
	short newElement = 0;
	short randomPath = 0;

	randomPath = rand() % 4;
	switch(randomPath)
	{
		// North
		case(0):
		if( map[x][wrapAround( y - 1 )].m_shark == 0 )
		{
			newElement = wrapAround( y - 1 );
			// Copy the Shark to the new position.
			map[x][newElement].m_shark = map[x][y].m_shark;
			// Set the old Shark position to null
			map[x][y].m_shark = 0;
			// Update the sharks starve time as he just ate.
			map[x][newElement].m_shark->m_starve -= 1;
			if ( map[x][newElement].m_shark->m_starve <= 0)
			{
				map[x][newElement].m_shark = 0;
			}
			else
			{
				map[x][newElement].m_shark->m_breed -= 1;
				if( map[x][newElement].m_shark->m_breed <= 0)
				{
					//breed( 's' , x, y);
					map[x][newElement].m_shark->m_breed = maxSharkBreedTime;
				}	
			}		
			// Mark the tile as updated for this turn.
			map[x][newElement].m_updated = true;

		}
		break;
		case(1):
		if( map[ wrapAround( x + 1 ) ][ y ].m_shark == 0 )
		{
			newElement = wrapAround( x + 1 );
			// Copy the Shark to the new position.
			map[newElement][ y].m_shark = map[x][y].m_shark;
			// Set the old Shark position to null
			map[x][y].m_shark = 0;
			// Update the sharks starve time as he just ate.
			map[newElement][ y].m_shark->m_starve -= 1;
			if ( map[newElement][ y].m_shark->m_starve <= 0)
			{
				map[newElement][ y].m_shark = 0;
			}
			else
			{
				map[newElement][ y].m_shark->m_breed -= 1;
				if( map[newElement][ y].m_shark->m_breed <= 0)
				{
					//breed( 's' , x, y);
					map[newElement][ y].m_shark->m_breed = maxSharkBreedTime;
				}	
			}	
			// Mark the tile as updated for this turn.
			map[newElement][ y].m_updated = true;
		}
		break;
		case(2):
		if( map[ x ][ wrapAround( y + 1 ) ].m_shark == 0 )
		{
			newElement = wrapAround( y + 1 );
			// Copy the Shark to the new position.
			map[x][newElement].m_shark = map[x][y].m_shark;
			// Set the old Shark position to null
			map[x][y].m_shark = 0;
			// Update the sharks starve time as he just ate.
			map[x][newElement].m_shark->m_starve -= 1;
			if ( map[x][newElement].m_shark->m_starve <= 0)
			{
				map[x][newElement].m_shark = 0;
			}
			else
			{
				map[x][newElement].m_shark->m_breed -= 1;
				if( map[x][newElement].m_shark->m_breed <= 0)
				{
					//breed( 's' , x, y);
					map[x][newElement].m_shark->m_breed = maxSharkBreedTime;
				}	
			}	
			// Mark the tile as updated for this turn.
			map[x][newElement].m_updated = true;
		}
		break;
		case(3):
		if( map[ wrapAround( x -1 ) ][ y ].m_shark == 0 )
		{
			newElement = wrapAround( x - 1 );
			// Copy the Shark to the new position.
			map[newElement][ y].m_shark = map[x][y].m_shark;
			// Set the old Shark position to null
			map[x][y].m_shark = 0;
			// Update the sharks starve time as he just ate.
			map[newElement][ y].m_shark->m_starve -= 1;
			if ( map[newElement][ y].m_shark->m_starve <= 0)
			{
				map[newElement][ y].m_shark = 0;
			}
			else
			{
				map[newElement][ y].m_shark->m_breed -= 1;
				if( map[newElement][ y].m_shark->m_breed <= 0)
				{
					//breed( 's' , x, y);
					map[newElement][ y].m_shark->m_breed = maxSharkBreedTime;
				}	
			}
			// Mark the tile as updated for this turn.
			map[newElement][ y].m_updated = true;
		}
		break;
	}
}


void breed( char type, short x, short y )
{
	if( type == 'f' )
	{
		map[x][x].m_fish = createFish( x, y );
	}
	else
	{
		map[x][x].m_shark = createShark( x, y );
	}
}

Fish* createFish(short x, short y){
	Fish* newFish = malloc( sizeof( Fish ) );
	newFish->m_breed = rand() % maxFishBreedTime;
	newFish->m_pos.m_x = x;
	newFish->m_pos.m_y = y;
	return newFish;
}

void updateFish(){
	int x = 0;
	int y = 0;

	for( x=0;x< ROWS; x++)
	{
		for( y=0;y<COLUMNS;y++)
		{
			if( !map[x][y].m_updated )
			{
				if( map[x][y].m_fish != 0 )
				{					
					// Move the shark.
					moveFish( x , y );	
				}
			}
		}
	}
}


void moveFish( short x, short y){
	// Check n,e,s,w
	short newElement = 0;
	short randomPath = 0;

	randomPath = rand() % 4;
	switch(randomPath)
	{
		// North
		case(0):
		if( map[x][wrapAround( y - 1 )].m_shark == 0 && map[x][wrapAround( y - 1 )].m_fish == 0)
		{
			newElement = wrapAround( y - 1 );
			// Copy the Shark to the new position.
			map[x][newElement].m_fish = map[x][y].m_fish;
			// Set the old Shark position to null
			map[x][y].m_fish = 0;		
		
			map[x][newElement].m_fish->m_breed -= 1;
			if( map[x][newElement].m_fish->m_breed <= 0)
			{
				breed( 'f' , x, y);
				map[x][newElement].m_fish->m_breed = maxFishBreedTime;
			}		
			// Mark the tile as updated for this turn.
			map[x][newElement].m_updated = true;

		}
		break;
		case(1):
		if( map[ wrapAround( x + 1 ) ][ y ].m_shark == 0 && map[ wrapAround( x + 1 ) ][ y ].m_fish == 0)
		{
			newElement = wrapAround( x + 1 );
			// Copy the Shark to the new position.
			map[newElement][ y].m_fish = map[x][y].m_fish;
			// Set the old Shark position to null
			map[x][y].m_fish = 0;

			map[newElement][ y].m_fish->m_breed -= 1;
			if( map[newElement][ y].m_fish->m_breed <= 0)
			{
				breed( 'f' , x, y);
				map[newElement][ y].m_fish->m_breed = maxFishBreedTime;
			}		
			// Mark the tile as updated for this turn.
			map[newElement][ y].m_updated = true;
		}
		break;
		case(2):
		if( map[ x ][ wrapAround( y + 1 ) ].m_shark == 0 && map[ x ][ wrapAround( y + 1 ) ].m_fish == 0)
		{
			newElement = wrapAround( y + 1 );
			// Copy the Shark to the new position.
			map[x][newElement].m_fish = map[x][y].m_fish;
			// Set the old Shark position to null
			map[x][y].m_fish = 0;
			map[x][newElement].m_fish->m_breed -= 1;
			if( map[x][newElement].m_fish->m_breed <= 0)
			{
				breed( 'f' , x, y);
				map[x][newElement].m_fish->m_breed = maxFishBreedTime;
			}
			// Mark the tile as updated for this turn.
			map[x][newElement].m_updated = true;
		}
		break;
		case(3):
		if( map[ wrapAround( x -1 ) ][ y ].m_shark == 0 && map[ wrapAround( x -1 ) ][ y ].m_fish== 0)
		{
			newElement = wrapAround( x - 1 );
			// Copy the Shark to the new position.
			map[newElement][ y].m_fish = map[x][y].m_fish;
			// Set the old Shark position to null
			map[x][y].m_fish = 0;

			map[newElement][ y].m_fish->m_breed -= 1;
			if( map[newElement][ y].m_fish->m_breed <= 0)
			{
				breed( 'f' , x, y);
				map[newElement][ y].m_fish->m_breed = maxFishBreedTime;
			}
			// Mark the tile as updated for this turn.
			map[newElement][ y].m_updated = true;
		}
		break;
	}

	
}


void killFish(){

}

short wrapAround( short value ){
	bool changed = false;

	// if the x or y are out of bounds, assign the new xy
	if ( value < 0 )
	{
		return (COLUMNS - 1);
	}
    else if ( value >= COLUMNS )
	{
		return 0;
	}


	if(changed)
	{
		//printf("In wrap around, passed like 'Position* oldPos', the value is : %d : %d", *x, *y) ;printf(" " );
	}
}


void draw(){

}

void event(SDL_Event* event){
	
	if(event->type == SDL_QUIT) {
		printf("A quit event has been fired from SDL.\n");
		SDL_Quit();
    	}
	
	
	while( SDL_PollEvent( event ) ){

		switch( event->type ){

		    // Look for a keypress.
		    case SDL_KEYDOWN:

		        // Check the SDLKey values and change the camera.
		        switch( event->key.keysym.sym ){
		            case SDLK_LEFT:
		                cam.x -= 10;
		                break;
		            case SDLK_RIGHT:
		                cam.x += 10;
		                break;
		            case SDLK_UP:
		                cam.y -= 10;
		                break;
		            case SDLK_DOWN:
		                cam.y += 10;
		                break;
			    case SDLK_ESCAPE:
				SDL_Quit();
		            default:
		                break;

		        }// End switch...
		    
		}// End switch... 
   
	}// End while...
	

}// End func...

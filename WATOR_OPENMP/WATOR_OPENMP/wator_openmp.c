/**@file wator.c
*@brief This Program is a simulation of the Wator world. It is a world consisting
* 	solely of water and populated only by Sharks and Fish. 
*	The simulation runs in cycles where in each cycles the Sharks hunt the fish
*	and both Sharks and Fish breed after a required number of cycles.
*	After either type of Entity has died off the simulation will end, but the 
*	idea is to get it balanced so as neither type ever dies off.
* 	The program has ran for in excess of 25,000 cycles without either entity
*	becoming extinct, which gives the impression that neither ever would if the 
*	program was to continue indefinetely.
*/


/**
	@Author/s:
	@Description:
	@Assumptions:
	-1 = Empty node.

*/

//	###############################			READ ME			#####################################
//
//			TO RUN THE SIMULATION; MODIFY VALUES BELOW, BUILD THE PROJECT AND THEN RUN IT.
//		                 IT WILL CONTINUE TO RUN UNTIL ONE SPECIES IS EXTINCT
//
//  #############################################################################################


#include <GL/gl.h>		//include OpnGL Dependencies
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>		//include Standard Library Dependencies
#include <time.h>

#include <omp.h>




typedef int bool;

typedef struct{

	int x;
	int y;

}Camera;

#define true 1				//Define Bool
#define false 0
#define sX 		1600		// Screen maxX
#define sY 		800		// Screen maxY. Both used for setting the VP.

#define width 250
#define height 250

#define empty -1

#define flagged 1
#define unflagged 0

// Opengl SDL..........................
SDL_Surface*    Surf_Display;
SDL_Event mEvent;
Camera cam;

//......................................
void Initialize();
void FishSwimAndBreed();
void SharkSwimAndBreed();
void Breed(int X,int Y,char type);
void DrawGrid();
void DisplayInfo();
void GetInput();
void PlaceSharks();
void PlaceFish();
int Wrap( int x,char d );
//.........OpenGl SDL func
void event(SDL_Event* Event);

//..........................................
int frameCount;
int currentTime;
int previousTime;
int fps;

int loopCount = 0;	//loop simulation x number of times (For debugging purposes).
int loopMax = 350;

int i, j, randX, randY, newX, newY;
int randBreed, randStarve, randomPath;	// Vars for initalising Entities.
int randSpawn,tries;
bool placed;
bool moved;

int NumShark 	= 70;
int NumFish  	= 500;
int FishBreed 	= 8;
int SharkBreed 	= 25;
int StarveTime 	= 12;

int Flags[width][height];		//Flag if a an entity at a position has already been processed ( 0 or 1 etc.)
int Fish[ width ][ height ];		// -1 = no Fish, other num = age	(check this age each loop for breed time)
int Sharks[ width ][ height ];		// -1 = no Shark, other num = age	(check this age each loop for breed time)
int Starve[ width ][ height ];		// -1 = no Shark, other num stores the the time since the shark at this position has 						// eaten, this needs to be update (set old pos to -1, new pos to time since last 						// feed) every time you move a shark






/**
	@Author: Paddy, James, Dean, Martin
	@Description:	
	//This is the main Program loop. Each of the Shark and Fish Update funstions are called from here.
	//Once the world has been updated, The drawGrid function is called which displays the Wator world using OpenGL.
*/
int main()
{
	char *myargv [1];
	int myargc=1;
	myargv [0]=strdup ("WATOR");
	glutInit(&myargc, myargv);
	
	srand( time( NULL ) );
	cam.x = 0;
	cam.y = 0;
	int c;
	Initialize();
	loopCount = 0;
	frameCount = 0;
	while ( (NumShark && NumFish > 0)  )
	{		
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
		
		FishSwimAndBreed();
		SharkSwimAndBreed();		
		//DrawGrid();
		printf( "Number of cycles: %d\n", loopCount );		
		loopCount++;	

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
	
	printf( " ########## END RESULT ##########\n\n");
	//DrawGrid();	
	DisplayInfo();
	printf( "Number of cycles: %d\n", loopCount );	
	
	getchar();
	return(0);
}



/**
	@Author: Paddy
	@Description:
	// This function loops through both the Shark and Fish arrays sets all values to -1 (empty).
	// It then randomly places NumFish Fish and NumShark Sharks within the world.
	// The function also initializes OpenGl.
*/
void Initialize(){	
	
	

	//
	// Initalise the Fish and Shark arrays as empty.
	//
	#pragma omp parallel for private(j)
	for ( i = 0; i < width; i++)
	{
		for ( j = 0; j < height; j++)
		{
			
			Fish[ i ][ j ] = empty;
			Sharks[ i ][ j ] = empty;			
		}		
	}		

	PlaceSharks();
	PlaceFish();

	//...........................................................

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

	//...........................................................

}// End func






/**
	@Author: Paddy
	@Description:
	//This function is used to place NumShark Sharks within the game world.
	//It loops until all Sharks have been placed randomly in an available position.
	//It also gives them random Breed and Starve values.	
*/
void PlaceSharks(){

	#pragma omp parallel for private(placed,randX,randY,randBreed,randStarve)
	for (	i = 0 ; i < NumShark; i++)
	{		
		placed = false;	

		//
		//	Loop until an available spawn location is found.
		//
		while( placed == false )
		{			
			randX = rand() % width;
			randY = rand() % height;	
			
			//
			// If there isn't already a fish or a shark here, place a shark.
			//
			if( ( Fish[ randX ][ randY ] == empty ) && ( Sharks[ randX ][ randY ] == empty ) )
			{				
				randBreed = rand() % SharkBreed;		// Generate a breed time for the Shark.
				randStarve = rand() % StarveTime;		// Generate a time at which the shark will starve.
				Sharks[ randX ][ randY ] = randBreed;		// Store the breed time in the shark array.
				Starve[ randX ][ randY ] = randStarve;		// Store the starve time in the starve array.
				placed = true;					// Set as placed and move onto the next shark.
			}//End if

		}//End while

	}//End for

}// End func







/**
	@Author: Martin, Dean
	@Description:
	//This function is used to place NumFish Fish within the game world.
	//It loops until all Fish have been placed randomly in an available position.
	//It also gives them random Breed and Starve values.	
	
*/
void PlaceFish(){

	#pragma omp parallel for private(placed,randX,randY,randBreed)
	for (	i = 0 ; i < NumFish; i++)
	{		
		placed = false;	

		//
		//	Loop until an available spawn location is found.
		//
		while( placed == false )
		{			
			randX = rand() % width;
			randY = rand() % height;	
			
			//
			// If there isn't already a fish or a shark here, place a fish.
			//
			if( ( Fish[ randX ][ randY ] == empty ) && ( Sharks[ randX ][ randY ] == empty ) )
			{				
				randBreed = rand() % FishBreed;		// Generate a breed time for the Shark.	 
				Fish[ randX ][ randY ] = randBreed;	// Store the breed time in the shark array.
				placed = true;				// Set as placed and move onto the next shark.
			}//End if

		}//End while

	}//End for

}// End func







/**
	@Author: Paddy, James
	@Description:
	//This function loops through the world and draws any Entity at each position.
	//OpenGl is used to draw the world, with Sharks showing as red and Fish showing as White.
	//The Display is Shown in a seperate program window and is refreshed and updated each cycle.
*/
void DrawGrid(){

	int x,y,boxSize,space;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    	glLoadIdentity();

	boxSize = 4;
	space = 1;
	
	glTranslatef( 100,100 - (boxSize+space),0 );
	
	for( y = 0; y < height; y++ ){

		for( x = 0; x < width; x++ ){			

			if ( Fish[ x ][ y ] != empty )
			{
				glColor3d(1, 1, 1);	
			}
			else if ( Sharks[ x ][ y ] != empty )
			{
				glColor3d(1, 0, 0);
			}
			else
			{
				glColor3d(0, 0, 0.5);
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

	// SDL_GL_SwapBuffers which basically takes the buffer and moves it to the primary display.
    	SDL_GL_SwapBuffers();

}// End func.


/**
	@Author: Martin, Dean
	@Description:
	//This function updates the fish each frame.
	//It first sets the flags for each position to 'unflagged' (These flags are used so no entity is updated twice).
	//It then loops until all fish are moved in a random direction to an empty adjacent position (if there is one available).
	//Next it checks the Fish's age and breeds if the neccessary time has elapsed.
*/
void FishSwimAndBreed()
{		
	//set flags to unflagged
	#pragma omp parallel for private(j)
	for( i = 0; i < width ; i++ )
	{
		for( j = 0; j < height; j++ )
		{
			Flags[i][j] = unflagged;
		}
	}
	
	//Check surrounding area( West = x-1, East = x+1, North = y-1, South = y+1 )
	#pragma omp parallel for private(j,newX,newY,tries,randomPath)	
	for( i = 0; i < width ; i++ )
	{		
		for( j = 0; j < height; j++ )
		{		

			if( Fish[ i ][ j ] != empty && Flags[ i ][ j ] == unflagged)
			{
										
				newX = i;
				newY = j;
				tries = 0;
				//look for fish				
				while(tries < 4)
				{
					
					randomPath = rand() % 4;
					switch(randomPath)
					{

					case(0):
					// Move in a random direction while avoiding 
					if( Fish[ Wrap( i,'w' )-1 ][ j ] == empty && Sharks[ Wrap( i,'w' )-1 ][ j ] == empty)// West
					{					
						newX = Wrap( i,'w' )-1;
						newY = j;
						Fish[ newX ][ newY ] = Fish[ i ][ j ];	
						Fish[ i ][ j ] = empty;				
						Flags[ newX ][ newY ] = flagged;
						tries = 4;
					}
					else
					{					
						tries++;
					}
					break;
					case(1):
					if( Fish[ Wrap(i,'e')+1 ][ j ] == empty && Sharks[ Wrap(i,'e')+1 ][ j ] == empty)// East
					{
						newX = Wrap( i,'e' )+1;
						newY = j;
						Fish[ newX ][ newY ] = Fish[ i ][ j ];
						Fish[ i ][ j ] = empty; 				
						Flags[ newX ][ newY ] = flagged;
						tries = 4;
					}
					else
					{					
						tries++;
					}
					break;
					case(2):
					if( Fish[ i ][ Wrap( j,'n' )-1 ] == empty && Sharks[ i ][ Wrap( j,'n' )-1 ] == empty)//north
					{
						newX = i;
						newY = Wrap( j,'n' )-1;
						Fish[ newX ][ newY ] = Fish[ i ][ j ];	
						Fish[ i ][ j ] = empty; 			
						Flags[ newX ][ newY ] = flagged;
						tries = 4;
					}
					else
					{					
						tries++;
					}
					break;
					case(3):
					if( Fish[ i ][ Wrap(j,'s')+1 ] == empty && Sharks[ i ][ Wrap(j,'s')+1 ] == empty)// South
					{
						newX = i;
						newY = Wrap( j,'s' )+1;
						Fish[ newX ][ newY ] = Fish[ i ][ j ];
						Fish[ i ][ j ] = empty;		
						Flags[ newX ][ newY ] = flagged;
						tries = 4;
					}
					else
					{					
						tries++;
					}
					break;
					}//end switch
				}//end while
				
				//check breed
				Fish[ newX ][ newY ]++;
				
				if(Fish[ newX ][ newY ] == FishBreed)
				{
					Fish[ newX ][ newY ] = 0;	//reset breed time
					Breed(newX,newY,'f');					
				}
				
			}
						
		}

	}	
}






/**
	@Author: Paddy, James
	@Description:
	//This function updates the Sharks each frame.
	//It first sets the flags for each position to 'unflagged' (These flags are used so no entity is updated twice).
	//It then loops and checks the Sharks adjacent positions. If there is a fish in one of these positions, the Shark
	//will move to these position and eat the fish, which in turn sets its starve time to 0.
	//If there is no adjacent fish, the shark will move in a radom direction (Once there is an available position to move to).
	//It also checks the Sharks Starve time, and kills it off if it has not fed before StarveTime has elapsed.
	//Next it checks the Shark's age and breeds if the neccessary time has elapsed (if the shark is still alive).
*/
void SharkSwimAndBreed()
{
	//set flags to unflagged
	#pragma omp parallel for private(j)
	for( i = 0; i < width ; i++ )
	{
		for( j = 0; j < height; j++ )
		{
			Flags[i][j] = unflagged;
		}
	}

	// Check surrounding area( West = x-1, East = x+1, North = y-1, South = y+1 )	
	// Loop through the grid.
	#pragma omp parallel for private(j,newX,newY,moved,tries,randomPath)
	for( i = 0; i < width ; i++ )
	{		
		for( j = 0; j < height; j++ )
		{		
			//If there is a shark here and he has not already been updated.
			if( Sharks[ i ][ j ] != empty && Flags[ i ][ j ] == unflagged) 
			{			
				newX = i;
				newY = j;
				moved = false;
				tries = 0;
				//look for fish				
				while(tries < 4)
				{
					randomPath = rand() % 4;
					
					switch(randomPath)
					{

						case(0):
						if( Fish[ Wrap( i,'w' )-1 ][ j ] != empty) 					
						{
							newX = Wrap( i,'w' )-1;
							newY = j;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	
							Sharks[ i ][ j ] = empty;				
							Fish[ newX ][ newY ]= empty;				
							NumFish--;
							Starve[ newX ] [ newY ] = StarveTime;		
							Starve[ i ] [ j ]= empty;				
							Flags[ newX ][ newY ] = flagged;
							tries = 4;	
							moved = true;
						}
						else
						{					
							tries++;
						}
						break;
						case(1):
						if( Fish[ Wrap(i,'e')+1 ][ j ] != empty)		
						{
							newX = Wrap( i,'e' )+1;
							newY = j;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	
							Sharks[ i ][ j ] = empty;				
							Fish[ newX ][ newY ]= empty; 				
							NumFish--;
							Starve[ newX ] [ newY ] = StarveTime;		
							Starve[ i ] [ j ]= empty;				
							Flags[ newX ][ newY ] = flagged;
							tries = 4;	
							moved = true;	
						}
						else
						{					
							tries++;
						}
						break;
						case(2):
						if( Fish[ i ][ Wrap( j,'n' )-1 ] != empty )		//north
						{
							newX = i;
							newY = Wrap( j,'n' )-1;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	
							Sharks[ i ][ j ] = empty;				
							Fish[ newX ][ newY ]= empty;			
							NumFish--;					
							Starve[ newX ] [ newY ] = StarveTime;		
							Starve[ i ] [ j ]= empty;				
							Flags[ newX ][ newY ] = flagged;
							tries = 4;
							moved = true;
						}
						else
						{					
							tries++;
						}
						break;
						case(3):
						if( Fish[ i ][ Wrap(j,'s')+1 ] != empty)		// South
						{
							newX = i;
							newY = Wrap( j,'s' )+1;				
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	
							Sharks[ i ][ j ] = empty;				
							Fish[ newX ][ newY ]= empty;			
							NumFish--;	 
							Starve[ newX ] [ newY ] = StarveTime;		
							Starve[ i ] [ j ]= empty;				
							Flags[ newX ][ newY ] = flagged;
							tries = 4;
							moved = true;
						}
						else
						{					
							tries++;
						}
						break;
					}//end switch
				}//end while
				

				if( moved != true) //move to empty position
				{
					// Move in a random direction while avoiding other sharks.
					randomPath = rand() % 4;
						
					switch(randomPath)
					{

						case(0):
						if( Sharks[ Wrap( i,'w' )-1 ][ j ] == empty )	// West
						{
							newX = Wrap( i,'w' )-1;
							newY = j;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	// set new value to new pos
							Sharks[ i ][ j ] = empty;			// set old pos to -1
							Starve[ newX ] [ newY ] = Starve[ i ] [ j ]; 	
							Starve[ i ] [ j ]= empty;    //set old starve to -1
							Flags[ newX ][ newY ] = flagged;
						} 							
						break;
						case(1):
						if( Sharks[ Wrap(i,'e')+1 ][ j ] == empty)		// East
						{
							newX = Wrap( i,'e' )+1;
							newY = j;							
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	// set new value to new pos
							Sharks[ i ][ j ] = empty;			// set old pos to -1
							Starve[ newX ] [ newY ] = Starve[ i ] [ j ]; 								
							Starve[ i ] [ j ]= empty;			//set old starve to -1 						
							Flags[ newX ][ newY ] = flagged;
						}
						break;
						case(2): 
						if( Sharks[ i ][ Wrap( j,'n' )-1 ] == empty )	//north
						{
							newX = i;
							newY = Wrap( j,'n' )-1;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j]; 								
							Sharks[ i ][ j ] = empty;			// set old pos to -1
							Starve[ newX ] [ newY ] = Starve[ i ] [ j ]; 								
							Starve[ i ] [ j ]= empty;			//set old starve to -1
							Flags[ newX ][ newY ] = flagged;
						} 							
						break;
						case(3): 
						if( Sharks[ i ][ Wrap(j,'s')+1 ] == empty)				// South
						{
							newX = i;
							newY = Wrap( j,'s' )+1;
							Sharks[ newX ][ newY ] = Sharks[ i ][ j ];	// set new value to new pos
							Sharks[ i ][ j ] = empty;				// set old pos to 
                            				Starve[ newX ] [ newY ] = Starve[ i ] [ j ]; 
							Starve[ i ] [ j ]= empty;                  //set old starve to -1
							Flags[ newX ][ newY ] = flagged;
                        			} 							
						
						
					}// end switch
					
					//only decrement starve if no fish was found
					Starve[ newX ] [ newY ]--;
					if( Starve[ newX ] [ newY ] <= 0 )
					{
						Sharks[ newX ][ newY ] = empty;
						Sharks[ i ][ j ] = empty;
						Starve[ newX ] [ newY ] = empty;
						NumShark--;				
						printf( "Shark Starved: %d : %d\n", newX ,newY); 						
					}
				}// end if moved
				
				if( Starve[ newX ] [ newY ] != empty )
				{
					//increment age
					Sharks[ newX ][ newY ]++;					
					// Check breed time
					if( Sharks[ newX ][ newY ] == SharkBreed)
					{
						Sharks[ newX ][ newY ] = 0;	//reset breed time
						Breed(newX,newY,'s');
					}
				}
				
			}// End if			
			
		}// End for

	}// End for		

}// End func


/**
	@Author: Paddy
	@Description: 
	//This function handles the spawning of both Sharks and Fish
	//It checks the adjacent positions of the Entity which is breeding.
	//It picks one of these positions at random and if it is empty is spawns another entity of the same type here.
*/
void Breed(int X,int Y,char type)
{		
	tries = 0;
	
	if (type == 's')
	{
		while( tries < 4 )
		{				
			randSpawn = rand() % 4;
			switch(randSpawn)
			{
				case(0):
				if(Sharks[ X ][ Wrap( Y,'n' )-1 ] == empty && Fish[ X ][ Wrap( Y,'n' )-1 ] == empty)	//north
				{
					Sharks[ X ][ Wrap( Y,'n' )-1 ] = 0;
					Starve[ X ][ Wrap( Y,'n' )-1 ] = StarveTime;
					tries = 4;
					NumShark++;		
					printf( "Shark Born: %d : %d\n", X ,Wrap( Y,'n' )-1 );					
				}
				else
				{					
					tries++;
				}
				break;
				case(1):
				if(Sharks[ X ][ Wrap(Y,'s')+1 ] == empty && Fish[ X ][ Wrap(Y,'s')+1 ] == empty)	//south
				{
					Sharks[ X ][ Wrap(Y,'s')+1 ] = 0;
					Starve[ X ][ Wrap(Y,'s')+1 ] = StarveTime;
					tries = 4;
					NumShark++;		
					printf( "Shark Born: %d : %d\n", X , Wrap(Y,'s')+1 );
				}
				else
				{					
					tries++;
				}
				break;
				case(2):
				if(Sharks[ Wrap(X,'e')+1 ][ Y ] == empty && Fish[ Wrap(X,'e')+1 ][ Y ] == empty)	//east
				{
					Sharks[ Wrap(X,'e')+1 ][ Y ] = 0;
					Starve[ Wrap(X,'e')+1 ][ Y ] = StarveTime;
					tries = 4;
					NumShark++;					
					printf( "Shark Born: %d : %d\n", Wrap(X,'e')+1 , Y );
				}
				else
				{					
					tries++;
				}
				break;
				case(3):
				if(Sharks[ Wrap( X,'w' )-1 ][ Y ] == empty && Fish[ Wrap( X,'w' )-1 ][ Y ] == empty)	//west
				{
					Sharks[ Wrap( X,'w' )-1 ][ Y ] = 0;
					Starve[ Wrap( X,'w' )-1 ][ Y ] = StarveTime;
					tries = 4;
					NumShark++;			
					printf( "Shark Born: %d : %d\n", Wrap( X,'w' )-1 , Y );
				}
				else
				{					
					tries++;
				}
				break;
			}
		}
	}
	else if (type =='f')
	{
		while( tries < 4 )
		{				
			randSpawn = rand() % 4;
			switch(randSpawn)
			{
				case(0):
				if(Fish[ X ][ Wrap( Y,'n' )-1 ] == empty && Sharks[ X ][ Wrap( Y,'n' )-1 ] == empty)	//north
				{
					Fish[ X ][ Wrap( Y,'n' )-1 ] = 0;					
					tries = 4;
					NumFish++;					
				}
				else
				{					
					tries++;
				}
				break;
				case(1):
				if(Fish[ X ][ Wrap(Y,'s')+1 ] == empty && Sharks[ X ][ Wrap(Y,'s')+1 ] == empty)	//south
				{
					Fish[ X ][ Wrap(Y,'s')+1 ] = 0;					
					tries = 4;
					NumFish++;					
				}
				else
				{					
					tries++;
				}
				break;
				case(2):
				if(Fish[ Wrap(X,'e')+1 ][ Y ] == empty && Sharks[ Wrap(X,'e')+1 ][ Y ] == empty)	//east
				{
					Fish[ Wrap(X,'e')+1 ][ Y ] = 0;					
					tries = 4;
					NumFish++;					
				}
				else
				{					
					tries++;
				}
				break;
				case(3):
				if(Fish[ Wrap( X,'w' )-1 ][ Y ] == empty && Sharks[ Wrap( X,'w' )-1 ][ Y ] == empty)	//west
				{
					Fish[ Wrap( X,'w' )-1 ][ Y ] = 0;					
					tries = 4;
					NumFish++;					
				}
				else
				{					
					tries++;
				}
				break;
			}
		}
	}
}





/**
	@Author: Paddy
	@Description:
	Deals with the wrap around of the map/Grid.
	Returns the same pos as the arg unless its either
	at 0 or height/Width (i.e at the edge of the screen).
*/
int Wrap( int pos , char dir)
{
	
	if( pos == 0 && dir == 'n' )					//if heading north and at top square
	{		
		pos = height;		
	}
	else if ( pos == height-1 && dir == 's' )			//if heading south and at bottom square
	{		
		pos = -1;		
	}
	else if ( pos == 0 && dir == 'w' )				//if heading west and at far west square
	{		
		pos = width;		
	}
	else if ( pos == width-1 && dir =='e' )			//if heading east and at far east square
	{		
		pos = -1;		
	}
	
	
	return pos;
}

/**
	@Author: James
	@Description:
	//Handles OpenGL Camera Movement and Key Events.
*/
void event(SDL_Event* event){
	
	if(event->type == SDL_QUIT) {
		printf("A quit event has been fired from SDL.\n");
		SDL_Quit();
    	}
	
	//while( SDL_PollEvent( event ) ){

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
   
	//}// End while...

}// End func...




/**
	@Author: Paddy, James, Martin, Dean
	@Description:
	Output the number of sharks left and the number of fish left.
	Could also output How long until next shark/fish spawn.

*/
void DisplayInfo(){
		printf( "Sharks %d \n", NumShark );
		printf( "Fish %d \n", NumFish );
}





/**
	@Author: Paddy, James, Martin, Dean
	@Description:
	Ask the player for information on how to proceed.
	Can use this as a debug tool to set variables at run time.
	Also slows down the command window until we sort some graphics.
*/
void GetInput(){

	 int val;
   
     printf( "Enter an integer and press enter.\n" );
     scanf( "%d", &val ); 
}

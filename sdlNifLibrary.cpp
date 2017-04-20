/* sdlNifLibrary.cpp*/

#include "erl_nif.h"
#include <SDL/SDL.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>
#include <iomanip>

#define maxBuffLen 1024

/*Global variables
---------------------------------------------------------------------------------------------------------------------------------------*/

/*Array for storing surfaces in order to access them from an Erlang call.*/
std::vector<SDL_Surface*> surfaceArray;

/*Array for storing the names associated with surfaces*/
std::vector<std::string> surfaceNames;

/*Vector for storing palettes. The names will remain the same as for a surface as each surface can have one palette*/
std::vector<SDL_Palette> surfacePalettes;

/*Vector for RGB Maps*/
std::vector<Uint32> rgbMaps;

/*Vector for RGB Map names*/
std::vector<std::string> mapNames;

/*Private Functions
-------------------------------------------------------------------------------------------------------------------------------------------*/
/**
* Looks up surface index
* @param surface name (string)
* @return Index int if found, or -1 if not found.
**/
int surfaceIndex(std::string surfaceName){
	//Returns the index of a surfaceName in surfaceNames
	int i = 0;
	while(i<surfaceNames.size()){
		//If the surfaceName exists in surfaceNames return the index
		const char* surfaceNamesValue = surfaceNames[i].c_str();
		const char* surfaceNameValue = surfaceName.c_str();
		if(std::strcmp(surfaceNamesValue,surfaceNameValue)==0){
			return i;
		}
		i++;
	}
	//If surface not found return -1
	return (-1);
}

/**
* Looks up map index
**/
int mapIndex(std::string mapName){
	//Returns the index of a surfaceName in surfaceNames
	int i = 0;
	while(i<mapNames.size()){
		//If the mapName exists in surfaceNames return the index
		const char* mapNamesValue = mapNames[i].c_str();
		const char* mapNameValue = mapName.c_str();
		if(std::strcmp(mapNamesValue,mapNameValue)==0){
			return i;
		}
		i++;
	}
	//If surface not found return -1
	return (-1);
}

/*NIF FUNCTIONS 
-------------------------------------------------------------------------------------------------------------------------------------------*/

/**
*	Wrapped SDL_Init function.
*	@params Requires a flag (char*). Is passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, or 0 on success
**/
static ERL_NIF_TERM sdl_Init (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char initFlag[maxBuffLen];
	enif_get_string(env, argv[0], initFlag, maxBuffLen, ERL_NIF_LATIN1); //Gets the string passed by the function call and puts it in initFlag.
	if (std::strcmp(initFlag, "SDL_INIT_VIDEO") == 0){
		SDL_Init(SDL_INIT_VIDEO);
		return enif_make_int(env,0); /*exit code, process terminated correctly*/
	}
	else{
		/* Flag not recognised, process not terminated correctly*/
		return enif_make_string(env, "Init Flag not recognised, Init terminated" , ERL_NIF_LATIN1);
	}
}

/**
*	Wrapped SDL_Quit function.
*	@params None.
*	@Return ERL_NIF_TERM 0 on success
**/
static ERL_NIF_TERM sdl_Quit (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	SDL_Quit();
	return enif_make_int(env,0); /*exit code*/
}

/**
*	Wrapped SDL_CreateSurface function.
*	@params Requires a surface name (char*). Is passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, an error if the surface already exists, or 0 on success
**/
static ERL_NIF_TERM sdl_CreateSurface (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	//If the surfaceName already exists throw a wobbly
	int i = surfaceIndex(surfaceString);
	if(i>=0){
		return enif_make_string(env, "The surface name specified already exists, new surface not created" , ERL_NIF_LATIN1);
	}
	surfaceNames.push_back(surfaceString);
	SDL_Surface* newSurface = new SDL_Surface();
	surfaceArray.push_back(newSurface);
	
	return enif_make_int(env,0); /*exit code*/
}

/**
*	Wrapped SDL_SetVideoMode function.
*	@params Requires integer values (width, height, bits-per-pixel), a flag (char*) and a surface. Passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, Surface not found error, or 0 on success
**/
static ERL_NIF_TERM sdl_SetVideoMode (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	int width, height, bits;
	char flag[maxBuffLen];
	char surfaceName [maxBuffLen];
	//If width, height and bits passed are not integers throw an error
	if (!enif_get_int(env, argv[0], &width) || !enif_get_int(env, argv[1], &height) || !enif_get_int(env, argv[2], &bits))
	{
		return enif_make_badarg(env);
	}
	//If flag passed is not a string throw an error
	if(!enif_get_string(env, argv[3], flag, maxBuffLen, ERL_NIF_LATIN1)){
		//If fileName passed is not a string throw an error
		return enif_make_badarg(env);
	}
	
	//If surface passed is not a string throw an error
	if(!enif_get_string(env, argv[4], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	
	//If surface doesn't exist throw an error
	int i = surfaceIndex(surfaceString);
	if(i<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_CreateRGBSurface" , ERL_NIF_LATIN1);
	}
	SDL_Surface* surface = surfaceArray[i];
	
	//Check flag
	if (std::strcmp(flag, "SDL_SWSURFACE") == 0){
		
		surface = SDL_SetVideoMode(width, height, bits, SDL_SWSURFACE);	
		surfaceArray[i]=surface;
		
		return enif_make_int(env,0); /*exit code, process terminated correctly*/
	}
	else{
		/* Flag not recognised, process not terminated correctly*/
		return enif_make_string(env, "Flag not recognised, sdl_SetVideoMode terminated" , ERL_NIF_LATIN1);
	}
}

/**
*	Wrapped SDL_LoadBMP function.
*	@params Requires the name of the file to load (char*), and a surface to load into. Are passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, surface not found error, or 0 on success
**/
static ERL_NIF_TERM sdl_LoadBMP (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char fileName[maxBuffLen];
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], fileName, maxBuffLen, ERL_NIF_LATIN1)){
		//If fileName passed is not a string throw an error
		return enif_make_badarg(env);
	}
	if(!enif_get_string(env, argv[1], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		//If surfaceName passed is not a string throw an error
		return enif_make_badarg(env);
	}
	//If surfaceName doesn't exist throw an error
	std::string surfaceString(surfaceName);
	int i = surfaceIndex(surfaceString);
	if(i<0){
		return enif_make_string(env, "SurfaceIndex<0 in sdl_LoadBMP" , ERL_NIF_LATIN1);
	}
	//Otherwise load the bmp into the surface	

	SDL_Surface* surface = surfaceArray[i];
	
 	surface = SDL_LoadBMP(fileName);
	surfaceArray[i] = surface;
	
	if(surface==NULL){
		return enif_make_string(env, "LOAD BMP ERRROR ", ERL_NIF_LATIN1);
	}
	return enif_make_int(env,0); /*exit code*/
}

/**
*	Wrapped SDL_BlitSurface function.
*	@params Requires a surface to blit from, flags for clipping and cropping, and a surface to blit to (surface from, Flag, surface to, Flag). 
*		Are passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, an error if either surface doesn't exist, or 0 on success
**/
static ERL_NIF_TERM sdl_BlitSurface (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char primarySurfaceName[maxBuffLen]; // move from
	char secondarySurfaceName[maxBuffLen]; // move into
	char flag1[maxBuffLen];
	char flag2[maxBuffLen];
	
	//If any of the arguments passed are not strings throw an error
	if(!enif_get_string(env, argv[0], primarySurfaceName, maxBuffLen, ERL_NIF_LATIN1) || !enif_get_string(env, argv[1], flag1, maxBuffLen, ERL_NIF_LATIN1) || !enif_get_string(env, argv[2], secondarySurfaceName, maxBuffLen, ERL_NIF_LATIN1) || !enif_get_string(env, argv[3], flag2, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	//Check if our surfaces exist
	std::string primSurfaceString(primarySurfaceName);
	std::string secSurfaceString(secondarySurfaceName);
	int pindex = surfaceIndex(primSurfaceString);
	int sindex = surfaceIndex(secSurfaceString);
	
	// If surface doesnt exist then error
	if(pindex <0  || sindex <0 ){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_BlitSurface", ERL_NIF_LATIN1);
	}

	SDL_Surface* primarySurface = surfaceArray[pindex];
	SDL_Surface* secondarySurface = surfaceArray[sindex];
	
	//Check our flags
	if (std::strcmp(flag1, "NULL") == 0 && std::strcmp(flag2, "NULL")==0){
		
		SDL_BlitSurface(primarySurface, NULL, secondarySurface, NULL);
		surfaceArray[pindex] = primarySurface;
		surfaceArray[sindex] = secondarySurface;
		
		return enif_make_int(env,0); /*exit code, process terminated correctly*/
	}
	else{
		/* Flag not recognised, process not terminated correctly*/
		return enif_make_string(env, "A Flag is not recognised, BlitSurface terminated" , ERL_NIF_LATIN1);
	}
}

/**
*	Wrapped SDL_Flip function.
*	@params Requires the name of the surface to flip (char*). Is passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, a surface not found error, or 0 on success
**/
static ERL_NIF_TERM sdl_Flip (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		//If argument passed is not a string throw an error
		return enif_make_badarg(env);
	}
	//If surfaceName doesn't exist throw an error
	std::string surfaceString(surfaceName);
	int i = surfaceIndex(surfaceString);
	if(i<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_Flip" , ERL_NIF_LATIN1);
	}
	SDL_Surface* surface = surfaceArray[i];
	SDL_Flip(surface);
	return enif_make_int(env,0); /*exit code*/	
}

/**
*	Wrapped SDL_Delay function.
*	@params Requires an integer value (in ms). Is passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, or 0 on success
**/
static ERL_NIF_TERM sdl_Delay (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	int x;
	if (!enif_get_int(env, argv[0], &x)) /*If argument isn't an integer throw an error */
	{
		return enif_make_badarg(env);
	}
	SDL_Delay(x);
	return enif_make_int(env, 0); /* Exit code */
}

/**
*	Wrapped SDL_FreeSurface function.
*	@params Requires the name of the surface to free. Is passed as an argument from Erlang.
*	@Return ERL_NIF_TERM A bad argument error, a string error if the surface is not found, or 0 on success
**/
static ERL_NIF_TERM sdl_FreeSurface (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_FreeSurface" , ERL_NIF_LATIN1);
	}
	
	//Free the surface
	SDL_FreeSurface(surfaceArray[index]);
	//Erase the surface and it's name from the arrays
	surfaceArray.erase(surfaceArray.begin()+index);
	surfaceNames.erase(surfaceNames.begin() + index);
	return enif_make_int(env, 0); /*Exit Code*/
}

/**
*	Wrapped SDL_UpdateRect function
*	@params Requires the name of the surface to be updated, the position (x,y) of the rectangle and it's size (w,h)
*	@return 0 on success, error String otherwise.
**/
static ERL_NIF_TERM sdl_UpdateRect (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	int x, y, w, h;
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	if(!enif_get_int(env, argv[1], &x) || !enif_get_int(env, argv[2], &y) || !enif_get_int(env, argv[3], &w) || !enif_get_int(env, argv[4], &h)){
		return enif_make_badarg(env);
	}
	
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_UpdateRect", ERL_NIF_LATIN1);
	}
	
	//Update the rect for the surface
	SDL_UpdateRect(surfaceArray[index], x, y, w, h);
	
	return enif_make_int(env, 0); /*Exit code*/
}

/**
*	New function for the library.
*	@param surfaceName name of the surface whose pixel format is being gotten.
*	@return ERL_NIF_TERM Erlang List of various pixel format elements, or an error message on failure.
**/
static ERL_NIF_TERM sdl_GetPixelFormat (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_UpdateRect", ERL_NIF_LATIN1);
	}
	
	SDL_Surface* surface = surfaceArray[index];
	
	//Get Pixel Format
	SDL_PixelFormat* pixelFormat = surface->format;
	
	//Make lists for loss, shift, and mask to cut down on size of pixelList. This means we will be passing back a list containing lists
	ERL_NIF_TERM lossList = enif_make_list4(env, enif_make_int(env, pixelFormat->Rloss), enif_make_int(env, pixelFormat->Gloss), enif_make_int(env, pixelFormat->Bloss), enif_make_int(env, pixelFormat->Aloss));
	
	ERL_NIF_TERM shiftList = enif_make_list4(env, enif_make_int(env, pixelFormat->Rshift), enif_make_int(env, pixelFormat->Gshift), enif_make_int(env, pixelFormat->Bshift), enif_make_int(env, pixelFormat->Ashift));
	
	ERL_NIF_TERM maskList = enif_make_list4(env, enif_make_int(env, pixelFormat->Rmask), enif_make_int(env, pixelFormat->Gmask), enif_make_int(env, pixelFormat->Bmask), enif_make_int(env, pixelFormat->Amask));
		
	ERL_NIF_TERM pixelList = enif_make_list8(env, enif_make_string(env, surfaceName, ERL_NIF_LATIN1), enif_make_int(env, pixelFormat->BitsPerPixel), enif_make_int(env, pixelFormat->BytesPerPixel), lossList, shiftList, maskList, enif_make_int(env, pixelFormat->colorkey), enif_make_int(env, pixelFormat->alpha));
		
	return(pixelList);
}

/**
*	Wrapped SDL_MapRGB
*	Stores new RGB maps in the rgbMap vector and their names in mapNames
*	Overwrites if the name already exists
*	@Param mapName Name of map (for storage and reference), formatName Name of surface whose format we're using,  r red value, g green value, b blue value
*	@Return 0 on success, error String otherwise.
**/
ERL_NIF_TERM sdl_MapRGB (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char mapName[maxBuffLen];
	char formatName[maxBuffLen];
	int r, g, b;
	//Ok, so it's fiddly to pass hex values in from erlang but we could convert integers
	if(!enif_get_string(env, argv[0], mapName, maxBuffLen, ERL_NIF_LATIN1) || !enif_get_string(env, argv[1], formatName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	
	if(!enif_get_int(env, argv[2], &r) || !enif_get_int(env, argv[3], &g) || !enif_get_int(env, argv[4], &b)){
		return enif_make_badarg(env);
	}
	if(r<0 || r>255 || g<0 || g>255 || b<0 || b>255){
		return enif_make_string(env, "R, G, or B value out of bounds in sdl_mapRGB. Please ensure value is 0<=X<=255", ERL_NIF_LATIN1);
	}
	
	//Lookup map name
	std::string mapString(mapName);
	int mIndex = mapIndex(mapString);
	
	bool mapExists = true;
	if(mIndex<0){
		//Map doesn't exist create a new one
		mIndex=mapNames.size();
		mapNames.push_back(mapString);
		mapExists = false;
	}
	//otherwise map exists, overwrite
	
	//Check format exists
	std::string formatString(formatName);
	int formatIndex = surfaceIndex(formatString);
	
	if(formatIndex<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_MapRGB", ERL_NIF_LATIN1);
	}
	
	Uint8 rHex = (Uint8) r;
	Uint8 gHex = (Uint8) g;
	Uint8 bHex = (Uint8) b;
	
	SDL_PixelFormat* pixelFormat = surfaceArray[formatIndex]->format;
	if(mapExists){
		rgbMaps[mIndex] = SDL_MapRGB(pixelFormat, rHex, gHex, bHex);
	}
	else{
		rgbMaps.push_back(SDL_MapRGB(pixelFormat, rHex, gHex, bHex));
	}
	
	return enif_make_int(env, 0); /*Exit code*/
}

/**
*	Wrapped MUSTLOCK SDL function
*	@param surfaceName Name of the surface to check lock status on
*	@return 0 if True, 1 if False, Error srting otherwise.
**/
ERL_NIF_TERM sdl_MUSTLOCK (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_MUSTLOCK" , ERL_NIF_LATIN1);
	}
	
	//Apparently Nifs don't have a make boolean option, so we've got to use int instead
	if(SDL_MUSTLOCK(surfaceArray[index])){
		//return TRUE
		return enif_make_int(env,0);
	}
	//return FALSE
	return enif_make_int(env,1);
}

/**
*	Wrapped LockSurface SDL Function. Locks a surface.
*	@Params surfaceName Name of surface to be locked.
*	@return 0 on success, error string otherwise.
**/
ERL_NIF_TERM sdl_LockSurface (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_LockSurface" , ERL_NIF_LATIN1);
	}
	
	 if ( SDL_LockSurface(surfaceArray[index]) < 0 ) {
            return enif_make_string(env, "Surface couldn't be locked in sdl_LockSurface" , ERL_NIF_LATIN1);
        }
	return enif_make_int(env,0); /*Exit code*/
}

/**
*	Wrapped UnlockSurface SDL Function. Unlocks a surface.
*	@Params surfaceName Name of surface to be unlocked.
*	@return 0 on success, error string otherwise.
**/
ERL_NIF_TERM sdl_UnlockSurface (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_LockSurface" , ERL_NIF_LATIN1);
	}
	
	SDL_UnlockSurface(surfaceArray[index]);
	return enif_make_int(env,0); /*Exit code*/
}

/**
*	New function for this library. Sets the rgb value of a pixel in a surface.
*	@param surfaceName Name of the target surface, x and y are the co-ordinates of the pixel, mapName Name of the RGB Map the pixel will inherit the colour value from.
*	@Return 0 on success, error string otherwise.
**/
ERL_NIF_TERM sdl_SetPixel (ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
	char surfaceName[maxBuffLen];
	char mapName[maxBuffLen];
	int x, y;
	if(!enif_get_string(env, argv[0], surfaceName, maxBuffLen, ERL_NIF_LATIN1) || !enif_get_string(env, argv[3], mapName, maxBuffLen, ERL_NIF_LATIN1)){
		return enif_make_badarg(env);
	}
	
	if(!enif_get_int(env, argv[1], &x) || !enif_get_int(env, argv[2], &y)){
		return enif_make_badarg(env);
	}
	
	std::string surfaceString(surfaceName);
	int index = surfaceIndex(surfaceString);
	if(index<0){
		return enif_make_string(env, "SurfaceIndex returned <0 in sdl_SetPixel" , ERL_NIF_LATIN1);
	}
	
	std::string mapString(mapName);
	int mIndex = mapIndex(mapString);
	if(mIndex<0){
		return enif_make_string(env, "MapIndex returned <0 in sdl_SetPixel" , ERL_NIF_LATIN1);
	}
	
	Uint32 newPixel = rgbMaps[mIndex];
	
	//Get the bytes per pixel and the old pixel value
	int bpp = surfaceArray[index]->format->BytesPerPixel;
	Uint8 *oldPixel = (Uint8 *)surfaceArray[index]->pixels + y * surfaceArray[index]->pitch + x * bpp;
	
	if (bpp == 1){
		*oldPixel = newPixel;
	}
	else if (bpp == 2){
		*(Uint16 *)oldPixel = newPixel;
	}
	else if (bpp == 3){
		//For a bits per pixel size of 3 the order of pixels may be reversed
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            oldPixel[0] = (newPixel >> 16) & 0xff;
            oldPixel[1] = (newPixel >> 8) & 0xff;
            oldPixel[2] = newPixel & 0xff;
        } else {
            oldPixel[0] = newPixel & 0xff;
            oldPixel[1] = (newPixel >> 8) & 0xff;
            oldPixel[2] = (newPixel >> 16) & 0xff;
        }
	}
	else if (bpp == 4){
		*(Uint32 *)oldPixel = newPixel;
	}
	else{
		//We shouldn't get here, if we do there's a problem with bpp
		return enif_make_string(env, "Bytes per pixel found to be of an invalid range in sdl_SetPixel()", ERL_NIF_LATIN1);
	}
	return enif_make_int(env,0);
}

/**
*	An array of the functions (as they are referred to by Erlang, their arity, and the function name in cpp)
**/
static ErlNifFunc nif_funcs[] = {
	{"sdl_Init", 1, sdl_Init},
	{"sdl_Quit", 0, sdl_Quit},
	{"sdl_CreateSurface",1,sdl_CreateSurface},
	{"sdl_SetVideoMode",5,sdl_SetVideoMode},
	{"sdl_LoadBMP",2,sdl_LoadBMP},
	{"sdl_BlitSurface",4,sdl_BlitSurface},
	{"sdl_Flip",1,sdl_Flip},
	{"sdl_Delay",1,sdl_Delay},
	{"sdl_FreeSurface",1,sdl_FreeSurface},
	{"sdl_UpdateRect",5,sdl_UpdateRect},
	{"sdl_GetPixelFormat", 1, sdl_GetPixelFormat},
	{"sdl_MapRGB",5,sdl_MapRGB},
	{"sdl_MUSTLOCK",1,sdl_MUSTLOCK},
	{"sdl_LockSurface",1,sdl_LockSurface},
	{"sdl_UnlockSurface",1,sdl_UnlockSurface},
	{"sdl_SetPixel",4,sdl_SetPixel}
};

/**
*	Variable for initialising the NIFs
**/
ERL_NIF_INIT(INSERT_ERLANG_MODULE_NAME, nif_funcs, NULL, NULL, NULL, NULL)

/*
  Simple program to help with running librespot and handling audio.
  What it does :
  - Display a simple background image (containing usage instructions)
  - Start librespot program with a pipe to handle audio data
  - Feed the audio data to SDL audio component
  - Stop librespot program when exiting

  Many thanks to the librespot-org project: https://github.com/librespot-org/librespot
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "SDL.h"
#include "SDL_ttf.h"

#define INITIAL_WINDOW_WIDTH    1280
#define INITIAL_WINDOW_HEIGHT   720

#define AUDIO_SAMPLES           4096

#define LIBRESPOT_START_CMD     "/home/apps/spotify4steamlink/librespot-org-build/arm-unknown-linux-gnueabihf/release/librespot --cache /var/cache --disable-audio-cache --name steamlink --bitrate 320 --initial-volume 85 --backend pipe 2>/tmp/spotify.log"
#define LIBRESPOT_KILL_CMD      "pidof /home/apps/spotify4steamlink/librespot-org-build/arm-unknown-linux-gnueabihf/release/librespot | xargs kill"
#define LIBRESPOT_LOG_FILE      "/tmp/spotify.log"

#define BACKGROUND_BMP_FILE     "spotify3.bmp"

#define FONT_FILE               "consolas.ttf"
#define FONT_SIZE               18

#define SCREEN_MAX_LINES        29

// Disable this flag when publishing to steamlink
//#define TEST_MODE


/* -- Global variables -- */

static SDL_Texture *sprite;
static int sprite_w, sprite_h;
SDL_Renderer *renderer;
int done;
static FILE *audio_buf; // global pointer to the audio buffer to be played
TTF_Font *font = NULL;
static FILE *spotify_log_file;
SDL_Color text_color = {255, 255, 255}; // white
SDL_Surface *text_surf = NULL;
static int text_square_pos_x, text_square_pos_y, text_square_pos_w, text_square_pos_h;


/* -- Functions -- */

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    exit(rc);
}

void compute_text_square_dimensions(SDL_Window* window)
{
    int window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    // Converting dimensions from a 1280*720 screen if necessary
    text_square_pos_x = 455 * window_w / 1280;
    text_square_pos_y = 84 * window_h / 720;
    text_square_pos_w = 700 * window_w / 1280;
    text_square_pos_h = 555 * window_h / 720;
}

int
LoadSprite(char *file, SDL_Renderer *renderer)
{
    SDL_Surface *temp;

    /* Load the sprite image */
    temp = SDL_LoadBMP(file);
    if (temp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load %s: %s\n", file, SDL_GetError());
        return (-1);
    }
    sprite_w = temp->w;
    sprite_h = temp->h;

    /* Create textures from the image */
    sprite = SDL_CreateTextureFromSurface(renderer, temp);
    if (!sprite) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(temp);
        return (-1);
    }
    SDL_FreeSurface(temp);

    /* We're ready to roll. :) */
    return (0);
}

// Read the tail of the log file (n = nb of lines to be read)
char* tailLogFile(int n)
{
  char * buffer;
  size_t result;
  int count = 0;  // To count '\n' characters
  unsigned long long lSize; // Total log size
  unsigned long long pos; // Reader position

  // obtain file size:
  rewind(spotify_log_file);
  fseek (spotify_log_file , 0 , SEEK_END);
  lSize = ftell (spotify_log_file);
  pos = lSize;

  // search for '\n' characters
  while (pos)
  {
    if (!fseek(spotify_log_file, pos-=sizeof(char), SEEK_SET))
    {
        if (fgetc(spotify_log_file) == '\n')
        {
            // stop reading when n newlines are found
            if (count++ == n)
            {
                break;
            }
        }
    }
  }

  // if we have stopped on a '\n' char, we ignore it
  if (count == n)
  {
     fseek(spotify_log_file, sizeof(char), SEEK_CUR);
  }

  // allocate memory to contain the last n lines:
  buffer = (char*) malloc (sizeof(char)*(lSize - pos));
  if (buffer == NULL) {quit(2);}

  // copy the file into the buffer:
  result = fread (buffer, sizeof(char), (lSize - pos), spotify_log_file);

  return buffer;
}

// Render the text surface
void
renderText(SDL_Renderer * renderer)
{
    // Write our text into the surface
    char* buffer = tailLogFile(SCREEN_MAX_LINES);
    text_surf = TTF_RenderUTF8_Blended_Wrapped(font, buffer, text_color, text_square_pos_w);
    free(buffer);
    if (text_surf == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create text surface from char buffer: %s\n", SDL_GetError());
        return;
    }
    // Create texture from the surface
    SDL_Texture *sprite = SDL_CreateTextureFromSurface(renderer, text_surf);
    if (!sprite) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from text surface: %s\n", SDL_GetError());
        SDL_FreeSurface(text_surf);
        quit (-1);
    }
    // Set the position and size of source and destination rectangles
    int actual_w = text_surf->w;
    int actual_h = (text_surf->h < text_square_pos_h) ? text_surf->h : text_square_pos_h;
    SDL_Rect *srcrect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    srcrect->x = 0;
    srcrect->y = 0;
    srcrect->w = actual_w;
    srcrect->h = actual_h;
    SDL_Rect *dstrect = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    dstrect->x = text_square_pos_x;
    dstrect->y = text_square_pos_y;
    dstrect->w = actual_w;
    dstrect->h = actual_h;
    // Blit the text onto the screen
    SDL_RenderCopy(renderer, sprite, srcrect, dstrect);
    // Free memory
    SDL_FreeSurface(text_surf);
    SDL_DestroyTexture(sprite);
    free(dstrect);
    free(srcrect);
}

// Render a sprite fullscreen
void
renderBackground(SDL_Renderer * renderer, SDL_Texture * sprite)
{
    /* Draw a dark background */
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    /* Blit the sprite onto the screen */
    SDL_RenderCopy(renderer, sprite, NULL, NULL);
}

// audio callback function
// Read fifo generated by spotify and add it to the audio queue
void my_audio_callback(void *userdata, Uint8 *dest_stream, int len) {
    SDL_memset(dest_stream, 0, len);  // init with silence.
    Sint16 *stream = (Sint16 *)dest_stream;
    fread(stream, sizeof(Uint8), len, audio_buf); // read as much as we can into stream buffer
}

void loop()
{
    SDL_Event event;

    /* Check for events */
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            SDL_GameControllerOpen(event.cdevice.which);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_KEYDOWN:
        case SDL_QUIT:
            done = 1;
            break;
        }
    }

    renderBackground(renderer, sprite);
    renderText(renderer);
    /* Update the screen! */
    SDL_RenderPresent(renderer);

#ifdef __EMSCRIPTEN__
    if (done) {
        emscripten_cancel_main_loop();
    }
#endif
}

// Init audio device and audio spec + start playing
void initAudio() {
    // Initialize SDL.
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
			quit(1);

	static SDL_AudioSpec want; // the specs of our piece of music
	SDL_memset(&want, 0, sizeof(want));
	want.freq = 44100;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = AUDIO_SAMPLES;
	// set the callback function
	want.callback = my_audio_callback;
	//want.callback = NULL;
	want.userdata = NULL;
	/* Open the audio device */
	if ( SDL_OpenAudio(&want, NULL) < 0 ){
	  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: %s\n", SDL_GetError());
	  quit(-1);
	}

	/* Start playing */
	SDL_PauseAudio(0);
}

// Open audio file buffer
void openAudioBuffer() {

    /* Start librespot and gather std output stream */
#ifdef TEST_MODE
    audio_buf = fopen("sample.bin", "rb");
#else
	// Send the SIGTERM signal to librespot, to kill any pre-existing daemon still running
	system(LIBRESPOT_KILL_CMD);
    // Make sure the env variable "http_proxy" is not set
	unsetenv("http_proxy");
    audio_buf = popen(LIBRESPOT_START_CMD, "r");
#endif // TEST_MODE

	if (audio_buf == NULL){
	  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error starting librespot: %s\n", SDL_GetError());
	  quit(-1);
	}
}

// Close audio file buffer properly
void closeAudioBuffer() {
#ifdef TEST_MODE
    if (audio_buf)
    {
        fclose(audio_buf);
    }
#else
	// Send the SIGTERM signal to librespot
	system(LIBRESPOT_KILL_CMD);
	if (audio_buf)
    {
        // Close the pipe to librespot
        pclose(audio_buf);
    }
#endif // TEST_MODE
}

// Stop the audio device properly
void stopAudio() {
    // shut everything down
    SDL_PauseAudio(0);
	SDL_Delay(1000); // let audio callback run for 1 sec.
	SDL_CloseAudio();
}

void openSpotifyLogFile() {

#ifdef TEST_MODE
    spotify_log_file = fopen("spotify.log", "r");
#else
    spotify_log_file = fopen(LIBRESPOT_LOG_FILE, "r");
#endif // TEST_MODE

	if (spotify_log_file == NULL){
	  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error opening librespot log file: %s\n", SDL_GetError());
	  quit(-1);
	}
}

void closeSpotifyLogFile() {
    fclose(spotify_log_file);
}


/* -- MAIN function -- */

int
main(int argc, char *argv[])
{
    // Close any pre-existing spotify instance: let's start clean!
    closeAudioBuffer();

    /* SDL Init - Graphics */
    SDL_Window *window;
    // Enable standard application logging
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    // Create window and renderer
#ifdef TEST_MODE
    Uint32 flags = SDL_WINDOW_SHOWN;
#else
    Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif // TEST_MODE
    if (SDL_CreateWindowAndRenderer(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, flags, &window, &renderer) < 0) {
        quit(2);
    }
    // Compute text square dimensions
    compute_text_square_dimensions(window);
    // Load the background image
    if (LoadSprite(BACKGROUND_BMP_FILE, renderer) < 0) {
        quit(2);
    }
    // Init subsystem with game controller support
    SDL_InitSubSystem( SDL_INIT_GAMECONTROLLER );

    /* SDL Init - Audio */
    // Start librespot and open audio buffer
    openAudioBuffer();
    // Open audio device and start playback
    initAudio();

    /* SDL Init - TTF */
    if(TTF_Init() == -1)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error in TTF_Init: %s\n", TTF_GetError());
        quit(EXIT_FAILURE);
    }
    // Load Font file
    font = TTF_OpenFont(FONT_FILE, FONT_SIZE);
    // Load Spotify log file
    openSpotifyLogFile();


    /* Main render loop */
    done = 0;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!done) {
        loop();
    }
#endif

    /*SDL Close - TTF */
    closeSpotifyLogFile();
    TTF_CloseFont(font);
    TTF_Quit();

    /* SDL Close - Audio */
    stopAudio();
    closeAudioBuffer();

    /* SDL Close - Graphics */
    SDL_QuitSubSystem( SDL_INIT_GAMECONTROLLER );

    quit(0);

    return 0; /* to prevent compiler warning */
}



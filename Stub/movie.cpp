#include "../3rdParty/libsmackerdec/include/SmackerDecoder.h"
#include "../types.h"

BYTE movie_playing;
BOOL loop_movie;

void __fastcall play_movie(char *pszMovie, BOOL user_can_close)
{
	DUMMY_PRINT("%s", pszMovie);
	void *video_stream;

	movie_playing = TRUE;
	//sound_disable_music(TRUE);
	sfx_stop();
	effects_play_sound("Sfx\\Misc\\blank.wav");

	//SVidPlayBegin(pszMovie, 0, 0, 0, 0, loop_movie ? 0x100C0808 : 0x10280808, &video_stream);
	if (video_stream) {
		SDL_Event event;
		while (video_stream) {
			if (user_can_close && !movie_playing)
				break;

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
				case SDL_MOUSEBUTTONDOWN:
					movie_playing = false;
					break;
				case SDL_QUIT:
					exit(0);
				}
			}
			//Mix_PlayChannel(-1, pSnd->chunk, 0);
			video_stream = NULL;
		}
		//if (video_stream)
			//SVidPlayEnd(video_stream);
	}
	//sound_disable_music(FALSE);
}

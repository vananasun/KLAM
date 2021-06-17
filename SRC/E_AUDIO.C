#include "E.H"
#include "E_AUDIO.H"
#include "E_FILE.H"
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <math.h>////TESTING

#define BUFFER_SIZE 512

#define SB_RESET                   0x6
#define SB_READ_DATA               0xA
#define SB_WRITE_DATA              0xC
#define SB_READ_DATA_STATUS        0xE
#define SB_ENABLE_SPEAKER          0xD1
#define SB_DISABLE_SPEAKER         0xD3
#define SB_SET_PLAYBACK_FREQUENCY  0x40
#define SB_SINGLE_CYCLE_PLAYBACK   0x14
#define SB_START_AUTOINIT_PLAYBACK 0xDA

#define MASK_REGISTER     0x0A
#define MODE_REGISTER     0x0B
#define MSB_LSB_FLIP_FLOP 0x0C
#define DMA_CHANNEL_0     0x87
#define DMA_CHANNEL_1     0x83
#define DMA_CHANNEL_3     0x82

#define MAX_SOUNDS 4
#define SOUND_SIZE 32768 / 2


int16 s_SBBase; // default 220h
int8 s_SBIRQ = 7; // default 7
int8 s_SBDMA = 1; // default 1
void interrupt (*s_OldIRQ)();

volatile uint8 s_Playing;
volatile uint32 s_ToBePlayed;
uint8* far s_DMABuffer;
int16 s_Page, s_Offset;


typedef struct {
	uint8 active;
	uint8* far buffer;
} Sound;

static Sound s_Sounds[MAX_SOUNDS];
static FILE* s_SoundFiles[MAX_SOUNDS];
static uint32 s_ReadPositions[MAX_SOUNDS];
static uint32 s_SamplePositions[MAX_SOUNDS];
static uint8 s_SoundCount = 0;
static uint8 s_ActiveVoices = 0;




static void interrupt sbIRQHandler() {
	// Log("IRQ Handler called", s_Playing);
	// inp(s_SBBase + SB_READ_DATA_STATUS);
	// outp(0x20, 0x20);
	// if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
	// 	outp(0xA0, 0x20);
	// }
	s_Playing = FALSE;
}

static uint8 resetDSP(int16 port) {
	outp(port + SB_RESET, 1);
	delay(3);
	outp(port + SB_RESET, 0);
	delay(3);
	if ( ((inp(port + SB_READ_DATA_STATUS) & 0x80) == 0x80)
	   && (inp(port + SB_READ_DATA) == 0x0AA))
	{
		s_SBBase = port;
		return TRUE;
	}
	return FALSE;
}

static void writeDSP(uint8 command) {
	while ((inp(s_SBBase + SB_WRITE_DATA) & 0x80) == 0x80);
	outp(s_SBBase + SB_WRITE_DATA, command);
}

static void initIRQ() {

	// @TODO: optimize into just 3 ifs and an else

	// Save the old IRQ vector
	if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
		if      (s_SBIRQ == 2 ) s_OldIRQ = _dos_getvect(0x71);
		else if (s_SBIRQ == 10) s_OldIRQ = _dos_getvect(0x72);
		else if (s_SBIRQ == 11) s_OldIRQ = _dos_getvect(0x73);
	} else {
		s_OldIRQ = _dos_getvect(s_SBIRQ + 8);
	}

	// Set our own IRQ vector
	if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
		if      (s_SBIRQ == 2 ) _dos_setvect(0x71, sbIRQHandler);
		else if (s_SBIRQ == 10) _dos_setvect(0x72, sbIRQHandler);
		else if (s_SBIRQ == 11) _dos_setvect(0x73, sbIRQHandler);
	} else {
		_dos_setvect(s_SBIRQ + 8, sbIRQHandler);
	}

	// Enable the IRQ with the mainboard's PIC
	if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
		if      (s_SBIRQ == 2 ) outp(0xA1, inp(0xA1) & 253);
		else if (s_SBIRQ == 10) outp(0xA1, inp(0xA1) & 251);
		else if (s_SBIRQ == 11) outp(0xA1, inp(0xA1) & 247);
		outp(0x21, inp(0x21) & 251);
	} else {
		outp(0x21, inp(0x21) & !(1 << s_SBIRQ));
	}
}

static void releaseIRQ() {
	// Restore the old IRQ vector
	if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
		if      (s_SBIRQ == 2 ) _dos_setvect(0x71, s_OldIRQ);
		else if (s_SBIRQ == 10) _dos_setvect(0x72, s_OldIRQ);
		else if (s_SBIRQ == 11) _dos_setvect(0x73, s_OldIRQ);
	} else {
		_dos_setvect(s_SBIRQ + 8, s_OldIRQ);
	}


	if (s_SBIRQ == 2 || s_SBIRQ == 10 || s_SBIRQ == 11) {
		if      (s_SBIRQ == 2 ) outp(0xA1, inp(0xA1) | 2);
		else if (s_SBIRQ == 10) outp(0xA1, inp(0xA1) | 4);
		else if (s_SBIRQ == 11) outp(0xA1, inp(0xA1) | 8);
		outp(0x21, inp(0x21) | 4);
	} else {
		outp(0x21, inp(0x21) & (1 << s_SBIRQ));
	}
}

static void assignDMABuffer() {
	uint8* tempBuffer;
	uint16 page1, page2;
	uint32 linearAddress;

	tempBuffer = (uint8*)malloc(BUFFER_SIZE);
	linearAddress = FP_SEG(tempBuffer);
	linearAddress = (linearAddress << 4) + FP_OFF(tempBuffer);
	page1 = linearAddress >> 16;
	page2 = (linearAddress + (BUFFER_SIZE-1)) >> 16;
	if (page1 != page2) {
		// Clever hack; If the buffer we attempted to allocate did not fit
		// inside it's page, then if we allocate another one and free the
		// first one then the second will surely be on the second page, and
		// thus will always fit!
		s_DMABuffer = (uint8*)malloc(BUFFER_SIZE);
		if (!s_DMABuffer) {
			Error("s_DMABuffer was %p on second allocation", s_DMABuffer);
		}
		free(tempBuffer);
	} else {
		s_DMABuffer = tempBuffer;
		if (!s_DMABuffer) {
			Error("s_DMABuffer was %p on first allocation", s_DMABuffer);
		}

	}

	linearAddress = FP_SEG(s_DMABuffer);
	linearAddress = (linearAddress << 4) + FP_OFF(s_DMABuffer);
	s_Page = linearAddress >> 16;
	s_Offset = linearAddress & 0xFFFF;

}

static uint8 sbDetect() {
	char* BLASTER;
	uint8 i, len;

	// Possible values: 210, 220, 230, 240, 260, 260, 280
	for (i = 1; i < 9; i++) {
		if ( (i != 7) && (resetDSP(0x200 + (i << 4))) ) {
			break;
		}
	}
	if (9 == i)
		return FALSE;

	// When there's no BLASTER environment variable, we will have to guess the
	// IRQ and DMA values.
	BLASTER = getenv("BLASTER");
	if (0 == BLASTER) {
		Log("BLASTER unset! Defaulting to D1 I7.");
		return TRUE;
	}
	
	// Look for DMA
	len = strlen(BLASTER);
	for (i = 0; i < len; i++) {
		if ((BLASTER[i] | 32) == 'd') {
			s_SBDMA = BLASTER[i + 1] - '0';
			break;
		}
	}

	// Look for IRQ
	for (i = 0; i < len; i++) {
		if ((BLASTER[i] | 32) == 'i') {
			s_SBIRQ = BLASTER[i + 1] - '0';
			if (BLASTER[i + 2] >= '0' && BLASTER[i + 2] <= '9')
				s_SBIRQ = s_SBIRQ * 10 + BLASTER[i + 2] - '0';
			break;
		}
	}

	return TRUE;
}

static void sbInit() {
	initIRQ();
	assignDMABuffer();
	writeDSP(SB_ENABLE_SPEAKER);
}

static void sbRelease() {
	writeDSP(SB_DISABLE_SPEAKER);
	SAFE_DELETE_PTR(s_DMABuffer);
	releaseIRQ();
}



static void singleCyclePlayback() {
	s_Playing = TRUE;
	// Program the DMA controller
	outp(MASK_REGISTER, 4 | s_SBDMA);
	outp(MSB_LSB_FLIP_FLOP, 0);
	outp(MODE_REGISTER, 0x48 | s_SBDMA);
	outp(s_SBDMA << 1, s_Offset & 0xFF);
	outp(s_SBDMA << 1, s_Offset >> 8);
	switch (s_SBDMA) {
	case 0: outp(DMA_CHANNEL_0, s_Page); break;
	case 1: outp(DMA_CHANNEL_1, s_Page); break;
	case 3: outp(DMA_CHANNEL_3, s_Page); break;
	}

	outp((s_SBDMA << 1) + 1, s_ToBePlayed & 0xFF);
	outp((s_SBDMA << 1) + 1, s_ToBePlayed >> 8);
	outp(MASK_REGISTER, s_SBDMA);
	writeDSP(SB_SINGLE_CYCLE_PLAYBACK);
	writeDSP(s_ToBePlayed & 0xFF);
	writeDSP(s_ToBePlayed >> 8);
	// writeDSP(SB_START_AUTOINIT_PLAYBACK);
	s_ToBePlayed = 0;
}

static void sbSinglePlay(uint8 soundId) {
	uint32 sample;
	
	memset(s_DMABuffer, 0, BUFFER_SIZE);

	
	// for (sample = 0; sample < BUFFER_SIZE; sample++) {
		
	// 	s_SamplePositions[soundId]++;
	// }

	writeDSP(SB_SET_PLAYBACK_FREQUENCY);
	writeDSP(256 - 1000000 / 22000); // 11000 = samplerate in HZ
	s_ToBePlayed = BUFFER_SIZE;
	singleCyclePlayback();
}

extern volatile float g_DTP;
static volatile uint32 smp = 0;
void Audio_Mix() {
	uint8 value;
	uint8 soundId;
	s_ActiveVoices = 1;
		outp(0x22C, 0x10);
	for (soundId = 0; soundId < s_ActiveVoices; soundId++) {
		// outp(0x22C, (rand()%255));
		value = (int)((sin(2.0f * 3.141593f * 432.0f * g_DTP * smp / 22000) / 2.0f + 0.5f) * 255);
		value *= g_DTP / 9.0f;
		outp(0x22C, value);
		smp++;
	}

// 	push ds
// 	push es
// 	push si
// 	push di
	
// 	mov ax, active_sounds_segment
// 	mov es, ax
	
// 	mov dx, 127 ; --> result mixed sample
	
// 	mov si, 0
// 	mov cl, 0
// .next_sound:

// 	mov al, [es:active_sounds + si] ; sound s (status)
// 	cmp al, 0
// 	jz .continue
	
// 	; mix sound
// 	mov bx, [es:active_sounds + si + 7] ; sound index
// 	mov ax, [es:active_sounds + si + 3] ; sound segment
// 	mov ds, ax
// 	mov di, [es:active_sounds + si + 1] ; sound offset
	// 	mov ah, 0
	// 	mov al, [ds:di + bx] ; get sound byte sample
// 	add dx, ax
// 	sub dx, 127 ; mixed_sample = sample_a + sample_b - 127
// 	;mov dx, ax
	
// 	inc word [es:active_sounds + si + 7] ; increment sound index
// 	mov bx, [es:active_sounds + si + 7] ; sound index
// 	cmp bx, [es:active_sounds + si + 5] ; end of sound
// 	jb .continue

// 	mov word [es:active_sounds + si + 7], 300 ; sound index

// 	; is loop ?
// 	cmp byte [es:active_sounds + si], 2
// 	je .continue
	
// 	; end of sound
// 	mov byte [es:active_sounds + si], 0 ; sound s (status)
	
// .continue:
// 	add si, 9
// 	inc cl
// 	cmp cl, 15
// 	jbe .next_sound
	
// 	; limit sample 0~255
// 	cmp dx, 255
// 	jbe .play_mixed_sample
// 	mov dx, 255
	
// .play_mixed_sample:
// 	mov bl, dl
	
// 	; send DSP Command 10h
// 	mov dx, 22ch
// 	mov al, 10h
// 	out dx, al

// 	; send byte audio sample
// 	mov al, bl
// 	out dx, al
	
// .end:
// 	pop di
// 	pop si
// 	pop es
// 	pop ds
// 	retf			
}

uint8 Audio_LoadWAV(const char* filename) {
	// uint32 length;
	// FILE* f;
	// if (s_SoundCount >= MAX_SOUNDS) {
	// 	Log("Max sounds reached.");
	// 	return FALSE;
	// }
	// f = fopen(filename, "rb");
	// if (!f) Error("Failed loading sound \"%s\".", filename);
	// length = File_GetLength(f);
	// s_ReadPositions[s_SoundCount] = length; // ? for streaming ?
	// fread(s_Sounds[s_SoundCount], MIN(length, SOUND_SIZE), 1, f);
	// fclose(f);
	// s_SamplePositions[s_SoundCount] = 0;
	// s_SoundCount++;
	return TRUE;
}

void Audio_Init() {


	if (!sbDetect()) {
		Error("Sound failed to initialize.");
	}

	Log("Sound Blaster found at A%x I%u D%i", s_SBBase, s_SBIRQ, s_SBDMA);

	sbInit();
	//sbSinglePlay("assets/audio/step1a.wav");

	sbSinglePlay(0);


}

void Audio_Cleanup() {
	// @TODO: if (!s_Initialized) return;
	sbRelease();
}


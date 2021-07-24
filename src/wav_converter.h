#include "intermediary.h"

#define WAV_FORMAT_MONO8    0x1100
#define WAV_FORMAT_MONO16   0x1101
#define WAV_FORMAT_STEREO8  0x1102
#define WAV_FORMAT_STEREO16 0x1103

struct Audio
{
	int format;
	int sample_rate;
	int size;
	byte* data;
};

typedef signed short int16;
typedef int int32;
typedef unsigned int uint;

void convert_wav(const char* path, const char* name)
{
	print("converting %s\n\n", path);

	FILE* file = fopen(path, "rb"); // rb = read binary
	if (file == NULL) { print("File not found"); return; }

	int32 header[12] = {};
	fread(header, sizeof(int32), 11, file);

	// we only support PCM files right now :(
	if ((header[5] & 131071) != 1) { print("unsupported file type"); fclose(file); return; }

	Audio audio = {};
	audio.sample_rate = header[6];
	audio.size        = header[10];

	// check number of channels & bits per sample
	if ((header[5] >> 16) == 1)
	{
		if ((header[8] >> 16) == 8)
			audio.format = WAV_FORMAT_MONO8;
		else
			audio.format = WAV_FORMAT_MONO16;
	}
	else
	{
		if ((header[8] >> 16) == 8)
			audio.format = WAV_FORMAT_STEREO8;
		else
			audio.format = WAV_FORMAT_STEREO16;
	}

	// read waveform data
	audio.data = (byte*)calloc(audio.size, sizeof(byte));
	fread(audio.data, sizeof(byte), audio.size, file);

	fclose(file);

	// create .audio file
	file = fopen(name, "wb");
	fwrite(&audio.format     , sizeof(uint), 1, file);
	fwrite(&audio.sample_rate, sizeof(uint), 1, file);
	fwrite(&audio.size       , sizeof(uint), 1, file);
	fwrite(audio.data        , sizeof(byte), audio.size, file);
	fclose(file);
}

void print_wav(const char* path)
{
	// these variable names should be illegal
	int   val = 0;
	int16 val2 = 0, channels, bps;
	char  val3[5] = {};
	
	FILE* file = fopen(path, "rb");
	if (file == NULL) { out("File not found"); return; }

	// RIFF
	fread(&val3, sizeof(int), 1, file);
	out("header is " << val3);

	//FILE SIZE
	fread(&val, sizeof(int), 1, file);
	out("File size: " << val);

	//WAVE
	fread(&val3, sizeof(int), 1, file);
	out("format is " << val3);

	//FORMAT
	fread(&val3, sizeof(int), 1, file);
	out("format is " << val3);

	//LENGTH OF FORMAT DATA ABOVE (16)
	fread(&val, sizeof(int), 1, file);
	out("Length of format data: " << val);

	//TYPE OF FORMAT
	fread(&val2, sizeof(int16), 1, file);
	out("Type of format data: " << val2);

	//NUMBER OF CHANNELS
	fread(&channels, sizeof(int16), 1, file);
	out("Number of channels: " << channels);

	//SAMPLE RATE
	fread(&val, sizeof(int), 1, file);
	out("Sample rate: " << val);

	//(Sample Rate * BitsPerSample * Channels) / 8 = BITS PER SECOND
	fread(&val, sizeof(int), 1, file);
	out("Bits per second: " << val);

	//(BitsPerSample * Channels) / 8.1 -> 8 bit mono2 – 8 bit stereo/16 bit mono4 – 16 bit stereo
	fread(&val2, sizeof(int16), 1, file);
	out("Audio type: " << val2);

	//BITS PER SAMPLE
	fread(&bps, sizeof(int16), 1, file);
	out("Bits per sample: " << bps);

	if (channels == 1)
	{
		if (bps == 8)
			out("Format: Mono 8");
		else
			out("Format: Mono 16");
	}
	else
	{
		if (bps == 8)
			out("Format: Stereo 8");
		else
			out("Format: Stereo 16");
	}

	//DATA
	fread(&val3, sizeof(int), 1, file);
	out("header is " << val3);

	//SIZE OF DATA
	fread(&val, sizeof(int), 1, file);
	out("Data size: " << val);

	fclose(file);
}
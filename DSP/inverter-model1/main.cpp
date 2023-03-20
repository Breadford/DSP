
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "D:\AADSP2\PROJEKAT\Vezbe\WAVheader.h"

#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 8

// Number of channels
#define INPUT_NUM_CHANNELS 2
#define OUTPUT_NUM_CHANNELS 4

// Channel IDs. 
// Should input and output channel IDs be separated?
#define LEFT_CH 0
#define RIGHT_CH 1
#define LS_CH 2
#define RS_CH 3

// Gain linear values. 
#define MINUS_3DB 0.707946 

// IO Buffers
static double sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];

// Processing related variables
static double variablesGain[INPUT_NUM_CHANNELS] = {MINUS_3DB, MINUS_3DB};

//Mode
static int mode = 0;


//Inverter
typedef struct {
	double degree;
	double gain;
} inverter_data_t;

//Global inverter
static inverter_data_t inverter;

void audio_invert_init(inverter_data_t* data, double degree, double gain)
{
	data->degree = degree;
	//linear gain
	data->gain = gain;
}

static void
gst_audio_invert_transform(inverter_data_t* data,
	double* input, double* output)
{
	int i;
	double dry = 1.0 - data->degree;
	double val;
	
	for (i = 0; i < BLOCK_SIZE; i++) {
		val = *input * dry - *input * data->degree;
		*output = val * data->gain;
		input++;
		output++;
	}
}

void gainProcessing(double pIn[][BLOCK_SIZE], double pOut[][BLOCK_SIZE])
{
	int i;
	//Model 1 
	double* samplePtrInL = *(pIn + LEFT_CH);
	double* samplePtrOutL = *(pOut + LEFT_CH);

	double* samplePtrInR = *(pIn + RIGHT_CH);
	double* samplePtrOutR = *(pOut + RIGHT_CH);

	double* samplePtrOutLs = *(pOut + LS_CH);
	double* samplePtrOutRs = *(pOut + RS_CH);

	double variableGainL = *(variablesGain + LEFT_CH);
	double variableGainR = *(variablesGain + RIGHT_CH);

	for (i = 0; i < BLOCK_SIZE; i++) {

		//Left
		*samplePtrInL = *(samplePtrInL) * variableGainL;
		*samplePtrOutL = *samplePtrInL;

		//Right
		*samplePtrInR = *(samplePtrInR) * variableGainR;
		*samplePtrOutR= *samplePtrInR;

		//Ls
		*samplePtrOutLs = *samplePtrInL;

		//Rs
		*samplePtrOutRs = *samplePtrInR;

		*samplePtrInL++;
		*samplePtrInR++;
		*samplePtrOutL++;
		*samplePtrOutR++;
		*samplePtrOutLs++;
		*samplePtrOutRs++;

	}

	if (mode == 1) {

		samplePtrInL -= BLOCK_SIZE;
		samplePtrInR -= BLOCK_SIZE;
		samplePtrOutL -= BLOCK_SIZE;
		samplePtrOutR -= BLOCK_SIZE;

		gst_audio_invert_transform(&inverter, samplePtrInL, samplePtrOutL);
		gst_audio_invert_transform(&inverter, samplePtrInR, samplePtrOutR);
	}

}

/////////////////////////////////////////////////////////////////////////////////
// @Author	<student name>
// @Date		<date>  
//
// Function:
// main
//
// @param - argv[0] - Input file name        last test case funky_sample_mod0       
//        - argv[1] - Output file name
//		  - argv[3] - enable                 1                          1                    1             tone l1k kao prvi funky ulazi
//		  - argv[4] - mode                   1                          1                    1 
//		  - argv[5] - input gain             0.501187 -> -6db           0.398107 -> -8db     default 
//		  - argv[6] - inverter degree        1.0                        0.3                  default
//		  - argv[7] - inverter gain          1.0                        0.7                  default
// @return - nothing
// Comment: main routine of a program
//
// E-mail:	<email>
//
/////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	FILE* wav_in = NULL;
	FILE* wav_out = NULL;
	char WavInputName[256];
	char WavOutputName[256];
	WAV_HEADER inputWAVhdr, outputWAVhdr;

	
	
	//enable
	int enable = 1;

	//default inv params
	double invDegree = 1.0;
	double invGain = 1.0;

	//args protection
	if(argc < 3){
		printf("Invalid number of arguments!");
		return 0;
	}

	if (argc >= 4){

		enable = atoi(argv[3]);
	}

	if (argc >= 5){

		mode = atoi(argv[4]);
	}

	if (argc >= 6){

		variablesGain[LEFT_CH] = atof(argv[5]);
		variablesGain[RIGHT_CH] = atof(argv[5]);
	}

	if (argc >= 7){

		invDegree = atof(argv[6]);
	}

	if (argc >= 8){

		invGain = atof(argv[7]);
	}

	// Init channel buffers
	for (int i = 0; i < MAX_NUM_CHANNEL; i++)
		memset(&sampleBuffer[i], 0, BLOCK_SIZE);

	// Open input and output wav files
	//-------------------------------------------------
	strcpy(WavInputName, argv[1]);
	wav_in = OpenWavFileForRead(WavInputName, "rb");
	strcpy(WavOutputName, argv[2]);
	wav_out = OpenWavFileForRead(WavOutputName, "wb");
	//-------------------------------------------------



	audio_invert_init(&inverter, invDegree, invGain);
	


	// Read input wav header
	//-------------------------------------------------
	ReadWavHeader(wav_in, inputWAVhdr);
	//-------------------------------------------------

	// Set up output WAV header
	//-------------------------------------------------	
	outputWAVhdr = inputWAVhdr;
	//enable for channels
	if(enable == 0)
	{
		outputWAVhdr.fmt.NumChannels = 2; // change number of channels
	} 
	else outputWAVhdr.fmt.NumChannels = OUTPUT_NUM_CHANNELS ;

	
	int oneChannelSubChunk2Size = inputWAVhdr.data.SubChunk2Size / inputWAVhdr.fmt.NumChannels;
	int oneChannelByteRate = inputWAVhdr.fmt.ByteRate / inputWAVhdr.fmt.NumChannels;
	int oneChannelBlockAlign = inputWAVhdr.fmt.BlockAlign / inputWAVhdr.fmt.NumChannels;

	outputWAVhdr.data.SubChunk2Size = oneChannelSubChunk2Size * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.ByteRate = oneChannelByteRate * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.BlockAlign = oneChannelBlockAlign * outputWAVhdr.fmt.NumChannels;


	// Write output WAV header to file
	//-------------------------------------------------
	WriteWavHeader(wav_out, outputWAVhdr);


	// Processing loop
	//-------------------------------------------------	
	{
		int sample;
		int BytesPerSample = inputWAVhdr.fmt.BitsPerSample / 8;
		const double SAMPLE_SCALE = -(double)(1 << 31);		//2^31
		int iNumSamples = inputWAVhdr.data.SubChunk2Size / (inputWAVhdr.fmt.NumChannels * inputWAVhdr.fmt.BitsPerSample / 8);

		// exact file length should be handled correctly...
		for (int i = 0; i < iNumSamples / BLOCK_SIZE; i++)
		{
			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				for (int k = 0; k < inputWAVhdr.fmt.NumChannels; k++)
				{
					sample = 0; //debug
					fread(&sample, BytesPerSample, 1, wav_in);
					sample = sample << (32 - inputWAVhdr.fmt.BitsPerSample); // force signextend
					sampleBuffer[k][j] = sample / SAMPLE_SCALE;				// scale sample to 1.0/-1.0 range		
				}
			}





			// enable 
			if (enable == 1) {
					gainProcessing(sampleBuffer, sampleBuffer);
			}




			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				for (int k = 0; k < outputWAVhdr.fmt.NumChannels; k++)
				{
					sample = sampleBuffer[k][j] * SAMPLE_SCALE;	// crude, non-rounding 			
					sample = sample >> (32 - inputWAVhdr.fmt.BitsPerSample);
					fwrite(&sample, outputWAVhdr.fmt.BitsPerSample / 8, 1, wav_out);
				}
			}
		}
	}



	// Close files
	//-------------------------------------------------	
	fclose(wav_in);
	fclose(wav_out);
	//-------------------------------------------------	

	return 0;
}

#include <stdio.h>
#include <dsplib\wavefile.h>
#include <stdfix.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <dsplib\timers.h>

// IO Buffers
__memY static DSPfract sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];

// Processing related variables
DSPfract variablesGain[INPUT_NUM_CHANNELS] = {MINUS_3DB, MINUS_3DB};

//Mode
DSPint mode = 0;

DSPint enable = 1;

DSPfract invDegree = FRACT_NUM(1.0);
DSPfract invGain = FRACT_NUM(1.0);

//Inverter
typedef struct {
	DSPfract degree;
	DSPfract gain;
} inverter_data_t;

//Global inverter
inverter_data_t inverter;

void audio_invert_init(inverter_data_t* data, DSPfract degree, DSPfract gain)
{
	data->degree = degree;
	//linear gain
	data->gain = gain;
}

static void
gst_audio_invert_transform(inverter_data_t* data,
	DSPfract* input, DSPfract* output)
{
	DSPint i;
	DSPfract dry = FRACT_NUM(1.0) - data->degree;
	DSPfract val;

	for (i = 0; i < BLOCK_SIZE; i++) {
		val = *input * dry - *input * data->degree;
		*output = val * data->gain;
		input++;
		output++;
	}
}
#ifdef GAIN_PROC_ASM
extern void gainProcessing(__memY DSPfract pIn[][BLOCK_SIZE], __memY DSPfract pOut[][BLOCK_SIZE]);
#else
void gainProcessing(__memY DSPfract pIn[][BLOCK_SIZE], __memY DSPfract pOut[][BLOCK_SIZE])
{
	DSPint i;
	//Model 3
	__memY DSPfract* samplePtrInL = *(pIn + LEFT_CH);
	__memY DSPfract* samplePtrOutL = *(pOut + LEFT_CH);

	__memY DSPfract* samplePtrInR = *(pIn + RIGHT_CH);
	__memY DSPfract* samplePtrOutR = *(pOut + RIGHT_CH);

	__memY DSPfract* samplePtrOutLs = *(pOut + LS_CH);
	__memY DSPfract* samplePtrOutRs = *(pOut + RS_CH);

	DSPfract variableGainL = *(variablesGain + LEFT_CH);
	DSPfract variableGainR = *(variablesGain + RIGHT_CH);

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
#endif

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
	WAVREAD_HANDLE *wav_in;
	    WAVWRITE_HANDLE *wav_out;

		char WavInputName[256];
		char WavOutputName[256];

	    DSPint inChannels;
	    DSPint outChannels;
	    DSPint bitsPerSample;
	    DSPint sampleRate;
	    DSPint iNumSamples;
	    DSPint i;
	    DSPint j;

	    unsigned long long count1, count2, spent_cycles;

	//enable
	enable = 1;
	mode = 1;



	// Init channel buffers
		for(i=0; i<MAX_NUM_CHANNEL; i++)
			for(j=0; j<BLOCK_SIZE; j++)
				sampleBuffer[i][j] = FRACT_NUM(0.0);

		// Open input wav file
			//-------------------------------------------------
			strcpy(WavInputName,argv[0]);
			wav_in = cl_wavread_open(WavInputName);
			 if(wav_in == NULL)
		    {
		        printf("Error: Could not open wavefile.\n");
		        return -1;
		    }
			//-------------------------------------------------



	audio_invert_init(&inverter, invDegree, invGain);



	// Read input wav header
		//-------------------------------------------------
		inChannels = cl_wavread_getnchannels(wav_in);
	    bitsPerSample = cl_wavread_bits_per_sample(wav_in);
	    sampleRate = cl_wavread_frame_rate(wav_in);
	    iNumSamples =  cl_wavread_number_of_frames(wav_in);
		//-------------------------------------------------

	// Set up output WAV header
	//-------------------------------------------------
	strcpy(WavOutputName,argv[1]);
	//enable for channels
	if(enable == 0)
	{
		outChannels = 2; // change number of channels
		wav_out = cl_wavwrite_open(WavOutputName, bitsPerSample, outChannels, sampleRate);
			if(!wav_out)
		    {
		        printf("Error: Could not open wavefile.\n");
		        return -1;
		    }
	}
	else {
		outChannels = OUTPUT_NUM_CHANNELS ;
		wav_out = cl_wavwrite_open(WavOutputName, bitsPerSample, outChannels, sampleRate);
			if(!wav_out)
		    {
		        printf("Error: Could not open wavefile.\n");
		        return -1;
		    }
	}

	//-------------------------------------------------

		// Processing loop
		//-------------------------------------------------
	    {
			int i;
			int j;
			int k;
			int c;
			int sample;

			// exact file length should be handled correctly...
			for(i=0; i<iNumSamples/BLOCK_SIZE; i++)
			{
				for(j=0; j<BLOCK_SIZE; j++)
				{
					for(k=0; k<inChannels; k++)
					{
						sample = cl_wavread_recvsample(wav_in);
	        			sampleBuffer[k][j] = rbits(sample);
					}
				}


				//////////////////////////////////////////////////////////
				// pozvati processing funkciju ovde
				if(enable == 1){
					count1 = cl_get_cycle_count();
					gainProcessing(sampleBuffer, sampleBuffer);
					count2 = cl_get_cycle_count();
				}
				spent_cycles = count2 - count1;
				printf("%llu\n", spent_cycles);
				////////////////////////////////////////////////////////////
				for(j=0; j<BLOCK_SIZE; j++)
				{
					for(k=0; k<outChannels; k++)
					{
						sample = bitsr(sampleBuffer[k][j]);
						cl_wavwrite_sendsample(wav_out, sample);
					}
				}
			}
		}

		// Close files
		//-------------------------------------------------
	    cl_wavread_close(wav_in);
	    cl_wavwrite_close(wav_out);
		//-------------------------------------------------

	    return 0;
}

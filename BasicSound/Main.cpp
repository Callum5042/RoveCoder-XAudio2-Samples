#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include <Windows.h>
#include <xaudio2.h>

const UINT32 sampleRate = 44100;
const float frequency = 440.0f; // 440 Hz

static void GenerateSquareWave(float* buffer, UINT32 numSamples)
{
	for (UINT32 i = 0; i < numSamples; ++i)
	{
		float t = static_cast<float>(i) / sampleRate;
		buffer[i] = (sin(2 * 3.14159f * frequency * t) >= 0) ? 1.0f : -1.0f;
	}
}

static void GenerateSawtoothWave(float* buffer, UINT32 numSamples)
{
	for (UINT32 i = 0; i < numSamples; ++i)
	{
		float t = static_cast<float>(i) / sampleRate;
		buffer[i] = 2.0f * (t * frequency - floor(0.5f + t * frequency));
	}
}

static void GenerateTriangleWave(float* buffer, UINT32 numSamples)
{
	for (UINT32 i = 0; i < numSamples; ++i)
	{
		float t = static_cast<float>(i) / sampleRate;
		buffer[i] = 2.0f * fabs(2 * (t - floor(t + 0.5f))) - 1.0f;
	}
}

static void GenerateSineWave(float* buffer, UINT32 numSamples)
{
	for (UINT32 i = 0; i < numSamples; ++i)
	{
		float t = static_cast<float>(i) / sampleRate;
		buffer[i] = sin(2 * 3.14159f * frequency * t);
	}
}

static void GenerateWhiteNoise(float* buffer, UINT32 numSamples)
{
	// Seed the random number generator
	srand(static_cast<unsigned int>(time(NULL)));

	for (UINT32 i = 0; i < numSamples; ++i)
	{
		// Generate a random number between -1 and 1
		buffer[i] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
	}
}

static void PlayBasicSound(IXAudio2* audio)
{
	std::vector<float> audio_buffer;
	audio_buffer.resize(sampleRate);
	GenerateWhiteNoise(audio_buffer.data(), sampleRate);

	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = sampleRate;
	waveFormat.wBitsPerSample = sizeof(float) * 8;
	waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

	// Create audio source buffer
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = sampleRate * sizeof(float);
	buffer.pAudioData = reinterpret_cast<BYTE*>(audio_buffer.data());

	// Create source voice
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	audio->CreateSourceVoice(&pSourceVoice, reinterpret_cast<WAVEFORMATEX*>(&waveFormat));
	pSourceVoice->SubmitSourceBuffer(&buffer);
	pSourceVoice->Start();

	// Loop until sound is finished
	while (true)
	{
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			break;
		}

		Sleep(100);
	}
}

int main(int args, char** argv)
{
	// Initialize COM
	if (CoInitializeEx(nullptr, COINIT_MULTITHREADED) != S_OK)
	{
		std::cerr << "CoInitializeEx failed\n";
		return -1;
	}

	// Initialize XAudio2 - XAudio2 can also be wrapped in a ComPtr
	IXAudio2* xaudio2 = nullptr;
	if (XAudio2Create(&xaudio2) != S_OK)
	{
		std::cerr << "XAudio2Create failed\n";
		CoUninitialize();
		return -1;
	}

	// Create mastering voice
	IXAudio2MasteringVoice* master_voice = nullptr;
	if (xaudio2->CreateMasteringVoice(&master_voice) != S_OK)
	{
		std::cerr << "CreateMasteringVoice failed\n";
		CoUninitialize();
		return -1;
	}

	std::cout << "XAudio2 successfully initialized\n";
	PlayBasicSound(xaudio2);

	// Cleanup
	master_voice->DestroyVoice();
	xaudio2->Release();
	CoUninitialize();

	return 0;
}
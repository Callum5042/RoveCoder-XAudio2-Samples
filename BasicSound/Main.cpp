#include <iostream>
#include <vector>
#include <cmath>

#include <Windows.h>
#include <xaudio2.h>

static std::vector<float> GenerateSineWave(UINT32 samples)
{
	// 440 Hz
	const float frequency = 440.0f; 

	std::vector<float> buffer(samples);
	for (UINT32 i = 0; i < samples; ++i)
	{
		float t = static_cast<float>(i) / samples;
		buffer[i] = sin(2 * 3.14159f * frequency * t);
	}

	return buffer;
}

static void PlayBasicSound(IXAudio2* audio)
{
	const UINT32 sample_rate = 44100;

	// Create source voice
	WAVEFORMATEX wave_format = {};
	wave_format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	wave_format.nChannels = 2;
	wave_format.nSamplesPerSec = sample_rate;
	wave_format.wBitsPerSample = sizeof(float) * 8;
	wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

	IXAudio2SourceVoice* source_voice = nullptr;
	audio->CreateSourceVoice(&source_voice, reinterpret_cast<WAVEFORMATEX*>(&wave_format));

	// Create audio source buffer
	std::vector<float> audio_buffer = GenerateSineWave(sample_rate);

	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = sample_rate * sizeof(float);
	buffer.pAudioData = reinterpret_cast<BYTE*>(audio_buffer.data());
	
	source_voice->SubmitSourceBuffer(&buffer);
	source_voice->Start();

	// Loop until sound is finished
	while (true)
	{
		XAUDIO2_VOICE_STATE state;
		source_voice->GetState(&state);
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
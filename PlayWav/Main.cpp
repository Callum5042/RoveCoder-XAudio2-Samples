#include <iostream>
#include <xaudio2.h>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>

enum class FourCC
{
	UNKNOWN = 0,
	RIFF = 'FFIR',
	FMT = ' tmf',
	DATA = 'atad',
	WAVE = 'EVAW',
};

void WaveScanType(std::ifstream& file)
{
	file.seekg(0, std::ios::beg);

	while (true)
	{
		FourCC chunk_type;
		file.read(reinterpret_cast<char*>(&chunk_type), sizeof(FourCC));

		if (chunk_type == FourCC::RIFF)
		{
			int chunk_size = 0;
			file.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

			FourCC file_type;
			file.read(reinterpret_cast<char*>(&file_type), sizeof(FourCC));

			if (file_type != FourCC::WAVE)
			{
				std::cerr << "Not a WAVE file\n";
				return;
			}

			return;
		}
	}
}

WAVEFORMATEXTENSIBLE WaveScanWFX(std::ifstream& file)
{
	file.seekg(0, std::ios::beg);

	while (true)
	{
		FourCC chunk_type;
		file.read(reinterpret_cast<char*>(&chunk_type), sizeof(FourCC));

		if (chunk_type == FourCC::FMT)
		{
			int chunk_size = 0;
			file.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

			WAVEFORMATEXTENSIBLE wfx;
			file.read(reinterpret_cast<char*>(&wfx), sizeof(WAVEFORMATEXTENSIBLE));

			return wfx;
		}
	}
}

std::vector<BYTE> WaveScanBuffer(std::ifstream& file)
{
	file.seekg(0, std::ios::beg);

	while (true)
	{
		FourCC chunk_type;
		file.read(reinterpret_cast<char*>(&chunk_type), sizeof(FourCC));

		if (chunk_type == FourCC::DATA)
		{
			int chunk_size = 0;
			file.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

			std::vector<BYTE> buffer;
			buffer.resize(chunk_size);

			file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			return buffer;
		}
	}
}

static HRESULT PlayWav(IXAudio2* audio, LPCWSTR fileName)
{
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	file.seekg(0, std::ios::beg);

	WaveScanType(file);
	WAVEFORMATEXTENSIBLE wfx1 = WaveScanWFX(file);
	std::vector<BYTE> buffer23 = WaveScanBuffer(file);

	// Create audio source buffer
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.AudioBytes = buffer23.size();  //size of the audio buffer in bytes
	buffer.pAudioData = buffer23.data();  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// Create source voice
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	audio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx1);
	pSourceVoice->SubmitSourceBuffer(&buffer);
	pSourceVoice->Start(0);

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

	return S_OK;
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
	PlayWav(xaudio2, L"C:/Users/Callum/Downloads/gun_44mag_11.wav");

	// Cleanup
	master_voice->DestroyVoice();
	xaudio2->Release();
	CoUninitialize();

	return 0;
}
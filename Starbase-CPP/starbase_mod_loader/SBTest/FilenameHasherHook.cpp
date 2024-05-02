#include "FilenameHasherHook.h"

#include "../Managers/Hook.h"
#include "../Utils/Debug.h"
#include "../Utils/PointerUtils.h"
#include "../Utils/Assertion.h"
#include <emmintrin.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define LOG(msg) Debug::Msg("[FilenameHasherHook] " msg)


#define DECL_HOOK(ret, name, ...) \
	typedef ret (*name##_fn_t)(__VA_ARGS__); \
	static name##_fn_t name##_fn; \
	ret name (__VA_ARGS__)

#define HOOK_SIG_METHOD(method, sig) do { \
		method##_fn = (method##_fn_t)PointerUtils::FindPattern(Module, sig); \
		if (method##_fn == nullptr) { \
			LOG("Failed to find " #method "_fn"); \
			return; \
		} \
		Hook::Attach(&(LPVOID&)method##_fn, method); \
		printf("Hooked " #method "_fn at 0x%08p\n", (uintptr_t)method##_fn); \
	} while (0)


template <typename T>
struct fb_array
{
	union
	{
		T* buffer;
		int flags : 3;
	};
	uint32_t _8;
	uint32_t _12;
	bool _16_bool;

	inline T* GetBuffer() { return (T*)(((uintptr_t)this->buffer) & ~3); }
};
typedef fb_array<char> fb_string;

template <typename T>
struct fb_szarray
{
	T* data;
	uint32_t length;
};
typedef fb_szarray<char> fb_szstring;

struct fb_error {
	uint32_t error;
	fb_string message;
};

typedef uintptr_t (*filenameHasher_fn_t)(const char* input, int len, uint32_t seed);
static filenameHasher_fn_t filenameHasher_fn;
static std::ofstream hashfile;

typedef FILE* (*fsopen_fn_t)(const char* filename, const char* mode, int flags);
static fsopen_fn_t fsopen_fn;

typedef void (*OpenGameFile_fn_t)(HANDLE* handleref, fb_szstring* filename, void* error_out);
static OpenGameFile_fn_t OpenGameFile_fn;

typedef HANDLE(*CreateFileW_fn_t)(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile);
static CreateFileW_fn_t CreateFileW_fn;

typedef void* (*LoadAndDecompressResFile_fn_t)(void* param_1, void* data_out, fb_szstring* filename, fb_error* error_out, bool param_5);
static LoadAndDecompressResFile_fn_t LoadAndDecompressResFile_fn;

typedef uint32_t (*unk_141776a30_fn_t)(uint32_t param_1, void* param_2, void* param_3, void* param_4, uint32_t param_5);
static unk_141776a30_fn_t unk_141776a30_fn;

//typedef void (*DecryptData_fn_t)(void* param_1, void* param_2, void* param_3);
//static DecryptData_fn_t DecryptData_fn;
typedef void (*DecryptData_fn_t)(uint64_t param_1, void* data, uint32_t size);
static DecryptData_fn_t DecryptData_fn;


static /*__declspec(thread)*/ int32_t padding = 0;

namespace SBTest
{
	uint32_t FilenameHasher_Hook(const char* input, int len, uint32_t seed)
	{
		// std::string log = "Game is requesting hash for \"" + (std::string)input + "\"";
		// Debug::Msg(log.c_str());

		uint32_t ret = filenameHasher_fn(input, len, seed);
		hashfile << std::hex << ret << "\t\"" << input << "\"" << std::endl;

		return ret;
	}

	FILE* fsopen_hook(const char* filename, const char* mode, uint32_t flags)
	{
		std::string log = "Game is opening file \"" + (std::string)filename + "\"";
		Debug::Msg(log.c_str());

		// uint32_t ret = filenameHasher_fn(input, len, seed);
		// hashfile << std::hex << ret << "\t\"" << input << "\"" << std::endl;

		//__debugbreak();

		return fsopen_fn(filename, mode, flags);
	}

	fb_string* FUN_1411acf70(fb_string* str)
	{
		bool* pbVar1 = &str->_16_bool;
		str->buffer = (char*)((uintptr_t)pbVar1 | 1);
		str->_8 = 0;
		str->_12 = 512;

		// TODO asserts

		return str;
	}

	uint32_t FUN_1413d7c70(uint32_t value)
	{
		uint32_t unk_0 = 0;
		if (value > INT_MAX)
			unk_0 = INT_MIN;

		uint32_t ret = value;
		if (value < 4)
			ret = 4;

		ret += (ret >> 1) - 1;
		ret |= ret >> 1;
		ret |= ret >> 2;
		ret |= ret >> 4;
		ret |= ret >> 8;
		ret |= ret >> 16;
		ret += 1;

		if (value > INT_MAX)
			ret = unk_0;

		if (ret < value)
		{
			// THROW 0x29
		}

		return ret;
	}

	void OpenGameFile_hook(HANDLE* handleref, fb_szstring* filename, void* error_out)
	{
		//std::string log = std::string(padding * 2, ' ') + "Game called OpenGameFile with file \"" + (std::string)filename->data + "\"";
		//Debug::Msg(log.c_str());

		// if (strcmp(filename->data, "fdb.bin") == 0)
		//	__debugbreak();

		padding++;

		if (*handleref != INVALID_HANDLE_VALUE)
		{
			CloseHandle(*handleref);
			*handleref = INVALID_HANDLE_VALUE;
		}
		fb_string str;
		FUN_1411acf70(&str);

		// log = std::string(padding * 2, ' ') + "  str._8 vs str._12: " + std::to_string(str._8) + " , " + std::to_string(str._12);
		// Debug::Msg(log.c_str());
		if (str._8 == str._12)
		{
			uint32_t unk_0 = FUN_1413d7c70(str._12 + 1);

			std::string log = std::string(padding * 2, ' ') + "  FUN_1413d7c70(" + std::to_string(str._12 + 1) + "): " + std::to_string(unk_0);
			Debug::Msg(log.c_str());

			/*
			if (str._8 < unk_0)
			{
				FUN_1413d8370(&str, 2, unk_0, NULL);
			}
			*/
		}

		// uint32_t ret = filenameHasher_fn(input, len, seed);
		// hashfile << std::hex << ret << "\t\"" << input << "\"" << std::endl;

		//__debugbreak();

		OpenGameFile_fn(handleref, filename, error_out);

		padding--;
		//Debug::Msg("padding--");
		return;
	}

	char* convertWStringToCharPtr(const wchar_t* input)
	{
		int inputLength = wcslen(input);
		int outputSize = inputLength + 1;
		char* outputString = new char[outputSize];
		size_t charsConverted = 0;
		wcstombs_s(&charsConverted, outputString, outputSize, input, inputLength);
		return outputString;
	}

	inline bool ends_with(std::string const& value, std::string const& ending)
	{
		if (ending.size() > value.size()) return false;
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}

	HANDLE CreateFileW_hook(
		LPCWSTR               lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
	)
	{
		char* nameUtf8 = convertWStringToCharPtr(lpFileName);
		std::string nameUtf8Str = nameUtf8;
		delete[] nameUtf8;

		if (
			!nameUtf8Str._Starts_with("C:\\Windows\\") &&
			!nameUtf8Str._Starts_with("\\\\?\\C:\\Users\\hugof\\AppData\\Roaming\\Starbase\\cache\\76561198000822649\\filedb") &&
			!ends_with(nameUtf8Str, ".pdb") &&
			!ends_with(nameUtf8Str, ".dll"))
		{
			std::cout << std::string(padding * 2, ' ') + "CreateFileW " << nameUtf8Str << std::endl;
#if 0
			std::string log = std::string(padding * 2, ' ') + "Game called CreateFileW with file \"" + nameUtf8Str + "\"";
			Debug::Msg(log.c_str());
#endif

			padding++;

#if 0
			if (nameUtf8Str._Starts_with(R"(\\?\D:\SteamLibrary\steamapps\common\Starbase\files\)"))
				__debugbreak();
#endif
			if (ends_with(nameUtf8Str, "\\1.fbe") || ends_with(nameUtf8Str, "\\dev_settings.bin") || ends_with(nameUtf8Str, "76561198000822649.bin"))
				__debugbreak();

			HANDLE ret = CreateFileW_fn(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
			//Debug::Msg("padding--");
			padding--;
			return ret;
		}
		return CreateFileW_fn(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	struct datablockheader // size: 12
	{
		uint32_t _0;
		uint64_t _4;
	};

#define BIG_CONSTANT(x) (x##LLU)
	inline uint64_t fmix(uint64_t k)
	{
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xff51afd7ed558ccd);
		k ^= k >> 33;
		k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
		k ^= k >> 33;

		return k;
	}

	// TODO make sure it's correct and rewrite something human-readable
	// Seems to return the correct value
	struct struct_0x30 { char* data[6]; };
	uint32_t FindOffsetInBuffer(uint32_t hash, fb_szstring* filename, uint32_t* buffer, void* bufferEnd, uint32_t bufferSize)
	{
		if (buffer == NULL)
			return bufferSize;

		uint32_t maxSize = bufferSize - 1;
		uint32_t currentPos = 0;
		uint32_t uVar5 = hash & maxSize;
		uint32_t targetOffset = uVar5;
		uint32_t currentValueInt = buffer[targetOffset];

		if (currentValueInt == 0)
			return bufferSize;

		while (currentValueInt != hash || ((struct_0x30*)bufferEnd)[targetOffset].data[0] != filename->data)
		{
			currentPos++;
			uVar5 = maxSize & (uVar5 + 1);
			targetOffset = uVar5;
			//printf("processing offset %i\n", targetOffset);
			currentValueInt = buffer[targetOffset];
			if (currentValueInt == 0 || currentPos > (maxSize & (uVar5 - currentValueInt)))
				return bufferSize;
		}

		return uVar5;
	}

	struct LoadAndDecompressResFile_param1
	{
		void* unk_0;
		void* unk_8;
		fb_array<uint32_t> dbfiledata;
	};
	void* LoadAndDecompressResFile_hook(LoadAndDecompressResFile_param1* param_1, void** data_out, fb_szstring* filename, fb_error* error_out, bool param_5)
	{
		std::cout << std::string(padding * 2, ' ') + "LoadAndDecompressResFile " + (std::string)filename->data << std::endl;
#if 0
		//__debugbreak();
		std::string log = std::string(padding * 2, ' ') + "LoadAndDecompressResFile \"" + (std::string)filename->data + "\"";
		Debug::Msg(log.c_str());

		//__debugbreak();


		// if (strcmp(filename->data, "fdb.bin") != 0)
		// 	__debugbreak();

		if (param_5)
		{
			data_out[0] = NULL;
			data_out[1] = NULL;

			// DEBUG
			std::string log = "param_5 was not 0 for " + (std::string)filename->data;
			Debug::Msg(log.c_str());

			return data_out;
		}

		uint32_t dbfile_size = param_1->dbfiledata._12;
		uint32_t* dbfile_buffer = param_1->dbfiledata.GetBuffer();
		uint32_t filenamehashComputed = (uint32_t)fmix((uint64_t)filename->data); // weird to truncate it but uh ok
		uint32_t filenamehash = 1;
		if (filenamehashComputed != 0)
			filenamehash = filenamehashComputed;

		/*std::cout << "First bytes of buffer (full size: " << dbfile_size << ") :" <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[0] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[1] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[2] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[3] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[4] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[5] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[6] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[7] <<
			" " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)((byte*)dbfile_buffer)[8] <<
			std::endl;*/

#if 0
		static bool alreadydumped = false;

		if (!alreadydumped)
		{
			alreadydumped = true;

			std::cout << "Dumping game data buffer to game_data_buffer.bin" << std::endl;
			std::ofstream outfile("game_data_buffer.bin");
			outfile.write((char*)dbfile_buffer, dbfile_size);
			outfile.close();
		}
#endif

		uint64_t fileInfoOffset = FindOffsetInBuffer(filenamehash, filename, dbfile_buffer, &dbfile_buffer[dbfile_size], dbfile_size);
		std::cout << "Filename hash: " << std::hex << filenamehash << ", hash used: " << std::hex << (filenamehash & (dbfile_size - 1)) << std::endl;
#if 0
		log = "FindOffsetInBuffer returned " + std::to_string(fileInfoOffset);
		Debug::Msg(log.c_str());
#endif
		// Debug
		if (fileInfoOffset == dbfile_size)
		{
			std::string log = "File " + (std::string)filename->data + " not found";
			Debug::Msg(log.c_str());

			uint32_t fileInfoOffsetOriginal = unk_141776a30_fn(filenamehash, filename, dbfile_buffer, &dbfile_buffer[dbfile_size], dbfile_size);
			log = "  Original returned " + std::to_string(fileInfoOffsetOriginal);
			Debug::Msg(log.c_str());

		}
#if 0
		// Original
		if (fileInfoOffset == dbfile_size)
		{
			error_out->error = 1;
			// error_out->message = "File " + filename + " not found";
			data_out[0] = NULL;
			data_out[1] = NULL;
			return data_out;
		}
#endif

		// ASSERT (fileInfoOffset < dbfile_size, 0xD4);

		HANDLE ressourceHandle = INVALID_HANDLE_VALUE;

		/*
			uVar17 = *(ulonglong *)(param_1 + 0x10) & 0xfffffffffffffffc;                   // QWORD uVar17 = (param_1 + 0x10) -> GetBuffer()
			lVar7 = (ulonglong)*(uint *)(param_1 + 0x1c) + (uVar13 & 0xffffffff) * 0xc + 2; // DWORD lVar7  = ((param_1 + 0x10) -> _12) + (DWORD)uVar13 * 0xc + 2 // "(DWORD)uVar13 * 0xc + 2" is "{array_size_0xc} [(DWORD)uVar13] -> _2"
			puVar1 = (ulonglong *)(uVar17 + lVar7 * 4);                                     // QWORD puVar1 = uVar17 [lVar7]
			iVar3 = *(int *)(uVar17 + lVar7 * 4 + 8);                                       // QWORD iVar3  = uVar17 [lVar7] -> _8
			...
			local_78 = (char *)(*puVar1 & 0xfffffffffffffffc);                              // puVar1 -> GetBuffer
		*/
#if 0
		uint32_t* buffer = param_1->dbfiledata.GetBuffer();
		uint32_t dataoffset = param_1->dbfiledata._12[(uint32_t)fileInfoOffset]->_4;
		fb_string* bufferData = (fb_string*)buffer[dataoffset];
		int bufferData_8 = buffer[dataoffset]->_8;
		LARGE_INTEGER size = { bufferData_8, 0 };
		char* resourceFilename = bufferData->GetBuffer();

		char lastChar;
		if (resourceFilename == NULL)
		{
			// TODO
			// lastChar = (&DAT_14a640bcc)[bufferData_8]
			// resourceFilename = &DAT_14a640bcc;
		}
		else
			lastChar = resourceFilename[bufferData_8];

		// ASSERT (lastChar == '\0', 0x14);

		fb_szstring resourceFilenameSZString = { resourceFilename, bufferData_8 };

		// DEBUG
		std::string log = "Target file: " + (std::string)resourceFilenameSZString.data;
		Debug::Msg(log.c_str());
		//#if 0
		if (!OpenGameFile(&ressourceHandle, &resourceFilenameSZString, error_out))
		{
			data_out[0] = NULL;
			data_out[1] = NULL;
			if (ressourceHandle != INVALID_HANDLE_VALUE)
				CloseHandle(ressourceHandle);
			return data_out;
		}

		if (bufferData->_12 == -1 && ressourceHandle != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER fileSize;
			if (!GetFileSizeEx(ressourceHandle, &fileSize) || fileSize == 0 || (*(uint*)(puVar1 + 2) == 0))
			{
				error_out->error = 2;
				// error_out->message = "File " + filename + " is empty";
				data_out[0] = NULL;
				data_out[1] = NULL;
				if (ressourceHandle != INVALID_HANDLE_VALUE)
					CloseHandle(ressourceHandle);
				return data_out;
			}
		}

		// TODO
#endif
#endif
		padding++;

		void* ret = LoadAndDecompressResFile_fn(param_1, data_out, filename, error_out, param_5);
		//__debugbreak();
		padding--;
		//Debug::Msg("padding--");
		return ret;
	}



	void* unk_141776a30_hook(uint32_t param_1, void* param_2, void* param_3, void* param_4, uint32_t param_5)
	{
#if 0
		void* ret = unk_141776a30_fn(param_1, param_2, param_3, param_4, param_5);
		if (padding > 0)
		{
			std::string log = std::string(padding * 2, ' ') + "unk_141776a30_fn returns: " + std::to_string((uintptr_t)ret);
			Debug::Msg(log.c_str());

			uint64_t rewriteResult = FindOffsetInBuffer(param_1, (fb_szstring*)param_2, (uint32_t*)param_3, param_4, param_5);

			log = std::string(padding * 2, ' ') + "unk_141776a30_rewrite returns: " + std::to_string((uintptr_t)rewriteResult);
			Debug::Msg(log.c_str());
		}
		return ret;
#else
		return (void*)FindOffsetInBuffer(param_1, (fb_szstring*)param_2, (uint32_t*)param_3, param_4, param_5);
#endif
	}

	std::string convert_int(uintptr_t n)
	{
		std::stringstream ss;
		ss << std::hex << n;
		return ss.str();
	}

	#define debugvalues(fullname) \
			std::cout << "  " #fullname ": "; \
			for (int iValue = 0; iValue < 4; ++iValue) \
			{ \
				for (int iByte = 0; iByte < 4; ++iByte) \
					std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)fullname##.m128i_u8[4 * iValue + iByte] << " "; \
			} \
			std::cout << std::endl;
	#undef debugvalues


	#define ROTL(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
	#define QR(a, b, c, d) (			\
		a += b,  d ^= a,  d = ROTL(d,16),	\
		c += d,  b ^= c,  b = ROTL(b,12),	\
		a += b,  d ^= a,  d = ROTL(d, 8),	\
		c += d,  b ^= c,  b = ROTL(b, 7))
	#define ROUNDS 8

	void chacha8_block(uint32_t out[16], uint32_t const in[16])
	{
		int i;
		uint32_t x[16];

		for (i = 0; i < 16; ++i)
			x[i] = in[i];
		// 10 loops × 2 rounds/loop = 20 rounds
		for (i = 0; i < ROUNDS; i += 2) {
			// Odd round
			QR(x[0], x[4], x[ 8], x[12]); // column 0
			QR(x[1], x[5], x[ 9], x[13]); // column 1
			QR(x[2], x[6], x[10], x[14]); // column 2
			QR(x[3], x[7], x[11], x[15]); // column 3
			// Even round
			QR(x[0], x[5], x[10], x[15]); // diagonal 1 (main diagonal)
			QR(x[1], x[6], x[11], x[12]); // diagonal 2
			QR(x[2], x[7], x[ 8], x[13]); // diagonal 3
			QR(x[3], x[4], x[ 9], x[14]); // diagonal 4
		}
		for (i = 0; i < 16; ++i)
			out[i] = x[i] + in[i];
	}


	void DecryptData(__m64 nonce, byte* data, uint32_t length)
	{
		if (length <= 0)
			return;

		std::cout << std::string(padding * 2, ' ') << "Decrypting data of length " << length << " with nonce: " << std::hex << nonce.m64_u64 << std::endl;

		uint32_t remainingBytesCount = length;

		uint32_t iIter = 0;

		do
		{
			uint32_t initialstate[] =
			{
				0xAA00B54B      , 0xDC7A80D4      , 0x46ECCABE      , 0xCF6BFF50,
				0x56B03217      , 0x12308085      , 0xC556B032      , 0x9CDDC7E1,
				0x20F83EC5      , 0xCEB1AC2D      , 0xFF5DF51C      , 0xEF8980D9,
				iIter           , 0               , nonce.m64_u32[0], nonce.m64_u32[1]
			};

			uint32_t block[16];

			chacha8_block(block, initialstate);

			++iIter;

			byte* blockBytes = (byte*)block;

			int remainingBytesCurrentIter = remainingBytesCount;
			if (remainingBytesCurrentIter > 64)
				remainingBytesCurrentIter = 64;

			for (int i = remainingBytesCurrentIter; i > 0; --i)
				*data++ ^= *blockBytes++;

			remainingBytesCount -= remainingBytesCurrentIter;
		}
		while (remainingBytesCount > 0);
	}


	void DecryptData_hook(uint64_t param_1, void* data, uint32_t bytecount)
	{
		DecryptData(__m64 {param_1}, (byte*)data, bytecount);

#if 0 // Debug
		// std::string log = "DecryptData called with param_1 = " + convert_int(param_1) + ", data = " + convert_int((uintptr_t)data) + ", compressedLength = " + convert_int(compressedLength);
		// Debug::Msg(log.c_str());

		// __debugbreak();

		// DecryptData_fn(param_1, data, compressedLength);

		byte* data0 = new byte[bytecount];
		byte* data1 = new byte[bytecount];
		memcpy(data0, data, bytecount);
		memcpy(data1, data, bytecount);

		if (param_1 == 18278098508171420331UL)
		{
			std::cout << "Saving data/script/init.lub.encrypted" << std::endl;
			std::ofstream outfile("data_script_init.lub.encrypted");
			outfile.write((char*)data1, bytecount);
			outfile.close();
		}

		DecryptData_fn(param_1, data0, bytecount);
		DecryptData(__m64 {param_1}, data1, bytecount);

		for (int i = 0; i < bytecount; i += 64)
		{
			for (int j = 0; j < 64 && i+j < bytecount; ++j)
			{
				if (data0[i+j] != data1[i+j])
				{
					printf("Mismatching decrypted data at offset %p (block %p). block data (original, custom):\n", i + j, i / 64);
					for (int k = 0; k < 64 && k < bytecount; ++k)
						std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data0[i+k] << " ";
					std::cout << std::endl;
					for (int k = 0; k < 64 && k < bytecount; ++k)
						std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data1[i+k] << " ";
					std::cout << std::endl;

					__debugbreak();

					break;
				}
			}
		}

		if (param_1 == 18278098508171420331UL)
		{
			std::cout << "Saving data/script/init.lub" << std::endl;
			std::ofstream outfile("data_script_init.lub");
			outfile.write((char*)data1, bytecount);
			outfile.close();
		}

		memcpy(data, data1, bytecount);

		// DecryptData (param_1, data, bytecount);
#endif
	}

#define BYTE0(v) ((unsigned __int8)(v >> 0))
#define BYTE1(v) ((unsigned __int8)(v >> 8))

	// typedef __int16 (*DecryptCacheFile_fn_t)(const __m128i* a1, __int64 a2, __m128i* a3, uintptr_t a4);
	// static DecryptCacheFile_fn_t DecryptCacheFile_fn;
	// 
	// __int16 DecryptCacheFile(const __m128i* a1, __int64 a2, __m128i* a3, uintptr_t a4)
	DECL_HOOK (__int16, DecryptCacheFile, const __m128i* a1, __int64 a2, __m128i* a3, uintptr_t a4)
	{
		static int callCount = 0;
		printf("[%04d] DecryptCacheFile: data = 0x%08p, length = %lli, keyptr = 0x%08p, rsbox = 0x%llx\n", callCount++, a1, a2, a3, a4);
		return DecryptCacheFile_fn(a1, a2, a3, a4);

#if 0
		__m128i block = _mm_load_si128(a3);


		do
		{
			__m128i a1_clone = _mm_xor_si128(block, _mm_loadu_si128(a1));

			__m128i processedBlock[4] = { 0 };


			for (int i = 0; i < 4; ++i)
			{
				int v7 = (_mm_cvtsi128_si32(a1_clone) << 0) & 0xF0F0F0F0;
				int v8 = (_mm_cvtsi128_si32(a1_clone) << 4) & 0xF0F0F0F0;

				for (int j = 0; j < 4; ++j)
					processedBlock[j] = _mm_xor_si128(processedBlock[j], *(__m128i*)((char*)&a3[0x42 + (i * 0x10)] + ((byte*)&v7)[j]));
				for (int j = 0; j < 4; ++j)
					processedBlock[j] = _mm_xor_si128(processedBlock[j], *(__m128i*)((char*)&a3[0x02 + (i * 0x10)] + ((byte*)&v8)[j]));

				a1_clone = _mm_srli_si128(a1_clone, 4);
			}

			// pslldq: Shift Double Quadword Left Logical
			// psrldq: Shift Double Quadword Right Logical
			// pxor: Logical Exclusive OR

			// our pslldq calls are "Shift xmm1 left by imm8 bytes while shifting in 0s."
			// our psrldq calls are "Shift xmm1 right by imm8 bytes while shifting in 0s."

			// pack 4 bytes 0-1-2-3?
			__m128i slixor0 = processedBlock[2];
			__m128i slixor1 = _mm_xor_si128(processedBlock[3], _mm_slli_si128(slixor0, 1)); // pssldq pxor
			__m128i slixor2 = _mm_xor_si128(processedBlock[0], _mm_slli_si128(slixor1, 1));
			__m128i slixor3 = _mm_xor_si128(processedBlock[1], _mm_slli_si128(slixor2, 1));

			v48 = 0;
			v48 = *(unsigned __int16*)(a4 + 2 * _mm_srli_si128(slixor0, 15).m128i_u64[0]) ^ v48; // psrldq
			v48 = v48 << 8;
			LOWORD(v48) = *(_WORD*)   (a4 + 2 * _mm_srli_si128(slixor1, 15).m128i_u64[0]) ^ v48; // psrldq
			v48 = v48 << 8;
			LOWORD(v48) = *(_WORD*)   (a4 + 2 * _mm_srli_si128(slixor2, 15).m128i_u64[0]) ^ v48; // psrldq


			block = _mm_xor_si128(_mm_cvtsi32_si128(v48), slixor3);


			a1++;
			a2--;
		} while (a2 != 0);

		*a3 = block;

#endif
	}



	void OnExit()
	{
		hashfile.close();
	}


	void DecryptGameFile(const char* path, const char* targetPath, uint32_t size)
	{
		std::cout << "Decrypting " << path << " to " << targetPath << std::endl;
		std::ifstream infile(path, std::ifstream::binary);

		byte* buffer = new byte[size];
		if (!infile.read((char*)buffer, 12))
		{
			std::cout << "  FAILED TO READ 8" << std::endl;
			return;
		}
		std::cout << "  read count: " << std::to_string(infile.tellg()) << std::endl;
		ZeroMemory(buffer, size);
		if (!infile.read((char*)buffer, size))
		{
			std::cout << "  FAILED TO READ " << size << std::endl;
			return;
		}
		std::cout << "  read count: " << std::to_string(infile.tellg()) << std::endl;

		//std::cout << "  first byte: " << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)buffer[0] << " " << (uint32_t)buffer[1] << ")" << std::endl;

		byte* bufferChacha = new byte[size];
		memcpy(bufferChacha, buffer, size);

		DecryptData(__m64 {18135472946200456474UL}, buffer, size);

		std::ofstream outfile(targetPath);
		outfile.write((char*)buffer, size);
		outfile.close();
		infile.close();
		delete[] buffer;
		delete[] bufferChacha;

	}

	DECL_HOOK(void, GCM_Base_SetKeyWithoutResync, uintptr_t _this, const byte* userKey, size_t keylength, const uintptr_t /* CryptoPP::NameValuePairs& */ params)
	{
		static int callCount = 0;
		printf("[%04d] GCM_Base_SetKeyWithoutResync: userKey = 0x%08p, keylength = %lli\nKey: ", callCount++, userKey, keylength);
		for (int i = 0; i < keylength; ++i)
			std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)userKey[i] << " ";
		std::cout << std::endl;

		/*
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV
		
		CryptoPP::NameValuePairs params2;
		params2.

		d.SetKey(userKey, keylength, params);
		*/

		GCM_Base_SetKeyWithoutResync_fn(_this, userKey, keylength, params);
	}

	DECL_HOOK(void, SimpleKeyingInterface_SetKeyWithIV, uintptr_t _this, const byte* key, size_t length, const byte* iv, size_t ivLength)
	{
		static int callCount = 0;
		printf("[%04d] SimpleKeyingInterface_SetKeyWithIV: key = 0x%08p, length = %lli, iv = 0x%08p, ivLength = %lli\nKey: ", callCount++, key, length, iv, ivLength);
		for (int i = 0; i < length; ++i)
			std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)key[i] << " ";
		std::cout << std::endl << "IV:";
		for (int i = 0; i < ivLength; ++i)
			std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)iv[i] << " ";
		std::cout << std::endl;

		/*
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(key, length, iv, ivLength);
		CryptoPP::AuthenticatedDecryptionFilter df(d, new CryptoPP::ArraySink(data), CryptoPP::HashVerificationFilter::Flags::HASH_AT_END);
		df.Put2(ivLength + iv, length2 - ivLength);

		*/

		SimpleKeyingInterface_SetKeyWithIV_fn(_this, key, length, iv, ivLength);
	}

	DECL_HOOK(void, MaybeDecryptData, uintptr_t _this, byte* key, uint32_t keyLength, uint32_t ivLength, byte* data, uint32_t length, char* unk)
	{
		static int callCount = 0;

		if (length > 16 && (ivLength != 10 || (uint32_t)key[0] != 0xba))
		{
			printf("[%04d] MaybeDecryptData: key = 0x%08p, keyLength = %i, ivLength = %i, data = 0x%08p, length = %i, unk->0x8: %i, unk->0xC: %i\nKey: ",
				callCount++, key, keyLength, ivLength, data, length, ((uint32_t*)unk)[2], ((uint32_t*)unk)[3]);
			for (int i = 0; i < keyLength; ++i)
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)key[i] << " ";
			std::cout << std::endl << "IV:  ";
			for (int i = 0; i < ivLength; ++i)
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data[i] << " ";
			std::cout << std::endl << "D>4: ";
			for (int i = ivLength; i < ivLength + 4 && i < length; ++i)
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data[i] << " ";
			std::cout << std::endl;
		}
		

		/*
		CryptoPP::GCM<CryptoPP::AES>::Decryption d;
		d.SetKeyWithIV(key, length, data, ivLength);
		CryptoPP::AuthenticatedDecryptionFilter df(d, new CryptoPP::ArraySink(), CryptoPP::AuthenticatedDecryptionFilter::Flags::MAC_AT_END);
		df.Put(data + ivLength, length - ivLength, 0);
		df.MessageEnd();
		// ...
		*/

		MaybeDecryptData_fn(_this, key, keyLength, ivLength, data, length, unk);

		/* Not working
		if (unk[0] == '{')
		{
			printf("[MaybeDecryptData] [%04d] [JSON] IV size: %i, key[0]: %02x, enclen: %i\n",
				callCount++, ivLength, (uint32_t)key[0], length);
		}
		*/
	}


	void FilenameHasherHook::Init()
	{
		HINSTANCE Module = LoadLibraryA("Starbase.exe");
		if (Module == NULL)
		{
			LOG("Failed to find Starbase.exe");
			return;
		}

		/*
		filenameHasher_fn = (filenameHasher_fn_t)PointerUtils::FindPattern(Module, "48 83 ec 08 4c 8d 14 11 4c 8b c9 48 83 fa 10 0f 82 a5 00 00 00 48 89 5c 24 10 45 8d 98 28 44 23 24 48 89 3c 24 41 8d 98 77 ca eb 85 49 8d 7a f0 41 8d 80 4f 86 c8 61 66 0f 1f 84 00 00 00 00 00 41 69 09 89 35 14 7a 44 2b d9 41 69 49 04 89 35 14 7a");
		if (filenameHasher_fn == nullptr)
		{
			LOG("Failed to find filenameHasher_fn");
			return;
		}

		 Hook::Attach(&(LPVOID&)filenameHasher_fn, FilenameHasher_Hook);
		 LOG("Hooked filenameHasher_fn");
		 hashfile.open("stringhashes.txt");
		 atexit(OnExit);
		*/

		/*
		fsopen_fn = (fsopen_fn_t)PointerUtils::FindPattern(Module, "48 89 5c 24 10 48 89 74 24 18 57 48 83 ec 30 41 8b f0 48 8b da 48 8b f9 48 85 c9 75 22 e8 6e b8 02 00 c7 00 16 00 00 00 e8 1b 18 00 00 33 c0 48 8b 5c 24 48 48 8b 74 24 50 48 83 c4 30 5f c3 48 85 db 74 d9 80 3a 00 74 d4 80 39 00 75 0d e8 3d b8 02 00 c7 00 16 00 00 00 eb d2 48 8d 4c 24 40 e8 17 eb 03 00 4c 8b 4c 24 40 4d 85 c9 75 0d e8 1c b8 02 00 c7 00 18 00 00 00 eb b1 48 83 64 24 20 00 44 8b c6 48 8b d3 48 8b cf e8 98 f4 03 00 48 8b d8 48 89 44 24 20 48 85 db 75 0a 48 8b 4c 24 40 e8 2d eb 03 00 48 8b 4c 24 40 e8 57 35 02 00 48 8b c3 e9 76 ff ff ff");
		if (fsopen_fn == nullptr)
		{
			LOG("Failed to find fsopen_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)fsopen_fn, fsopen_hook);
		LOG("Hooked fsopen_fn");
		*/

		/*
		OpenGameFile_fn = (OpenGameFile_fn_t)PointerUtils::FindPattern(Module, "48 89 5c 24 20 55 56 57 41 56 41 57 48 8d ac 24 80 fc ff ff 48 81 ec 80 04 00 00 48 8b 05 be 23 8f 04 48 33 c4 48 89 85 70 03 00 00 48 8b f1 4d 8b f8 48 8b 09 4c 8b f2 48 83 f9 ff 74 0d ff 15 3c cd 07 04 48 c7 06 ff ff ff ff 48 8d 4c 24 60 c6 46 08 00 e8 17 db a1 ff 8b 5c 24 68 8b 4c 24 6c 3b d9 75 21 ff c1 e8 04 88 c4 ff 3b 44 24 68 76 14 45 33 c9 48 8d 4c 24 60 44 8b c0 41 8d 51 02 e8 ea 8e c4 ff");
		if (OpenGameFile_fn == nullptr)
		{
			LOG("Failed to find OpenGameFile_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)OpenGameFile_fn, OpenGameFile_hook);
		LOG("Hooked OpenGameFile_fn");
		*/
		/*
		HINSTANCE kernel32_module = LoadLibraryA("kernel32.dll");
		if (kernel32_module == NULL)
		{
			LOG("Failed to find kernel32.dll");
			return;
		}
		CreateFileW_fn = (CreateFileW_fn_t)GetProcAddress(kernel32_module, "CreateFileW"); // Find export of kernel32
		if (CreateFileW_fn == nullptr)
		{
			LOG("Failed to find CreateFileW_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)CreateFileW_fn, CreateFileW_hook);
		LOG("Hooked CreateFileW_hook");

		LoadAndDecompressResFile_fn = (LoadAndDecompressResFile_fn_t)PointerUtils::FindPattern(Module, "48 89 5c 24 20 48 89 4c 24 08 55 57 41 54 41 56 41 57 48 8d 6c 24 d1 48 81 ec c0 00 00 00 80 7d 7f 00 4d 8b e1 49 8b f8 4c 8b fa 48 8b d9 74 0f 45 33 f6 4c 89 32 4c 89 72 08 e9 d8 06 00 00 49 8b 08 48 b8 cd 8c 55 ed d7 af 51 ff 44 8b 73 1c 48 c1 e9 21 49 33 08 4c 8b 43 10 48 0f af c8 49 83 e0 fc 48 89 b4 24 f8 00 00 00 48 8b c1 44 89 74 24 20 48 c1 e8 21 48 33 c1 48 b9 53 ec 85 1a fe b9 ce c4 48 0f af c1 b9 01 00 00 00 4f 8d 0c b0 48 8b d0 48 c1 ea 21 33 d0 0f 45 ca 48 8b d7 e8 bb 1c 00 00");
		if (LoadAndDecompressResFile_fn == nullptr)
		{
			LOG("Failed to find LoadAndDecompressResFile_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)LoadAndDecompressResFile_fn, LoadAndDecompressResFile_hook);
		LOG("Hooked LoadAndDecompressResFile_fn");

		unk_141776a30_fn = (unk_141776a30_fn_t)PointerUtils::FindPattern(Module, "48 89 5c 24 08 48 89 6c 24 10 48 89 74 24 18 48 89 7c 24 20 41 56 41 57 44 8b 74 24 38 49 8b e9 8b d9 4d 85 c0 74 5c 41 8d 76 ff 33 ff 44 8b d6 44 23 d1 45 8b da 47 8b 0c 90 45 85 c9 74 44 44 8d 7f 01 44 3b cb 75 11 48 8b 02 4b 8d 0c 5b 48 03 c9 48 39 44 cd 00 74 46 41 ff c2 ff c7 44 23 d6 41 8b ca 45 8b da 47 8b 0c 90 45 85 c9 0f 94 c0 41 2b c9 23 ce 0f b6 c0 3b f9 41 0f 47 c7 84 c0 74 c0 41 8b c6 48 8b 5c 24 18 48 8b 6c 24 20 48 8b 74 24 28 48 8b 7c 24 30 41 5f 41 5e c3 41 8b c2 eb e2");
		if (unk_141776a30_fn == nullptr)
		{
			LOG("Failed to find unk_141776a30_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)unk_141776a30_fn, unk_141776a30_hook);
		LOG("Hooked unk_141776a30_fn");

		DecryptData_fn = (DecryptData_fn_t)PointerUtils::FindPattern(Module, "4c 8b dc 55 49 8d 6b a8 48 81 ec 50 01 00 00 48 8b 05 7a d5 90 04 48 33 c4 48 89 45 00 33 c0 89 4d b8 48 c1 e9 20 44 89 44 24 04 48 89 54 24 08 c7 45 80 4b b5 00 aa c7 45 84 d4 80 7a dc c7 45 88 be ca ec 46 c7 45 8c 50 ff 6b cf c7 45 90 17 32 b0 56 c7 45 94 85 80 30 12 c7 45 98 32 b0 56 c5 c7 45 9c e1 c7 dd 9c c7 45 a0 c5 3e f8 20 c7 45 a4 2d ac b1 ce c7 45 a8 1c f5 5d ff c7 45 ac d9 80 89 ef");
		if (DecryptData_fn == nullptr)
		{
			LOG("Failed to find DecryptData_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)DecryptData_fn, DecryptData_hook);
		LOG("Hooked DecryptData_fn");
		*/
		
		// Requires to force-disable g_hasCLMUL (change 0x01 to 0x00 at 0x145607178 - c6 05 d5 90 f5 04 01 TO c6 05 d5 90 f5 04 00)
		//HOOK_SIG_METHOD(DecryptCacheFile, "48 56 57 53 49 8b f0 4d 8b d9 66 0f 6f 06 f3 0f 6f 21 66 0f ef c4 66 0f 7e c3 b8 f0 f0 f0 f0 23 c3 c1 e3 04 81 e3 f0 f0 f0 f0 0f b6 fc 66 0f 6f ac 3e 20 04 00 00");

		//HOOK_SIG_METHOD(GCM_Base_SetKeyWithoutResync, "40 53 55 56 57 41 54 41 55 41 56 41 57 48 81 ec d8 00 00 00 48 c7 44 24 38 fe ff ff ff 48 8b 05 dc 76 a9 00 48 33 c4 48 89 84 24 c0 00 00 00 4d 8b f9 49 8b f0 48 8b fa 4c 8b f1 48 8b 01 ff 90 f8 00 00 00");
		//HOOK_SIG_METHOD(SimpleKeyingInterface_SetKeyWithIV, "4c 8b dc 53 55 56 57 48 81 ec 88 00 00 00 49 c7 43 b0 fe ff ff ff 49 8b e8 48 8b f2 48 8b d9 48 8b 84 24 d0 00 00 00");
		HOOK_SIG_METHOD(MaybeDecryptData, "40 53 56 57 41 54 41 55 41 56 41 57 48 81 ec e0 03 00 00 48 8b 05 f6 b1 e2 00 48 33 c4 48 89 84 24 d0 03 00 00 45 8b f1 45 8b e8 48 8b fa 4c 8b a4 24 40 04 00 00 48 8b b4 24 50 04 00 00 8b 9c 24 48 04 00 00 41 3b de 73 07 32 c0 e9 9e 03 00 00");

		/* OLD
		DecryptCacheFile_fn = (DecryptCacheFile_fn_t)PointerUtils::FindPattern(Module, "48 56 57 53 49 8b f0 4d 8b d9 66 0f 6f 06 f3 0f 6f 21 66 0f ef c4 66 0f 7e c3 b8 f0 f0 f0 f0 23 c3 c1 e3 04 81 e3 f0 f0 f0 f0 0f b6 fc 66 0f 6f ac 3e 20 04 00 00");
		if (DecryptCacheFile_fn == nullptr)
		{
			LOG("Failed to find DecryptCacheFile_fn");
			return;
		}
		Hook::Attach(&(LPVOID&)DecryptCacheFile_fn, DecryptCacheFile);
		LOG("Hooked DecryptData_fn");
		*/

		//DecryptGameFile("fdb.bin", "fdb.bin.decrypted", 14799891);


#if 0
		LOG("Waiting for debugger to attach");
		while (!IsDebuggerPresent())
			Sleep(1);
		LOG("Debugger attached!");
#endif
#if 0
		LOG("Custom mode, allowing to test the deserial func. Write custom texts here.");
		while (true)
		{
			std::string command;
			std::cin >> command;

			if (command == "continue")
				break;

			int length = command.length();
			unsigned char* data = new unsigned char[length]();
			memcpy(data, command.c_str(), length);
			
			std::string log = "transformed data will be at " + convert_int((uintptr_t)data) + " (size: " + std::to_string(length + 1) + ")";
			Debug::Msg(log.c_str());

			DecryptData_fn(0, data, length);

			for (int i = 0; i < length; ++i)
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data[i] << " ";
			std::cout << std::endl;

			delete[] data;

			data = new unsigned char[length]();
			memcpy(data, command.c_str(), length);

			DecryptData(__m64 { 0UL }, (__m128i*)data, length);

			for (int i = 0; i < length; ++i)
				std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)data[i] << " ";
			std::cout << std::endl;

			delete[] data;

		}
#endif

	}


}
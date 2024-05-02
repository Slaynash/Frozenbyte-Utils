#include <iostream>
#include <inttypes.h>
#include <fstream>

#include "cryptlib.h"
#include "filters.h"
#include "gcm.h"
#include "aes.h"

// Mainly used for ships, IV length is 10
const uint8_t key0[] = {
	0xba, 0xae, 0x2f, 0x66, 0xe0, 0xd6, 0xac, 0x05,
	0x70, 0x9b, 0x96, 0x15, 0x63, 0xd9, 0x66, 0xf7,
	0x53, 0x84, 0x4b, 0xc4, 0x46, 0x9b, 0x68, 0xbe,
	0x45, 0xee, 0x2c, 0xb7, 0xdc, 0x1e, 0x7b, 0xd2
};

// Used for some config files, IV length is 9
const uint8_t key1[] = {
	0xab, 0x22, 0x36, 0xab, 0x34, 0x13, 0x6e, 0x50,
	0x75, 0x32, 0xc4, 0xe9, 0x36, 0x67, 0xc4, 0x3a,
	0x5a, 0x95, 0x7e, 0x19, 0x57, 0xa5, 0x4d, 0xf5,
	0x22, 0x26, 0xdd, 0x4f, 0xe6, 0xe5, 0x43, 0x22
};

/*
FBUF file format:

[   4 bytes] "FBUF"
[9-10 Bytes] IV
[...]        AES-GCM encrypted data + MAC
*/

int DecryptFile(std::string path, bool shouldUseKey1)
{
	size_t ivLength = shouldUseKey1 ? 9 : 10;


	// Read the file and store it in a buffer (most of the time the ships aren't *that* big.
	// In case they become too big, we should directly use a FileSink to decrypt it, rather
	// than read & ArraySink.

	std::ifstream infile(path.c_str(), std::ios_base::binary);

	if (!infile)
	{
		// TODO throw
		return -1;
	}

	infile.seekg(0, std::ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, std::ios::beg);

	if (length < (4 + ivLength))
	{
		// TODO throw
		return -2;
	}

	char *rawdata = new char[length];
	infile.read(rawdata, length);
	infile.close();

	char* filesig = rawdata;

	if (strncmp(filesig, "FBUF", 4) != 0)
	{
		delete[] rawdata;
		// TODO throw
		return -3;
	}


	// Decrypt the file according to its format

	uint8_t* iv   = (uint8_t*)rawdata + 4;
	uint8_t* data = (uint8_t*)rawdata + 4 + ivLength;

	size_t dataLength = length - 4 - ivLength;

	CryptoPP::byte* decryptedData = new CryptoPP::byte[dataLength];

	CryptoPP::GCM<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(shouldUseKey1 ? key1 : key0, sizeof(key0) / sizeof(uint8_t), iv, ivLength);

	CryptoPP::ArraySink rs(decryptedData, dataLength);

	CryptoPP::ArraySource(data, dataLength, true,
		new CryptoPP::AuthenticatedDecryptionFilter(d,
			new CryptoPP::Redirector(rs),
			CryptoPP::AuthenticatedDecryptionFilter::Flags::DEFAULT_FLAGS /* use MAC_AT_END to disable exception on invalid tag */));


	// Save the decrypted data to <source>.decrypted

	size_t recoveredLength = rs.TotalPutLength();

	std::ofstream outfile((path + ".decrypted").c_str(), std::ios_base::binary);
	outfile.write((char*)decryptedData, rs.TotalPutLength());
	outfile.close();


	// Cleanup

	delete[] decryptedData;
	delete[] rawdata;

	return 0;
}



int main(int argc, char** argv)
{
	char* pValue;
	size_t len;
	errno_t err = _dupenv_s(&pValue, &len, "APPDATA");
	if (err)
		return err;
	std::string appdata(pValue);
	delete[] pValue;

	// Example of SSC files
	//return DecryptFile(R"(C:\Users\hugof\AppData\Roaming\Starbase\ssc\autosave\ship_blueprints\ship_50.fbe)", false);
	//return DecryptFile(R"(C:\Users\hugof\AppData\Roaming\Starbase\blueprints\2681\19457\1.fbe)", false);
	// Example of a config file
	return DecryptFile(appdata + R"(\Starbase\ssc\user_box_layout_ship.lua)", false);
}
# FrozenByte Utils

A collection of tools and ressources related to FrozenByte game engine and Starbase files.

Disclaimer: I am not affiliated with Frozenbyte Inc and the ressources available here are only for educational purposes. If you are a Frozenbyte employee and wishes to contact me, please ping/DM me on Discord: `slaynash`.

Everything available here is based on Shadwen from 2016 and Starbase 922 (may 2023).

# Tools

### CPP: starbase_mod_proxy

Based on MelonLoader, generates a proxy DLL file used to load starbase_mod_loader

### CPP: starbase_mod_loader

Based on MelonLoader, the only file really interfacing with Starbase as of now is SBTest/FinenameHasherHook.cpp`.<br>
This file is a mess of tests, but there might is some interesting things in it.

### CPP: FBUFDecrypt

Decrypt some game files like ships, config and cache.

### CS: FDBUtils

Extract and decompress game files listed in the FDB file (untested on Trine 5). Note that this is 70GB+ of data on Starbase.

### CS: FBEUtils

Extract `PACKED_ZSTD` files (including .fbe files) into `PACKED_DATA` files.
This can also partially read datas from .fbe files (maps).<br>

Note: To extract Shadwen files, you need to manually compile and replace zlib.dll with `ZSTD_LEGACY_SUPPORT 4`

# Tools and library used in those tools

- Modified [MelonLoader](https://github.com/LavaGang/MelonLoader) 5.6.3: Mod Loader base
- [Ghidra](https://github.com/NationalSecurityAgency/ghidra) 10.3.3: Inspect the game's assembly code
- unluac 1.2.3_511: Decompile binary Lua files
- [CryptoPP](https://github.com/weidai11/cryptopp) 8.6: Decrypt AES-GCM data
- [ZStdNet](https://github.com/skbkontur/ZstdNet) 1.4.5: Decompress ZStd data
- Modified [CSharp-ChaCha20-NetStandard](https://github.com/mcraiha/CSharp-ChaCha20-NetStandard): Decrypt ChaCha8 data

# Ressources

## FDB

- `fdb.bin`: File DataBase.
- `data/`: game files

### Raw:
```
[4 bytes] Unknown, likely the file version (0x34)
[8 bytes] 64-bit Chacha8 nonce
[4 bytes] Decompressed data length
[...]     ZStandard-compressed data
```
### Decrypted:
The decrypted data contains an array of "file blocks" with the following format:
```
[CString] original name
[CString] name in data/
[5 bytes] Unknown
[4 bytes] Data offset (only not 0 if contained inside fdb.bin)
[4 bytes] Data length
[8 bytes] 64-bit Chacha8 nonce
```

## Packed Files
Some files are compressed. They usually start with `PACKED_ZSTD`. Their format usually is:
```
[11 bytes] "PACKED_DATA"
[ 4 bytes] Decompressed data length
[...]      ZStandard-compressed data
```

## FBUF files
```
[   4 bytes] "FBUF"
[9-10 Bytes] IV (used for decryption)
[...]        AES-GCM encrypted data
[? 12 Bytes] TAG (?) (MAC_AT_END with default size)
```
Note: some of them may contain compressed (again) data

## Keys

### Main [Chacha8](<https://en.wikipedia.org/wiki/Salsa20#ChaCha_variant>) key:
Note: The first row replace the standard "expand 32-byte k" row
```c
0xAA00B54B 0xDC7A80D4 0x46ECCABE 0xCF6BFF50
0x56B03217 0x12308085 0xC556B032 0x9CDDC7E1
0x20F83EC5 0xCEB1AC2D 0xFF5DF51C 0xEF8980D9
```

### Starbase Ships [AES](<https://en.wikipedia.org/wiki/Advanced_Encryption_Standard>)-[GCM](<https://en.wikipedia.org/wiki/Galois/Counter_Mode>) key
IV Size: 10 bytes
```
ba ae 2f 66 e0 d6 ac 05
70 9b 96 15 63 d9 66 f7
53 84 4b c4 46 9b 68 be
45 ee 2c b7 dc 1e 7b d2
```
### Unknown Starbase AES-GCM key (likely config files)
IV Size: 9 bytes
```
ab 22 36 ab 34 13 6e 50
75 32 c4 e9 36 67 c4 3a
5a 95 7e 19 57 a5 4d f5
22 26 dd 4f e6 e5 43 22
```

## Additional Starbase-specific infos

## Libraries used in the game

- PhysX 3.3
- LuaL (unknown lib, Lua 5.3.5)
- [crunch](https://github.com/BinomialLLC/crunch)
- [xAtlas](https://github.com/jpcy/xatlas)
- [CryptoPP](https://github.com/weidai11/cryptopp) (>=6_0_0)
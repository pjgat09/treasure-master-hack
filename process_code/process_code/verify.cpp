#include "data_sizes.h"
#include "verify.h"

uint8 carnival_world_working_code[0x80] = { 0xFD, 0x22, 0x3C, 0x40, 0x77, 0xEB, 0xD4, 0xEF, 0x9C, 0x44,
											0x93, 0x1C, 0xD7, 0xF8, 0x10, 0x97, 0x14, 0x93, 0x84, 0x22,
											0xDD, 0xE3, 0x3E, 0x77, 0x5C, 0x47, 0x11, 0x31, 0xAA, 0xD9,
											0xF1, 0x97, 0xE2, 0x44, 0x4E, 0x78, 0x05, 0x25, 0xCD, 0xBF,
											0xAE, 0xED, 0xCA, 0xD6, 0x1F, 0xD9, 0x30, 0x4D, 0x88, 0x18,
											0xB2, 0x89, 0xF6, 0x70, 0x43, 0xFE, 0x56, 0x3E, 0xF3, 0x1B,
											0x7C, 0xA0, 0xF7, 0xF8, 0xDF, 0xF5, 0x3C, 0xC7, 0xE9, 0xD5,
											0x24, 0x0E, 0xDA, 0xA9, 0xB0, 0xAA, 0x86, 0x51, 0x1F, 0x8F,
											0x4A, 0xEF, 0x8C, 0x81, 0xF8, 0x80, 0x4F, 0x8F, 0x54, 0xF2,
											0x8C, 0x14, 0x9C, 0xFA, 0xFE, 0xCF, 0x03, 0x82, 0x96, 0x4E,
											0x82, 0x4C, 0x4A, 0x72, 0x1C, 0x52, 0x2C, 0xDE, 0x0F, 0x94,
											0x58, 0xC2, 0xD6, 0x99, 0x36, 0x7F, 0xA3, 0xF0, 0xD1, 0x29,
											0xD0, 0x93, 0xBF, 0x42, 0xCF, 0x3D, 0xD2, 0x56 };

uint8 carnival_code_decrypted_machine_code[0x80] = {    0xA2, 0x05, 0xEC, 0x51, 0x04, 0xF0, 0x03, 0x20, 0x88, 0xED,
														0xA9, 0x00, 0x85, 0x3F, 0xA5, 0xFC, 0xC9, 0x02, 0xD0, 0x14,
														0xA0, 0x0F, 0x20, 0x7F, 0xAF, 0xB0, 0x0D, 0xA5, 0xD4, 0xD0,
														0x09, 0xAD, 0x68, 0x05, 0x18, 0x69, 0x10, 0x8D, 0x68, 0x05,
														0x4C, 0x35, 0xB5, 0xA0, 0x00, 0x20, 0xB6, 0x88, 0x4C, 0x35,
														0xB5, 0xA0, 0x01, 0x20, 0xB6, 0x88, 0x8A, 0xF0, 0x20, 0xA0,
														0x02, 0xDD, 0xA0, 0x03, 0x20, 0xB6, 0x88, 0xAD, 0x58, 0x05,
														0xC9, 0xF0, 0x90, 0x11, 0xA2, 0x12, 0xBD, 0x8B, 0xB9, 0x9D,
														0x57, 0x01, 0xCA, 0x10, 0xF7, 0x20, 0x52, 0x89, 0x4C, 0xEE,
														0x81, 0xA5, 0x48, 0x29, 0x07, 0xD0, 0x0C, 0xAD, 0x0B, 0x01,
														0xAE, 0x0A, 0x01, 0x8E, 0x0B, 0x01, 0x8D, 0x0A, 0x01, 0x4C,
														0x35, 0xB5, 0xC9, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
														0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int carnival_code_length = 0x72;
uint8 carnival_code[0x72] = {   0xF4, 0xD7, 0xD1, 0x9E, 0x46, 0x4F, 0x90, 0xF0, 0xA1, 0x3C, 
								0x59, 0xA3, 0xFA, 0x09, 0x3C, 0x2A, 0x0B, 0x5A, 0x44, 0x1B, 
								0x7E, 0x23, 0x72, 0x63, 0xDD, 0xFA, 0x41, 0x27, 0x9A, 0x46, 
								0x8B, 0xAE, 0xA7, 0xFB, 0xE2, 0xF5, 0x04, 0x01, 0x9A, 0x51,
								0xC3, 0x7A, 0x35, 0x58, 0x81, 0xAC, 0x59, 0xC2, 0xC3, 0x2A, 
								0xE4, 0x26, 0xAB, 0x90, 0x1F, 0x52, 0x84, 0xD4, 0xF5, 0x49, 
								0xC5, 0xE1, 0x55, 0xDC, 0xD8, 0x41, 0x28, 0xD1, 0x43, 0xF6, 
								0xF7, 0xA6, 0x6E, 0x52, 0xD2, 0xE4, 0x34, 0x39, 0xA1, 0x15, 
								0x1A, 0x31, 0x13, 0x0F, 0x21, 0xEA, 0xBF, 0x27, 0xF3, 0x23, 
								0xA4, 0xA0, 0x30, 0x67, 0x43, 0x32, 0x9B, 0x5C, 0xD2, 0xAB, 
								0x9F, 0x1B, 0x46, 0xD2, 0x7C, 0x3F, 0x6E, 0xD7, 0x23, 0xC8, 
								0xA6, 0xA1, 0x5E, 0x3D };

int other_world_code_length = 0x53;
uint8 other_world_code[0x53] = {    0x50, 0xF1, 0xFB, 0x44, 0xBD, 0xC1, 0xB1, 0x5E, 0xE4, 0x18, 
									0x03, 0x52, 0x1A, 0x1C, 0x93, 0x36, 0x6E, 0x2D, 0x2B, 0x2B, 
									0xB9, 0x5A, 0xA1, 0x58, 0x7B, 0x32, 0xDB, 0x9A, 0xA3, 0x49, 
									0x40, 0x12, 0x06, 0x9C, 0xBB, 0x49, 0xAE, 0xB3, 0xFF, 0x67, 
									0xF0, 0xD2, 0x8F, 0x6E, 0x45, 0xB7, 0xE5, 0x9A, 0x80, 0xAB, 
									0xFF, 0xD3, 0x98, 0x9A, 0x94, 0x0A, 0x72, 0x81, 0xCF, 0x0A, 
									0xFF, 0xFB, 0x54, 0xD9, 0x0C, 0xE3, 0x22, 0xF1, 0xE2, 0xD2, 
									0xF4, 0xC7, 0x86, 0x81, 0x90, 0x0B, 0x04, 0xD2, 0x44, 0x66, 
									0xC1, 0x68, 0xCA };

uint8 * decrypt_memory(uint8 * working_code, uint8 * encrypted_memory, int length)
{
	uint8 * decrypted_memory = new uint8[128];
	for (int i = 0; i < length; i++)
	{
		decrypted_memory[i] = encrypted_memory[i] ^ working_code[127 - i];
	}

	for (int i = length; i < 128; i++)
	{
		decrypted_memory[i] = 0;
	}

	return decrypted_memory;
}

bool verify_checksum(uint8 * memory, int length)
{
	uint16 sum = 0;
	for (int i = 0; i < length - 2; i++)
	{
		sum += memory[i];
	}

	return (memory[length - 1] == ((sum >> 8) & 0xFF)) && (memory[length - 2] == (sum & 0xFF));
}

bool compare_working_code(uint8 * block1, uint8 * block2)
{
	for (int i = 0; i < 128; i++)
	{
		if (block1[i] != block2[i])
			return false;
	}

	return true;
}

// TODO
int check_for_machine_code(uint8 * memory, int length)
{
	return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define BYTE uint8_t
#define BLOCK_LENGTH  16
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KRESET "\x1B[0m"

/*
 * The following lookup tables and functions are for internal use only!
 */
BYTE AES_Sbox[] = {99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171,
                   118, 202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192, 183, 253,
                   147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21, 4, 199, 35, 195, 24, 150, 5, 154,
                   7, 18, 128, 226, 235, 39, 178, 117, 9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227,
                   47, 132, 83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207, 208, 239, 170,
                   251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168, 81, 163, 64, 143, 146, 157, 56, 245,
                   188, 182, 218, 33, 16, 255, 243, 210, 205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61,
                   100, 93, 25, 115, 96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219, 224,
                   50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121, 231, 200, 55, 109, 141, 213,
                   78, 169, 108, 86, 244, 234, 101, 122, 174, 8, 186, 120, 37, 46, 28, 166, 180, 198, 232, 221,
                   116, 31, 75, 189, 139, 138, 112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29,
                   158, 225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223, 140, 161,
                   137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22
                  };

BYTE AES_ShiftRowTab[] = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};

BYTE AES_Sbox_Inv[256];
BYTE AES_ShiftRowTab_Inv[BLOCK_LENGTH];
BYTE AES_xtime[256];

void AES_SubBytes(BYTE state[], BYTE sbox[])
{
	int i;
	for (i = 0; i < BLOCK_LENGTH; i++)
		state[i] = sbox[state[i]];
}

void AES_AddRoundKey(BYTE state[], BYTE rkey[])
{
	int i;
	for (i = 0; i < BLOCK_LENGTH; i++)
		state[i] ^= rkey[i];
}

void AES_ShiftRows(BYTE state[], BYTE shifttab[])
{
	BYTE h[BLOCK_LENGTH];
	memcpy(h, state, BLOCK_LENGTH);
	int i;
	for (i = 0; i < BLOCK_LENGTH; i++)
		state[i] = h[shifttab[i]];
}

void AES_MixColumns(BYTE state[])
{
	int i;
	for (i = 0; i < BLOCK_LENGTH; i += 4) {
		BYTE s0 = state[i + 0], s1 = state[i + 1];
		BYTE s2 = state[i + 2], s3 = state[i + 3];
		BYTE h = s0 ^ s1 ^ s2 ^ s3;
		state[i + 0] ^= h ^ AES_xtime[s0 ^ s1];
		state[i + 1] ^= h ^ AES_xtime[s1 ^ s2];
		state[i + 2] ^= h ^ AES_xtime[s2 ^ s3];
		state[i + 3] ^= h ^ AES_xtime[s3 ^ s0];
	}
}

void AES_MixColumns_Inv(BYTE state[])
{
	int i;
	for (i = 0; i < BLOCK_LENGTH; i += 4) {
		BYTE s0 = state[i + 0], s1 = state[i + 1];
		BYTE s2 = state[i + 2], s3 = state[i + 3];
		BYTE h = s0 ^ s1 ^ s2 ^ s3;
		BYTE xh = AES_xtime[h];
		BYTE h1 = AES_xtime[AES_xtime[xh ^ s0 ^ s2]] ^ h;
		BYTE h2 = AES_xtime[AES_xtime[xh ^ s1 ^ s3]] ^ h;
		state[i + 0] ^= h1 ^ AES_xtime[s0 ^ s1];
		state[i + 1] ^= h2 ^ AES_xtime[s1 ^ s2];
		state[i + 2] ^= h1 ^ AES_xtime[s2 ^ s3];
		state[i + 3] ^= h2 ^ AES_xtime[s3 ^ s0];
	}
}

/*
 * AES_Init: initialize the tables needed at runtime.
 * Call this function before the (first) key expansion.
 */
void AES_Init()
{
	int i;
	for (i = 0; i < 256; i++)
		AES_Sbox_Inv[AES_Sbox[i]] = i;

	for (i = 0; i < BLOCK_LENGTH; i++)
		AES_ShiftRowTab_Inv[AES_ShiftRowTab[i]] = i;

	for (i = 0; i < 128; i++) {
		AES_xtime[i] = i << 1;
		AES_xtime[128 + i] = (i << 1) ^ 0x1b;
	}
}

/*
 * AES_Done: release memory reserved by AES_Init.
 * Call this function after the last encryption/decryption operation.
 */
void AES_Done()
{
	/*It dose nothing here*/
}

/* AES_ExpandKey: expand a cipher key. Depending on the desired encryption
 * strength of 128, 192 or 256 bits 'key' has to be a byte array of length
 * 16, 24 or 32, respectively. The key expansion is done "in place", meaning
 * that the array 'key' is modified.
 */
int AES_ExpandKey(BYTE key[], int keyLen)
{
	int kl = keyLen, ks = 0, Rcon = 1, i, j;
	BYTE temp[4], temp2[4];
	switch (kl) {
	case 16:
		ks = 16 * (10 + 1);
		break;
	case 24:
		ks = 16 * (12 + 1);
		break;
	case 32:
		ks = 16 * (14 + 1);
		break;
	default:
		printf("AES_ExpandKey: Only key lengths of 16, 24 or 32 bytes allowed!");
	}
	for (i = kl; i < ks; i += 4) {
		memcpy(temp, &key[i - 4], 4);
		if (i % kl == 0) {
			temp2[0] = AES_Sbox[temp[1]] ^ Rcon;
			temp2[1] = AES_Sbox[temp[2]];
			temp2[2] = AES_Sbox[temp[3]];
			temp2[3] = AES_Sbox[temp[0]];
			memcpy(temp, temp2, 4);
			if ((Rcon <<= 1) >= 256)
				Rcon ^= 0x11b;
		} else if ((kl > 24) && (i % kl == BLOCK_LENGTH)) {
			temp2[0] = AES_Sbox[temp[0]];
			temp2[1] = AES_Sbox[temp[1]];
			temp2[2] = AES_Sbox[temp[2]];
			temp2[3] = AES_Sbox[temp[3]];
			memcpy(temp, temp2, 4);
		}
		for (j = 0; j < 4; j++)
			key[i + j] = key[i + j - kl] ^ temp[j];
	}
	return ks;
}

/*
 * AES_Encrypt: encrypt the 16 byte array 'block' with the previously expanded key 'key'.
 */
void AES_Encrypt(BYTE block[], BYTE key[], int keyLen)
{
	BYTE h0[BLOCK_LENGTH];
    BYTE h1;
	AES_AddRoundKey(block, &key[0]);
	for (int i = BLOCK_LENGTH; i < keyLen - BLOCK_LENGTH; i += BLOCK_LENGTH) {
		//AES_SubBytes(block, AES_Sbox);
	    for (int j = 0; j < BLOCK_LENGTH; j++) {
		    block[j] = AES_Sbox[block[j]];
        }

		//AES_ShiftRows(block, AES_ShiftRowTab);
	    memcpy(h0, block, BLOCK_LENGTH);
	    for (int j = 0; j < BLOCK_LENGTH; j++) {
		    block[j] = h0[AES_ShiftRowTab[j]];
        }

		//AES_MixColumns(block);
	    for (int j = 0; j < BLOCK_LENGTH; j += 4) {
		    BYTE s0 = block[j + 0], s1 = block[j + 1];
		    BYTE s2 = block[j + 2], s3 = block[j + 3];
		    h1 = s0 ^ s1 ^ s2 ^ s3;
		    block[j + 0] ^= h1 ^ AES_xtime[s0 ^ s1];
		    block[j + 1] ^= h1 ^ AES_xtime[s1 ^ s2];
		    block[j + 2] ^= h1 ^ AES_xtime[s2 ^ s3];
		    block[j + 3] ^= h1 ^ AES_xtime[s3 ^ s0];
	    }

		//AES_AddRoundKey(block, &key[i]);
	    for (int j = 0; j < BLOCK_LENGTH; j++)
        {
		    block[j] ^= key[i+j];
        }
	}
	AES_SubBytes(block, AES_Sbox);
	AES_ShiftRows(block, AES_ShiftRowTab);
	AES_AddRoundKey(block, &key[keyLen-BLOCK_LENGTH]);
}

void AES_Encrypt_all(BYTE *inputs,BYTE *key,int expandKeyLen,uint32_t BLOCK_count)
{
    BYTE *p2block;
    for(uint32_t i = 0;i < BLOCK_count;i++) {
        p2block = inputs + i*BLOCK_LENGTH;
        AES_Encrypt(p2block, key, expandKeyLen);
    }
}
/*
 * AES_Decrypt: decrypt the 16 byte array 'block' with the previously expanded key 'key'.
 */
void AES_Decrypt(BYTE block[], BYTE key[], int keyLen)
{
	int l = keyLen, i;
	AES_AddRoundKey(block, &key[l - BLOCK_LENGTH]);
	AES_ShiftRows(block, AES_ShiftRowTab_Inv);
	AES_SubBytes(block, AES_Sbox_Inv);
	for (i = l - 32; i >= BLOCK_LENGTH; i -= BLOCK_LENGTH) {
		AES_AddRoundKey(block, &key[i]);
		AES_MixColumns_Inv(block);
		AES_ShiftRows(block, AES_ShiftRowTab_Inv);
		AES_SubBytes(block, AES_Sbox_Inv);
	}
	AES_AddRoundKey(block, &key[0]);
}

void AES_Verify(FILE *src_file, FILE*dest_file)
{
	int read_count_src;
	int read_count_dest;
	BYTE block_src[BLOCK_LENGTH];
	BYTE block_dest[BLOCK_LENGTH];
	read_count_src = fread(block_src, sizeof(BYTE), sizeof(block_src), src_file);
	read_count_dest = fread(block_dest, sizeof(BYTE), sizeof(block_dest), dest_file);
	while (read_count_src > 0) {
		if (read_count_src == read_count_dest) {
			if (memcmp(block_src, block_dest, read_count_src) != 0) {
			    fprintf(stderr, KRED "Verification FAIL!\n");
				exit(EXIT_FAILURE);
			}
			read_count_src = fread(block_src, sizeof(BYTE), sizeof(block_src), src_file);
			read_count_dest = fread(block_dest, sizeof(BYTE), sizeof(block_dest), dest_file);
		} else {
			fprintf(stderr, KRED "Verification FAIL!\n");
			exit(EXIT_FAILURE);
		}
	}
	printf( KGRN "Verification SUCCESS!\n");
}

int main(int argc, char **argv)
{
	int i;
	int read_count;
	BYTE block[BLOCK_LENGTH];
	BYTE key[BLOCK_LENGTH * (14 + 1)];
	int keyLen = BLOCK_LENGTH;
    struct timeval time_start_1;
    struct timeval time_end_1;
    double time_diff_1;
    struct timeval time_start_2;
    struct timeval time_end_2;
    double time_diff_2;

    FILE* output_file_decryption; 
	FILE* input_file = fopen(argv[1], "rb");
	FILE* input_file_2 = fopen(argv[1], "rb");
	FILE* output_file_encryption = fopen("test_files/output/output_file_encryption", "wb");
	if (input_file == NULL) {
		fprintf(stderr, "Open input file error!\n");
		exit(EXIT_FAILURE);
	}
	if (output_file_encryption == NULL) {
		fprintf(stderr, "Open output file error!\n");
		exit(EXIT_FAILURE);
	}

	AES_Init();
	for (i = 0; i < keyLen; i++) {
		key[i] = i;
	}

	int expandKeyLen = AES_ExpandKey(key, keyLen);
    /*get file size*/
    uint32_t BLOCK_count = 0;
    uint32_t last_BLOCK_size = 0;
    gettimeofday(&time_start_2,NULL);
	read_count = fread(block, sizeof(BYTE), sizeof(block), input_file_2);
	while (read_count > 0) {
		if (read_count == BLOCK_LENGTH) {
            BLOCK_count++;
		}else {
            last_BLOCK_size = read_count;
        }
		/*next iteration*/
		read_count = fread(block, sizeof(BYTE), sizeof(block), input_file_2);
	}
    fclose(input_file_2);
    uint32_t file_size = sizeof(BYTE)*(BLOCK_count*BLOCK_LENGTH+last_BLOCK_size);
    printf("file size:%u bytes\n",file_size);
    BYTE *inputs = (BYTE*)malloc(file_size);
    /*load file into ram*/
	read_count = fread(inputs, sizeof(BYTE), file_size, input_file);
    fclose(input_file);

    /*Start counting time*/
    gettimeofday(&time_start_1,NULL);

	/*Encrpyt the whole input file*/
    AES_Encrypt_all(inputs,key,expandKeyLen,BLOCK_count);

    /*End of counting time*/
    gettimeofday(&time_end_1,NULL);
    gettimeofday(&time_end_2,NULL);
    time_diff_1 = (1000000.0 * (double)(time_end_1.tv_sec-time_start_1.tv_sec)+(double)(time_end_1.tv_usec-time_start_1.tv_usec))/1000000.0;
    time_diff_2 = (1000000.0 * (double)(time_end_2.tv_sec-time_start_2.tv_sec)+(double)(time_end_2.tv_usec-time_start_2.tv_usec))/1000000.0;

    printf(KYEL "Encrpytion without disk I/O takes: %lf secs\n" KRESET,time_diff_1);
    printf(KYEL "Encrpytion with    disk I/O takes: %lf secs\n" KRESET,time_diff_2);
    printf(KYEL "disk I/O takes: %lf secs(with - without)\n" KRESET,time_diff_2-time_diff_1);
    printf("===============================================\n");
    printf("DO NOT do Parallelization for Decryption,this process is for Verification\n");
    printf("Decrypting for Verification...please wait\n");

    /*write the encrypted memory block into disk*/
    fwrite(inputs, sizeof(BYTE), file_size, output_file_encryption);
	fclose(output_file_encryption);

	output_file_encryption = fopen("test_files/output/output_file_encryption", "rb");
	if (output_file_encryption == NULL) {
		fprintf(stderr, "Open output file error!\n");
		exit(EXIT_FAILURE);
	}
	output_file_decryption = fopen("test_files/output/output_file_decryption", "wb");
	if (output_file_decryption == NULL) {
		fprintf(stderr, "Open output file error!\n");
		exit(EXIT_FAILURE);
	}
	/*Decrpyt the whole encrypted file*/
	read_count = fread(block, sizeof(BYTE), sizeof(block), output_file_encryption);
	while (read_count > 0) {
		if (read_count == BLOCK_LENGTH) {
			/*Decryption*/
			AES_Decrypt(block, key, expandKeyLen);
		}
		fwrite(block, sizeof(BYTE), read_count, output_file_decryption);
		/*next iteration*/
		read_count = fread(block, sizeof(BYTE), sizeof(block), output_file_encryption);
	}

	fclose(output_file_decryption);
	fclose(output_file_encryption);

	/*Verify the decryped file whether it is as same as the original input file*/
	output_file_decryption = fopen("test_files/output/output_file_decryption", "rb");
	input_file = fopen(argv[1], "rb");
	AES_Verify(input_file, output_file_decryption);
	AES_Done();
	fclose(input_file);
	fclose(output_file_decryption);
    free(inputs);
}

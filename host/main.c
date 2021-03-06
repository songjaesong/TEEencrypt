/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <TEEencrypt_ta.h>

/*
argv[1] : option(-e, -d) argv[2] : filename
*/
int main(int argc, char* argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_TEEencrypt_UUID;
	uint32_t err_origin;

	char plaintext[64] = {0,};
	char ciphertext[64] = {0,};
	int encKey = 0;
	int len = 64;

	res = TEEC_InitializeContext(NULL, &ctx);
	
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,TEEC_VALUE_INOUT, TEEC_NONE,
					TEEC_NONE);
	op.params[0].tmpref.buffer = plaintext;
	op.params[0].tmpref.size = len;
	
	//option : -e
	if(strcmp(argv[1], "-e") == 0){
		//read plaintext.txt
		FILE* fp = NULL;
		if( (fp = fopen("plaintext.txt", "r")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}
		fgets(plaintext, sizeof(plaintext), fp);
		memcpy(op.params[0].tmpref.buffer, plaintext, len);

		printf("====================Encryption Text=====================\n");
		res = TEEC_InvokeCommand(&sess, TA_TEEencrypt_CMD_ENC_VALUE, &op,
					 &err_origin);
		if(res != TEEC_SUCCESS){
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);
		}
		
		memcpy(ciphertext, op.params[0].tmpref.buffer, len);
		printf("Ciphertext : %s\n", ciphertext);
		
		printf("==================Encryption Key===================\n");
		res = TEEC_InvokeCommand(&sess, TA_TEEencrypt_CMD_ENCKEY_GET, &op,
					 &err_origin);
		encKey = op.params[1].value.a;
		printf("EncKey : %d\n", encKey);
		fclose(fp);

		//write ciphertext.txt
		FILE* fc = NULL;
		if((fc = fopen("ciphertext.txt", "w")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}
		fprintf(fc, ciphertext);
		fclose(fc);

		//write encKey.txt
		FILE* fk = NULL;
		if((fk = fopen("encKey.txt", "w")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}
		fprintf(fk,"%d", encKey);
		fclose(fk);
	}
	//option : -d
	else if(strcmp(argv[1], "-d") == 0){
		FILE* fdf = NULL;
		if((fdf = fopen(argv[2], "r")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}
		fgets(ciphertext, sizeof(ciphertext), fdf);
		memcpy(op.params[0].tmpref.buffer, ciphertext, len);
		
		//read encKey.txt
		FILE* fdk = NULL;
		if((fdk = fopen(argv[3], "r")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}		
		char encKeyOpen[10];
		fgets(encKeyOpen, sizeof(encKeyOpen), fdk);
		op.params[1].value.a = atoi(encKeyOpen);
		
		printf("====================Decryption Text=====================\n");
		res = TEEC_InvokeCommand(&sess, TA_TEEencrypt_CMD_DEC_VALUE, &op,
					 &err_origin);
		if(res != TEEC_SUCCESS){
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);
		}
		memcpy(ciphertext, op.params[0].tmpref.buffer, len);
		printf("encryptiontext : %s\n", ciphertext);

		//write decrypted.txt
		FILE* fdw = NULL;
		if((fdw = fopen("decrypted.txt", "w")) == NULL){
			fprintf(stderr, "Error ");
			exit(1);
		}
		fprintf(fdw, ciphertext);
		fclose(fdw);
		
	}else{	
		printf("argv[1] error");
		exit(1);
	}
	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);
	return 0;
}

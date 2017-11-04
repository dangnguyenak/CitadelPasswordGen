
/*
 ---------------------------------------------------------------------------
 Copyright (c) 2003, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 1/06/2003

 This file contains the definitions required to use AES (Rijndael) in C++.
*/

#ifndef _AESCPP_H
#define _AESCPP_H

#include "aes.h"

class AESencrypt
{   aes_encrypt_ctx cx[1];
public:
    AESencrypt(void)    
            { };

	~AESencrypt()
	{
		for (int i=0; i<KS_LENGTH; ++i)
		{
			cx[0].ks[i] = rand();
		}
	}
#ifdef  AES_128
    AESencrypt(const unsigned char in_key[])
            { aes_encrypt_key128(in_key, cx); }
    void key128(const unsigned char in_key[])
            { aes_encrypt_key128(in_key, cx); }
#endif
#ifdef  AES_192
    void key192(const unsigned char in_key[])
            { aes_encrypt_key192(in_key, cx); }
#endif
#ifdef  AES_256
    void key256(const unsigned char in_key[])
            { aes_encrypt_key256(in_key, cx); }
#endif
#ifdef  AES_VAR
    void key(const unsigned char in_key[], int key_len)
            { aes_encrypt_key(in_key, key_len, cx); }
#endif
    void encrypt(const unsigned char in_blk[], unsigned char out_blk[]) const
            { aes_encrypt(in_blk, out_blk, cx);  }
};

class AESdecrypt
{   aes_decrypt_ctx cx[1];
public:
    AESdecrypt(void)    
            { };
#ifdef  AES_128
    AESdecrypt(const unsigned char in_key[])
            { aes_decrypt_key128(in_key, cx); }
    void key128(const unsigned char in_key[])
            { aes_decrypt_key128(in_key, cx); }
#endif
#ifdef  AES_192
    void key192(const unsigned char in_key[])
            { aes_decrypt_key192(in_key, cx); }
#endif
#ifdef  AES_256
    void key256(const unsigned char in_key[])
            { aes_decrypt_key256(in_key, cx); }
#endif
#ifdef  AES_VAR
    void key(const unsigned char in_key[], int key_len)
            { aes_decrypt_key(in_key, key_len, cx); }
#endif
    void decrypt(const unsigned char in_blk[], unsigned char out_blk[]) const
            { aes_decrypt(in_blk, out_blk, cx);  }
};

#endif

/*
 * QEMU Crypto XTS cipher mode
 *
 * Copyright (c) 2015-2016 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * This code is originally derived from public domain / WTFPL code in
 * LibTomCrypt crytographic library http://libtom.org. The XTS code
 * was donated by Elliptic Semiconductor Inc (www.ellipticsemi.com)
 * to the LibTom Projects
 *
 */

#include "qemu/osdep.h"
#include "crypto/init.h"
#include "crypto/xts.h"
#include "crypto/aes.h"

typedef struct {
    const char *path;
    int keylen;
    unsigned char key1[32];
    unsigned char key2[32];
    uint64_t seqnum;
    unsigned long PTLEN;
    unsigned char PTX[512], CTX[512];
} QCryptoXTSTestData;

static const QCryptoXTSTestData test_data[] = {
    /* #1 32 byte key, 32 byte PTX */
    {
        "/crypto/xts/t-1-key-32-ptx-32",
        32,
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        0,
        32,
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x91, 0x7c, 0xf6, 0x9e, 0xbd, 0x68, 0xb2, 0xec,
          0x9b, 0x9f, 0xe9, 0xa3, 0xea, 0xdd, 0xa6, 0x92,
          0xcd, 0x43, 0xd2, 0xf5, 0x95, 0x98, 0xed, 0x85,
          0x8c, 0x02, 0xc2, 0x65, 0x2f, 0xbf, 0x92, 0x2e },
    },

    /* #2, 32 byte key, 32 byte PTX */
    {
        "/crypto/xts/t-2-key-32-ptx-32",
        32,
        { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
          0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 },
        { 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
          0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 },
        0x3333333333LL,
        32,
        { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 },
        { 0xc4, 0x54, 0x18, 0x5e, 0x6a, 0x16, 0x93, 0x6e,
          0x39, 0x33, 0x40, 0x38, 0xac, 0xef, 0x83, 0x8b,
          0xfb, 0x18, 0x6f, 0xff, 0x74, 0x80, 0xad, 0xc4,
          0x28, 0x93, 0x82, 0xec, 0xd6, 0xd3, 0x94, 0xf0 },
    },

    /* #5 from xts.7, 32 byte key, 32 byte PTX */
    {
        "/crypto/xts/t-5-key-32-ptx-32",
        32,
        { 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
          0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0 },
        { 0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
          0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0 },
        0x123456789aLL,
        32,
        { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
          0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 },
        { 0xb0, 0x1f, 0x86, 0xf8, 0xed, 0xc1, 0x86, 0x37,
          0x06, 0xfa, 0x8a, 0x42, 0x53, 0xe3, 0x4f, 0x28,
          0xaf, 0x31, 0x9d, 0xe3, 0x83, 0x34, 0x87, 0x0f,
          0x4d, 0xd1, 0xf9, 0x4c, 0xbe, 0x98, 0x32, 0xf1 },
    },

    /* #4, 32 byte key, 512 byte PTX  */
    {
        "/crypto/xts/t-4-key-32-ptx-512",
        32,
        { 0x27, 0x18, 0x28, 0x18, 0x28, 0x45, 0x90, 0x45,
          0x23, 0x53, 0x60, 0x28, 0x74, 0x71, 0x35, 0x26 },
        { 0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93,
          0x23, 0x84, 0x62, 0x64, 0x33, 0x83, 0x27, 0x95 },
        0,
        512,
        {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
            0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
            0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
            0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
            0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
            0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
            0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
            0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
            0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
            0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
            0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
            0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
            0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
            0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
            0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
            0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
            0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
            0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
            0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
            0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
            0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
            0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
            0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
            0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
            0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
            0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
            0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
            0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
            0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
            0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
            0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
            0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
            0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
            0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
            0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
            0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
            0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
            0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
            0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
            0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
            0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
            0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        },
        {
            0x27, 0xa7, 0x47, 0x9b, 0xef, 0xa1, 0xd4, 0x76,
            0x48, 0x9f, 0x30, 0x8c, 0xd4, 0xcf, 0xa6, 0xe2,
            0xa9, 0x6e, 0x4b, 0xbe, 0x32, 0x08, 0xff, 0x25,
            0x28, 0x7d, 0xd3, 0x81, 0x96, 0x16, 0xe8, 0x9c,
            0xc7, 0x8c, 0xf7, 0xf5, 0xe5, 0x43, 0x44, 0x5f,
            0x83, 0x33, 0xd8, 0xfa, 0x7f, 0x56, 0x00, 0x00,
            0x05, 0x27, 0x9f, 0xa5, 0xd8, 0xb5, 0xe4, 0xad,
            0x40, 0xe7, 0x36, 0xdd, 0xb4, 0xd3, 0x54, 0x12,
            0x32, 0x80, 0x63, 0xfd, 0x2a, 0xab, 0x53, 0xe5,
            0xea, 0x1e, 0x0a, 0x9f, 0x33, 0x25, 0x00, 0xa5,
            0xdf, 0x94, 0x87, 0xd0, 0x7a, 0x5c, 0x92, 0xcc,
            0x51, 0x2c, 0x88, 0x66, 0xc7, 0xe8, 0x60, 0xce,
            0x93, 0xfd, 0xf1, 0x66, 0xa2, 0x49, 0x12, 0xb4,
            0x22, 0x97, 0x61, 0x46, 0xae, 0x20, 0xce, 0x84,
            0x6b, 0xb7, 0xdc, 0x9b, 0xa9, 0x4a, 0x76, 0x7a,
            0xae, 0xf2, 0x0c, 0x0d, 0x61, 0xad, 0x02, 0x65,
            0x5e, 0xa9, 0x2d, 0xc4, 0xc4, 0xe4, 0x1a, 0x89,
            0x52, 0xc6, 0x51, 0xd3, 0x31, 0x74, 0xbe, 0x51,
            0xa1, 0x0c, 0x42, 0x11, 0x10, 0xe6, 0xd8, 0x15,
            0x88, 0xed, 0xe8, 0x21, 0x03, 0xa2, 0x52, 0xd8,
            0xa7, 0x50, 0xe8, 0x76, 0x8d, 0xef, 0xff, 0xed,
            0x91, 0x22, 0x81, 0x0a, 0xae, 0xb9, 0x9f, 0x91,
            0x72, 0xaf, 0x82, 0xb6, 0x04, 0xdc, 0x4b, 0x8e,
            0x51, 0xbc, 0xb0, 0x82, 0x35, 0xa6, 0xf4, 0x34,
            0x13, 0x32, 0xe4, 0xca, 0x60, 0x48, 0x2a, 0x4b,
            0xa1, 0xa0, 0x3b, 0x3e, 0x65, 0x00, 0x8f, 0xc5,
            0xda, 0x76, 0xb7, 0x0b, 0xf1, 0x69, 0x0d, 0xb4,
            0xea, 0xe2, 0x9c, 0x5f, 0x1b, 0xad, 0xd0, 0x3c,
            0x5c, 0xcf, 0x2a, 0x55, 0xd7, 0x05, 0xdd, 0xcd,
            0x86, 0xd4, 0x49, 0x51, 0x1c, 0xeb, 0x7e, 0xc3,
            0x0b, 0xf1, 0x2b, 0x1f, 0xa3, 0x5b, 0x91, 0x3f,
            0x9f, 0x74, 0x7a, 0x8a, 0xfd, 0x1b, 0x13, 0x0e,
            0x94, 0xbf, 0xf9, 0x4e, 0xff, 0xd0, 0x1a, 0x91,
            0x73, 0x5c, 0xa1, 0x72, 0x6a, 0xcd, 0x0b, 0x19,
            0x7c, 0x4e, 0x5b, 0x03, 0x39, 0x36, 0x97, 0xe1,
            0x26, 0x82, 0x6f, 0xb6, 0xbb, 0xde, 0x8e, 0xcc,
            0x1e, 0x08, 0x29, 0x85, 0x16, 0xe2, 0xc9, 0xed,
            0x03, 0xff, 0x3c, 0x1b, 0x78, 0x60, 0xf6, 0xde,
            0x76, 0xd4, 0xce, 0xcd, 0x94, 0xc8, 0x11, 0x98,
            0x55, 0xef, 0x52, 0x97, 0xca, 0x67, 0xe9, 0xf3,
            0xe7, 0xff, 0x72, 0xb1, 0xe9, 0x97, 0x85, 0xca,
            0x0a, 0x7e, 0x77, 0x20, 0xc5, 0xb3, 0x6d, 0xc6,
            0xd7, 0x2c, 0xac, 0x95, 0x74, 0xc8, 0xcb, 0xbc,
            0x2f, 0x80, 0x1e, 0x23, 0xe5, 0x6f, 0xd3, 0x44,
            0xb0, 0x7f, 0x22, 0x15, 0x4b, 0xeb, 0xa0, 0xf0,
            0x8c, 0xe8, 0x89, 0x1e, 0x64, 0x3e, 0xd9, 0x95,
            0xc9, 0x4d, 0x9a, 0x69, 0xc9, 0xf1, 0xb5, 0xf4,
            0x99, 0x02, 0x7a, 0x78, 0x57, 0x2a, 0xee, 0xbd,
            0x74, 0xd2, 0x0c, 0xc3, 0x98, 0x81, 0xc2, 0x13,
            0xee, 0x77, 0x0b, 0x10, 0x10, 0xe4, 0xbe, 0xa7,
            0x18, 0x84, 0x69, 0x77, 0xae, 0x11, 0x9f, 0x7a,
            0x02, 0x3a, 0xb5, 0x8c, 0xca, 0x0a, 0xd7, 0x52,
            0xaf, 0xe6, 0x56, 0xbb, 0x3c, 0x17, 0x25, 0x6a,
            0x9f, 0x6e, 0x9b, 0xf1, 0x9f, 0xdd, 0x5a, 0x38,
            0xfc, 0x82, 0xbb, 0xe8, 0x72, 0xc5, 0x53, 0x9e,
            0xdb, 0x60, 0x9e, 0xf4, 0xf7, 0x9c, 0x20, 0x3e,
            0xbb, 0x14, 0x0f, 0x2e, 0x58, 0x3c, 0xb2, 0xad,
            0x15, 0xb4, 0xaa, 0x5b, 0x65, 0x50, 0x16, 0xa8,
            0x44, 0x92, 0x77, 0xdb, 0xd4, 0x77, 0xef, 0x2c,
            0x8d, 0x6c, 0x01, 0x7d, 0xb7, 0x38, 0xb1, 0x8d,
            0xeb, 0x4a, 0x42, 0x7d, 0x19, 0x23, 0xce, 0x3f,
            0xf2, 0x62, 0x73, 0x57, 0x79, 0xa4, 0x18, 0xf2,
            0x0a, 0x28, 0x2d, 0xf9, 0x20, 0x14, 0x7b, 0xea,
            0xbe, 0x42, 0x1e, 0xe5, 0x31, 0x9d, 0x05, 0x68,
        }
    },

    /* #7, 32 byte key, 17 byte PTX */
    {
        "/crypto/xts/t-7-key-32-ptx-17",
        32,
        { 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
          0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0 },
        { 0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
          0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0 },
        0x123456789aLL,
        17,
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 },
        { 0x6c, 0x16, 0x25, 0xdb, 0x46, 0x71, 0x52, 0x2d,
          0x3d, 0x75, 0x99, 0x60, 0x1d, 0xe7, 0xca, 0x09, 0xed },
    },

    /* #15, 32 byte key, 25 byte PTX */
    {
        "/crypto/xts/t-15-key-32-ptx-25",
        32,
        { 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
          0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0 },
        { 0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
          0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0 },
        0x123456789aLL,
        25,
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
          0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 },
        { 0x8f, 0x4d, 0xcb, 0xad, 0x55, 0x55, 0x8d, 0x7b,
          0x4e, 0x01, 0xd9, 0x37, 0x9c, 0xd4, 0xea, 0x22,
          0xed, 0xbf, 0x9d, 0xac, 0xe4, 0x5d, 0x6f, 0x6a, 0x73 },
    },

    /* #21, 32 byte key, 31 byte PTX */
    {
        "/crypto/xts/t-21-key-32-ptx-31",
        32,
        { 0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
          0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf2, 0xf1, 0xf0 },
        { 0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
          0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0 },
        0x123456789aLL,
        31,
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
          0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
          0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e },
        { 0xd0, 0x5b, 0xc0, 0x90, 0xa8, 0xe0, 0x4f, 0x1b,
          0x3d, 0x3e, 0xcd, 0xd5, 0xba, 0xec, 0x0f, 0xd4,
          0xed, 0xbf, 0x9d, 0xac, 0xe4, 0x5d, 0x6f, 0x6a,
          0x73, 0x06, 0xe6, 0x4b, 0xe5, 0xdd, 0x82 },
    },
};

#define STORE64L(x, y)                                                  \
    do {                                                                \
        (y)[7] = (unsigned char)(((x) >> 56) & 255);                    \
        (y)[6] = (unsigned char)(((x) >> 48) & 255);                    \
        (y)[5] = (unsigned char)(((x) >> 40) & 255);                    \
        (y)[4] = (unsigned char)(((x) >> 32) & 255);                    \
        (y)[3] = (unsigned char)(((x) >> 24) & 255);                    \
        (y)[2] = (unsigned char)(((x) >> 16) & 255);                    \
        (y)[1] = (unsigned char)(((x) >> 8) & 255);                     \
        (y)[0] = (unsigned char)((x) & 255);                            \
    } while (0)

struct TestAES {
    AES_KEY enc;
    AES_KEY dec;
};

static void test_xts_aes_encrypt(const void *ctx,
                                 size_t length,
                                 uint8_t *dst,
                                 const uint8_t *src)
{
    const struct TestAES *aesctx = ctx;

    AES_encrypt(src, dst, &aesctx->enc);
}


static void test_xts_aes_decrypt(const void *ctx,
                                 size_t length,
                                 uint8_t *dst,
                                 const uint8_t *src)
{
    const struct TestAES *aesctx = ctx;

    AES_decrypt(src, dst, &aesctx->dec);
}


static void test_xts(const void *opaque)
{
    const QCryptoXTSTestData *data = opaque;
    unsigned char OUT[512], Torg[16], T[16];
    uint64_t seq;
    int j;
    unsigned long len;
    struct TestAES aesdata;
    struct TestAES aestweak;

    for (j = 0; j < 2; j++) {
        /* skip the cases where
         * the length is smaller than 2*blocklen
         * or the length is not a multiple of 32
         */
        if ((j == 1) && ((data->PTLEN < 32) || (data->PTLEN % 32))) {
            continue;
        }
        len = data->PTLEN / 2;

        AES_set_encrypt_key(data->key1, data->keylen / 2 * 8, &aesdata.enc);
        AES_set_decrypt_key(data->key1, data->keylen / 2 * 8, &aesdata.dec);
        AES_set_encrypt_key(data->key2, data->keylen / 2 * 8, &aestweak.enc);
        AES_set_decrypt_key(data->key2, data->keylen / 2 * 8, &aestweak.dec);

        seq = data->seqnum;
        STORE64L(seq, Torg);
        memset(Torg + 8, 0, 8);

        memcpy(T, Torg, sizeof(T));
        if (j == 0) {
            xts_encrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, data->PTLEN, OUT, data->PTX);
        } else {
            xts_encrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, len, OUT, data->PTX);
            xts_encrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, len, &OUT[len], &data->PTX[len]);
        }

        g_assert(memcmp(OUT, data->CTX, data->PTLEN) == 0);

        memcpy(T, Torg, sizeof(T));
        if (j == 0) {
            xts_decrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, data->PTLEN, OUT, data->CTX);
        } else {
            xts_decrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, len, OUT, data->CTX);
            xts_decrypt(&aesdata, &aestweak,
                        test_xts_aes_encrypt,
                        test_xts_aes_decrypt,
                        T, len, &OUT[len], &data->CTX[len]);
        }

        g_assert(memcmp(OUT, data->PTX, data->PTLEN) == 0);
    }
}


int main(int argc, char **argv)
{
    size_t i;

    g_test_init(&argc, &argv, NULL);

    g_assert(qcrypto_init(NULL) == 0);

    for (i = 0; i < G_N_ELEMENTS(test_data); i++) {
        g_test_add_data_func(test_data[i].path, &test_data[i], test_xts);
    }

    return g_test_run();
}

/*
 * Copyright (c) 2010-2012 Frank Morgner and Dominik Oepen
 *
 * This file is part of OpenPACE.
 *
 * OpenPACE is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * OpenPACE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * OpenPACE.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file eac_dh.c
 * @brief Diffie Hellman helper functions
 *
 * @author Frank Morgner <frankmorgner@gmail.com>
 * @author Dominik Oepen <oepen@informatik.hu-berlin.de>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "eac_dh.h"
#include "eac_err.h"
#include "misc.h"
#include <eac/eac.h>
#include <openssl/bn.h>
#include <openssl/evp.h>

static const unsigned char rfc_5114_modp_1_p[] = {
   0xB1, 0x0B, 0x8F, 0x96, 0xA0, 0x80, 0xE0, 0x1D, 0xDE, 0x92, 0xDE, 0x5E,
   0xAE, 0x5D, 0x54, 0xEC, 0x52, 0xC9, 0x9F, 0xBC, 0xFB, 0x06, 0xA3, 0xC6,
   0x9A, 0x6A, 0x9D, 0xCA, 0x52, 0xD2, 0x3B, 0x61, 0x60, 0x73, 0xE2, 0x86,
   0x75, 0xA2, 0x3D, 0x18, 0x98, 0x38, 0xEF, 0x1E, 0x2E, 0xE6, 0x52, 0xC0,
   0x13, 0xEC, 0xB4, 0xAE, 0xA9, 0x06, 0x11, 0x23, 0x24, 0x97, 0x5C, 0x3C,
   0xD4, 0x9B, 0x83, 0xBF, 0xAC, 0xCB, 0xDD, 0x7D, 0x90, 0xC4, 0xBD, 0x70,
   0x98, 0x48, 0x8E, 0x9C, 0x21, 0x9A, 0x73, 0x72, 0x4E, 0xFF, 0xD6, 0xFA,
   0xE5, 0x64, 0x47, 0x38, 0xFA, 0xA3, 0x1A, 0x4F, 0xF5, 0x5B, 0xCC, 0xC0,
   0xA1, 0x51, 0xAF, 0x5F, 0x0D, 0xC8, 0xB4, 0xBD, 0x45, 0xBF, 0x37, 0xDF,
   0x36, 0x5C, 0x1A, 0x65, 0xE6, 0x8C, 0xFD, 0xA7, 0x6D, 0x4D, 0xA7, 0x08,
   0xDF, 0x1F, 0xB2, 0xBC, 0x2E, 0x4A, 0x43, 0x71
};

static const unsigned char rfc_5114_modp_1_g[] = {
    0xA4, 0xD1, 0xCB, 0xD5, 0xC3, 0xFD, 0x34, 0x12, 0x67, 0x65, 0xA4, 0x42,
    0xEF, 0xB9, 0x99, 0x05, 0xF8, 0x10, 0x4D, 0xD2, 0x58, 0xAC, 0x50, 0x7F,
    0xD6, 0x40, 0x6C, 0xFF, 0x14, 0x26, 0x6D, 0x31, 0x26, 0x6F, 0xEA, 0x1E,
    0x5C, 0x41, 0x56, 0x4B, 0x77, 0x7E, 0x69, 0x0F, 0x55, 0x04, 0xF2, 0x13,
    0x16, 0x02, 0x17, 0xB4, 0xB0, 0x1B, 0x88, 0x6A, 0x5E, 0x91, 0x54, 0x7F,
    0x9E, 0x27, 0x49, 0xF4, 0xD7, 0xFB, 0xD7, 0xD3, 0xB9, 0xA9, 0x2E, 0xE1,
    0x90, 0x9D, 0x0D, 0x22, 0x63, 0xF8, 0x0A, 0x76, 0xA6, 0xA2, 0x4C, 0x08,
    0x7A, 0x09, 0x1F, 0x53, 0x1D, 0xBF, 0x0A, 0x01, 0x69, 0xB6, 0xA2, 0x8A,
    0xD6, 0x62, 0xA4, 0xD1, 0x8E, 0x73, 0xAF, 0xA3, 0x2D, 0x77, 0x9D, 0x59,
    0x18, 0xD0, 0x8B, 0xC8, 0x85, 0x8F, 0x4D, 0xCE, 0xF9, 0x7C, 0x2A, 0x24,
    0x85, 0x5E, 0x6E, 0xEB, 0x22, 0xB3, 0xB2, 0xE5
};

static const unsigned char rfc_5114_modp_1_q[] = {
    0xF5, 0x18, 0xAA, 0x87, 0x81, 0xA8, 0xDF, 0x27, 0x8A, 0xBA, 0x4E, 0x7D,
    0x64, 0xB7, 0xCB, 0x9D, 0x49, 0x46, 0x23, 0x53
};

static const unsigned char rfc_5114_modp_2_p[] = {
    0xAD, 0x10, 0x7E, 0x1E, 0x91, 0x23, 0xA9, 0xD0, 0xD6, 0x60, 0xFA, 0xA7,
    0x95, 0x59, 0xC5, 0x1F, 0xA2, 0x0D, 0x64, 0xE5, 0x68, 0x3B, 0x9F, 0xD1,
    0xB5, 0x4B, 0x15, 0x97, 0xB6, 0x1D, 0x0A, 0x75, 0xE6, 0xFA, 0x14, 0x1D,
    0xF9, 0x5A, 0x56, 0xDB, 0xAF, 0x9A, 0x3C, 0x40, 0x7B, 0xA1, 0xDF, 0x15,
    0xEB, 0x3D, 0x68, 0x8A, 0x30, 0x9C, 0x18, 0x0E, 0x1D, 0xE6, 0xB8, 0x5A,
    0x12, 0x74, 0xA0, 0xA6, 0x6D, 0x3F, 0x81, 0x52, 0xAD, 0x6A, 0xC2, 0x12,
    0x90, 0x37, 0xC9, 0xED, 0xEF, 0xDA, 0x4D, 0xF8, 0xD9, 0x1E, 0x8F, 0xEF,
    0x55, 0xB7, 0x39, 0x4B, 0x7A, 0xD5, 0xB7, 0xD0, 0xB6, 0xC1, 0x22, 0x07,
    0xC9, 0xF9, 0x8D, 0x11, 0xED, 0x34, 0xDB, 0xF6, 0xC6, 0xBA, 0x0B, 0x2C,
    0x8B, 0xBC, 0x27, 0xBE, 0x6A, 0x00, 0xE0, 0xA0, 0xB9, 0xC4, 0x97, 0x08,
    0xB3, 0xBF, 0x8A, 0x31, 0x70, 0x91, 0x88, 0x36, 0x81, 0x28, 0x61, 0x30,
    0xBC, 0x89, 0x85, 0xDB, 0x16, 0x02, 0xE7, 0x14, 0x41, 0x5D, 0x93, 0x30,
    0x27, 0x82, 0x73, 0xC7, 0xDE, 0x31, 0xEF, 0xDC, 0x73, 0x10, 0xF7, 0x12,
    0x1F, 0xD5, 0xA0, 0x74, 0x15, 0x98, 0x7D, 0x9A, 0xDC, 0x0A, 0x48, 0x6D,
    0xCD, 0xF9, 0x3A, 0xCC, 0x44, 0x32, 0x83, 0x87, 0x31, 0x5D, 0x75, 0xE1,
    0x98, 0xC6, 0x41, 0xA4, 0x80, 0xCD, 0x86, 0xA1, 0xB9, 0xE5, 0x87, 0xE8,
    0xBE, 0x60, 0xE6, 0x9C, 0xC9, 0x28, 0xB2, 0xB9, 0xC5, 0x21, 0x72, 0xE4,
    0x13, 0x04, 0x2E, 0x9B, 0x23, 0xF1, 0x0B, 0x0E, 0x16, 0xE7, 0x97, 0x63,
    0xC9, 0xB5, 0x3D, 0xCF, 0x4B, 0xA8, 0x0A, 0x29, 0xE3, 0xFB, 0x73, 0xC1,
    0x6B, 0x8E, 0x75, 0xB9, 0x7E, 0xF3, 0x63, 0xE2, 0xFF, 0xA3, 0x1F, 0x71,
    0xCF, 0x9D, 0xE5, 0x38, 0x4E, 0x71, 0xB8, 0x1C, 0x0A, 0xC4, 0xDF, 0xFE,
    0x0C, 0x10, 0xE6, 0x4F
};

static const unsigned char rfc_5114_modp_2_g[] = {
    0xAC, 0x40, 0x32, 0xEF, 0x4F, 0x2D, 0x9A, 0xE3, 0x9D, 0xF3, 0x0B, 0x5C,
    0x8F, 0xFD, 0xAC, 0x50, 0x6C, 0xDE, 0xBE, 0x7B, 0x89, 0x99, 0x8C, 0xAF,
    0x74, 0x86, 0x6A, 0x08, 0xCF, 0xE4, 0xFF, 0xE3, 0xA6, 0x82, 0x4A, 0x4E,
    0x10, 0xB9, 0xA6, 0xF0, 0xDD, 0x92, 0x1F, 0x01, 0xA7, 0x0C, 0x4A, 0xFA,
    0xAB, 0x73, 0x9D, 0x77, 0x00, 0xC2, 0x9F, 0x52, 0xC5, 0x7D, 0xB1, 0x7C,
    0x62, 0x0A, 0x86, 0x52, 0xBE, 0x5E, 0x90, 0x01, 0xA8, 0xD6, 0x6A, 0xD7,
    0xC1, 0x76, 0x69, 0x10, 0x19, 0x99, 0x02, 0x4A, 0xF4, 0xD0, 0x27, 0x27,
    0x5A, 0xC1, 0x34, 0x8B, 0xB8, 0xA7, 0x62, 0xD0, 0x52, 0x1B, 0xC9, 0x8A,
    0xE2, 0x47, 0x15, 0x04, 0x22, 0xEA, 0x1E, 0xD4, 0x09, 0x93, 0x9D, 0x54,
    0xDA, 0x74, 0x60, 0xCD, 0xB5, 0xF6, 0xC6, 0xB2, 0x50, 0x71, 0x7C, 0xBE,
    0xF1, 0x80, 0xEB, 0x34, 0x11, 0x8E, 0x98, 0xD1, 0x19, 0x52, 0x9A, 0x45,
    0xD6, 0xF8, 0x34, 0x56, 0x6E, 0x30, 0x25, 0xE3, 0x16, 0xA3, 0x30, 0xEF,
    0xBB, 0x77, 0xA8, 0x6F, 0x0C, 0x1A, 0xB1, 0x5B, 0x05, 0x1A, 0xE3, 0xD4,
    0x28, 0xC8, 0xF8, 0xAC, 0xB7, 0x0A, 0x81, 0x37, 0x15, 0x0B, 0x8E, 0xEB,
    0x10, 0xE1, 0x83, 0xED, 0xD1, 0x99, 0x63, 0xDD, 0xD9, 0xE2, 0x63, 0xE4,
    0x77, 0x05, 0x89, 0xEF, 0x6A, 0xA2, 0x1E, 0x7F, 0x5F, 0x2F, 0xF3, 0x81,
    0xB5, 0x39, 0xCC, 0xE3, 0x40, 0x9D, 0x13, 0xCD, 0x56, 0x6A, 0xFB, 0xB4,
    0x8D, 0x6C, 0x01, 0x91, 0x81, 0xE1, 0xBC, 0xFE, 0x94, 0xB3, 0x02, 0x69,
    0xED, 0xFE, 0x72, 0xFE, 0x9B, 0x6A, 0xA4, 0xBD, 0x7B, 0x5A, 0x0F, 0x1C,
    0x71, 0xCF, 0xFF, 0x4C, 0x19, 0xC4, 0x18, 0xE1, 0xF6, 0xEC, 0x01, 0x79,
    0x81, 0xBC, 0x08, 0x7F, 0x2A, 0x70, 0x65, 0xB3, 0x84, 0xB8, 0x90, 0xD3,
    0x19, 0x1F, 0x2B, 0xFA
};

static const unsigned char rfc_5114_modp_2_q[] = {
    0x80, 0x1C, 0x0D, 0x34, 0xC5, 0x8D, 0x93, 0xFE, 0x99, 0x71, 0x77, 0x10,
    0x1F, 0x80, 0x53, 0x5A, 0x47, 0x38, 0xCE, 0xBC, 0xBF, 0x38, 0x9A, 0x99,
    0xB3, 0x63, 0x71, 0xEB
};

static const unsigned char rfc_5114_modp_3_p[] = {
    0x87, 0xA8, 0xE6, 0x1D, 0xB4, 0xB6, 0x66, 0x3C, 0xFF, 0xBB, 0xD1, 0x9C,
    0x65, 0x19, 0x59, 0x99, 0x8C, 0xEE, 0xF6, 0x08, 0x66, 0x0D, 0xD0, 0xF2,
    0x5D, 0x2C, 0xEE, 0xD4, 0x43, 0x5E, 0x3B, 0x00, 0xE0, 0x0D, 0xF8, 0xF1,
    0xD6, 0x19, 0x57, 0xD4, 0xFA, 0xF7, 0xDF, 0x45, 0x61, 0xB2, 0xAA, 0x30,
    0x16, 0xC3, 0xD9, 0x11, 0x34, 0x09, 0x6F, 0xAA, 0x3B, 0xF4, 0x29, 0x6D,
    0x83, 0x0E, 0x9A, 0x7C, 0x20, 0x9E, 0x0C, 0x64, 0x97, 0x51, 0x7A, 0xBD,
    0x5A, 0x8A, 0x9D, 0x30, 0x6B, 0xCF, 0x67, 0xED, 0x91, 0xF9, 0xE6, 0x72,
    0x5B, 0x47, 0x58, 0xC0, 0x22, 0xE0, 0xB1, 0xEF, 0x42, 0x75, 0xBF, 0x7B,
    0x6C, 0x5B, 0xFC, 0x11, 0xD4, 0x5F, 0x90, 0x88, 0xB9, 0x41, 0xF5, 0x4E,
    0xB1, 0xE5, 0x9B, 0xB8, 0xBC, 0x39, 0xA0, 0xBF, 0x12, 0x30, 0x7F, 0x5C,
    0x4F, 0xDB, 0x70, 0xC5, 0x81, 0xB2, 0x3F, 0x76, 0xB6, 0x3A, 0xCA, 0xE1,
    0xCA, 0xA6, 0xB7, 0x90, 0x2D, 0x52, 0x52, 0x67, 0x35, 0x48, 0x8A, 0x0E,
    0xF1, 0x3C, 0x6D, 0x9A, 0x51, 0xBF, 0xA4, 0xAB, 0x3A, 0xD8, 0x34, 0x77,
    0x96, 0x52, 0x4D, 0x8E, 0xF6, 0xA1, 0x67, 0xB5, 0xA4, 0x18, 0x25, 0xD9,
    0x67, 0xE1, 0x44, 0xE5, 0x14, 0x05, 0x64, 0x25, 0x1C, 0xCA, 0xCB, 0x83,
    0xE6, 0xB4, 0x86, 0xF6, 0xB3, 0xCA, 0x3F, 0x79, 0x71, 0x50, 0x60, 0x26,
    0xC0, 0xB8, 0x57, 0xF6, 0x89, 0x96, 0x28, 0x56, 0xDE, 0xD4, 0x01, 0x0A,
    0xBD, 0x0B, 0xE6, 0x21, 0xC3, 0xA3, 0x96, 0x0A, 0x54, 0xE7, 0x10, 0xC3,
    0x75, 0xF2, 0x63, 0x75, 0xD7, 0x01, 0x41, 0x03, 0xA4, 0xB5, 0x43, 0x30,
    0xC1, 0x98, 0xAF, 0x12, 0x61, 0x16, 0xD2, 0x27, 0x6E, 0x11, 0x71, 0x5F,
    0x69, 0x38, 0x77, 0xFA, 0xD7, 0xEF, 0x09, 0xCA, 0xDB, 0x09, 0x4A, 0xE9,
    0x1E, 0x1A, 0x15, 0x97
};

static const unsigned char rfc_5114_modp_3_g[] = {
    0x3F, 0xB3, 0x2C, 0x9B, 0x73, 0x13, 0x4D, 0x0B, 0x2E, 0x77, 0x50, 0x66,
    0x60, 0xED, 0xBD, 0x48, 0x4C, 0xA7, 0xB1, 0x8F, 0x21, 0xEF, 0x20, 0x54,
    0x07, 0xF4, 0x79, 0x3A, 0x1A, 0x0B, 0xA1, 0x25, 0x10, 0xDB, 0xC1, 0x50,
    0x77, 0xBE, 0x46, 0x3F, 0xFF, 0x4F, 0xED, 0x4A, 0xAC, 0x0B, 0xB5, 0x55,
    0xBE, 0x3A, 0x6C, 0x1B, 0x0C, 0x6B, 0x47, 0xB1, 0xBC, 0x37, 0x73, 0xBF,
    0x7E, 0x8C, 0x6F, 0x62, 0x90, 0x12, 0x28, 0xF8, 0xC2, 0x8C, 0xBB, 0x18,
    0xA5, 0x5A, 0xE3, 0x13, 0x41, 0x00, 0x0A, 0x65, 0x01, 0x96, 0xF9, 0x31,
    0xC7, 0x7A, 0x57, 0xF2, 0xDD, 0xF4, 0x63, 0xE5, 0xE9, 0xEC, 0x14, 0x4B,
    0x77, 0x7D, 0xE6, 0x2A, 0xAA, 0xB8, 0xA8, 0x62, 0x8A, 0xC3, 0x76, 0xD2,
    0x82, 0xD6, 0xED, 0x38, 0x64, 0xE6, 0x79, 0x82, 0x42, 0x8E, 0xBC, 0x83,
    0x1D, 0x14, 0x34, 0x8F, 0x6F, 0x2F, 0x91, 0x93, 0xB5, 0x04, 0x5A, 0xF2,
    0x76, 0x71, 0x64, 0xE1, 0xDF, 0xC9, 0x67, 0xC1, 0xFB, 0x3F, 0x2E, 0x55,
    0xA4, 0xBD, 0x1B, 0xFF, 0xE8, 0x3B, 0x9C, 0x80, 0xD0, 0x52, 0xB9, 0x85,
    0xD1, 0x82, 0xEA, 0x0A, 0xDB, 0x2A, 0x3B, 0x73, 0x13, 0xD3, 0xFE, 0x14,
    0xC8, 0x48, 0x4B, 0x1E, 0x05, 0x25, 0x88, 0xB9, 0xB7, 0xD2, 0xBB, 0xD2,
    0xDF, 0x01, 0x61, 0x99, 0xEC, 0xD0, 0x6E, 0x15, 0x57, 0xCD, 0x09, 0x15,
    0xB3, 0x35, 0x3B, 0xBB, 0x64, 0xE0, 0xEC, 0x37, 0x7F, 0xD0, 0x28, 0x37,
    0x0D, 0xF9, 0x2B, 0x52, 0xC7, 0x89, 0x14, 0x28, 0xCD, 0xC6, 0x7E, 0xB6,
    0x18, 0x4B, 0x52, 0x3D, 0x1D, 0xB2, 0x46, 0xC3, 0x2F, 0x63, 0x07, 0x84,
    0x90, 0xF0, 0x0E, 0xF8, 0xD6, 0x47, 0xD1, 0x48, 0xD4, 0x79, 0x54, 0x51,
    0x5E, 0x23, 0x27, 0xCF, 0xEF, 0x98, 0xC5, 0x82, 0x66, 0x4B, 0x4C, 0x0F,
    0x6C, 0xC4, 0x16, 0x59
};

static const unsigned char rfc_5114_modp_3_q[] = {
    0x8C, 0xF8, 0x36, 0x42, 0xA7, 0x09, 0xA0, 0x97, 0xB4, 0x47, 0x99, 0x76,
    0x40, 0x12, 0x9D, 0xA2, 0x99, 0xB1, 0xA4, 0x7D, 0x1E, 0xB3, 0x75, 0x0B,
    0xA3, 0x08, 0xB0, 0xFE, 0x64, 0xF5, 0xFB, 0xD3
};

/**
 * @brief Create a DH structure using the parameters from one of the MODP groups
 *         defined in RFC 5114.
 *
 * @param[in] num number of the group to be generated. The valid range is from
 *         one to three
 * @return new DH group or NULL in case of an error
 */
static DH *
get_rfc5114_modp(int num);
/**
 * @brief Public key validation method described in RFC 2631.
 *
 * Verify that DH->pub_key lies within the interval [2,p-1]. If it does not,
 * the key is invalid.
 * If DH->q exists, compute y^q mod p. If the result == 1, the key is valid.
 * Otherwise the key is invalid.
 *
 * @param[in] dh DH object to use
 * @param[in] ctx BN_CTX object
 * @param[out] ret Can contain these flags as result:
 * DH_CHECK_PUBKEY_TOO_SMALL (smaller than 2)
 * DH_CHECK_PUBKEY_TOO_LARGE (bigger than p-1)
 * DH_CHECK_PUBKEY_INVALID (y^q mod p != 1)
 *
 * @return 1 on success or 0 if an error occurred
 */
static int
DH_check_pub_key_rfc(const DH *dh, BN_CTX *ctx, int *ret);
#define DH_CHECK_PUBKEY_INVALID        0x04

int
init_dh(DH ** dh, int standardizedDomainParameters)
{
    int i;
    DH *tmp = NULL;

    check(dh, "Invalid arguments");

    if (!*dh) {
        tmp = get_rfc5114_modp(standardizedDomainParameters);
        if (!tmp)
            goto err;
    } else {
        /*Note: this could be something not matching standardizedDomainParameters */
        tmp = *dh;
    }

    if (!DH_check(tmp, &i))
        goto err;

    /* RFC 5114 parameters do not use safe primes and OpenSSL does not know
     * how to deal with generator other then 2 or 5. Therefore we have to
     * ignore some of the checks */
    i &= ~DH_CHECK_P_NOT_SAFE_PRIME;
    i &= ~DH_UNABLE_TO_CHECK_GENERATOR;

    check(!i, "Bad DH key");

    *dh = tmp;

    return 1;

err:
    if (tmp && !*dh) {
        DH_free(tmp);
    }

    return 0;
}

static int
DH_check_pub_key_rfc(const DH *dh, BN_CTX *ctx, int *ret)
{
    BIGNUM *bn = NULL;
    int ok = 0;
    const BIGNUM *pub_key, *p, *q, *g;

    check((dh && ret), "Invalid arguments");

    BN_CTX_start(ctx);

    DH_get0_key(dh, &pub_key, NULL);
    DH_get0_pqg(dh, &p, &q, &g);

    /* Verify that y lies within the interval [2,p-1]. */
    if (!DH_check_pub_key(dh, pub_key, ret))
        goto err;

    /* If the DH is conform to RFC 2631 it should have a non-NULL q.
     * Others (like the DHs generated from OpenSSL) might have a problem with
     * this check. */
    if (q) {
        /* Compute y^q mod p. If the result == 1, the key is valid. */
        bn = BN_CTX_get(ctx);
        if (!bn || !BN_mod_exp(bn, pub_key, q, p, ctx))
            goto err;
        if (!BN_is_one(bn))
            *ret |= DH_CHECK_PUBKEY_INVALID;
    }
    ok = 1;

err:
    BN_CTX_end(ctx);
    return ok;
}


BIGNUM *
DH_get_q(const DH *dh, BN_CTX *ctx)
{
    BIGNUM *q_new = NULL, *bn = NULL;
    int i;
    const BIGNUM *p, *q;

    check(dh, "Invalid arguments");

    DH_get0_pqg(dh, &p, &q, NULL);
    if (!q) {
        q_new = BN_new();
        bn = BN_dup(p);

        /* DH primes should be strong, based on a Sophie Germain prime q
         * p=(2*q)+1 or (p-1)/2=q */
        if (!q_new || !bn ||
                !BN_sub_word(bn, 1) ||
                !BN_rshift1(q_new, bn)) {
            goto err;
        }
    } else {
        q_new = BN_dup(q);
    }

    /* q should always be prime */
    i = BN_is_prime_ex(q_new, BN_prime_checks, ctx, NULL);
    if (i <= 0) {
       if (i == 0)
          log_err("Unable to get Sophie Germain prime");
       goto err;
    }

    return q_new;

err:
    if (bn)
        BN_clear_free(bn);
    if (q_new)
        BN_clear_free(q_new);

    return NULL;
}

BIGNUM *
DH_get_order(const DH *dh, BN_CTX *ctx)
{
    BIGNUM *order = NULL, *bn = NULL;
    const BIGNUM *p, *g;

    check(dh && ctx, "Invalid argument");

    BN_CTX_start(ctx);

    DH_get0_pqg(dh, &p, NULL, &g);

    /* suppose the order of g is q-1 */
    order = DH_get_q(dh, ctx);
    bn = BN_CTX_get(ctx);
    if (!bn || !order || !BN_sub_word(order, 1)
          || !BN_mod_exp(bn, g, order, p, ctx))
        goto err;

    if (BN_cmp(bn, BN_value_one()) != 0) {
        /* if bn != 1, then q-1 is not the order of g, but p-1 should be */
        if (!BN_sub(order, p, BN_value_one()) ||
              !BN_mod_exp(bn, g, order, p, ctx))
           goto err;
        check(BN_cmp(bn, BN_value_one()) == 0, "Unable to get order");
    }

    BN_CTX_end(ctx);

    return order;

err:
    if (order)
        BN_clear_free(order);
    BN_CTX_end(ctx);

    return NULL;
}

BUF_MEM *
dh_generate_key(EVP_PKEY *key, BN_CTX *bn_ctx)
{
    int suc;
    DH *dh = NULL;
    BUF_MEM *ret = NULL;
    const BIGNUM *pub_key;


    check(key, "Invalid arguments");

    dh = EVP_PKEY_get1_DH(key);
    if (!dh)
        goto err;

    if (!DH_generate_key(dh) || !DH_check_pub_key_rfc(dh, bn_ctx, &suc))
        goto err;

    if (suc)
        goto err;

    DH_get0_key(dh, &pub_key, NULL);

    ret = BN_bn2buf(pub_key);

err:
    if (dh)
        DH_free(dh);
    return ret;
}

BUF_MEM *
dh_compute_key(EVP_PKEY *key, const BUF_MEM * in, BN_CTX *bn_ctx)
{
    BUF_MEM * out = NULL;
    BIGNUM * bn = NULL;
    DH *dh = NULL;

    check(key && in, "Invalid arguments");

    dh = EVP_PKEY_get1_DH(key);
    if (!dh)
        return NULL;

    /* decode public key */
    bn = BN_bin2bn((unsigned char *) in->data, in->length, bn);
    if (!bn)
        goto err;

    out = BUF_MEM_create(DH_size(dh));
    if (!out)
        goto err;

    out->length = DH_compute_key((unsigned char *) out->data, bn, dh);
    if ((int) out->length < 0)
        goto err;

    BN_clear_free(bn);
    DH_free(dh);

    return out;

err:
    if (out)
        BUF_MEM_free(out);
    if (bn)
        BN_clear_free(bn);
    if (dh)
        DH_free(dh);

    return NULL;
}

static DH *
get_rfc5114_modp(int num)
{
    DH *ret = NULL;
    BIGNUM *p = NULL, *g = NULL, *q = NULL;
    int check = 0;

    ret = DH_new();
    if (!ret)
        goto err;

    switch(num) {
        case 0:
            p = BN_bin2bn(rfc_5114_modp_1_p, sizeof(rfc_5114_modp_1_p), p);
            g = BN_bin2bn(rfc_5114_modp_1_g, sizeof(rfc_5114_modp_1_g), g);
            q = BN_bin2bn(rfc_5114_modp_1_q, sizeof(rfc_5114_modp_1_q), q);
            break;
        case 1:
            p = BN_bin2bn(rfc_5114_modp_2_p, sizeof(rfc_5114_modp_2_p), p);
            g = BN_bin2bn(rfc_5114_modp_2_g, sizeof(rfc_5114_modp_2_g), g);
            q = BN_bin2bn(rfc_5114_modp_2_q, sizeof(rfc_5114_modp_2_q), q);
            break;
        case 2:
            p = BN_bin2bn(rfc_5114_modp_3_p, sizeof(rfc_5114_modp_3_p), p);
            g = BN_bin2bn(rfc_5114_modp_3_g, sizeof(rfc_5114_modp_3_g), g);
            q = BN_bin2bn(rfc_5114_modp_3_q, sizeof(rfc_5114_modp_3_q), q);
            break;
        default:
            log_err("Invalid arguments");
            goto err;
    }

    if (!p || !g || !q)
        goto err;

    if (!DH_set0_pqg(ret, p, q, g))
       goto err;

    /* Perform some checks. OpenSSL only knows generators 2 and 5, so the
     * DH_UNABLE_TO_CHECK_GENERATOR will be set, but the prime should be safe
     * nevertheless */
    if (!DH_check(ret, &check)) goto err;
    if (check & DH_CHECK_P_NOT_PRIME)
        goto err;

    return ret;

err:
    if (ret)
        DH_free(ret);
    if (p)
        BN_free(p);
    if (g)
        BN_free(g);
    if (q)
        BN_free(q);
    return NULL;
}

DH *
DHparams_dup_with_q(DH *dh)
{
    const BIGNUM *p, *q, *g;

    DH *dup = DHparams_dup(dh);
    DH_get0_pqg(dh, &p, &q, &g);
    DH_set0_pqg(dup, BN_dup(p), BN_dup(q), BN_dup(g));

    return dup;
}

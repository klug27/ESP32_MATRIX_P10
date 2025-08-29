#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>
#include "verify_crypto_license.h"

/* (Include same SHA256, HMAC and base64url implementation as in generator)
   For brevity we reuse the same functions: sha256_*, hmac_sha256, base64url_decode...
   I'll paste the minimal necessary pieces (adapted). */

/* ---------- SHA256/HMAC code: (same as generator) ---------- */
/* (Copy the sha256_ctx, sha256_* and hmac_sha256 functions here — identical to previous file) */

/* For brevity in this message, assume the functions sha256_init, sha256_update, sha256_final,
   hmac_sha256 exist and behave exactly like in the generator file. In your project, include
   both files or put the shared crypto functions in a common header/source. */


typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} sha256_ctx;



#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x)&(y)) ^ ((x)&(z)) ^ ((y)&(z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))


static const uint32_t k[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(sha256_ctx *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,t1,t2,m[64];
    int i;
    for (i=0;i<16;i++)
        m[i] = (data[i*4]<<24) | (data[i*4+1]<<16) | (data[i*4+2]<<8) | (data[i*4+3]);
    for (i=16;i<64;i++)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i=0;i<64;i++) {
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}


static void sha256_final(sha256_ctx *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;
    // pad
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    // append length
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i=0;i<4;i++) {
        for (int j=0;j<8;j++) {
            hash[i + j*4] = (ctx->state[j] >> (24 - i*8)) & 0x000000ff;
        }
    }
}

static void sha256_init(sha256_ctx *ctx) {
    ctx->datalen = 0; ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}


static void sha256_update(sha256_ctx *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0;i<len;i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}


/* ---------------------- HMAC-SHA256 ---------------------- */
static void hmac_sha256(const uint8_t *key, size_t keylen,
                        const uint8_t *msg, size_t msglen,
                        uint8_t out[32]) {
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];
    if (keylen > 64) {
        sha256_ctx tctx; sha256_init(&tctx); sha256_update(&tctx, key, keylen); sha256_final(&tctx, tk);
        key = tk; keylen = 32;
    }
    memset(k_ipad, 0x36, 64);
    memset(k_opad, 0x5c, 64);
    for (size_t i=0;i<keylen;i++) {
        k_ipad[i] ^= key[i] ^ 0x36; // initial 0x36 then xor key
        k_opad[i] ^= key[i] ^ 0x5c;
    }
    // inner
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, msg, msglen);
    uint8_t ihash[32];
    sha256_final(&ctx, ihash);
    // outer
    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, ihash, 32);
    sha256_final(&ctx, out);
}


/* ---------- Base64URL decode (reverse of encode) ---------- */
static int base64url_char_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '-') return 62; // plus -> '-'
    if (c == '_') return 63; // slash -> '_'
    return -1;
}

static int base64url_decode_to_buf(const char *in, uint8_t *out, size_t *outlen) {
    size_t inlen = strlen(in);
    size_t i = 0, j = 0;
    uint32_t buffer = 0;
    int bits_collected = 0;
    while (i < inlen) {
        int val = base64url_char_val(in[i++]);
        if (val < 0) return -1;
        buffer = (buffer << 6) | (uint32_t)val;
        bits_collected += 6;
        if (bits_collected >= 8) {
            bits_collected -= 8;
            if (j >= *outlen) return -1;
            out[j++] = (buffer >> bits_collected) & 0xFF;
        }
    }
    *outlen = j;
    return 0;
}

/* constant-time compare */
static int consttime_memcmp(const uint8_t *a, const uint8_t *b, size_t n) {
    uint8_t r = 0;
    for (size_t i=0;i<n;i++) r |= a[i] ^ b[i];
    return r; // 0 if equal
}

/* simple parser for payload format ver=1|mac=...|uid=...|issued=...|expiry=... */
static int extract_field(const char *payload, const char *key, char *out, size_t out_sz) {
    size_t keylen = strlen(key);
    const char *p = payload;
    while (*p) {
        if (strncmp(p, key, keylen) == 0 && p[keylen] == '=') {
            const char *val = p + keylen + 1;
            const char *end = strchr(val, '|');
            if (!end) end = val + strlen(val);
            size_t len = end - val;
            if (len >= out_sz) return 0;
            memcpy(out, val, len);
            out[len] = '\0';
            return 1;
        }
        const char *next = strchr(p, '|');
        if (!next) break;
        p = next + 1;
    }
    return 0;
}

//--------------------------------------------------------------
/* --- Implémentation portable de timegm() --- */
static int is_leap_year(int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static int days_before_month(int year, int month) {
    static const int days[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int d = days[month];
    if (month > 1 && is_leap_year(year)) {
        d += 1;
    }
    return d;
}

static time_t my_timegm(const struct tm *tm) {
    int year = tm->tm_year + 1900; // struct tm: années depuis 1900
    int month = tm->tm_mon;        // 0..11
    int day = tm->tm_mday - 1;     // 0..30

    int64_t days = 0;
    for (int y = 1970; y < year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }
    days += days_before_month(year, month);
    days += day;

    int64_t seconds = days * 86400LL
                    + tm->tm_hour * 3600LL
                    + tm->tm_min  * 60LL
                    + tm->tm_sec;

    return (time_t)seconds;
}

/* --- Validation stricte du format ISO8601 --- */
static int validate_iso8601_utc(const char *s) {
    if (!s) return 0;
    if (strlen(s) != 20) return 0; // Format exact attendu

    // Positions fixes : YYYY-MM-DDTHH:MM:SSZ
    for (int i = 0; i < 4; i++) if (!isdigit((unsigned char)s[i])) return 0; // YYYY
    if (s[4] != '-') return 0;
    if (!isdigit((unsigned char)s[5]) || !isdigit((unsigned char)s[6])) return 0; // MM
    if (s[7] != '-') return 0;
    if (!isdigit((unsigned char)s[8]) || !isdigit((unsigned char)s[9])) return 0; // DD
    if (s[10] != 'T') return 0;
    if (!isdigit((unsigned char)s[11]) || !isdigit((unsigned char)s[12])) return 0; // HH
    if (s[13] != ':') return 0;
    if (!isdigit((unsigned char)s[14]) || !isdigit((unsigned char)s[15])) return 0; // mm
    if (s[16] != ':') return 0;
    if (!isdigit((unsigned char)s[17]) || !isdigit((unsigned char)s[18])) return 0; // SS
    if (s[19] != 'Z') return 0;

    return 1;
}
//--------------------------------------------------------------

/* parse ISO8601 time YYYY-MM-DDTHH:MM:SSZ into time_t (UTC) */
static int parse_iso8601_utc(const char *s, time_t *out) {
    if (!s) return 0;
    if (!validate_iso8601_utc(s)) return 0;

    struct tm tm = {0};
    char buf[5];
    char b2[3];

    // initialization of time
    memset(&tm, 0, sizeof(tm));

    // year
    memcpy(buf, s, 4); buf[4] = '\0';
    tm.tm_year = atoi(buf) - 1900;

    // Month
    memcpy(b2, s + 5, 2); b2[2] = '\0';
    tm.tm_mon = atoi(b2) - 1;
    if (tm.tm_mon < 0 || tm.tm_mon > 11) return 0;

    // Date
    memcpy(b2, s + 8, 2); b2[2] = '\0';
    tm.tm_mday = atoi(b2);
    if (tm.tm_mday < 1 || tm.tm_mday > 31) return 0;

    // Hour
    memcpy(b2, s + 11, 2); b2[2] = '\0';
    tm.tm_hour = atoi(b2);
    if (tm.tm_hour < 0 || tm.tm_hour > 23) return 0;

    // Minutes
    memcpy(b2, s + 14, 2); b2[2] = '\0';
    tm.tm_min = atoi(b2);
    if (tm.tm_min < 0 || tm.tm_min > 59) return 0;

    // Seconds
    memcpy(b2, s + 17, 2); b2[2] = '\0';
    tm.tm_sec = atoi(b2);
    if (tm.tm_sec < 0 || tm.tm_sec > 59) return 0;

    tm.tm_isdst = 0; // UTC

    #if defined(_WIN32)
        *out = _mkgmtime(&tm);
    #elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
        *out = timegm(&tm);
    #elif defined(ESP32) || defined(ARDUINO_ARCH_ARM) || defined(__arm__)
        *out = my_timegm(&tm);
    #else
        *out = my_timegm(&tm);
    #endif

    return 1;
}

/* Shared secret: MUST MATCH server secret (or derive per device) */
static const uint8_t SECRET_KEY[] = {
    0x3f,0x92,0xa1,0x7c,0x5b,0x21,0x4d,0x90,0xab,0xcd,0xef,0x01,0x23,0x45,0x67,0x89,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00
};
static const size_t SECRET_KEY_LEN = 32;

int verify_token(const char *token, const char *local_mac, const char *local_uid) {
    // split token into payload_b64 and sig_b64
    const char *dot = strchr(token, '.');
    if (!dot) return 0;
    size_t p_b64_len = dot - token;
    char *p_b64 = (char *)malloc(p_b64_len + 1);
    if (!p_b64) return 0;
    memcpy(p_b64, token, p_b64_len); p_b64[p_b64_len]=0;
    const char *s_b64 = dot + 1;

    // decode payload
    uint8_t payload_buf[512]; size_t payload_len = sizeof(payload_buf);
    if (base64url_decode_to_buf(p_b64, payload_buf, &payload_len) != 0) { free(p_b64); return 0; }
    free(p_b64);
    // decode signature
    uint8_t sig_buf[64]; size_t sig_len = sizeof(sig_buf);
    if (base64url_decode_to_buf(s_b64, sig_buf, &sig_len) != 0) return 0;
    // compute HMAC
    uint8_t expected_sig[32];
    hmac_sha256(SECRET_KEY, SECRET_KEY_LEN, payload_buf, payload_len, expected_sig);
    if (sig_len != 32) return 0;
    if (consttime_memcmp(expected_sig, sig_buf, 32) != 0) return 0;

    // signature ok -> parse payload string
    char payload_str[513];
    if (payload_len >= sizeof(payload_str)) return 0;
    memcpy(payload_str, payload_buf, payload_len); payload_str[payload_len]=0;

    char mac_in[64], uid_in[64], expiry[64];
    if (!extract_field(payload_str, "mac", mac_in, sizeof(mac_in))) return 0;
    if (!extract_field(payload_str, "uid", uid_in, sizeof(uid_in))) return 0;
    if (!extract_field(payload_str, "expiry", expiry, sizeof(expiry))) expiry[0]=0;

    // compare identifiers (case-insensitive for mac)
    // normalize MAC to upper/lower? simple case-insensitive compare:
    if (strcasecmp(mac_in, local_mac) != 0) return 0;
    if (strcmp(uid_in, local_uid) != 0) return 0;

    // check expiry if present
    if (expiry[0]) {
        time_t exp_t;
        if (!parse_iso8601_utc(expiry, &exp_t)) return 0;
        time_t now = time(NULL);
        if (now > exp_t) return 0; // expired
    }
    return 1;
}


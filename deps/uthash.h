/*
Copyright (c) 2003-2006, Troy Hanson     http://uthash.sourceforge.net
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of the copyright holder nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h> /* memcmp,strlen */

#ifndef UTHASH_H
#define UTHASH_H 

#define uthash_fatal(msg) exit(-1)        /* fatal error (out of memory,etc) */
#define uthash_bkt_malloc(sz) malloc(sz)  /* malloc fcn for UT_hash_bucket's */
#define uthash_bkt_free(ptr) free(ptr)    /* free fcn for UT_hash_bucket's   */
#define uthash_tbl_malloc(sz) malloc(sz)  /* malloc fcn for UT_hash_table    */
#define uthash_tbl_free(ptr) free(ptr)    /* free fcn for UT_hash_table      */

#define uthash_noexpand_fyi(tbl)          /* can be defined to log noexpand  */
#define uthash_expand_fyi(tbl)            /* can be defined to log expands   */

/* initial number of buckets */
#define HASH_INITIAL_NUM_BUCKETS 32  /* initial number of buckets        */
#define HASH_BKT_CAPACITY_THRESH 10  /* expand when bucket count reaches */

#define HASH_FIND(hh,head,keyptr,keylen_in,out)                          \
do {                                                                     \
  out=head;                                                              \
  if (head) {                                                            \
     (head)->hh.tbl->key = (char*)(keyptr);                              \
     (head)->hh.tbl->keylen = keylen_in;                                 \
     HASH_FCN((head)->hh.tbl->key,(head)->hh.tbl->keylen,                \
             (head)->hh.tbl->num_buckets,(head)->hh.tbl->bkt,            \
             (head)->hh.tbl->i, (head)->hh.tbl->j,(head)->hh.tbl->k);    \
     HASH_FIND_IN_BKT(hh, (head)->hh.tbl->buckets[ (head)->hh.tbl->bkt], \
             keyptr,keylen_in,out);                                      \
  }                                                                      \
} while (0)

#define HASH_ADD(hh,head,fieldname,keylen_in,add)                        \
        HASH_ADD_KEYPTR(hh,head,&add->fieldname,keylen_in,add)
 
#define HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add)                    \
do {                                                                     \
 add->hh.next = NULL;                                                    \
 add->hh.key = (char*)keyptr;                                            \
 add->hh.keylen = keylen_in;                                             \
 add->hh.elmt = add;                                                     \
 if (!(head)) {                                                          \
    head = add;                                                          \
    (head)->hh.prev = NULL;                                              \
    (head)->hh.tbl = (UT_hash_table*)uthash_tbl_malloc(                  \
                    sizeof(UT_hash_table));                              \
    if (!((head)->hh.tbl))  { uthash_fatal( "out of memory"); }          \
    (head)->hh.tbl->name = #head;                                        \
    (head)->hh.tbl->tail = &(add->hh);                                   \
    (head)->hh.tbl->noexpand = 0;                                        \
    (head)->hh.tbl->hash_q = 1<<16;                                          \
    (head)->hh.tbl->num_buckets = HASH_INITIAL_NUM_BUCKETS;              \
    (head)->hh.tbl->num_items = 0;                                       \
    (head)->hh.tbl->hho = ((long)(&add->hh) - (long)(add));              \
    (head)->hh.tbl->buckets = (UT_hash_bucket*)uthash_bkt_malloc(        \
            HASH_INITIAL_NUM_BUCKETS*sizeof(struct UT_hash_bucket));     \
    if (! (head)->hh.tbl->buckets) { uthash_fatal( "out of memory"); }   \
    memset((head)->hh.tbl->buckets, 0,                                   \
            HASH_INITIAL_NUM_BUCKETS*sizeof(struct UT_hash_bucket));     \
 } else {                                                                \
    (head)->hh.tbl->tail->next = add;                                    \
    add->hh.prev = (head)->hh.tbl->tail->elmt;                           \
    (head)->hh.tbl->tail = &(add->hh);                                   \
 }                                                                       \
 (head)->hh.tbl->num_items++;                                            \
 add->hh.tbl = (head)->hh.tbl;                                           \
 (head)->hh.tbl->key = (char*)keyptr;                                    \
 (head)->hh.tbl->keylen = keylen_in;                                     \
 HASH_FCN((head)->hh.tbl->key,(head)->hh.tbl->keylen,                    \
         (head)->hh.tbl->num_buckets,                                    \
         (head)->hh.tbl->bkt,                                            \
         (head)->hh.tbl->i, (head)->hh.tbl->j, (head)->hh.tbl->k );      \
 HASH_ADD_TO_BKT(hh,(head)->hh.tbl->buckets[(head)->hh.tbl->bkt],add);   \
 HASH_EMIT_KEY(hh,head,keyptr,keylen_in);                                \
 HASH_FSCK(head);                                                        \
} while(0)

#define HASH_DELETE(hh,head,delptr)                                      \
do {                                                                     \
    if ( ((delptr)->hh.prev == NULL) && ((delptr)->hh.next == NULL) )  { \
        uthash_bkt_free((head)->hh.tbl->buckets );                       \
        uthash_tbl_free((head)->hh.tbl);                                 \
        head = NULL;                                                     \
    } else {                                                             \
        if ((delptr) == (head)->hh.tbl->tail->elmt) {                    \
            (head)->hh.tbl->tail = (void*)(((long)((delptr)->hh.prev)) + \
                                           (head)->hh.tbl->hho);         \
        }                                                                \
        if ((delptr)->hh.prev) {                                         \
            ((UT_hash_handle*)(((long)((delptr)->hh.prev)) +             \
                    (head)->hh.tbl->hho))->next = (delptr)->hh.next;     \
        } else {                                                         \
            head = (delptr)->hh.next;                                    \
        }                                                                \
        if ((delptr)->hh.next) {                                         \
            ((UT_hash_handle*)(((long)((delptr)->hh.next)) +             \
                    (head)->hh.tbl->hho))->prev = (delptr)->hh.prev;     \
        }                                                                \
        (head)->hh.tbl->key = (char*)((delptr)->hh.key);                 \
        (head)->hh.tbl->keylen = (delptr)->hh.keylen;                    \
        HASH_FCN((head)->hh.tbl->key,(head)->hh.tbl->keylen,             \
                (head)->hh.tbl->num_buckets,(head)->hh.tbl->bkt,         \
                (head)->hh.tbl->i,(head)->hh.tbl->j,(head)->hh.tbl->k ); \
        HASH_DEL_IN_BKT(hh,(head)->hh.tbl->buckets[(head)->hh.tbl->bkt], \
                delptr);                                                 \
        (head)->hh.tbl->num_items--;                                     \
    }                                                                    \
    HASH_FSCK(head);                                                     \
} while (0)


/* convenience forms of HASH_FIND/HASH_ADD/HASH_DEL */
#define HASH_FIND_STR(head,findstr,out)                                 \
    HASH_FIND(hh,head,findstr,strlen(findstr),out)
#define HASH_ADD_STR(head,strfield,add)                                 \
    HASH_ADD(hh,head,strfield,strlen(add->strfield),add)
#define HASH_FIND_INT(head,findint,out)                                 \
    HASH_FIND(hh,head,findint,sizeof(int),out)
#define HASH_ADD_INT(head,intfield,add)                                 \
    HASH_ADD(hh,head,intfield,sizeof(int),add)
#define HASH_DEL(head,delptr)                                           \
    HASH_DELETE(hh,head,delptr)

/* HASH_FSCK checks hash integrity on every add/delete when HASH_DEBUG is defined.
 * This is for uthash developer only; it compiles away if HASH_DEBUG isn't defined.
 * This function misuses fields in UT_hash_table for its bookkeeping variables.
 */
#ifdef HASH_DEBUG
#define HASH_OOPS(...) do { fprintf(stderr,__VA_ARGS__); exit(-1); } while (0)
#define HASH_FSCK(head)                                                  \
do {                                                                     \
    if (head) {                                                          \
        (head)->hh.tbl->keylen = 0;   /* item counter */                 \
        for(    (head)->hh.tbl->bkt_i = 0;                               \
                (head)->hh.tbl->bkt_i < (head)->hh.tbl->num_buckets;     \
                (head)->hh.tbl->bkt_i++)                                 \
        {                                                                \
            (head)->hh.tbl->bkt_ideal = 0; /* bkt item counter */        \
            (head)->hh.tbl->hh =                                         \
            (head)->hh.tbl->buckets[(head)->hh.tbl->bkt_i].hh_head;      \
            (head)->hh.tbl->key = NULL;  /* hh_prev */                   \
            while ((head)->hh.tbl->hh) {                                 \
               if ((head)->hh.tbl->key !=                                \
                   (char*)((head)->hh.tbl->hh->hh_prev)) {               \
                   HASH_OOPS("invalid hh_prev %x, actual %x\n",          \
                    (head)->hh.tbl->hh->hh_prev,                         \
                    (head)->hh.tbl->key );                               \
               }                                                         \
               (head)->hh.tbl->bkt_ideal++;                              \
               (head)->hh.tbl->key = (char*)((head)->hh.tbl->hh);        \
               (head)->hh.tbl->hh = (head)->hh.tbl->hh->hh_next;         \
            }                                                            \
            (head)->hh.tbl->keylen +=  (head)->hh.tbl->bkt_ideal;        \
            if ((head)->hh.tbl->buckets[(head)->hh.tbl->bkt_i].count     \
               !=  (head)->hh.tbl->bkt_ideal) {                          \
               HASH_OOPS("invalid bucket count %d, actual %d\n",         \
                (head)->hh.tbl->buckets[(head)->hh.tbl->bkt_i].count,    \
                (head)->hh.tbl->bkt_ideal);                              \
            }                                                            \
        }                                                                \
        if ((head)->hh.tbl->keylen != (head)->hh.tbl->num_items) {       \
            HASH_OOPS("invalid hh item count %d, actual %d\n",           \
                (head)->hh.tbl->num_items, (head)->hh.tbl->keylen );     \
        }                                                                \
        /* traverse hh in app order; check next/prev integrity, count */ \
        (head)->hh.tbl->keylen = 0;   /* item counter */                 \
        (head)->hh.tbl->key = NULL;  /* app prev */                      \
        (head)->hh.tbl->hh =  &(head)->hh;                               \
        while ((head)->hh.tbl->hh) {                                     \
           (head)->hh.tbl->keylen++;                                     \
           if ((head)->hh.tbl->key !=(char*)((head)->hh.tbl->hh->prev)) {\
              HASH_OOPS("invalid prev %x, actual %x\n",                  \
                    (head)->hh.tbl->hh->prev,                            \
                    (head)->hh.tbl->key );                               \
           }                                                             \
           (head)->hh.tbl->key = (head)->hh.tbl->hh->elmt;               \
           (head)->hh.tbl->hh = ( (head)->hh.tbl->hh->next ?             \
             (UT_hash_handle*)((long)((head)->hh.tbl->hh->next) +        \
                               (head)->hh.tbl->hho)                      \
                                 : NULL );                               \
        }                                                                \
        if ((head)->hh.tbl->keylen != (head)->hh.tbl->num_items) {       \
            HASH_OOPS("invalid app item count %d, actual %d\n",          \
                (head)->hh.tbl->num_items, (head)->hh.tbl->keylen );     \
        }                                                                \
    }                                                                    \
} while (0)
#else
#define HASH_FSCK(head) 
#endif

/* When compiled with -DHASH_EMIT_KEYS, length-prefixed keys are emitted to 
 * the descriptor to which this macro is defined for tuning the hash function.
 * The app can #include <unistd.h> to get the prototype for write(2). */
#ifdef HASH_EMIT_KEYS
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                           \
    (head)->hh.tbl->keylen = fieldlen;                                   \
    write(HASH_EMIT_KEYS, &((head)->hh.tbl->keylen), sizeof(int));       \
    write(HASH_EMIT_KEYS, keyptr, fieldlen);          
#else 
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                    
#endif

/* default to Jenkins unless specified e.g. DHASH_FUNCTION=HASH_SAX */
#ifdef HASH_FUNCTION 
#define HASH_FCN HASH_FUNCTION
#else
#define HASH_FCN HASH_JEN
#endif

/* The Bernstein hash function, used in Perl prior to v5.6 */
#define HASH_BER(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 0;                                                             \
  while (keylen--)  bkt = (bkt * 33) + *key++;                         \
  bkt &= (num_bkts-1);          


/* SAX/FNV/OAT/JEN/JSW hash functions are macro variants of those listed at 
 * http://eternallyconfuzzled.com/tuts/hashing.html (thanks Julienne Walker) */
#define HASH_SAX(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 0;                                                             \
  for(i=0; i < keylen; i++)                                            \
      bkt ^= (bkt << 5) + (bkt >> 2) + key[i];                         \
  bkt &= (num_bkts-1);          

#define HASH_FNV(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 2166136261UL;                                                  \
  for(i=0; i < keylen; i++)                                            \
      bkt = (bkt * 16777619) ^ key[i];                                 \
  bkt &= (num_bkts-1);
 
#define HASH_OAT(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 0;                                                             \
  for(i=0; i < keylen; i++) {                                          \
      bkt += key[i];                                                   \
      bkt += (bkt << 10);                                              \
      bkt ^= (bkt >> 6);                                               \
  }                                                                    \
  bkt += (bkt << 3);                                                   \
  bkt ^= (bkt >> 11);                                                  \
  bkt += (bkt << 15);                                                  \
  bkt &= (num_bkts-1);

#define HASH_JEN_MIX(a,b,c)                                            \
{                                                                      \
  a -= b; a -= c; a ^= ( c >> 13 );                                    \
  b -= c; b -= a; b ^= ( a << 8 );                                     \
  c -= a; c -= b; c ^= ( b >> 13 );                                    \
  a -= b; a -= c; a ^= ( c >> 12 );                                    \
  b -= c; b -= a; b ^= ( a << 16 );                                    \
  c -= a; c -= b; c ^= ( b >> 5 );                                     \
  a -= b; a -= c; a ^= ( c >> 3 );                                     \
  b -= c; b -= a; b ^= ( a << 10 );                                    \
  c -= a; c -= b; c ^= ( b >> 15 );                                    \
}

#define HASH_JEN(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 0xfeedbeef;                                                    \
  i = j = 0x9e3779b9;                                                  \
  k = keylen;                                                          \
  while (k >= 12) {                                                    \
     i += (key[0] + ( (unsigned)key[1] << 8 )                          \
       + ( (unsigned)key[2] << 16 )                                    \
       + ( (unsigned)key[3] << 24 ) );                                 \
     j += (key[4] + ( (unsigned)key[5] << 8 )                          \
       + ( (unsigned)key[6] << 16 )                                    \
       + ( (unsigned)key[7] << 24 ) );                                 \
   bkt += (key[8] + ( (unsigned)key[9] << 8 )                          \
       + ( (unsigned)key[10] << 16 )                                   \
       + ( (unsigned)key[11] << 24 ) );                                \
                                                                       \
     HASH_JEN_MIX(i, j, bkt);                                          \
                                                                       \
     key += 12;                                                        \
     k -= 12;                                                          \
  }                                                                    \
  bkt += keylen;                                                       \
  switch ( k ) {                                                       \
     case 11: bkt += ( (unsigned)key[10] << 24 );                      \
     case 10: bkt += ( (unsigned)key[9] << 16 );                       \
     case 9:  bkt += ( (unsigned)key[8] << 8 );                        \
     case 8:  j += ( (unsigned)key[7] << 24 );                         \
     case 7:  j += ( (unsigned)key[6] << 16 );                         \
     case 6:  j += ( (unsigned)key[5] << 8 );                          \
     case 5:  j += key[4];                                             \
     case 4:  i += ( (unsigned)key[3] << 24 );                         \
     case 3:  i += ( (unsigned)key[2] << 16 );                         \
     case 2:  i += ( (unsigned)key[1] << 8 );                          \
     case 1:  i += key[0];                                             \
  }                                                                    \
  HASH_JEN_MIX(i, j, bkt);                                             \
  bkt &= (num_bkts-1);

#define HASH_JSW(key,keylen,num_bkts,bkt,i,j,k)                        \
  bkt = 16777551;                                                      \
  for(i=0; i < keylen; i++) {                                          \
      bkt = (bkt << 1 | bkt >> 31) ^                                   \
      *(int*)((long)(                                                  \
      "\xe9\x81\x51\xe4\x84\x9d\x32\xd9\x2d\xda\xca\x94\xa7\x85\x1e"   \
      "\x28\xfe\xa3\x18\x60\x28\x45\xa6\x48\x67\xdb\xd5\xa2\x91\x4d"   \
      "\x1a\x2f\x97\x37\x82\xd8\xe9\x1c\xb7\x7b\x3c\xa5\x4c\x23\x2"    \
      "\x42\x85\x20\x78\x6c\x6\x67\x6f\xa5\xcb\x53\x8c\xe1\x1f\x12"    \
      "\x66\xcb\xa0\xbe\x47\x59\x8\x20\xd5\x31\xd9\xdc\xcc\x27\xc3"    \
      "\x4d\x8\x9f\xb3\x50\x8\x90\x4f\x1f\x20\x60\xb8\xe2\x7b\x63"     \
      "\x49\xc0\x64\xc7\xaf\xc9\x81\x9c\x5f\x7d\x45\xc5\xe4\xe4\x86"   \
      "\xaf\x1a\x15\x6c\x9b\xc3\x7c\xc5\x88\x2b\xf3\xd9\x72\x76\x47"   \
      "\x56\xe6\x8c\xd1\x6c\x94\x41\x59\x4d\xe2\xd7\x44\x9a\x55\x5e"   \
      "\xee\x9d\x7c\x8f\x21\x57\x10\x77\xf7\x4b\xd8\x7e\xc0\x4d\xba"   \
      "\x1f\x96\x2a\x60\x13\xae\xab\x58\x70\xe5\x23\x62\x2b\x63\xb6"   \
      "\x42\x8e\x8f\x57\xf2\xfa\x47\x37\x91\xac\x11\x3d\x9a\x85\x73"   \
      "\x9e\x39\x65\xc8\xd4\x5b\xaa\x35\x72\x5f\x40\x31\x9a\xb0\xdd"   \
      "\xa9\x2c\x16\xa3\x32\xef\xcb\x8c\x80\x33\x60\xd\x85\xce\x22"    \
      "\x8c\x28\x6\x7f\xff\xf6\x8a\x5f\x21\x8e\xf2\xd0\xd9\x63\x66"    \
      "\x22\xe8\xe6\x3\x39\xfd\x10\x69\xce\x6c\xc4\xde\xf3\x87\x56"    \
      "\xc8\x4a\x31\x51\x58\xc5\x62\x30\x8e\xd\xd5\x2f\x7c\x24\xca"    \
      "\xd1\x12\x1b\x3a\x3e\x95\x99\xa\x7\xc1\x83\xd0\x4f\x97\x8c"     \
      "\xf1\xb0\x9c\xd8\xb9\x72\xd7\x3e\x6b\x66\x83\x8e\xe9\x86\xad"   \
      "\xfa\xc2\xe\x4\xb5\x7b\x5d\x0\xbc\x47\xbe\x4\x69\xfa\xd1"       \
      "\x29\x5c\x77\x38\xfc\x88\xeb\xd5\xe1\x17\x54\xf6\xe5\xb3\xae"   \
      "\xc7\x14\xb6\x4b\xa6\x42\x4b\xa3\xdf\xa5\xcf\xdb\xad\xcd\x2c"   \
      "\xa3\x3\x13\xc0\x42\x5d\x6e\x3c\xfe\xd8\xeb\xa7\x96\x47\x2b"    \
      "\x61\xb3\x70\xc9\x6d\xff\x1a\x82\x65\xdc\x92\x4b\x1a\x52\x75"   \
      "\xa5\x61\x55\x2b\xe\x7\xde\x1e\x71\xc5\x12\x34\x59\x4f\x19"     \
      "\x2\x9\xb6\x5\xe6\x7b\xad\xb6\x92\xfb\x84\x32\xf1\x45\x6c"      \
      "\xec\x1a\xcb\x39\x32\x2\x47\x51\xd6\xc8\x9d\xd0\xb1\xdb\xa8"    \
      "\x90\x4c\x65\x5a\x77\x1f\xca\x74\x8e\x3b\xce\x76\x55\x8b\x78"   \
      "\x3c\xf3\x19\x8f\xe1\xc3\xa9\x8a\xc8\xf3\x14\x30\x4e\x77\xe9"   \
      "\xd5\x6a\xcb\x96\x2f\x31\x35\xff\x6b\x10\x92\xf7\xc4\x33\xb8"   \
      "\x76\x35\x6\xf\x82\x1c\xfa\x1f\x92\x47\xa1\xf9\x7e\xe5\x51"     \
      "\xee\x63\xaa\x9a\x38\xa3\xa1\x86\xbf\xf0\xe8\x29\xe1\x19\x83"   \
      "\xff\x36\x3c\x26\x15\x89\x36\x22\x93\x41\x3e\x63\x36\x34\x4c"   \
      "\xda\x18\xd4\x18\xd8\xc8\x8a\x10\x1f\x14\x4c\x7f\x79\xfc\x46"   \
      "\xbb\xc8\x24\x51\xc7\xe4\xfb\xc0\x78\xb1\xe9\xac\xf1\x3d\x55"   \
      "\x51\x9c\x8\xf0\xa6\x3\xcb\x91\xc6\xf4\xe2\xd4\xe5\x18\x61"     \
      "\xfc\x8f\x8a\xce\x89\x33\xcd\xf\x7d\x50\xa0\x7d\x3f\xac\x49"    \
      "\xe1\x71\x92\xc7\x8d\xc0\xd0\x6e\xe4\xf7\xcd\xc1\x47\x9f\x99"   \
      "\xd5\x7\x20\xad\x64\xdb\xab\x44\xd4\x8\xc6\x9a\xa4\xa7\x7c"     \
      "\x9b\x13\xe4\x9c\x88\xec\xc4\xcb\xe1\x3f\x5\x5\xf\xd\x3a"       \
      "\x75\xed\xfa\xc0\x23\x34\x74\xfd\xca\x1c\x74\x77\x29\xc8\xb6"   \
      "\xe2\xbb\xa1\xa\x2e\xae\x65\x3e\xcb\xf5\x5e\xe0\x29\x4c\xfa"    \
      "\xab\x35\xea\x7\x9f\xb3\x3b\x9c\x4e\x86\xe8\x5b\x76\x11\xf1"    \
      "\xbf\x7f\x73\x34\x71\x9\x2d\x2a\x60\x8f\x14\x12\xba\x26\x84"    \
      "\xb9\x94\xa9\x59\x38\x25\xfd\x77\xc3\xe5\x86\xc4\x3\xda\x32"    \
      "\x30\xd8\x84\x81\x83\x14\x8c\x24\xee\x51\xa9\x92\x61\xb2\xeb"   \
      "\xce\xac\x34\xc1\xad\x24\x74\xce\xf9\xce\x5c\xfd\x45\x69\x1d"   \
      "\xc6\xc2\xaf\x7c\x8d\x5\x52\xb5\x88\x2f\x9f\xee\x6b\x5f\xbd"    \
      "\xfe\x22\x6\x47\xa2\xc8\x25\x37\x67\x44\x4c\xe\xfe\x7e\x5a"     \
      "\x36\x7f\x18\x83\x8f\x82\x87\x3b\xbf\xb8\xd2\x37\xff\x52\x60"   \
      "\xb5\xf3\xd\x20\x80\xcc\xb2\x7a\xdd\xc2\x94\xbc\xe3\xb1\x87"    \
      "\x3e\x49\x57\xcc\xe9\x5a\xea\xb4\xe\xdf\xa6\x8f\x70\x60\x32"    \
      "\xb\x7d\x74\xf5\x46\xb6\x93\xc2\x5\x92\x72\xfc\xd9\xd2\xe5"     \
      "\x90\x36\x2a\xd4\xf9\x50\x33\x52\xa5\xcc\xcf\x14\x9e\xdc\x4f"   \
      "\xb7\x7d\xcf\x25\xdb\xc0\x46\xdb\xea\xe\x27\xc8\x18\x40\x39"    \
      "\xbd\xec\x48\xa3\xfa\x87\xa3\x18\x68\xfc\x7a\x44\xa8\xc5\x8c"   \
      "\x45\x81\x70\x72\x14\x70\xf9\x40\xc8\xe7\x41\xcb\xde\xd\x4e"    \
      "\x35\x4d\xcd\xe2\x40\xa3\x2e\xbb\xb7\x50\x6c\x26\xb8\xbe\x2a"   \
      "\x36\x8e\x23\xb\xa\xfe\xed\xa\xe7\xa0\x16\x73\xad\x24\x51"      \
      "\x7f\xda\x9d\xd7\x9f\x18\xe6\xa8\xe4\x98\xbc\x62\x77\x55\x60"   \
      "\x88\x16\x25\xbf\x95\xad\xea\xe1\x87\x18\x35\x9e\x7c\x51\xee"   \
      "\xc0\x80\x8b\xb8\x37\xfd\x95\xfe\x87\x15\xf4\x97\xd5\x61\x4f"   \
      "\x97\xfa\xaf\x48\xd\x5b\x84\x2d\xdb\x15\xf2\xb4\x17\x4f\x41"    \
      "\x31\x58\x32\x93\xc1\x52\x34\xa6\x17\xd\x56\x5\xee\xfb\xfb"     \
      "\x2d\x69\x14\xbe\x24\x94\x8\xb0\xfc\x9f\x2\x95\x88\x7d\xd6"     \
      "\xe7\xa4\x5b\xbb\xf2\x7d\xd8\xa5\xd2\x7c\x9\x62\x22\x5\x53"     \
      "\xd0\x67\xeb\x68\xfc\x82\x80\xf\xc9\x73\x76\x36\xb8\x13\x9f"    \
      "\xb1\xf1\xee\x61\x12\xe7\x5d\x75\x65\xb8\x84\x17\xb\x7b\x28"    \
      "\x4c\xb7\xda\xbb" )                                             \
      + ( (unsigned char)key[i] * sizeof(int) ));                      \
  }                                                                    \
  bkt &= (num_bkts-1);

/* key comparison function; return 0 if keys equal */
#define HASH_KEYCMP(a,b,len) memcmp(a,b,len) 

/* iterate over items in a known bucket to find desired item */
#define HASH_FIND_IN_BKT(hh,head,keyptr,keylen_in,out)               \
out = (head.hh_head) ? (head.hh_head->elmt) : NULL;                  \
while (out) {                                                        \
    if (out->hh.keylen == keylen_in) {                               \
        if ((HASH_KEYCMP(out->hh.key,keyptr,keylen_in)) == 0) break; \
    }                                                                \
    out= (out->hh.hh_next) ? (out->hh.hh_next->elmt) : NULL;         \
}

/* add an item to a bucket  */
#define HASH_ADD_TO_BKT(hh,head,add)                                 \
 head.count++;                                                       \
 add->hh.hh_next = head.hh_head;                                     \
 add->hh.hh_prev = NULL;                                             \
 if (head.hh_head) head.hh_head->hh_prev = &add->hh;                 \
 head.hh_head=&add->hh;                                              \
 if (head.count >= ((head.expand_mult+1) * HASH_BKT_CAPACITY_THRESH) \
     && add->hh.tbl->noexpand != 1) {                                \
       HASH_EXPAND_BUCKETS(add->hh.tbl)                              \
 }

/* remove an item from a given bucket */
#define HASH_DEL_IN_BKT(hh,head,delptr)                              \
    (head).count--;                                                  \
    if ((head).hh_head->elmt == delptr) {                            \
      (head).hh_head = delptr->hh.hh_next;                           \
    }                                                                \
    if (delptr->hh.hh_prev) {                                        \
        delptr->hh.hh_prev->hh_next = delptr->hh.hh_next;            \
    }                                                                \
    if (delptr->hh.hh_next) {                                        \
        delptr->hh.hh_next->hh_prev = delptr->hh.hh_prev;            \
    }                                                                

#define HASH_EXPAND_BUCKETS(tbl)                                     \
    tbl->new_buckets = (UT_hash_bucket*)uthash_bkt_malloc(           \
             2 * tbl->num_buckets * sizeof(struct UT_hash_bucket));  \
    if (!tbl->new_buckets) { uthash_fatal( "out of memory"); }       \
    memset(tbl->new_buckets, 0,                                      \
            2 * tbl->num_buckets * sizeof(struct UT_hash_bucket));   \
    tbl->bkt_ideal= (tbl->num_items /  tbl->num_buckets*2) +         \
                   ((tbl->num_items % (tbl->num_buckets*2)) ? 1 : 0);\
    tbl->sum_of_deltas = 0;                                          \
    for(tbl->bkt_i = 0; tbl->bkt_i < tbl->num_buckets; tbl->bkt_i++) \
    {                                                                \
        tbl->hh = tbl->buckets[ tbl->bkt_i ].hh_head;                \
        while (tbl->hh) {                                            \
           tbl->hh_nxt = tbl->hh->hh_next;                           \
           tbl->key = tbl->hh->key;                                  \
           tbl->keylen = tbl->hh->keylen;                            \
           HASH_FCN(tbl->key,tbl->keylen,tbl->num_buckets*2,tbl->bkt,\
                   tbl->i,tbl->j,tbl->k);                            \
           tbl->newbkt = &(tbl->new_buckets[ tbl->bkt ]);            \
           if (++(tbl->newbkt->count) > tbl->bkt_ideal) {            \
             tbl->sum_of_deltas++;                                   \
             tbl->newbkt->expand_mult = tbl->newbkt->count /         \
                                        tbl->bkt_ideal;              \
           }                                                         \
           tbl->hh->hh_prev = NULL;                                  \
           tbl->hh->hh_next = tbl->newbkt->hh_head;                  \
           if (tbl->newbkt->hh_head) tbl->newbkt->hh_head->hh_prev = \
                tbl->hh;                                             \
           tbl->newbkt->hh_head = tbl->hh;                           \
           tbl->hh = tbl->hh_nxt;                                    \
        }                                                            \
    }                                                                \
    tbl->num_buckets *= 2;                                           \
    uthash_bkt_free( tbl->buckets );                                 \
    tbl->buckets = tbl->new_buckets;                                 \
    tbl->new_hash_q = (1<<16) - ((tbl->sum_of_deltas << 16) / tbl->num_items); \
    if (tbl->hash_q < (1<<15) && tbl->new_hash_q < (1<<15)) {                \
        tbl->noexpand=1;                                             \
        uthash_noexpand_fyi(tbl);                                    \
    }                                                                \
    tbl->hash_q = tbl->new_hash_q;                                   \
    uthash_expand_fyi(tbl);                                         


/* This is an adaptation of Simon Tatham's O(n log(n)) mergesort */
#define HASH_SORT(head,cmpfcn)                                                 \
  if (head) {                                                                  \
      (head)->hh.tbl->insize = 1;                                              \
      (head)->hh.tbl->looping = 1;                                             \
      (head)->hh.tbl->list = &((head)->hh);                                    \
      while ((head)->hh.tbl->looping) {                                        \
          (head)->hh.tbl->p = (head)->hh.tbl->list;                            \
          (head)->hh.tbl->list = NULL;                                         \
          (head)->hh.tbl->tale = NULL;                                         \
          (head)->hh.tbl->nmerges = 0;                                         \
          while ((head)->hh.tbl->p) {                                          \
              (head)->hh.tbl->nmerges++;                                       \
              (head)->hh.tbl->q = (head)->hh.tbl->p;                           \
              (head)->hh.tbl->psize = 0;                                       \
              for ( (head)->hh.tbl->i = 0;                                     \
                    (head)->hh.tbl->i  < (head)->hh.tbl->insize;               \
                    (head)->hh.tbl->i++ ) {                                    \
                  (head)->hh.tbl->psize++;                                     \
                  (head)->hh.tbl->q = (((head)->hh.tbl->q->next) ?             \
                      ((void*)(((long)((head)->hh.tbl->q->next)) +             \
                      (head)->hh.tbl->hho)) : NULL);                           \
                  if (! ((head)->hh.tbl->q) ) break;                           \
              }                                                                \
              (head)->hh.tbl->qsize = (head)->hh.tbl->insize;                  \
              while (((head)->hh.tbl->psize > 0) ||                            \
                      (((head)->hh.tbl->qsize > 0) && (head)->hh.tbl->q )) {   \
                  if ((head)->hh.tbl->psize == 0) {                            \
                      (head)->hh.tbl->e = (head)->hh.tbl->q;                   \
                      (head)->hh.tbl->q = (((head)->hh.tbl->q->next) ?         \
                          ((void*)(((long)((head)->hh.tbl->q->next)) +         \
                          (head)->hh.tbl->hho)) : NULL);                       \
                      (head)->hh.tbl->qsize--;                                 \
                  } else if ( ((head)->hh.tbl->qsize == 0) ||                  \
                             !((head)->hh.tbl->q) ) {                          \
                      (head)->hh.tbl->e = (head)->hh.tbl->p;                   \
                      (head)->hh.tbl->p = (((head)->hh.tbl->p->next) ?         \
                          ((void*)(((long)((head)->hh.tbl->p->next)) +         \
                          (head)->hh.tbl->hho)) : NULL);                       \
                      (head)->hh.tbl->psize--;                                 \
                  } else if ((                                                 \
                      cmpfcn((head)->hh.tbl->p->elmt,(head)->hh.tbl->q->elmt)) \
                          <= 0) {                                              \
                      (head)->hh.tbl->e = (head)->hh.tbl->p;                   \
                      (head)->hh.tbl->p = (((head)->hh.tbl->p->next) ?         \
                          ((void*)(((long)((head)->hh.tbl->p->next)) +         \
                          (head)->hh.tbl->hho)) : NULL);                       \
                      (head)->hh.tbl->psize--;                                 \
                  } else {                                                     \
                      (head)->hh.tbl->e = (head)->hh.tbl->q;                   \
                      (head)->hh.tbl->q = (((head)->hh.tbl->q->next) ?         \
                          ((void*)(((long)((head)->hh.tbl->q->next)) +         \
                                   (head)->hh.tbl->hho)) : NULL);              \
                      (head)->hh.tbl->qsize--;                                 \
                  }                                                            \
                  if ( (head)->hh.tbl->tale ) {                                \
                      (head)->hh.tbl->tale->next = (((head)->hh.tbl->e) ?      \
                               ((head)->hh.tbl->e->elmt) : NULL);              \
                  } else {                                                     \
                      (head)->hh.tbl->list = (head)->hh.tbl->e;                \
                  }                                                            \
                  (head)->hh.tbl->e->prev = (((head)->hh.tbl->tale) ?          \
                                 ((head)->hh.tbl->tale->elmt) : NULL);         \
                  (head)->hh.tbl->tale = (head)->hh.tbl->e;                    \
              }                                                                \
              (head)->hh.tbl->p = (head)->hh.tbl->q;                           \
          }                                                                    \
          (head)->hh.tbl->tale->next = NULL;                                   \
          if ( (head)->hh.tbl->nmerges <= 1 ) {                                \
              (head)->hh.tbl->looping=0;                                       \
              (head)->hh.tbl->tail = (head)->hh.tbl->tale;                     \
              (head) = (head)->hh.tbl->list->elmt;                             \
          }                                                                    \
          (head)->hh.tbl->insize *= 2;                                         \
      }                                                                        \
      HASH_FSCK(head);                                                         \
 }

typedef struct UT_hash_bucket {
   struct UT_hash_handle *hh_head;
   unsigned count;  
   unsigned expand_mult;  
} UT_hash_bucket;

typedef struct UT_hash_table {
   UT_hash_bucket *buckets;
   unsigned num_buckets;
   unsigned num_items;
   int noexpand;  /* when set, inhibits expansion of buckets for this hash  */
   int hash_q; /* measures the evenness of the items among buckets (0-1) */
   struct UT_hash_handle *tail; /* tail hh in app order, for fast append    */
   char *name;    /* macro-stringified name of list head, used by libut     */
   int hho;
   /* scratch */
   unsigned bkt;
   char *key;
   size_t keylen;
   int i,j,k;
   /* scratch for bucket expansion */
   UT_hash_bucket *new_buckets, *newbkt;
   struct UT_hash_handle *hh, *hh_nxt;
   unsigned bkt_i, bkt_ideal, sum_of_deltas;
   int new_hash_q;
   /* scratch for sort */
   int looping,nmerges,insize,psize,qsize;
   struct UT_hash_handle *p, *q, *e, *list, *tale;
   
} UT_hash_table;


typedef struct UT_hash_handle {
   struct UT_hash_table *tbl;
   void *elmt;                       /* ptr to enclosing element       */
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
   void *key;                        /* ptr to enclosing struct's key  */
   size_t keylen;                       /* enclosing struct's key len     */
} UT_hash_handle;

#endif /* UTHASH_H */

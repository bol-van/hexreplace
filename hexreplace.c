//for gcc
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
 #ifdef _WIN64
  #pragma comment( linker, "/subsystem:console,5.02" )
 #else
  #pragma comment( linker, "/subsystem:console,5.01" )
 #endif
 #define xftell _ftelli64
 #define xfseek _fseeki64
#else
 #define xftell ftell
 #define xfseek fseek
#endif

typedef unsigned char BYTE;

BYTE *find_bin(void *data, size_t len, const void *blk, size_t blk_len)
{
 while (len >= blk_len)
 {
  if (!memcmp(data, blk, blk_len))
    return data;
  data = (BYTE*)data + 1;
  len--;
 }
 return NULL;
}

bool is_hex(const char *s)
{
 char c;
 for(;*s;s++)
 {
  c=tolower(*s);
  if (!((c>='0' && c<='9') || (c>='a' && c<='f')))
   return false;
 }
 return true;
}

int readfile(const char *fname,BYTE **buf,size_t *size)
{
 FILE *F;

 F = fopen(fname, "rb");
 if (!F)
 {
   printf("could not open file\n");
   return 2;
 }
 xfseek(F, 0, SEEK_END);
 *size = xftell(F);
 xfseek(F, 0, SEEK_SET);
 printf("file size is %zu\n",*size);
 *buf = malloc(*size);
 if (!*buf)
 {
   fclose(F);
   printf("malloc error\n");
   return 3;
 }
 if (fread(*buf, *size, 1, F)!=1)
 {
   free(*buf);
   fclose(F);
   printf("file read error\n");
   return 4;
 }
 fclose(F);
 return 0;
}
int savefile(const char *fname,BYTE *buf,size_t size)
{
 FILE *F;

 F = fopen(fname, "wb");
 if (!F)
 {
   printf("could not open file for write\n");
   return 2;
 }
 if (fwrite(buf, size, 1, F)!=1)
 {
   fclose(F);
   printf("file write error\n");
   return 5;
 }
 fclose(F);
 return 0;
}
BYTE hexdigit(char hex)
{
 const char *digits = "0123456789abcdef";
 const char *p = strchr(digits, tolower(hex));
 return p ? p-digits : 0;
}
BYTE hexbyte(const char *hex)
{
 return (hexdigit(hex[0])<<4) | hexdigit(hex[1]);
}
// must be freed with free()
BYTE *hexstring_decode(const char *hex)
{
 int i,len;
 BYTE *bin;

 if (!hex || !*hex) return NULL;
 len = strlen(hex)>>1;
 bin = malloc(len);
 if (!bin) return NULL;
 for(i=0;i<len;i++)
  bin[i]=hexbyte(hex+(i<<1));
 return bin;
}

int main(int argc,char **argv)
{
 size_t size;
 BYTE *buf,*e,*p,*found,*sfrom,*sto;
 int r,pattern_len,ct_found;

 if (argc<5)
 {
   printf("hexreplace 1.0\nhexreplace <infile> <outfile> <hex_from> <hex_to>\n hex strings must be equal length\n");
   return 1;
 }
 pattern_len=strlen(argv[3]);
 if (pattern_len&1 || pattern_len!=strlen(argv[4]) || !is_hex(argv[3]) || !is_hex(argv[4]))
 {
   printf("hex strings must be valid and equal length\n");
   return 1;
 }
 pattern_len>>=1;

 if (r=readfile(argv[1],&buf,&size))
  return r;

 printf("replacing '%s' with '%s'\n",argv[3],argv[4]);

 sfrom = hexstring_decode(argv[3]);
 sto = hexstring_decode(argv[4]);
 if (!sfrom || !sto)
 {
   printf("error converting hex strings\n");
   free(sfrom);
   free(sto);
   free(buf);
   return 1;
 }
 for (p=buf,e=buf+size,ct_found=0 ; found=find_bin(p,e-p,sfrom,pattern_len) ; ct_found++,p=found+pattern_len)
 {
   printf("found pattern at offset 0x%X\n",(unsigned int)(found-buf));
   memcpy(found,sto,pattern_len);
 }
 printf("* found total %d patterns\n",ct_found);

 free(sfrom);
 free(sto);
 r = savefile(argv[2],buf,size);
 free(buf);
 return r;
}

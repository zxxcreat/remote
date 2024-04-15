#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/aes.h>

typedef struct{
    RSA *rsa;
    BIGNUM *big_num;
}rsa_all;

int rsa_ini(rsa_all * data);
int rsa_close(rsa_all * data);

int rsa_encode(char *data ,int data_size,char *key,int key_size,char *out,int out_size);
int rsa_encode_1(RSA *rsa,char *data ,int data_size,char *out,int out_size);
int rsa_decode(char *data ,int data_size,char *key,int key_size,char *out,int out_size);
int rsa_decode_1(RSA *rsa,char *data,int data_size,char *out,int out_size);
int rsa_private_char(RSA *rsa ,char * result,int size);
int rsa_public_char(RSA *rsa ,char * result,int size);
typedef struct{
    AES_KEY aes;
    unsigned char key[32];
    unsigned char iv[16];
}aes_all;
int aes_ini(aes_all *data);
//  AES_KEY ak;
//     unsigned char key[32]="asfdghd21345";
//     unsigned char iv[16]="fasdas213412313";
//     unsigned char iv2[16]="fasdas213412313";
//     AES_set_decrypt_key(key,256,&ak);
//     unsigned char aa[]="hello world!!";
//     unsigned char bb[512],cc[512];
//     AES_cbc_encrypt(aa,bb,sizeof(aa),&ak,iv,1);
//     AES_cbc_encrypt(bb,cc,16,&ak,iv2,0);
//     for (int i=0;i<16;i++){
//         printf("%c ",(char)aa[i]);
//     }
//     printf("\n");

    //         RSA                      *r;

    //         int                         bits=4096,ret;

    //         unsigned long  e=RSA_3;

    //         BIGNUM               *bne;


    //         bne=BN_new();

    //         ret=BN_set_word(bne,e);

    //         r=RSA_new();

    //         ret=RSA_generate_key_ex(r,bits,bne,NULL);
            
            
    //     int num =RSA_size(r)-11;
    //     unsigned char en[4096]; 
    //     for (int i=0;i<num;i++){
    //         en[i]=i;

    //     } 
       
    //     unsigned char buffer[4096];unsigned char buffer21[4096];
     
    //     unsigned char *bb=(unsigned char *)malloc(4096);
    //     unsigned char cc[2000];
    //     int ss=i2d_RSAPublicKey(r,&(&cc[0]));
    //     int len=RSA_public_encrypt(num,(unsigned char *)en,(unsigned char *)buffer,r,RSA_PKCS1_PADDING);

    //    int len2= RSA_private_decrypt(RSA_size(r),(unsigned char *)buffer,(unsigned char *)buffer21,r,RSA_PKCS1_PADDING);
    //    printf("size= %d len=%d len2=%d ret=%d ss=%d \n",RSA_size(r),len, len2,ret,ss); 

    //     for (int i=0;i<num;i++){
    //         printf("%02x ",buffer21[i]);
    //     } 
    //     printf("\n----------------------------------------\n");
        
    //         if(ret!=1)

    //         {

    //                 printf("RSA_generate_key_ex err!\n");

    //                 return -1;

    //         }

    //         RSA_free(r);
    //         BN_free(bne);
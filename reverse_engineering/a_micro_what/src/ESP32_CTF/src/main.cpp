#include "Arduino.h"
#include "mbedtls/md.h"
#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"

char* password;
//void aes_enc(unsigned char* pass) {
//    mbedtls_cipher_context_t ctx;
//    mbedtls_cipher_context_t ctx2;
//
//    unsigned char error[1024];
//
//    unsigned char key[32] = {0};
//
//    unsigned char iv[16] =  {
//        0x77, 0x61, 0x6b, 0x65, 0x20, 0x75, 0x70, 0x2c,
//        0x20, 0x6e, 0x65, 0x6f, 0x2e, 0x2e, 0x2e, 0x00
//    };
//    
//    for (int i = 0; i < 32; i++) {
//      key[i] = pass[i];
//    }
//    
//    
//    size_t olen = 0;
//    size_t dlen = 0;
//    unsigned char output[1024];
//    unsigned char decrypted[1024];
//    const unsigned char *input = (const unsigned char*)   "ARC{You_did_ESP_well_on_this!}\0";
//    size_t total = 0;
//
//    mbedtls_cipher_init(&ctx);
//    mbedtls_cipher_setup(&ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC));
//    mbedtls_cipher_setkey(&ctx, key, sizeof(key) * 8, MBEDTLS_ENCRYPT);
//    mbedtls_cipher_set_iv(&ctx, iv, sizeof(iv));
//
//    mbedtls_cipher_update(&ctx, input, strlen((const char*) input), output, &olen);
//    total = olen;
//    mbedtls_cipher_finish(&ctx, output + total, &olen);
//    total += olen;
//    printf("Enc len %zu \n", total);
//    printf("Enc txt ");
//    for (int i = 0; i < total; ++i) {
//        static const char hex[] = "0123456789abcdef";
//        printf("0x%c%c,", hex[output[i] >> 4], hex[output[i] & 0x0f]);
//    }
//    puts("");
//
//    mbedtls_cipher_init(&ctx2);
//    mbedtls_cipher_setup(&ctx2, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC));
//    mbedtls_cipher_setkey(&ctx2, key, sizeof(key) * 8, MBEDTLS_DECRYPT);
//    mbedtls_cipher_set_iv(&ctx2, iv, sizeof(iv));
//
//    mbedtls_cipher_update(&ctx2, output, total, decrypted, &dlen);
//    total = dlen;
//    mbedtls_cipher_finish(&ctx2, decrypted + total, &dlen);
//    total += dlen;
//    printf("Test Decrypt len %zu \n", total);
//    printf("Test Decrypt txt |%.*s|\n", total, decrypted);
//}

void print_byte_array(unsigned char* data, int size) {
  for(int i= 0; i< size; i++){
      char str[3];

      sprintf(str, "%02x", (int)data[i]);
      Serial.print(str);
  }
  Serial.println("");
}

char* ask_pass() {
  char *password;
  password = (char*)malloc(13 * sizeof(char));
  char rx_byte = 0;
  int char_cnt = 0;
  Serial.println("Please enter the password: ");
  fflush(stdin);
  while (char_cnt < 12){
    if(Serial.available() > 0) {
      rx_byte = Serial.read();
      password[char_cnt] = rx_byte;
      char_cnt++;
    } else {
      delay(50);
    }
  }
  password[12] = '\0';
  //Serial.print(password);
  return (char*) password;
}

void get_flag_AES_CBC(unsigned char* pass) {
  Serial.println("Decrypting");
    mbedtls_cipher_context_t ctx;

    unsigned char flag[32] = {
        0x30, 0x7b, 0xcb, 0xd3, 0x1c, 0x74, 0xf0, 0x95,
        0xce, 0x47, 0xdc, 0xe0, 0xa1, 0x9e, 0xe6, 0x57,
        0x5e, 0x09, 0x7b, 0x81, 0xd4, 0x5e, 0x70, 0x66,
        0x18, 0x3a, 0x47, 0xd1, 0x2d, 0x36, 0xf6, 0xd8
    };

    unsigned char key[32] = {0};

    unsigned char iv[16] =  {
        0x77, 0x61, 0x6b, 0x65, 0x20, 0x75, 0x70, 0x2c,
        0x20, 0x6e, 0x65, 0x6f, 0x2e, 0x2e, 0x2e, 0x00
    };
    
    for (int i = 0; i < 32; i++) {
      key[i] = pass[i];
    }
    size_t dlen = 0;
    unsigned char decrypted[1024];
    size_t total = 0;

    mbedtls_cipher_init(&ctx);
    mbedtls_cipher_setup(&ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC));
    mbedtls_cipher_setkey(&ctx, key, sizeof(key) * 8, MBEDTLS_DECRYPT);
    mbedtls_cipher_set_iv(&ctx, iv, sizeof(iv));
    mbedtls_cipher_update(&ctx, flag, sizeof(flag) * sizeof(unsigned char), decrypted, &dlen);
    total = dlen;
    mbedtls_cipher_finish(&ctx, decrypted + total, &dlen);
    total += dlen;
    printf("PlainText: %.*s\n", total, decrypted);
}

unsigned char* get_hash(char* input) {
  unsigned char *shaResult;
  shaResult = (unsigned char*)malloc(32*sizeof(unsigned char));
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  const size_t payloadLength = strlen(input);
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) input, payloadLength); 
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  delay(1000);
  Serial.print("Hash: ");
  print_byte_array(shaResult, 32);
  return shaResult;
}

bool check_pass(char* password){
  // Password == H@kDaP1@n3t!
  if (strlen(password) != 12) {
    return false;
  }

  if (password[1] != '@') {
    return false;
  }

  if (password[4] != 'a') {
    return false;
  }

  if (password[7] != password[1]) {
    return false;
  }
  
  if (password[11] != '!') {
    return false;
  }
  
  if (password[6] != '1') {
    return false;
  }
  
  if (password[10] != 't') {
    return false;
  }
  
  if (password[12] != '\0') {
    return false;
  }

  if (password[0] != 'H') {
    return false;
  }

  if (password[2] != 'k') {
    return false;
  }
  
  if (password[9] != '3') {
    return false;
  }

  if (password[5] != 'P') {
    return false;
  }

  if (password[3] != 'D') {
    return false;
  }
  
  if (password[8] != 'n') {
    return false;
  }

  return true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  //rtc_wdt_protect_off();
  //rtc_wdt_disable();
  disableCore0WDT();
  disableCore1WDT();

  // Uncoment to set new flag
  //aes_enc(get_hash("H@kDaP1@n3t!"));
  //get_flag_AES_CBC(get_hash("H@kDaP1@n3t!"));
}

void loop() {
  // put your main code here, to run repeatedly:
  char *password = ask_pass();
  if (check_pass(password)) {
    unsigned char* hash = get_hash(password);
    get_flag_AES_CBC(hash);
    free(hash);
  } else {
    Serial.println("Access Denied: Incorrect password");
  }
  free(password);
  delay(1000);
}
#ifndef PASSWORD_H
#define PASSWORD_H


#define PW_MD5_SALT     0x75d5df0d

bool password_check(uint32_t hash);
bool password_check_plaintext(char *pass, uint8_t len);
void password_init(void);

#endif

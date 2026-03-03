#ifndef CLIENT_H
#define CLIENT_H

#define MAX_COOKIE_LEN 1024

#define MAX_USERNAME_LEN 100
#define MAX_TOKEN_LEN 600

#define SERVER_HOST "63.32.125.183"
#define SERVER_PORT 8081

typedef struct {
    char admin_cookie[MAX_COOKIE_LEN];  // Separate cookie for admin
    char user_cookie[MAX_COOKIE_LEN];   // Separate cookie for user
    int logged_in_flag;
    int user_logged_in_flag;
    char logged_in_username[MAX_USERNAME_LEN];
    char jwt_token[MAX_TOKEN_LEN]; // JWT token
} ClientState;

ClientState init_client_state(void);

#endif

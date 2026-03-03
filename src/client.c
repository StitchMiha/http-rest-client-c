#include "client.h"

#include <arpa/inet.h>
#include <netdb.h>     
#include <netinet/in.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "commands.h"
#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include "utils.h"

ClientState init_client_state(void) {
    ClientState state;
    memset(state.admin_cookie, 0, MAX_COOKIE_LEN);
    memset(state.user_cookie, 0, MAX_COOKIE_LEN);
    memset(state.logged_in_username, 0, MAX_USERNAME_LEN);
    memset(state.jwt_token, 0, MAX_TOKEN_LEN);
    state.logged_in_flag = 0;
    state.user_logged_in_flag = 0;
    return state;
}

int main(void) {
    ClientState state = init_client_state();
    char command[LINELEN];

    while (1) {
        if (!fgets(command, LINELEN, stdin)) {
            break;
        }

        remove_newline(command);

        if (strcmp(command, "login_admin") == 0) {
            login_admin(&state);
        } else if (strcmp(command, "add_user") == 0) {
            add_user(&state);
        } else if (strcmp(command, "get_users") == 0) {
            get_users(&state);
        } else if (strcmp(command, "delete_user") == 0) {
            delete_user(&state);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "logout_admin") == 0) {
            logout_admin(&state);
        } else if (strcmp(command, "login") == 0) {
            login_user(&state);
        } else if (strcmp(command, "logout") == 0) {
            logout_user(&state);
        } else if (strcmp(command, "get_access") == 0) {
            get_access(&state);
        } else if (strcmp(command, "get_movies") == 0) {
            get_movies(&state);
        } else if (strcmp(command, "get_movie") == 0) {
            get_movie(&state);
        } else if (strcmp(command, "add_movie") == 0) {
            add_movie(&state);
        } else if (strcmp(command, "update_movie") == 0) {
            update_movie(&state);
        } else if (strcmp(command, "delete_movie") == 0) {
            delete_movie(&state);
        } else if (strcmp(command, "get_collections") == 0) {
            get_collections(&state);
        } else if (strcmp(command, "get_collection") == 0) {
            get_collection(&state);
        } else if (strcmp(command, "add_collection") == 0) {
            add_collection(&state);
        } else if (strcmp(command, "delete_collection") == 0) {
            delete_collection(&state);
        } else if (strcmp(command, "add_movie_to_collection") == 0) {
            add_movie_to_collection(&state);
        } else if (strcmp(command, "delete_movie_from_collection") == 0) {
            delete_movie_from_collection(&state);
        }

        else {
            printf("ERROR: Unknown !!\n");
        }
    }

    return 0;
}

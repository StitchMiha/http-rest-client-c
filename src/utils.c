
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "client.h"
#include "helpers.h"
#include "parson.h"
#include "requests.h"

// Helper function to combine the logic of both opening and closing a socket in order to avoid code repetition many times
char *send_and_receive_request(char *request) {
    int sockfd = open_connection(SERVER_HOST, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, request);
    char *response = receive_from_server(sockfd);
    close_connection(sockfd);
    return response;
}

// Helper function to remove newline
void remove_newline(char *line) {
    if ((strlen(line) > 0) && (line[strlen(line) - 1] == '\n')) {
        line[strlen(line) - 1] = '\0';
    }
}

// This is a helper function that I use when I need to read input by the user, so that I avoid repetition of code
void read_input(const char *label, char *buffer, size_t size) {
    printf("%s", label);
    fflush(stdout);
    if (!fgets(buffer, size, stdin)) {
        error("Failed to read input.");
    }
    remove_newline(buffer);
}

// This function I implemented so that to extract the Set-Cookie: value from the HTTP response and to store it in the correct session cookie field
// either admin_cookie or use_cookie
void extract_cookie(const char *response, char *dest, int *flag) {
    const char *cookie_start = strstr(response, "Set-Cookie: ");
    if (cookie_start) {
        cookie_start += strlen("Set-Cookie: ");
        const char *cookie_end = strchr(cookie_start, ';');
        if (cookie_end) {
            size_t len = cookie_end - cookie_start;
            strncpy(dest, cookie_start, len);
            dest[len] = '\0';
            if (flag) *flag = 1;
        }
    }
}

// Helper functions to help me build a JSON object depending on how many key-value pairs I have
char *build_json_body(const char *key1, const char *val1, const char *key2, const char *val2, JSON_Value **out_root) {
    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, key1, val1);
    json_object_set_string(obj, key2, val2);
    *out_root = root;  // so that the caller can free it up later
    return json_serialize_to_string(root);
}

char *build_json_3_fields(const char *k1, const char *v1, const char *k2, const char *v2, const char *k3, const char *v3, JSON_Value **out_root) {
    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, k1, v1);
    json_object_set_string(obj, k2, v2);
    json_object_set_string(obj, k3, v3);
    *out_root = root;
    return json_serialize_to_string(root);
}

// This is a helper function that helps me parse and print all collections from a JSON response that shows each collection its ID and title
void print_collections(const char *response) {
    char *json_str = basic_extract_json_response(response);
    JSON_Value *val = json_parse_string(json_str);
    JSON_Object *root = json_value_get_object(val);
    JSON_Array *collections = json_object_get_array(root, "collections");

    if (!collections) {
        printf("ERROR: There was an error when trying to parse the collections array.\n");
        json_value_free(val);
        return;
    }

    size_t count = json_array_get_count(collections);
    printf("SUCCESS: List of collections \n");
    for (size_t i = 0; i < count; i++) {
        JSON_Object *col = json_array_get_object(collections, i);
        int id = (int)json_object_get_number(col, "id");
        const char *title = json_object_get_string(col, "title");
        printf("#%d: %s\n", id, title);
    }

    json_value_free(val);
}

// Helper fucntion that helps me extract the HTTP status code from the first line of the response
int get_response_status(const char *response) {
    int status_code = -1;

    if (sscanf(response, "HTTP/1.1 %d", &status_code) == 1) {
        return status_code;
    }

    return -1;
}

// This is a helper function that frees all dynamically allocated memory related to an HTTP request.
void cleanup_request_resources(JSON_Value *root, char *body, char *request, char *response) {
    if (body) json_free_serialized_string(body);
    if (root) json_value_free(root);
    if (request) free(request);
    if (response) free(response);
}

// This is a helper function that will help me resset session state when a user logs out
void clear_user_session(ClientState *state) {
    state->user_logged_in_flag = 0;
    memset(state->user_cookie, 0, MAX_COOKIE_LEN);
    memset(state->logged_in_username, 0, MAX_USERNAME_LEN);
    memset(state->jwt_token, 0, MAX_TOKEN_LEN);
}

// Prints a error message when a login is attempted while already logged in
void print_already_logged_in_error() {
    printf("ERROR: Already logged in.\n");
}

// Prints a error message when an admin-only command is attempted while we have no admin
void print_admin_not_logged_error() {
    printf("ERROR: You must be logged in dirst as an admin to perform this action.\n");
}

// Returns 1 if a user is logged in, otherwise prints an error
int is_user_logged_in(ClientState *state) {
    if (!state->user_logged_in_flag) {
        printf("ERROR: No user is currently logged in.\n");
        return 0;
    }
    return 1;
}

// Checks whether the user has access to the library (JWT token exists)
int does_have_access(ClientState *state) {
    if (strlen(state->jwt_token) == 0) {
        printf("ERROR: You do not have access to the library.\n");
        return 0;
    }
    return 1;
}

// This is a helper function that parses and prints a list of users from a JSON response
void print_user_list_from_response(const char *response) {
    char *json_str = basic_extract_json_response(response);
    if (!json_str) {
        return;
    }

    JSON_Value *json_val = json_parse_string(json_str);
    if (!json_val) {
        return;
    }

    JSON_Object *json_obj = json_value_get_object(json_val);
    JSON_Array *users = json_object_get_array(json_obj, "users");

    for (size_t i = 0; i < json_array_get_count(users); ++i) {
        JSON_Object *user_obj = json_array_get_object(users, i);
        int id = (int)json_object_get_number(user_obj, "id");
        const char *username = json_object_get_string(user_obj, "username");
        const char *password = json_object_get_string(user_obj, "password");
        printf("#%d %s:%s\n", id, username, password);
    }

    json_value_free(json_val);
}

// Helper function that extracts the token field from a JSON response and stores it in the dest
void extract_token_from_response(const char *response, char *dest) {
    char *json = basic_extract_json_response(response);
    if (!json) return;

    JSON_Value *val = json_parse_string(json);
    if (!val) return;

    JSON_Object *obj = json_value_get_object(val);
    const char *token = json_object_get_string(obj, "token");

    if (token) {
        strncpy(dest, token, MAX_TOKEN_LEN - 1);
    }

    json_value_free(val);
}

// Parses a list of movies from the JSON response and prints each movie's ID and title. It is used in get_movies()
void print_movies(const char *response) {
    char *json = basic_extract_json_response(response);
    if (!json) return;

    JSON_Value *val = json_parse_string(json);
    if (!val) return;

    JSON_Object *root_obj = json_value_get_object(val);
    if (!root_obj) {
        json_value_free(val);
        return;
    }

    JSON_Array *movies = json_object_get_array(root_obj, "movies");
    if (!movies) {
        json_value_free(val);
        return;
    }

    for (size_t i = 0; i < json_array_get_count(movies); i++) {
        JSON_Object *movie = json_array_get_object(movies, i);
        int id = (int)json_object_get_number(movie, "id");
        const char *title = json_object_get_string(movie, "title");
        printf("#%d %s\n", id, title);
    }

    json_value_free(val);
}

// Builds a JSON object representing a movie
char *build_movie_json(const char *title, int year, const char *description, double rating, JSON_Value **out_root) {
    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);

    json_object_set_string(obj, "title", title);
    json_object_set_number(obj, "year", year);
    json_object_set_string(obj, "description", description);
    json_object_set_number(obj, "rating", rating);

    *out_root = root;
    return json_serialize_to_string(root);
}
// This is a helper function that parses and prints details of a single movie from the response
void print_movie_details(const char *response) {
    char *json_str = basic_extract_json_response(response);
    JSON_Value *val = json_parse_string(json_str);
    JSON_Object *obj = json_value_get_object(val);

    const char *title = json_object_get_string(obj, "title");
    const char *description = json_object_get_string(obj, "description");
    int year = (int)json_object_get_number(obj, "year");

    const char *rating_str = json_object_get_string(obj, "rating");
    double rating = atof(rating_str);

    printf("title: %s\n", title);
    printf("year: %d\n", year);
    printf("description: %s\n", description);
    printf("rating: %.1f\n", rating);

    json_value_free(val);
}

// Helper function that builds a JSON object for a collection with a single field the title field
char *build_collection_body(const char *title, JSON_Value **out_root) {
    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_string(obj, "title", title);
    *out_root = root;
    return json_serialize_to_string(root);
}

// Helper function that builds a JSON object for a collection with a single field the id field, it is used when adding a movie to a collection
char *build_id_body(int id, JSON_Value **out_root) {
    JSON_Value *root = json_value_init_object();
    JSON_Object *obj = json_value_get_object(root);
    json_object_set_number(obj, "id", id);
    *out_root = root;
    return json_serialize_to_string(root);
}

// This is a helper function that parses and prints details of a collection  like the title, owner, movies from a JSON response. Used in get_collection()
void print_collection_details(const char *response) {
    char *json_str = basic_extract_json_response(response);
    JSON_Value *val = json_parse_string(json_str);
    JSON_Object *obj = json_value_get_object(val);

    const char *title = json_object_get_string(obj, "title");
    const char *owner = json_object_get_string(obj, "owner");
    JSON_Array *movies = json_object_get_array(obj, "movies");

    printf("SUCCESS: Details about this collection include:\n");
    printf("title: %s\n", title);
    printf("owner: %s\n", owner);

    for (size_t i = 0; i < json_array_get_count(movies); i++) {
        JSON_Object *movie = json_array_get_object(movies, i);
        int movie_id = (int)json_object_get_number(movie, "id");
        const char *movie_title = json_object_get_string(movie, "title");
        printf("#%d: %s\n", movie_id, movie_title);
    }

    json_value_free(val);
}

// This is a helper function that retrieves the id of the most recerecently added collection.
int get_last_collection_id(const char *auth_header) {
    char *headers[] = {(char *)auth_header};
    // Sends a GET request to get all collections
    char *request = compute_get_request2(SERVER_HOST, "/api/v1/tema/library/collections", NULL, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);

    char *json_str = basic_extract_json_response(response);
    JSON_Value *val = json_parse_string(json_str);
    // Here we access the collections array from the JSON object
    JSON_Array *collections = json_object_get_array(json_value_get_object(val), "collections");

    int last_id = -1;
    if (collections && json_array_get_count(collections) > 0) {
        // Here we get the id of the last collection in the array
        JSON_Object *last_col = json_array_get_object(collections, json_array_get_count(collections) - 1);
        last_id = (int)json_object_get_number(last_col, "id");
    }

    json_value_free(val);
    free(request);
    free(response);
    return last_id;
}

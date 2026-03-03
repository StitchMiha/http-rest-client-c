#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "helpers.h"
#include "parson.h"
#include "requests.h"
#include "utils.h"

void login_admin(ClientState *state) {
    if (state->logged_in_flag) {
        print_already_logged_in_error();  // second error being treated
        return;
    }

    char username[USER_LEN] = {0};
    char password[PASS_LEN] = {0};

    read_input("username=", username, USER_LEN);
    read_input("password=", password, PASS_LEN);

    JSON_Value *root = NULL;
    char *body = build_json_body("username", username, "password", password, &root);

    char *request = compute_post_json_request_json(SERVER_HOST, "/api/v1/tema/admin/login", body, NULL, 0);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        extract_cookie(response, state->admin_cookie, &state->logged_in_flag);
        strncpy(state->logged_in_username, username, MAX_USERNAME_LEN - 1);
        printf("SUCCESS: Admin logged in successfully\n");
    } else if (status == 403) {
        printf("ERROR: Credentials do not match.\n");  // first error being treated
    } else {
        printf("ERROR: Login failed.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void add_user(ClientState *state) {
    if (!state->logged_in_flag) {
        print_admin_not_logged_error();  // first error being treated
        return;
    }

    char username[USER_LEN] = {0};
    char password[PASS_LEN] = {0};

    read_input("username=", username, USER_LEN);
    read_input("password=", password, PASS_LEN);

    JSON_Value *root = NULL;
    char *body = build_json_body("username", username, "password", password, &root);

    // Prepare cookies array, we need this to prove that youve logged in as an admin, that's why the cookies array is passed when making the request
    char *cookies[] = {state->admin_cookie};

    char *request = compute_post_json_request_json(SERVER_HOST, "/api/v1/tema/admin/users", body, cookies, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200 || status == 201) {
        printf("SUCCESS: User added with success!!\n");
    } else if (status == 403) {
        printf("ERROR: There are no admin privileges.\n");  // second treated error
    } else if (status == 409) {
        printf("ERROR: User already exists. Cannot add the same user twice!\n");
    } else {
        printf("ERROR: Something else went wrong.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void get_users(ClientState *state) {
    if (!state->logged_in_flag) {
        print_admin_not_logged_error();  // first error being treated
        return;
    }

    char *cookies[] = {state->admin_cookie};
    char *request = compute_get_request(SERVER_HOST, "/api/v1/tema/admin/users", NULL, cookies, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 403) {
        printf("ERROR: You do not have admin privileges.\n");
    } else if (status == 200) {
        printf("SUCCESS: This is the list of users: \n");
        print_user_list_from_response(response);
    } else {
        printf("ERROR: This response is not known.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}

void delete_user(ClientState *state) {
    if (!state->logged_in_flag) {
        print_admin_not_logged_error();  // first error being treated
        return;
    }

    char username[USER_LEN] = {0};
    read_input("username=", username, USER_LEN);

    char url[URL_LEN];
    snprintf(url, URL_LEN, "/api/v1/tema/admin/users/%s", username);

    char *cookies[] = {state->admin_cookie};

    // Build DELETE request using helper function compute_delete_request defined almost identical as the ones in lab
    char *request = compute_delete_request(SERVER_HOST, url, cookies, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: User deleted successfully.\n");
    } else if (status == 403) {
        printf("ERROR: You don't have admin privileges.\n");
    } else if (status == 404) {
        printf("ERROR: Username not found.\n");
    } else {
        printf("ERROR: Something went wrong while trying to delete the user.\n");
    }

    cleanup_request_resources(NULL, NULL, request, response);
}

void logout_admin(ClientState *state) {
    if (!state->logged_in_flag) {
        print_admin_not_logged_error();  // errors treated
        return;
    }

    char *cookies[] = {state->admin_cookie};
    char *request = compute_get_request(SERVER_HOST, "/api/v1/tema/admin/logout", NULL, cookies, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        state->logged_in_flag = 0;
        memset(state->admin_cookie, 0, MAX_COOKIE_LEN);
        printf("SUCCESS: Admin logged out successfully.\n");
    } else if (status == 401) {
        printf("ERROR: You are not authenticated.\n");
    } else {
        printf("ERROR: Logout failed.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}

void login_user(ClientState *state) {
    char admin_username[USER_LEN] = {0};
    char username[USER_LEN] = {0};
    char password[PASS_LEN] = {0};

    read_input("admin_username=", admin_username, USER_LEN);
    read_input("username=", username, USER_LEN);
    read_input("password=", password, PASS_LEN);

    // This is the reauthentication error being treated
    if (state->user_logged_in_flag) {
        if (strcmp(state->logged_in_username, username) == 0) {
            printf("SUCCESS: User '%s' is already logged in.\n", username);
            return;
        } else {
            printf("ERROR: Another user is already logged in. Please log out first.\n");
            return;
        }
    }

    // Build JSON body with 3 fields using another custom helper function
    JSON_Value *root = NULL;
    char *body = build_json_3_fields("admin_username", admin_username, "username", username, "password", password, &root);

    char *cookies[] = {state->admin_cookie};

    char *request = compute_post_json_request_json(SERVER_HOST, "/api/v1/tema/user/login", body, cookies, 1);

    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: User '%s' logged in successfully.\n", username);
        // Extract user session cookie from response
        extract_cookie(response, state->user_cookie, &state->user_logged_in_flag);  // parses the cookie value from the response and saves it in state->user_cookie
        strncpy(state->logged_in_username, username, MAX_USERNAME_LEN - 1);         // saves the currently logged in username to detect reauthentication
        state->user_logged_in_flag = 1;
    } else if (status == 403) {
        printf("ERROR: Incorrect credentials.\n");  // This is the first error being treated
    } else {
        printf("ERROR: User login failed.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void logout_user(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    char *cookies[] = {state->user_cookie};
    char *request = compute_get_request(SERVER_HOST, "/api/v1/tema/user/logout", NULL, cookies, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        // we need to clear the session state
        clear_user_session(state);
        printf("SUCCESS: User logged out successfully.\n");
    } else if (status == 401) {
        printf("ERROR: You are not authenticated as a user.\n");
    } else {
        printf("ERROR: User logout failed.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}

void get_access(ClientState *state) {
    if (!is_user_logged_in(state)) {  // error being treated
        return;
    }

    char *cookies[] = {state->user_cookie};
    char *request = compute_get_request(SERVER_HOST, "/api/v1/tema/library/access", NULL, cookies, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        // we extract the JWT token and store it
        extract_token_from_response(response, state->jwt_token);
        printf("SUCCESS: JWT token received successfully.\n");
    } else if (status == 401) {
        printf("ERROR: You need to be logged in first.\n");
    } else {
        printf("ERROR: Failed to request access token. Error code: %d\n", status);
    }

    cleanup_request_resources(NULL, NULL, request, response);
}

void get_movies(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_get_request2(SERVER_HOST, "/api/v1/tema/library/movies", NULL, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS:This is the list of movies:\n");
        print_movies(response);
    } else if (status == 403) {
        printf("ERROR: No access to this library.\n");
    } else {
        printf("ERROR: Something else went wrong!.\n");
    }

    cleanup_request_resources(NULL, NULL, request, response);
}

void get_movie(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char movie_id[20] = {0};
    read_input("id=", movie_id, sizeof(movie_id));

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/movies/%s", movie_id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_get_request2(SERVER_HOST, url, NULL, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        print_movie_details(response);
    } else if (status == 403) {
        printf("ERROR: You don't have access to the library.\n");
    } else if (status == 404) {
        printf("ERROR: Invalid movie ID.\n");
    } else {
        printf("ERROR: Unexpected error.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}

void add_movie(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char title[100], description[300];
    char year_str[10], rating_str[10];

    read_input("title=", title, sizeof(title));
    read_input("year=", year_str, sizeof(year_str));
    read_input("description=", description, sizeof(description));
    read_input("rating=", rating_str, sizeof(rating_str));

    int year = atoi(year_str);  // needed to convert year and rating to a valid field
    double rating = atof(rating_str);

    if (rating < 0 && rating > 9.9) {
        printf("ERROR: The rating has to be between the values of 0 and 9.9");  // had an error at the beggining that it cannot be more than 9.9
        return;
    }

    JSON_Value *root = NULL;
    char *body = build_movie_json(title, year, description, rating, &root);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_post_json_request2(SERVER_HOST, "/api/v1/tema/library/movies", body, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200 || status == 201) {
        printf("SUCCESS: Movie added successfully.\n");
    } else if (status == 403) {
        printf("ERROR: You don't have access to the library.\n");
    } else if (status == 400) {
        printf("ERROR: Invalid or incomplete movie data.\n");
    } else {
        printf("ERROR: Unexpected server error.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void update_movie(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;  // first error being treated
    }

    char id[20], title[100], description[300], year_str[10], rating_str[10];

    read_input("id=", id, sizeof(id));
    read_input("title=", title, sizeof(title));
    read_input("year=", year_str, sizeof(year_str));
    read_input("description=", description, sizeof(description));
    read_input("rating=", rating_str, sizeof(rating_str));

    int year = atoi(year_str);
    double rating = atof(rating_str);

    if (year <= 0 || rating < 0 || rating > 10) {
        printf("ERROR: Invalid year or rating.\n");
        return;
    }

    JSON_Value *root = NULL;
    char *body = build_movie_json(title, year, description, rating, &root);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/movies/%s", id);

    // PUT request
    char *request = compute_put_json_request(SERVER_HOST, url, body, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: Film has been updated successfully!\n");
    } else if (status == 403) {
        printf("ERROR: You don't have access to the library.\n");
    } else if (status == 404) {
        printf("ERROR: Invalid movie ID.\n");  // second error being treated
    } else if (status == 400) {
        printf("ERROR: Invalid or incomplete movie data.\n");  // 3rd error being treated
    } else {
        printf("ERROR: Unexpected server error.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void delete_movie(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char movie_id[20] = {0};
    read_input("id=", movie_id, sizeof(movie_id));

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/movies/%s", movie_id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_delete_request2(SERVER_HOST, url, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: Movie has been deleted successfully!\n");
    } else if (status == 403) {
        printf("ERROR: You need to have authorization firstly!\n");
    } else if (status == 404) {
        printf("ERROR: The movie ID is not valid.\n");
    } else {
        printf("ERROR: There was a server error.\n");
    }

    cleanup_request_resources(NULL, NULL, request, response);
}
void get_collections(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;  // error being treated
    }

    char auth_header[MAX_TOKEN_LEN + 64];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_get_request2(SERVER_HOST, "/api/v1/tema/library/collections", NULL, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        print_collections(response);
    } else if (status == 403) {
        printf("ERROR: You don't have access to the library.\n");
    } else {
        printf("ERROR: Unexpected server error.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}

void get_collection(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char collection_id[20] = {0};
    read_input("id=", collection_id, sizeof(collection_id));

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%s", collection_id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_get_request2(SERVER_HOST, url, NULL, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        print_collection_details(response);
    } else if (status == 403) {
        printf("ERROR: You don't have access to the library.\n");
    } else if (status == 404) {
        printf("ERROR: Invalid collection ID.\n");
    } else {
        printf("ERROR: Failed to retrieve collection details.\n");
    }

    cleanup_request_resources(NULL, NULL, request, response);
}

void add_collection(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char title[100];
    read_input("title=", title, sizeof(title));

    JSON_Value *root = NULL;
    char *body = build_collection_body(title, &root);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_post_json_request2(SERVER_HOST, "/api/v1/tema/library/collections", body, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status != 200 && status != 201) {
        printf("ERROR: Failed to create collection with status error: %d\n", status);
        cleanup_request_resources(root, body, request, response);
        return;
    }

    printf("SUCCESS: Collection has been successfully added!\n");
    printf("title: %s\n", title);
    printf("owner: %s\n", state->logged_in_username);

    cleanup_request_resources(root, body, request, response);

    char num_str[10];
    read_input("num_movies=", num_str, sizeof(num_str));  // how many movies the user wants
    int num_movies = atoi(num_str);

    // we get the latest collection ID
    int collection_id = get_last_collection_id(auth_header);
    if (collection_id == -1) {
        printf("ERROR: Failed to determine collection ID.\n");
        return;
    }

    // Here we loop to add each movie by id into the collection
    for (int i = 0; i < num_movies; i++) {
        char label[20], input[10];
        snprintf(label, sizeof(label), "movie_id[%d]=", i);
        read_input(label, input, sizeof(input));
        int movie_id = atoi(input);

        JSON_Value *movie_val = json_value_init_object();
        JSON_Object *movie_obj = json_value_get_object(movie_val);
        json_object_set_number(movie_obj, "id", movie_id);
        char *movie_body = json_serialize_to_string(movie_val);

        char url[URL_LEN];
        snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%d/movies", collection_id);

        char *add_request = compute_post_json_request2(SERVER_HOST, url, movie_body, NULL, 0, headers, 1);
        char *add_response = send_and_receive_request(add_request);
        int add_status = get_response_status(add_response);

        if (add_status == 200 || add_status == 201) {
            printf("SUCCESS: Movie ID %d added to collection #%d\n", movie_id, collection_id);
        } else {
            printf("ERROR: Failed to add movie ID %d to collection #%d (status %d)\n", movie_id, collection_id, add_status);
        }

        cleanup_request_resources(movie_val, movie_body, add_request, add_response);
    }
}

void delete_collection(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char id_str[20];
    read_input("id=", id_str, sizeof(id_str));
    int id = atoi(id_str);

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%d", id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_delete_request2(SERVER_HOST, url, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);

    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: Collection has been deleted with success!\n");
    } else if (status == 403) {
        printf("ERROR: You have no access.\n");
    } else if (status == 404) {
        printf("ERROR: ID invalid.\n");
    } else {
        printf("ERROR: Something went wrong with the deletion of the collection.\n");
    }

    cleanup_request_resources(NULL, NULL, request, response);
}

void add_movie_to_collection(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char collection_id_str[20];
    char movie_id_str[20];

    read_input("collection_id=", collection_id_str, sizeof(collection_id_str));
    read_input("movie_id=", movie_id_str, sizeof(movie_id_str));

    int collection_id = atoi(collection_id_str);
    int movie_id = atoi(movie_id_str);

    JSON_Value *root = NULL;
    char *body = build_id_body(movie_id, &root);

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%d/movies", collection_id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_post_json_request2(SERVER_HOST, url, body, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200 || status == 201) {
        printf("SUCCESS: Film added in the collection with great success!\n");
    } else if (status == 403) {
        printf("ERROR: You have no access.\n");
    } else if (status == 400) {
        printf("ERROR: Invalid data\n");
    } else {
        printf("ERROR: Something went wrong with the additon of film in the collection.\n");
    }

    cleanup_request_resources(root, body, request, response);
}

void delete_movie_from_collection(ClientState *state) {
    if (!is_user_logged_in(state)) {
        return;
    }

    if (!does_have_access(state)) {
        return;
    }

    char collection_id_str[20];
    char movie_id_str[20];

    read_input("collection_id=", collection_id_str, sizeof(collection_id_str));
    read_input("movie_id=", movie_id_str, sizeof(movie_id_str));

    int collection_id = atoi(collection_id_str);
    int movie_id = atoi(movie_id_str);

    char url[URL_LEN];
    snprintf(url, sizeof(url), "/api/v1/tema/library/collections/%d/movies/%d", collection_id, movie_id);

    char auth_header[MAX_TOKEN_LEN + 100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", state->jwt_token);
    char *headers[] = {auth_header};

    char *request = compute_delete_request2(SERVER_HOST, url, NULL, 0, headers, 1);
    char *response = send_and_receive_request(request);
    int status = get_response_status(response);

    if (status == 200) {
        printf("SUCCESS: Film deleted in the collection with great success\n");
    } else if (status == 403) {
        printf("ERROR: You have no access.\n");
    } else if (status == 404) {
        printf("ERROR: Doesnt exist.\n");
    } else {
        printf("ERROR: Something went wrong with the deletion of film in the collection.\n");
    }
    cleanup_request_resources(NULL, NULL, request, response);
}


#include "client.h"

void remove_newline(char *line);
void read_input(const char *label, char *buffer, size_t size);
void extract_cookie(const char *response, char *dest, int *flag);
char *build_json_body(const char *key1, const char *val1, const char *key2, const char *val2, JSON_Value **out_root);
int get_response_status(const char *response);
void cleanup_request_resources(JSON_Value *root, char *body, char *request, char *response);
void clear_user_session(ClientState *state);
void print_collections(const char *response);
void print_collection_details(const char *response);
char *build_collection_body(const char *title, JSON_Value **out_root);
void print_admin_not_logged_error();
int is_user_logged_in(ClientState *state);
int does_have_access(ClientState *state);
void print_already_logged_in_error();
int get_last_collection_id(const char *auth_header);
char *build_id_body(int id, JSON_Value **out_root);
void print_user_list_from_response(const char *response);
char *build_json_3_fields(const char *k1, const char *v1, const char *k2, const char *v2, const char *k3, const char *v3, JSON_Value **out_root);
void extract_token_from_response(const char *response, char *dest);
void print_movies(const char *response);
char *build_movie_json(const char *title, int year, const char *description, double rating, JSON_Value **out_root);
char *send_and_receive_request(char *request);
void print_movie_details(const char *response);
char *build_collection_json(const char *title, const char *owner, JSON_Value **out_root);





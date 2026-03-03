#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count);

char *compute_post_json_request_json(char *host, char *url, const char *json_body,
                                     char **cookies, int cookies_count);

char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count);

char *compute_get_request2(char *host, char *url, char *query_params,
                           char **cookies, int cookies_count,
                           char **headers, int headers_count);

char *compute_post_json_request2(char *host, char *url, char *json_body,
                                 char **cookies, int cookies_count,
                                 char **headers, int headers_count);

char *compute_delete_request2(char *host, char *url, char **cookies, int cookies_count, char **headers, int headers_count);

char *compute_put_json_request(const char *host, const char *url, const char *json_body,
                               char **cookies, int cookies_count,
                               char **headers, int headers_count);

#endif

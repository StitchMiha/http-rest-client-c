#include "requests.h"

#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "helpers.h"

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }

    compute_message(message, line);

    if (cookies != NULL) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
            compute_message(message, line);
        }
    }
    compute_message(message, "");

    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    if (host != NULL) {
        sprintf(line, "Host: %s", host);
    }

    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, body_data[i]);
        if (i < body_data_fields_count - 1) {
            strcat(body_data_buffer, "&");
        }
    }

    snprintf(line, LINELEN, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_post_json_request_json(char *host, char *url, const char *json_body, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: application/json");
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(json_body));
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    strcat(message, json_body);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_json_request2(char *host, char *url, char *json_body,
                                 char **cookies, int cookies_count,
                                 char **headers, int headers_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) return NULL;

    char line[LINELEN] = {0};

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    for (int i = 0; i < headers_count; i++) {
        compute_message(message, headers[i]);
    }

    compute_message(message, "Content-Type: application/json");
    sprintf(line, "Content-Length: %ld", strlen(json_body));
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    strcat(message, json_body);

    return message;
}


// I neeeded a new get_request because my first implementation didnt support the headers 

char *compute_get_request2(char *host, char *url, char *query_params, char **cookies, int cookies_count, char **headers, int headers_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) return NULL;

    char line[LINELEN] = {0};

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //Add the headers
    for (int i = 0; i < headers_count; i++) {
        compute_message(message, headers[i]);
    }

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");

    return message;
}
// Includes also headers
char *compute_delete_request2(char *host, char *url,
                                          char **cookies, int cookies_count,
                                          char **headers, int headers_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    if (!message) return NULL;

    char line[LINELEN] = {0};

    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    for (int i = 0; i < headers_count; i++) {
        compute_message(message, headers[i]);
    }

    if (cookies != NULL && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char *compute_put_json_request(const char *host, const char *url, const char *json_body,
                               char **cookies, int cookies_count,
                               char **headers, int headers_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char line[LINELEN];

    sprintf(line, "PUT %s HTTP/1.1", url);
    compute_message(message, line);

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    compute_message(message, "Content-Type: application/json");
    sprintf(line, "Content-Length: %ld", strlen(json_body));
    compute_message(message, line);

    for (int i = 0; i < headers_count; i++) {
        compute_message(message, headers[i]);
    }

    if (cookies && cookies_count > 0) {
        strcpy(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) strcat(line, "; ");
        }
        compute_message(message, line);
    }

    compute_message(message, "");
    strcat(message, json_body);

    return message;
}


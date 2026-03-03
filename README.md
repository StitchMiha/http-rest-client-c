HTTP REST Client in C

A command-line HTTP client written in C using raw TCP sockets.

Implements a REST API client with JSON payloads, session cookies (admin + user), JWT authentication, and CRUD operations for movies and collections.

Features

- Manual HTTP/1.1 request building over TCP sockets
- JSON serialization and parsing (Parson)
- Admin authentication via session cookie (Set-Cookie parsing)
- User authentication via session cookie + JWT access token
- Authorization using Authorization: Bearer <token>
- Supports GET / POST / PUT / DELETE requests
- CLI command dispatcher for interactive usage

Authentication Flow

- login_admin → stores admin session cookie
- Admin actions: add_user, get_users, delete_user, logout_admin
- login → stores user session cookie
- get_access → stores JWT token
- Library actions require JWT token in Authorization header

Commands

## Admin

- login_admin
- add_user
- get_users
- delete_user
- logout_admin

## User

- login
- logout
- get_access

## Movies

- get_movies
- get_movie
- add_movie
- update_movie
- delete_movie

## Collections

- get_collections
- get_collection
- add_collection
- delete_collection
- add_movie_to_collection
- delete_movie_from_collection

Implementation Notes

- Session state is stored in a ClientState structure:

    - admin cookie

    - user cookie

    - JWT token

    - login flags / current username

- Added request helpers that support custom headers (needed for JWT):

    - compute_get_request2, compute_post_json_request2, compute_delete_request2

- Response parsing helpers:

    - HTTP status code extraction

    - cookie extraction (Set-Cookie)

    - token extraction from JSON
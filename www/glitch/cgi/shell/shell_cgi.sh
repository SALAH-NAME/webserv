#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

HTML_FILE_PATH="$SCRIPT_DIR/shell_cgi.html"

if [ -f "$HTML_FILE_PATH" ]; then
    HTML_CONTENT=$(cat "$HTML_FILE_PATH")
    
    CONTENT_LENGTH=$(echo -n "$HTML_CONTENT" | wc -c)
    
    printf "Content-Type: text/html\r\n"
    printf "Content-Length: %s\r\n" "$CONTENT_LENGTH"
    printf "\r\n"
    
    printf "%s" "$HTML_CONTENT"
else
    ERROR_CONTENT="<html><body><h1>Error: shell_cgi.html not found</h1></body></html>"
    CONTENT_LENGTH=$(echo -n "$ERROR_CONTENT" | wc -c)
    
    printf "Content-Type: text/html\r\n"
    printf "Content-Length: %s\r\n" "$CONTENT_LENGTH"
    printf "\r\n"
    
    printf "%s" "$ERROR_CONTENT"
fi

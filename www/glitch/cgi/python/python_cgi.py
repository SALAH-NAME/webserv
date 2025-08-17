#!/usr/bin/env python3
import os

script_dir = os.path.dirname(os.path.abspath(__file__))
html_file_path = os.path.join(script_dir, "python_cgi.html")

try:
    with open(html_file_path, 'r', encoding='utf-8') as file:
        html_content = file.read()
    
    print("Content-Type: text/html", end="\r\n")
    print(f"Content-Length: {len(html_content.encode('utf-8'))}", end="\r\n")
    print("", end="\r\n")
    
    print(html_content, end="")
    
except FileNotFoundError:
    error_content = "<html><body><h1>Error: python_cgi.html not found</h1></body></html>"
    print("Content-Type: text/html", end="\r\n")
    print(f"Content-Length: {len(error_content.encode('utf-8'))}", end="\r\n")
    print("", end="\r\n")
    print(error_content, end="")
except Exception as e:
    error_content = f"<html><body><h1>Error: {str(e)}</h1></body></html>"
    print("Content-Type: text/html", end="\r\n")
    print(f"Content-Length: {len(error_content.encode('utf-8'))}", end="\r\n")
    print("", end="\r\n")
    print(error_content, end="")

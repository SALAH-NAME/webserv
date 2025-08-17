#!/usr/bin/env python3

import os
import json
import stat
import time
from datetime import datetime

def get_file_size(size_bytes):
    """Convert bytes to human readable format"""
    if size_bytes == 0:
        return "0 B"
    size_names = ["B", "KB", "MB", "GB"]
    i = 0
    while size_bytes >= 1024 and i < len(size_names) - 1:
        size_bytes /= 1024.0
        i += 1
    return f"{size_bytes:.1f} {size_names[i]}"

def get_file_type(filename):
    """Get file type based on extension"""
    ext = filename.split('.')[-1].lower() if '.' in filename else ''
    type_map = {
        'txt': 'Text File',
        'html': 'HTML File',
        'css': 'Stylesheet',
        'js': 'JavaScript',
        'json': 'JSON File',
        'py': 'Python Script',
        'md': 'Markdown',
        'jpg': 'Image',
        'jpeg': 'Image',
        'png': 'Image',
        'gif': 'Image',
        'mp4': 'Video',
        'avi': 'Video',
        'mov': 'Video',
        'wmv': 'Video',
        'webm': 'Video',
        'pdf': 'PDF Document',
        'zip': 'Archive',
        'tar': 'Archive',
        'gz': 'Archive'
    }
    return type_map.get(ext, 'File')

def main():
    print("Content-Type: application/json",end="\r\n")
    print("", end="\r\n")
    
    try:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        files_dir = os.path.join(script_dir, '../../manipulate_files')
        
        files_list = []
        
        if not os.path.exists(files_dir):
            print(json.dumps({
                "error": "manipulate_files directory not found",
                "files": []
            }))
            return
            
        if not os.path.isdir(files_dir):
            print(json.dumps({
                "error": "manipulate_files is not a directory",
                "files": []
            }))
            return
        
        try:
            for filename in os.listdir(files_dir):
                file_path = os.path.join(files_dir, filename)
                
                if os.path.isfile(file_path) and not filename.startswith('.'):
                    try:
                        stat_info = os.stat(file_path)
                        
                        file_info = {
                            "name": filename,
                            "size": get_file_size(stat_info.st_size),
                            "size_bytes": stat_info.st_size,
                            "type": get_file_type(filename),
                            "modified": datetime.fromtimestamp(stat_info.st_mtime).strftime('%Y-%m-%d %H:%M:%S'),
                            "path": f"manipulate_files/{filename}"
                        }
                        files_list.append(file_info)
                        
                    except OSError as e:
                        continue
                        
        except OSError as e:
            print(json.dumps({
                "error": f"Cannot read directory: {str(e)}",
                "files": []
            }))
            return
        
        files_list.sort(key=lambda x: x['name'].lower())
        
        result = {
            "success": True,
            "directory": "manipulate_files",
            "count": len(files_list),
            "files": files_list
        }
        
        print(json.dumps(result, indent=2))
        
    except Exception as e:
        print(json.dumps({
            "error": f"Script error: {str(e)}",
            "files": []
        }))

if __name__ == "__main__":
    main()
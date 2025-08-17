#!/usr/bin/env python3
from sys import exit, stdin, stderr
from os import mkdir, environ, access, remove, F_OK, R_OK, path, makedirs
from random import random
from urllib.parse import unquote, parse_qs
import hashlib
import json
from datetime import datetime, timedelta

users_dir = "/tmp/webserv-users"
sessions_dir = "/tmp/webserv-login-sessions"
log_dir = "/tmp/webserv"
log_file_path = path.join(log_dir, "login_logs.txt")
session_timeout = 3600

makedirs(users_dir, exist_ok=True)
makedirs(sessions_dir, exist_ok=True)
makedirs(log_dir, exist_ok=True)

if not path.exists(log_file_path):
    open(log_file_path, "w").close()

response_header = "Content-Type: text/html\r\n"
log_file = open(log_file_path, "w+")

def logger(msg, end='\n'):
    global log_file
    print(msg, file=log_file, end=end)
    log_file.flush()

def hash_password(password):
    """Hash password using SHA-256"""
    return hashlib.sha256(password.encode()).hexdigest()

def generate_session_id():
    """Generate a unique session ID"""
    return hashlib.md5(str(random() * 1000000).encode()).hexdigest()

def is_post():
    """Check if request method is POST"""
    if "REQUEST_METHOD" not in environ:
        return False
    return environ["REQUEST_METHOD"] == "POST"

def get_cookies():
    """Parse cookies from HTTP headers"""
    cookies = {}
    
    cookie_string = None
    if "HTTP_COOKIE" in environ:
        cookie_string = environ["HTTP_COOKIE"]
        logger(f"Cookie header (HTTP_COOKIE): {cookie_string}")
    elif "HTTP_cookie" in environ:
        cookie_string = environ["HTTP_cookie"]
        logger(f"Cookie header (HTTP_cookie): {cookie_string}")
    
    if cookie_string:
        for pair in cookie_string.split(';'):
            pair = pair.strip()
            if '=' in pair:
                key, value = pair.split('=', 1)
                cookies[key.strip()] = value.strip()
                logger(f"Parsed cookie: {key.strip()}={value.strip()}")
    else:
        logger("No cookie header found")
    return cookies

def create_user(username, email, password):
    """Create a new user account"""
    user_file = path.join(users_dir, f"user_{username}.json")
    
    if path.exists(user_file):
        return False, "Username already exists"
    
    user_data = {
        "username": username,
        "email": email,
        "password_hash": hash_password(password),
        "created_at": datetime.now().isoformat(),
        "notes": []
    }
    
    try:
        with open(user_file, 'w') as f:
            json.dump(user_data, f)
        logger(f"Created user: {username}")
        return True, "User created successfully"
    except Exception as e:
        logger(f"Error creating user: {str(e)}")
        return False, "Error creating user"

def authenticate_user(username, password):
    """Authenticate user credentials"""
    user_file = path.join(users_dir, f"user_{username}.json")
    
    if not path.exists(user_file):
        return False, "User not found"
    
    try:
        with open(user_file, 'r') as f:
            user_data = json.load(f)
        
        if user_data["password_hash"] == hash_password(password):
            logger(f"User authenticated: {username}")
            return True, user_data
        else:
            logger(f"Authentication failed for: {username}")
            return False, "Invalid password"
    except Exception as e:
        logger(f"Error authenticating user: {str(e)}")
        return False, "Authentication error"

def create_session(username):
    """Create a new session for user"""
    session_id = generate_session_id()
    session_file = path.join(sessions_dir, f"session_{session_id}.json")
    
    session_data = {
        "username": username,
        "created_at": datetime.now().isoformat(),
        "expires_at": (datetime.now() + timedelta(seconds=session_timeout)).isoformat()
    }
    
    try:
        with open(session_file, 'w') as f:
            json.dump(session_data, f)
        logger(f"Created session for: {username}")
        return session_id
    except Exception as e:
        logger(f"Error creating session: {str(e)}")
        return None

def validate_session(session_id):
    """Validate session and return user data"""
    session_file = path.join(sessions_dir, f"session_{session_id}.json")
    
    if not path.exists(session_file):
        return False, None
    
    try:
        with open(session_file, 'r') as f:
            session_data = json.load(f)
        
        expires_at = datetime.fromisoformat(session_data["expires_at"])
        if datetime.now() > expires_at:
            remove(session_file)
            logger("Session expired and removed")
            return False, None
        
        return True, session_data
    except Exception as e:
        logger(f"Error validating session: {str(e)}")
        return False, None

def get_user_data(username):
    """Get user data by username"""
    user_file = path.join(users_dir, f"user_{username}.json")
    
    if not path.exists(user_file):
        return None
    
    try:
        with open(user_file, 'r') as f:
            return json.load(f)
    except Exception as e:
        logger(f"Error getting user data: {str(e)}")
        return None

def save_note(username, note):
    """Save a note for the user"""
    user_file = path.join(users_dir, f"user_{username}.json")
    
    if not path.exists(user_file):
        return False
    
    try:
        with open(user_file, 'r') as f:
            user_data = json.load(f)
        
        note_entry = {
            "id": len(user_data.get("notes", [])) + 1,
            "content": note,
            "created_at": datetime.now().isoformat()
        }
        
        if "notes" not in user_data:
            user_data["notes"] = []
        
        user_data["notes"].append(note_entry)
        
        with open(user_file, 'w') as f:
            json.dump(user_data, f, indent=2)
        
        logger(f"Note saved for user: {username}")
        return True
    except Exception as e:
        logger(f"Error saving note: {str(e)}")
        return False

def delete_note(username, note_id):
    """Delete a note for the user"""
    user_file = path.join(users_dir, f"user_{username}.json")
    
    if not path.exists(user_file):
        return False
    
    try:
        with open(user_file, 'r') as f:
            user_data = json.load(f)
        
        if "notes" not in user_data:
            return False
        
        original_count = len(user_data["notes"])
        user_data["notes"] = [note for note in user_data["notes"] if note.get("id") != int(note_id)]
        
        if len(user_data["notes"]) < original_count:
            with open(user_file, 'w') as f:
                json.dump(user_data, f, indent=2)
            logger(f"Note {note_id} deleted for user: {username}")
            return True
        return False
    except Exception as e:
        logger(f"Error deleting note: {str(e)}")
        return False

def read_post_data():
    """Read and parse POST data"""
    try:
        content_length = int(environ.get('CONTENT_LENGTH', 0))
    except ValueError:
        content_length = 0
    
    if content_length > 0:
        post_data = stdin.read(content_length)
        logger(f"POST data: {post_data}")
        return parse_qs(post_data)
    return {}

def render_success_page(title, message, redirect_url=None):
    """Render success page with glitch styling"""
    redirect_script = ""
    if redirect_url:
        redirect_script = f'setTimeout(function() {{ window.location.href = "{redirect_url}"; }}, 2000);'
    
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{title} - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .success-container {{
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
        }}
        .success-card {{
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 600px;
            width: 100%;
            text-align: center;
            position: relative;
            animation: cardPulse 4s ease-in-out infinite;
        }}
        .success-card::before {{
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: linear-gradient(45deg, #00ff00, #00ffff);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0.6;
        }}
        .success-title {{
            font-size: var(--font-size-4xl);
            font-weight: var(--font-weight-extra-bold);
            color: #00ff00;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-xl);
            text-shadow: 0 0 20px #00ff00;
        }}
        .success-message {{
            font-size: var(--font-size-lg);
            color: var(--color-text-secondary);
            line-height: 1.6;
            margin-bottom: var(--spacing-2xl);
        }}
        @keyframes cardPulse {{
            0%, 100% {{ box-shadow: 0 0 20px rgba(0, 255, 0, 0.3); }}
            50% {{ box-shadow: 0 0 40px rgba(0, 255, 255, 0.3); }}
        }}
    </style>
</head>
<body>
    <div class="success-container">
        <div class="success-card">
            <h1 class="success-title">{title}</h1>
            <div class="success-message">
                <p>{message}</p>
            </div>
        </div>
    </div>
    <script>
        {redirect_script}
    </script>
</body>
</html>"""

def render_dashboard(user_data):
    """Render user dashboard with notes functionality"""
    notes_html = ""
    for note in user_data.get("notes", []):
        note_id = note.get("id", 1)
        notes_html += f"""
        <div class="note-item">
            <div class="note-main">
                <div class="note-content">{note['content']}</div>
                <div class="note-date">{note['created_at'][:19]}</div>
            </div>
            <div class="note-actions">
                <form style="display: inline;" action="/loginManager.py" method="post">
                    <input type="hidden" name="action" value="delete_note">
                    <input type="hidden" name="note_id" value="{note_id}">
                    <button type="submit" class="delete-button" onclick="return confirm('Are you sure you want to delete this note?')">Delete</button>
                </form>
            </div>
        </div>
        """
    
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .dashboard-container {{
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: var(--spacing-xl);
        }}
        .dashboard-header {{
            text-align: center;
            margin-bottom: var(--spacing-2xl);
        }}
        .dashboard-title {{
            font-size: var(--font-size-4xl);
            font-weight: var(--font-weight-extra-bold);
            color: var(--color-primary-cyan);
            text-transform: uppercase;
            letter-spacing: 0.1em;
            text-shadow: 0 0 20px var(--color-primary-cyan);
        }}
        .welcome-message {{
            font-size: var(--font-size-xl);
            color: var(--color-text-secondary);
            margin-top: var(--spacing-base);
        }}
        .dashboard-card {{
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 800px;
            width: 100%;
            margin-bottom: var(--spacing-xl);
            animation: cardPulse 6s ease-in-out infinite;
        }}
        .note-form {{
            margin-bottom: var(--spacing-2xl);
        }}
        .note-textarea {{
            width: 100%;
            padding: var(--spacing-base);
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            color: var(--color-text-primary);
            font-family: inherit;
            font-size: var(--font-size-base);
            resize: vertical;
            min-height: 120px;
        }}
        .note-textarea:focus {{
            outline: none;
            border-color: var(--color-primary-pink);
            box-shadow: 0 0 20px rgba(255, 0, 128, 0.3);
        }}
        .submit-button {{
            background: var(--gradient-primary);
            border: none;
            border-radius: var(--border-radius-base);
            padding: var(--spacing-base) var(--spacing-xl);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            font-family: inherit;
            cursor: pointer;
            text-transform: uppercase;
            transition: all var(--transition-base);
        }}
        .submit-button:hover {{
            transform: scale(1.05);
            box-shadow: var(--shadow-button);
        }}
        .notes-section {{
            margin-top: var(--spacing-2xl);
        }}
        .notes-title {{
            font-size: var(--font-size-2xl);
            color: var(--color-primary-pink);
            margin-bottom: var(--spacing-xl);
            text-shadow: 0 0 10px var(--color-primary-pink);
        }}
        .note-item {{
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            padding: var(--spacing-base);
            margin-bottom: var(--spacing-base);
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
        }}
        .note-main {{
            flex: 1;
        }}
        .note-actions {{
            display: flex;
            gap: var(--spacing-sm);
            margin-left: var(--spacing-base);
            align-items: center;
        }}
        .delete-button {{
            background: linear-gradient(45deg, #ff4444, #cc0000);
            border: none;
            border-radius: var(--border-radius-base);
            padding: var(--spacing-xs) var(--spacing-sm);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            cursor: pointer;
            text-transform: uppercase;
            transition: all var(--transition-base);
            font-size: var(--font-size-xs);
        }}
        .delete-button:hover {{
            transform: scale(1.05);
            box-shadow: 0 0 15px rgba(255, 68, 68, 0.6);
        }}
        .note-content {{
            color: var(--color-text-primary);
            font-size: var(--font-size-base);
            line-height: 1.6;
            margin-bottom: var(--spacing-sm);
        }}
        .note-date {{
            color: var(--color-text-muted);
            font-size: var(--font-size-sm);
            text-align: right;
        }}
        .logout-button {{
            background: linear-gradient(45deg, #ff4444, #ff0088);
            border: none;
            border-radius: var(--border-radius-base);
            padding: var(--spacing-sm) var(--spacing-base);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            cursor: pointer;
            text-transform: uppercase;
            transition: all var(--transition-base);
            font-size: var(--font-size-sm);
        }}
        .logout-button:hover {{
            transform: scale(1.05);
            box-shadow: 0 0 20px rgba(255, 68, 68, 0.5);
        }}
        .header-actions {{
            display: flex;
            gap: var(--spacing-base);
            margin-top: var(--spacing-base);
            justify-content: center;
            align-items: center;
        }}
        @keyframes cardPulse {{
            0%, 100% {{ box-shadow: 0 0 20px rgba(255, 0, 128, 0.2); }}
            50% {{ box-shadow: 0 0 40px rgba(0, 255, 255, 0.2); }}
        }}
    </style>
</head>
<body>
    <div class="dashboard-container">
        <div class="dashboard-header">
            <h1 class="dashboard-title">Dashboard</h1>
            <div class="welcome-message">Welcome back, {user_data['username']}!</div>
            <div class="header-actions">
                <a href="/#bonus" class="submit-button">Home</a>
                <form style="display: inline;" action="/loginManager.py" method="post">
                    <input type="hidden" name="action" value="logout">
                    <button type="submit" class="logout-button">Logout</button>
                </form>
            </div>
        </div>
        
        <div class="dashboard-card">
            <h2 class="notes-title">Add New Note</h2>
            <form class="note-form" action="/loginManager.py" method="post">
                <input type="hidden" name="action" value="add_note">
                <textarea name="note" class="note-textarea" placeholder="Write your private note here..." required></textarea>
                <br><br>
                <button type="submit" class="submit-button">Save Note</button>
            </form>
            
            <div class="notes-section">
                <h2 class="notes-title">Your Private Notes ({len(user_data.get("notes", []))} notes)</h2>
                {notes_html if notes_html else '<div class="note-item"><div class="note-content">No notes yet. Add your first note above!</div></div>'}
            </div>
        </div>
    </div>
</body>
</html>"""

def render_login_form():
    """Render login form for when accessed directly via /loginManager.py"""
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login System - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .login-container {{
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
            position: relative;
            overflow: hidden;
        }}
        .login-card {{
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 500px;
            width: 100%;
            position: relative;
            z-index: 2;
            animation: cardPulse 6s ease-in-out infinite;
        }}
        .login-card::before {{
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: var(--gradient-primary);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0;
            transition: opacity var(--transition-base);
        }}
        .login-card:hover::before {{
            opacity: 0.5;
        }}
        .login-title {{
            font-size: var(--font-size-3xl);
            font-weight: var(--font-weight-extra-bold);
            color: var(--color-primary-cyan);
            text-align: center;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-xl);
            text-shadow: 0 0 20px var(--color-primary-cyan);
        }}
        .tab-container {{
            display: flex;
            margin-bottom: var(--spacing-xl);
            border-radius: var(--border-radius-base);
            background: rgba(0, 0, 0, 0.3);
            overflow: hidden;
        }}
        .tab {{
            flex: 1;
            padding: var(--spacing-base);
            background: transparent;
            border: none;
            color: var(--color-text-secondary);
            cursor: pointer;
            transition: all var(--transition-base);
            font-weight: var(--font-weight-bold);
            text-transform: uppercase;
        }}
        .tab.active {{
            background: var(--gradient-primary);
            color: var(--color-text-primary);
        }}
        .form-group {{
            margin-bottom: var(--spacing-base);
        }}
        .form-input {{
            width: 100%;
            padding: var(--spacing-base);
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            color: var(--color-text-primary);
            font-family: inherit;
            transition: all var(--transition-base);
        }}
        .form-input:focus {{
            outline: none;
            border-color: var(--color-primary-pink);
            box-shadow: 0 0 20px rgba(255, 0, 128, 0.3);
        }}
        .submit-button {{
            width: 100%;
            padding: var(--spacing-base);
            background: var(--gradient-primary);
            border: none;
            border-radius: var(--border-radius-base);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            cursor: pointer;
            text-transform: uppercase;
            transition: all var(--transition-base);
            margin-top: var(--spacing-base);
        }}
        .submit-button:hover {{
            transform: scale(1.02);
            box-shadow: var(--shadow-button);
        }}
        .form-container {{
            display: none;
        }}
        .form-container.active {{
            display: block;
        }}
        @keyframes cardPulse {{
            0%, 100% {{ box-shadow: 0 0 20px rgba(255, 0, 128, 0.2); }}
            50% {{ box-shadow: 0 0 40px rgba(0, 255, 255, 0.2); }}
        }}
    </style>
</head>
<body>
    <div class="login-container">
        <div class="login-card">
            <h1 class="login-title">WebServ Portal</h1>
            
            <div class="tab-container">
                <button class="tab active" onclick="switchTab('login')">Login</button>
                <button class="tab" onclick="switchTab('register')">Register</button>
            </div>
            
            <!-- Login Form -->
            <div id="login-form" class="form-container active">
                <form action="/loginManager.py" method="post">
                    <input type="hidden" name="action" value="login">
                    <div class="form-group">
                        <input type="text" name="username" class="form-input" placeholder="Username" required>
                    </div>
                    <div class="form-group">
                        <input type="password" name="password" class="form-input" placeholder="Password" required>
                    </div>
                    <button type="submit" class="submit-button">Login</button>
                </form>
            </div>
            
            <!-- Register Form -->
            <div id="register-form" class="form-container">
                <form action="/loginManager.py" method="post">
                    <input type="hidden" name="action" value="register">
                    <div class="form-group">
                        <input type="text" name="username" class="form-input" placeholder="Username" required>
                    </div>
                    <div class="form-group">
                        <input type="email" name="email" class="form-input" placeholder="Email" required>
                    </div>
                    <div class="form-group">
                        <input type="password" name="password" class="form-input" placeholder="Password" required>
                    </div>
                    <div class="form-group">
                        <input type="password" name="confirm_password" class="form-input" placeholder="Confirm Password" required>
                    </div>
                    <button type="submit" class="submit-button">Register</button>
                </form>
            </div>
        </div>
    </div>
    
    <script>
        function switchTab(tabName) {{
            // Hide all forms
            document.querySelectorAll('.form-container').forEach(form => {{
                form.classList.remove('active');
            }});
            
            // Remove active class from all tabs
            document.querySelectorAll('.tab').forEach(tab => {{
                tab.classList.remove('active');
            }});
            
            // Show selected form and activate tab
            document.getElementById(tabName + '-form').classList.add('active');
            event.target.classList.add('active');
        }}
    </script>
</body>
</html>"""

def render_error_page(title, message):
    """Render error page with glitch styling"""
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{title} - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .error-container {{
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
        }}
        .error-card {{
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-2xl);
            backdrop-filter: blur(15px);
            max-width: 600px;
            width: 100%;
            text-align: center;
            position: relative;
        }}
        .error-card::before {{
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: linear-gradient(45deg, #ff4444, #ff0088);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0.6;
        }}
        .error-title {{
            font-size: var(--font-size-4xl);
            font-weight: var(--font-weight-extra-bold);
            color: #ff4444;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-xl);
            text-shadow: 0 0 20px #ff4444;
        }}
        .error-message {{
            font-size: var(--font-size-lg);
            color: var(--color-text-secondary);
            line-height: 1.6;
            margin-bottom: var(--spacing-2xl);
        }}
        .back-button {{
            background: var(--gradient-primary);
            border: none;
            border-radius: var(--border-radius-base);
            padding: var(--spacing-base) var(--spacing-xl);
            color: var(--color-text-primary);
            font-weight: var(--font-weight-bold);
            text-decoration: none;
            display: inline-block;
            text-transform: uppercase;
            transition: all var(--transition-base);
        }}
        .back-button:hover {{
            transform: scale(1.05);
            box-shadow: var(--shadow-button);
        }}
    </style>
</head>
<body>
    <div class="error-container">
        <div class="error-card">
            <h1 class="error-title">{title}</h1>
            <div class="error-message">
                <p>{message}</p>
            </div>
            <a href="/login" class="back-button">← Back to Login</a>
        </div>
    </div>
</body>
</html>"""

def main():
    """Main CGI handler"""
    logger("Login CGI started")
    
    cookies = get_cookies()
    session_id = cookies.get('session_id')
    
    if not is_post():
        if session_id:
            valid, session_data = validate_session(session_id)
            if valid:
                user_data = get_user_data(session_data['username'])
                if user_data:
                    logger(f"User already logged in: {session_data['username']}")
                    print(response_header + "\r\n", end="")
                    print(render_dashboard(user_data))
                    return
        
        print(response_header + "\r\n", end="")
        print(render_login_form())
        return
    
    if is_post():
        post_data = read_post_data()
        action = post_data.get('action', [''])[0]
        
        logger(f"POST action: {action}")
        
        if action == 'register':
            username = post_data.get('username', [''])[0]
            email = post_data.get('email', [''])[0]
            password = post_data.get('password', [''])[0]
            confirm_password = post_data.get('confirm_password', [''])[0]
            
            if password != confirm_password:
                print(response_header + "\r\n", end="")
                print(render_error_page("Registration Failed", "Passwords do not match!"))
                return
            
            if len(password) < 6:
                print(response_header + "\r\n", end="")
                print(render_error_page("Registration Failed", "Password must be at least 6 characters long!"))
                return
            
            success, message = create_user(username, email, password)
            
            if success:
                print(response_header + "\r\n", end="")
                print(render_success_page("Registration Successful", f"Welcome {username}! You can now log in.", "/login"))
            else:
                print(response_header + "\r\n", end="")
                print(render_error_page("Registration Failed", message))
        
        elif action == 'login':
            username = post_data.get('username', [''])[0]
            password = post_data.get('password', [''])[0]
            
            success, result = authenticate_user(username, password)
            
            if success:
                session_id = create_session(username)
                if session_id:
                    response_with_cookie = response_header + f"Set-Cookie: session_id={session_id}; Path=/; Max-Age=3600\r\n\r\n"
                    print(response_with_cookie, end="")
                    print(render_dashboard(result))
                else:
                    print(response_header + "\r\n", end="")
                    print(render_error_page("Login Failed", "Could not create session"))
            else:
                print(response_header + "\r\n", end="")
                print(render_error_page("Login Failed", result))
        
        elif action == 'logout':
            if session_id:
                session_file = path.join(sessions_dir, f"session_{session_id}.json")
                if path.exists(session_file):
                    remove(session_file)
                    logger("User logged out")
            
            response_with_logout = response_header + "Set-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n\r\n"
            print(response_with_logout, end="")
            print(render_success_page("Logged Out", "You have been successfully logged out.", "/login"))
        
        elif action == 'add_note':
            cookies = get_cookies()
            session_id = cookies.get('session_id')
            logger(f"Add note - session_id from cookies: {session_id}")
            
            if session_id:
                valid, session_data = validate_session(session_id)
                logger(f"Session validation result: valid={valid}, data={session_data}")
                if valid:
                    note_content = post_data.get('note', [''])[0]
                    if note_content:
                        success = save_note(session_data['username'], note_content)
                        if success:
                            user_data = get_user_data(session_data['username'])
                            print(response_header + "\r\n", end="")
                            print(render_dashboard(user_data))
                        else:
                            print(response_header + "\r\n", end="")
                            print(render_error_page("Error", "Could not save note"))
                    else:
                        print(response_header + "\r\n", end="")
                        print(render_error_page("Error", "Note content is required"))
                else:
                    response_with_logout = response_header + "Set-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n\r\n"
                    print(response_with_logout, end="")
                    print(render_error_page("Session Expired", "Please log in again."))
            else:
                print(response_header + "\r\n", end="")
                print(render_error_page("Unauthorized", "Please log in first."))
        
        elif action == 'delete_note':
            cookies = get_cookies()
            session_id = cookies.get('session_id')
            logger(f"Delete note - session_id from cookies: {session_id}")
            
            if session_id:
                valid, session_data = validate_session(session_id)
                if valid:
                    note_id = post_data.get('note_id', [''])[0]
                    if note_id:
                        success = delete_note(session_data['username'], note_id)
                        if success:
                            user_data = get_user_data(session_data['username'])
                            print(response_header + "\r\n", end="")
                            print(render_dashboard(user_data))
                        else:
                            print(response_header + "\r\n", end="")
                            print(render_error_page("Error", "Could not delete note"))
                    else:
                        print(response_header + "\r\n", end="")
                        print(render_error_page("Error", "Note ID is required"))
                else:
                    response_with_logout = response_header + "Set-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n\r\n"
                    print(response_with_logout, end="")
                    print(render_error_page("Session Expired", "Please log in again."))
            else:
                print(response_header + "\r\n", end="")
                print(render_error_page("Unauthorized", "Please log in first."))
        
        else:
            print(response_header + "\r\n", end="")
            print(render_error_page("Invalid Action", "Unknown action requested."))

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        logger(f"CGI Error: {str(e)}")
        print("Content-Type: text/html\r\n\r\n")
        print(render_error_page("Server Error", f"An error occurred: {str(e)}"))
    finally:
        if log_file:
            log_file.close()

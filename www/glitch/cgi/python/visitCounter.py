from os import environ
from datetime import datetime
from sys import stderr

response_header = "Content-Type: text/html\r\n"
timestamp = ""
counter = 0

def InitializeCookies():
	global response_header
	global timestamp
	global counter

	if counter < 0:
		counter = 0
	now = datetime.now()
	timestamp = now.strftime("%Y-%m-%d %H:%M:%S")
	response_header += f"Set-Cookie: visit-counter={counter + 1}\r\n"
	response_header += f"Set-Cookie: last-visit={timestamp}\r\n\r\n"
	return

def SetVisiCount():
	global counter
	Cookies = ""

	if "HTTP_cookie" not in environ:
		return 0
	else:
		Cookies = environ["HTTP_cookie"]
	pairs = Cookies.split(';')
	for pair in pairs:
		key_value = pair.split('=')
		if len(key_value) != 2:
			continue
		key = key_value[0].strip()
		value = key_value[1].strip()
		if key == "visit-counter":
			counter = int(value)

SetVisiCount()
InitializeCookies()
print(response_header)
print(
f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Visit Counter - WebServ</title>
    <link rel="stylesheet" href="/style.css">
    <style>
        .counter-container {{
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: var(--spacing-xl);
            position: relative;
            overflow: hidden;
        }}

        .counter-card {{
            background: var(--color-bg-glass);
            border: 2px solid var(--color-border-primary);
            border-radius: var(--border-radius-lg);
            padding: var(--spacing-3xl);
            backdrop-filter: blur(15px);
            max-width: 700px;
            width: 100%;
            text-align: center;
            position: relative;
            animation: counterPulse 4s ease-in-out infinite;
        }}

        .counter-card::before {{
            content: '';
            position: absolute;
            top: -3px;
            left: -3px;
            right: -3px;
            bottom: -3px;
            background: var(--gradient-primary);
            border-radius: var(--border-radius-lg);
            z-index: -1;
            opacity: 0.5;
            animation: borderGlow 3s ease-in-out infinite;
        }}

        .counter-card::after {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: 
                repeating-linear-gradient(
                    0deg,
                    transparent,
                    transparent 2px,
                    rgba(255, 0, 128, 0.03) 2px,
                    rgba(255, 0, 128, 0.03) 4px
                );
            pointer-events: none;
            border-radius: var(--border-radius-lg);
            animation: scanlines 2s linear infinite;
        }}

        .counter-title {{
            font-size: var(--font-size-3xl);
            font-weight: var(--font-weight-extra-bold);
            color: var(--color-text-primary);
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-2xl);
            position: relative;
            z-index: 2;
            animation: titleGlow 2s ease-in-out infinite alternate;
        }}

        .counter-title::before {{
            content: attr(data-text);
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: var(--gradient-primary);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            z-index: -1;
            animation: textShift 3s ease-in-out infinite;
        }}

        .visit-counter {{
            font-size: var(--font-size-5xl);
            font-weight: var(--font-weight-extra-bold);
            color: var(--color-primary-cyan);
            text-shadow: 
                0 0 20px var(--color-primary-cyan),
                0 0 40px var(--color-primary-cyan);
            margin: var(--spacing-xl) 0;
            position: relative;
            z-index: 2;
            animation: 
                numberGlow 2s ease-in-out infinite alternate,
                numberFloat 4s ease-in-out infinite;
            display: block;
            text-align: center;
        }}

        .visit-counter::before {{
            content: '';
            display: none;
        }}

        .visit-text {{
            font-size: var(--font-size-xl);
            color: var(--color-text-secondary);
            margin-bottom: var(--spacing-xl);
            position: relative;
            z-index: 2;
            animation: textFade 3s ease-in-out infinite;
            text-align: center;
            line-height: 1.5;
        }}

        .last-visit-container {{
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--color-border-primary);
            border-radius: var(--border-radius-base);
            padding: var(--spacing-xl);
            margin-top: var(--spacing-xl);
            position: relative;
            z-index: 2;
            backdrop-filter: blur(10px);
            text-align: center;
        }}

        .last-visit-label {{
            font-size: var(--font-size-sm);
            color: var(--color-text-muted);
            text-transform: uppercase;
            letter-spacing: 0.1em;
            margin-bottom: var(--spacing-sm);
            font-weight: var(--font-weight-bold);
        }}

        .last-visit-time {{
            font-size: var(--font-size-lg);
            color: var(--color-primary-pink);
            font-weight: var(--font-weight-bold);
            font-family: var(--font-family-primary);
            text-shadow: 0 0 10px var(--color-primary-pink);
            animation: timeGlow 2s ease-in-out infinite alternate;
            display: block;
            text-align: center;
        }}

        .back-link, .refresh-button, .reset-button {{
            display: inline-block;
            margin: var(--spacing-base);
            padding: var(--spacing-base) var(--spacing-xl);
            border-radius: var(--border-radius-base);
            color: var(--color-text-primary);
            text-decoration: none;
            font-family: inherit;
            font-size: var(--font-size-base);
            font-weight: var(--font-weight-bold);
            text-transform: uppercase;
            letter-spacing: 0.05em;
            transition: all var(--transition-base);
            position: relative;
            z-index: 10;
            cursor: pointer;
            border: none;
            min-width: 140px;
            text-align: center;
        }}

        .back-link {{
            background: var(--gradient-primary);
            overflow: hidden;
        }}

        .back-link::before {{
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: var(--gradient-primary-reverse);
            transition: left var(--transition-base);
            z-index: -1;
        }}

        .back-link:hover::before {{
            left: 0;
        }}

        .back-link:hover {{
            transform: scale(1.05) translateY(-2px);
            box-shadow: var(--shadow-button);
        }}

        .refresh-button, .reset-button {{
            background: var(--color-bg-glass);
            border: 1px solid var(--color-border-primary);
            color: var(--color-text-secondary);
            backdrop-filter: blur(10px);
        }}

        .reset-button {{
            background: linear-gradient(45deg, #ff4444, #ff0088);
            color: var(--color-text-primary);
            border: 1px solid #ff4444;
        }}

        .refresh-button:hover, .reset-button:hover {{
            transform: scale(1.05) translateY(-2px);
            box-shadow: 0 0 20px rgba(255, 0, 128, 0.4);
        }}

        .refresh-button:hover {{
            color: var(--color-primary-cyan);
            border-color: var(--color-primary-cyan);
            box-shadow: 0 0 15px rgba(0, 255, 255, 0.3);
        }}

        .reset-button:hover {{
            background: linear-gradient(45deg, #ff6666, #ff00aa);
            box-shadow: 0 0 20px rgba(255, 68, 68, 0.5);
        }}

        .button-container {{
            text-align: center;
            margin-top: var(--spacing-2xl);
            position: relative;
            z-index: 10;
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: var(--spacing-base);
        }}

        /* Animations */
        @keyframes counterPulse {{
            0%, 100% {{ box-shadow: 0 0 20px rgba(255, 0, 128, 0.2); }}
            50% {{ box-shadow: 0 0 40px rgba(0, 255, 255, 0.3); }}
        }}

        @keyframes borderGlow {{
            0%, 100% {{ opacity: 0.5; }}
            50% {{ opacity: 0.8; }}
        }}

        @keyframes titleGlow {{
            0%, 100% {{ text-shadow: 0 0 20px rgba(255, 255, 255, 0.5); }}
            50% {{ text-shadow: 0 0 30px var(--color-primary-pink), 0 0 40px var(--color-primary-cyan); }}
        }}

        @keyframes numberGlow {{
            0%, 100% {{ 
                text-shadow: 
                    0 0 20px var(--color-primary-cyan),
                    0 0 40px var(--color-primary-cyan);
            }}
            50% {{ 
                text-shadow: 
                    0 0 30px var(--color-primary-cyan),
                    0 0 50px var(--color-primary-cyan),
                    0 0 70px var(--color-primary-pink);
            }}
        }}

        @keyframes numberFloat {{
            0%, 100% {{ transform: translateY(0px); }}
            50% {{ transform: translateY(-10px); }}
        }}

        @keyframes textShift {{
            0%, 100% {{ transform: translate(0, 0); }}
            25% {{ transform: translate(-1px, 1px); }}
            50% {{ transform: translate(1px, -1px); }}
            75% {{ transform: translate(-1px, -1px); }}
        }}

        @keyframes glitchShift {{
            0%, 100% {{ transform: translate(0, 0); opacity: 0.3; }}
            50% {{ transform: translate(-2px, 2px); opacity: 0.6; }}
        }}

        @keyframes textFade {{
            0%, 100% {{ opacity: 0.8; }}
            50% {{ opacity: 1; }}
        }}

        @keyframes timeGlow {{
            0%, 100% {{ text-shadow: 0 0 10px var(--color-primary-pink); }}
            50% {{ text-shadow: 0 0 20px var(--color-primary-pink), 0 0 30px var(--color-primary-cyan); }}
        }}

        @keyframes scanlines {{
            0% {{ transform: translateY(-100%); }}
            100% {{ transform: translateY(100vh); }}
        }}

        /* Responsive design */
        @media (max-width: 768px) {{
            .counter-container {{
                padding: var(--spacing-base);
            }}
            
            .counter-card {{
                padding: var(--spacing-2xl);
                max-width: 100%;
            }}
            
            .visit-counter {{
                font-size: var(--font-size-4xl);
            }}
            
            .counter-title {{
                font-size: var(--font-size-2xl);
            }}
        }}
    </style>
</head>
<body>
    <div class="counter-container">
        <div class="counter-card">
            <h1 class="counter-title" data-text="Visit Counter">Visit Counter</h1>
            
            <div class="visit-counter">{counter + 1}</div>
            
            <div class="visit-text">
                You have visited this page <strong>{counter + 1}</strong> times!
            </div>
            
            <div class="last-visit-container">
                <div class="last-visit-label">Last Visit</div>
                <div class="last-visit-time">{timestamp}</div>
            </div>
            
            <div class="button-container">
                <a href="/#bonus" class="back-link">← Back to Home</a>
                <button onclick="window.location.reload()" class="refresh-button">↻ Refresh Counter</button>
                <button onclick="resetCounter()" class="reset-button">🔄 Reset Counter</button>
            </div>
        </div>
    </div>
    
    <script>
        function resetCounter() {{
            // Clear the visit counter and last visit cookies
            document.cookie = "visit-counter=0; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT";
            document.cookie = "last-visit=; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT";
            
            // Show confirmation and reload
            if (confirm("Are you sure you want to reset the visit counter to 0?")) {{
                setTimeout(function() {{
                    window.location.reload();
                }}, 100);
            }}
        }}
        
        // Add some interactive effects
        document.addEventListener('DOMContentLoaded', function() {{
            const buttons = document.querySelectorAll('.back-link, .refresh-button, .reset-button');
            buttons.forEach(button => {{
                button.addEventListener('mouseenter', function() {{
                    this.style.transform = 'scale(1.05) translateY(-2px)';
                }});
                button.addEventListener('mouseleave', function() {{
                    this.style.transform = 'scale(1) translateY(0)';
                }});
            }});
        }});
    </script>
</body>
</html>
"""
)
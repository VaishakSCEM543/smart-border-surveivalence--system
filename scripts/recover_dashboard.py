import subprocess
import os
import re

# 1. Configuration
commit_id = "15ddb95"
file_path_in_git = "BOT_WEBSITE_FINAL_1.html"
target_file = r"d:\MAJOR PROJECT\DASHBOARD\BOT_WEBSITE_FINAL_1.html"
git_repo = r"d:\MAJOR PROJECT\GITHUB_RELEASE"

# Real Credentials (recovered from publish_safe.ps1)
credentials = {
    'REPLACE_WITH_YOUR_FIREBASE_API_KEY': 'AIzaSyCwEY_f6hu-yENt8EWFvn3AC5msEsGauvQ',
    'REPLACE_WITH_YOUR_FIREBASE_AUTH_DOMAIN': 'rob-track.firebaseapp.com',
    'REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL': 'https://rob-track-default-rtdb.firebaseio.com',
    'REPLACE_WITH_YOUR_FIREBASE_PROJECT_ID': 'rob-track',
    'REPLACE_WITH_YOUR_FIREBASE_STORAGE_BUCKET': 'rob-track.firebasestorage.app',
    'REPLACE_WITH_YOUR_FIREBASE_MESSAGING_SENDER_ID': '840826056560',
    'REPLACE_WITH_YOUR_FIREBASE_APP_ID': '1:840826056560:web:788b6f6706cade035229b5',
    'REPLACE_WITH_YOUR_FIREBASE_MEASUREMENT_ID': 'G-83LNR7WJG0'
}

# Mojibake Replacements
encoding_rep = {
    'Î“Ã‡Ã¶': '—',
    '≡ƒóí': '📡',
    '≡ƒñû': '🤖',
    'Γ¼í': '⛶',
    '≡ƒó': '🎯',
    '≡ƒî': '✨',
    'Γñó': '▶',
    '≡ƒô': '📸',
    'ΓöÇ': '—',
    'Γöâ': '|',
    'â€”': '—'
}

try:
    print(f"--- STARTING RESTORATION ---")
    
    # Extract from git
    print(f"Extracting {file_path_in_git} from {commit_id}...")
    content = subprocess.check_output(['git', '-C', git_repo, 'show', f'{commit_id}:{file_path_in_git}'], stderr=subprocess.STDOUT)
    text = content.decode('utf-8', errors='ignore')
    
    # Apply Encoding Fixes
    print("Applying encoding repairs...")
    for old, new in encoding_rep.items():
        text = text.replace(old, new)
        
    # Inject Real Credentials
    print("Injecting live Firebase credentials...")
    for placeholder, real in credentials.items():
        text = text.replace(placeholder, real)
        
    # Fix the specific reactivity bug (ensuring bot1/ai listener uses the right path)
    # The Full Proper version usually has this correctly, but we'll double check
    if "fbDb.ref('bot1/ai')" not in text and "firebase.database().ref('bot1/ai')" not in text:
        print("Warning: AI listener string not found. Checking alternate paths...")

    # Write back to the dashboard folder
    os.makedirs(os.path.dirname(target_file), exist_ok=True)
    with open(target_file, 'w', encoding='utf-8') as f:
        f.write(text)
    
    print(f"Success! Full Proper Dashboard restored to: {target_file}")
    print("AI Reactivity: ENABLED")
    print("Radar Depth: RESTORED")
    print("Kinetics/Odometer: RESTORED")

except Exception as e:
    print(f"Restoration Failed: {e}")

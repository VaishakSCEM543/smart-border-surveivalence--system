import { initializeApp } from "https://www.gstatic.com/firebasejs/10.8.1/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/10.8.1/firebase-database.js";

// TODO: Replace with your actual Firebase config
const firebaseConfig = {
  apiKey: "YOUR_API_KEY",
  authDomain: "YOUR_PROJECT_ID.firebaseapp.com",
  databaseURL: "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com",
  projectId: "YOUR_PROJECT_ID",
  storageBucket: "YOUR_PROJECT_ID.appspot.com",
  messagingSenderId: "YOUR_MESSAGING_SENDER_ID",
  appId: "YOUR_APP_ID"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

let currentBot = 'bot1';

// References to UI elements
const cameraFrame = document.getElementById('camera-stream');
const personDetectedEl = document.getElementById('ai-person');
const aiConfidenceEl = document.getElementById('ai-confidence');
const radarPresenceEl = document.getElementById('sensor-radar');
const satellitesEl = document.getElementById('sensor-satellites');
const botStatusEl = document.getElementById('bot-status');

// Google Maps Setup
let map;
let marker;
let pathCoordinates = [];
let pathPolyline;

window.initMap = function() {
    map = new google.maps.Map(document.getElementById('map'), {
        zoom: 18,
        center: { lat: 0, lng: 0 },
        disableDefaultUI: true, // cleaner look for dashboard
        styles: [
            { elementType: "geometry", stylers: [{ color: "#242f3e" }] },
            { elementType: "labels.text.stroke", stylers: [{ color: "#242f3e" }] },
            { elementType: "labels.text.fill", stylers: [{ color: "#746855" }] },
            { featureType: "road", elementType: "geometry", stylers: [{ color: "#38414e" }] },
            { featureType: "road", elementType: "geometry.stroke", stylers: [{ color: "#212a37" }] },
            { featureType: "water", elementType: "geometry", stylers: [{ color: "#17263c" }] }
          ]
    });

    marker = new google.maps.Marker({
        map: map,
        title: 'Active Bot',
        icon: {
            path: google.maps.SymbolPath.CIRCLE,
            scale: 8,
            fillColor: "#00ffcc",
            fillOpacity: 1,
            strokeWeight: 2,
            strokeColor: "#ffffff"
        }
    });

    pathPolyline = new google.maps.Polyline({
        path: pathCoordinates,
        geodesic: true,
        strokeColor: '#00ffcc',
        strokeOpacity: 0.5,
        strokeWeight: 2,
        map: map
    });

    // Initial setup listener
    setupListeners(currentBot);
};

// Update map location and draw path trail
function updateMapLocation(lat, lng) {
    if(!map || !marker) return;
    
    // Ignore invalid default coords
    if (lat === 0 && lng === 0) return;
    if (lat === undefined || lng === undefined) return;
    
    const pos = { lat: parseFloat(lat), lng: parseFloat(lng) };
    
    marker.setPosition(pos);
    map.panTo(pos);
    
    pathCoordinates.push(pos);
    pathPolyline.setPath(pathCoordinates);
}

// Store active listeners to turn them off on bot switch
let activeListeners = [];

function setupListeners(botId) {
    // Clear old path and active listeners
    pathCoordinates = [];
    if(pathPolyline) pathPolyline.setPath(pathCoordinates);
    
    activeListeners.forEach(unsubscribe => unsubscribe());
    activeListeners = [];

    // Reset UI placeholders
    cameraFrame.src = "";
    personDetectedEl.textContent = "--";
    aiConfidenceEl.textContent = "--";
    radarPresenceEl.textContent = "--";
    satellitesEl.textContent = "--";
    botStatusEl.textContent = `SYNCED: ${botId.toUpperCase()}`;

    // === GPS Listener (Both Bots) ===
    const gpsRef = ref(db, `${botId}/gps`);
    const unsubGps = onValue(gpsRef, (snapshot) => {
        const data = snapshot.val();
        if (data) {
            updateMapLocation(data.latitude, data.longitude);
            if(data.satellites !== undefined) {
                satellitesEl.textContent = data.satellites;
            }
            // For bot 2 without satellites in schema, we can leave it as --
        }
    });
    activeListeners.push(unsubGps);

    // === BOT 1 Specific Listeners ===
    if (botId === 'bot1') {
        // AI Analytics
        const aiRef = ref(db, 'bot1/ai');
        const unsubAi = onValue(aiRef, (snapshot) => {
            const data = snapshot.val();
            if (data) {
                personDetectedEl.textContent = data.personDetected ? "YES" : "NO";
                personDetectedEl.style.color = data.personDetected ? "#ff3344" : "#00ffcc";
                aiConfidenceEl.textContent = `${data.confidence}%`;
            }
        });
        activeListeners.push(unsubAi);

        // Radar Sensor
        const radarRef = ref(db, 'bot1/radar/presence');
        const unsubRadar = onValue(radarRef, (snapshot) => {
            const presence = snapshot.val();
            radarPresenceEl.textContent = presence ? "DETECTED" : "CLEAR";
            radarPresenceEl.style.color = presence ? "#ff3344" : "#00ffcc";
        });
        activeListeners.push(unsubRadar);

        // Camera Feed
        const cameraRef = ref(db, 'bot1/camera/stream_url');
        const unsubCamera = onValue(cameraRef, (snapshot) => {
            const url = snapshot.val();
            if (url && cameraFrame.src !== url) {
                cameraFrame.src = url;
            }
        });
        activeListeners.push(unsubCamera);
        
        // Ensure panels are bright
        document.getElementById('ai-panel').style.opacity = '1';
        document.getElementById('camera-panel').style.opacity = '1';
    } 
    // === BOT 2 Specific Listeners ===
    else if (botId === 'bot2') {
        const networkRef = ref(db, 'bot2/network/mesh_status');
        const unsubNetwork = onValue(networkRef, (snapshot) => {
            const status = snapshot.val();
            radarPresenceEl.textContent = status || "--";
            radarPresenceEl.style.color = "#00ffcc";
            satellitesEl.textContent = "--";
        });
        activeListeners.push(unsubNetwork);
        
        // Dim unavailable panels for Bot 2
        document.getElementById('ai-panel').style.opacity = '0.3';
        document.getElementById('camera-panel').style.opacity = '0.3';
    }
}

// Bot Switcher 
window.switchBot = function(botId) {
    currentBot = botId;
    
    // Update Tab Styles
    document.querySelectorAll('.bot-tab').forEach(tab => {
        tab.classList.remove('active');
    });
    document.getElementById(`tab-${botId}`).classList.add('active');
    
    // Re-bind Firebase setup map
    setupListeners(botId);
}

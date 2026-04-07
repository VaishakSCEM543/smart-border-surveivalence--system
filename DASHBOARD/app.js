import { database } from "./firebase-config.js";
import { ref, onValue, off } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-database.js";

// DOM Elements
const d = {
    botSelector: document.getElementById('bot-selector'),
    networkStatus: document.getElementById('network-status'),
    indicator: document.querySelector('.status-indicator'),
    
    // GPS
    lat: document.getElementById('val-lat'),
    lng: document.getElementById('val-lng'),
    gpsTime: document.getElementById('gps-timestamp'),
    
    // Camera
    stream: document.getElementById('camera-stream'),
    
    // AI
    person: document.getElementById('val-person'),
    confidence: document.getElementById('val-confidence'),
    aiTime: document.getElementById('ai-timestamp'),
    
    // Sensors
    radar: document.getElementById('val-radar'),
    sats: document.getElementById('val-sats'),
    mesh: document.getElementById('val-mesh')
};

// Map State
let map;
let marker;
let pathLine;
let pathCoordinates = [];

function initMap() {
    // Initialize Leaflet Map (Dark theme tiles via CartoDB)
    map = L.map('map', { zoomControl: false }).setView([0, 0], 2);
    
    L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
        attribution: '&copy; OpenStreetMap contributors &copy; CARTO',
        subdomains: 'abcd',
        maxZoom: 20
    }).addTo(map);

    // Add UI Controls
    L.control.zoom({ position: 'bottomright' }).addTo(map);

    // Custom tactical marker icon
    const tacticalIcon = L.divIcon({
        className: 'tactical-marker',
        html: `<div style="width: 16px; height: 16px; background: #ef4444; border: 2px solid white; border-radius: 50%; box-shadow: 0 0 10px #ef4444;"></div>`,
        iconSize: [16, 16],
        iconAnchor: [8, 8]
    });

    marker = L.marker([0, 0], { icon: tacticalIcon }).addTo(map);
    pathLine = L.polyline([], { color: '#ef4444', weight: 3, dashArray: '5, 5' }).addTo(map);
}

// Data Binding
let currentListenerRef = null;

function bindBotData(botId) {
    // Disconnect old listener
    if (currentListenerRef) {
        off(currentListenerRef);
    }

    // Reset Map Trail
    pathCoordinates = [];
    pathLine.setLatLngs([]);
    d.indicator.className = 'status-indicator offline';
    d.networkStatus.innerText = 'Connecting...';

    // Hook new listener
    currentListenerRef = ref(database, botId);
    
    onValue(currentListenerRef, (snapshot) => {
        const data = snapshot.val();
        
        if (!data) {
            d.indicator.className = 'status-indicator offline';
            d.networkStatus.innerText = 'No Data Signal';
            return;
        }

        d.indicator.className = 'status-indicator online';
        d.networkStatus.innerText = 'System Online';

        updateUI(data);
    });
}

function updateUI(data) {
    // 1. GPS UPDATES
    if (data.gps && data.gps.latitude !== undefined && data.gps.longitude !== undefined) {
        const lat = parseFloat(data.gps.latitude);
        const lng = parseFloat(data.gps.longitude);
        
        d.lat.innerText = lat.toFixed(5);
        d.lng.innerText = lng.toFixed(5);
        d.gpsTime.innerText = data.gps.timestamp || new Date().toLocaleTimeString();

        // Update Map Marker
        const newPos = [lat, lng];
        marker.setLatLng(newPos);
        
        // Push path history and auto-pan if recent
        pathCoordinates.push(newPos);
        pathLine.setLatLngs(pathCoordinates);
        
        // Only pan if it's the first fix, or occasionally, to prevent annoying user map drag overriding
        if (pathCoordinates.length === 1) {
            map.setView(newPos, 16);
        }
        
        d.sats.innerText = data.gps.satellites ?? "N/A";
    } else {
        d.lat.innerText = "NO FIX";
        d.lng.innerText = "NO FIX";
        d.sats.innerText = "N/A";
    }

    // 2. CAMERA UPDATES
    if (data.camera && data.camera.stream_url) {
        // Only override if the URL actually changed to prevent iframe/img flickering
        if (d.stream.src !== data.camera.stream_url) {
            d.stream.src = data.camera.stream_url;
        }
    } else {
        d.stream.src = ""; // Offline
    }

    // 3. AI UPDATES
    if (data.ai) {
        d.aiTime.innerText = data.ai.timestamp || "--:--:--";
        
        const detected = String(data.ai.personDetected).toUpperCase() === 'YES' || data.ai.personDetected === true;
        
        if (detected) {
            d.person.innerText = "YES";
            d.person.className = "text-red";
        } else {
            d.person.innerText = "NO";
            d.person.className = "text-green";
        }

        d.confidence.innerText = data.ai.confidence ? `${data.ai.confidence}%` : "N/A";
    } else {
        d.person.innerText = "OFFLINE";
        d.person.className = "text-secondary";
        d.confidence.innerText = "--%";
        d.aiTime.innerText = "--:--:--";
    }

    // 4. RADAR & MESH
    if (data.radar && data.radar.presence) {
        d.radar.innerText = "DETECTED";
        d.radar.className = "text-red";
    } else {
        d.radar.innerText = "CLEAR";
        d.radar.className = "text-green";
    }

    if (data.network && data.network.mesh_status) {
        d.mesh.innerText = data.network.mesh_status;
        d.mesh.className = "text-blue";
    } else {
        d.mesh.innerText = "N/A";
        d.mesh.className = "";
    }
}

// Event Listeners
d.botSelector.addEventListener('change', (e) => {
    bindBotData(e.target.value);
});

// Boot Sequence
initMap();
bindBotData('bot1');

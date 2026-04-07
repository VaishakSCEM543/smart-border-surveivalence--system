import { initializeApp } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-app.js";
import { getDatabase } from "https://www.gstatic.com/firebasejs/10.8.0/firebase-database.js";

const firebaseConfig = {
    apiKey: "REPLACE_WITH_YOUR_FIREBASE_API_KEY",
    authDomain: "REPLACE_WITH_YOUR_FIREBASE_AUTH_DOMAIN",
    databaseURL: "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL",
    projectId: "REPLACE_WITH_YOUR_FIREBASE_PROJECT_ID",
    storageBucket: "REPLACE_WITH_YOUR_FIREBASE_STORAGE_BUCKET",
    messagingSenderId: "REPLACE_WITH_YOUR_FIREBASE_MESSAGING_SENDER_ID",
    appId: "REPLACE_WITH_YOUR_FIREBASE_APP_ID",
    measurementId: "REPLACE_WITH_YOUR_FIREBASE_MEASUREMENT_ID"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
export const database = getDatabase(app);

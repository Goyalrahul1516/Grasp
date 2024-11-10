// Import the functions you need from the SDKs you need
import { initializeApp } from "https://www.gstatic.com/firebasejs/10.13.0/firebase-app.js";
import { getAuth, signInWithEmailAndPassword, signOut } from "https://www.gstatic.com/firebasejs/10.13.0/firebase-auth.js";
import { getFirestore, collection, getDocs , query, where} from "https://www.gstatic.com/firebasejs/10.13.0/firebase-firestore.js";

// Your web app's Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyAm0LK6poFHw06ZrYaXzNXvZqv5D9sdMOo",
    authDomain: "grasp-6b2ea.firebaseapp.com",
    projectId: "grasp-6b2ea",
    storageBucket: "grasp-6b2ea.appspot.com",
    messagingSenderId: "185235518954",
    appId: "1:185235518954:web:79a723cd10ee60fc212e93",
    measurementId: "G-3HS7NJ34JB"
};
        
// Initialize Firebase
const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const db = getFirestore(app);

export { auth, signInWithEmailAndPassword, db, collection, getDocs, signOut, query, where };
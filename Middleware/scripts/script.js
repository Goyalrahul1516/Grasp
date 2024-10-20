import { auth, signInWithEmailAndPassword, signOut} from '.././scripts/firebase.js'; // Import Firebase-related functions from the new file

// Handle login form submission
document.getElementById('login-form')?.addEventListener('submit', function (e) {
    e.preventDefault();
    
    var email = document.getElementById('email')?.value;
    var password = document.getElementById('password')?.value;
    
    signInWithEmailAndPassword(auth, email, password)
    .then((userCredential) => {
        // Signed in
        var user = userCredential.user;
        alert('Login Successful');
        window.location.href = "selection.html"; // Redirect to the new page
    })
    .catch((error) => {
        var errorCode = error.code;
        var errorMessage = error.message;
        alert('Error: ' + errorMessage);
    });
});

// Handle logout button click
document.getElementById('logout-btn')?.addEventListener('click', () => {
    signOut(auth).then(() => {
        // Sign-out successful.
        alert('Logged out successfully');
        window.location.href = "index.html"; // Redirect to login page or any other page
    }).catch((error) => {
        // An error happened.
        console.error('Logout Error:', error);
        alert('Error logging out: ' + error.message);
    });
});

// Handle back button click
document.getElementById('back-btn')?.addEventListener('click', () => {
    window.location.href = "selection.html"; // Redirect to the new page
});

// Handle dashboard button click
document.getElementById('dashboard-button')?.addEventListener('click', () => {
    window.location.href = "dashboard.html"; // Redirect to the new page
});

// Handle database button click
document.getElementById('database-button')?.addEventListener('click', () => {
    window.location.href = "firestore.html"; // Redirect to the new page
});


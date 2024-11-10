import { db, collection, getDocs, query, where } from '.././scripts/firebase.js';

async function fetchGrainEntries() {
    try {
        // Get the storage location ID from the URL query parameters
        const urlParams = new URLSearchParams(window.location.search);
        const storageLocationID = urlParams.get('storageLocationID');

        if (!storageLocationID) {
            throw new Error('Storage Location ID is missing from the URL.');
        }

        // Fetch grain entries
        const grainEntriesRef = collection(db, 'grainentries');
        const q = query(grainEntriesRef, where('storageLocationID', '==', storageLocationID));
        const grainEntriesSnapshot = await getDocs(q);
        const locationsRef = collection(db, 'storagelocations');
        const x = query(locationsRef, where('storageLocationID', '==', storageLocationID));
        const locationSnapshot = await getDocs(x);
        const doc = locationSnapshot.docs[0];
        const locationData = doc.data();

        // Prepare HTML output
        let locationname = locationData.locationName;
        let output = '<table><tr><th>Batch Number</th><th>Initial Weight(Kg.)</th><th>RFID Tag</th><th>Package Date</th></tr>';
        grainEntriesSnapshot.forEach((doc) => {
            const entry = doc.data();
            const packageDate = entry.packagedate ? new Date(entry.packagedate) : null; // Handle ISO 8601 string format
            const now = new Date();
            const ageInMonths = packageDate ? (now - packageDate) / (1000 * 60 * 60 * 24 * 30) : 0;

            // Determine row color based on age
            let rowColor = '';
            if (ageInMonths > 6) {
                rowColor = 'style="background-color: #da5353;"';
            } else if (ageInMonths > 3) {
                rowColor = 'style="background-color: #ef9c9c;"';
            }

            output += `<tr ${rowColor}>
                <td>${entry.batchnumber || 'N/A'}</td>
                <td>${entry.initialweight || 'N/A'}</td>
                <td>${entry.rfidtag || 'N/A'}</td>
                <td>${packageDate ? packageDate.toLocaleDateString() : 'N/A'}</td>
            </tr>`;
        });
        output += '</table>';

        // Display the data in the HTML element with ID 'grainentries-data'
        document.getElementById('grainentries-data').innerHTML = output;
        document.getElementById('title').innerHTML = locationname; // Show the table

    } catch (error) {
        console.error('Error fetching data:', error);
        alert('Failed to fetch data from Firestore. Check console for details.');
    }
}

// Call the function to fetch and display data on page load
fetchGrainEntries();

// Add functionality for back button
document.getElementById('back-btn').addEventListener('click', () => {
    window.location.href = 'firestore.html'; // Navigate back to firestore.html
});

setInterval(() => {
    location.reload();
}, 30000);

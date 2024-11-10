import { db, collection, getDocs } from '.././scripts/firebase.js';

async function fetchStorageData() {
    try {
        // Fetch storage locations
        const locationsSnapshot = await getDocs(collection(db, 'storagelocations'));

        // Map storage locations by their storageLocationID
        const locationData = {};
        locationsSnapshot.forEach((doc) => {
            const location = doc.data();
            locationData[location.storageLocationID] = {
                name: location.locationName || 'Unknown Location',
                totalweight: 0, // Initialize total weight
                numberOfsacks: 0 // Initialize sacks count
            };
        });

        // Fetch grain entries
        const grainEntriesSnapshot = await getDocs(collection(db, 'grainentries'));
        grainEntriesSnapshot.forEach((doc) => {
            const entry = doc.data();
            const storageLocationID = entry.storageLocationID; // Use storageLocationID

            console.log(`Grain Entry Location ID: ${storageLocationID}`); // Debugging line
            console.log(`Grain Entry Weight: ${entry.initialweight}`); // Debugging line

            if (locationData[storageLocationID]) {
                locationData[storageLocationID].numberOfsacks += 1; // Count every grain entry as a sack
                locationData[storageLocationID].totalweight += parseFloat(entry.initialweight) || 0; // Sum the weights
            } else {
                console.warn(`No matching location found for ID: ${storageLocationID}`); // Debugging line
            }
        });

        // Prepare HTML output
        let output = '<table><tr><th>Storage Location</th><th>Total Weight (Kg.)</th><th>Number of Sacks</th></tr>';
        for (const id in locationData) {
            output += `<tr onclick="window.location.href='grainentries.html?storageLocationID=${id}'">
                <td>${locationData[id].name}</td>
                <td>${locationData[id].totalweight.toFixed(2)}</td> <!-- Fixed to 2 decimal places -->
                <td>${locationData[id].numberOfsacks}</td>
            </tr>`;
        }
        output += '</table>';

        // Display the data in the HTML element with ID 'storage-data'
        document.getElementById('storage-data').innerHTML = output;

    } catch (error) {
        console.error('Error fetching data:', error);
        alert('Failed to fetch data from Firestore. Check console for details.');
    }
}

// Call the function to fetch and display data on page load
fetchStorageData();

// Refresh the page every 30 seconds (30000 milliseconds)
setInterval(() => {
    location.reload();
}, 30000);
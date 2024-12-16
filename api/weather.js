export default async function handler(req, res) {
    try {
        // Fetch data from the sensor's API endpoint
        const response = await fetch("http://192.168.0.120/"); // Your sensor API URL

        // Ensure that the response is in JSON format
        if (!response.ok) {
            throw new Error('Failed to fetch sensor data');
        }

        const sensorData = await response.json();

        // Send the sensor data as a JSON response to the frontend
        res.status(200).json(sensorData);
    } catch (error) {
        console.error("Error fetching sensor data:", error);
        res.status(500).json({ error: 'Failed to fetch sensor data' });
    }
}

export default async function handler(req, res) {
    try {
        // Fetch data from your local sensor API or external API
        const response = await fetch("http://192.168.0.120/"); // Your local sensor API URL
        const sensorData = await response.json();

        // Return the sensor data as a JSON response
        res.status(200).json(sensorData);
    } catch (error) {
        console.error("Error fetching sensor data:", error);
        res.status(500).json({ error: 'Failed to fetch sensor data' });
    }
}
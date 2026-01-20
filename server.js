const express = require('express');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Simple parking data (array of spots: true = free, false = taken)
let parkingSpots = Array(10).fill(true); // 10 spots, all free at start

app.use(express.static('public')); // Serve frontend files

// API to get spots
app.get('/api/spots', (req, res) => {
  res.json(parkingSpots);
});

// API to book a spot (we didn't check if it's free—oops!)
app.post('/api/book/:id', (req, res) => {
  const id = parseInt(req.params.id);
  if (id >= 0 && id < parkingSpots.length) {
    parkingSpots[id] = false; // Mark as taken
    io.emit('update', parkingSpots); // Send live update to all clients
    res.send('Booked!');
  } else {
    res.status(400).send('Invalid spot');
  }
});

// Simulate IoT: Randomly change spots every 5 seconds (like sensors detecting cars)
setInterval(() => {
  const randomSpot = Math.floor(Math.random() * parkingSpots.length);
  parkingSpots[randomSpot] = !parkingSpots[randomSpot]; // Toggle free/taken
  io.emit('update', parkingSpots); // Live update
}, 5000);

server.listen(3000, () => {
  console.log('Server running on http://localhost:3000');
});
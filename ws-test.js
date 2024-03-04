const WebSocket = require('ws');

const ws = new WebSocket('ws://192.168.1.76:80/ws');

ws.on('open', () => {
  console.log('Connected to WebSocket server');
  ws.send('hello');
});

ws.on('message', (data) => {
  console.log('Received message:', data);
});

ws.on('close', () => {
  console.log('Disconnected from WebSocket server');
});

ws.on('error', (error) => {
  console.error('WebSocket error:', error);
});


const WebSocket = require('ws');

let ws = null;

const connect = () => {
  ws = new WebSocket('ws://192.168.1.76:80/ws');

  ws.onopen = () => {
    console.log('webSocket connected');
    ws.send('hello');
  };

const util = require('util');

  ws.onmessage = (data) => {
    console.log('received message:', data.data);
  };

  ws.onclose = () => {
    console.log('webSocket disconnected');
    ws = null;
    setTimeout(() => {
      connect();
    }, 1000);
  };

  ws.onerror = (err) => {
    console.error('ws error: ', err.message);
    ws.close();
  };
}

connect();

process.stdin.setRawMode(true).setEncoding('utf8')
             .resume().on('data', key => {

  if (key === '\u0003' ) process.exit();

  if(ws === null) return;

  console.log(key);
  switch (key) {
    case 's': ws.send('setb');  break;
    case 'u': ws.send('up');    break;
    case 'd': ws.send('down');  break;
    case 'h': ws.send('hold');  break;
    case 'q': ws.send('query'); break;
  }
});

const util      = require('util');
const WebSocket = require('ws');

let ws = null;

const connect = () => {
  ws = new WebSocket('ws://192.168.1.79:80/ws');

  ws.onopen = () => {
    console.log('webSocket connected');
    // ws.send('query');
  };

  ws.onmessage = (data) => {
    const msg    = data.data;
    if (msg === 'ping') return;
    const msgObj = JSON.parse(msg);
    const disp   = util.inspect(msgObj, 
                        {showHidden: false, depth: null});
    console.log('received message:', disp);
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
  if(key = 'q') ws.send('query');
});

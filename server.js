/**
 * Canvas Cast (Serial)
 *
 * Use Canvas Cast over a USB serial interface.
 * This allows for larger displays, higher framerates and no Wifi requirements.
 *
 * Edit the config settings below to run `node server.js`
 */
const config = {
  serialPort: '/dev/tty.usbserial-1420', // ie. (COM1, /dev/tty-usbserial1)
  baudRate: 1000000,
  serverPort: 8081,
};

if (!config.serialPort) {
  console.error('ERROR: Please set a Serial port to continue.')
  return;
}

console.log('---\nWelcome to Canvas Cast (Serial)\n---');


// Start serial port
const SerialPort = require('serialport');

// List available serila devices
SerialPort.list().then((ports) => {
  console.log('---\nAvailable devices found:');
  ports.forEach((port) => {
    console.log(`${port.path}\t${port.manufacturer}`);
  });
  console.log('---');
});

// Connect to device
const serialDevice = new SerialPort(config.serialPort, {
  baudRate: config.baudRate,
});

// Start WebSocket Server
const WebSocketServer = require('ws').Server;
const wss = new WebSocketServer({port: config.serverPort});
let wsClient;

// Booted
console.log(`Serial server started on: ${config.serialPort}.`);
console.log(`Update your Canvas Cast config to use: localhost:${config.serverPort}`);

// WebSocket has connected
wss.on('connection', onConnected);
function onConnected(client) {
  // Only allow one client
  if (wsClient) {
    client.send('Busy');
    client.close();
    return;
  }

  // Welcome user
  wsClient = client;
  wsClient.send('Connected');

  // On message
  wsClient.on('message', onMessage);

  // On close
  wsClient.on('close', () => onClose(client));
}

// WebSocket has received a message/data
function onMessage(data) {
  switch (typeof data) {
    // Message, normally a command
    case 'string':
      console.log(`MSG: ${data}`);
      const command = data.split(':');

      switch(command[0]) {
        case 'BRIGHTNESS':
          const val = parseInt(command[1]);
          // Commands are prefaced with "C0FFEE"
          const cmdBytearray = new Uint8Array(4);
          cmdBytearray.set(new Uint8Array([0xC0, 0xFF, 0xEE, `0x${val.toString(16)}`]), 0);
          serialDevice.write(Buffer.from(cmdBytearray.buffer));
          break;
      }
    break;

    // Frame of data received as an object
    case 'object':
      // Prefix bytearray with start code "DEC0DE"
      const bytearray = new Uint8Array(data.byteLength + 3);
      bytearray.set(new Uint8Array([0xDE, 0xC0, 0xDE]), 0);
      bytearray.set(data, 3);

      // Send frame to device
      serialDevice.write(Buffer.from(bytearray.buffer));
    break;

    default:
  }
}

// On WS close
function onClose(client) {
  console.log('Goodbye');
  client.send('Goodbye');
  wsClient = null;
}

// Serial device is reporting an error
serialDevice.on('error', (err) => {
  console.log('Serial port error: ', err.message);
  if (wsClient) {
    wsClient.send('Closed');
  }
});

// Serial device connection has opened
serialDevice.on('open', () => {
  console.log('Serial port opened and ready!');
  if (wsClient) {
    wsClient.send('Connected');
  }
});

// Serial device connection closed
serialDevice.on('close', () => {
  console.log('Serial port closed.');
  if (wsClient) {
    wsClient.send('Closed');
  }
});

// Message received from serial device
serialDevice.on('read', (msg) => {
  console.log('Serial message:');
  console.log(msg);
});
